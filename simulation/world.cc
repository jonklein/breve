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

int gPhysicsError;
char *gPhysicsErrorMessage;
 
#include "simulation.h"
#include "world.h"
#include "tiger.h"
#include "link.h"
#include "multibody.h"
#include "integrate.h"
#include "vclip.h"
#include "vclipData.h"
#include "gldraw.h"

void *operator new (size_t size) {
	// void *p = calloc(1, size); 
	void *p = malloc( size ); 

 	if (p == NULL) throw std::bad_alloc(); 
 
 	return p;
}

void operator delete (void *p) {
	free(p); 
}

void slODEErrorHandler(int errnum, const char *msg, va_list ap) {
	static char error[2048];

	vsnprintf(error, 2047, msg, ap);
	gPhysicsErrorMessage = (char*)error;	
}

/*!
	\brief Creates a new empty world.	
*/

slWorld *slWorldNew() {
	slWorld *w;
	slVector g;

	gPhysicsError = 0;
	gPhysicsErrorMessage = NULL;

	w = new slWorld;
	memset(w, 0, sizeof(slWorld));

	slAllocIntegrationVectors(w);

	w->_odeWorldID = dWorldCreate();
	dWorldSetQuickStepNumIterations(w->_odeWorldID, 60);

	dWorldSetCFM (w->_odeWorldID,1e-6);
	dWorldSetERP(w->_odeWorldID,0.1);
	dSetErrorHandler(slODEErrorHandler);
	dSetMessageHandler(slODEErrorHandler);

	w->_odeCollisionGroupID = dJointGroupCreate(0);
	w->_odeJointGroupID = dJointGroupCreate(0);

	w->_resolveCollisions = 0;
	w->_detectCollisions = 0;

	w->_initialized = 0;

	w->_age = 0.0;

	w->_detectCollisions = 1;

	w->collisionCallback = NULL;
	w->collisionCheckCallback = NULL;

	w->integrator = slEuler;

	w->_boundingBoxOnlyCollisions = 0;

	w->_odeStepMode = 0;

	w->backgroundTexture = 0;

	slVectorSet(&w->backgroundTextureColor, 1, 1, 1);

	slVectorSet(&w->_lightExposureCamera._target, 0, 0, 0);

	w->clipData = slVclipDataNew();

	w->gisData = NULL;

	slVectorSet(&g, 0.0, -9.81, 0.0);
	slWorldSetGravity(w, &g);

	return w;
}

slGISData *slWorldLoadTigerFile(slWorld *w, char *f, slTerrain *t) {
	w->gisData = new slGISData(f, t);

	return w->gisData;
}

/*!
	\brief Startup a netsim server.
*/

int slWorldStartNetsimServer(slWorld *w) {
#if HAVE_LIBENET
	enet_initialize();

	w->netsimData.isMaster = 1;

	w->netsimData.server = slNetsimCreateServer(w);
	slNetsimStartServer(w->netsimData.server);

	if (!w->netsimData.server)
		return -1;

	return 0;
#else 
	slMessage(DEBUG_ALL, "error: cannot start netsim server -- not compiled with enet support\n");
	return -1;
#endif
}

/*!
	\brief Startup as a netsim slave.
*/

int slWorldStartNetsimSlave(slWorld *w, char *host) {
#if HAVE_LIBENET
	enet_initialize();

	w->netsimData.isMaster = 0;

	w->netsimData.server = new slNetsimServerData(w);
	w->netsimClient = slNetsimOpenConnection(w->netsimData.server->host,
	    host, NETSIM_MASTER_PORT);
	slNetsimStartServer(w->netsimData.server);

	if (!w->netsimData.server)
		return -1;

	return 0;
#else
	slMessage(DEBUG_ALL, "error: cannot start netsim slave -- not compiled with enet support\n");

	return -1;
#endif
}

/*!  
	\brief frees an slWorld object, including all of its objects and its clipData.
*/

void slWorldFree(slWorld *w) {
	std::vector<slWorldObject*>::iterator wi;
	std::vector<slPatchGrid*>::iterator pi;

	for( wi = w->objects.begin(); wi != w->objects.end(); wi++ ) 
		slWorldFreeObject( *wi );

	for( pi = w->patches.begin(); pi != w->patches.end(); pi++ ) 
		delete *pi;

	if(w->clipData) slFreeClipData(w->clipData);
	if(w->proximityData) slFreeClipData(w->proximityData);

	dWorldDestroy(w->_odeWorldID);
	dJointGroupDestroy(w->_odeCollisionGroupID);
	dJointGroupDestroy(w->_odeJointGroupID);

#if HAS_LIBENET
	if (w->netsimData.server)
		enet_deinitialize();
#endif

	slFreeIntegrationVectors(w);

	delete w;
}

