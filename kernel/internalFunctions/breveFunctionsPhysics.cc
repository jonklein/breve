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

#include "kernel.h"

/*! \addtogroup InternalFunctions */
/*@{*/

/*!
	\brief Set the acceleration of the world's gravity.

	void setGravity(vector acceleration).
*/

int brIWorldSetGravity(brEval args[], brEval *target, brInstance *i) {
	slVector *v = &BRVECTOR(&args[0]);
 
	slWorldSetGravity(i->engine->world, v);
	
	return EC_OK;
}   

/*!
	\brief Generate a rotation matrix from a vector axis and a rotation angle
	about that axis.

	matrix rotationMatrix(vector axis, double rotation).
*/

int brIRotationMatrix(brEval args[], brEval *target, brInstance *i) {
	slVector *vector = &BRVECTOR(&args[0]);
	double rotation = BRDOUBLE(&args[1]);

	slRotationMatrix(vector, rotation, BRMATRIX(target));

	return EC_OK;
}

/*!
	\brief Applies torque to a joint.

	void jointApplyTorque(slJoint pointer joint, vector torque).

	Since the joint may be 1, 2 or 3 DOF, only some elements of 
	the torque vector may be used.
*/

int brIJointApplyTorque(brEval args[], brEval *target, brInstance *i) {
	slJoint *j = BRPOINTER(&args[0]);
	slVector *tVector = &BRVECTOR(&args[1]);
	double torque = tVector->x;
	slVector t;
	dVector3 axis;

	if(!j) {
		slMessage(DEBUG_ALL, "jointApplyTorque failed\n");
		return EC_ERROR;
	}

	dJointGetHingeAxis(j->odeJointID, axis);

	t.x = axis[0] * torque;
	t.y = axis[1] * torque;
	t.z = axis[2] * torque;


	if(j->parent) dBodySetTorque(j->parent->odeBodyID, t.x, t.y, t.z);
	if(j->child) dBodySetTorque(j->child->odeBodyID, -t.x, -t.y, -t.z);

	return EC_OK;
}

/*!
	\brief Create a revolute joint between two bodies.

	slJoint pointer jointLinkRevolute(slLink pointer parent, slLink pointer child
									vector normal, vector parentLinkPoint,
									vector childLinkPoint).
*/

int brJointILinkRevolute(brEval args[], brEval *target, brInstance *i) {
	slLink *parent = BRPOINTER(&args[0]);
	slLink *child = BRPOINTER(&args[1]);
	slVector *normal = &BRVECTOR(&args[2]);
	slVector *ppoint = &BRVECTOR(&args[3]);
	slVector *cpoint = &BRVECTOR(&args[4]);
	slJoint *joint;

	if(!child) {
		slMessage(DEBUG_ALL, "NULL pointer passed to jointLinkRevolute\n");
		return EC_ERROR;
	}

	joint = slLinkLinks(i->engine->world, parent, child, JT_REVOLUTE, normal, ppoint, cpoint, BRMATRIX(&args[5]));
	joint->callbackData = i;

	if(!joint) {
		slMessage(DEBUG_ALL, "error creating joint: jointLinkRevolute failed\n");
		return EC_ERROR;
	}

	i->engine->world->initialized = 0;

	BRPOINTER(target) = joint;
	target->type = AT_POINTER;
	
	return EC_OK;
}   

/*!
	\brief Modify the normal vector for a joint.

	void jointSetNormal(slJoint pointer joint, vector normal).
*/

int brIJointSetNormal(brEval args[], brEval *target, brInstance *i) {
	slJoint *j = BRPOINTER(&args[0]);
	slVector *n = &BRVECTOR(&args[1]);

	if(!j) {
		slMessage(DEBUG_ALL, "jointSetNormal called with uninitialized joint\n");
		return EC_ERROR;
	}

	slJointSetNormal(j, n);

	return EC_OK;
}

/*!
	\brief Modify the link points for a joint.

	void jointSetLinkPoints(slJoint pointer joint, vector parentPoint, vector childPoint).
*/

int brIJointSetLinkPoints(brEval args[], brEval *target, brInstance *i) {
	slJoint *j = BRPOINTER(&args[0]);
	slVector *n1 = &BRVECTOR(&args[1]);
	slVector *n2 = &BRVECTOR(&args[2]);
	double rot[3][3];

	slMatrixCopy(BRMATRIX(&args[3]), rot);

	if(!j) {
		slMessage(DEBUG_ALL, "jointSetLinkPoints called with uninitialized joint\n");
		return EC_ERROR;
	}

	slJointSetLinkPoints(j, n1, n2, rot);

	return EC_OK;
}

