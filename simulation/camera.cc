/*****************************************************************************
 *                                                                           *
 * The breve Simulation Environment                                          *
 * Copyright (C) 2000, 2001, 2002, 2003 Jonathan Klein                       *
 *                                                                           *
 * This program is free software; you can redistribute it and/or modify      *
 * it under the terms of the GNU General Public License as published by      *
 * the Free Software Foundation; either version 2 of the License, or         *
 * (at your option) any later version.                                       *
 *                                                                           *
 * This program is distributed in the hope that it will be useful,           *
 * but WITHOUT ANY WARRANTY; without even the implied warranty of            *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the             *
 * GNU General Public License for more details.                              *
 *                                                                           *
 * You should have received a copy of the GNU General Public License         *
 * along with this program; if not, write to the Free Software               *
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA *
 *****************************************************************************/

#include "slutil.h"
#include "simulation.h"
#include "camera.h"
#include "world.h"
#include "shape.h"
#include "glIncludes.h"

/*!
	\brief Creates a new camera of a given size.
*/

slCamera::slCamera( int x, int y ) {
	slCameraText tx;
	slCameraText &t = tx;

	unsigned int n;

	_activateContextCallback = NULL;
	_renderContextCallback = NULL;

	_text.insert( _text.begin(), 8, t );

	slVectorSet( &_textColor, 0, 0, 0 );
	_textScale = 1;

	_width = x;
	_height = y;
	_originx = 0;
	_originy = 0;

	_zClip = 500.0;
	_frontClip = 0.1;

	_fov = 40;

	// billboarding works poorly when all the billboards
	// are on the same plane, so we'll offset the camera
	// just slightly.  enough so that the alpha bending
	// works correctly, but not so that it's visable.

	_rx = 0.001;

	_ry = 0.001;

	_shadowCatcher = NULL;

	slVectorSet( &_target, 0, 0, 0 );
	slVectorSet( &_location, 0, 10, 100 );

	_zoom = 10;

	_drawFog = false;
	_drawLights = false;
	_drawShadow = false;
	_drawShadowVolumes = false;
	_drawOutline = false;
	_drawReflection = false;
	_drawText = true;
	_drawBlur = false;

	_blurFactor = 0.1;

	_billboardCount = 0;

	_maxBillboards = 8;

	_billboards = ( slBillboardEntry** )slMalloc( sizeof( slBillboardEntry* ) * _maxBillboards );

	_fogIntensity = .1;
	_fogStart = 10;
	_fogEnd = 40;

	_readbackTexture = new slTexture2D();

	for ( n = 0; n < _maxBillboards; n++ ) 
		_billboards[n] = new slBillboardEntry;

	_billboardBuffer.resize( 4, VB_XYZ | VB_UV );
}

