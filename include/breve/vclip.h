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

#ifndef _VCLIP_H
#define _VCLIP_H

typedef unsigned char slPairFlags;

#include "vclipData.h"

#define MC_TOLERANCE	0.00
#define VC_WARNING_TOLERANCE -0.1

#define slVclipPairFlags(pe, x, y)		((x)>(y)?(&(pe)->pairArray[(x)][(y)]):(&(pe)->pairArray[(y)][(x)]))
#define slVclipPairFlagValue(pe, x, y)		((x)>(y)?((pe)->pairArray[(x)][(y)]):((pe)->pairArray[(y)][(x)]))

enum collisionTypes {
	CC_SIMULATE,
	CC_CALLBACK
};

enum clipCodes {
	CT_DISJOINT = 0,
	CT_CONTINUE,
	CT_PENETRATE,
	CT_ERROR
};

enum slBoundSortTypes {
	BT_MIN = 0,
	BT_MAX
};

enum slCollisionFlags {
	BT_XAXIS 	= 0x01, // 0000001 
	BT_YAXIS 	= 0x02, // 0000010 
	BT_ZAXIS 	= 0x04, // 0000100 
	BT_CHECK 	= 0x08, // 0001000 
	BT_ALL   	= 0x0f, // 0001111 -- x, y, z overlap and checking desired
	BT_CALLBACK	= 0x10,	// 0010000 -- these objects require a callback
	BT_SIMULATE = 0x20,	// 0100000 -- these objects require simulation
	BT_UNKNOWN	= 0x40	// 1000000 -- need to check callback & simulation
};

#define slVclipFlagsShouldTest(x)		(((x) & BT_ALL) == BT_ALL)

/*!
	\brief The minima and maxima of the collision detection pruning stage.

	Hold the maxima or minima data for all the objects in the world.  These 
	are sorted with an insertion sort in order to locate overlapping pairs.
*/

struct slBoundSort {
	unsigned int number;
	char type;
	double *value;
};

#ifdef __cplusplus
#include <vector>
#include <map>

/*!
	\brief A record of a collision.

	When a collision occurs, one of these entries is created.  It
	is used to add a contact point for the collision.
*/

class slCollision {
	public:
		slVector normal;

		std::vector<slVector> points;
		std::vector<double> depths;

		unsigned int n1; 
		unsigned int n2;
};

/*! 
	\brief Data associated with the v-clip algorithm.
*/

class slVclipData {
	public:
		/*!
		 * Runs collision detection for all candidate pairs.
		 */

		int clip(double tolerance, int pruneOnly, int boundingBoxOnly);

		/*!
		 * Tests a single pair stored in a \ref slCollisionCandidate.
		 */

		int testPair(slCollisionCandidate *e, slCollision *ce);

		slWorld *world;

		std::vector<slBoundSort*> boundListPointers[3];
		std::vector<slBoundSort> boundLists[3];

		std::vector<slWorldObject*> objects;

		std::vector<slCollision> collisions;
		unsigned int collisionCount;

		slPairFlags **pairArray;

		std::map< slPairFlags* , slCollisionCandidate > candidates;

		unsigned int count;
		unsigned int maxCount;
};

class slCollisionCandidate {
	public:
		slCollisionCandidate() {};

		slCollisionCandidate(slVclipData *vc, int o1, int o2) {
			slWorldObject *w1, *w2;

			_x = o1; _y = o2;

			w1 = vc->objects[_x];
			w2 = vc->objects[_y];

			_position1 = &w1->position;	
			_position2 = &w2->position;	

			_shape1 = w1->shape;
			_shape2 = w2->shape;

			if(_shape1 && _shape1->_type == ST_NORMAL) _feature1 = _shape1->features[0];
			if(_shape2 && _shape2->_type == ST_NORMAL) _feature2 = _shape2->features[0];
		}

		slFeature *_feature1;
		slFeature *_feature2;

		slShape *_shape1;
		slShape *_shape2;

