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
#include "vclip.h"
#include "vclipData.h"

/*!
	\brief Initialize the slVclipData bound lists.

	This must be called to sort the lists for the first time and properly
	initialize the lists.
*/

void slInitBoundSort(slVclipData *d) {
	unsigned int listSize = d->count * 2;
	unsigned int x, y;

	d->candidates.clear();

	for(x=0;x<3;x++) {
		d->boundListPointers[x].clear();

		for(y=0;y<listSize;y++) d->boundListPointers[x].push_back(&d->boundLists[x][y]);
	}

	slInitBoundSortList(d->boundListPointers[0], d, BT_XAXIS);
	slInitBoundSortList(d->boundListPointers[1], d, BT_YAXIS);
	slInitBoundSortList(d->boundListPointers[2], d, BT_ZAXIS);
}

/*!
	\brief Adds an object-pair as a candidate for collision.

	If the first stage pruning indicates a potential collision, a 
	collision candidate is added so that the second stage detection
	is run.
*/

void slAddCollisionCandidate(slVclipData *vc, slPairFlags flags, int x, int y) {
	slCollisionCandidate c( vc, x, y);

	// the UNKNOWN flag indicates that we have not yet preformed a callback to 
	// determine whether further collision detection is necessary.

	if(flags & BT_UNKNOWN) flags = slVclipDataInitPairFlags(vc, x, y);

	if(!(flags & BT_CHECK)) return;

	vc->candidates[ slVclipPairFlags(vc, x, y) ] = c;
}

/*! 
	\brief Removes the collision candidate from the candidate list.
*/

void slRemoveCollisionCandidate(slVclipData *vc, int x, int y) {
	vc->candidates.erase( slVclipPairFlags(vc, x, y) );
}

bool slBoundSortCompare(const slBoundSort *a, const slBoundSort *b) {
	if(*a->value == *b->value) return (a->number < b->number);

	return (*a->value < *b->value);
}

