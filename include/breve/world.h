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

#ifdef __cplusplus
#include <vector>
#include <algorithm> 
#include <stdexcept>

class slPatchGrid;
class slSpring;
class slVclipData;
class slDrawCommandList;
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
};

#endif

/*!
	\brief A structure holding the simulated world.
*/

#ifdef __cplusplus
class slGISData;

struct slWorld {
	// when objects are added or removed from the world, this flag must be 
	// set to 0 so that vclip structures are reinitialized.

	bool initialized;
	unsigned char odeStepMode;

	dWorldID odeWorldID;
	dJointGroupID odeCollisionGroupID;
	dJointGroupID odeJointGroupID;

	// sunlight detection

	unsigned char detectLightExposure;
	unsigned char drawLightExposure;

	slVector lightExposureSource;
	slVector lightExposureTarget;

	// integration vectors -- DV_VECTOR_COUNT depends on the requirements
	// of the integration algorithm we're using... mbEuler requires only 
	// a single derivation vector, while RK4 needs about 6.

	double *dv[DV_VECTOR_COUNT];

	int (*integrator)(slWorld *w, slLink *m, double *dt, int skip);

	// the collision callback is called when the collision is detected --
	// at the estimated time of collision.  

	void (*collisionCallback)(void *body1, void *body2, int type);

	// the collisionCheckCallback is a callback defined by the program
	// using the physics engine.  it takes two userData (see slWorldObject)
	// pointers and returns whether collision detection should be preformed
	// on the objects or not.

	int (*collisionCheckCallback)(void *body1, void *body2, int type);

	unsigned char boundingBoxOnly;

	// age is the simulation time of the world.

	double age;

	std::vector<slWorldObject*> objects;
	std::vector<slPatchGrid*> patches;
	std::vector<slCamera*> cameras;
	std::vector<slObjectConnection*> connections;
	std::vector<slDrawCommandList*> drawings;

	// we have one slVclipData for the regular collision detection
	// and one which will be used to answer "proximity" questions:
	// to allow objects to ask for all objects within a certain radius

	slVclipData *clipData;
	slVclipData *proximityData;

	slVector gravity;

	bool detectCollisions;
	bool resolveCollisions;

	// drawing the world...

	slVector backgroundColor;
	slVector backgroundTextureColor;
	slVector shadowColor;

	int backgroundTexture;
	int isBackgroundImage;

	slGISData *gisData;

#ifdef HAVE_LIBENET
	slNetsimData netsimData;
	slNetsimClientData *netsimClient;
#endif
};
#endif

#ifdef __cplusplus
extern "C"{
#endif
slWorld *slWorldNew(void);
slWorldObject *slWorldNewObject(void *, int);

int slWorldStartNetsimServer(slWorld *);
int slWorldStartNetsimSlave(slWorld *, char *);

void slWorldAddCamera(slWorld *, slCamera *);
void slWorldRemoveCamera(slWorld *, slCamera *);

slPatchGrid *slPatchGridAdd(slWorld *, slVector *, slVector *, int, int, int);

void slWorldFree(slWorld *);
void slWorldFreeObject(slWorldObject *);
void slFreeClipData(slVclipData *);
void slRenderWorldCameras(slWorld *);

slWorldObject *slWorldAddObject(slWorld *, slWorldObject *, int);
void slRemoveObject(slWorld *, slWorldObject *);

double slRunWorld(slWorld *, double, double, int *);
double slWorldStep(slWorld *, double, int *);
void slNeighborCheck(slWorld *);

slVclipData *slVclipDataNew(void);

slStationary *slNewStationary(slShape *, slVector *, double [3][3], void *);

void slWorldSetGravity(slWorld *, slVector *);

void slWorldSetBoundsOnlyCollisionDetection(slWorld *, int);

void slInitProximityData(slWorld *);

slObjectLine *slWorldAddObjectLine(slWorld *, slWorldObject *, slWorldObject *, int, slVector *);
void slWorldRemoveConnection(slWorld *, slObjectConnection *);
void slWorldAddConnection(slWorld *, slObjectConnection *);

double slWorldGetAge(slWorld *);
void slWorldSetAge(slWorld *, double);

void slWorldSetUninitialized(slWorld *);
void slWorldSetCollisionResolution(slWorld *, int);
void slWorldSetPhysicsMode(slWorld *, int);

void slWorldSetBackgroundColor(slWorld *, slVector *);
void slWorldSetBackgroundTextureColor(slWorld *, slVector *);
void slWorldSetBackgroundTexture(slWorld *, int, int);

void slWorldSetLightExposureDetection(slWorld *, int);
void slWorldSetLightExposureSource(slWorld *, slVector *);
int slWorldGetLightExposureDetection(slWorld *);

void slWorldSetCollisionCallbacks(slWorld *, int (*)(void *, void *, int), void (*)(void *, void *, int));

void slWorldSetQuickstepIterations(slWorld *, int);

slWorldObject *slWorldGetObject(slWorld *, unsigned int);

int slWorldLoadTigerFile(slWorld *, char *);
#ifdef __cplusplus
}
#endif

#endif /* _WORLD_H */
