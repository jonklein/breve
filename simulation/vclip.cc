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
	\brief Initialize the slVclipData bound lists.

	This must be called to sort the lists for the first time and properly
	initialize the lists.
*/

void slInitBoundSort(slVclipData *d) {
	int listSize = d->count * 2;
	int x, y;

	d->candidates.clear();

	for(x=1;x<d->count;x++) {
		for(y=0;y<x;y++) {
			d->pairList[x][y].candidateNumber = -1;
			d->pairList[x][y].x = x;
			d->pairList[x][y].y = y;
		}
	}

	for(x=0;x<listSize;x++) {
		d->xListPointers[x] = &d->xList[x];
		d->yListPointers[x] = &d->yList[x];
		d->zListPointers[x] = &d->zList[x];
	}

	slInitBoundSortList(d->xListPointers, listSize, d, BT_XAXIS);
	slInitBoundSortList(d->yListPointers, listSize, d, BT_YAXIS);
	slInitBoundSortList(d->zListPointers, listSize, d, BT_ZAXIS);
}

/*
	\brief Adds an object-pair as a candidate for collision.

	If the first stage pruning indicates a potential collision, a 
	collision candidate is added so that the second stage detection
	is run.
*/

void slAddCollisionCandidate(slVclipData *vc, slPairEntry *pair) {
	if(!(pair->flags & BT_CHECK)) return;

	if(pair->candidateNumber != -1) {
		if(vc->candidates[pair->candidateNumber] == pair) return;

		slDebug("vclip candidate inconsistancy -- mismatch for candidate pair %d\n", pair->candidateNumber);	
	}

	pair->candidateNumber = vc->candidates.size();
	vc->candidates.push_back(pair);
}

/*! 
	\brief Removes the collision candidate from the candidate list.
*/

void slRemoveCollisionCandidate(slVclipData *vc, slPairEntry *pair) {
	if(pair->candidateNumber < 0) {
		// slDebug("vclip candidate inconsistancy -- removing missing candidate\n");
		return;
	}

	if(vc->candidates.size() < 1) {
		slDebug("vclip candidate inconsistancy -- removing from empty candidate list\n");
		return;
	}

	vc->candidates[pair->candidateNumber] = vc->candidates[vc->candidates.size() - 1];
	vc->candidates[pair->candidateNumber]->candidateNumber = pair->candidateNumber;

	vc->candidates.pop_back();
	pair->candidateNumber = -1;
}

/*!
	\brief Run the collision-detection algorithm.

	Tolerance is currently unused.  If pruneOnly is true, then 
	detection is stopped after the bounding box stage and no
	collisions are registered, so that only "proximity" 
	information is given.  If boundingBoxOnly is true, then
	detection is stopped after the bounding box stage and 
	collisions ARE registered.
*/

int slVclip(slVclipData *d, double tolerance, int pruneOnly, int boundingBoxOnly) {
	int result;
	slCollisionEntry *ce;
	slPairEntry *pe;
	std::vector<slPairEntry*>::iterator ci;

	slIsort(d, d->xListPointers, d->count * 2, BT_XAXIS);
	slIsort(d, d->yListPointers, d->count * 2, BT_YAXIS);
	slIsort(d, d->zListPointers, d->count * 2, BT_ZAXIS);

	d->collisionCount = 0;

	if(pruneOnly) return 0;

	/* the old way to go through this was O(n^2), now we just look */
	/* at the actual collisions */

	ce = slNextCollisionEntry(d);

	if(boundingBoxOnly) {
		for(ci = d->candidates.begin(); ci != d->candidates.end(); ci++ ) {
			pe = *ci;

			if(!pe) slDebug("missing collision candidate!\n");

			if(d->objects[pe->x] && d->objects[pe->y] && pe->x != pe->y) {
				ce->n1 = pe->x;
				ce->n2 = pe->y;

				ce = slNextCollisionEntry(d);
			}
		}

		d->collisionCount--;

		return d->collisionCount;
	}

	for(ci = d->candidates.begin(); ci != d->candidates.end(); ci++ ) {
		pe = *ci;

		result = slVclipTestPair(d, pe, ce);

		if(result == CT_ERROR) return -1;
		if(result == CT_PENETRATE) ce = slNextCollisionEntry(d);
	}

	// d->collisionCount is pointing to the next empty collision 
	// in other words, 1 + the actual number of collisions

	d->collisionCount--;

	return d->collisionCount;
}

slCollisionEntry *slNextCollisionEntry(slVclipData *v) {
	int n, oldMax;

	if(v->collisionCount == v->maxCollisions) {
		oldMax = v->maxCollisions;
		v->maxCollisions *= 2;
		v->collisions = slRealloc(v->collisions, v->maxCollisions * sizeof(slCollisionEntry*));

		for(n=oldMax;n<v->maxCollisions;n++) {
			v->collisions[n] = slMalloc(sizeof(slCollisionEntry));
			v->collisions[n]->maxPoints = 0;
		}
	}

	v->collisions[v->collisionCount]->pointCount = 0;

	return v->collisions[v->collisionCount++];
}

void slNextCollisionPoint(slCollisionEntry *e) {
	e->pointCount++;

	if(e->maxPoints == 0) {
		e->maxPoints = 4;
		e->worldPoints = slMalloc(e->maxPoints * sizeof(slVector));
		e->pointDepths = slMalloc(e->maxPoints * sizeof(double));
	} else if(e->pointCount == e->maxPoints) {
		e->maxPoints *= 2;
		e->worldPoints = slRealloc(e->worldPoints, e->maxPoints * sizeof(slVector));
		e->pointDepths = slRealloc(e->pointDepths, e->maxPoints * sizeof(double));
	}
}

/*!
	\brief Insertion-sort one of the dimensions of the sort list.  

	As we swap the elements, we set flags indicating whether they are 
	overlapping other objects or not.  When we're done, potential 
	collisions will be overlapping in all 3 dimensions.

	What happens if there's a tie while we're sorting?  

	If we stop, then we might miss a case: A and B are equal, C comes along 
	and matches.  If we stop now, then we haven't noticed the potential 
	collision between A + C.  So we'll have to continue through equal 
	scores.

	This doesn't seem like such a big deal, but is a MAJOR slowdown in 
	2-dimensional simulations--when the objects don't leave a certain
	plane, we do *n^2 iterations*.

	So what we do is to remember the list element to the left of us.  If 
	we are equal to the element on the left and it is the same one that was
	there previously, then we don't have to move it to the left because
	the flags will already be set.
*/

void slIsort(slVclipData *d, slBoundSort **list, int size, char boundTypeFlag) {
	int n, current;
	slPairEntry *pe;
	slBoundSort *tempList;
	int x, y, skippedSwap;

	if(size == 0) return;

	if(isnan(*list[0]->value)) list[0]->infnan = 1;
	else list[0]->infnan = 0;

	for(n=1;n<size;n++) {
		current = n;

		x = list[n]->number; 

		if(isnan(*list[n]->value)) list[n]->infnan = 1;
		else list[n]->infnan = 0;

		/* if we are equal to the left neighbor and it was the */
		/* same one we had last time, then we don't have to	*/
		/* do any switching. */

		skippedSwap = 0;

		if(*(list[current]->value) == *(list[current-1]->value)) {
			if(list[current]->previousLeft == list[current-1]) {
				current = 0;
				skippedSwap = 1;
			}
		} 

		while(current > 0 && (*(list[current]->value) <= *(list[current-1]->value) || list[current]->infnan)) {
			/* NaN used to flip out an entire simulation because the bounding box */
			/* lists wouldn't get sorted properly and then collisions wouldn't get */
			/* detected, and so forth.  So now, we give NaNs a place in the list  */
			/* so that the rest of the list can get sorted in peace */

			y = list[current - 1]->number;

			pe = slVclipPairEntry(d->pairList, x, y);
			
			if(x != y && (pe->flags & BT_CHECK)) {
				if(list[current]->type == BT_MIN && list[current - 1]->type == BT_MAX) {

					if((pe->flags & boundTypeFlag) && *(list[current]->value) != *(list[current-1]->value)) slMessage(DEBUG_WARN, "vclip inconsistancy [flag already on]!\n");

					pe->flags |= boundTypeFlag;

					if(slVclipFlagsShouldTest(pe->flags)) slAddCollisionCandidate(d, pe);

				} else if(list[current]->type == BT_MAX && list[current - 1]->type == BT_MIN && 
						*(list[current]->value) != *(list[current - 1]->value)) {

					if(!(pe->flags & boundTypeFlag) && *(list[current]->value) != *(list[current-1]->value)) slMessage(DEBUG_WARN, "vclip inconsistancy [flag already off]!\n");

					if(slVclipFlagsShouldTest(pe->flags)) slRemoveCollisionCandidate(d, pe);

					pe->flags ^= boundTypeFlag;
				}
			}

			/* swap the two */
			
			tempList = list[current - 1];
			list[current - 1] = list[current];
			list[current] = tempList;
			
			current--;
		}

		if(!skippedSwap) {
			if(current < (size-1)) {
				if(*(list[current]->value) == *(list[current+1]->value)) {
					list[current+1]->previousLeft = list[current];
				} else {
					list[current+1]->previousLeft = NULL;   
				}
			}
		}
	}
}