void slCamera::updateFrustum() {
	slVector loc;
	float proj[16], frust[16], modl[16];

	glGetFloatv( GL_PROJECTION_MATRIX, proj );
	glGetFloatv( GL_MODELVIEW_MATRIX,  modl );

	// combine the two matrices (multiply projection by modelview)
	frust[ 0] = modl[ 0] * proj[ 0] + modl[ 1] * proj[ 4] + modl[ 2] * proj[ 8] + modl[ 3] * proj[12];
	frust[ 1] = modl[ 0] * proj[ 1] + modl[ 1] * proj[ 5] + modl[ 2] * proj[ 9] + modl[ 3] * proj[13];
	frust[ 2] = modl[ 0] * proj[ 2] + modl[ 1] * proj[ 6] + modl[ 2] * proj[10] + modl[ 3] * proj[14];
	frust[ 3] = modl[ 0] * proj[ 3] + modl[ 1] * proj[ 7] + modl[ 2] * proj[11] + modl[ 3] * proj[15];

	frust[ 4] = modl[ 4] * proj[ 0] + modl[ 5] * proj[ 4] + modl[ 6] * proj[ 8] + modl[ 7] * proj[12];
	frust[ 5] = modl[ 4] * proj[ 1] + modl[ 5] * proj[ 5] + modl[ 6] * proj[ 9] + modl[ 7] * proj[13];
	frust[ 6] = modl[ 4] * proj[ 2] + modl[ 5] * proj[ 6] + modl[ 6] * proj[10] + modl[ 7] * proj[14];
	frust[ 7] = modl[ 4] * proj[ 3] + modl[ 5] * proj[ 7] + modl[ 6] * proj[11] + modl[ 7] * proj[15];

	frust[ 8] = modl[ 8] * proj[ 0] + modl[ 9] * proj[ 4] + modl[10] * proj[ 8] + modl[11] * proj[12];
	frust[ 9] = modl[ 8] * proj[ 1] + modl[ 9] * proj[ 5] + modl[10] * proj[ 9] + modl[11] * proj[13];
	frust[10] = modl[ 8] * proj[ 2] + modl[ 9] * proj[ 6] + modl[10] * proj[10] + modl[11] * proj[14];
	frust[11] = modl[ 8] * proj[ 3] + modl[ 9] * proj[ 7] + modl[10] * proj[11] + modl[11] * proj[15];

	frust[12] = modl[12] * proj[ 0] + modl[13] * proj[ 4] + modl[14] * proj[ 8] + modl[15] * proj[12];
	frust[13] = modl[12] * proj[ 1] + modl[13] * proj[ 5] + modl[14] * proj[ 9] + modl[15] * proj[13];
	frust[14] = modl[12] * proj[ 2] + modl[13] * proj[ 6] + modl[14] * proj[10] + modl[15] * proj[14];
	frust[15] = modl[12] * proj[ 3] + modl[13] * proj[ 7] + modl[14] * proj[11] + modl[15] * proj[15];

	slVectorAdd( &_target, &_location, &loc );
	slVectorCopy( &loc, &_frustumPlanes[0].vertex );
	slVectorCopy( &loc, &_frustumPlanes[1].vertex );
	slVectorCopy( &loc, &_frustumPlanes[2].vertex );
	slVectorCopy( &loc, &_frustumPlanes[3].vertex );
	slVectorCopy( &loc, &_frustumPlanes[4].vertex );
	slVectorCopy( &loc, &_frustumPlanes[5].vertex );

	_frustumPlanes[0].normal.x = frust[3 ] - frust[0];
	_frustumPlanes[0].normal.y = frust[7 ] - frust[4];
	_frustumPlanes[0].normal.z = frust[11] - frust[8];

	_frustumPlanes[1].normal.x = frust[3 ] + frust[0];
	_frustumPlanes[1].normal.y = frust[7 ] + frust[4];
	_frustumPlanes[1].normal.z = frust[11] + frust[8];

	_frustumPlanes[2].normal.x = frust[3 ] - frust[1];
	_frustumPlanes[2].normal.y = frust[7 ] - frust[5];
	_frustumPlanes[2].normal.z = frust[11] - frust[9];

	_frustumPlanes[3].normal.x = frust[3 ] + frust[1];
	_frustumPlanes[3].normal.y = frust[7 ] + frust[5];
	_frustumPlanes[3].normal.z = frust[11] + frust[9];

	slVectorMul( &_location, -1.0, &_frustumPlanes[4].normal );

	// Is this correct?  I don't believe it is!
	slVectorCopy( &_location, &_frustumPlanes[5].normal );

	slVectorNormalize( &_frustumPlanes[0].normal );
	slVectorNormalize( &_frustumPlanes[1].normal );
	slVectorNormalize( &_frustumPlanes[2].normal );
	slVectorNormalize( &_frustumPlanes[3].normal );
	slVectorNormalize( &_frustumPlanes[4].normal );
	slVectorNormalize( &_frustumPlanes[5].normal );
}

/*
	\brief Tests whether a single point is inside the camera frustum.
*/

int slCamera::pointInFrustum( slVector *test ) {
	int n;

	for ( n = 0;n < 5;n++ )
		if ( slPlaneDistance( &_frustumPlanes[n], test ) < 0.0 ) return 0;

	return 1;
}

/*
	\brief Tests whether the min and max vectors of an object are inside the camera frustum.
*/

int slCamera::minMaxInFrustum( slVector *min, slVector *max ) {
	return ( pointInFrustum( min ) || pointInFrustum( max ) );
}

/*!
	\brief Tests whether a polygon is inside the camera frustum.
*/