		slPosition *_position1;
		slPosition *_position2;

		unsigned int _x;
		unsigned int _y;
};

slCollision *slNextCollision(slVclipData *v);
slCollision *slNextCollision(slVclipData *v, int x, int y);

void slInitBoundSort(slVclipData *v);
void slIsort(slVclipData *vc, std::vector<slBoundSort*> &list, char boundTypeFlag);
void slInitBoundSortList(std::vector<slBoundSort*> &list, slVclipData *v, char boundTypeFlag);

#endif

void slAddCollisionCandidate(slVclipData *d, slPairFlags f, int x, int y);
void slRemoveCollisionCandidate(slVclipData *d, int x, int y);

slPlane *slPositionPlane(slPosition *p, slPlane *p1, slPlane *pt);

int slSphereSphereCheck(slVclipData *vc, int x, int y, slCollision *ce, slPosition *p1, slSphere *s1, slPosition *p2, slSphere *s2);

int slSphereShapeCheck(slVclipData *vc, slFeature **f, int x, int y, slCollision *ce);

int slVclipPruneAndSweep(slVclipData *d);
int slVclip(slVclipData *d, double tolerance, int pruneOnly, int boundingBoxOnly);

int slVclipTestPair(slVclipData *, slCollisionCandidate*, slCollision *);
int slVclipTestPairAllFeatures(slVclipData*, slCollisionCandidate*, slCollision *);

int slClipPoint(slVector *p, slPlane *v, slPosition *vp, int count, int *update, double *dist);
int slClipPointMax(slVector *p, slPlane *v, slPosition *vpos, int vcount, int *update);
int slClipEdge(slEdge *e, slPosition *ep, slPlane *v, slPosition *vp, int vcount, int *sf, int *ef, double *sL, double *eL);
int slClipEdgePoints(slVector *s, slVector *e, slPlane *v, slPosition *vp, int vcount, int *sf, int *ef, double *sL, double *eL);
int slClipFace(slFace *f, slPosition *fp, slPlane *v, slPosition *vp, int count, int *update);

int slPointPointClip(slFeature **nf1, slPosition *p1p, slShape *s1, slFeature **nf2, slPosition *p2p, slShape *s2, slCollision *ce);
int slPointFaceClip(slFeature **nf1, slPosition *pp, slShape *s1, slFeature **nf2, slPosition *fp, slShape *fs, slVclipData *vc, int x, int y, slCollision *ce);
int slEdgePointClip(slFeature **nf1, slPosition *ep, slShape *s1, slFeature **nf2, slPosition *pp, slShape *s2, slCollision *ce);
int slEdgeEdgeClip(slFeature **nf1, slPosition *p1, slShape *s1, slFeature **nf2, slPosition *p2, slShape *s2, slCollision *ce);
int slEdgeFaceClip(slFeature **nf1, slFeature **nf2, slVclipData *v, int x, int y, slCollision *ce);
int slFaceFaceClip(slFeature **nf1, slPosition *f1p, slShape *s1, slFeature **nf2, slPosition *f2p, slShape *s2, slVclipData *v, int x, int y);

double signEdgePlaneDistanceDeriv(slPlane *p, slVector *edgeVector, slVector *lambda);

int slCountFaceCollisionPoints(slCollision *c, slFeature *f1, slFeature *f2, slPosition *p1, slPosition *p2, slShape *s1, slShape *s2);

double slMinPointDist(slFace *f, slPosition *fp, slVector *p, slPosition *pp);

void slEdgeFaceCountCollisionPoints(slCollision *p, slEdge *e, slPosition *ep, slFace *f, slPosition *fp, slShape *s1, slShape *s2);

int slEdgePointOnFace(slEdge *e, slPosition *ep, slFace *f, slPosition *fp, slVector *point);

double slPointLineDist(slVector *p1, slVector *p2, slVector *point, slVector *i);

#endif /* _VCLIP_H */