/* 
	our pair flags is an array of (x^2 + x)/2 

	the larger number is the column, the smaller the row:

		x x x
		x x
		x

	+ slInitBoundSortList
	=
	= initializes a slBoundSort list by sorting it.  unlike the normal
	= bound sort, the list has not been previously sorted and the flags
	= are thus not consistant.
*/

void slInitBoundSortList(slBoundSort **list, int size, slVclipData *v, char boundTypeFlag) {
	int n, extend;
	unsigned char *pair;
	slPairEntry *pe;
	int x, y;
	int otherSide;

	int finished;
	
	slIsort(v, list, size, boundTypeFlag);

	/* zero out this entry for all pairs */

	for(n=0;n<v->count;n++) {
		for(extend=n+1;extend < v->count;extend++) {
			if(v->pairList[extend][n].flags & boundTypeFlag) {

				if(slVclipFlagsShouldTest(v->pairList[extend][n].flags)) {
					slRemoveCollisionCandidate(v, &v->pairList[extend][n]);
				}

				v->pairList[extend][n].flags ^= boundTypeFlag;
			}
		}
	}

	if(v->candidates.size() != 0) slDebug("%d candidates remaining\n", v->candidates.size());
	
	for(n=0;n<size;n++) {
		if(list[n]->type == BT_MIN) {
			extend = n+1;

			finished = 0;

			/* we move to the right until the end of the list or until */
			/* the corresponding BT_MAX has been found AND the extend  */
			/* value is equal to the BT_MAX value  */

			otherSide = -1;
		 
			while(extend < size && (otherSide == -1 || *list[otherSide]->value == *list[extend]->value)) {
				if(list[extend]->number == list[n]->number) otherSide = extend;
				else {
					x = list[n]->number;
					y = list[extend]->number;   
			
					if(x > y) pe = &v->pairList[x][y];
					else pe = &v->pairList[y][x];

					pair = &pe->flags;

					if(list[extend]->type == BT_MIN) {
						if(*pair & boundTypeFlag) {
							slMessage(DEBUG_ALL, "vclip init inconsistancy [flag is already on]\n");
						}

						*pair |= boundTypeFlag;

						if(slVclipFlagsShouldTest(*pair)) slAddCollisionCandidate(v, pe);
					} 
				}

				extend++;
			}
		}
	}
}

/*!
	\brief Transform vector v with position p, placing output in o.
*/

slVector *slPositionVertex(slPosition *p, slVector *v, slVector *o) {
	slVectorXform(p->rotation, v, o);
	slVectorAdd(&p->location, o, o);

	return o;
}

/*!
	\brief transforms p1 with the position p, placing the transformed plane in pt.
*/

slPlane *slPositionPlane(slPosition *p, slPlane *p1, slPlane *pt) {
	slVectorXform(p->rotation, &p1->normal, &pt->normal);
	slPositionVertex(p, &p1->vertex, &pt->vertex);

	return pt;
}

int slVclipTestPair(slVclipData *vc, slPairEntry *e, slCollisionEntry *ce) {
	int result = 0, iterations = 0, limit;
	int x, y;

	slPosition *p1, *p2;
	slShape *s1, *s2;
	slFeature **f1, **f2;

	x = e->x;
	y = e->y;

	if(!vc->objects[x] || !vc->objects[y]) return 0;

	/* 
		this here is dirty, because we're not supposed to expect the object
		fields to necessarily be slWorldObjects, but that's what the terrain
		checker is expecting.
	*/

	s1 = vc->shapes[x];
	s2 = vc->shapes[y];

	p1 = vc->positions[x];
	p2 = vc->positions[y];

	if(!s1 || !s2) {
		return slTerrainTestPair(vc->objects[x], vc->objects[y], s1, s2, p1, p2, x, y, ce);
	}

	f1 = &e->f1;
	f2 = &e->f2;

	limit = 2 * ((s1->features.size() * s2->features.size()) + 1);

	if(ce) {
		ce->pointCount = 0;
	}

	if(s1->type == ST_SPHERE && s2->type == ST_SPHERE) {
		result = slSphereSphereCheck(vc, x, y, ce);
	} else if(s1->type == ST_SPHERE && s2->type == ST_NORMAL) {
		result = slSphereShapeCheck(vc, f2, 0, x, y, ce);
	} else if(s1->type == ST_NORMAL && s2->type == ST_SPHERE) {
		result = slSphereShapeCheck(vc, f1, 1, y, x, ce);
	} else {
		do {
			if(iterations == limit) {
				// this is the worst-case scenario.  it indicates serious problems
				// in the logic of the vclip algorithm.

				slMessage(DEBUG_WARN, "warning: vclip feature loop detected, attempting brute force detection\n");
				return slVclipTestPairAllFeatures(vc, e, ce);
			}

			if(((*f1)->type == FT_POINT) && ((*f2)->type == FT_POINT)) {
				result = slPointPointClip(f1, p1, s1, f2, p2, s2, ce);
			} else if(((*f1)->type == FT_POINT) && ((*f2)->type == FT_EDGE)) {
				result = slEdgePointClip(f2, p2, s2, f1, p1, s1, ce);
			} else if(((*f1)->type == FT_EDGE) && ((*f2)->type == FT_POINT)) {
				result = slEdgePointClip(f1, p1, s1, f2, p2, s2, ce);
			} else if(((*f1)->type == FT_POINT) && ((*f2)->type == FT_FACE)) {
				result = slPointFaceClip(f1, p1, s1, f2, p2, s2, vc, 0, x, y, ce);
			} else if(((*f1)->type == FT_FACE) && ((*f2)->type == FT_POINT)) {
				result = slPointFaceClip(f2, p2, s2, f1, p1, s1, vc, 1, y, x, ce);
			} else if(((*f1)->type == FT_EDGE) && ((*f2)->type == FT_EDGE)) {
				result = slEdgeEdgeClip(f1, p1, s1, f2, p2, s2, ce);
			} else if(((*f1)->type == FT_EDGE) && ((*f2)->type == FT_FACE)) {
				result = slEdgeFaceClip(f1, f2, vc, 0, x, y, ce);
			} else if(((*f1)->type == FT_FACE) && ((*f2)->type == FT_EDGE)) {
				result = slEdgeFaceClip(f2, f1, vc, 1, y, x, ce);
			} else if(((*f1)->type == FT_FACE) && ((*f2)->type == FT_FACE)) {
				result = slFaceFaceClip(f1, p1, s1, f2, p2, s2, vc, x, y);
			} else {
				slMessage(DEBUG_ALL, "INTERNAL ERROR: invalid slFeature types in slTestPair\n");
				return CT_ERROR;
			}

			iterations++;
		} while(result == CT_CONTINUE);
	}

	if(result == CT_PENETRATE) {
		// e->collision = ce;
		return CT_PENETRATE;
	} 

	return CT_DISJOINT;
}

/*
	= there appear to be some cases in which the regular feature testing
	= fails and causes a loop.  this is obviously a bug, and should be 
	= fixed.  however, since the simulation must go on, as they say in
	= show business, we'll have a "brute force" method that checks
	= every feature pair.
	=
	= this is only done if a feature loop is detected.
*/

