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

enum jointTypes {
	JT_REVOLUTE = 1,
	JT_PRISMATIC,
	JT_BALL,
	JT_UNIVERSAL,
	JT_FIX
};

/*!
	\brief A logical collection of attached links.
*/

struct slMultibody { 
	slWorld *world;
	struct slLink *root;
	slList *linkList;

	char *label;

	double mass;

	unsigned char handleSelfCollisions;

	unsigned int linkCount;

	void *callbackData;
};

/*!
	\brief A joint connecting two links.
*/

struct slJoint {
	slLink *parent;
	slLink *child;
	dJointID odeJointID;
	dJointID odeMotorID;

	double kDamp;
	double kSpring;
	double sMax;
	double sMin;
	double torque;
	double targetSpeed;

	unsigned char type;
	unsigned char isMbJoint;
	int vectorOffset;

	void *callbackData;
};

slMultibody *slMultibodyNew(slWorld *w);
void slMultibodySetRoot(slMultibody *m, slLink *root);

void slJointSetLimits(slJoint *joint, slVector *min, slVector *max);

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

void slJointGetVelocity(slJoint *m, slVector *v);
void slJointSetVelocity(slJoint *j, slVector *speed);

void slJointGetPosition(slJoint *m, slVector *v);

int slJointSetNormal(slJoint *joint, slVector *normal);
int slJointSetLinkPoints(slJoint *joint, slVector *plinkPoint, slVector *clinkPoint, double rotation[3][3]);

void slJointSetMaxTorque(slJoint *joint, double max);

slLink *slJointBreak(slJoint *joint);
void slJointDestroy(slJoint *joint);

void slMultibodyUpdate(slMultibody *root);

slMultibody *slLinkFindMultibody(slLink *root);

slList *slLinkList(slLink *root, slList *list, int mbOnly);

void slMultibodyRotate(slMultibody *m, double rotation[3][3]);

void slMultibodyUpdatePositions(slMultibody *m);
void slLinkUpdatePositions(slLink *l);

void slSlToODEMatrix(double m[3][3], dReal *r);
void slODEToSlMatrix(dReal *r, double m[3][3]);

void slNullOrphanMultibodies(slLink *orphan);

int slMultibodyCheckSelfPenetration(slWorld *w, slMultibody *m);

