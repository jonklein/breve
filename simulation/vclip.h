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

#define MC_TOLERANCE	0.00
#define VC_WARNING_TOLERANCE -0.1

#define slVclipPairEntry(pe, x, y)		((x)>(y)?(&(pe)[(x)][(y)]):(&(pe)[(y)][(x)]))

enum collisionTypes {
	CC_SIMULATE,
	CC_CALLBACK
};

enum clipCodes {
	CT_DISJOINT = 0,
	CT_TOUCH,
	CT_CONTINUE,
	CT_PENETRATE,
	CT_ERROR
};

enum slBoundSortTypes {
	BT_MIN = 0,
	BT_MAX
};

enum boundListTypeFlags {
	BT_XAXIS 	= 0x01, /* 000001 */
	BT_YAXIS 	= 0x02, /* 000010 */
	BT_ZAXIS 	= 0x04, /* 000100 */
	BT_CHECK 	= 0x08, /* 001000 */
	BT_ALL   	= 0x0f, /* 001111 */
	BT_CALLBACK	= 0x10,	/* 010000 */
	BT_SIMULATE = 0x20	/* 100000 */
};

#define slVclipFlagsShouldTest(x)		(((x) & BT_ALL) == BT_ALL)

/*!
	\brief The minima and maxima of the collision detection pruning stage.

	Hold the maxima or minima data for all the objects in the world.  These 
	are sorted with an insertion sort in order to locate overlapping pairs.
*/

struct slBoundSort {
	int number;
	slBoundSort *previousLeft;
	char type;
	char infnan;	// is this a fucked up entry?
	double *value;
};

/*!
	\brief A record of a potential collision between two objects.

	We make n^2 of these, so we'd like to be able to keep them small.
*/

#ifdef __cplusplus
#include <vector>
struct slPairEntry {
	slFeature *f1;
	slFeature *f2;

	long candidateNumber;

	long x;
	long y;

	unsigned char flags;
};

/*!
	\brief A record of a collision.

	When a collision occurs, one of these entries is created.  It
	is used to add a contact point for the collision.
*/

struct slCollisionEntry {
	slVector normal;

	std::vector<slVector> points;
	std::vector<double> depths;

	long n1; 
	long n2;
};

/*! 
	\brief Data associated with the v-clip algorithm.
*/


struct slVclipData {
	std::vector<slBoundSort*> boundListPointers[3];

	std::vector<slBoundSort> boundLists[3];

	slShape **shapes;
	
	std::vector<slWorldObject*> objects;

	std::vector<slCollisionEntry> collisions;

	slPairEntry **pairList;

	std::vector<slPairEntry*> candidates;

	unsigned int count;
	int maxCount;
};

slCollisionEntry *slNextCollisionEntry(slVclipData *v);
slCollisionEntry *slNextCollisionEntry(slVclipData *v, int x, int y);

void slInitBoundSort(slVclipData *v);
void slIsort(slVclipData *vc, std::vector<slBoundSort*> &list, int size, char boundTypeFlag);
void slInitBoundSortList(std::vector<slBoundSort*> &list, int size, slVclipData *v, char boundTypeFlag);

#endif

void slAddCollisionCandidate(slVclipData *d, slPairEntry *e);
void slRemoveCollisionCandidate(slVclipData *d, slPairEntry *e);

slVector *slPositionVertex(slPosition *p, slVector *i, slVector *o);
slPlane *slPositionPlane(slPosition *p, slPlane *p1, slPlane *pt);

int slSphereSphereCheck(slVclipData *vc, int x, int y, slCollisionEntry *ce);

int slSphereShapeCheck(slVclipData *vc, slFeature **f, int flip, int x, int y, slCollisionEntry *ce);

int slVclip(slVclipData *d, double tolerance, int pruneOnly, int boundingBoxOnly);

double slPlaneDistance(slPlane *pl, slVector *p);

int slVclipTestPair(slVclipData *v, slPairEntry *e, slCollisionEntry *ce);
int slVclipTestPairAllFeatures(slVclipData *v, slPairEntry *e, slCollisionEntry *ce);

int slClipPoint(slVector *p, slPlane *v, slPosition *vp, int count, int *update, double *dist);
int slClipPointMax(slVector *p, slPlane *v, slPosition *vpos, int vcount, int *update);
int slClipEdge(slEdge *e, slPosition *ep, slPlane *v, slPosition *vp, int vcount, int *sf, int *ef, double *sL, double *eL);
int slClipEdgePoints(slVector *s, slVector *e, slPlane *v, slPosition *vp, int vcount, int *sf, int *ef, double *sL, double *eL);
int slClipFace(slFace *f, slPosition *fp, slPlane *v, slPosition *vp, int count, int *update);

int slPointPointClip(slFeature **nf1, slPosition *p1p, slShape *s1, slFeature **nf2, slPosition *p2p, slShape *s2, slCollisionEntry *ce);
int slPointFaceClip(slFeature **nf1, slPosition *pp, slShape *s1, slFeature **nf2, slPosition *fp, slShape *fs, slVclipData *vc, int pairFlip, int x, int y, slCollisionEntry *ce);
int slEdgePointClip(slFeature **nf1, slPosition *ep, slShape *s1, slFeature **nf2, slPosition *pp, slShape *s2, slCollisionEntry *ce);
int slEdgeEdgeClip(slFeature **nf1, slPosition *p1, slShape *s1, slFeature **nf2, slPosition *p2, slShape *s2, slCollisionEntry *ce);
int slEdgeFaceClip(slFeature **nf1, slFeature **nf2, slVclipData *v, int pairFlip, int x, int y, slCollisionEntry *ce);
int slFaceFaceClip(slFeature **nf1, slPosition *f1p, slShape *s1, slFeature **nf2, slPosition *f2p, slShape *s2, slVclipData *v, int x, int y);

double signEdgePlaneDistanceDeriv(slPlane *p, slVector *edgeVector, slVector *lambda);

int slCountFaceCollisionPoints(slCollisionEntry *c, slFeature *f1, slFeature *f2, slPosition *p1, slPosition *p2, slShape *s1, slShape *s2);

double slFeatureDistance(slPairEntry *fp, slPosition *p1, slPosition *p2);
double slMinPointDist(slFace *f, slPosition *fp, slVector *p, slPosition *pp);

void slEdgeFaceCountCollisionPoints(slCollisionEntry *p, int pairFlip, slEdge *e, slPosition *ep, slFace *f, slPosition *fp, slShape *s1, slShape *s2);

int slEdgePointOnFace(slEdge *e, slPosition *ep, slFace *f, slPosition *fp, slVector *point);

double slPointLineDist(slVector *p1, slVector *p2, slVector *point, slVector *i);