int slVclipTestPairAllFeatures(slVclipData *vc, slPairEntry *e, slCollisionEntry *ce) {
	int result;
	slPosition *p1, *p2;
	slShape *s1, *s2;
	slFeature **f1, **f2, *fp1, *fp2;
	std::vector<slFeature*>::iterator fi1, fi2;

	int x, y;
	
	x = e->x;
	y = e->y;

	if(!vc->objects[x] || !vc->objects[y]) return 0;

	s1 = vc->shapes[x];
	s2 = vc->shapes[y];

	p1 = vc->positions[x];
	p2 = vc->positions[y];

	f1 = &fp1;
	f2 = &fp2;

	for(fi1 = s1->features.begin(); fi1 != s1->features.end(); fi1++ ) {
		for(fi2 = s1->features.begin(); fi2 != s1->features.end(); fi2++ ) {
			fp1 = *fi1;
			fp2 = *fi2;

			if(((*f1)->type == FT_POINT) && ((*f2)->type == FT_POINT)) {
				result = slPointPointClip(f1, p1, s1, f2, p2, s2, ce);
			} else if(((*f1)->type == FT_POINT) && ((*f2)->type == FT_EDGE)) {
				result = slEdgePointClip(f2, p2, s2, f1, p1, s1, ce);
			} else if(((*f1)->type == FT_EDGE) && ((*f2)->type == FT_POINT)) {
				result = slEdgePointClip(f1, p1, s1, f2, p2, s2, ce);
			} else if(((*f1)->type == FT_POINT) && ((*f2)->type == FT_FACE)) {
				result = slPointFaceClip(f1, p1, s1, f2, p2, s2, vc, 0, x, y, ce);
			} else if(((*f1)->type == FT_FACE) && ((*f2)->type == FT_POINT)) {
				result = slPointFaceClip(f2, p2, s2, f1, p1, s1, vc, 1, y, x, ce);
			} else if(((*f1)->type == FT_EDGE) && ((*f2)->type == FT_EDGE)) {
				result = slEdgeEdgeClip(f1, p1, s1, f2, p2, s2, ce);
			} else if(((*f1)->type == FT_EDGE) && ((*f2)->type == FT_FACE)) {
				result = slEdgeFaceClip(f1, f2, vc, 0, x, y, ce);
			} else if(((*f1)->type == FT_FACE) && ((*f2)->type == FT_EDGE)) {
				result = slEdgeFaceClip(f2, f1, vc, 1, y, x, ce);
			} else if(((*f1)->type == FT_FACE) && ((*f2)->type == FT_FACE)) {
				/* this is not currently implemented and would only print an error, so skip it */

				// result = slFaceFaceClip(f1, p1, s1, f2, p2, s2, vc, x, y);
				result = CT_CONTINUE;
			} else {
				slMessage(DEBUG_ALL, "INTERNAL ERROR: invalid slFeature types in slTestPair\n");
				return CT_DISJOINT;
			}

			if(result != CT_CONTINUE) return result;
		}
	}

	return CT_DISJOINT;
}

int slSphereSphereCheck(slVclipData *vc, int x, int y, slCollisionEntry *ce) {
	slVector diff, tempV1;
	double totalLen, depth;

	slShape *s1 = vc->shapes[x];
	slShape *s2 = vc->shapes[y];
	slPosition *p1 = vc->positions[x];
	slPosition *p2 = vc->positions[y];

	slVectorSub(&p2->location, &p1->location, &diff);

	totalLen = slVectorLength(&diff);

	depth = totalLen - (s1->radius + s2->radius);

	if(depth > MC_TOLERANCE) {
		if(ce) ce->distance = depth;

		return CT_DISJOINT;
	}

	if(!ce) return CT_PENETRATE;
 
	slNextCollisionPoint(ce);

	ce->distance = depth;
	ce->pointDepths[0] = depth;

	ce->n1 = x;
	ce->n2 = y;

	slVectorCopy(&diff, &ce->normal);

	slVectorNormalize(&ce->normal);

	slVectorMul(&ce->normal, s1->radius, &tempV1);
	slVectorAdd(&tempV1, &p1->location, &ce->worldPoints[0]);

	slVectorMul(&ce->normal, -1, &ce->normal);
	return CT_PENETRATE;
}

int slSphereShapeCheck(slVclipData *vc, slFeature **feat, int flip, int x, int y, slCollisionEntry *ce) {
	unsigned int n = 0;
	int included, update;
	slFeature *minFeature = 0;

	slVector vertex, tempV1, spPoint, shPoint, tStart, tEnd;

	slPoint *p, *start, *end;
	slEdge *e;
	slFace *f;

	double dist, centerDist, minDist = DBL_MAX;

	slPlane transformedPlane;

	slShape *s1 = vc->shapes[x];
	slShape *s2 = vc->shapes[y];
	slPosition *p1 = vc->positions[x];
	slPosition *p2 = vc->positions[y];

	if(!*feat) *feat = s2->features[0];

	do {
		switch((*feat)->type) {
			case FT_FACE:
				f = *feat;

				included = slClipPoint(&p1->location, f->voronoi, p2, f->edgeCount, &update, NULL);

				slPositionPlane(p2, &f->plane, &transformedPlane);
				centerDist = slPlaneDistance(&transformedPlane, &p1->location);

				if(included) {
					dist = centerDist - s1->radius;

					if(dist > MC_TOLERANCE) {
						if(ce) ce->distance = dist;
						return CT_DISJOINT;
					} else {
						if(!ce) return CT_PENETRATE;

						ce->distance = dist;
						slVectorCopy(&transformedPlane.normal, &ce->normal);
						slNextCollisionPoint(ce);

						ce->pointDepths[0] = dist;

						slVectorMul(&ce->normal, -s1->radius, &tempV1);
						slVectorInvXform(p1->rotation, &tempV1, &spPoint);

						slVectorAdd(&tempV1, &p1->location, &ce->worldPoints[0]);

						/* now take the sphere location + the offset to find the collision */
						/* point in world coordinates, and then translate into shape coord */

						slVectorAdd(&tempV1, &p1->location, &tempV1);
						slVectorSub(&tempV1, &p2->location, &tempV1);
						slVectorInvXform(p2->rotation, &tempV1, &shPoint);

						if(flip) {
							ce->n1 = x;
							ce->n2 = y;
						} else {
							slVectorMul(&ce->normal, -1, &ce->normal);

							ce->n1 = y;
							ce->n2 = x;
						}

						return CT_PENETRATE;
					} 
				} 

				*feat = f->neighbors[update];

				if(centerDist < minDist) {
					minDist = centerDist;
					minFeature = f;
				}

				break;

			case FT_EDGE:
				e = *feat;

				included = slClipPoint(&p1->location, e->voronoi, p2, 4, &update, NULL);

				if(included) {
					start = e->points[0];
					end = e->points[1];

					slPositionVertex(p2, &start->vertex, &tStart);
					slPositionVertex(p2, &end->vertex, &tEnd);
		   
					dist = slPointLineDist(&tStart, &tEnd, &p1->location, &shPoint) - s1->radius;

					if(dist <= MC_TOLERANCE) {
						if(!ce) return CT_PENETRATE;

						ce->distance = dist;

						slNextCollisionPoint(ce);

						ce->pointDepths[0] = dist;

						slVectorSub(&p1->location, &shPoint, &ce->normal);
						slVectorCopy(&shPoint, &ce->worldPoints[0]);

						slVectorNormalize(&ce->normal);

						slVectorMul(&ce->normal, -s1->radius, &tempV1);
						slVectorInvXform(p1->rotation, &tempV1, &spPoint);
						slVectorInvXform(p2->rotation, &tempV1, &shPoint);

						if(flip) {
							ce->n1 = x;
							ce->n2 = y;
						} else {
							slVectorMul(&ce->normal, -1, &ce->normal);

							ce->n1 = y;
							ce->n2 = x;
						}

						return CT_PENETRATE;
					}

					if(ce) ce->distance = dist;
					return CT_DISJOINT;
				}

				*feat = e->neighbors[update];

				break;

			case FT_POINT:
				p = *feat;

				included = slClipPoint(&p1->location, p->voronoi, p2, p->edgeCount, &update, NULL);

				if(included) {
					slPositionVertex(p2, &p->vertex, &vertex);
					slVectorSub(&p1->location, &vertex, &vertex);

					dist = slVectorLength(&vertex) - s1->radius;

					if(dist <= MC_TOLERANCE) {
						if(!ce) return CT_PENETRATE;

						ce->distance = dist;

						slNextCollisionPoint(ce);

						ce->pointDepths[0] = dist;

						slVectorCopy(&vertex, &ce->normal);
						slVectorNormalize(&ce->normal);

						slVectorCopy(&vertex, &ce->worldPoints[0]);						

						if(flip) {
							ce->n1 = x;
							ce->n2 = y;
						} else {
							slVectorMul(&ce->normal, -1, &ce->normal);

							ce->n1 = y;
							ce->n2 = x;
						}

						return CT_PENETRATE;
					}

					if(ce) ce->distance = dist;

					return CT_DISJOINT;
				}

				*feat = p->neighbors[update];

				break;
		}

		n++;
	} while(n < s2->features.size());

	// oh crap!  we're inside the sphere.  that's not a metaphor  
	// or anything like "in the zone", or something--i mean, we're
	// inside the sphere, or the sphere is inside us.

	f = minFeature;

	slPositionPlane(p2, &f->plane, &transformedPlane);

	if(!ce) return CT_PENETRATE;

	slNextCollisionPoint(ce);
	slVectorCopy(&transformedPlane.normal, &ce->normal);
	slVectorNormalize(&ce->normal);

	/* give an arbitrary VERY DEEP collision */
	
	if(flip) {
		ce->n1 = x;
		ce->n2 = y;
	} else {
		ce->n1 = y;
		ce->n2 = x;
	}
		

	ce->distance = -10; 
	ce->pointDepths[0] = -10;

	return CT_PENETRATE;
}

