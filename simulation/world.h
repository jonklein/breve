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

#ifdef __cplusplus
#include <vector>
#include <algorithm> 
#endif

#define DV_VECTOR_COUNT	10

enum slWorldObjectTypes {
	WO_LINK = 1,
	WO_STATIONARY,
	WO_PATCH_GRID,
	WO_TERRAIN
};

enum slDrawModes {
	DM_DRAW 			= 0x01,
	DM_BOUND 			= 0x02,
	DM_AXIS				= 0x04,
	DM_STENCIL			= 0x08,
	DM_NEIGHBOR_LINES		= 0x10,
	DM_INVISIBLE			= 0x20,
	DM_HIGHLIGHT			= 0x40,
	DM_ALL				= 0xFFFF
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

struct slWorldObject {
	/* type is one of the slWorldObjectTypes -- a stationary or a multibody */
	/* the data pointer is thus a pointer to the corresponding structure.   */

	unsigned char type;
	void *data;

	slVector color;

	int lightExposure;

	int texture;
	char textureMode;
	unsigned char drawMode;
	float billboardRotation;
	float alpha;

	float textureScale;

	// bounding box information here is used for "proximity" data

	slVector max;
	slVector min;

	double e;
	double eT;
	double mu;

	double proximityRadius;
	slStack *neighbors;

	// the list of lines that this object makes to other objects

	slList *outLines;

	// and--so that we can eliminate the connections when this
	// object is freed--we'll keep the lines other objects make
	// to this one.

	slList *inLines;

	// the userData is defined by users of the physics engine so that, 
	// for example, the engine can report which objects are colliding  
	// using the collision callback.  in the specific case of breve, 
	// the user data represents the steve instance associated with the
	// world object... 

	void *userData;
};

/*!
	\brief A stationary object in the simulated world.
*/

struct slStationary {
	slShape *shape;

	slPosition position;

	int lightExposure;

	/* min and max get pointed to by the vclip bounding box data */

	slVector min;
	slVector max;
};

/*!
	\brief A line drawn from one object to another.
*/

struct slObjectLine {
	slVector color;
	int stipple;
	slWorldObject *destination;
};

/*!
	\brief A structure holding the simulated world.
*/

#ifdef __cplusplus
struct slWorld {
	/* when objects are added or removed from the world, this flag must be */
	/* set to 0 so that vclip structures are reinitialized.				*/

	unsigned char initialized;
	unsigned char odeStepMode;

	dWorldID odeWorldID;
	dJointGroupID odeCollisionGroupID;
	dJointGroupID odeJointGroupID;

	// sunlight detection

	unsigned char detectLightExposure;
	unsigned char drawLightExposure;
	slVector lightExposureSource;

	// patches 

	// slPatchGrid *patches;

	// visions are rendered from certain perspectives in the world

	slStack *cameras;

	// integration vectors -- DV_VECTOR_COUNT depends on the requirements
	// of the integration algorithm we're using... mbEuler requires only 
	// a single derivation vector, while RK4 needs about 6.

	double *dv[DV_VECTOR_COUNT];

	int (*integrator)(slWorld *w, slLink *m, double *dt, int skip);

	// the collision callback is called when the collision is detected --
	// at the estimated time of collision.  

	int (*collisionCallback)(void *body1, void *body2, int type);

	// the collisionCheckCallback is a callback defined by the program
	// using the physics engine.  it takes two userData (see slWorldObject)
	// pointers and returns whether collision detection should be preformed
	// on the objects or not.

	int (*collisionCheckCallback)(void *body1, void *body2, int type);

	unsigned char boundingBoxOnly;

	// age is the simulation time of the world.

	double age;

	std::vector<slWorldObject*> objects;

	// these structures will replace the standard objects above in the 
	// near future -- OR WILL THEY?!

	slStack *stationaryObjects;
	slStack *mobileObjects;
	slStack *patchGridObjects;
	slStack *springObjects;

	std::vector<slSpring*> springs;

	// we have one slVclipData for the regular collision detection
	// and one which will be used to answer "proximity" questions:
	// to allow objects to ask for all objects within a certain radius

	slVclipData *clipData;
	slVclipData *proximityData;

	slVector gravity;

	unsigned char detectCollisions;
	unsigned char resolveCollisions;

	// drawing the world...

	slVector backgroundColor;
	slVector backgroundTextureColor;
	slVector shadowColor;

	int backgroundTexture;
	int isBackgroundImage;

#ifdef HAVE_LIBENET
	slNetsimData netsimData;
	slNetsimClientData *netsimClient;
#endif
};
#endif

#ifdef __cplusplus
extern "C"{
#endif
slWorld *slWorldNew();
slWorldObject *slWorldNewObject(void *d, int type);

int slWorldStartNetsimServer(slWorld *w);
int slWorldStartNetsimSlave(slWorld *w, char *host);

void slWorldAddCamera(slWorld *w, slCamera *camera);
void slWorldRemoveCamera(slWorld *w, slCamera *camera);

slPatchGrid *slAddPatchGrid(slWorld *w, slVector *center, slVector *patchSize, int x, int y, int z);

void slWorldFree(slWorld *w);
void slWorldFreeObject(slWorldObject *o);
void slFreeClipData(slVclipData *v);
void slRenderWorldCameras(slWorld *w);

slWorldObject *slWorldAddObject(slWorld *w, void *p, int type);
void slRemoveObject(slWorld *w, slWorldObject *p);

double slRunWorld(slWorld *w, double deltaT, double stepSize, int *error);
double slWorldStep(slWorld *w, double stepSize, int *error);
void slNeighborCheck(slWorld *w);

void slVclipDataInit(slWorld *w);
slVclipData *slVclipDataNew();
void slVclipDataRealloc(slVclipData *v, int count);
int slObjectSortFunc(const void *a, const void *b);

void slAddBoundingBoxForVectors(slWorld *w, int offset, slVector *max, slVector *min);

slStationary *slNewStationary(slShape *s, slVector *loc, double rot[3][3]);

void slIgnoreAllCollisions(slVclipData *d, int object);

void slWorldSetGravity(slWorld *w, slVector *gravity);

void slSetBoundsOnlyCollisionDetection(slWorld *w, int value);

void slInitProximityData(slWorld *w);

slObjectLine *slWorldAddObjectLine(slWorldObject *src, slWorldObject *dst, int stipple, slVector *color);
int slRemoveObjectLine(slWorldObject *src, slWorldObject *dst);
int slRemoveAllObjectLines(slWorldObject *src);

slObjectLine *slFindObjectLine(slWorldObject *src, slWorldObject *dst);

void slVclipDataAddPairEntry(slWorld *w, int x, int y);

double slWorldGetAge(slWorld *w);
void slWorldSetAge(slWorld *w, double time);

void slWorldSetUninitialized(slWorld *w);
void slWorldSetCollisionResolution(slWorld *w, int n);
void slWorldSetPhysicsMode(slWorld *w, int n);

void slWorldSetBackgroundColor(slWorld *w, slVector *v);
void slWorldSetBackgroundTextureColor(slWorld *w, slVector *v);
void slWorldSetBackgroundTexture(slWorld *w, int n, int mode);

void slWorldSetLightExposureDetection(slWorld *w, int n);
void slWorldSetLightExposureSource(slWorld *w, slVector *v);

void slWorldSetCollisionCallbacks(slWorld *w, int (*check)(void*, void*), int (*collide)(void*, void*, int type));

slWorldObject *slWorldGetObject(slWorld *w, int n);

#ifdef __cplusplus
}
#endif
