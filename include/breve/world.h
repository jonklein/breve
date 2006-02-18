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

/*!
	\brief A stationary object in the simulated world.
*/

class slStationary: public slWorldObject {
	public:
		slStationary( slShape *s, slVector *loc, double rot[3][3], void *data );
};

#endif

/*!
	\brief A structure holding the simulated world.
*/

#ifdef __cplusplus
class slGISData;

class slWorld {
	public:
		slWorld();

		dJointGroupID _odeJointGroupID;
	
		// sunlight detection
	
		bool _detectLightExposure;
		bool _drawLightExposure;

		bool detectLightExposure() { return _detectLightExposure; }
		bool drawLightExposure() { return _detectLightExposure; }

		void setDetectLightExposure( bool d ) { _detectLightExposure = d; }
		void setDrawLightExposure( bool d ) { _drawLightExposure = d; }

		void setLightExposureSource( slVector *src ) { slVectorCopy( src, &_lightExposureCamera._location); }

		slCamera *getLightExposureCamera( ) { return &_lightExposureCamera; }

		// integration vectors -- DV_VECTOR_COUNT depends on the requirements
		// of the integration algorithm we're using... mbEuler requires only 
		// a single derivation vector, while RK4 needs about 6.
	
		double *dv[DV_VECTOR_COUNT];
	
		int (*integrator)(slWorld *w, slLink *m, double *dt, int skip);
	
		// the collision callback is called when the collision is detected --
		// at the estimated time of collision.  
	
		void (*_collisionCallback)(void *body1, void *body2, int type);
	
		// the collisionCheckCallback is a callback defined by the program
		// using the physics engine.  it takes two userData (see slWorldObject)
		// pointers and returns whether collision detection should be preformed
		// on the objects or not.
	
		int (*_collisionCheckCallback)(void *body1, void *body2, int type);
	
		// age is the simulation time of the world.
	
		double _age;
	
		std::vector< slWorldObject* > objects;
		std::vector< slPatchGrid* > patches;
		std::vector< slCamera* > cameras;
		std::vector< slObjectConnection* > _connections;
		std::vector< slDrawCommandList* > drawings;
	
		// we have one slVclipData for the regular collision detection
		// and one which will be used to answer "proximity" questions:
		// to allow objects to ask for all objects within a certain radius
	
		slPatchGrid *_clipGrid;
		slVclipData *_clipData;
		slVclipData *_proximityData;
	
		slVector _gravity;
	
		// drawing the world...
	
		slVector backgroundColor;
		slVector backgroundTextureColor;
		slVector shadowColor;
		
		int backgroundTexture;
		int isBackgroundImage;

		slGISData *gisData;

		void updateNeighbors();

		double getAge();
		void setAge( double age );

		void removeConnection( slObjectConnection* );
		void addConnection( slObjectConnection* );

		double runWorld( double, double, int* );
		double step( double, int* );

		void setGravity( slVector* );

		slWorldObject *addObject( slWorldObject* );
		void removeObject( slWorldObject* );

		void renderShadowVolume(slCamera *c);
		void renderObjectShadowVolumes(slCamera *c);
	
		void setQuickstepIterations( int );

		dWorldID _odeWorldID;
		dJointGroupID _odeCollisionGroupID;

		bool _detectCollisions;
		bool _resolveCollisions;
		bool _boundingBoxOnlyCollisions;
	
		// when objects are added or removed from the world, this flag must be 
		// set to 0 so that vclip structures are reinitialized.
	
		bool _initialized;
		unsigned char _odeStepMode;

		slCamera _lightExposureCamera;

#if HAVE_LIBENET
		slNetsimData netsimData;
		slNetsimClientData *netsimClient;
#endif

	private:
};
#endif

#ifdef __cplusplus
extern "C"{
#endif

int slWorldStartNetsimServer(slWorld *);
int slWorldStartNetsimSlave(slWorld *, char *);

void slWorldAddCamera(slWorld *, slCamera *);
void slWorldRemoveCamera(slWorld *, slCamera *);

slPatchGrid *slPatchGridAdd(slWorld *, slVector *, slVector *, int, int, int);
void slPatchGridRemove(slWorld *w, slPatchGrid *g);

void slWorldFree(slWorld *);
void slWorldFreeObject(slWorldObject *);
void slFreeClipData(slVclipData *);
void slRenderWorldCameras(slWorld *);

slVclipData *slVclipDataNew(void);

void slWorldSetBoundsOnlyCollisionDetection(slWorld *, int);

void slInitProximityData(slWorld *);

slObjectLine *slWorldAddObjectLine(slWorld *, slWorldObject *, slWorldObject *, int, slVector *);

void slWorldSetUninitialized(slWorld *);
void slWorldSetCollisionResolution(slWorld *, int);
void slWorldSetPhysicsMode(slWorld *, int);

void slWorldSetBackgroundColor(slWorld *, slVector *);
void slWorldSetBackgroundTextureColor(slWorld *, slVector *);
void slWorldSetBackgroundTexture(slWorld *, int, int);

void slWorldSetCollisionCallbacks(slWorld *, int (*)(void *, void *, int), void (*)(void *, void *, int));

slWorldObject *slWorldGetObject(slWorld *, unsigned int);

slGISData *slWorldLoadTigerFile(slWorld *, char *, slTerrain *);
#ifdef __cplusplus
}
#endif

#endif /* _WORLD_H */