int slPointPointClip(slFeature **nf1, slPosition *p1p, slShape *s1, slFeature **nf2, slPosition *p2p, slShape *s2, slCollisionEntry *ce) {
	int update;
	slVector v1, v2, dist;
   
	slPoint *p1 = (*nf1);
	slPoint *p2 = (*nf2);

	slPositionVertex(p1p, &p1->vertex, &v1);

	if(!slClipPoint(&v1, p2->voronoi, p2p, p2->edgeCount, &update, NULL)) {
		*nf2 = p2->neighbors[update];
		return CT_CONTINUE;
	}

	slPositionVertex(p2p, &p2->vertex, &v2);

	if(!slClipPoint(&v2, p1->voronoi, p1p, p1->edgeCount, &update, NULL)) {
		*nf1 = p1->neighbors[update];
		return CT_CONTINUE;
	} 

	if(ce) {
		slVectorSub(&v1, &v2, &dist);
		ce->distance = slVectorLength(&dist);
	}
	
	return CT_DISJOINT;
}

int slEdgeFaceClip(slFeature **nf1, slFeature **nf2, slVclipData *v, int pairFlip, int x, int y, slCollisionEntry *ce) {
	int update1, update2, startEdge, total, included, n;
	double l1, l2, eD, sD;
	slVector edgeVector, tempV1, eLambda, sLambda, transEdgeVector, eStart;
	slPoint *start, *end;
	slVector tStart, tPoint, tEnd, linePoint;
	
	slPlane transformedPlane;
	slFace *f;
	slEdge *e, *newEdge;

	slPosition *ep = v->positions[x];
	slPosition *fp = v->positions[y];
	slShape *s1 = v->shapes[x];
	slShape *s2 = v->shapes[y];

	double maxIncDist;
	slFeature *maxIncFeat;
	double maxDist;
	slFeature *maxFeat;
   
	f = *nf2;
	e = *nf1;

	start = e->points[0];
	end = e->points[1];

	slVectorSub(&start->vertex, &end->vertex, &edgeVector);
	slVectorXform(ep->rotation, &edgeVector, &transEdgeVector);
	
	included = slClipEdge(e, ep, f->voronoi, fp, f->edgeCount, &update1, &update2, &l1, &l2);

	slPositionPlane(fp, &f->plane, &transformedPlane);

	slVectorMul(&edgeVector, l1, &sLambda);
	slVectorAdd(&end->vertex, &sLambda, &tempV1);
	slPositionVertex(ep, &tempV1, &sLambda);
	sD = slPlaneDistance(&transformedPlane, &sLambda);	
	
	slVectorMul(&edgeVector, l2, &eLambda);
	slVectorAdd(&end->vertex, &eLambda, &tempV1);
	slPositionVertex(ep, &tempV1, &eLambda);
	eD = slPlaneDistance(&transformedPlane, &eLambda);

	/* if these points are on opposite sides of the plane, then we're definitely penetrating */

	if(included && (eD <= MC_TOLERANCE || sD <= MC_TOLERANCE) && (sD > MC_TOLERANCE || eD > MC_TOLERANCE)) {
		slVector theVertex;
		double dist, maxDist = -DBL_MAX;
		std::vector<slFace*>::iterator fi;

		if(!ce) return CT_PENETRATE;

		slVectorCopy(&transformedPlane.normal, &ce->normal);

		if(sD >= MC_TOLERANCE) {
			*nf1 = e->neighbors[1];
			slVectorCopy(&eLambda, &theVertex);
		} else {
			*nf1 = e->neighbors[0];
			slVectorCopy(&sLambda, &theVertex);
		}

		// find the shallowest penetration plane for this point 

		for(fi = s2->faces.begin(); fi != s2->faces.end(); fi++ ) {
			slFace *face = *fi;
			slPlane p;

			slPositionPlane(fp, &face->plane, &p);

			dist = slPlaneDistance(&p, &theVertex);

			if(dist > maxDist) {
				maxDist = dist;
				*nf2 = *fi;
			}
		}

		if(!pairFlip) {
			slCountFaceCollisionPoints(ce, *nf1, *nf2, ep, fp, s1, s2);
			ce->n1 = x;
			ce->n2 = y;
		} else {
			slCountFaceCollisionPoints(ce, *nf2, *nf1, fp, ep, s2, s1);
			ce->n1 = y;
			ce->n2 = x;
		}

		return CT_PENETRATE;
	} 

	if(!included) {
		/* find closest edge or vertex */

		if(update1 != -1) startEdge = update1;
		else if(update2 != -1) startEdge = update2;
		else startEdge = 0;

		maxDist = -DBL_MAX;

		maxIncDist = DBL_MAX;
		maxIncFeat = NULL;

		n = startEdge;
		total = 0;

		newEdge = NULL;

		/* the transformed beginning of the edge... */
		
		slPositionVertex(ep, &((slPoint*)e->neighbors[0])->vertex, &eStart);

		slPositionVertex(ep, &start->vertex, &tStart);
		slPositionVertex(ep, &end->vertex, &tEnd);

		included = 0;

		maxFeat = f->neighbors[0];

		while(total++ < f->edgeCount + 1) {
			newEdge = f->neighbors[n];
			start = newEdge->points[0];

			slPositionVertex(fp, &start->vertex, &tPoint);
			included = slClipPoint(&tPoint, e->voronoi, ep, 2, &update1, &eD);

			if(included) {
				eD = slPointLineDist(&tStart, &tEnd, &tPoint, &linePoint);

				if(eD < maxIncDist) {
					maxIncDist = eD;
					maxIncFeat = start;
				}
			} else {
				if(eD > maxDist) {
					maxDist = eD;
					maxFeat = start;
				}
			}

			n++;

			if(n == f->edgeCount) n = 0;
		} 

		if(maxIncFeat) *nf2 = maxIncFeat;
		else *nf2 = maxFeat;

		return CT_CONTINUE;

		/* THIS SHOULD BE UNREACHABLE.  I HOPE. */
	}

	if(fabs(eD) < fabs(sD)) {
		if(update2 != -1) *nf2 = f->neighbors[update2];
		else *nf1 = e->neighbors[1];
	} else {
		if(update1 != -1) *nf2 = f->neighbors[update1];
		else *nf1 = e->neighbors[0];
	}

	return CT_CONTINUE;
}

/* check the return of this function for <> 0--the value is meaningless */

double slSignEdgePlaneDistanceDeriv(slPlane *p, slVector *edgeVector, slVector *lambda) {
	double d;

	d = slPlaneDistance(p, lambda);

	return d * slVectorDot(edgeVector, &p->normal);
}

