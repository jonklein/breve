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

#include "simulation.h"

/*!
	\brief Initializes collision data for the world.

	This is called every time an object is added to or removed from the 
	world, which is indicated by the slWorld initialized field being set to 0.

	Sets the slWorld field initialized to 1.
*/

void slVclipDataInit(slWorld *w) {
	slStationary *st;
	slLink *link;
	unsigned int x, y;
	slTerrain *terrain;

	w->initialized = 1;

	if(w->proximityData) {
		slInitProximityData(w);
		slInitBoundSort(w->proximityData);
	}
	
	slVclipDataRealloc(w->clipData, w->objects.size());

	w->clipData->objects = w->objects;

	// for each object in the world, fill in it's shape, position and 
	// min/max vectors

	w->clipData->boundLists[0].clear();
	w->clipData->boundLists[1].clear();
	w->clipData->boundLists[2].clear();

	for(x=0;x<w->objects.size();x++) {
		switch(w->objects[x]->type) {
			case WO_LINK:
				link = w->objects[x];
				link->clipNumber = x;

				slLinkUpdateBoundingBox(link);
				slAddBoundingBoxForVectors(w->clipData, x, &link->min, &link->max);

				break;
			case WO_STATIONARY:
				st = w->objects[x];

				slShapeBounds(st->shape, &st->position, &st->min, &st->max);
				slAddBoundingBoxForVectors(w->clipData, x, &st->min, &st->max);

				break;
			case WO_TERRAIN:
				terrain = w->objects[x];

				slAddBoundingBoxForVectors(w->clipData, x, &terrain->min, &terrain->max);

				break;
			default:
				slMessage(DEBUG_ALL, "unknown object type in vclipData init\n");
				break;
		}
	}

	for(x=0;x<w->objects.size();x++) {
		for(y=0;y<x;y++) slVclipDataAddPairEntry(w, x, y);
	}

	for(x=0;x<w->objects.size();x++) {
	 	if(w->objects[x]->type == WO_LINK) {
	 		link = (slLink*)w->objects[x];
	 
	 		if(link->multibody) slMultibodyInitCollisionFlags(link->multibody, w->clipData->pairList);
		}
	}

	if(w->detectCollisions) {
		slInitBoundSort(w->clipData);
		slVclip(w->clipData, 0.0, 0, w->boundingBoxOnly);
	}
}

/*!
	\brief Initializes a single slPairEntry struct.

	Initializes the data which stores the collision status for 
	a single pair of objects.

	Calls the neccessary callback functions to determine whether
	collisions should be detected/handled for this object pair or
	not.
*/

void slVclipDataAddPairEntry(slWorld *w, int x, int y) {
	slWorldObject *o1, *o2;
	void *c1, *c2;
	slPairEntry *pe;
	int t1, t2;
	int sim1, sim2;
	int callback = 0;
	int simulate = 0;

	pe = slVclipPairEntry(w->clipData->pairList, x, y);

	o1 = w->objects[x];
	o2 = w->objects[y];

	if(o1->shape && o1->shape->type == ST_NORMAL) pe->f1 = o1->shape->features[0];
	else pe->f1 = NULL;

	if(o2->shape && o2->shape->type == ST_NORMAL) pe->f2 = o2->shape->features[0];
	else pe->f2 = NULL;

	t1 = w->objects[x]->type;
	t2 = w->objects[y]->type;

	c1 = w->objects[x]->userData;
	c2 = w->objects[y]->userData;

	// collision detection is never turned on for 2 non-link objects.
	   
	if(t1 != WO_LINK && t2 != WO_LINK) {
		if(pe->flags & BT_CHECK) pe->flags ^= BT_CHECK;
		return;
	}
	
	sim1 = 1;
	sim2 = 1;

	// see if simulation is enabled for both of these objects 

	if(t1 == WO_LINK) sim1 = ((slLink*)o1)->simulate;
	if(t2 == WO_LINK) sim2 = ((slLink*)o2)->simulate;

	// see if the user wants to simulate them and/or callback for them

	if(c1 && c2 && w->collisionCheckCallback) {
		simulate = w->collisionCheckCallback(c1, c2, CC_SIMULATE);
		callback = w->collisionCheckCallback(c1, c2, CC_CALLBACK);
	}

	if(((sim1 && sim2) && simulate) || callback) {
		w->detectCollisions = 1;
		pe->flags |= BT_CHECK;

		if(simulate) pe->flags |= BT_SIMULATE;
		if(callback) pe->flags |= BT_CALLBACK;
	} else if(pe->flags & BT_CHECK) {
		pe->flags ^= BT_CHECK;
	}

	pe->candidateNumber = -1;
}

