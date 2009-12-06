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

/*! \addtogroup InternalFunctions */
/*@{*/

#include "kernel.h"
#include "camera.h"
#include "world.h"
#include "gldraw.h"

#define BRCAMERAPOINTER(p)	((slCamera*)BRPOINTER(p))

/*!
	\brief Sets the z-clipping plane distance for a camera.

	void cameraSetZClip(slCamera pointer camera, int distance).
*/

int brICameraSetZClip( brEval args[], brEval *target, brInstance *i ) {
	slCamera *camera = BRCAMERAPOINTER( &args[0] );
	camera->_zClip = abs( BRINT( &args[1] ) );
	return EC_OK;
}

/**
 * \brief Clears the camera with the background color.
 */

int brICameraClear( brEval args[], brEval *target, brInstance *i ) {
	slCamera *camera = BRCAMERAPOINTER( &args[0] );

	if ( camera->_activateContextCallback ) 
		camera -> _activateContextCallback();

    slMessage( DEBUG_ALL, "cameraClear not implemented\n" );

	return EC_OK;
}

/*!
	\brief Sets OpenGL smoothing for a camera.

	void cameraSetDrawBlur(int blur).
*/

int brICameraSetBlur( brEval args[], brEval *target, brInstance *i ) {
	slCamera *camera = BRCAMERAPOINTER( &args[0] );

	if( camera -> _activateContextCallback ) 
		camera -> _activateContextCallback();

	camera -> _drawBlur = BRINT( &args[1] );
	camera -> setRecompile();

	return EC_OK;
}

/*!
	\brief Sets OpenGL smoothing for a camera.

	void cameraSetBlurFactor(double factor).
*/

int brICameraSetBlurFactor( brEval args[], brEval *target, brInstance *i ) {
	slCamera *camera = BRCAMERAPOINTER( &args[0] );
	camera->_blurFactor = BRDOUBLE( &args[1] );
	camera->setRecompile();
	return EC_OK;
}

/*!
	\brief Sets OpenGL fog for a camera.

	void cameraSetDrawFog(int fog).
*/

int brICameraSetDrawFog( brEval args[], brEval *target, brInstance *i ) {
	slCamera *camera = BRCAMERAPOINTER( &args[0] );
	camera->_drawFog = BRINT( &args[1] );
	camera->setRecompile();
	return EC_OK;
}

/*!
	\brief Set OpenGL fog intensity for a camera.

	void setFogIntensity(double intensity).
*/

int brICameraSetFogIntensity( brEval args[], brEval *target, brInstance *i ) {
	slCamera *camera = BRCAMERAPOINTER( &args[0] );
	camera->_fogIntensity = BRDOUBLE( &args[1] );
	return EC_OK;
}

/*!
	\brief Set OpenGL fog color for a camera.

	void setFogColor(vector color).
*/

int brICameraSetFogColor( brEval args[], brEval *target, brInstance *i ) {
	slCamera *camera = BRCAMERAPOINTER( &args[0] );
	slVectorCopy( &BRVECTOR( &args[1] ), &camera->_fogColor );
	return EC_OK;
}

/*!
	\brief Positions the vision in the window.

	void cameraPositionDisplay(slCamera pointer camera, int x, int y).
*/

int brICameraPositionDisplay( brEval args[], brEval *target, brInstance *i ) {
	slCamera *camera = BRCAMERAPOINTER( &args[0] );
	int x = BRINT( &args[1] );
	int y = BRINT( &args[2] );

	camera->_originx = x;
	camera->_originy = y;

	return EC_OK;
}

/*!
	\brief Set the size of a perspective in the window.

	void cameraResizeDisplay(slCamera pointer camera, int x, int y).
*/

int brICameraResizeDisplay( brEval args[], brEval *target, brInstance *i ) {
	slCamera *camera = BRCAMERAPOINTER( &args[0] );

	camera->setBounds( BRINT( &args[1] ), BRINT( &args[2] ) );

	return EC_OK;
}

/**
 * Gets the camera X-size.
 */

int brICameraGetWidth( brEval args[], brEval *target, brInstance *i ) {
	slCamera *camera = BRCAMERAPOINTER( &args[0] );

	target->set( camera->_width );

	return EC_OK;
}

/**
 * Gets the camera Y-size.
 */

int brICameraGetHeight( brEval args[], brEval *target, brInstance *i ) {
	slCamera *camera = BRCAMERAPOINTER( &args[0] );

	target->set( camera->_height );

	return EC_OK;
}

/*!
	\brief Creates a new vision.

	slCamera pointer cameraNew().
*/

int brICameraNew( brEval args[], brEval *target, brInstance *i ) {
	slCamera *camera;

	camera = new slCamera( 0, 0 );

	slVectorSet( &camera->_target, 1, 0, 0 );
	slVectorSet( &camera->_location, 0, 0, 0 );

	target->set( camera );

	i->engine->world->addCamera( camera );

	return EC_OK;
}

/*!
	\brief Frees an slCamera.

	void cameraFree(slCamera pointer).
*/

int brICameraFree( brEval args[], brEval *target, brInstance *i ) {
	slCamera *camera = BRCAMERAPOINTER( &args[0] );

	i->engine->world->removeCamera( camera );

	delete camera;

	return EC_OK;
}

/*!
	\brief Positions a vision's camera.

	void cameraPosition(slCamera pointer, vector, vector).

	The first vector the the location of the camera, the second is the
	target of the camera.
*/