int slEdgePointClip(slFeature **nf1, slPosition *ep, slShape *s1, slFeature **nf2, slPosition *pp, slShape *s2, slCollisionEntry *ce) {
	int update1, update2;
	double l1, l2; /* edge clip lambdas */
	slVector edgeVector, eLambda, transEdgeVector, tempV1;
	slPoint *start, *end;
	int included;

	slVector tStart, tEnd, tPoint, linePoint;

	slEdge *e = (*nf1);
	slPoint *p = (*nf2);

	slPositionVertex(pp, &p->vertex, &tPoint);
	
	if(!slClipPoint(&tPoint, e->voronoi, ep, 4, &update1, NULL)) {
		*nf1 = e->neighbors[update1];
		return CT_CONTINUE;
	}
	
	included = slClipEdge(e, ep, p->voronoi, pp, p->edgeCount, &update1, &update2, &l1, &l2);

	start = e->points[0];
	end = e->points[1];

	if(!included) {
		slVectorSub(&start->vertex, &end->vertex, &edgeVector);
		slVectorXform(ep->rotation, &edgeVector, &transEdgeVector);

		if(update1 != -1 && update1 == update2) {
			*nf2 = p->neighbors[update2];
			return CT_CONTINUE;
		}

		if(update1 != -1) {
			slVectorMul(&edgeVector, l1, &eLambda);
			slVectorAdd(&eLambda, &end->vertex, &tempV1);
			slPositionVertex(ep, &tempV1, &eLambda);

			slPositionVertex(pp, &p->vertex, &tempV1);
			slVectorSub(&eLambda, &tempV1, &eLambda);

			if(slVectorDot(&transEdgeVector, &eLambda) > 0) {
				*nf2 = p->neighbors[update1];		   
				return CT_CONTINUE;
			}
		} 
		
		if(update2 != -1) {
			slVectorMul(&edgeVector, l2, &eLambda);
			slVectorAdd(&eLambda, &end->vertex, &tempV1);
			slPositionVertex(ep, &tempV1, &eLambda);

			slPositionVertex(pp, &p->vertex, &tempV1);
			slVectorSub(&eLambda, &tempV1, &eLambda);

			if(slVectorDot(&transEdgeVector, &eLambda) < 0) {
				*nf2 = p->neighbors[update2];
				return CT_CONTINUE;
			}		
		}
	} 

	if(ce) {
		slPositionVertex(ep, &start->vertex, &tStart);
		slPositionVertex(ep, &end->vertex, &tEnd);
		ce->distance = slPointLineDist(&tStart, &tEnd, &tPoint, &linePoint);
	}
	
	return CT_DISJOINT;
}

int slEdgeEdgeClip(slFeature **nf1, slPosition *p1, slShape *s1, slFeature **nf2, slPosition *p2, slShape *s2, slCollisionEntry *ce) {
	int update1, update2;
	double l1, l2; /* edge lambdas */
	slPoint *start, *end;
	int included;
	
	slVector edgeVector, transEdgeVector, eLambda, transPoint, tempV1;
	slFeature *neighbor;

	slFace *neighborFace;

	slPlane transformedPlane;

	slEdge *e1, *e2;

	e1 = *nf1;
	e2 = *nf2;

	/* we clip the voronoi planes two at a time... first the vertex planes, then the slFace planes */
	
	included = slClipEdge(e2, p2, &e1->voronoi[0], p1, 2, &update1, &update2, &l1, &l2);

	if(!included || update1 != -1 || update2 != -1) {
		if(update1 != -1 && update1 == update2) {
			*nf1 = e1->neighbors[update1];
			return CT_CONTINUE;
		}
		
		start = e2->points[0];
		end = e2->points[1];
		
		slVectorSub(&start->vertex, &end->vertex, &edgeVector);
		slVectorXform(p2->rotation, &edgeVector, &transEdgeVector);

		if(update2 != -1) {
			neighbor = e1->neighbors[update2];

			slVectorMul(&edgeVector, l2, &eLambda);
			slVectorAdd(&end->vertex, &eLambda, &tempV1);
			slPositionVertex(p2, &tempV1, &eLambda);

			slPositionVertex(p1, &((slPoint*)neighbor)->vertex, &transPoint);
			slVectorSub(&eLambda, &transPoint, &eLambda);

			if(slVectorDot(&transEdgeVector, &eLambda) > 0) {
				*nf1 = neighbor;
				return CT_CONTINUE;
			}
		} 

		/* check the < > here */
		
		if(update1 != -1) {
			neighbor = e1->neighbors[update1];

			slVectorMul(&edgeVector, l1, &eLambda);
			slVectorAdd(&end->vertex, &eLambda, &tempV1);
			slPositionVertex(p2, &tempV1, &eLambda);

			slPositionVertex(p1, &((slPoint*)neighbor)->vertex, &transPoint);
			slVectorSub(&eLambda, &transPoint, &eLambda);

			if(slVectorDot(&transEdgeVector, &eLambda) < 0) {
				*nf1 = neighbor;
				return CT_CONTINUE;
			}		
		}
	}

	included = slClipEdge(e2, p2, &e1->voronoi[2], p1, 2, &update1, &update2, &l1, &l2);

	if(!included || update1 != -1 || update2 != -1) {
		if(update1 != -1 && update1 == update2) {
			*nf1 = e1->neighbors[update1 + 2];
			return CT_CONTINUE;
		}
		
		start = e2->points[0];
		end = e2->points[1];

		slVectorSub(&start->vertex, &end->vertex, &edgeVector);
		slVectorXform(p2->rotation, &edgeVector, &transEdgeVector);

		if(update2 != -1) {
			neighborFace = ((slFace*)e1->neighbors[update2 + 2]);
		
			slVectorMul(&edgeVector, l2, &eLambda);
			slVectorAdd(&end->vertex, &eLambda, &tempV1);
			slPositionVertex(p2, &tempV1, &eLambda);

			slPositionPlane(p1, &neighborFace->plane, &transformedPlane);

			if(slSignEdgePlaneDistanceDeriv(&transformedPlane, &transEdgeVector, &eLambda) > 0) {
				*nf1 = e1->neighbors[update2 + 2];
				return CT_CONTINUE;
			}
		}		

		if(update1 != -1) {
			neighborFace = ((slFace*)e1->neighbors[update1 + 2]);
		
			slVectorMul(&edgeVector, l1, &eLambda);
			slVectorAdd(&end->vertex, &eLambda, &tempV1);
			slPositionVertex(p2, &tempV1, &eLambda);

			slPositionPlane(p1, &neighborFace->plane, &transformedPlane);


			if(slSignEdgePlaneDistanceDeriv(&transformedPlane, &transEdgeVector, &eLambda) < 0) {
				*nf1 = e1->neighbors[update1 + 2];
				return CT_CONTINUE;
			}
		}		
	}
	
	included = slClipEdge(e1, p1, &e2->voronoi[0], p2, 2, &update1, &update2, &l1, &l2);

	if(!included || update1 != -1 || update2 != -1) {
		if(update1 != -1 && update1 == update2) {
			*nf2 = e2->neighbors[update1];
			return CT_CONTINUE;
		}
 
		start = e1->points[0];
		end = e1->points[1];

		slVectorSub(&start->vertex, &end->vertex, &edgeVector);
		slVectorXform(p1->rotation, &edgeVector, &transEdgeVector);
		
		if(update2 != -1) {
			neighbor = e2->neighbors[update2];

			slVectorMul(&edgeVector, l2, &eLambda);
			slVectorAdd(&end->vertex, &eLambda, &tempV1);
			slPositionVertex(p1, &tempV1, &eLambda);

			slPositionVertex(p2, &((slPoint*)neighbor)->vertex, &transPoint);
			slVectorSub(&eLambda, &transPoint, &eLambda);

			if(slVectorDot(&transEdgeVector, &eLambda) > 0) {
				*nf2 = neighbor;
				return CT_CONTINUE;
			}
		} 
		
		if(update1 != -1) {
			neighbor = e2->neighbors[update1];

			slVectorMul(&edgeVector, l1, &eLambda);
			slVectorAdd(&end->vertex, &eLambda, &tempV1);
			slPositionVertex(p1, &tempV1, &eLambda);

			slPositionVertex(p2, &((slPoint*)neighbor)->vertex, &transPoint);
			slVectorSub(&eLambda, &transPoint, &eLambda);

			if(slVectorDot(&transEdgeVector, &eLambda) < 0) {
				*nf2 = neighbor;
				return CT_CONTINUE;
			}		
		}   
	}

	included = slClipEdge(e1, p1, &e2->voronoi[2], p2, 2, &update1, &update2, &l1, &l2);

	if(!included || update1 != -1 || update2 != -1) {
		if(update1 != -1 && update1 == update2) {
			*nf2 = e2->neighbors[update1 + 2];
			return CT_CONTINUE;
		}
 
		start = e1->points[0];
		end = e1->points[1];
		
		slVectorSub(&start->vertex, &end->vertex, &edgeVector);
		slVectorXform(p1->rotation, &edgeVector, &transEdgeVector);

		if(update2 != -1) {
			neighborFace = ((slFace*)e2->neighbors[update2 + 2]);
		
			slVectorMul(&edgeVector, l2, &eLambda);
			slVectorAdd(&end->vertex, &eLambda, &tempV1);
			slPositionVertex(p1, &tempV1, &eLambda);

			slPositionPlane(p2, &neighborFace->plane, &transformedPlane);

			if(slSignEdgePlaneDistanceDeriv(&transformedPlane, &transEdgeVector, &eLambda) > 0) {
				*nf2 = e2->neighbors[update2 + 2];
				return CT_CONTINUE;
			}
		}		

		if(update1 != -1) {
			neighborFace = ((slFace*)e2->neighbors[update1 + 2]);
		
			slVectorMul(&edgeVector, l1, &eLambda);
			slVectorAdd(&end->vertex, &eLambda, &tempV1);
			slPositionVertex(p1, &tempV1, &eLambda);

			slPositionPlane(p2, &neighborFace->plane, &transformedPlane);

			if(slSignEdgePlaneDistanceDeriv(&transformedPlane, &transEdgeVector, &eLambda) < 0) {
				*nf2 = e2->neighbors[update1 + 2];
				return CT_CONTINUE;
			}
		}		
	}

	return CT_DISJOINT;	
}