/* 
	\brief Allocate memory for a vclip data structure.

	Allocate memory for the Voronoi-clip collision detection algorithm.
*/

slVclipData *slVclipDataNew() {
	unsigned int listSize, n, m;

	slVclipData *v;

	v = new slVclipData;

	v->count = 0;
	v->maxCount = 32;

	listSize = v->maxCount * 2;

	v->pairList.insert(v->pairList.end(), v->maxCount + 1, NULL);

	/* init the pair list--we need an entry for every possible object pair */
	/* we only need a diagonal matrix here.  initialize the CHECK flag	 */
	/* for all entries, we'll remove some later.			   */ 

	for(n=1;n<v->maxCount;n++) {
		v->pairList[n] = new slPairEntry[n];		

		for(m=0;m<n;m++) v->pairList[n][m].flags = BT_CHECK;
	}

	return v;
}

/*!
	\brief Reallocates slVclipData to hold the specified number of objects.

	vclipData never shrinks, it only grows when neccessary.
*/

void slVclipDataRealloc(slVclipData *v, unsigned int count) {
	unsigned int listSize, n, m;
	int oldMax;

	v->count = count;

	if(v->count < v->maxCount) return;

	oldMax = v->maxCount;

	while(v->count > v->maxCount) v->maxCount *= 2;

	listSize = v->maxCount * 2;

	v->pairList.insert(v->pairList.end(), (v->maxCount + 1) - v->pairList.size(), NULL);

	// init the pair list--we need an entry for every possible object pair
	// we only need a diagonal matrix here.  initialize the CHECK flag
	// for all entries, we'll remove some later.

	for(n=oldMax;n<v->maxCount;n++) {
		v->pairList[n] = new slPairEntry[n];

		for(m=0;m<n;m++) v->pairList[n][m].flags = BT_CHECK;
	}
}

/*!
	\brief Adds minima and maxima vectors to the vclipData.

	This is required as part of the initialization of the pruning stage of 
	the collision detection algorithm.  The vectors provided are expected
	to always hold up-to-date minima and maxima information when the 
	vclip algorithm is run.
*/

void slAddBoundingBoxForVectors(slVclipData *data, int offset, slVector *min, slVector *max) {
	slBoundSort minB, maxB;

	minB.type = BT_MIN;
	maxB.type = BT_MAX;

	minB.previousLeft = NULL;
	maxB.previousLeft = NULL;
	
	minB.number = offset;
	maxB.number = offset;

	minB.value = &min->x;
	maxB.value = &max->x;
	data->boundLists[0].push_back(minB);
	data->boundLists[0].push_back(maxB);

	minB.value = &min->y;
	maxB.value = &max->y;
	data->boundLists[1].push_back(minB);
	data->boundLists[1].push_back(maxB);

	minB.value = &min->z;
	maxB.value = &max->z;
	data->boundLists[2].push_back(minB);
	data->boundLists[2].push_back(maxB);
}

void slInitProximityData(slWorld *w) {
	unsigned int n, x, y;
	slPairEntry *pe;
	std::vector<slWorldObject*>::iterator wi;

	slVclipDataRealloc(w->proximityData, w->objects.size());

	w->proximityData->boundLists[0].clear();
	w->proximityData->boundLists[1].clear();
	w->proximityData->boundLists[2].clear();

	for(n=0;n < w->objects.size(); n++) {
		slWorldObject *wo = w->objects[n];

		slAddBoundingBoxForVectors(w->proximityData, n, &wo->neighborMin, &wo->neighborMax);

		wo->neighborMin.x = wo->position.location.x - wo->proximityRadius;
		wo->neighborMin.y = wo->position.location.y - wo->proximityRadius;
		wo->neighborMin.z = wo->position.location.z - wo->proximityRadius;
		wo->neighborMax.x = wo->position.location.x + wo->proximityRadius;
		wo->neighborMax.y = wo->position.location.y + wo->proximityRadius;
		wo->neighborMax.z = wo->position.location.z + wo->proximityRadius;
	}

	// for the proximity data we don't need anything except the	
	// x and y set in the pairList.  no features or object pointers.

	for(x=1;x<w->objects.size();x++) {
		for(y=0;y<x;y++) {
			pe = &w->proximityData->pairList[x][y];

			pe->x = x;
			pe->y = y;
			pe->candidateNumber = -1;
		}
	}
}

/*!
	\brief Frees an slVclipData structure.
*/

void slFreeClipData(slVclipData *v) {
	unsigned int n;

	if(!v->maxCount) {
		// everything is uninitialized and empty
		delete v;
		return;
	}

	for(n=1;n<v->maxCount;n++) delete[] v->pairList[n];

	delete v;
}