/*!
	\brief Create a revolute joint between two bodies.

	slJoint pointer jointLinkRevolute(slLink pointer parent, 
									slLink pointer child
									vector normal,
									vector parentLinkPoint,
									vector childLinkPoint).
*/

int brJointILinkPrismatic(brEval args[], brEval *target, brInstance *i) {
	slLink *parent = BRPOINTER(&args[0]);
	slLink *child = BRPOINTER(&args[1]);
	slVector *normal = &BRVECTOR(&args[2]);
	slVector *ppoint = &BRVECTOR(&args[3]);
	slVector *cpoint = &BRVECTOR(&args[4]);
	slJoint *joint;

	if(!child) {
		slMessage(DEBUG_ALL, "NULL pointer passed to jointLinkPrismatic\n");
		return EC_ERROR;
	}
 
	joint = slLinkLinks(i->engine->world, parent, child, JT_PRISMATIC, normal, ppoint, cpoint, BRMATRIX(&args[5]));

	if(!joint) {
		slMessage(DEBUG_ALL, "error creating joint: jointLinkPrismatic failed\n");
		return EC_ERROR;
	}

	i->engine->world->initialized = 0;

	BRPOINTER(target) = joint;
	target->type = AT_POINTER;
	
	return EC_OK;
}

/*!
	\brief Create a ball joint between two bodies.

	slJoint pointer jointLinkBall(slLink pointer parent, slLink pointer child
									vector normal, vector parentLinkPoint,
									vector childLinkPoint).
*/

int brJointILinkBall(brEval args[], brEval *target, brInstance *i) {
	slLink *parent = BRPOINTER(&args[0]);
	slLink *child = BRPOINTER(&args[1]);
	slVector *normal = &BRVECTOR(&args[2]);
	slVector *ppoint = &BRVECTOR(&args[3]);
	slVector *cpoint = &BRVECTOR(&args[4]);
	slJoint *joint;

	if(!child) {
		slMessage(DEBUG_ALL, "NULL pointer passed to jointLinkBall\n");
		return EC_ERROR;
	}
 
	joint = slLinkLinks(i->engine->world, parent, child, JT_BALL, normal, ppoint, cpoint, BRMATRIX(&args[5]));

	if(!joint) {
		slMessage(DEBUG_ALL, "error creating joint: jointLinkBall failed\n");
		return EC_ERROR;
	}

	i->engine->world->initialized = 0;

	BRPOINTER(target) = joint;
	target->type = AT_POINTER;
	
	return EC_OK;
}

/*!
	\brief Create a universal joint between two bodies.

	slJoint pointer jointLinkUniversal(slLink pointer parent, slLink pointer child
									vector normal, vector parentLinkPoint,
									vector childLinkPoint).
*/

int brJointILinkUniversal(brEval args[], brEval *target, brInstance *i) {
	slLink *parent = BRPOINTER(&args[0]);
	slLink *child = BRPOINTER(&args[1]);
	slVector *normal = &BRVECTOR(&args[2]);
	slVector *ppoint = &BRVECTOR(&args[3]);
	slVector *cpoint = &BRVECTOR(&args[4]);
	slJoint *joint;

	if(!child) {
		slMessage(DEBUG_ALL, "NULL pointer passed to jointLinkUniversal\n");
		return EC_ERROR;
	}
 
	joint = slLinkLinks(i->engine->world, parent, child, JT_UNIVERSAL, normal, ppoint, cpoint, BRMATRIX(&args[5]));

	if(!joint) {
		slMessage(DEBUG_ALL, "error creating joint: jointLinkUniversal\n");
		return EC_ERROR;
	}

	i->engine->world->initialized = 0;

	BRPOINTER(target) = joint;
	target->type = AT_POINTER;
	
	return EC_OK;
}

/*!
	\brief Create a static joint between two bodies.

	slJoint pointer jointLinkStatic(slLink pointer parent, slLink pointer child
									vector normal, vector parentLinkPoint,
									vector childLinkPoint).

	The normal vector is currently unused.
*/

int brJointILinkStatic(brEval args[], brEval *target, brInstance *i) {
	slLink *parent = BRPOINTER(&args[0]);
	slLink *child = BRPOINTER(&args[1]);
	slVector *normal = &BRVECTOR(&args[2]);
	slVector *ppoint = &BRVECTOR(&args[3]);
	slVector *cpoint = &BRVECTOR(&args[4]);
	slJoint *joint;

	if(!child) {
		slMessage(DEBUG_ALL, "NULL pointer passed to jointLinkStatic\n");
		return EC_ERROR;
	}
 
	joint = slLinkLinks(i->engine->world, parent, child, JT_FIX, normal, ppoint, cpoint, BRMATRIX(&args[5]));

	if(!joint) {
		slMessage(DEBUG_ALL, "error creating joint: jointLinkStatic failed\n");
		return EC_ERROR;
	}

	i->engine->world->initialized = 0;

	BRPOINTER(target) = joint;
	target->type = AT_POINTER;
	
	return EC_OK;
}