/*!
	\brief Adds a camera to the world.
*/

void slWorldAddCamera(slWorld *w, slCamera *camera) {
	w->cameras.push_back(camera);
}

/*!
	\brief Removes a camera from the world.
*/

void slWorldRemoveCamera(slWorld *w, slCamera *camera) {
	std::vector<slCamera*>::iterator ci;

	ci = std::find(w->cameras.begin(), w->cameras.end(), camera);

	w->cameras.erase(ci);
}

/*!
	\brief Renders all of the cameras in the world .
*/

void slRenderWorldCameras(slWorld *w) {
	std::vector<slCamera*>::iterator ci;

	for(ci = w->cameras.begin(); ci != w->cameras.end(); ci++)
		slRenderWorld(w, *ci, 0, 1);
}

/*!
	\brief frees an object, and decreases the reference count on its slShape 
	through freeShape
*/

void slWorldFreeObject(slWorldObject *o) {
	delete o;
}

/*!
	\brief Adds an object to the world.
*/

slWorldObject *slWorldAddObject(slWorld *w, slWorldObject *no, int type) {
	no->type = type;

	// if(type == WO_LINK) w->objects.insert(w->objects.begin(), no);
	// else w->objects.push_back(no);

	w->objects.push_back(no);

	w->_initialized = 0;

	return no;
}

/*!
	\brief Removes an object from the world.
*/

void slRemoveObject(slWorld *w, slWorldObject *p) {
	std::vector<slWorldObject*>::iterator wi;

	wi = std::find(w->objects.begin(), w->objects.end(), p);

	if(wi != w->objects.end()) {
		w->objects.erase(wi);
		w->_initialized = 0;
	}

}

/**
 *  \brief Adds a patch grid to the world
 */
slPatchGrid *slPatchGridAdd(slWorld *w, slVector *center, slVector *patchSize, int x, int y, int z)
{

	slPatchGrid *g = new slPatchGrid(center, patchSize, x, y, z);
    
	w->patches.push_back(g);

	return g;
}

/**
 *  \brief Removes a patch grid from the world
 */
void slPatchGridRemove(slWorld *w, slPatchGrid *g)
{

	std::vector<slPatchGrid*>::iterator pi;

	pi = std::find(w->patches.begin(), w->patches.end(), g);

	if(pi != w->patches.end()) {
        delete *pi;
        w->patches.erase(pi);
		w->_initialized = 0;
	}

}

/*!
	\brief Creates a new stationary object.
*/

slStationary *slNewStationary(slShape *s, slVector *loc, double rot[3][3], void *data) {
	slStationary *so;

	s->_referenceCount++;
 
	so = new slStationary;

	so->shape = s;
	so->userData = data;

	slVectorCopy(loc, &so->position.location);
	slMatrixCopy(rot, so->position.rotation);

	return so;
}

/*!
	\brief Runs the world for deltaT seconds, in steps of the given size.

	Makes repeated calls to \ref slWorldStep to step the world forward
	deltaT seconds.
*/

double slRunWorld(slWorld *w, double deltaT, double step, int *error) {
	double total = 0.0;
#if HAVE_LIBENET
	static int lastSecond = 0;
#endif

	*error = 0;

	if ( !w->_initialized ) slVclipDataInit( w );

	while ( total < deltaT && !*error ) 
		total += slWorldStep(w, step, error);

	w->_age += total;

#if HAVE_LIBENET
	if (w->netsimData.server && w->netsimData.isMaster &&
	    (int)w->_age >= lastSecond) {
		lastSecond = (int)w->_age + 1;

		slNetsimBroadcastSyncMessage( w->netsimData.server, w->_age );
	}

	if (w->netsimData.server && !w->netsimData.isMaster &&
	    w->_detectCollisions) {
		int maxIndex;
		slVector max, min;

		maxIndex = (w->clipData->count * 2) - 1;

		min.x = *w->clipData->boundListPointers[0][0]->value;
		min.y = *w->clipData->boundListPointers[2][0]->value;
		min.z = *w->clipData->boundListPointers[2][0]->value;

		max.x = *w->clipData->boundListPointers[0][maxIndex]->value;
		max.y = *w->clipData->boundListPointers[1][maxIndex]->value;
		max.z = *w->clipData->boundListPointers[2][maxIndex]->value;

		slNetsimSendBoundsMessage(w->netsimClient, &min, &max);
	}
#endif

	return total;
}

/*!
	\brief Takes a single simulation step.
*/

