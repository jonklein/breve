#include "simulation.h"
#include "worldObject.h"
#include "glIncludes.h"
#include "sensor.h"

#include "render.h"
#include "world.h"

slWorldObject::slWorldObject() {
	_drawMode = 0;
	_texture = NULL;
	_textureMode = 0;
	_textureScaleX = 16;
	_textureScaleY = 16;
	_simulate = 0;
	_drawAsPoint = false;
	_drawShadow = true;

	_lightExposure = 0;

	_shape = NULL;
	_displayShape = NULL;

	_proximityRadius = 0.00001;

	_billboardRotation = 0;
	_alpha = 1.0;

	_userData = NULL;

	_e = 0.4;
	_mu = 0.15;

	slVectorSet(&_color, 1, 1, 1);

	slMatrixIdentity(_position.rotation);
	slVectorSet(&_position.location, 0, 0, 0);
}

slWorldObject::~slWorldObject() {
	std::vector<slObjectConnection*>::iterator ci;

	for(ci = _connections.begin(); ci != _connections.end(); ci++ ) {
		(*ci)->_src = NULL;
		(*ci)->_dst = NULL;
	}

	if( _shape ) 
		_shape -> release();

	if( _displayShape ) 
		_displayShape -> release();

}

void slWorldObject::draw( const slRenderGL& inRenderer ) {
	if ( _displayShape ) {
	
		inRenderer.PushMatrix( slMatrixGeometry );
		inRenderer.Translate( slMatrixGeometry, _position.location.x, _position.location.y, _position.location.z );

		inRenderer.MulMatrix( slMatrixGeometry, _position.rotation );
		inRenderer.MulMatrix( slMatrixGeometry, _displayShape -> _transform );

		if( _texture ) {	
			inRenderer.PushMatrix( slMatrixTexture );
			inRenderer.Translate( slMatrixTexture, 0.5, 0.5, 0.0 );

			if( _textureScaleX > 0.0 && _textureScaleY > 0.0 )
				inRenderer.Scale( slMatrixTexture, 1.0 / _textureScaleX, 1.0 / _textureScaleY, 1.0 );

			_texture -> bind();
		}
		
		float c[] = { _color.x, _color.y, _color.z, _alpha };
		inRenderer.SetBlendColor( c );

		_displayShape -> draw( inRenderer );

		if( _texture ) {
			inRenderer.PopMatrix( slMatrixTexture );
			_texture -> unbind();
		}
		
		inRenderer.PopMatrix( slMatrixGeometry );
	}
}

void slObjectLine::draw( slCamera *camera ) {	
	if( _src && _dst && _stipple ) {
		const slVector *v1 = &_src->getPosition().location;
		const slVector *v2 = &_dst->getPosition().location;

#ifndef OPENGLES
		glLineStipple( 2, _stipple );
		glEnable( GL_LINE_STIPPLE );
#endif

		slVertexBufferGL buffer;
		buffer.resize( 2, VB_XYZ );

		float *v;

		// float c[] = { _color.x, _color.y, _color.z, _transparency };
		// inRenderer.SetBlendColor( c );

		v = buffer.vertex( 0 );
		v[ 0 ] = v1 -> x;
		v[ 1 ] = v1 -> y;
		v[ 2 ] = v1 -> z;
		v = buffer.vertex( 1 );
		v[ 0 ] = v2 -> x;
		v[ 1 ] = v2 -> y;
		v[ 2 ] = v2 -> z;

		buffer.draw( VB_LINE_STRIP );

#ifndef OPENGLES
		glDisable( GL_LINE_STIPPLE );
#endif
	}
}

void slWorldObject::setCallbackData( void *data ) {
	_userData = data;
}

void *slWorldObject::getCallbackData() {
	return _userData;
}

void slWorldObject::setCollisionE( double e ) {
	_e = e;
}

void slWorldObject::setCollisionMU( double mu ) {
	_mu = mu;
}

void slWorldObject::setNeighborhoodSize( double size ) {
	_proximityRadius = size;
}

void slWorldObject::setTexture( slTexture2D *inTexture ) {
	_texture = inTexture;
}

void slWorldObject::setTextureMode( int m ) {
	_textureMode = m;
}

void slWorldObject::setDrawAsPoint( bool p ) {
	_drawAsPoint = p;
}

void slWorldObject::setTextureScale( double inTX, double inTY ) {
	_textureScaleX = inTX;
	_textureScaleY = inTY;
}

