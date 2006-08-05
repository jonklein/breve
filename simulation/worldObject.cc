#include "simulation.h"
#include "worldObject.h"
#include "glIncludes.h"
#include "sensor.h"

void slWorldObject::draw(slCamera *camera) {
	if ( _shape )
		_shape->draw( camera, &_position, _textureScaleX, _textureScaleY, _drawMode, 0 );
}

void slObjectLine::draw(slCamera *camera) {
	const slVector *x, *y;

	if (!_src || !_dst) return;

	if (!_stipple) return;

	x = &_src->getPosition().location;
	y = &_dst->getPosition().location;

	glLineStipple(2, _stipple);
	glEnable(GL_LINE_STIPPLE);

	glColor4f(_color.x, _color.y, _color.z, 0.8);

	glBegin(GL_LINES);

	glVertex3f( x->x, x->y, x->z );
	glVertex3f( y->x, y->y, y->z );

	glEnd();

	glDisable(GL_LINE_STIPPLE);
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

void slWorldObject::setCollisionET( double eT ) {
	_eT = eT;
}

void slWorldObject::setCollisionMU( double mu ) {
	_mu = mu;
}

void slWorldObject::setNeighborhoodSize( double size ) {
	_proximityRadius = size;
}

/*!
	\brief Sets the texture number for this object.

	Previous versions of this library used -1 to indicate no texture,
	while the current version uses 0.  This function recognizes negative
	numbers for backward compatability.
*/

void slWorldObject::setTexture( int t ) {
	_texture = ( t > 0) ? t : 0;
}

void slWorldObject::setTextureMode( int m ) {
	_textureMode = m;
}

void slWorldObject::setDrawAsPoint( bool p ) {
	_drawAsPoint = p;
}

void slWorldObject::setTextureScale( double sx, double sy ) {
	_textureScaleX = sx;
	_textureScaleY = sy;

	if( _shape )
		_shape->recompile();
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
	slVectorCopy( c, &_color);
}

double slWorldObject::irSense(slPosition *sensorPos, std::string sensorType){
	//ProximitySensor::getProximitySensor();
//	printf("proximiy Radius%f\n",_proximityRadius);
	return irSense2(&_neighbors, sensorPos, sensorType);
}

int slWorldObject::raytrace( slVector *location, slVector* direction, slVector *erg_dir ) {

	if ( !_shape ) {
		slMessage(DEBUG_ALL, "slWorldObject::raytrace: This WorldObject has no shape\n");
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
   
	slVectorCopy(direction, &direction_norm);
	slVectorNormalize(&direction_norm);
 
	slVectorInvXform( _position.rotation, &direction_norm, &dir_wo_help);
	slVectorMul(&dir_wo_help, -1, &dir_wo);   

	slVectorSub(location, &_position.location, &loc_wo_help);
	slVectorInvXform( _position.rotation, &loc_wo_help, &loc_wo);

//   slMessage(DEBUG_ALL, " [ %f, %f, %f ] %f %f ", dir_wo.x, dir_wo.y, dir_wo.z, atan2(dir_wo.z, dir_wo.x)*180/M_PI, atan2(direction->z, direction->x)*180/M_PI );
//   slMessage(DEBUG_ALL, " [ %f, %f, %f ] ", loc_wo.x, loc_wo.y, loc_wo.z );
 
	slVector point;

	if ( _shape->rayHitsShape( &dir_wo, &loc_wo, &point ) < 0 ) {
		slVectorSet(erg_dir, 0.0, 0.0, 0.0); //no hit
		return -1;
	}

	double d = slVectorLength(&point);
	slVectorMul(&direction_norm, d, erg_dir);
    
//	slMessage(DEBUG_ALL, "wo: erg_dir: [ %f, %f, %f ] distance:%f \n", erg_dir->x, erg_dir->y, erg_dir->z ,slVectorLength(erg_dir));

	return 0;
}

void slWorldObject::updateBoundingBox() {
	if( !_shape ) 
		_shape->bounds( &_position, &_min, &_max );
}

void slWorldObject::getBounds(slVector *minBounds, slVector *maxBounds) {
	if( minBounds ) slVectorCopy( &_min, minBounds );
	if( maxBounds ) slVectorCopy( &_max, maxBounds );
}