double slWorldStep(slWorld *w, double stepSize, int *error) {
	unsigned simulate = 0;
	std::vector<slWorldObject*>::iterator wi;
	std::vector<slObjectConnection*>::iterator li;
	int result;

	for(wi = w->objects.begin(); wi != w->objects.end(); wi++) {
		simulate += (*wi)->simulate;
		(*wi)->step(w, stepSize);
	}

	for(li = w->connections.begin(); li != w->connections.end(); li++ ) 
		(*li)->step(stepSize);

	if( w->_detectCollisions ) {
		if( !w->_initialized ) slVclipDataInit(w);

		if(w->_clipGrid) {
			w->_clipGrid->assignObjectsToPatches(w);
			result = 0;
		} else {
			w->clipData->pruneAndSweep();
			result = w->clipData->clip( 0.0, 0, w->_boundingBoxOnlyCollisions );
		}

		if(result == -1) {
			slMessage(DEBUG_ALL, "warning: error in vclip\n");
			(*error)++;
			return 0;
		}
	
		unsigned int cn;	

		for(cn = 0; cn < w->clipData->collisionCount; cn++ ) {
			slCollision *c = &w->clipData->collisions[ cn];
			slWorldObject *w1;
			slWorldObject *w2;
			slPairFlags *flags;
			unsigned int x;

			w1 = w->objects[c->n1];
			w2 = w->objects[c->n2];

			flags = slVclipPairFlags(w->clipData, c->n1, c->n2);

			if(w1 && w2 && (*flags & BT_SIMULATE)) {
				dBodyID bodyX = NULL, bodyY = NULL;
				dJointID id;
				double mu = 0;
				double e = 0;

				if(w1->type == WO_LINK) {
					slLink *l = (slLink*)w1;
					bodyX = l->_odeBodyID;
				}
				
				if(w2->type == WO_LINK) {
					slLink *l = (slLink*)w2;
					bodyY = l->_odeBodyID;
				}

				mu = 1.0 + (w1->mu + w2->mu) / 2.0;
				e = (w1->e + w2->e) / (2.0 * c->points.size());

				for(x=0;x<c->points.size();x++) {
					dContact contact;

					memset(&contact, 0, sizeof(dContact));
					contact.surface.mode = dContactSoftERP|dContactApprox1|dContactBounce;

					// contact.surface.soft_cfm = 0.01;
					contact.surface.soft_erp = 0.05;
					contact.surface.mu = mu;
					contact.surface.mu2 = 0;
					contact.surface.bounce = e;
					contact.surface.bounce_vel = 0.05;

					if(w1->type == WO_LINK && w2->type == WO_LINK) {
						slMultibody *mb1 = ((slLink*)w1)->getMultibody();
						slMultibody *mb2 = ((slLink*)w2)->getMultibody();

						if(mb1 && mb1 == mb2 && mb1->getCFM() != 0.0) {
							contact.surface.mode = dContactSoftERP|dContactApprox1|dContactBounce|dContactSoftCFM;
							contact.surface.soft_cfm = mb1->getCFM();
							contact.surface.soft_erp = mb1->getERP();
						}
					}

					if(c->depths[x] < -0.5) {
						// this is a scenerio we might want to look at... it may indicate 
						// problems with the collision detection code.
						slMessage(50, "warning: collision depth = %f for pair (%d, %d)\n", c->depths[x], c->n1, c->n2);
					}

					contact.geom.depth = -c->depths[x];
					contact.geom.g1 = NULL;
					contact.geom.g2 = NULL;

					contact.geom.normal[0] = -c->normal.x;
					contact.geom.normal[1] = -c->normal.y;
					contact.geom.normal[2] = -c->normal.z;

					slVector *v = &c->points[x];

					contact.geom.pos[0] = v->x;
					contact.geom.pos[1] = v->y;
					contact.geom.pos[2] = v->z;

					id = dJointCreateContact(w->_odeWorldID, w->_odeCollisionGroupID, &contact);
					dJointAttach(id, bodyY, bodyX);
				}
			}

			if((*flags & BT_CALLBACK) && w->collisionCallback && w1 && w2) {
				 w->collisionCallback(w1->userData, w2->userData, CC_NORMAL);
			}
		}
	}

	if(simulate != 0) {
		if( w->_odeStepMode == 0 ) {
			dWorldStep(w->_odeWorldID, stepSize);
		} else {
			dWorldQuickStep(w->_odeWorldID, stepSize);
		}

		dJointGroupEmpty(w->_odeCollisionGroupID);

		if(gPhysicsError) (*error)++;
	}

	return stepSize;
}