/*!
	\brief Break a joint.

	void jointBreak(slJoint pointer).
*/

int brIJointBreak(brEval args[], brEval *target, brInstance *i) {
	slJoint *joint = BRPOINTER(&args[0]);

	if(!joint) {
		slMessage(DEBUG_ALL, "NULL pointer passed to breakJoint\n");
		return EC_ERROR;
	}

	slJointBreak(joint);
	slJointDestroy(joint);
	i->engine->world->initialized = 0;

	return EC_OK;
}

/*!
	\brief Turn collision resolution on or off.

	void worldSetCollisionResolution(int state).
*/

int brIWorldSetCollisionResolution(brEval args[], brEval *target, brInstance *i) {
	i->engine->world->resolveCollisions = BRINT(&args[0]);
	
	target->type = AT_NULL;
	
	return EC_OK;
}

/*!
	\brief Set the collision properties (mu, e, eT) of a stationary object.

	void setStationaryCollisionProperties(slWorldObject pointer stationary, double e, double eT, double mu).
*/

int brISetCollisionProperties(brEval args[], brEval *target, brInstance *i) {
	slWorldObject *o = BRPOINTER(&args[0]);
	
	double e = BRDOUBLE(&args[1]);
	double eT = BRDOUBLE(&args[2]);
	double mu = BRDOUBLE(&args[3]);

	if(!o) {
		slMessage(DEBUG_ALL, "null pointer passed to setCollisionProperties\n");
		return EC_ERROR;
	}

	o->e = e;
	o->eT = eT;
	o->mu = mu;

	return EC_OK;
}

/*!
	\brief Return the distance of this joint from its natural position.

	vector getJointPosition(slJoint pointer).

	Since the joint may be 1, 2 or 3 DOF, not all of the vector components may be filled in.
*/

int brIJointGetPosition(brEval args[], brEval *target, brInstance *i) {
	slJoint *j = BRPOINTER(&args[0]);

	if(!j) {
		slMessage(DEBUG_ALL, "NULL pointer passed to getJointPosition\n");
		return EC_ERROR;
	}

	slJointGetPosition(j, &BRVECTOR(target));

	return EC_OK;
}

/*!
	\brief Sets the velocity of this joint.

	void jointSetVelocity(slJoint pointer joint, vector velocity).

	Since the joint may be 1, 2 or 3 DOF, some of the elements of velocity may
	not be used.
*/

int brIJointSetVelocity(brEval args[], brEval *target, brInstance *i) {
	slJoint *j = BRPOINTER(&args[0]);
	slVector *velocity = &BRVECTOR(&args[1]);

	if(!j) return EC_OK;

	slJointSetVelocity(j, velocity);

	return EC_OK;
}

/*!
	\brief Gets the velocity of this joint.

	vector jointGetVelocity(slJoint pointer joint).

	Since the joint may be 1, 2 or 3 DOF, some of the elements of velocity may
	not be set.
*/

int brIJointGetVelocity(brEval args[], brEval *target, brInstance *i) {
	slJoint *j = BRPOINTER(&args[0]);

	if(!j) {
		slMessage(DEBUG_ALL, "NULL pointer passed to getJointVelocity\n");
		return EC_ERROR;
	}

	slJointGetVelocity(j, &BRVECTOR(target));

	return EC_OK;
}

/*!
	\brief Set joint damping.

	jointSetDamping(slJoint pointer joint, double damping).
*/

int brIJointSetDamping(brEval args[], brEval *target, brInstance *i) {
	slJoint *j = BRPOINTER(&args[0]);
	double value = BRDOUBLE(&args[1]);

	if(!j) {
		slMessage(DEBUG_ALL, "NULL pointer passed to jointSetDamping\n");
		return EC_ERROR;
	}

	j->kDamp = value;

	return EC_OK;
}

/*!
	\brief Set the joint spring.

	void jointSetSpring(slJoint pointer joint, double strength, double min, double max).
*/

int brIJointSetSpring(brEval args[], brEval *target, brInstance *i) {
	slJoint *j = BRPOINTER(&args[0]);
	double strength = BRDOUBLE(&args[1]);
	double min = BRDOUBLE(&args[2]);
	double max = BRDOUBLE(&args[3]);

	if(!j) {
		slMessage(DEBUG_ALL, "NULL pointer passed to jointSetSpring\n");
		return EC_ERROR;
	}

	j->kSpring = strength;
	j->sMin = min;
	j->sMax = max;

	return EC_OK;
}