int slCamera::polygonInFrustum( slVector *test, int n ) {
	int x;
	char violations[6] = { 0, 0, 0, 0, 0, 0 };
	int v = 0;

	for ( x = 0;x < n;x++ ) {
		int plane;

		for ( plane = 0;plane < 5;plane++ ) {
			if ( slPlaneDistance( &_frustumPlanes[plane], &test[x] ) < 0.0 ) {
				violations[plane]++;
				v++;
			}
		}
	}

	// no violations -- the polygon is entirely in the frustum

	if ( v == 0 ) return 1;

	// all violating on one side -- the polygon is perfectly excluded

	if ( violations[0] == n || violations[1] == n || violations[2] == n || violations[3] == n || violations[4] == n ) return 0;

	// multiple violations -- the polygon is possibly interesting the frustum

	return 1;
}

/*!
	\brief Frees the camera.
*/

slCamera::~slCamera() {
	delete _readbackTexture;

	for ( unsigned int n = 0; n < _maxBillboards; n++ ) 
		delete _billboards[ n ];

	slFree( _billboards );
}

/**
 * \brief Updates the camera's internal state after changes have been made.
 * 
 * Used to update the camera's internal state after changes have been made
 * to the rotation or zoom settings.
 */

void slCamera::update() {
	double m[3][3], n[3][3];
	slVector yaxis, xaxis, unit;

	slVectorSet( &unit, 0, 0, 1 );

	// build individual rotation matrices for rotation around
	// x and y, and then multiply them together

	slVectorSet( &yaxis, 0, 1, 0 );
	slVectorSet( &xaxis, 1, 0, 0 );

	if ( isnan( _rx ) ) _rx = 0.0;

	if ( isnan( _ry ) ) _ry = 0.0;

	_rx = fmod( _rx, M_PI * 2.0 );

	_ry = fmod( _ry, M_PI * 2.0 );

	if ( _rx < 0.0 ) _rx += 2.0 * M_PI;

	if ( _ry < 0.0 ) _ry += 2.0 * M_PI;

	slRotationMatrix( &yaxis, _ry, m );

	slRotationMatrix( &xaxis, _rx, n );

	slMatrixMulMatrix( m, n, _rotation );

	// preform the rotation around the unit vector

	slVectorXform( _rotation, &unit, &_location );

	// apply the zoom

	slVectorMul( &_location, _zoom, &_location );
}

/**
 * \brief Adds a string of text to the camera's output display.
 */

void slCamera::setCameraText( int n, char *string, float x, float y, slVector *v ) {
	if (( unsigned int )n >= _text.size() || n < 0 ) {
		slMessage( DEBUG_ALL, "out of bounds text position %d in slSetCameraText\n", n );
		return;
	}

	_text[n].text = string;

	_text[n].x = x;
	_text[n].y = y;

	if ( v ) slVectorCopy( v, &_text[n].color );
	else slVectorZero( &_text[n].color );
}

/*!
	\brief Sets an object to catch shadows and reflections.

	Take a certain shape and find the plane whose normal matches
	closest to a given normal, and set the plane to be the shadow
	catcher.
*/

void slCamera::setShadowCatcher( slStationary *s, slVector *normal ) {
	slFace *face;
	double best = 0.0, dot;
	std::vector< slFace* >::iterator fi;
	slFace *bestFace = NULL;

	const slPosition &position = s->getPosition();

	for ( fi = s->_shape->faces.begin(); fi != s->_shape->faces.end(); fi++ ) {
		face = *fi;

		dot = slVectorDot( &face->plane.normal, normal );

		if ( dot > best ) {
			bestFace = face;
			best = dot;
		}
	}

	if ( !bestFace ) {
		// this shouldn't happen
		_drawShadow = 0;
		return;
	}

	memcpy( &_shadowPlane, &bestFace->plane, sizeof( slPlane ) );

	slVectorAdd( &_shadowPlane.vertex, &position.location, &_shadowPlane.vertex );

	_recompile = 1;
	_shadowCatcher = s;
}




/**
 * \brief The sort function used to sort billboards from back to front.
 */

bool slBillboardCompare( const slBillboardEntry *a, const slBillboardEntry *b ) {
	return a -> z < b -> z;
}

/*!
 * \brief Renders a stationary object.
 */

