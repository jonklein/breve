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
	int n;
	int x, y;
	slTerrain *terrain;

	w->initialized = 1;

	if(w->objectCount > 1) qsort(w->objects, w->objectCount, sizeof(slWorldObject*), slObjectSortFunc);

	if(w->proximityData) {
		slInitProximityData(w);
		slInitBoundSort(w->proximityData);
	}
	
	// allocate the right amount of space for the current set of collisions

	slVclipDataRealloc(w->clipData, w->objectCount);

	w->clipData->objects = w->objects;

	// for each object in the world, fill in it's shape, position and 
	// min/max vectors

	for(n=0;n<w->objectCount;n++) {
		switch(w->objects[n]->type) {
			case WO_LINK:
				link = w->objects[n]->data;
				link->clipNumber = n;

				w->clipData->shapePositions[n] = &link->position;
				w->clipData->shapeList[n] = link->shape;

				slLinkUpdateBoundingBox(link);
				slAddBoundingBoxForVectors(w, n, &link->min, &link->max);

				break;
			case WO_STATIONARY:
				st = w->objects[n]->data;
				w->clipData->shapePositions[n] = &st->position;
				w->clipData->shapeList[n] = st->shape;

				slShapeBounds(st->shape, &st->position, &st->min, &st->max);
				slAddBoundingBoxForVectors(w, n, &st->min, &st->max);

				break;
			case WO_TERRAIN:
				terrain = w->objects[n]->data;
				w->clipData->shapePositions[n] = NULL;
				w->clipData->shapeList[n] = NULL;

				slAddBoundingBoxForVectors(w, n, &terrain->min, &terrain->max);

				break;
			default:
				slMessage(DEBUG_ALL, "unknown object type in vclipData init\n");
				break;
		}
	}

	for(x=1;x<w->objectCount;x++) {
		for(y=0;y<x;y++) {
			slVclipDataAddPairEntry(w, x, y);
		}
	}

	for(n=0;n<w->objectCount;n++) {
	 	if(w->objects[n]->type == WO_LINK) {
	 		link = ((slLink*)w->objects[n]->data);
	 
	 		if(link->mb) slMultibodyInitCollisionFlags(link->mb, w->clipData->pairList);
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

	pe = slVclipPairEntry(w->clipData->pairList, x, y);

	o1 = w->objects[x];
	o2 = w->objects[y];

	if(w->clipData->shapeList[x]) pe->f1 = w->clipData->shapeList[x]->features[0];
	if(w->clipData->shapeList[y]) pe->f2 = w->clipData->shapeList[y]->features[0];

	t1 = w->objects[x]->type;
	t2 = w->objects[y]->type;

	c1 = w->objects[x]->userData;
	c2 = w->objects[y]->userData;

	/* find out if collision detection is neccessary between	   */
	/* each object pair.  it's never turned on for 2 non-link      */
	/* objects.  for all other object pairs, we ask the callback.  */
	   
	if(t1 != WO_LINK && t2 != WO_LINK) {
		if(pe->flags & BT_CHECK) pe->flags ^= BT_CHECK;
	} else if(w->resolveCollisions) {
		int sim1 = 0, sim2 = 0;

		/* see if simulation is enabled for both of these objects */

		if(t1 == WO_LINK) sim1 = ((slLink*)o1->data)->simulate;
		if(t2 == WO_LINK) sim2 = ((slLink*)o2->data)->simulate;

		if(sim1 || sim2) {
			w->detectCollisions = 1;
			pe->flags |= BT_CHECK;
		} else if(pe->flags & BT_CHECK) pe->flags ^= BT_CHECK;

		if(w->collisionCheckCallback && c1 && c2 && w->collisionCheckCallback(c1, c2)) {
			w->detectCollisions = 1;
			pe->flags |= BT_CHECK;
			pe->flags |= BT_CALLBACK;
		} 

	} else if(w->collisionCheckCallback && c1 && c2) {
		// no collision resolution -- but do the users want callbacks?

		if(!w->collisionCheckCallback(c1, c2)) {
			if(pe->flags & BT_CHECK) pe->flags ^= BT_CHECK;
		} else {
			w->detectCollisions = 1;
			pe->flags |= BT_CALLBACK;
			pe->flags |= BT_CHECK;
		}
	} else {
		// printf("will not check %p and %p\n", c1, c2);
	}

	pe->candidateNumber = -1;
}

/* 
	\brief Allocate memory for a vclip data structure.

	Allocate memory for the Voronoi-clip collision detection algorithm.
*/

slVclipData *slVclipDataNew() {
	int listSize, n, m;

	slVclipData *v;

	v = slMalloc(sizeof(slVclipData));

	v->count = 0;
	v->maxCount = 32;

	listSize = v->maxCount * 2;

	/* init the collision candidates */

	v->maxCandidates = 8; 
	v->candidateCount = 0;
	v->collisionCandidates = slMalloc(sizeof(slPairEntry*) * v->maxCandidates);

	/* init the collisions */

	v->maxCollisions = 8;
	v->collisionCount = 0;
	v->collisions = slMalloc(sizeof(slCollisionEntry*) * v->maxCollisions);

	for(n=0;n<v->maxCollisions;n++) {
		v->collisions[n] = slMalloc(sizeof(slCollisionEntry));
		v->collisions[n]->maxPoints = 0;
	}

	/* init the bound lists and bound list pointers */

	v->xListPointers = slMalloc(sizeof(slBoundSort*) * listSize);
	v->yListPointers = slMalloc(sizeof(slBoundSort*) * listSize);
	v->zListPointers = slMalloc(sizeof(slBoundSort*) * listSize);

	v->xList = slMalloc(sizeof(slBoundSort) * listSize);
	v->yList = slMalloc(sizeof(slBoundSort) * listSize);
	v->zList = slMalloc(sizeof(slBoundSort) * listSize);

	for(n=0;n<listSize;n++) {
		v->xListPointers[n] = &v->xList[n];
		v->yListPointers[n] = &v->yList[n];
		v->zListPointers[n] = &v->zList[n];
	}
	
	v->pairList = slMalloc(sizeof(slPairEntry*) * (v->maxCount + 1));

	/* init the pair list--we need an entry for every possible object pair */
	/* we only need a diagonal matrix here.  initialize the CHECK flag	 */
	/* for all entries, we'll remove some later.			   */ 

	for(n=1;n<v->maxCount;n++) {
		v->pairList[n] = slMalloc(sizeof(slPairEntry) * n);		

		for(m=0;m<n;m++) v->pairList[n][m].flags = BT_CHECK;
	}

	v->shapeList = slMalloc(sizeof(slShape*) * v->maxCount);
	v->shapePositions = slMalloc(sizeof(slPosition*) * v->maxCount);

	return v;
}

/*!
	\brief Reallocates slVclipData to hold the specified number of objects.

	vclipData never shrinks, it only grows when neccessary.
*/

void slVclipDataRealloc(slVclipData *v, int count) {
	int listSize, n, m;
	int oldMax;

	v->count = count;

	if(v->count < v->maxCount) return;

	oldMax = v->maxCount;

	while(v->count > v->maxCount) v->maxCount *= 2;

	listSize = v->maxCount * 2;

	// get rid of the previous candidates

	v->candidateCount = 0;

	// init the bound lists and bound list pointers 

	v->xListPointers = slRealloc(v->xListPointers, sizeof(slBoundSort*) * listSize);
	v->yListPointers = slRealloc(v->yListPointers, sizeof(slBoundSort*) * listSize);
	v->zListPointers = slRealloc(v->zListPointers, sizeof(slBoundSort*) * listSize);

	v->xList = slRealloc(v->xList, sizeof(slBoundSort) * listSize);
	v->yList = slRealloc(v->yList, sizeof(slBoundSort) * listSize);
	v->zList = slRealloc(v->zList, sizeof(slBoundSort) * listSize);

	for(n=0;n<listSize;n++) {
		v->xListPointers[n] = &v->xList[n];
		v->yListPointers[n] = &v->yList[n];
		v->zListPointers[n] = &v->zList[n];
	}

	v->pairList = slRealloc(v->pairList, sizeof(slPairEntry*) * (v->maxCount + 1));

	// init the pair list--we need an entry for every possible object pair
	// we only need a diagonal matrix here.  initialize the CHECK flag
	// for all entries, we'll remove some later.

	for(n=oldMax;n<v->maxCount;n++) {
		v->pairList[n] = slMalloc(sizeof(slPairEntry) * n);		

		for(m=0;m<n;m++) v->pairList[n][m].flags = BT_CHECK;
	}

	v->shapeList = slRealloc(v->shapeList, sizeof(slShape*) * v->maxCount);
	v->shapePositions = slRealloc(v->shapePositions, sizeof(slPosition*) * v->maxCount);
}

/*!
	\brief Adds minima and maxima vectors to the vclipData.

	This is required as part of the initialization of the pruning stage of 
	the collision detection algorithm.  The vectors provided are expected
	to always hold up-to-date minima and maxima information when the 
	vclip algorithm is run.
*/

void slAddBoundingBoxForVectors(slWorld *w, int offset, slVector *min, slVector *max) {
	w->clipData->xList[offset * 2].type = BT_MAX;	
	w->clipData->xList[offset * 2].value = &max->x;
	w->clipData->xList[offset * 2].number = offset;
	w->clipData->xList[(offset * 2) + 1].type = BT_MIN;	
	w->clipData->xList[(offset * 2) + 1].value = &min->x;
	w->clipData->xList[(offset * 2) + 1].number = offset;
	w->clipData->xList[(offset * 2) + 1].previousLeft = 0;

	w->clipData->yList[offset * 2].type = BT_MAX;	
	w->clipData->yList[offset * 2].value = &max->y;
	w->clipData->yList[offset * 2].number = offset;
	w->clipData->yList[(offset * 2) + 1].type = BT_MIN;	
	w->clipData->yList[(offset * 2) + 1].value = &min->y;
	w->clipData->yList[(offset * 2) + 1].number = offset;
	w->clipData->yList[(offset * 2) + 1].previousLeft = 0;

	w->clipData->zList[offset * 2].type = BT_MAX;	
	w->clipData->zList[offset * 2].value = &max->z;
	w->clipData->zList[offset * 2].number = offset;
	w->clipData->zList[(offset * 2) + 1].type = BT_MIN;	
	w->clipData->zList[(offset * 2) + 1].value = &min->z;
	w->clipData->zList[(offset * 2) + 1].number = offset;	
	w->clipData->zList[(offset * 2) + 1].previousLeft = 0;
}

/*
	\brief Disables collisions with the specified object.

	Removes the BT_CHECK flag for all object pairs containing this
	object.
*/

void slIgnoreAllCollisions(slVclipData *d, int object) {
	int n;
	slPairEntry *pe;

	for(n=1;n<d->count;n++) {
		pe = slVclipPairEntry(d->pairList, n, object);

		if(pe->flags & BT_CHECK) pe->flags ^= BT_CHECK;
	}   
}

/*!
	\brief Turn on/off bounds-only collision detection.
*/

void slSetBoundsOnlyCollisionDetection(slWorld *w, int b) {
	w->boundingBoxOnly = b;
}

void slInitProximityData(slWorld *w) {
	int n, x, y;
	slPairEntry *pe;

	slVclipDataRealloc(w->proximityData, w->objectCount);

	for(n=0;n<w->objectCount;n++) {
		w->proximityData->xList[(n * 2)	].type = BT_MIN;
		w->proximityData->xList[(n * 2) + 1].type = BT_MAX;
		w->proximityData->xList[(n * 2)	].number = n;
		w->proximityData->xList[(n * 2) + 1].number = n;
		w->proximityData->xList[(n * 2)	].value = &w->objects[n]->min.x;
		w->proximityData->xList[(n * 2) + 1].value = &w->objects[n]->max.x;

		w->proximityData->yList[(n * 2)	].type = BT_MIN;
		w->proximityData->yList[(n * 2) + 1].type = BT_MAX;
		w->proximityData->yList[(n * 2)	].number = n;
		w->proximityData->yList[(n * 2) + 1].number = n;
		w->proximityData->yList[(n * 2)	].value = &w->objects[n]->min.y;
		w->proximityData->yList[(n * 2) + 1].value = &w->objects[n]->max.y;

		w->proximityData->zList[(n * 2)	].type = BT_MIN;
		w->proximityData->zList[(n * 2) + 1].type = BT_MAX;
		w->proximityData->zList[(n * 2)	].number = n;
		w->proximityData->zList[(n * 2) + 1].number = n;
		w->proximityData->zList[(n * 2)	].value = &w->objects[n]->min.z;
		w->proximityData->zList[(n * 2) + 1].value = &w->objects[n]->max.z;

		switch(w->objects[n]->type) {
			slWorldObject *wo;
			slStationary *s;
			slLink *l;

			case WO_STATIONARY:
				wo = w->objects[n];
				s = w->objects[n]->data;
				wo->min.x = s->position.location.x - wo->proximityRadius;
				wo->min.y = s->position.location.y - wo->proximityRadius;
				wo->min.z = s->position.location.z - wo->proximityRadius;
				wo->max.x = s->position.location.x + wo->proximityRadius;
				wo->max.y = s->position.location.y + wo->proximityRadius;
				wo->max.z = s->position.location.z + wo->proximityRadius;
				break;
			case WO_LINK:
				wo = w->objects[n];
				l = w->objects[n]->data;
				wo->min.x = l->position.location.x - wo->proximityRadius;
				wo->min.y = l->position.location.y - wo->proximityRadius;
				wo->min.z = l->position.location.z - wo->proximityRadius;
				wo->max.x = l->position.location.x + wo->proximityRadius;
				wo->max.y = l->position.location.y + wo->proximityRadius;
				wo->max.z = l->position.location.z + wo->proximityRadius;
				break;
			case WO_TERRAIN:
				wo = w->objects[n];
				slVectorCopy(&((slTerrain*)wo->data)->max, &wo->max);
				slVectorCopy(&((slTerrain*)wo->data)->min, &wo->min);
				wo->min.x -= wo->proximityRadius;
				wo->min.y -= wo->proximityRadius;
				wo->min.z -= wo->proximityRadius;
				wo->max.x += wo->proximityRadius;
				wo->max.y += wo->proximityRadius;
				wo->max.z += wo->proximityRadius;
				break;
			default: 
				slMessage(DEBUG_ALL, "Unknown world object type in slVclipDataInit()\n");
				return;
		}
	}

	// for the proximity data we don't need anything except the	
	// x and y set in the pairList.  no features or object pointers.

	for(x=1;x<w->objectCount;x++) {
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
	int n;

	if(!v->maxCount) {
		slFree(v);
		return;
	}

	if(v->shapeList) slFree(v->shapeList);
	if(v->shapePositions) slFree(v->shapePositions);

	for(n=1;n<v->maxCount;n++) slFree(v->pairList[n]);
	if(v->pairList) slFree(v->pairList);

	for(n=0;n<v->maxCollisions;n++) {
		if(v->collisions[n]->maxPoints) {
			slFree(v->collisions[n]->pointDepths);
			slFree(v->collisions[n]->worldPoints);
		}

		slFree(v->collisions[n]);
	}

	if(v->collisions) slFree(v->collisions);

	if(v->collisionCandidates) slFree(v->collisionCandidates);

	if(v->xList) slFree(v->xList);
	if(v->yList) slFree(v->yList);
	if(v->zList) slFree(v->zList);
	if(v->xListPointers) slFree(v->xListPointers);
	if(v->yListPointers) slFree(v->yListPointers);
	if(v->zListPointers) slFree(v->zListPointers);

	slFree(v);
}
