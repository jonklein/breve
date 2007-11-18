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
#include "world.h"

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
	BT_SIMULATE 	= 0x20,	// 0100000 -- these objects require simulation
	BT_UNKNOWN	= 0x40	// 1000000 -- need to check callback & simulation
};

#define slVclipFlagsShouldTest(x)		(((x) & BT_ALL) == BT_ALL)

/**
 * The minima and maxima of the collision detection pruning stage.
 *
 * Hold the maxima or minima data for all the objects in the world.  These 
 * are sorted with an insertion sort in order to locate overlapping pairs.
 */

struct slBoundSort {
	unsigned int number;
	char type;
	double *value;
};

#ifdef __cplusplus
#include <vector>
#include <map>

/**
 * \brief A record of a collision.
 * 
 * When a collision occurs, one of these entries is created.  It
 * is used to add a contact point for the collision.
 */

#define MAX_ODE_CONTACTS	128

class slCollision {
	public:
									slCollision() {
										_contactPoints = 0;
									}
		slVector normal;

		std::vector<slVector> points;
		std::vector<double> depths;

		unsigned int n1; 
		unsigned int n2;

		dContactGeom 				_contactGeoms[ MAX_ODE_CONTACTS ];
		int							_contactPoints;
};

/**
 * \brief Data associated with the v-clip algorithm.
 */

class slVclipData {
	public:
		slVclipData();
		~slVclipData();

		void realloc( int count );

		/**
 		 * Runs the prune and sweep phase of the collision detection.
		 */

		int pruneAndSweep();

		/**
		 * Runs collision detection for all candidate pairs.
		 */

		int clip(double tolerance, int pruneOnly, int boundingBoxOnly);

		void addBoundingBoxPointers( int offset, slVector *min, slVector *max );

		/**
		 * Tests a single pair stored in a \ref slCollisionCandidate.
		 */

		int testPair(slCollisionCandidate *e, slCollision *ce);

		slWorld *world;

		std::vector<slBoundSort*> boundListPointers[3];
		std::vector<slBoundSort> boundLists[3];

		std::vector<slCollision> collisions;
		unsigned int collisionCount;

		slPairFlags **pairArray;

		std::map< slPairFlags* , slCollisionCandidate > candidates;

		unsigned int _count;
		unsigned int _maxCount;

		slPairFlags initPairFlags( int, int );

};

class slCollisionCandidate {
	public:
		slCollisionCandidate() { 
			_shape1 = NULL; 
			_shape2 = NULL; 
			_feature1 = NULL; 
			_feature2 = NULL; 
		};

		slCollisionCandidate(slVclipData *vc, int o1, int o2) {
			slWorldObject *w1, *w2;

			_x = o1; _y = o2;

			w1 = vc->world->_objects[ _x ];
			w2 = vc->world->_objects[ _y ];

			_position1 = &w1->getPosition();	
			_position2 = &w2->getPosition();	

			if( w1 && w2 ) {
				_shape1 = w1->getShape();
				_shape2 = w2->getShape();

				if( _shape1 && _shape1->features.size() != 0 ) _feature1 = _shape1->features[0];
				else _feature1 = NULL;

				if( _shape2 && _shape2->features.size() != 0 ) _feature2 = _shape2->features[0];
				else _feature2 = NULL;
			}
		}

		slFeature *_feature1;
		slFeature *_feature2;

		const slShape *_shape1;
		const slShape *_shape2;

		const slPosition *_position1;
		const slPosition *_position2;

		unsigned int _x;
		unsigned int _y;
};

slCollision *slNextCollision( slVclipData *v );
slCollision *slNextCollision( slVclipData *v, int x, int y );

void slInitBoundSort(slVclipData *v);
void slIsort(slVclipData *vc, std::vector<slBoundSort*> &list, char boundTypeFlag);
void slInitBoundSortList(std::vector<slBoundSort*> &list, slVclipData *v, char boundTypeFlag);

#endif

void slAddCollisionCandidate(slVclipData *d, slPairFlags f, int x, int y);
void slRemoveCollisionCandidate(slVclipData *d, int x, int y);

slPlane *slPositionPlane( const slPosition *p, const slPlane *p1, slPlane *pt );

int slSphereSphereCheck(slVclipData *vc, int x, int y, slCollision *ce, const slPosition *p1, slSphere *s1, const slPosition *p2, slSphere *s2);

int slSphereShapeCheck(slVclipData *vc, slFeature **f, int x, int y, slCollision *ce);

int slVclip(slVclipData *d, double tolerance, int pruneOnly, int boundingBoxOnly);

int slVclipTestPair(slVclipData *, slCollisionCandidate*, slCollision *);
int slVclipTestPairAllFeatures(slVclipData*, slCollisionCandidate*, slCollision *);

int slClipPoint( const slVector *p, const slPlane *v, const slPosition *vp, int count, int *update, double *dist );
int slClipPointMax( const slVector *p, const slPlane *v, const slPosition *vpos, int vcount, int *update, double *dist );
int slClipEdge( slEdge *e, const slPosition *ep, slPlane *v, const slPosition *vp, int vcount, int *sf, int *ef, double *sL, double *eL );
int slClipEdgePoints( slVector *s, slVector *e, slPlane *v, const slPosition *vp, int vcount, int *sf, int *ef, double *sL, double *eL );
int slClipFace( slFace *f, const slPosition *fp, slPlane *v, const slPosition *vp, int count, int *update );

int slPointPointClip(slFeature **nf1, const slPosition *p1p, const slShape *s1, slFeature **nf2, const slPosition *p2p, const slShape *s2, slCollision *ce);
int slPointFaceClip(slFeature **nf1, const slPosition *pp, const slShape *s1, slFeature **nf2, const slPosition *fp, const slShape *fs, slVclipData *vc, int x, int y, slCollision *ce);
int slEdgePointClip(slFeature **nf1, const slPosition *ep, const slShape *s1, slFeature **nf2, const slPosition *pp, const slShape *s2, slCollision *ce);
int slEdgeEdgeClip(slFeature **nf1, const slPosition *p1, const slShape *s1, slFeature **nf2, const slPosition *p2, const slShape *s2, slCollision *ce);
int slEdgeFaceClip(slFeature **nf1, slFeature **nf2, slVclipData *v, int x, int y, slCollision *ce);
int slFaceFaceClip(slFeature **nf1, const slPosition *f1p, const slShape *s1, slFeature **nf2, const slPosition *f2p, const slShape *s2, slVclipData *v, int x, int y);

double signEdgePlaneDistanceDeriv(slPlane *p, slVector *edgeVector, slVector *lambda);

int slCountFaceCollisionPoints(slCollision *c, slFeature *f1, slFeature *f2, const slPosition *p1, const slPosition *p2, const slShape *s1, const slShape *s2);

double slMinPointDist(slFace *f, const slPosition *fp, slVector *p, const slPosition *pp);

void slEdgeFaceCountCollisionPoints(slCollision *p, slEdge *e, const slPosition *ep, slFace *f, const slPosition *fp, const slShape *s1, const slShape *s2);

int slEdgePointOnFace(slEdge *e, const slPosition *ep, slFace *f, const slPosition *fp, slVector *point);

double slPointLineDist( const slVector *p1, const slVector *p2, const slVector *point, slVector *i);

#endif /* _VCLIP_H */