void slCamera::processBillboards( slWorld *inWorld ) {
	GLfloat matrix[ 16 ];
	std::vector< slWorldObject* >::iterator wi;

	glGetFloatv( GL_MODELVIEW_MATRIX, matrix );

	_billboardCount = 0;

	for ( wi = inWorld->_objects.begin(); wi != inWorld->_objects.end(); wi++ ) {
		slWorldObject *wo = *wi;

		if ( wo && wo -> _textureMode != slBitmapNone && wo->_displayShape && wo->_displayShape->_type == ST_SPHERE ) {
			double z = 0;

			slSphere *ss = static_cast< slSphere* >( wo->_displayShape );

			z = matrix[2] * wo->_position.location.x + matrix[6] * wo->_position.location.y + matrix[10] * wo->_position.location.z;

			addBillboard( wo, ss -> radius(), z );
		}
	}

	if( _billboardCount == 0 ) 
		return;

	std::sort( _billboards, _billboards + _billboardCount, slBillboardCompare );

	_billboardX.x = matrix[0];
	_billboardX.y = matrix[4];
	_billboardX.z = matrix[8];

	_billboardY.x = matrix[1];
	_billboardY.y = matrix[5];
	_billboardY.z = matrix[9];

	_billboardZ.x = matrix[2];
	_billboardZ.y = matrix[6];
	_billboardZ.z = matrix[10];
	
	float *v;
	
	v = _billboardBuffer.texcoord( 0 );
	v[ 0 ] = 1.0;
	v[ 1 ] = 1.0;

	v = _billboardBuffer.texcoord( 1 );
	v[ 0 ] = 0.0;	
	v[ 1 ] = 1.0;
	
	v = _billboardBuffer.texcoord( 2 );
	v[ 0 ] = 1.0;
	v[ 1 ] = 0.0;
	
	v = _billboardBuffer.texcoord( 3 );
	v[ 0 ] = 0.0;
	v[ 1 ] = 0.0;

	v = _billboardBuffer.vertex( 0 );
	v[ 0 ] =  _billboardX.x + _billboardY.x;  
	v[ 1 ] =  _billboardX.y + _billboardY.y;
	v[ 2 ] =  _billboardX.z + _billboardY.z;

	v = _billboardBuffer.vertex( 1 );
	v[ 0 ] = -_billboardX.x + _billboardY.x;
	v[ 1 ] = -_billboardX.y + _billboardY.y;
	v[ 2 ] = -_billboardX.z + _billboardY.z;

	v = _billboardBuffer.vertex( 2 );
	v[ 0 ] =  _billboardX.x - _billboardY.x;
	v[ 1 ] =  _billboardX.y - _billboardY.y;
	v[ 2 ] =  _billboardX.z - _billboardY.z;

	v = _billboardBuffer.vertex( 3 );
	v[ 0 ] = -_billboardX.x - _billboardY.x;
	v[ 1 ] = -_billboardX.y - _billboardY.y;
	v[ 2 ] = -_billboardX.z - _billboardY.z;

}

/**
 * \brief Adds an entry for a billboard.
 *
 * This is used dynamically while rendering billboards in order to correctly render
 * them from back to front.
 */

void slCamera::addBillboard( slWorldObject *object, float size, float z ) {
	unsigned int n, last;

	if ( _billboardCount == _maxBillboards ) {
		last = _maxBillboards;
		_maxBillboards *= 2;

		_billboards = ( slBillboardEntry** )slRealloc( _billboards, sizeof( slBillboardEntry* ) * _maxBillboards );

		for ( n = last; n < _maxBillboards; n++ ) _billboards[n] = new slBillboardEntry;
	}

	_billboards[ _billboardCount ]->z = z;
	_billboards[ _billboardCount ]->size = size;
	_billboards[ _billboardCount ]->object = object;

	_billboardCount++;
}

/**
 *	\brief Renders preprocessed billboards.
 */