void slNeighborCheck(slWorld *w) {
	double dist;
	slVector *location, diff;
	slWorldObject *o1, *o2;
	std::vector<slWorldObject*>::iterator wi;

	if( !w->_initialized ) slVclipDataInit(w);

	if( !w->proximityData ) {
		w->proximityData = slVclipDataNew();
		slVclipDataRealloc(w->proximityData, w->objects.size());
		slInitProximityData(w);
		slInitBoundSort(w->proximityData);
		w->proximityData->clip(0.0, 1, 0);
	}

	// update all the bounding boxes first 

	for(wi = w->objects.begin(); wi != w->objects.end(); wi++ ) {
		slWorldObject *wo = *wi;
		location = &wo->position.location;

		wo->neighborMin.x = location->x - wo->proximityRadius;
		wo->neighborMin.y = location->y - wo->proximityRadius;
		wo->neighborMin.z = location->z - wo->proximityRadius;
		wo->neighborMax.x = location->x + wo->proximityRadius;
		wo->neighborMax.y = location->y + wo->proximityRadius;
		wo->neighborMax.z = location->z + wo->proximityRadius;
		wo->neighbors.clear();
	}

	// vclip, but stop after the pruning stage 

	w->proximityData->pruneAndSweep();

	std::map< slPairFlags* , slCollisionCandidate >::iterator ci;

	for(ci = w->proximityData->candidates.begin(); 
		ci != w->proximityData->candidates.end(); ci++) {
		slCollisionCandidate c = ci->second;

		o1 = w->objects[c._x];
		o2 = w->objects[c._y];

		slVectorSub(&o1->position.location, &o2->position.location, &diff);
		dist = slVectorLength(&diff);

		if(dist < o1->proximityRadius) o1->neighbors.push_back(o2);
		if(dist < o2->proximityRadius) o2->neighbors.push_back(o1);
	}
}

void slWorldSetGravity(slWorld *w, slVector *gravity) {
	dWorldSetGravity(w->_odeWorldID, gravity->x, gravity->y, gravity->z);
}

slObjectLine *slWorldAddObjectLine(slWorld *w, slWorldObject *src, slWorldObject *dst, int stipple, slVector *color) {
	slObjectLine *line;

	if(src == dst) return NULL;

	line = new slObjectLine;

	line->_stipple = stipple;
	line->_dst = dst;
	line->_src = src;
	slVectorCopy(color, &line->_color);

	slWorldAddConnection(w, line);

	return line;
}

void slWorldAddConnection(slWorld *w, slObjectConnection *c) {
	w->connections.push_back( c);
	c->_src->connections.push_back( c);
	c->_dst->connections.push_back( c);
}

void slWorldRemoveConnection(slWorld *w, slObjectConnection *c) {
	std::vector<slObjectConnection*>::iterator li;

	if(!c) return;

	if(c->_src) {
		li = find(c->_src->connections.begin(), c->_src->connections.end(), c);
		if(li != c->_src->connections.end()) c->_src->connections.erase(li);
	}

	if(c->_dst) {
		li = find(c->_dst->connections.begin(), c->_dst->connections.end(), c);
		if(li != c->_dst->connections.end()) c->_dst->connections.erase(li);
	}

	li = find(w->connections.begin(), w->connections.end(), c);
	if(li != w->connections.end()) w->connections.erase(li);

	delete c;
}

/*!
	\brief Returns the age of the world.
*/

double slWorldGetAge(slWorld *w) {
	return w->_age;
}

/*!
	\brief Sets the age of the world.
*/

void slWorldSetAge(slWorld *w, double a) {
	w->_age = a;
}

/*!
	\brief Sets the collision detection structures as uninitialized.
*/

void slWorldSetUninitialized(slWorld *w) {
	w->_initialized = 0;
}

/*!
	\brief Sets collision resolution on or off.
*/

void slWorldSetCollisionResolution(slWorld *w, int n) {
	w->_resolveCollisions = n;
}

/*!
    \brief Turn on/off bounds-only collision detection.
*/

void slWorldSetBoundsOnlyCollisionDetection(slWorld *w, int b) {
    w->_boundingBoxOnlyCollisions = b;
}


void slWorldSetPhysicsMode(slWorld *w, int n) {
	w->_odeStepMode = n;
}

void slWorldSetBackgroundColor(slWorld *w, slVector *v) {
	slVectorCopy(v, &w->backgroundColor);
}

void slWorldSetBackgroundTextureColor(slWorld *w, slVector *v) {
	slVectorCopy(v, &w->backgroundTextureColor);
}

void slWorldSetBackgroundTexture(slWorld *w, int n, int mode) {
	w->backgroundTexture = n;
	w->isBackgroundImage = mode;
}

void slWorldSetCollisionCallbacks(slWorld *w, int (*check)(void*, void*, int t), void (*collide)(void*, void*, int t)) {
	w->collisionCallback = collide;
	w->collisionCheckCallback = check;
}

slWorldObject *slWorldGetObject(slWorld *w, unsigned int n) {
	if(n > w->objects.size()) return NULL;
	return w->objects[n];
}

void slWorld::setQuickstepIterations( int n ) {
	dWorldSetQuickStepNumIterations( _odeWorldID, n );
}