/* slPointFaceClip takes the additional argument of the slShape that the slFace */
/* belongs to because handleLocalMinima needs to traverse through all the */
/* slFaces in the entire slShape											  */

int slPointFaceClip(slFeature **nf1, slPosition *pp, slShape *ps, slFeature **nf2, slPosition *fp, slShape *fs, slVclipData *v, int pairFlip, int x, int y, slCollisionEntry *ce) {
	int update;
	int n, maxFeature = 0;
	slVector tPoint, tEnd;
	slPoint *endPoint;
	double sD, asD, eD, max;
	slFace *newFace;
	slPlane transformedPlane;

	slFace *f = *nf2;
	slPoint *p = *nf1;

	std::vector<slFace*>::iterator fi;
	
	slPositionVertex(pp, &p->vertex, &tPoint);

	if(!slClipPointMax(&tPoint, f->voronoi, fp, f->edgeCount, &update)) {
		*nf2 = f->neighbors[update];
		return CT_CONTINUE;
	}

	slPositionPlane(fp, &f->plane, &transformedPlane);
	
	sD = slPlaneDistance(&transformedPlane, &tPoint);
	asD = fabs(sD);

	// go through each of this points edges, checking to see if the other 
	// end of any of the edges is closer to the plane... 

	for(n=0;n<p->edgeCount;n++) {
		endPoint = p->neighbors[n]->neighbors[0];

		/* we may have gotten the end we already have */

		if(endPoint == p) endPoint = p->neighbors[n]->neighbors[1];
		
		slPositionVertex(pp, &endPoint->vertex, &tEnd);

		eD = slPlaneDistance(&transformedPlane, &tEnd);

		/* we'll continue with this edge if:
			- the other end of the edge is closer to the plane than the current vertex
			- OR if the edge appears to intersect the plane
		*/

        if(fabs(eD) < asD || ((eD <= MC_TOLERANCE || sD <= MC_TOLERANCE) && (eD > MC_TOLERANCE || sD > MC_TOLERANCE))) {
			*nf1 = p->neighbors[n];
			return CT_CONTINUE;
		}
	}

	if(sD > MC_TOLERANCE) {
		if(ce) ce->distance = sD;
		return CT_DISJOINT;
	}

	// handle local minima code 

	/* check sidedness of the point against each slFace in the slShape... */
	/* if all values are negative then there is penetration		   */

	/* i think the original code i had is backwards--we want to be	*/
	/* finding the minimum above zero, not the max					*/

	// for the love of god, check this code!!! 

	max = -DBL_MAX;

	for(fi = fs->faces.begin(); fi != fs->faces.end(); fi++ ) {
		newFace = *fi;

		slPositionPlane(fp, &newFace->plane, &transformedPlane); 

		sD = slPlaneDistance(&transformedPlane, &tPoint);

		if(sD > max) {
			maxFeature = n;
			max = sD;
		} 
	}

	/* we'll end up here if there's nothing above zero */

	if(max <= MC_TOLERANCE) { 
		if(!ce) return CT_PENETRATE;

		*nf2 = fs->features[maxFeature];

		slVectorXform(fp->rotation, &f->plane.normal, &ce->normal);

		if(!pairFlip) {
			ce->n1 = x;
			ce->n2 = y;

			slCountFaceCollisionPoints(ce, *nf1, *nf2, pp, fp, ps, fs);
		} else {
			ce->n1 = y;
			ce->n2 = x;

			slCountFaceCollisionPoints(ce, *nf2, *nf1, fp, pp, fs, ps);
		}
		
		return CT_PENETRATE;
	}

	*nf2 = fs->features[maxFeature];

	return CT_CONTINUE;
}


int slFaceFaceClip(slFeature **nf1, slPosition *f1p, slShape *s1, slFeature **nf2, slPosition *f2p, slShape *s2, slVclipData *v, int x, int y) {
	slMessage(DEBUG_ALL, "internal warning: face/face clipping not implemented\n");
	return CT_CONTINUE;
}

/*
	\brief Find whether a point lies within a voronoi region.

	Returns 1 if a point is included within the specified region--
	otherwise returns 1, update will be the violated feature plane,
	dist will be the violation distance.
*/

int slClipPoint(slVector *p, slPlane *v, slPosition *vp, int vcount, int *update, double *dist) {
	slPlane tVoronoi;	
	int n;
	double d;

	*update = -1;

	for(n=0;n<vcount;n++) {
		slPositionPlane(vp, &v[n], &tVoronoi);

		d = slPlaneDistance(&tVoronoi, p);

		if(d < 0) {
			if(dist) *dist = d;
			*update = n;
			return 0;
		}
	}

	return 1;
}

/*
	\brief Find the maximally violated voronoi plane.
*/

int slClipPointMax(slVector *p, slPlane *v, slPosition *vp, int vcount, int *update) {
	slPlane tVoronoi;	 /* transformed voronoi plane */
	int n, minFeature = 0;
	double minScore = 0.0, m;

	*update = -1;

	for(n=0;n<vcount;n++) {
		slPositionPlane(vp, &v[n], &tVoronoi);

		m = slPlaneDistance(&tVoronoi, p);	 
		if(m < minScore) {
			minScore = m;
			minFeature = n;
		}   
	}

	if(minScore < 0.0) {
		*update = minFeature;
		return 0;
	}

	return 1;
}

/*
	\brief Determines whether an edge is inside a voronoi region

	Takes an edge "e" with a shape position "ep", and calculates 
	whether it is included in the area defined by voronoi planes
	"voronoi" with "vcount" planes at position "vp".

	If the edge is completely outside of the area without going
	through it at all, 0 is returned.

	Otherwise, 1 is returned.  

	If the start of the edge goes is outside the area, sf is 
	updated to the plane number that was violated, and hLambda is 
	set to a number from 0.0 to 1.0 indicating how far along the 
	edge the violation occured.  

	If the end of the edge goes is outside the area, ef is 
	updated to the plane number that was violated, and tLambda is 
	set to a number from 0.0 to 1.0 indicating how far along the 
	edge the violation occured.  
*/

int slClipEdge(slEdge *e, slPosition *ep, slPlane *voronoi, slPosition *vp, int vcount, int *sf, int *ef, double *hLambda, double *tLambda) {
	slVector transformedStart, transformedEnd;
	slPoint *start, *end;
	
	start = e->points[0];
	end = e->points[1];

	slPositionVertex(ep, &start->vertex, &transformedStart);
	slPositionVertex(ep, &end->vertex, &transformedEnd);
	
	return slClipEdgePoints(&transformedStart, &transformedEnd, voronoi, vp, vcount, sf, ef, hLambda, tLambda);
}