void slWorldObject::setBitmapRotation( double rot ) {
	_billboardRotation = rot;
}

void slWorldObject::addDrawMode( int mode ) {
	_drawMode |= mode;
}

void slWorldObject::removeDrawMode( int mode ) {
	if ( _drawMode & mode ) _drawMode ^= mode;
}

std::vector< void* > &slWorldObject::getNeighborData() {
	return _neighborData;
}

int slWorldObject::getLightExposure() {
	return _lightExposure;
}

void slWorldObject::setAlpha( double alpha ) {
	_alpha = alpha;
}

void slWorldObject::setColor( slVector *c ) {
	slVectorCopy( c, &_color );
}

void slWorldObject::setDrawShadows( bool inDraw ) {
	_drawShadow = inDraw;
}


double slWorldObject::irSense(slPosition *sensorPos, std::string sensorType){
	//ProximitySensor::getProximitySensor();
//	printf("proximiy Radius%f\n",_proximityRadius);
	return irSense2(&_neighbors, sensorPos, sensorType);
}

double slWorldObject::calculateQuality(slPosition* sensorPos, slVector* targetLoc, std::string sensorType, slWorldObject* target){
	if(target!=NULL){
		targetLoc = &target->_position.location;
	}
	return calculateQuality2(&_neighbors, sensorPos, targetLoc, sensorType, target);//sensor.cc
}

double slWorldObject::calcQualNoRay(slPosition* sensorPos, slVector* targetLoc, std::string sensorType, slWorldObject* target){
	if(target!=NULL){
		targetLoc = &target->_position.location;
	}
	return calcQualNoRay2(&_neighbors, sensorPos, targetLoc, sensorType, target);//sensor.cc
}
int slWorldObject::raytrace( slVector *location, slVector* direction, slVector *erg_dir ) {

	if ( !_shape ) {
		slMessage( DEBUG_ALL, "slWorldObject::raytrace: This WorldObject has no shape\n" );
		return -2;
	}

	//printf("proximiy Radius%f\n",_proximityRadius); // ahh falsches objekt!

	//slMessage(DEBUG_ALL, "slWorldObjectRaytrace: irSense:");
	// double r = irSense(direction, "PROXIMITY");
	// slMessage(DEBUG_ALL, "slWorldObject::raytrace: irSense: %f \n",r);

	//direction and location in wo's coordinates
	slVector dir_wo, dir_wo_help;

	slVector loc_wo_help;

	slVector loc_wo;

	slVector direction_norm;

	slVectorCopy( direction, &direction_norm );

	slVectorNormalize( &direction_norm );

	slVectorInvXform( _position.rotation, &direction_norm, &dir_wo_help );

	slVectorMul( &dir_wo_help, -1, &dir_wo );

	slVectorSub( location, &_position.location, &loc_wo_help );

	slVectorInvXform( _position.rotation, &loc_wo_help, &loc_wo );

	slVector point;

	if ( _shape->rayHitsShape( &dir_wo, &loc_wo, &point ) < 0 ) {
		slVectorSet( erg_dir, 0.0, 0.0, 0.0 ); //no hit
		return -1;
	}

	double d = slVectorLength( &point );

	slVectorMul( &direction_norm, d, erg_dir );

	return 0;
}

void slWorldObject::updateBoundingBox() {
	if ( _shape )
		_shape->bounds( &_position, &_min, &_max );
	else {
		slVectorSet( &_min, 0, 0, 0 );
		slVectorSet( &_max, 0, 0, 0 );
	}
}

void slWorldObject::getBounds( slVector *minBounds, slVector *maxBounds ) {
	if ( minBounds ) 
		slVectorCopy( &_min, minBounds );

	if ( maxBounds ) 
		slVectorCopy( &_max, maxBounds );
}

void slWorldObject::setShape( slShape *inShape ) {
	if( _shape ) 	
		_shape->release();

	_shape = inShape;
	_shape->retain();
}

void slWorldObject::setDisplayShape( slShape *inDisplayShape ) {
	if( _displayShape ) 	
		_displayShape->release();

	_displayShape = inDisplayShape;
	_displayShape->retain();
}

void slWorldObject::setRotation( double inRotation[ 3 ][ 3 ] ) {
	slMatrixCopy( inRotation, _position.rotation );
	updateBoundingBox();
}

void slWorldObject::setLocation( slVector *inLocation ) {
	slVectorCopy( inLocation, &_position.location );
	updateBoundingBox();
}
