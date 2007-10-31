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

#ifndef _WORLD_H
#define _WORLD_H

#include "ode/ode.h"
#include "worldObject.h"
#include "netsim.h"
#include "patch.h"
#include "camera.h"
#include "skybox.h"

#ifdef __cplusplus
#include <vector>
#include <algorithm> 
#include <stdexcept>

class slPatchGrid;
class slSpring;
class slVclipData;
class slDrawCommandList;

class slLink;
class slTerrain;
class slMultibody;
#else
#endif


#define DV_VECTOR_COUNT	10

enum slWorldObjectTypes {
	WO_LINK = 1,
	WO_STATIONARY,
	WO_TERRAIN
};

enum slDrawModes {
	DM_DRAW 			= 0x01,
	DM_BOUND 			= 0x02,
	DM_AXIS				= 0x04,
	DM_STENCIL			= 0x08,
	DM_NEIGHBOR_LINES	= 0x10,
	DM_INVISIBLE		= 0x20,
	DM_HIGHLIGHT		= 0x40,
};

enum textureSettings {
	TS_NONE 			= 0,
	TS_TEXTURE,
	TS_BILLBOARD
};

enum collisionCallbackTypes {
	CC_NORMAL			= 0,
	CC_DEEP				= 1
};

/*!
	\brief An object in the simulated world.

	This object holds general color and display information, as well 
	as a pointer to the actual object type (stationary, mobile, terrain, etc).
*/

#ifdef __cplusplus

/**
 * A stationary object in the simulated world.
 */

class slStationary: public slWorldObject {
	public:
		slStationary( slShape *s, slVector *loc, double rot[3][3], void *data );
};

#endif

/**
 * A structure holding the simulated world.
 */

#ifdef __cplusplus
class slGISData;

class slWorld {
	public:
							slWorld();
							~slWorld();

		// sunlight detection
	
		inline bool 				detectLightExposure() { return _detectLightExposure; }
		inline bool 				drawLightExposure() { return _drawLightExposure; }

		void 					setDetectLightExposure( bool d ) { _detectLightExposure = d; }
		void 					setDrawLightExposure( bool d ) { _drawLightExposure = d; }

		void 					setLightExposureSource( slVector *src ) { slVectorCopy( src, &_lightExposureCamera._location); }

		slCamera 				*getLightExposureCamera() { return &_lightExposureCamera; }

		/**
		 * Indicates that collision detection structures must be reinitialized.
		 */

		void 					setUninitialized();

		/**
		 * Enables/disables collision resolution.  
		 */

		void 					setCollisionResolution( bool );
		void 					setBoundsOnlyCollisionDetection( bool );
		void 					setPhysicsMode( int );
		void 					setBackgroundColor( slVector* );
		void 					setBackgroundTextureColor( slVector* );
		void 					setBackgroundTexture( int, int );

		/**
		 * Sets the ODE error reduction parameter for the world.
		 */

		void					setERP( double inERP ) {
								if( _odeWorldID ) 
									dWorldSetERP( _odeWorldID, inERP );
							}

		/**
		 * Sets the ODE constraint force mixing parameter for the world.
		 */

		void					setCFM( double inCFM ) {
								if( _odeWorldID ) 
									dWorldSetCFM( _odeWorldID, inCFM );
							}

		// integration vectors -- DV_VECTOR_COUNT depends on the requirements
		// of the integration algorithm we're using... mbEuler requires only 
		// a single derivation vector, while RK4 needs about 6.
	
		double 					*dv[ DV_VECTOR_COUNT ];
	
		int 					(*integrator)(slWorld *w, slLink *m, double *dt, int skip);
	
		// the collision callback is called when the collision is detected --
		// at the estimated time of collision.  
	
		void 					(*_collisionCallback)(void *body1, void *body2, int type, slVector *position, slVector *face);
	
		// the collisionCheckCallback is a callback defined by the program
		// using the physics engine.  it takes two userData (see slWorldObject)
		// pointers and returns whether collision detection should be preformed
		// on the objects or not.
	
		int 					(*_collisionCheckCallback)(void *body1, void *body2, int type);
	
		slObjectLine 				*addObjectLine( slWorldObject*, slWorldObject*, int, slVector* );

		slPatchGrid 				*addPatchGrid( slVector *center, slVector *patchSize, int x, int y, int z );
		void 					removePatchGrid( slPatchGrid *g );

		// age is the simulation time of the world.
	
		double 					_age;
	
		std::vector< slWorldObject* > 		_objects;
		std::vector< slPatchGrid* > 		_patches;
		std::vector< slCamera* > 		_cameras;
		std::vector< slObjectConnection* > 	_connections;
		std::vector< slDrawCommandList* > 	_drawings;
	
		// we have one slVclipData for the regular collision detection
		// and one which will be used to answer "proximity" questions:
		// to allow objects to ask for all objects within a certain radius
	
		slPatchGrid 				*_clipGrid;
		slVclipData 				*_clipData;
		slVclipData 				*_proximityData;

		slVector 				_gravity;
	
		// drawing the world...
	
		slVector 				backgroundColor;
		slVector 				backgroundTextureColor;
		slVector 				shadowColor;

		slSkybox				_skybox;
		
		int 					backgroundTexture;
		int 					isBackgroundImage;

		slGISData 				*gisData;

		void 					updateNeighbors();

		double 					getAge();
		void 					setAge( double age );

		void 					addCamera( slCamera* );
		void 					removeCamera( slCamera* );

		void 					removeConnection( slObjectConnection* );
		void 					addConnection( slObjectConnection* );

		double 					runWorld( double, double, int* );
		double 					step( double, int* );

		void					addObject( slWorldObject* );
		void 					removeObject( slWorldObject* );
		slWorldObject 				*getObject( unsigned int );
		void 					removeEmptyObjects();

		void 					setGravity( slVector* );
		void 					setQuickstepIterations( int );
		void 					setAutoDisableFlag( bool f );

		void 					setCollisionCallbacks( int (*)(void *, void *, int), void (*)(void *, void *, int, slVector* , slVector* ) );

		void 					renderCameras();

		int 					startNetsimServer();
		int 					startNetsimSlave( char* );

		dWorldID 				_odeWorldID;
		dJointGroupID 				_odeCollisionGroupID;
		unsigned char 				_odeStepMode;

		bool 					_detectCollisions;
		bool 					_resolveCollisions;
		bool 					_boundingBoxOnlyCollisions;
	
		// when objects are added or removed from the world, this flag must be 
		// set to 0 so that vclip structures are reinitialized.
	
		bool 					_initialized;

		slCamera 				_lightExposureCamera;

#if HAVE_LIBENET
		slNetsimData 				_netsimData;
		slNetsimClient	 			*_netsimClient;
#endif

	private:
		bool 					_detectLightExposure;
		bool 					_drawLightExposure;


};
#endif

#ifdef __cplusplus
extern "C"{
#endif

void slInitProximityData(slWorld *);

slGISData *slWorldLoadTigerFile(slWorld *, char *, slTerrain *);
#ifdef __cplusplus
}
#endif

#endif /* _WORLD_H */