/*
	\brief Does the work for slClipEdge, after the transformed points have been computed.
*/

int slClipEdgePoints(slVector *transformedStart, slVector *transformedEnd, slPlane *voronoi, slPosition *vp, int vcount, int *sf, int *ef, double *hLambda, double *tLambda) {
	double dS, dE, lambda;
	slPlane tVoronoi; /* transformed voronoi plane */
	int n;
	
	/* initialize tail lambda and head lambda */
	
	*tLambda = 0;
	*hLambda = 1;
	
	*sf = -1;
	*ef = -1;
	for(n=0;n<vcount;n++) {
		slPositionPlane(vp, &voronoi[n], &tVoronoi);

		dS = slPlaneDistance(&tVoronoi, transformedStart);
		dE = slPlaneDistance(&tVoronoi, transformedEnd);
	
		/* both sides of the edge violate one of the planes */
	
		if(dS < 0 && dE < 0) {
			*ef = n;
			*sf = n;
			
			return 0;
		}
   
		/* figure out where each plane clips this edge.  if the  */
		/* tLambda and the hLambda cross it means that one plane */
		/* clips from 0 to eLambda and another from sLambda to 1 */
		/* and thus no part of the edge satisfies both planes	*/
		
		if(dE < 0) {
			/* everything from 0 to lambda is clipped by this plane */
  
			lambda = dE / (dE - dS);
			
			if(lambda > *tLambda) {
				*tLambda = lambda;				

				*ef = n;
				
				if(*tLambda > *hLambda) return 0;
			} 
		} else if(dS < 0) {
			/* everything from 1 to lambda is clipped by this plane */

			lambda = dE / (dE - dS);
		
			if(lambda < *hLambda) {
				*hLambda = lambda;

				*sf = n;
				
				if(*hLambda < *tLambda) return 0;
			}
		}	   
	} 

	/* the edge runs through the voronoi region */

	return 1;
}

/*!
	\brief Determine the colliding faces for intersecting objects.

	In order to catch all of the collision points between two objects, 
	we really have to look at two faces in their entireties.  The problem 
	is that the collision detection sometimes only catches one point on 
	a colliding face, so this function takes two shapes, along with the 
	colliding features, and figures out which faces are best used for the 
	collision computations.

	We know that the colliding features are on the colliding faces, so we 
	look at all the combinations of faces that the colliding features are 
	touching.  

	In many cases, it won't actually be faces colliding, so this will just 
	be precautionary.  When there are faces colliding, we know that they 
	should have opposing normal vectors and thus a dot product of near -1.  
	So, we simply look for the lowest dot product of all possible face 
	combinations.
*/

void slFindCollisionFaces(slShape *s1, slPosition *p1, slFeature **f1p, slShape *s2, slPosition *p2, slFeature **f2p) {
	int count1, count2, x, y;
	slFeature *f1, *f2;
	slFace **faces1, **faces2;
	double best2depth = -DBL_MAX, best1depth = -DBL_MAX;
	slFace *face1, *face2;
	slPlane plane1, plane2;

	f1 = *f1p;
	f2 = *f2p;

	// for each feature, get a pointer to the faces to check and the number of faces 

	if(f1->type == FT_POINT) {
		count1 = ((slPoint*)f1)->edgeCount;
		faces1 = ((slPoint*)f1)->faces;
	} else if(f1->type == FT_EDGE) {
		count1 = 2;
		faces1 = &((slEdge*)f1)->neighbors[2];
	} else {
		count1 = ((slFace*)f1)->edgeCount + 1;
		faces1 = ((slFace*)f1)->faces;
	}

	if(f2->type == FT_POINT) {
		count2 = ((slPoint*)f2)->edgeCount;
		faces2 = ((slPoint*)f2)->faces;
	} else if(f2->type == FT_EDGE) {
		count2 = 2;
		faces2 = &((slEdge*)f2)->neighbors[2];
	} else {
		count2 = ((slFace*)f2)->edgeCount + 1;
		faces2 = ((slFace*)f2)->faces;
	}

	for(x=0;x<count1;x++) {
		double d1, d2;

		face1 = faces1[x];
		slPositionPlane(p1, &face1->plane, &plane1);

		d1 = slPlaneDistance(&plane1, &p2->location);

		if(d1 > 0.0) {
			for(y=0;y<count2;y++) {
				face2 = faces2[y];
				slPositionPlane(p2, &face2->plane, &plane2);

				d2 = slPlaneDistance(&plane2, &p1->location);

				if(d2 > 0.0) {
					slPoint *point;
					slVector tv;
					double dist;
					double p1depths = 0, p2depths = 0;
					int pC = 0;
					std::vector<slPoint*>::iterator pi;

					for(pi = s1->points.begin(); pi != s1->points.end(); pi++ ) {
						point = *pi;
						slPositionVertex(p1, &point->vertex, &tv);
						dist = slPlaneDistance(&plane2, &tv);

						if(dist < 0.0) {
							pC++;
							p2depths += dist;
						}
					}

					if(pC) p2depths /= pC;

					if(p2depths < 0.0 && p2depths < best2depth) {
						best2depth = p2depths;
						*f2p = faces2[y];

						// slVectorPrint(&plane2.normal);
					}

					pC = 0;

					for(pi = s1->points.begin(); pi != s1->points.end(); pi++ ) {
						point = *pi;
						slPositionVertex(p2, &point->vertex, &tv);
						dist = slPlaneDistance(&plane1, &tv);

						if(dist < 0.0) {
							pC++;
							p1depths += dist;
						}
					}

					if(pC) p1depths /= pC;

					if(p1depths < 0.0 && p1depths < best1depth) {
						best1depth = p1depths;
						*f1p = faces1[x];

						// slVectorPrint(&plane1.normal);
					}
				} 
			}
		}
	}

	best1depth = -DBL_MAX;
	best2depth = -DBL_MAX;

	if((*f1p)->type == FT_FACE && (*f2p)->type == FT_FACE) return;

	for(x=0;x<count1;x++) {
		face1 = faces1[x];
		slPositionPlane(p1, &face1->plane, &plane1);

		for(y=0;y<count2;y++) {
			slPoint *point;
			slVector tv;
			double dist;
			double p1depths = 0, p2depths = 0;
			std::vector<slPoint*>::iterator pi;

			face2 = faces2[y];
			slPositionPlane(p2, &face2->plane, &plane2);

			for(pi = s1->points.begin(); pi != s1->points.end(); pi++ ) {
				point = *pi;
				slPositionVertex(p1, &point->vertex, &tv);
				dist = slPlaneDistance(&plane2, &tv);

				if(dist < 0.0) p2depths += dist;
			}

			if(p2depths < 0.0 && p2depths > best2depth) {
				best2depth = p2depths;
				*f2p = faces2[y];

				// slVectorPrint(&plane2.normal);
			}

			for(pi = s1->points.begin(); pi != s1->points.end(); pi++ ) {
				point = *pi;
				slPositionVertex(p2, &point->vertex, &tv);
				dist = slPlaneDistance(&plane1, &tv);

				if(dist < 0.0) p1depths += dist;
			}

			if(p1depths < 0.0 && p1depths > best1depth) {
				best1depth = p1depths;
				*f1p = faces1[x];

				// slVectorPrint(&plane1.normal);
			}
		}
	}
}

/*!
	\brief Find the collision points between two faces.

	For each plane, we look at its vertices, and check them against the other 
	plane.  This is the first part.

	We also have to be aware that collisions can occur at non vertex areas--that 
	is to say, the points at which edges of the suspect planes cross.  Think of 
	a book hanging over the edge of a table.  Two points are at vertices, while 
	the other two points are at the edge crossings.

	Thus, for one face, we check its vertices, while for the other face, we check 
	the vertices and we compute points on edges that go through the suspect plane.
*/

