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

#include "ode/ode.h"

enum jointTypes {
	JT_REVOLUTE = 1,
	JT_PRISMATIC,
	JT_BALL,
	JT_UNIVERSAL,
	JT_FIX
};

class slLink;

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

	void *userData;
};

#ifdef __cplusplus
extern "C" {
#endif
void slJointGetVelocity(slJoint *m, slVector *v);
void slJointSetVelocity(slJoint *j, slVector *speed);
void slJointGetPosition(slJoint *m, slVector *v);
int slJointSetNormal(slJoint *joint, slVector *normal);
int slJointSetLinkPoints(slJoint *joint, slVector *plinkPoint, slVector *clinkPoint, double rotation[3][3]);
void slJointSetMaxTorque(slJoint *joint, double max);
void slJointSetLimits(slJoint *joint, slVector *min, slVector *max);
void slJointBreak(slJoint *joint);
void slJointFree(slJoint *joint);
void slJointApplyTorque(slJoint *j, slVector *torque);

#ifdef __cplusplus
}
#endif
