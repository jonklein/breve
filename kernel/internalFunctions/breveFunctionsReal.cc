
/*****************************************************************************
 * The breve Simulation Environment                                          *
 * Copyright (C) 2000-2004 Jonathan Klein                                    *
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

#include "kernel.h"
#include "breveFunctionsImage.h"
#include "world.h"

/*! \addtogroup InternalFunctions */
/*@{*/

#define BRWORLDOBJECTPOINTER(p) ((slWorldObject*)BRPOINTER(p))

/**
 * \brief Removes an object from the world and frees it.
 * removeObject(slWorldObject pointer).
 */

int brIRealRemoveObject( brEval args[], brEval *target, brInstance *i ) {
	i->engine->world->removeObject( BRWORLDOBJECTPOINTER( &args[0] ) );

	delete BRWORLDOBJECTPOINTER( &args[0] );

	return EC_OK;
}


/**
 * Sets the collision shape of a worldObject.
 * void realSetShape(slWorldObject pointer, slShape pointer).
 */

int brIRealSetShape( brEval args[], brEval *target, brInstance *i ) {
	slWorldObject *w = BRWORLDOBJECTPOINTER( &args[0] );
	slShape *s = ( slShape* )BRPOINTER( &args[1] );

	w->setShape( s );

	return EC_OK;
}

/**
 * Sets the display shape of a worldObject.
 * void realSetDisplayShape(slWorldObject pointer, slShape pointer).
 */

int brIRealSetDisplayShape( brEval args[], brEval *target, brInstance *i ) {
	slWorldObject *w = BRWORLDOBJECTPOINTER( &args[0] );
	slShape *s = ( slShape* )BRPOINTER( &args[1] );

	w->setDisplayShape( s );

	return EC_OK;
}

/**
 * Moves a worldObject to a new location.
 * void realSetLocation(slLink pointer, vector).
 */

int brIRealSetLocation( brEval args[], brEval *target, brInstance *i ) {
	slWorldObject *w = BRWORLDOBJECTPOINTER( &args[0] );
	slVector *v = &BRVECTOR( &args[1] );

	w->setLocation( v );

	return EC_OK;
}

/**
 * Sets the rotation of a worldObject to the given matrix.
 * void realSetRotationMatrix(slLink pointer, matrix).
 */

int brIRealSetRotationMatrix( brEval args[], brEval *target, brInstance *i ) {
	slWorldObject *w = BRWORLDOBJECTPOINTER( &args[0] );

	w->setRotation( BRMATRIX( &args[1] ) );

	return EC_OK;
}

/**
 * Sets the rotation of a worldObject to an angle about a given matrix.
 * void realSetRotation(slLink pointer, vector, double).
 */

int brIRealSetRotation( brEval args[], brEval *target, brInstance *i ) {
	slWorldObject *w = BRWORLDOBJECTPOINTER( &args[0] );
	slVector *v = &BRVECTOR( &args[1] );
	double len = BRDOUBLE( &args[2] );
	double m[3][3];

	slRotationMatrix( v, len, m );
	w->setRotation( m );

	return EC_OK;
}

/**
 * Returns the location of a link.
 * vector realGetLocation(slLink pointer).
 */

int brIRealGetLocation( brEval args[], brEval *target, brInstance *i ) {
	slWorldObject *worldObject = BRWORLDOBJECTPOINTER( &args[0] );

	target->set( worldObject->getPosition().location );

	return EC_OK;
}

/**
 * Returns the rotation matrix of a link.
 * matrix linkGetRotation(slLink pointer).
 */

int brIRealGetRotation( brEval args[], brEval *target, brInstance *i ) {
	slWorldObject *worldObject = BRWORLDOBJECTPOINTER( &args[0] );

	target->set( worldObject->getPosition().rotation );

	return EC_OK;
}




/**
 * Set the collision properties (mu, e, eT) of a stationary object.
 * void setCollisionProperties(slWorldObject pointer stationary, double e, double eT, double mu).
 */

int brIRealSetCollisionProperties( brEval args[], brEval *target, brInstance *i ) {
	slWorldObject *o = BRWORLDOBJECTPOINTER( &args[0] );

	double e = BRDOUBLE( &args[ 1 ] );
	double mu = BRDOUBLE( &args[ 2 ] );

	o->setCollisionE( e );
	o->setCollisionMU( mu );

	return EC_OK;
}



/**
	\brief Sets the neighborhood size for an object.

	setNeighborhoodSize(slWorld

	The neighborhood size specifies how far an object looks when collecting
	neighbor information.
*/