int slFaceFaceCollisionPoints(slCollisionEntry *ce, slShape *s1, slPosition *p1, slFeature *f1, slShape *s2, slPosition *p2, slFeature *f2) {
	int n, update1, update2, included;
	double l1, l2;

	slFace *face1, *face2;
	slEdge *theEdge;
	slPoint *thePoint;
	slPlane plane1, plane2;
	slVector *back, *front;
	slVector point, edgeVector, edgePoint;
	int useNorm1 = 0;
	double maxDepth;
	double distance;

    if(f1->type != FT_FACE || f2->type != FT_FACE) {
		slMessage(DEBUG_WARN, "non-face types in slFaceFaceCollisionPoints\n");
        return -1;
    }

	face1 = f1;
	face2 = f2;

	slPositionPlane(p1, &face1->plane, &plane1);
	slPositionPlane(p2, &face2->plane, &plane2);

	maxDepth = -fabs(slVectorDot(&face2->plane.normal, &face2->plane.vertex));

	for(n=0;n<face1->edgeCount;n++) {
		theEdge = face1->neighbors[n];
		included = slClipEdge(theEdge, p1, face2->voronoi, p2, face2->edgeCount, &update1, &update2, &l1, &l2);

		/* if either side of the edge is included, *and* that side has the point */
		/* matching the current point number (so that we don't do repeats), then */
		/* it gets tested. */

		if(update1 == -1 && theEdge->points[0] == face1->points[n]) {
			thePoint = theEdge->points[0];
		} else if(update2 == -1 && theEdge->points[1] == face1->points[n]) {
			thePoint = theEdge->points[1];
		} else {
			thePoint = NULL;
		}

		/* so here we've found the point we're looking for */

		if(thePoint) {
			/* the actual point is inside the suspect plane */

			slPositionVertex(p1, &thePoint->vertex, &point);
			distance = slPlaneDistance(&plane2, &point);

			if(distance < -1 && distance > maxDepth) {
				slDebug("point depth less than 1!\n");
			}

			if(distance <= MC_TOLERANCE && distance > maxDepth) {
				slNextCollisionPoint(ce);

				ce->pointDepths[ce->pointCount - 1] = distance;

				slVectorCopy(&point, &ce->worldPoints[ce->pointCount - 1]);
			}
		}

		if(included && update1 != -1) {
			front = &((slPoint*)theEdge->neighbors[0])->vertex;
			back = &((slPoint*)theEdge->neighbors[1])->vertex;

			/* there is a point on one end of this edge which enters the suspect plane */

			slVectorSub(front, back, &edgeVector);
			slVectorMul(&edgeVector, l1, &edgePoint);
			slVectorAdd(&edgePoint, back, &edgePoint);
			slPositionVertex(p1, &edgePoint, &point);

			/* compute the point from sD, check the distance */

			distance = slPlaneDistance(&plane2, &point);

			if(distance < -1 && distance > maxDepth) {
				slDebug("depth less than 1!\n");
			}

			if(distance <= MC_TOLERANCE && distance > maxDepth) {
				slNextCollisionPoint(ce);

				ce->pointDepths[ce->pointCount - 1] = distance;
				slVectorCopy(&point, &ce->worldPoints[ce->pointCount - 1]);
			}
		}

		if(included && update2 != -1) {
			front = &((slPoint*)theEdge->neighbors[0])->vertex;
			back = &((slPoint*)theEdge->neighbors[1])->vertex;

			/* there is a point on the other end of this edge which enters the suspect plane */

			slVectorSub(front, back, &edgeVector);
			slVectorMul(&edgeVector, l2, &edgePoint);
			slVectorAdd(&edgePoint, back, &edgePoint);
			slPositionVertex(p1, &edgePoint, &point);

			/* compute the point from eD, check the distance */

			distance = slPlaneDistance(&plane2, &point);

			if(distance < -1 && distance > maxDepth) {
				slDebug("depth less than 10!\n");
			}

			if(distance <= MC_TOLERANCE && distance > maxDepth) {
				slNextCollisionPoint(ce);

				ce->pointDepths[ce->pointCount - 1] = distance;
				slVectorCopy(&point, &ce->worldPoints[ce->pointCount - 1]);
			}
		}
	}

	/* for face2, it's a bit easier--we're only interested in the vertices */

	for(n=0;n<face2->edgeCount;n++) {
		thePoint = face2->points[n];
		slPositionVertex(p2, &thePoint->vertex, &point);

		distance = slPlaneDistance(&plane1, &point);

		if(distance <= MC_TOLERANCE) {
			included = slClipPoint(&point, face1->voronoi, p1, face1->edgeCount, &update1, &distance);

			if(included) {
				slNextCollisionPoint(ce);

				ce->pointDepths[ce->pointCount - 1] = distance;
				slVectorCopy(&point, &ce->worldPoints[ce->pointCount - 1]);

				/* we're piercing plane1, so we'll use its normal */

				useNorm1 = 1;
			}
		}
	}

	ce->distance = 0.0;

	for(n=0;n<ce->pointCount;n++)
		if(ce->pointDepths[n] < ce->distance) ce->distance = ce->pointDepths[n];

	if(useNorm1) {
		slVectorMul(&plane1.normal, -1, &ce->normal);
	} else {
		slVectorCopy(&plane2.normal, &ce->normal);
	}

	/* a deep collision is usually an indication that we're using the wrong plane */

	if(ce->distance < -0.1) {
		slMessage(100, "Unusually deep collision: %f meters\n", ce->distance);

		// getchar();

		// slVectorPrint(&plane1.normal);
		// slVectorPrint(&plane2.normal);
	}

	return 0;
}

/*!
	\brief Find colliding faces and count collision points for colliding objects.
*/

int slCountFaceCollisionPoints(slCollisionEntry *ce, slFeature *f1, slFeature *f2, slPosition *p1, slPosition *p2, slShape *s1, slShape *s2) {
	slFeature *newF1 = f1, *newF2 = f2;

	slFindCollisionFaces(s1, p1, &newF1, s2, p2, &newF2);
	slFaceFaceCollisionPoints(ce, s1, p1, newF1, s2, p2, newF2);
	return ce->pointCount;
}

/*!
	\brief Give the shortest distance from a point to a line.

	Intersection point i of point p3 on line p1->p2

	i = p1 + u(p2 - p1)
	(p2 - p1) dot (p3 - i) = 0, substitute:
	(p2 - p1) dot (p3 - p1 - u(p2 - p1)) = 0
	(p2 - p1) dot (p3 - p1) - (p2 - p1) dot u(p2 - p1) = 0
	(p2 - p1) dot (p3 - p1) = u * (p2 - p1) dot (p2 - p1)

	u = (p2 - p1) dot (P - p1) / (p2 - p1) dot (p2 - p1);

	whew.
*/

double slPointLineDist(slVector *p1, slVector *p2, slVector *src, slVector *i) {
	slVector p2m1, srcm1, tempV1;
	double u, top, bottom;

	slVectorSub(p2, p1, &p2m1);
	slVectorSub(src, p1, &srcm1);

	top = slVectorDot(&p2m1, &srcm1);
	bottom = slVectorDot(&p2m1, &p2m1);

	u = top / bottom;

	/* find where i is based on scalar u */

	slVectorMul(&p2m1, u, i);

	slVectorAdd(i, p1, i);

	slVectorSub(i, src, &tempV1);

	return slVectorLength(&tempV1);
}

/*!
	\brief Calculate the distance between a plane and a point.

	A positive value indicates that the point is on the "positive"
	side of the plane, in the same direction as the plane's normal.
*/

double slPlaneDistance(slPlane *pl, slVector *p) {
	return pl->normal.x * (p->x - pl->vertex.x) + 
		   pl->normal.y * (p->y - pl->vertex.y) + 
		   pl->normal.z * (p->z - pl->vertex.z); 
}

/*!
	\brief Find the minimally violated face for a point inside a shape.
*/

int slCollidingFaceForPoint(slVector *p, slShape *shape, slPosition *pos) {
	int n;
	slFace *face;
	slPlane plane;
	double dist, bestDist = -DBL_MAX;
	int bestFeature = -1;
	std::vector<slFace*>::iterator fi;

	for(fi = shape->faces.begin(); fi != shape->faces.end(); fi++ ) {
		face = *fi;

		slPositionPlane(pos, &face->plane, &plane);

		dist = slPlaneDistance(&plane, p);

		if(dist <= MC_TOLERANCE && dist > bestDist) {
			bestDist = dist;
			bestFeature = n;
		}
	}

	return bestFeature;
}