int brICameraPosition( brEval args[], brEval *target, brInstance *i ) {
	slCamera *camera = BRCAMERAPOINTER( &args[0] );
	slVector *l = &BRVECTOR( &args[1] );
	slVector *t = &BRVECTOR( &args[2] );

	slVectorCopy( l, &camera->_location );
	slVectorCopy( t, &camera->_target );

	return EC_OK;
}

/*!
	\brief Enables or disables a camera.

	void cameraSetEnabled(slCamera pointer, int).
*/

int brICameraSetEnabled( brEval args[], brEval *target, brInstance *i ) {
	// slCamera *camera = BRCAMERAPOINTER(&args[0]);
	// camera->_enabled = BRINT(&args[1]);

	return EC_OK;
}

/*!
	\brief Sets the rotation for a camera

	void cameraSetRotation(slCamera pointer, float, float).
*/

int brICameraSetRotation( brEval args[], brEval *target, brInstance *i ) {
	slCamera *camera = BRCAMERAPOINTER( &args[0] );
	camera->_rx = BRDOUBLE( &args[1] );
	camera->_ry = BRDOUBLE( &args[2] );

	camera->update();

	return EC_OK;
}

/*!
	\brief Gets the rotation for a camera

	void cameraGetRotation(slCamera pointer, int).
*/

int brICameraGetRotation( brEval args[], brEval *target, brInstance *i ) {
	slCamera *camera = BRCAMERAPOINTER( &args[0] );
	slVector v;

	v.x = camera->_rx;
	v.y = camera->_ry;
	v.z = 0;

	target->set( v );

	return EC_OK;
}

/*!
	\brief Gets the position for a camera

	void cameraGetPosition(slCamera pointer, int).
*/

int brICameraGetPosition( brEval args[], brEval *target, brInstance *i ) {
	slCamera *camera = BRCAMERAPOINTER( &args[0] );
	slVector v;

	v.x = camera->_location.x;
	v.y = camera->_location.y;
	v.z = camera->_location.z;

	target->set( v );

	return EC_OK;
}

/**
 * Enables or disables a camera's text display.
 * void cameraTextSetEnabled(slCamera pointer, int).
 */

int brICameraTextSetEnabled( brEval args[], brEval *target, brInstance *i ) {
	slCamera *camera = BRCAMERAPOINTER( &args[0] );

	camera->_drawText = BRINT( &args[1] );

	return EC_OK;
}

/**
 * Sets the color of the display text.
 */

int brICameraSetTextColor( brEval args[], brEval *target, brInstance *i ) {
	slCamera *camera = BRCAMERAPOINTER( &args[0] );
	slVector *color = &BRVECTOR( &args[1] );

	camera->setTextColor( color );

	return EC_OK;
}


/*@}*/

void breveInitCameraFunctions( brNamespace *n ) {
	brNewBreveCall( n, "cameraSetZClip", brICameraSetZClip, AT_NULL, AT_POINTER, AT_INT, 0 );
	brNewBreveCall( n, "cameraPositionDisplay", brICameraPositionDisplay, AT_NULL, AT_POINTER, AT_INT, AT_INT, 0 );
	brNewBreveCall( n, "cameraResizeDisplay", brICameraResizeDisplay, AT_NULL, AT_POINTER, AT_INT, AT_INT, 0 );
	brNewBreveCall( n, "cameraSetRotation", brICameraSetRotation, AT_NULL, AT_POINTER, AT_DOUBLE, AT_DOUBLE, 0 );
	brNewBreveCall( n, "cameraGetRotation", brICameraGetRotation, AT_VECTOR, AT_POINTER, 0 );
	brNewBreveCall( n, "cameraGetPosition", brICameraGetPosition, AT_VECTOR, AT_POINTER, 0 );	
	brNewBreveCall( n, "cameraNew", brICameraNew, AT_POINTER, 0 );
	brNewBreveCall( n, "cameraSetBlur", brICameraSetBlur, AT_NULL, AT_POINTER, AT_INT, 0 );
	brNewBreveCall( n, "cameraClear", brICameraClear, AT_NULL, AT_POINTER, 0 );

	brNewBreveCall( n, "cameraSetTextColor", brICameraSetTextColor, AT_NULL, AT_POINTER, AT_VECTOR, 0 );
	brNewBreveCall( n, "cameraGetHeight", brICameraGetHeight, AT_INT, AT_POINTER, 0 );
	brNewBreveCall( n, "cameraGetWidth", brICameraGetWidth, AT_INT, AT_POINTER, 0 );

	brNewBreveCall( n, "cameraSetDrawFog", brICameraSetDrawFog, AT_NULL, AT_POINTER, AT_INT, 0 );
	brNewBreveCall( n, "cameraSetFogIntensity", brICameraSetDrawFog, AT_NULL, AT_POINTER, AT_DOUBLE, 0 );
	brNewBreveCall( n, "cameraSetFogColor", brICameraSetFogColor, AT_NULL, AT_POINTER, AT_VECTOR, 0 );

	brNewBreveCall( n, "cameraSetBlurFactor", brICameraSetBlurFactor, AT_NULL, AT_POINTER, AT_DOUBLE, 0 );
	brNewBreveCall( n, "cameraFree", brICameraFree, AT_NULL, AT_POINTER, 0 );
	brNewBreveCall( n, "cameraPosition", brICameraPosition, AT_NULL, AT_POINTER, AT_VECTOR, AT_VECTOR, 0 );
	brNewBreveCall( n, "cameraSetEnabled", brICameraSetEnabled, AT_NULL, AT_POINTER, AT_INT, 0 );
	brNewBreveCall( n, "cameraTextSetEnabled", brICameraTextSetEnabled, AT_NULL, AT_POINTER, AT_INT, 0 );
}