/*!
	\brief Sets the motion limits for this joint.

	void jointSetLimits(slJoint pointer joint, vector min, vector max).

	Since the joint may be 1, 2 or 3 DOF, some of the elements of velocity may
	not be used.
*/

int brIJointSetLimits(brEval args[], brEval *target, brInstance *i) {
	slJoint *j = BRPOINTER(&args[0]);
	slVector *min = &BRVECTOR(&args[1]);
	slVector *max = &BRVECTOR(&args[2]);

	if(!j) {
		slMessage(DEBUG_ALL, "NULL pointer passed to jointSetLimits\n");
		return EC_ERROR;
	}

	slJointSetLimits(j, min, max);

	return EC_OK;
}

/*!
	\brief Sets the maximum strength for this joint.

	jointSetMaxStrength(slJoint pointer joint, double max torque).
*/

int brIJointSetMaxStrength(brEval args[], brEval *target, brInstance *i) {
	slJoint *j = BRPOINTER(&args[0]);
	double strength = BRDOUBLE(&args[1]);
	
	slJointSetMaxTorque(j, strength);

	return EC_OK;
}

/*@}*/

void breveInitPhysicsFunctions(brNamespace *n) {
	brNewBreveCall(n, "worldSetCollisionResolution", brIWorldSetCollisionResolution, AT_NULL, AT_INT, 0);
	brNewBreveCall(n, "worldSetGravity", brIWorldSetGravity, AT_NULL, AT_VECTOR, 0);

	brNewBreveCall(n, "rotationMatrix", brIRotationMatrix, AT_MATRIX, AT_VECTOR, AT_DOUBLE, 0);

	brNewBreveCall(n, "setCollisionProperties", brISetCollisionProperties, AT_NULL, AT_POINTER, AT_DOUBLE, AT_DOUBLE, AT_DOUBLE, 0);

	brNewBreveCall(n, "jointApplyTorque", brIJointApplyTorque, AT_NULL, AT_POINTER, AT_DOUBLE, 0);

	brNewBreveCall(n, "jointLinkRevolute", brJointILinkRevolute, AT_POINTER, AT_POINTER, AT_POINTER, AT_VECTOR, AT_VECTOR, AT_VECTOR, AT_MATRIX, 0);
	brNewBreveCall(n, "jointLinkPrismatic", brJointILinkPrismatic, AT_POINTER, AT_POINTER, AT_POINTER, AT_VECTOR, AT_VECTOR, AT_VECTOR, AT_MATRIX, 0);
	brNewBreveCall(n, "jointLinkBall", brJointILinkBall, AT_POINTER, AT_POINTER, AT_POINTER, AT_VECTOR, AT_VECTOR, AT_VECTOR, AT_MATRIX, 0);
	brNewBreveCall(n, "jointLinkStatic", brJointILinkStatic, AT_POINTER, AT_POINTER, AT_POINTER, AT_VECTOR, AT_VECTOR, AT_VECTOR, AT_MATRIX, 0);
	brNewBreveCall(n, "jointLinkUniversal", brJointILinkUniversal, AT_POINTER, AT_POINTER, AT_POINTER, AT_VECTOR, AT_VECTOR, AT_VECTOR, AT_MATRIX, 0);

	brNewBreveCall(n, "jointBreak", brIJointBreak, AT_NULL, AT_POINTER, 0);
	brNewBreveCall(n, "jointSetVelocity", brIJointSetVelocity, AT_NULL, AT_POINTER, AT_VECTOR, 0);
	brNewBreveCall(n, "jointSetDamping", brIJointSetDamping, AT_NULL, AT_POINTER, AT_DOUBLE, 0);
	brNewBreveCall(n, "jointSetLimits", brIJointSetLimits, AT_NULL, AT_POINTER, AT_VECTOR, AT_VECTOR, 0);
	brNewBreveCall(n, "jointSetSpring", brIJointSetSpring, AT_NULL, AT_POINTER, AT_DOUBLE, AT_DOUBLE, AT_DOUBLE, 0);
	brNewBreveCall(n, "jointSetMaxStrength", brIJointSetMaxStrength, AT_NULL, AT_POINTER, AT_DOUBLE, 0);
	brNewBreveCall(n, "jointSetLinkPoints", brIJointSetLinkPoints, AT_NULL, AT_POINTER, AT_VECTOR, AT_VECTOR, AT_MATRIX, 0);
	brNewBreveCall(n, "jointSetNormal", brIJointSetNormal, AT_NULL, AT_POINTER, AT_VECTOR, 0);

	brNewBreveCall(n, "jointGetPosition", brIJointGetPosition, AT_VECTOR, AT_POINTER, 0);
	brNewBreveCall(n, "jointGetVelocity", brIJointGetVelocity, AT_VECTOR, AT_POINTER, 0);

}