int brIRealSetNeighborhoodSize( brEval args[], brEval *target, brInstance *i ) {
	slWorldObject *wo = BRWORLDOBJECTPOINTER( &args[0] );
	double size = BRDOUBLE( &args[1] );

	wo->setNeighborhoodSize( size );

	return EC_OK;
}


/**
 * Gets an object's neighbors.
 * list getNeighbors(slWorldObject pointer).
 */ 

int brIRealGetNeighbors( brEval args[], brEval *target, brInstance *i ) {
	slWorldObject *wo = BRWORLDOBJECTPOINTER( &args[0] );
	std::vector< void* >::iterator wi;
	brEval eval;

	target->set( new brEvalListHead() );

	std::vector< void* > &neighbors = wo->getNeighborData();

	for ( wi = neighbors.begin(); wi != neighbors.end(); wi++ ) {
		// grab the neighbor instances from the userData of the neighbors

		eval.set(( brInstance* )( *wi ) );

		if ( BRINSTANCE( &eval ) && BRINSTANCE( &eval )->status == AS_ACTIVE )
			brEvalListInsert( BRLIST( target ), 0, &eval );
	}

	return EC_OK;
}

/**
 * Returns the light exposure of a single object in the world, if light exposure detection is in use.
 * int realGetLightExposure(slWorldObject pointer).
 */

int brIRealGetLightExposure( brEval args[], brEval *target, brInstance *i ) {
	slWorldObject *wo = BRWORLDOBJECTPOINTER( &args[0] );

	target->set( wo->getLightExposure() );

	return EC_OK;
}


/**
 * Enables or disables drawing of the bounding box for an object.
 * setBoundingBox(slWorldObject pointer, int).
 */

int brIRealSetBoundingBox( brEval args[], brEval *target, brInstance *i ) {
	slWorldObject *o = BRWORLDOBJECTPOINTER( &args[0] );
	int value = BRINT( &args[1] );

	if ( value ) o->addDrawMode( DM_BOUND );
	else o->removeDrawMode( DM_BOUND );

	return EC_OK;
}

/**
 * Enables or disables drawing of the axis lines for an object.
 * setDrawAxis(slWorldObject pointer, int).
 */

int brIRealSetDrawAxis( brEval args[], brEval *target, brInstance *i ) {
	slWorldObject *o = BRWORLDOBJECTPOINTER( &args[0] );
	int value = BRINT( &args[1] );

	if ( !o ) {
		slMessage( DEBUG_ALL, "null pointer passed to setDrawAxis\n" );
		return EC_OK;
	}

	if ( value ) o->addDrawMode( DM_AXIS );
	else o->removeDrawMode( DM_AXIS );

	return EC_OK;
}

/**
 * Enables or disables drawing of the neighbor lines for an object.
 * setNeighborLines(slWorldObject pointer, int).
 */

int brIRealSetNeighborLines( brEval args[], brEval *target, brInstance *i ) {
	slWorldObject *o = BRWORLDOBJECTPOINTER( &args[0] );
	int value = BRINT( &args[1] );

	if ( value ) o->addDrawMode( DM_NEIGHBOR_LINES );
	else o->removeDrawMode( DM_NEIGHBOR_LINES );

	return EC_OK;
}

/**
 * Enables or disables drawing of an object entirely.
 * setVisible(slWorldObject pointer, int).
 */

int brIRealSetVisible( brEval args[], brEval *target, brInstance *i ) {
	slWorldObject *o = BRWORLDOBJECTPOINTER( &args[0] );
	int visible = BRINT( &args[1] );

	if ( !visible ) o->addDrawMode( DM_INVISIBLE );
	else o->removeDrawMode( DM_INVISIBLE );

	return EC_OK;
}

/**
 * Sets the texture for an object.
 * setTexture(slWorldObject pointer, int).
 */

int brIRealSetTexture( brEval args[], brEval *target, brInstance *i ) {
	slWorldObject *o = BRWORLDOBJECTPOINTER( &args[ 0 ] );
	slTexture2D *t = ( (brImageData*)BRPOINTER( &args[ 1 ] ) ) -> _texture;

	o->setTexture( t );
	o->setTextureMode( BBT_NONE );

	return EC_OK;
}

/**
 * Sets the texture scale for an object.
 * setTexture(slWorldObject pointer, float, float).
 */
 