void slCamera::renderBillboards( slRenderGL& inRenderer ) {
	slVector normal;
	slBillboardEntry *b;
	unsigned int n;
	slTexture2D *lastTexture = NULL;

	if( _billboardCount == 0 )
		return;

	slVectorCopy( &_location, &normal );
	slVectorNormalize( &normal );

	// we do want to have a depth test against other objects in the world.
	// but we do our own back-to-front billboard sort and we do not want
	// them fighting in the depth buffer.  so we'll disable depth-buffer
	// writing so that no new info goes there.
	
	_billboardBuffer.bind();

	for ( n = 0; n < _billboardCount; n++ ) {
		slWorldObject *object;
		
		inRenderer.PushMatrix( slMatrixGeometry );

		b = _billboards[ n ];
		object = b->object;

		unsigned char color[ 4 ] = { object->_color.x * 255.0f, object->_color.y * 255.0f, object->_color.z * 255.0f, object->_alpha * 255.0f };
		inRenderer.SetBlendColor( color );

		if ( lastTexture != object -> _texture ) {
			object -> _texture -> bind();
			lastTexture = object->_texture;
		}

		inRenderer.SetBlendMode( object -> _textureMode == slLightmap ? slBlendLight : slBlendAlpha );
		
		inRenderer.Translate( slMatrixGeometry, object->_position.location.x, object->_position.location.y, object->_position.location.z );
		inRenderer.Rotate( slMatrixGeometry, object->_billboardRotation, normal.x, normal.y, normal.z );
		inRenderer.Scale( slMatrixGeometry, b -> size, b -> size, b -> size );
		
		_billboardBuffer.draw( VB_TRIANGLE_STRIP );

		inRenderer.PopMatrix( slMatrixGeometry );
	}
	
	lastTexture -> unbind();

	_billboardBuffer.unbind();
}






/**
 * \brief Sets the size of the camera window.
 */

void slCamera::setBounds( unsigned int nx, unsigned int ny ) {
	_width = nx;
	_height = ny;
	_fov = ( double )_width / ( double )_height;
}

/**
 * \brief Gets the size of the camera window.
 */

void slCamera::getBounds( unsigned int *nx, unsigned int *ny ) {
	*nx = _width;
	*ny = _height;
}

/**
 * \brief Gets the camera's x and y rotation.
 */

void slCamera::getRotation( double *x, double *y ) {
	*x = _rx;
	*y = _ry;
}

/**
 * \brief Gets the camera's x,y,z position
 */

void slCamera::getPosition( double *x, double *y, double *z ) {
	*x = _location.x;
	*y = _location.y;
	*z = _location.z;
}

/*!
	\brief Sets the recompile flag for this camera, indicating that drawlists
	need to be recompiled.
*/

void slCamera::setRecompile() {
	_recompile = 1;
}

void slCamera::setActivateContextCallback( int( *f )() ) {
	_activateContextCallback = f;
}

/*!
	\brief Moves the camera in resoponse to a mouse movement.

	This is called by the interface when a mouse movement is made
	with the motion modifier key pressed, or with the motion tool is
	selected.
*/

void slCamera::moveWithMouseMovement( double dx, double dy ) {
	slVector xaxis, yaxis, tempV, location;

	slVectorSet( &yaxis, 0, 1, 0 );
	slVectorCopy( &_location, &location );

	slVectorCross( &location, &yaxis, &xaxis );
	slVectorNormalize( &xaxis );
	slVectorMul( &xaxis, _zoom * -dx / 100.0, &tempV );
	slVectorSub( &_target, &tempV, &_target );

	slVectorCross( &location, &xaxis, &tempV );
	slVectorNormalize( &tempV );

	if ( tempV.y > 0 ) slVectorMul( &tempV, -1, &tempV );

	slVectorMul( &tempV, _zoom * -dy / 100.0, &tempV );

	slVectorAdd( &_target, &tempV, &_target );
}

/*!
	\brief Rotates the camera in resoponse to a mouse movement.

	This is called by the interface when a mouse movement is made
	with the rotate modifier key pressed, or with the rotate tool is
	sineelected.
*/

void slCamera::rotateWithMouseMovement( double dx, double dy ) {
	if ( _rx > M_PI / 2.0 && _rx < 3.0 / 2.0 * M_PI ) dy *= -1;

	_ry -= dx * .01;

	_rx -= dy * .01;

	update();
}

/*!
	\brief Zooms the camera in resoponse to a mouse movement.

	This is called by the interface when a mouse movement is made
	with the zoom modifier key pressed, or with the zoom tool is
	selected.
*/

void slCamera::zoomWithMouseMovement( double dx, double dy ) {
	if ( 0.1 * dy < _zoom ) {
		_zoom -= 0.002 * _zoom * dy;
		update();
	}
}
