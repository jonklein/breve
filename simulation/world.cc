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

slWorld::slWorld() {
	slVector g;

	gPhysicsError = 0;
	gPhysicsErrorMessage = NULL;

	slAllocIntegrationVectors( this );

	_odeWorldID = dWorldCreate();
	dWorldSetQuickStepNumIterations( _odeWorldID, 60 );

	dWorldSetCFM ( _odeWorldID, 1e-6 );
	dWorldSetERP( _odeWorldID, 0.1 );
	dSetErrorHandler(slODEErrorHandler);
	dSetMessageHandler(slODEErrorHandler);

	_odeCollisionGroupID = dJointGroupCreate(0);
	_odeJointGroupID = dJointGroupCreate(0);

	_resolveCollisions = 0;
	_detectCollisions = 0;

	_initialized = 0;

	_age = 0.0;

	_detectCollisions = 1;

	netsimData.server = NULL;

	_collisionCallback = NULL;
	_collisionCheckCallback = NULL;

	integrator = slEuler;

	_boundingBoxOnlyCollisions = 0;

	_odeStepMode = 0;

	backgroundTexture = 0;

	slVectorSet( &backgroundTextureColor, 1, 1, 1 );

	slVectorSet( &_lightExposureCamera._target, 0, 0, 0 );

	_proximityData = NULL;
	_clipGrid = NULL;
	_clipData = slVclipDataNew();

	gisData = NULL;

	slVectorSet(&g, 0.0, -9.81, 0.0);

	setGravity( &g );
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

	if( w->_clipData ) slFreeClipData( w->_clipData );
	if( w->_proximityData ) slFreeClipData( w->_proximityData );

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
		( *ci )->renderWorld( w, 0, 1 );
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

slWorldObject *slWorld::addObject( slWorldObject *no ) {
	objects.push_back(no);

	_initialized = 0;

	return no;
}

/*!
	\brief Removes an object from the world.
*/

void slWorld::removeObject( slWorldObject *p ) {
	std::vector<slWorldObject*>::iterator wi;

	wi = std::find( objects.begin(), objects.end(), p );

	if(wi != objects.end()) {
		objects.erase(wi);
		_initialized = 0;
	}
}

/**
 *  \brief Adds a patch grid to the world
 */
slPatchGrid *slPatchGridAdd(slWorld *w, slVector *center, slVector *patchSize, int x, int y, int z) {

	slPatchGrid *g = new slPatchGrid(center, patchSize, x, y, z);
    
	w->patches.push_back(g);

	return g;
}

/**
 *  \brief Removes a patch grid from the world
 */
void slPatchGridRemove(slWorld *w, slPatchGrid *g) {
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

slStationary::slStationary(slShape *s, slVector *loc, double rot[3][3], void *data) {
	_type = WO_STATIONARY;

	s->_referenceCount++;
 
	_shape = s;
	_userData = data;

	slVectorCopy( loc, &_position.location );
	slMatrixCopy( rot,  _position.rotation );
}

/*!
	\brief Runs the world for deltaT seconds, in steps of the given size.

	Makes repeated calls to \ref slWorldStep to step the world forward
	deltaT seconds.
*/

double slWorld::runWorld( double deltaT, double timestep, int *error ) {
	double total = 0.0;
#if HAVE_LIBENET
	static int lastSecond = 0;
#endif

	*error = 0;

	if ( !_initialized ) slVclipDataInit( this );

	while ( total < deltaT && !*error ) 
		total += step( timestep, error );

	_age += total;

#if HAVE_LIBENET
	if ( netsimData.server && netsimData.isMaster && (int)_age >= lastSecond) {
		lastSecond = (int)_age + 1;

		slNetsimBroadcastSyncMessage( netsimData.server, _age );
	}

	if ( netsimData.server && !netsimData.isMaster && _detectCollisions ) {
		int maxIndex;
		slVector max, min;

		maxIndex = ( _clipData->count * 2 ) - 1;

		min.x = *_clipData->boundListPointers[0][0]->value;
		min.y = *_clipData->boundListPointers[2][0]->value;
		min.z = *_clipData->boundListPointers[2][0]->value;

		max.x = *_clipData->boundListPointers[0][maxIndex]->value;
		max.y = *_clipData->boundListPointers[1][maxIndex]->value;
		max.z = *_clipData->boundListPointers[2][maxIndex]->value;

		slNetsimSendBoundsMessage( netsimClient, &min, &max );
	}
#endif

	return total;
}

/*!
	\brief Takes a single simulation step.
*/

double slWorld::step( double stepSize, int *error ) {
	unsigned simulate = 0;
	std::vector<slWorldObject*>::iterator wi;
	std::vector<slObjectConnection*>::iterator li;
	int result;

	for(wi = objects.begin(); wi != objects.end(); wi++) {
		simulate += ( *wi )->isSimulated();
		( *wi )->step( this, stepSize );
	}

	for(li = _connections.begin(); li != _connections.end(); li++ ) 
		(*li)->step(stepSize);

	if( _detectCollisions ) {
		if( !_initialized ) slVclipDataInit( this );

		if( _clipGrid ) {
			_clipGrid->assignObjectsToPatches( this );
			result = 0;
		} else {
			_clipData->pruneAndSweep();
			result = _clipData->clip( 0.0, 0, _boundingBoxOnlyCollisions );
		}

		if(result == -1) {
			slMessage(DEBUG_ALL, "warning: error in vclip\n");
			(*error)++;
			return 0;
		}
	
		unsigned int cn;	

		for(cn = 0; cn < _clipData->collisionCount; cn++ ) {
			slCollision *c = &_clipData->collisions[ cn];
			slWorldObject *w1;
			slWorldObject *w2;
			slPairFlags *flags;
			unsigned int x;

			w1 = objects[c->n1];
			w2 = objects[c->n2];

			flags = slVclipPairFlags(_clipData, c->n1, c->n2);

			if(w1 && w2 && (*flags & BT_SIMULATE)) {
				dBodyID bodyX = NULL, bodyY = NULL;
				dJointID id;
				double mu = 0;
				double e = 0;

				if( w1->getType() == WO_LINK ) {
					slLink *l = (slLink*)w1;
					bodyX = l->_odeBodyID;
				}
				
				if( w2->getType() == WO_LINK ) {
					slLink *l = (slLink*)w2;
					bodyY = l->_odeBodyID;
				}

				mu = 1.0 + ( w1->_mu + w2->_mu ) / 2.0;
				e = ( w1->_e + w2->_e ) / ( 2.0 * c->points.size() );

				for( x = 0; x < c->points.size(); x++ ) {
					dContact contact;

					memset(&contact, 0, sizeof(dContact));
					contact.surface.mode = dContactSoftERP|dContactApprox1|dContactBounce;

					// contact.surface.soft_cfm = 0.01;
					contact.surface.soft_erp = 0.05;
					contact.surface.mu = mu;
					contact.surface.mu2 = 0;
					contact.surface.bounce = e;
					contact.surface.bounce_vel = 0.05;

					if( w1->getType() == WO_LINK && w2->getType() == WO_LINK ) {
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

					id = dJointCreateContact(_odeWorldID, _odeCollisionGroupID, &contact);
					dJointAttach(id, bodyY, bodyX);
				}
			}

			if((*flags & BT_CALLBACK) && _collisionCallback && w1 && w2) {
				 _collisionCallback( w1->getCallbackData(), w2->getCallbackData(), CC_NORMAL );
			}
		}
	}

	if(simulate != 0) {
		if( _odeStepMode == 0 ) {
			dWorldStep( _odeWorldID, stepSize );
		} else {
			dWorldQuickStep( _odeWorldID, stepSize );
		}

		dJointGroupEmpty( _odeCollisionGroupID );

		if(gPhysicsError) (*error)++;
	}

	return stepSize;
}

void slWorld::updateNeighbors() {
	double dist;
	slVector *location, diff;
	slWorldObject *o1, *o2;
	std::vector<slWorldObject*>::iterator wi;

	if( !_initialized ) slVclipDataInit( this );

	if( !_proximityData ) {
		_proximityData = slVclipDataNew();
		slVclipDataRealloc( _proximityData, objects.size() );
		slInitProximityData( this );
		slInitBoundSort( _proximityData );
		_proximityData->clip(0.0, 1, 0);
	}

	// update all the bounding boxes first 

	for(wi = objects.begin(); wi != objects.end(); wi++ ) {
		slWorldObject *wo = *wi;
		location = &wo->_position.location;

		wo->_neighborMin.x = location->x - wo->_proximityRadius;
		wo->_neighborMin.y = location->y - wo->_proximityRadius;
		wo->_neighborMin.z = location->z - wo->_proximityRadius;
		wo->_neighborMax.x = location->x + wo->_proximityRadius;
		wo->_neighborMax.y = location->y + wo->_proximityRadius;
		wo->_neighborMax.z = location->z + wo->_proximityRadius;
		wo->_neighbors.clear();
	}

	// vclip, but stop after the pruning stage 

	_proximityData->pruneAndSweep();

	std::map< slPairFlags* , slCollisionCandidate >::iterator ci;

	for( ci = _proximityData->candidates.begin(); ci != _proximityData->candidates.end(); ci++ ) {
		slCollisionCandidate c = ci->second;

		o1 = objects[c._x];
		o2 = objects[c._y];

		slVectorSub( &o1->_position.location, &o2->_position.location, &diff );
		dist = slVectorLength(&diff);

		if( dist < o1->_proximityRadius ) o1->_neighbors.push_back(o2);
		if( dist < o2->_proximityRadius ) o2->_neighbors.push_back(o1);
	}
}

void slWorld::setGravity( slVector *gravity ) {
	dWorldSetGravity( _odeWorldID, gravity->x, gravity->y, gravity->z );
}

slObjectLine *slWorldAddObjectLine(slWorld *w, slWorldObject *src, slWorldObject *dst, int stipple, slVector *color) {
	slObjectLine *line;

	if(src == dst) return NULL;

	line = new slObjectLine;

	line->_stipple = stipple;
	line->_dst = dst;
	line->_src = src;
	slVectorCopy(color, &line->_color);

	w->addConnection( line );

	return line;
}

void slWorld::addConnection( slObjectConnection *c ) {
	_connections.push_back( c);
	c->_src->_connections.push_back( c);
	c->_dst->_connections.push_back( c);
}

void slWorld::removeConnection( slObjectConnection *c ) {
	std::vector<slObjectConnection*>::iterator li;

	if( !c ) return;

	if( c->_src ) {
		li = std::find( c->_src->_connections.begin(), c->_src->_connections.end(), c );
		if( li != c->_src->_connections.end() ) c->_src->_connections.erase( li );
	}

	if( c->_dst ) {
		li = std::find( c->_dst->_connections.begin(), c->_dst->_connections.end(), c );
		if( li != c->_dst->_connections.end() ) c->_dst->_connections.erase( li );
	}

	li = std::find( _connections.begin(), _connections.end(), c );

	if( li != _connections.end() ) _connections.erase( li );

	delete c;
}

/*!
	\brief Returns the age of the world.
*/

double slWorld::getAge() {
	return _age;
}

/*!
	\brief Sets the age of the world.
*/

void slWorld::setAge( double a ) {
	_age = a;
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
	w->_collisionCallback = collide;
	w->_collisionCheckCallback = check;
}

slWorldObject *slWorldGetObject(slWorld *w, unsigned int n) {
	if(n > w->objects.size()) return NULL;
	return w->objects[n];
}

void slWorld::setQuickstepIterations( int n ) {
	dWorldSetQuickStepNumIterations( _odeWorldID, n );
}