int brIRealSetTextureScale( brEval args[], brEval *target, brInstance *i ) {
	slWorldObject *o = BRWORLDOBJECTPOINTER( &args[0] );
	float valueX = BRDOUBLE( &args[1] );
	float valueY = BRDOUBLE( &args[2] );

	if ( valueX < 0.0 ) {
		slMessage( DEBUG_ALL, "warning: texture scale must be positive (%f)\n", valueX );
		valueX = 0.001;
	}

	if ( valueY < 0.0 ) {
		slMessage( DEBUG_ALL, "warning: texture scale must be positive (%f)\n", valueY );
		valueY = 0.001;
	}

	o->setTextureScale( valueX, valueY );

	return EC_OK;
}

/**
 * Sets a billboard bitmap rendering for an object.
 * setBitmap(slWorldObject pointer, int).
 */

int brIRealSetBitmap( brEval args[], brEval *target, brInstance *i ) {
	slWorldObject *o = BRWORLDOBJECTPOINTER( &args[ 0 ] );
	slTexture2D *t = ( (brImageData*)BRPOINTER( &args[ 1 ] ) ) -> _texture;

	o->setTexture( t );
	o->setTextureMode( BBT_BITMAP );

	return EC_OK;
}


/**
 * Sets a billboard bitmap rotation for an object.
 * setBitmapRotation(slWorldObject pointer, float ).
 */

int brIRealSetBitmapRotation( brEval args[], brEval *target, brInstance *i ) {
	slWorldObject *o = BRWORLDOBJECTPOINTER( &args[0] );

	o->setBitmapRotation( RADTODEG( BRDOUBLE( &args[1] ) ) );

	return EC_OK;
}

/**
 * Sets the billboard bitmap rotation towards a real world vector.
 *
 * step 1: compute the vector projection on the viewer's perspective plane.
 * this is c . (nc + v) where c is the camera vector, v is the
 * vector in question, and n is some constant.
 * step 2: it becomes n * c.c + c.v = 0
 * step 3: c.c = 1, so n = -c.v
 */

int brIRealSetBitmapRotationTowardsVector( brEval args[], brEval *target, brInstance *i ) {
	slWorldObject *o = BRWORLDOBJECTPOINTER( &args[0] );
	slVector *v = &BRVECTOR( &args[1] );
	slVector offset, vdiff, vproj, up, axis;
	double dot, rotation;

	slVectorSet( &up, 0, 1, 0 );

	slVectorCopy( &i->engine->camera->_location, &offset );
	slVectorNormalize( &offset );
	slVectorNormalize( v );

	slVectorCross( &up, &offset, &axis );
	slVectorCross( &offset, &axis, &up );

	/* c.v = -n */

	dot = -slVectorDot( &offset, v );

	slVectorMul( &offset, dot, &vdiff );
	slVectorAdd( &vdiff, v, &vproj );

	// we need to get the scalar rotation of v about offset vector

	rotation = acos( slVectorDot( &up, &vproj ) );

	if ( slVectorDot( &axis, &vproj ) > 0.0 ) rotation = 2 * M_PI - rotation;

	o->setBitmapRotation( RADTODEG( rotation ) );

	return EC_OK;
}

/**
 * Sets the alpha value for an object
 * setAlpha(slWorldObject pointer, float ).
 */
 
int brIRealSetAlpha( brEval args[], brEval *target, brInstance *i ) {
	slWorldObject *o = BRWORLDOBJECTPOINTER( &args[0] );

	o->setAlpha( BRDOUBLE( &args[1] ) );

	return EC_OK;
}

/**
 * Sets a lightmap texture for an object
 * setLightmap(slWorldObject pointer, int ).
 */
 
int brIRealSetLightmap( brEval args[], brEval *target, brInstance *i ) {
	slWorldObject *o = BRWORLDOBJECTPOINTER( &args[ 0 ] );
	slTexture2D *t = ( (brImageData*)BRPOINTER( &args[ 1 ] ) ) -> _texture;

	o->setTexture( t );
	o->setTextureMode( BBT_LIGHTMAP );

	return EC_OK;
}


/**
 * Sets shadow drawing for an object
 * setDrawShadows(slWorldObject pointer, int ).
 */

int brIRealSetDrawShadows( brEval args[], brEval *target, brInstance *i ) {
	slWorldObject *o = BRWORLDOBJECTPOINTER( &args[0] );
	o->setDrawShadows( BRINT( &args[ 1 ] ) );
	return EC_OK;
}

/**
 * Enables or disables point drawing for an object.
 * setDrawAsPoint( slWorldObject pointer, int )
 */

int brIRealSetDrawAsPoint( brEval args[], brEval *target, brInstance *i ) {
	BRWORLDOBJECTPOINTER( &args[0] )->setDrawAsPoint( BRINT( &args[1] ) );
	return EC_OK;
}