int slVclipPruneAndSweep(slVclipData *d) {
	slIsort(d, d->boundListPointers[0], BT_XAXIS);
	slIsort(d, d->boundListPointers[1], BT_YAXIS);
	slIsort(d, d->boundListPointers[2], BT_ZAXIS);

	return 0;
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

int slVclipData::clip(double tolerance, int pruneOnly, int boundingBoxOnly) {
	int result;
	slCollision *ce;
	std::map< slPairFlags* , slCollisionCandidate >::iterator ci;

	collisionCount = 0;

	ce = slNextCollision(this);

	for(ci = candidates.begin(); ci != candidates.end(); ci++ ) {
		slCollisionCandidate c = ci->second;

		if(boundingBoxOnly) {
			if(objects[c._x] && objects[c._y] && c._x != c._y) {
				ce->n1 = c._x;
				ce->n2 = c._y;

				ce = slNextCollision(this);
			}
		} else {
			result = testPair(&c, ce);

			if(result == CT_ERROR) return -1;
			if(result == CT_PENETRATE) ce = slNextCollision(this);
		}
	}

	// collisionCount is pointing to the next empty collision 
	// in other words, 1 + the actual number of collisions

	collisionCount--;

	return collisionCount;
}

slCollision *slNextCollision(slVclipData *v, int x, int y) {
	slCollision *e = slNextCollision(v);

	e->n1 = x;
	e->n2 = y;

	return e;
}

slCollision *slNextCollision(slVclipData *v) {
	slCollision *e;

	if( v->collisionCount >= v->collisions.size()) {
		v->collisions.resize( v->collisions.size() + 10);
	}

	e = &v->collisions[ v->collisionCount++ ];
	e->points.clear();
	e->depths.clear();

	slVectorSet(&e->normal, 0, 0, 0);

	return e;
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
	plane, we do *n^2 iterations*.  So, to break ties, we rely on the 
	number field of the bound entry.

	This function is a major bottleneck for large simulations, and has
	been optimized accordingly.
*/

void slIsort(slVclipData *d, std::vector<slBoundSort*> &list, char boundTypeFlag) {
	int x, y;

	std::vector<slBoundSort*>::iterator currentI, leftI;

	if(list.size() == 0) return;

	for(currentI = list.begin() + 1; currentI != list.end(); currentI++) {
		const slBoundSort *currentSort = *currentI;
		slBoundSort *leftSort; 
		const int currentSortType = currentSort->type;

		x = currentSort->number; 

		if( d->objects[ x]->_moved || 1) {

			// NaNs mess up the logic of the sort since they are not =, < or > than
			// any other value, meaning they have no proper place in the list.  
			// -Infinity does have a proper (yet "meaningless") place in the list

			if (isnan(*currentSort->value)) *currentSort->value = -HUGE_VAL;

			// Keep moving to the left; until:
			// 1) there are no more entries to the left
			// 2) this entry is greater than the entry to the left 
			// 3) this entry is equal to the entry on the left AND has a larger number
		
			leftI = currentI;
			leftSort = *(leftI - 1);

			while(leftI > list.begin() && ((*currentSort->value < *leftSort->value) ||
				(*currentSort->value == *leftSort->value && currentSort->number < leftSort->number))) {

				y = leftSort->number;

				if(currentSortType != leftSort->type && x != y) {
					slPairFlags flags = slVclipPairFlagValue(d, x, y);

					if(currentSortType == BT_MIN && (flags & BT_CHECK)) {
						// the min moving to the left a max -- overlap, turn on the flag.

#ifdef DEBUG
						if((flags & boundTypeFlag) && *currentSort->value != *leftSort->value) slMessage(DEBUG_WARN, "vclip inconsistancy [flag already on]!\n");
#endif
						flags |= boundTypeFlag;
						slVclipPairFlagValue(d, x, y) = flags;

						if(slVclipFlagsShouldTest(flags)) slAddCollisionCandidate(d, flags, x, y);

					} else if(currentSortType == BT_MAX && flags & BT_CHECK && *currentSort->value != *leftSort->value) {
						// the max moving to the left a min -- no overlap, turn off the flag.
	
#ifdef DEBUG
						if(!(flags & boundTypeFlag)) slMessage(DEBUG_WARN, "vclip inconsistancy [flag already off]!\n");
#endif
	
						if(slVclipFlagsShouldTest(flags)) slRemoveCollisionCandidate(d, x, y);
	
						slVclipPairFlagValue(d, x, y) ^= boundTypeFlag;
					}
				}

				*(leftI - 1) = (slBoundSort*)currentSort;
				*leftI = leftSort;

				leftI--;

				if(leftI != list.begin()) {
					leftSort = *(leftI - 1);
				}
			}
		}
	}
}

/* 
	+ slInitBoundSortList
	=
	= initializes a slBoundSort list by sorting it.  unlike the normal
	= bound sort, the list has not been previously sorted and the flags
	= are thus not consistant.
*/

void slInitBoundSortList(std::vector<slBoundSort*> &list, slVclipData *v, char boundTypeFlag) {
	unsigned int n, extend;
	int x, y;
	int otherSide;
	slPairFlags *flags;

	std::sort(list.begin(), list.end(), slBoundSortCompare);

	// zero out this entry for all pairs 

	for(n=0;n<list.size();n++) {

		if(list[n]->type == BT_MIN) {
			extend = n+1;

			x = list[n]->number;

			// we move to the right until the end of the list, or until
			// the corresponding BT_MAX has been found AND the extend 
			// value is greater than the BT_MAX value 

			otherSide = -1;
		 
			while(extend < list.size() && (otherSide == -1 || *list[otherSide]->value == *list[extend]->value)) {
				if(list[extend]->number == list[n]->number) otherSide = extend;
				else {
					y = list[extend]->number;   
			
					flags = slVclipPairFlags(v, x, y);

					if(list[extend]->type == BT_MIN) {
						if(*flags & boundTypeFlag) {
							slMessage(DEBUG_ALL, "vclip init inconsistancy [flag is already on]\n");
						}

						*flags |= boundTypeFlag;

						if(slVclipFlagsShouldTest(*flags)) slAddCollisionCandidate(v, *flags, x, y);
					} 
				}

				extend++;
			}
		}
	}
}

/*!
	\brief transforms p1 with the position p, placing the transformed plane in pt.
*/

slPlane *slPositionPlane(slPosition *p, slPlane *p1, slPlane *pt) {
	slVectorXform(p->rotation, &p1->normal, &pt->normal);
	slPositionVertex(p, &p1->vertex, &pt->vertex);

	return pt;
}

/*!
	\brief Preforms a second-stage collision check on the specified object pair.
*/

int slVclipData::testPair(slCollisionCandidate *candidate, slCollision *ce) {
	int result = 0, iterations = 0;
	int x, y;
	int brute = 0;

	slPosition *p1, *p2;
	slShape *s1, *s2;
	slFeature **f1, **f2;

	x = candidate->_x;
	y = candidate->_y;

	/* 
		this here is dirty, because we're not supposed to expect the object
		fields to necessarily be slWorldObjects, but that's what the terrain
		checker is expecting.
	*/

	s1 = candidate->_shape1;
	s2 = candidate->_shape2;

	p1 = candidate->_position1;
	p2 = candidate->_position2;

	if(!s1 || !s2) return slTerrainTestPair(this, x, y, ce);

	f1 = &candidate->_feature1;
	f2 = &candidate->_feature2;

	if(s1->_type == ST_SPHERE && s2->_type == ST_SPHERE) {
		result = slSphereSphereCheck(this, x, y, ce, p1, (slSphere*)s1, p2, (slSphere*)s2);
	} else if(s1->_type == ST_SPHERE && s2->_type == ST_NORMAL) {
		result = slSphereShapeCheck(this, f2, x, y, ce);
	} else if(s1->_type == ST_NORMAL && s2->_type == ST_SPHERE) {
		result = slSphereShapeCheck(this, f1, y, x, ce);
	} else {
		int limit = ((s1->features.size() * s2->features.size()) + 1);

		do {
			if(brute) slMessage(DEBUG_WARN, "loop %p [%d] %p [%d]\n", *f1, (*f1)->type, *f2, (*f2)->type);

			if(iterations == limit) {
				// this is the worst-case scenario.  it indicates serious 
				// problems in the logic (by which i mean my implementation) 
				// of the vclip algorithm.

				slMessage(DEBUG_ALL, "warning: vclip feature loop detected [%d iterations], using brute force detection\n", iterations);

				return slVclipTestPairAllFeatures(this, candidate, ce);
	
				brute = 1;
			}

			if(((*f1)->type == FT_POINT) && ((*f2)->type == FT_POINT)) {
				result = slPointPointClip(f1, p1, s1, f2, p2, s2, ce);
			} else if(((*f1)->type == FT_POINT) && ((*f2)->type == FT_EDGE)) {
				result = slEdgePointClip(f2, p2, s2, f1, p1, s1, ce);
			} else if(((*f1)->type == FT_EDGE) && ((*f2)->type == FT_POINT)) {
				result = slEdgePointClip(f1, p1, s1, f2, p2, s2, ce);
			} else if(((*f1)->type == FT_POINT) && ((*f2)->type == FT_FACE)) {
				result = slPointFaceClip(f1, p1, s1, f2, p2, s2, this, x, y, ce);
			} else if(((*f1)->type == FT_FACE) && ((*f2)->type == FT_POINT)) {
				result = slPointFaceClip(f2, p2, s2, f1, p1, s1, this, y, x, ce);
			} else if(((*f1)->type == FT_EDGE) && ((*f2)->type == FT_EDGE)) {
				result = slEdgeEdgeClip(f1, p1, s1, f2, p2, s2, ce);
			} else if(((*f1)->type == FT_EDGE) && ((*f2)->type == FT_FACE)) {
				result = slEdgeFaceClip(f1, f2, this, x, y, ce);
			} else if(((*f1)->type == FT_FACE) && ((*f2)->type == FT_EDGE)) {
				result = slEdgeFaceClip(f2, f1, this, y, x, ce);
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

/*! 
	\brief A brute-force collision check.

	There appear to be some cases in which the regular feature testing
	Fails and causes a loop.  this is obviously a bug, and should be 
	fixed.  however, since the simulation must go on, as they say in
	show business, we'll have a brute force method that checks
	every feature pair.
	
	This is only done if a feature loop is detected.
*/

int slVclipTestPairAllFeatures(slVclipData *vc, slCollisionCandidate *candidate, slCollision *ce) {
	int result;
	slPosition *p1, *p2;
	slShape *s1, *s2;
	slFeature **f1, **f2, *fp1, *fp2;
	std::vector<slFeature*>::iterator fi1, fi2;

	int x, y;
	
	x = candidate->_x;
	y = candidate->_y;

	if(!vc->objects[x] || !vc->objects[y]) return 0;

	s1 = candidate->_shape1;
	s2 = candidate->_shape2;

	p1 = candidate->_position1;
	p2 = candidate->_position2;

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
				result = slPointFaceClip(f1, p1, s1, f2, p2, s2, vc, x, y, ce);
			} else if(((*f1)->type == FT_FACE) && ((*f2)->type == FT_POINT)) {
				result = slPointFaceClip(f2, p2, s2, f1, p1, s1, vc, y, x, ce);
			} else if(((*f1)->type == FT_EDGE) && ((*f2)->type == FT_EDGE)) {
				result = slEdgeEdgeClip(f1, p1, s1, f2, p2, s2, ce);
			} else if(((*f1)->type == FT_EDGE) && ((*f2)->type == FT_FACE)) {
				result = slEdgeFaceClip(f1, f2, vc, x, y, ce);
			} else if(((*f1)->type == FT_FACE) && ((*f2)->type == FT_EDGE)) {
				result = slEdgeFaceClip(f2, f1, vc, y, x, ce);
			} else if(((*f1)->type == FT_FACE) && ((*f2)->type == FT_FACE)) {
				// this is not currently implemented and would only print an error, so skip it 

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

double slSignEdgePlaneDistanceDeriv(slPlane *p, slVector *edgeVector, slVector *lambda) {
	double d;

	d = slPlaneDistance(p, lambda);

	return d * slVectorDot(edgeVector, &p->normal);
}

int slSphereSphereCheck(slVclipData *vc, int x, int y, slCollision *ce, slPosition *p1, slSphere *s1, slPosition *p2, slSphere *s2) {
	slVector diff, tempV1;
	double totalLen, totalDiff, sqrtDiff, depth;

	totalLen = (s1->_radius + s2->_radius);

	slVectorSub(&p2->location, &p1->location, &diff);

	// the actual diff is sqrt(totalDiff), but we'll avoid the sqrt if possible 

	totalDiff = (diff.x * diff.x) + (diff.y * diff.y) + (diff.z * diff.z);

	depth = totalDiff - (totalLen * totalLen);

	if(depth > 0.0) return CT_DISJOINT;

	if(!ce) return CT_PENETRATE;

	// collision detected, do the sqrt to find the actual depth.

	sqrtDiff = sqrt(totalDiff);

	depth = (totalLen - sqrtDiff);

	ce->n1 = x;
	ce->n2 = y;

	slVectorMul(&diff, 1.0/sqrtDiff, &ce->normal);

	slVectorMul(&ce->normal, s1->_radius, &tempV1);

	slVectorAdd(&tempV1, &p1->location, &tempV1);

	ce->points.push_back(tempV1);
	ce->depths.push_back(depth);

	slVectorMul(&ce->normal, -1, &ce->normal);
	return CT_PENETRATE;
}

int slSphereShapeCheck(slVclipData *vc, slFeature **feat, int x, int y, slCollision *ce) {
	unsigned int n = 0;
	int included, update;
	slFeature *minFeature = 0;

	slVector vertex, tempV1, spPoint, shPoint, tStart, tEnd;

	slPoint *p, *start, *end;
	slEdge *e;
	slFace *f;

	double dist, centerDist, minDist = DBL_MAX;

	slPlane transformedPlane;

	slSphere *s1 = (slSphere*)vc->objects[x]->shape;
	slShape *s2 = vc->objects[y]->shape;
	slPosition *p1 = &vc->objects[x]->position;
	slPosition *p2 = &vc->objects[y]->position;

	if(!*feat) *feat = s2->features[0];
	minFeature = s2->faces[0];

	while(n < s2->features.size()) {
		*feat = s2->features[n];

		switch((*feat)->type) {
			case FT_FACE:
				f = (slFace*)*feat;

				included = slClipPoint(&p1->location, f->voronoi, p2, f->edgeCount, &update, NULL);

				slPositionPlane(p2, &f->plane, &transformedPlane);
				centerDist = slPlaneDistance(&transformedPlane, &p1->location);

				if(included) {
					dist = centerDist - s1->_radius;

					if(dist > MC_TOLERANCE) {
						// if(ce) ce->distance = dist;
						return CT_DISJOINT;
					} else if(dist > -s1->_radius) {
						if(!ce) return CT_PENETRATE;

						slVectorCopy(&transformedPlane.normal, &ce->normal);

						slVectorMul(&ce->normal, -s1->_radius, &tempV1);
						slVectorInvXform(p1->rotation, &tempV1, &spPoint);

						slVectorAdd(&tempV1, &p1->location, &tempV1);

						ce->points.push_back(tempV1);
						ce->depths.push_back(dist);

						// now take the sphere location + the offset to find the 
						// collision point in world coordinates, and then 
						// translate into shape coord 

						slVectorAdd(&tempV1, &p1->location, &tempV1);
						slVectorSub(&tempV1, &p2->location, &tempV1);
						slVectorInvXform(p2->rotation, &tempV1, &shPoint);

						ce->n1 = x;
						ce->n2 = y;

						return CT_PENETRATE;
					} 

					*feat = f->neighbors[0];
				} else { 
					*feat = f->neighbors[update];
				}

				if(centerDist < minDist) {
					minDist = centerDist;
					minFeature = f;
				}

				break;

			case FT_EDGE:
				e = (slEdge*)*feat;

				included = slClipPointMax(&p1->location, e->voronoi, p2, 4, &update);

				if(included) {
					start = e->points[0];
					end = e->points[1];

					slPositionVertex(p2, &start->vertex, &tStart);
					slPositionVertex(p2, &end->vertex, &tEnd);
		   
					dist = slPointLineDist(&tStart, &tEnd, &p1->location, &shPoint) - s1->_radius;

					if(dist <= MC_TOLERANCE) {
						if(!ce) return CT_PENETRATE;

						slVectorSub(&p1->location, &shPoint, &ce->normal);

						ce->points.push_back(shPoint);
						ce->depths.push_back(dist);

						slVectorNormalize(&ce->normal);

						slVectorMul(&ce->normal, -s1->_radius, &tempV1);
						slVectorInvXform(p1->rotation, &tempV1, &spPoint);
						slVectorInvXform(p2->rotation, &tempV1, &shPoint);

						ce->n1 = x;
						ce->n2 = y;

						return CT_PENETRATE;
					}

					// if(ce) ce->distance = dist;
					return CT_DISJOINT;
				}

				*feat = e->neighbors[update];

				break;

			case FT_POINT:
				p = (slPoint*)*feat;

				included = slClipPoint(&p1->location, p->voronoi, p2, p->edgeCount, &update, NULL);

				if(included) {
					slPositionVertex(p2, &p->vertex, &vertex);
					slVectorSub(&p1->location, &vertex, &vertex);

					dist = slVectorLength(&vertex) - s1->_radius;

					if(dist <= MC_TOLERANCE) {
						if(!ce) return CT_PENETRATE;

						slVectorCopy(&vertex, &ce->normal);
						slVectorNormalize(&ce->normal);

						ce->points.push_back(vertex);
						ce->depths.push_back(dist);

						ce->n1 = x;
						ce->n2 = y;

						return CT_PENETRATE;
					}

					// if(ce) ce->distance = dist;

					return CT_DISJOINT;
				}

				*feat = p->neighbors[update];

				break;
		}

		n++;
	}

	// oh crap!  we're inside the sphere.  that's not a metaphor  
	// or anything like "in the zone", or something--i mean, we are
	// inside the sphere, or the sphere is inside us.

	f = (slFace*)minFeature;

	slPositionPlane(p2, &f->plane, &transformedPlane);

	if(!ce) return CT_PENETRATE;

	slVectorCopy(&transformedPlane.normal, &ce->normal);
	slVectorNormalize(&ce->normal);

	/* give an arbitrary VERY DEEP collision */
	
	ce->n1 = y;
	ce->n2 = x;

	ce->depths.push_back(-10);

	return CT_PENETRATE;
}

int slPointPointClip(slFeature **nf1, slPosition *p1p, slShape *s1, slFeature **nf2, slPosition *p2p, slShape *s2, slCollision *ce) {
	int update;
	slVector v1, v2;
   
	slPoint *p1 = (slPoint*)*nf1;
	slPoint *p2 = (slPoint*)*nf2;

	slPositionVertex(p2p, &p2->vertex, &v2);

	if(!slClipPoint(&v2, p1->voronoi, p1p, p1->edgeCount, &update, NULL)) {
		*nf1 = p1->neighbors[update];
		return CT_CONTINUE;
	} 

	slPositionVertex(p1p, &p1->vertex, &v1);

	if(!slClipPoint(&v1, p2->voronoi, p2p, p2->edgeCount, &update, NULL)) {
		*nf2 = p2->neighbors[update];
		return CT_CONTINUE;
	}

	return CT_DISJOINT;
}

int slEdgeFaceClip(slFeature **nf1, slFeature **nf2, slVclipData *v, int x, int y, slCollision *ce) {
	int update1, update2, included, n;
	double l1, l2, eD, sD;
	slVector edgeVector, tempV1, eLambda, sLambda, transEdgeVector;
	slPoint *start, *end;
	slVector tStart, tPoint, tEnd, linePoint;
	
	slPlane transformedPlane;
	slFace *f;
	slEdge *e;

	slPosition *ep = &v->objects[x]->position;
	slPosition *fp = &v->objects[y]->position;
	slShape *s1 = v->objects[x]->shape;
	slShape *s2 = v->objects[y]->shape;

	double maxDist;
	slFeature *maxFeat;
   
	f = (slFace*)*nf2;
	e = (slEdge*)*nf1;

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

	if(included) {
		if((eD <= MC_TOLERANCE && sD >= MC_TOLERANCE) || (sD <= MC_TOLERANCE && eD >= MC_TOLERANCE)) {
			slVector theVertex;
			double dist;
			std::vector<slFace*>::iterator fi;

			maxDist = -DBL_MAX;
	
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
	
			slCountFaceCollisionPoints(ce, *nf1, *nf2, ep, fp, s1, s2);
			ce->n1 = x;
			ce->n2 = y;
	
			return CT_PENETRATE;
		}

		if(slSignEdgePlaneDistanceDeriv(&transformedPlane, &transEdgeVector, &eLambda) > 0.0) {
			if(update2 != -1) *nf2 = f->neighbors[update2];
			else *nf1 = e->neighbors[1];
		}

		if(slSignEdgePlaneDistanceDeriv(&transformedPlane, &transEdgeVector, &sLambda) < 0.0) {
			if(update1 != -1) *nf2 = f->neighbors[update1];
			else *nf1 = e->neighbors[0];
		}

		return CT_CONTINUE;
	} else {
		// Find closest vertex.  This is not how Mirtich describes it,
		// but it looks like it's working (5 years after I originally
		// wrote it).

		maxDist = DBL_MAX;

		// the transformed beginning of the edge... 
		
		slPositionVertex(ep, &start->vertex, &tStart);
		slPositionVertex(ep, &end->vertex, &tEnd);

		maxFeat = f->points[0];

		for(n = 0; n < f->edgeCount ; n++ ) {
			start = f->points[n];

			slPositionVertex(fp, &start->vertex, &tPoint);

			eD = slPointLineDist(&tStart, &tEnd, &tPoint, &linePoint);

			if(eD < maxDist) {
				maxDist = eD;
				maxFeat = start;
			}
		} 

		*nf2 = maxFeat;

		if(! maxFeat) slMessage(DEBUG_WARN, "vclip edge/face error: no max feature found\n");

		return CT_CONTINUE;
	}

	return CT_CONTINUE;
}

int slEdgePointClip(slFeature **nf1, slPosition *ep, slShape *s1, slFeature **nf2, slPosition *pp, slShape *s2, slCollision *ce) {
	int update1, update2;
	double l1, l2; /* edge clip lambdas */
	slVector edgeVector, eLambda, transEdgeVector, tempV1;
	slPoint *start, *end;

	slVector tPoint;

	slEdge *e = (slEdge*)*nf1;
	slPoint *p = (slPoint*)*nf2;

	slPositionVertex(pp, &p->vertex, &tPoint);
	
	if(!slClipPoint(&tPoint, e->voronoi, ep, 4, &update1, NULL)) {
		*nf1 = e->neighbors[update1];
		return CT_CONTINUE;
	}
	
	slClipEdge(e, ep, p->voronoi, pp, p->edgeCount, &update1, &update2, &l1, &l2);

	if(update1 != -1 || update2 != -1) {
		if(update1 == update2) {
			*nf2 = p->neighbors[update2];
			return CT_CONTINUE;
		}

		start = e->points[0];
		end = e->points[1];

		slVectorSub(&start->vertex, &end->vertex, &edgeVector);
		slVectorXform(ep->rotation, &edgeVector, &transEdgeVector);

		if(update2 != -1) {
			slVectorMul(&edgeVector, l2, &eLambda);
			slVectorAdd(&eLambda, &end->vertex, &tempV1);
			slPositionVertex(ep, &tempV1, &eLambda);

			slVectorSub(&eLambda, &tPoint, &eLambda);

			if(slVectorDot(&transEdgeVector, &eLambda) > 0.0) {
				*nf2 = p->neighbors[update2];
				return CT_CONTINUE;
			}		
		}

		if(update1 != -1) {
			slVectorMul(&edgeVector, l1, &eLambda);
			slVectorAdd(&eLambda, &end->vertex, &tempV1);
			slPositionVertex(ep, &tempV1, &eLambda);

			slVectorSub(&eLambda, &tPoint, &eLambda);

			if(slVectorDot(&transEdgeVector, &eLambda) < 0.0) {
				*nf2 = p->neighbors[update1];		   
				return CT_CONTINUE;
			}
		} 
	} 

	//// If we wanted the distance between non-colliding features...

	// if(ce) {
	// 	slPositionVertex(ep, &start->vertex, &tStart);
	// 	slPositionVertex(ep, &end->vertex, &tEnd);
	// 	ce->distance = slPointLineDist(&tStart, &tEnd, &tPoint, &linePoint);
	// }
	
	return CT_DISJOINT;
}

int slEdgeEdgeClip(slFeature **nf1, slPosition *p1, slShape *s1, slFeature **nf2, slPosition *p2, slShape *s2, slCollision *ce) {
	int update1, update2;
	double l1, l2; /* edge lambdas */
	slPoint *start, *end;
	
	slVector edgeVector, transEdgeVector, eLambda, transPoint, tempV1;
	slFeature *neighbor;

	slFace *neighborFace;

	slPlane transformedPlane;

	slEdge *e1, *e2;

	e1 = (slEdge*)*nf1;
	e2 = (slEdge*)*nf2;

	// we clip the voronoi planes two at a time... first the vertex planes, then the slFace planes 
	
	slClipEdge(e2, p2, &e1->voronoi[0], p1, 2, &update1, &update2, &l1, &l2);

	if(update1 != -1 || update2 != -1) {
		if(update1 == update2) {
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

			if(slVectorDot(&transEdgeVector, &eLambda) > 0.0) {
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

			if(slVectorDot(&transEdgeVector, &eLambda) < 0.0) {
				*nf1 = neighbor;
				return CT_CONTINUE;
			}		
		}
	}

	slClipEdge(e2, p2, &e1->voronoi[2], p1, 2, &update1, &update2, &l1, &l2);

	if(update1 != -1 || update2 != -1) {
		if(update1 == update2) {
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

			if(slSignEdgePlaneDistanceDeriv(&transformedPlane, &transEdgeVector, &eLambda) > 0.0) {
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

			if(slSignEdgePlaneDistanceDeriv(&transformedPlane, &transEdgeVector, &eLambda) < 0.0) {
				*nf1 = e1->neighbors[update1 + 2];
				return CT_CONTINUE;
			}
		}		
	}
	
	slClipEdge(e1, p1, &e2->voronoi[0], p2, 2, &update1, &update2, &l1, &l2);

	if(update1 != -1 || update2 != -1) {
		if(update1 == update2) {
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

			if(slVectorDot(&transEdgeVector, &eLambda) > 0.0) {
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

			if(slVectorDot(&transEdgeVector, &eLambda) < 0.0) {
				*nf2 = neighbor;
				return CT_CONTINUE;
			}		
		}   
	}

	slClipEdge(e1, p1, &e2->voronoi[2], p2, 2, &update1, &update2, &l1, &l2);

	if(update1 != -1 || update2 != -1) {
		if(update1 == update2) {
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

			if(slSignEdgePlaneDistanceDeriv(&transformedPlane, &transEdgeVector, &eLambda) > 0.0) {
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

			if(slSignEdgePlaneDistanceDeriv(&transformedPlane, &transEdgeVector, &eLambda) < 0.0) {
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

int slPointFaceClip(slFeature **nf1, slPosition *pp, slShape *ps, slFeature **nf2, slPosition *fp, slShape *fs, slVclipData *v, int x, int y, slCollision *ce) {
	int update;
	int n;
	slVector tPoint, tEnd;
	slPoint *endPoint;
	double sD, asD, eD, max;
	slFace *newFace;
	slPlane transformedPlane;

	slFace *f = (slFace*)*nf2;
	slPoint *p = (slPoint*)*nf1;

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
		slEdge *edgeNeighbor = p->neighbors[n];

		// we may have gotten the end we already have 

		endPoint = edgeNeighbor->points[0];
		if(endPoint == p) endPoint = edgeNeighbor->points[1];
		
		slPositionVertex(pp, &endPoint->vertex, &tEnd);

		eD = slPlaneDistance(&transformedPlane, &tEnd);

		// we'll continue with this edge if:
		//   - the other end of the edge is closer to the plane than the current vertex
		//   - OR if the edge appears to intersect the plane

        if(fabs(eD) < asD || ((eD <= MC_TOLERANCE || sD <= MC_TOLERANCE) && (eD > MC_TOLERANCE || sD > MC_TOLERANCE))) {
			*nf1 = p->neighbors[n];
			return CT_CONTINUE;
		}
	}

	if(sD > MC_TOLERANCE) return CT_DISJOINT;

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
			*nf2 = *fi;
			max = sD;
		} 
	}

	/* we'll end up here if there's nothing above zero */

	if(max <= MC_TOLERANCE) {
		if(!ce) return CT_PENETRATE;

		slVectorXform(fp->rotation, &f->plane.normal, &ce->normal);

		ce->n1 = x;
		ce->n2 = y;

		slCountFaceCollisionPoints(ce, *nf1, *nf2, pp, fp, ps, fs);
		
		return CT_PENETRATE;
	}

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

int slClipEdgePoints(slVector *transformedStart, slVector *transformedEnd, slPlane *voronoi, slPosition *vp, int vcount, int *sf, int *ef, double *hLambda, double *tLambda) {
    double dS, dE, lambda;
    slPlane tVoronoi; 
    int n;

	// initialize tail lambda and head lambda 
	
	*tLambda = 0;
	*hLambda = 1;
	
	*sf = -1;
	*ef = -1;

	for(n=0;n<vcount;n++) {
		slPositionPlane(vp, &voronoi[n], &tVoronoi);

		dS = slPlaneDistance(&tVoronoi, transformedStart);
		dE = slPlaneDistance(&tVoronoi, transformedEnd);
	
		// both sides of the edge violate one of the planes 
	
		if(dS < 0.0 && dE < 0.0) {
			*ef = n;
			*sf = n;
			
			return 0;
		}
   
		// figure out where each plane clips this edge.  if the  
		// tLambda and the hLambda cross it means that one plane 
		// clips from 0 to eLambda and another from sLambda to 1 
		// and thus no part of the edge satisfies both planes	
		
		if(dE < 0.0) {
			// everything from 0 to lambda is clipped by this plane 
  
			lambda = dE / (dE - dS);
			
			if(lambda > *tLambda) {
				*tLambda = lambda;				

				*ef = n;
				
				if(*tLambda > *hLambda) return 0;
			} 
		} else if(dS < 0.0) {
			/* everything from 1 to lambda is clipped by this plane */

			lambda = dE / (dE - dS);
		
			if(lambda < *hLambda) {
				*hLambda = lambda;

				*sf = n;
				
				if(*hLambda < *tLambda) return 0;
			}
		}	   
	} 

	// the edge runs through the voronoi region 

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
	double bestDist;
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
		faces1 = ((slEdge*)f1)->faces;
	} else {
        count1 = ((slFace*)f1)->edgeCount + 1;
        faces1 = ((slFace*)f1)->faces;
	}

	if(f2->type == FT_POINT) {
		count2 = ((slPoint*)f2)->edgeCount;
		faces2 = ((slPoint*)f2)->faces;
	} else if(f2->type == FT_EDGE) {
		count2 = 2;
		faces2 = ((slEdge*)f2)->faces;
	} else {
        count2 = ((slFace*)f2)->edgeCount + 1;
        faces2 = ((slFace*)f2)->faces;
	}

	*f1p = NULL;
	*f2p = NULL;

	//
	// STEP 1: find any face which has the other shape's center on the positive 
	// side and inside its voronoi region
	//

	bestDist = -DBL_MAX;

	for(x=0;x<count1;x++) {
		double dist, d2;
		int included, update;

		face1 = faces1[x];
		slPositionPlane(p1, &face1->plane, &plane1);

		dist = slPlaneDistance(&plane1, &p2->location);

		included = slClipPoint(&p2->location, face1->voronoi, p1, face1->edgeCount, &update, &d2);

		if(included && dist > 0) {
			*f1p = face1;
			x = count1;
		}
	}

	bestDist = -DBL_MAX;

	for(x=0;x<count2;x++) {
		double dist, d2;
		int included, update;

		face2 = faces2[x];
		slPositionPlane(p2, &face2->plane, &plane2);

		dist = slPlaneDistance(&plane2, &p1->location);

		included = slClipPoint(&p1->location, face2->voronoi, p2, face2->edgeCount, &update, &d2);

		if(included && dist > 0) {
			*f2p = face2;
			x = count2;
		}
	}

	//
	// If we find two faces which fit this requirement, then we're done.
	//

	if(*f2p && *f1p) return;

	//
	// STEP 2: If we've only found one plane which contains the other shape's 
	// center, then there are many candidates for the other plane.  Pick one
	// with the most negative dot(norm1, norm2).
	//

	if(*f1p) {
		slPositionPlane(p1, &((slFace*)*f1p)->plane, &plane1);

		bestDist = 0.0;

		for(x=0;x<count2;x++) {
			double dot;

			face2 = faces2[x];
			slPositionPlane(p2, &face2->plane, &plane2);

			dot = slVectorDot(&plane2.normal, &plane1.normal);

			if(dot < bestDist) {
				*f2p = face2;
				bestDist = dot;
			}
		}

		if(*f2p) return;
	}

	if(*f2p) {
		slPositionPlane(p2, &((slFace*)*f2p)->plane, &plane2);

		bestDist = 0.0;

		for(x=0;x<count1;x++) {
			double dot;

			face1 = faces1[x];
			slPositionPlane(p1, &face1->plane, &plane1);

			dot = slVectorDot(&plane2.normal, &plane1.normal);

			if(dot < bestDist) {
				*f1p = face1;
				bestDist = dot;
			}
		}
	
		if(*f1p) return;
	}

	//
	// STEP 3: go through all N*M face pairs.
	//

	best1depth = -DBL_MAX;
	best2depth = -DBL_MAX;

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
			}

			for(pi = s2->points.begin(); pi != s2->points.end(); pi++ ) {
				point = *pi;
				slPositionVertex(p2, &point->vertex, &tv);
				dist = slPlaneDistance(&plane1, &tv);

				if(dist < 0.0) p1depths += dist;
			}

			if(p1depths < 0.0 && p1depths > best1depth) {
				best1depth = p1depths;
				*f1p = faces1[x];
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

int slFaceFaceCollisionPoints(slCollision *ce, slShape *s1, slPosition *p1, slFace *face1, slShape *s2, slPosition *p2, slFace *face2) {
	int n, update1, update2, included;
	double l1, l2;

	slEdge *theEdge;
	slPoint *thePoint;
	slPlane plane1, plane2;
	slVector *back, *front;
	slVector point, edgeVector, edgePoint;
	int useNorm1 = 0;
	double maxDepth;
	double distance;
	slVector zero;

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
				slMessage(DEBUG_WARN, "vclip deep collision: point depth less than 1\n");
			}

			if(distance <= MC_TOLERANCE && distance > maxDepth) {
				ce->points.push_back(point);
				ce->depths.push_back(distance);
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
				slMessage(DEBUG_WARN, "depth less than 1!\n");
			}

			if(distance <= MC_TOLERANCE && distance > maxDepth) {
				ce->points.push_back(point);
				ce->depths.push_back(distance);
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
				slMessage(DEBUG_WARN, "vclip deep collision: point depth less than 10!\n");
			}

			if(distance <= MC_TOLERANCE && distance > maxDepth) {
				ce->points.push_back(point);
				ce->depths.push_back(distance);
			}
		}
	}

	// for face2, it's a bit easier--we're only interested in the vertices 

	slVectorSet(&zero, 0, 0, 0);
	maxDepth = slPlaneDistance(&face1->plane, &zero);

	for(n=0;n<face2->edgeCount;n++) {
		thePoint = face2->points[n];
		slPositionVertex(p2, &thePoint->vertex, &point);

		distance = slPlaneDistance(&plane1, &point);

		if(distance <= MC_TOLERANCE) {
			double d2;

			included = slClipPoint(&point, face1->voronoi, p1, face1->edgeCount, &update1, &d2);

			if(included && distance > maxDepth) {
				ce->points.push_back(point);
				ce->depths.push_back(distance);

				// we're piercing plane1, so we'll use its normal 

				useNorm1 = 1;
			} else if(included) { 
				// slDebug("crazy collison %f [%f]\n", distance, maxDepth);
			} 
		}
	}

	if(useNorm1) {
		slVectorMul(&plane1.normal, -1, &ce->normal);
	} else {
		slVectorCopy(&plane2.normal, &ce->normal);
	}

	return 0;
}

/*!
	\brief Find colliding faces and count collision points for colliding objects.
*/

int slCountFaceCollisionPoints(slCollision *ce, slFeature *f1, slFeature *f2, slPosition *p1, slPosition *p2, slShape *s1, slShape *s2) {
	slFeature *newF1 = f1, *newF2 = f2;

	slFindCollisionFaces(s1, p1, &newF1, s2, p2, &newF2);
	slFaceFaceCollisionPoints(ce, s1, p1, (slFace*)newF1, s2, p2, (slFace*)newF2);

	return ce->points.size();
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

	// find where i is based on scalar u 

	slVectorMul(&p2m1, u, i);

	slVectorAdd(i, p1, i);

	slVectorSub(i, src, &tempV1);

	return slVectorLength(&tempV1);
}
