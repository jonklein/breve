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

#ifdef __cplusplus
#include <vector>
#include <algorithm>

/*!
	\brief A logical collection of attached links.
*/

struct slMultibody { 
	slWorld *world;
	struct slLink *root;
	slList *linkList;

	std::vector<slLink*> links;

	char *label;

	double mass;

	unsigned char handleSelfCollisions;

	unsigned int linkCount;

	void *callbackData;
};

void slLinkList(slLink *root, std::vector<slLink*> *list, int mbOnly);

#endif

#ifdef __cplusplus
extern "C"{
#endif
slMultibody *slMultibodyNew(slWorld *w);
void slMultibodySetRoot(slMultibody *m, slLink *root);

void slMultibodyInitCollisionFlags(slMultibody *m, slPairEntry **pe);

int slInitShapeList(slShape **slist, slPosition **plist, slMultibody *root);

slList *slMultibodyAllCallbackData(slMultibody *mb);

void slMultibodyFree(slMultibody *m);

void slMultibodyPosition(slMultibody *m, slVector *location, double rotation[3][3]);
void slMultibodyRotAngleToMatrix(slVector *axis, double r, double rot[3][3]);

void slMultibodyOffsetPosition(slMultibody *m, slVector *offset);

void slMultibodySetVelocity(slMultibody *m, slVector *linear, slVector *rotational);
void slLinkSetVelocity(slLink *m, slVector *linear, slVector *rotational);

void slLinkGetVelocity(slLink *m, slVector *velocity, slVector *rotational);

void slMultibodySetAcceleration(slMultibody *m, slVector *linear, slVector *rotational);
void slLinkSetAcceleration(slLink *m, slVector *velocity, slVector *rotational);

int slMultibodyCountLinks(slMultibody *m);
double slMultibodyComputeMass(slMultibody *m);

void slMultibodyUpdate(slMultibody *root);

slMultibody *slLinkFindMultibody(slLink *root);

void slMultibodyRotate(slMultibody *m, double rotation[3][3]);

void slMultibodyUpdatePositions(slMultibody *m);
void slLinkUpdatePositions(slLink *l);

void slSlToODEMatrix(double m[3][3], dReal *r);
void slODEToSlMatrix(dReal *r, double m[3][3]);

void slNullOrphanMultibodies(slLink *orphan);

int slMultibodyCheckSelfPenetration(slWorld *w, slMultibody *m);
void *slMultibodyGetCallbackData(slMultibody *m);
void slMultibodySetCallbackData(slMultibody *m, void *c);

void slMultibodySetHandleSelfCollisions(slMultibody *m, int n);

#ifdef __cplusplus
}
#endif