/**
 * Sets the color of an object.
 * setColor( 
 */

int brIRealSetColor( brEval args[], brEval *target, brInstance *i ) {
	slWorldObject *o = BRWORLDOBJECTPOINTER( &args[0] );
	slVector *color = &BRVECTOR( &args[1] );

	o->setColor( color );

	return EC_OK;
}

/*@}*/




void breveInitRealFunctions( brNamespace *n ) {
	brNewBreveCall( n, "realRemoveObject", brIRealRemoveObject, AT_NULL, AT_POINTER, 0 );

	brNewBreveCall( n, "realSetShape", brIRealSetShape, AT_NULL, AT_POINTER, AT_POINTER, 0 );
	brNewBreveCall( n, "realSetDisplayShape", brIRealSetDisplayShape, AT_NULL, AT_POINTER, AT_POINTER, 0 );
	brNewBreveCall( n, "realSetLocation", brIRealSetLocation, AT_NULL, AT_POINTER, AT_VECTOR, 0 );
	brNewBreveCall( n, "realSetRotation", brIRealSetRotation, AT_NULL, AT_POINTER, AT_VECTOR, AT_DOUBLE, 0 );
	brNewBreveCall( n, "realGetLocation", brIRealGetLocation, AT_VECTOR, AT_POINTER, 0 );
	brNewBreveCall( n, "realGetRotation", brIRealGetRotation, AT_MATRIX, AT_POINTER, 0 );
	brNewBreveCall( n, "realSetRotationMatrix", brIRealSetRotationMatrix, AT_NULL, AT_POINTER, AT_MATRIX, 0 );

	brNewBreveCall( n, "realGetLightExposure", brIRealGetLightExposure, AT_INT, AT_POINTER, 0 );

	brNewBreveCall( n, "realGetNeighbors", brIRealGetNeighbors, AT_LIST, AT_POINTER, 0 );
	brNewBreveCall( n, "realSetNeighborhoodSize", brIRealSetNeighborhoodSize, AT_NULL, AT_POINTER, AT_DOUBLE, 0 );
	brNewBreveCall( n, "realSetCollisionProperties", brIRealSetCollisionProperties, AT_NULL, AT_POINTER, AT_DOUBLE, AT_DOUBLE, 0 );

	brNewBreveCall( n, "realSetBoundingBox", brIRealSetBoundingBox, AT_NULL, AT_POINTER, AT_INT, 0 );
	brNewBreveCall( n, "realSetDrawAxis", brIRealSetDrawAxis, AT_NULL, AT_POINTER, AT_INT, 0 );
	brNewBreveCall( n, "realSetNeighborLines", brIRealSetNeighborLines, AT_NULL, AT_POINTER, AT_INT, 0 );
	brNewBreveCall( n, "realSetVisible", brIRealSetVisible, AT_NULL, AT_POINTER, AT_INT, 0 );
	brNewBreveCall( n, "realSetTextureScale", brIRealSetTextureScale, AT_NULL, AT_POINTER, AT_DOUBLE, AT_DOUBLE, 0 );

	brNewBreveCall( n, "realSetTexture", brIRealSetTexture, AT_NULL, AT_POINTER, AT_POINTER, 0 );
	brNewBreveCall( n, "realSetLightmap", brIRealSetLightmap, AT_NULL, AT_POINTER, AT_POINTER, 0 );
	brNewBreveCall( n, "realSetBitmap", brIRealSetBitmap, AT_NULL, AT_POINTER, AT_POINTER, 0 );

	brNewBreveCall( n, "realSetBitmapRotation", brIRealSetBitmapRotation, AT_NULL, AT_POINTER, AT_DOUBLE, 0 );
	brNewBreveCall( n, "realSetBitmapRotationTowardsVector", brIRealSetBitmapRotationTowardsVector, AT_NULL, AT_POINTER, AT_VECTOR, 0 );
	brNewBreveCall( n, "realSetAlpha", brIRealSetAlpha, AT_NULL, AT_POINTER, AT_DOUBLE, 0 );
	brNewBreveCall( n, "realSetColor", brIRealSetColor, AT_NULL, AT_POINTER, AT_VECTOR, 0 );
	brNewBreveCall( n, "realSetDrawShadows", brIRealSetDrawShadows, AT_NULL, AT_POINTER, AT_INT, 0 );
	brNewBreveCall( n, "realSetDrawAsPoint", brIRealSetDrawAsPoint, AT_NULL, AT_POINTER, AT_INT, 0 );
}
