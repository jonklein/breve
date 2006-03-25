/*****************************************************************************
 *                                                                           *
 * The breve Simulation Environment                                          *
 * Copyright (C) 2000-2005 Jonathan Klein                                    *
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

#include "simulation.h"
#include "vclip.h"
#include "vclipData.h"
#include "link.h"
#include "multibody.h"
#include "joint.h"

/*!
	\brief Initializes collision data for the world.

	This is called every time an object is added to or removed from the 
	world, which is indicated by the slWorld initialized field being set to 0.

	Sets the slWorld field initialized to 1.
*/

void slVclipDataInit( slWorld *w ) {
	unsigned int x;

	w->_initialized = 1;

	if ( w->_proximityData ) {
		slInitProximityData(w);
		slInitBoundSort(w->_proximityData);
	}
	
	w->_clipData->realloc( w->_objects.size() );

	w->_clipData->objects = w->_objects;
	w->_clipData->world = w;
	w->_clipData->collisionCount = 0;

	// for each object in the world, fill in its shape, position and 
	// min/max vectors

	w->_clipData->boundLists[ 0 ].clear();
	w->_clipData->boundLists[ 1 ].clear();
	w->_clipData->boundLists[ 2 ].clear();

	for(x = 0; x < w->_objects.size(); x++) {
		w->_objects[ x ]->updateBoundingBox();

		w->_clipData->addBoundingBoxPointers( x, &w->_objects[x]->_min, &w->_objects[x]->_max );

		if( w->_objects[ x ]->getType() == WO_LINK ) {
			slLink *link = (slLink *)w->_objects[ x ];
			link->_clipNumber = x;
		}
	}

	for (x = 0; x < w->_clipData->maxCount; x++) 
		memset(w->_clipData->pairArray[x], (BT_CHECK | BT_UNKNOWN), w->_clipData->maxCount);

	for(x = 0; x < w->_objects.size(); x++) {
	 	if ( w->_objects[x]->getType() == WO_LINK ) {
			std::vector<slJoint*>::iterator ji;

	 		slLink *link = (slLink *)w->_objects[x];
	 
			for(ji = link->_outJoints.begin(); ji != link->_outJoints.end(); ji++ ) {
				slPairFlags *flags;
				slLink *link2 = (*ji)->_child;

				flags = slVclipPairFlags(w->_clipData, link->_clipNumber, link2->_clipNumber);

				*flags &= ~BT_CHECK;
			}

	 		if ( link->getMultibody() ) link->getMultibody()->initCollisionFlags(w->_clipData);
		}
	}

	if ( w->_detectCollisions ) {
		slInitBoundSort( w->_clipData );
		w->_clipData->clip( 0.0, 0, w->_boundingBoxOnlyCollisions );
	}
}

/*!
	\brief Initializes the slPairFlags for an object pair.

	Initializes the data which stores the collision status for 
	a single pair of objects.

	Calls the neccessary callback functions to determine whether
	collisions should be detected/handled for this object pair or
	not.
*/

slPairFlags slVclipData::initPairFlags( int x, int y ) {
	void *c1, *c2;
	slPairFlags flags;
	int t1, t2;
	int sim1, sim2;
	int callback = 0;
	int simulate = 0;

	flags = slVclipPairFlagValue( this, x, y );

	flags &= ~BT_UNKNOWN;

	t1 = world->_objects[x]->getType();
	t2 = world->_objects[y]->getType();

	// collision detection is never turned on for 2 non-link objects.
	   
	if (t1 != WO_LINK && t2 != WO_LINK) {
		flags &= ~BT_CHECK;

		slVclipPairFlagValue( this, x, y ) = flags;

		return flags;
	}
	
	sim1 = 1;
	sim2 = 1;

	c1 = world->_objects[x]->getCallbackData();
	c2 = world->_objects[y]->getCallbackData();

	// see if simulation is enabled for both of these objects 

	if ( t1 == WO_LINK ) sim1 = world->_objects[x]->isSimulated();
	if ( t2 == WO_LINK ) sim2 = world->_objects[y]->isSimulated();

	// see if the user wants to simulate them and/or callback for them

	if ( c1 && c2 && world->_collisionCheckCallback ) {
		if (sim1 && sim2)
			simulate = world->_collisionCheckCallback( c1, c2, CC_SIMULATE );

		callback = world->_collisionCheckCallback( c1, c2, CC_CALLBACK );
	}

	if (simulate || callback) {
		world->_detectCollisions = 1;
		flags |= BT_CHECK;

		if (simulate)
			flags |= BT_SIMULATE;
		if (callback)
			flags |= BT_CALLBACK;
	} else
		flags &= ~BT_CHECK;

	slVclipPairFlagValue( this, x, y) = flags;

	return flags;
}

/* 
	\brief Allocate memory for a vclip data structure.

	Allocate memory for the Voronoi-clip collision detection algorithm.
*/

slVclipData::slVclipData() {
	count = 0;
	maxCount = 32;

	pairArray = new unsigned char*[ maxCount ];

	for ( unsigned int n = 0; n < maxCount; ++n)
		pairArray[n] = new unsigned char[ maxCount ];
}

slVclipData::~slVclipData() {
	if ( !maxCount ) return;

	for (unsigned int n = 0; n < maxCount; ++n) delete[] pairArray[n];

	delete[] pairArray;
}

/*!
	\brief Reallocates slVclipData to hold the specified number of objects.

	vclipData never shrinks, it only grows when neccessary.  This function
	can be used to reserve arbitrarily large collision structures to ensure
	that no runtime reallocation is required.
*/

void slVclipData::realloc( int c ) {
	count = c;

	if ( count < maxCount ) return;

	while ( count >= maxCount ) maxCount *= 2;

	pairArray = new unsigned char*[ maxCount ];

	for (unsigned int n = 0; n < maxCount; ++n) {
		pairArray[n] = new unsigned char[ maxCount ];
	}
}

/*!
	\brief Adds minima and maxima vectors to the vclipData.

	This is required as part of the initialization of the pruning stage of 
	the collision detection algorithm.  The vectors provided are expected
	to always hold up-to-date minima and maxima information when the 
	vclip algorithm is run.
*/

void slVclipData::addBoundingBoxPointers( int offset, slVector *min, slVector *max ) {
	slBoundSort minB, maxB;

	minB.type = BT_MIN;
	maxB.type = BT_MAX;

	minB.number = offset;
	maxB.number = offset;

	minB.value = &min->x;
	maxB.value = &max->x;
	boundLists[0].push_back(minB);
	boundLists[0].push_back(maxB);

	minB.value = &min->y;
	maxB.value = &max->y;
	boundLists[1].push_back(minB);
	boundLists[1].push_back(maxB);

	minB.value = &min->z;
	maxB.value = &max->z;
	boundLists[2].push_back(minB);
	boundLists[2].push_back(maxB);
}

void slInitProximityData(slWorld *w) {
	unsigned int n;

	w->_proximityData->realloc( w->_objects.size() );

	w->_proximityData->world = w;
	w->_proximityData->objects = w->_objects;

	w->_proximityData->boundLists[0].clear();
	w->_proximityData->boundLists[1].clear();
	w->_proximityData->boundLists[2].clear();

	for( n = 0; n < w->_proximityData->maxCount; n++ ) 
		memset( w->_proximityData->pairArray[n], BT_CHECK, w->_proximityData->maxCount );

	for( n = 0; n < w->_objects.size(); n++ ) {
		slWorldObject *wo = w->_objects[n];
		const slPosition &p = wo->getPosition();

		w->_proximityData->addBoundingBoxPointers( n, &wo->_neighborMin, &wo->_neighborMax );

		wo->_neighborMin.x = p.location.x - wo->_proximityRadius;
		wo->_neighborMin.y = p.location.y - wo->_proximityRadius;
		wo->_neighborMin.z = p.location.z - wo->_proximityRadius;
		wo->_neighborMax.x = p.location.x + wo->_proximityRadius;
		wo->_neighborMax.y = p.location.y + wo->_proximityRadius;
		wo->_neighborMax.z = p.location.z + wo->_proximityRadius;
	}
}

