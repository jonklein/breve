#include "simulation.h"

// is this really correct?

void slJointApplyTorque(slJoint *j, slVector *torque) {
	slVector t;
	dVector3 axis;

	dJointGetHingeAxis(j->odeJointID, axis);

	t.x = axis[0] * torque->x;
	t.y = axis[1] * torque->x;
	t.z = axis[2] * torque->x;

	if(j->parent) dBodySetTorque(j->parent->odeBodyID, t.x, t.y, t.z);
   	if(j->child) dBodySetTorque(j->child->odeBodyID, -t.x, -t.y, -t.z);
}

/*!
	\brief Frees an slJoint struct.
*/

void slJointFree(slJoint *joint) {
	slJointBreak(joint);
	delete joint;
}

/*!
	\brief Breaks an slJoint struct.

	This triggers an automatic recomputation of multibodies.
*/

void slJointBreak(slJoint *joint) {
	slLink *parent = joint->parent, *child = joint->child;
	slMultibody *parentBody = NULL, *childBody, *newMb;
	std::vector<slJoint*>::iterator ji;

	if(!parent && !child) return;

	childBody = joint->child->multibody;

	if(parent) parentBody = parent->multibody;

	if(parent) {
		ji = std::find(parent->outJoints.begin(), parent->outJoints.end(), joint);
		if(ji != parent->outJoints.end()) parent->outJoints.erase(ji);
	}

	ji = std::find(child->inJoints.begin(), child->inJoints.end(), joint);
	if(ji != child->inJoints.end()) child->inJoints.erase(ji);

	dJointAttach(joint->odeJointID, NULL, NULL);
	dJointDestroy(joint->odeJointID);

	if(joint->odeMotorID) {
		dJointAttach(joint->odeMotorID, NULL, NULL);
		dJointDestroy(joint->odeMotorID);
	}

	joint->child = NULL;
	joint->parent = NULL;

	if(parentBody) slMultibodyUpdate(parentBody);
	if(childBody && childBody != parentBody) slMultibodyUpdate(childBody);

	// figure out if the broken links are still part of those bodies
	// ... if not, then try to adopt the links 
	// ... if not, then NULL the multibody entries 

	if(parent && parentBody && (std::find(parentBody->links.begin(), parentBody->links.end(), parent) != parentBody->links.end())) {
		if((newMb = slLinkFindMultibody(parent))) slMultibodyUpdate(newMb);
		else slNullOrphanMultibodies(parent);
	}

	if(child && childBody && (std::find(childBody->links.begin(), childBody->links.end(), child) != childBody->links.end())) {
		if((newMb = slLinkFindMultibody(child))) slMultibodyUpdate(newMb);
		else slNullOrphanMultibodies(child);
	}
}

/*!
	\brief Gives the position of a joint, relative to its native position.
*/

void slJointGetPosition(slJoint *j, slVector *r) {
	switch(j->type) {
		case JT_REVOLUTE:
			r->x = dJointGetHingeAngle(j->odeJointID);
			r->y = 0;
			r->z = 0;
			break;
		case JT_PRISMATIC:
			r->x = dJointGetSliderPosition(j->odeJointID);
			r->y = 0;
			r->z = 0;
			break;
		case JT_BALL:
		case JT_UNIVERSAL:
			r->x = dJointGetAMotorAngle(j->odeMotorID, dParamVel);
			r->y = dJointGetAMotorAngle(j->odeMotorID, dParamVel2);
			r->z = dJointGetAMotorAngle(j->odeMotorID, dParamVel3);
			break;
		default:
			break;
	}

	return;
}

/*!
	\brief Gets the joint velocity of a joint.

	Returns the 1-DOF value for backwards compatability, but also
	fills in the velocity vector.
*/

void slJointGetVelocity(slJoint *j, slVector *velocity) {
	switch(j->type) {
		case JT_REVOLUTE:
			velocity->x = dJointGetHingeAngleRate(j->odeJointID);
			break;
		case JT_PRISMATIC:
			velocity->x = dJointGetSliderPositionRate(j->odeJointID);
			break;
		case JT_BALL:
			velocity->z = dJointGetAMotorParam(j->odeMotorID, dParamVel3);
		case JT_UNIVERSAL:
			velocity->x = dJointGetAMotorParam(j->odeMotorID, dParamVel);
			velocity->y = dJointGetAMotorParam(j->odeMotorID, dParamVel2);
			break;
	}
}

/*!
	\brief Sets the velocity of a joint.

	The joint may be 1-, 2- or 3DOF, so only the relevant fields 
	of the speed vector are used.
*/

void slJointSetVelocity(slJoint *j, slVector *speed) {
	j->targetSpeed = speed->x;

	// if(j->type == JT_REVOLUTE) dJointSetHingeParam (j->odeJointID, dParamVel, speed->x);
	// else if(j->type == JT_PRISMATIC) dJointSetSliderParam (j->odeJointID, dParamVel, speed->x);

	if(j->type == JT_UNIVERSAL) {
		dJointSetAMotorParam(j->odeMotorID, dParamVel, speed->x);
		dJointSetAMotorParam(j->odeMotorID, dParamVel3, speed->y);
	} else if(j->type == JT_BALL) {
		dJointSetAMotorParam(j->odeMotorID, dParamVel, speed->x);
		dJointSetAMotorParam(j->odeMotorID, dParamVel2, speed->y);
		dJointSetAMotorParam(j->odeMotorID, dParamVel3, speed->z);
	}
}

/*!
	\brief Sets minima and maxima for a joint.

	The minima and maxima are relative to the joint's natural state.
	Since the joint may be 1-, 2- or 3-DOF, only the relevant field
	of the vectors are used.
*/

void slJointSetLimits(slJoint *joint, slVector *min, slVector *max) {
	switch(joint->type) {
		case JT_PRISMATIC:	
			dJointSetSliderParam(joint->odeJointID, dParamStopERP, .1);
			dJointSetSliderParam(joint->odeJointID, dParamLoStop, min->x);
			dJointSetSliderParam(joint->odeJointID, dParamHiStop, max->x);
			break;
		case JT_REVOLUTE:	
			dJointSetHingeParam(joint->odeJointID, dParamStopERP, .1);
			dJointSetHingeParam(joint->odeJointID, dParamLoStop, min->x);
			dJointSetHingeParam(joint->odeJointID, dParamHiStop, max->x);
			break;
		case JT_BALL:	
			if(max->y >= M_PI/2.0 - 0.001) max->y = M_PI/2.0 - 0.001;
			if(min->y <= -M_PI/2.0 - 0.001) min->y = -M_PI/2.0 + 0.001;

			dJointSetAMotorParam(joint->odeMotorID, dParamLoStop, min->x);
			dJointSetAMotorParam(joint->odeMotorID, dParamLoStop2, min->y);
			dJointSetAMotorParam(joint->odeMotorID, dParamLoStop3, min->z);
			dJointSetAMotorParam(joint->odeMotorID, dParamStopERP, .1);
			dJointSetAMotorParam(joint->odeMotorID, dParamStopERP2, .1);
			dJointSetAMotorParam(joint->odeMotorID, dParamStopERP3, .1);
			dJointSetAMotorParam(joint->odeMotorID, dParamHiStop, max->x);
			dJointSetAMotorParam(joint->odeMotorID, dParamHiStop2, max->y);
			dJointSetAMotorParam(joint->odeMotorID, dParamHiStop3, max->z);
			break;
		case JT_UNIVERSAL:
			dJointSetAMotorParam(joint->odeMotorID, dParamLoStop, min->x);
			dJointSetAMotorParam(joint->odeMotorID, dParamLoStop3, min->y);
			dJointSetAMotorParam(joint->odeMotorID, dParamStopERP, .1);
			dJointSetAMotorParam(joint->odeMotorID, dParamStopERP3, .1);
			dJointSetAMotorParam(joint->odeMotorID, dParamHiStop, max->x);
			dJointSetAMotorParam(joint->odeMotorID, dParamHiStop3, max->y);
			break;
	}
}

/*!
	\brief Set the maximum torque that a joint can affect.
*/

void slJointSetMaxTorque(slJoint *joint, double max) {
	switch(joint->type) {
		case JT_REVOLUTE:
			dJointSetHingeParam(joint->odeJointID, dParamFMax, max);
			break;
		case JT_PRISMATIC:
			dJointSetSliderParam(joint->odeJointID, dParamFMax, max);
			break;
		case JT_UNIVERSAL:
			dJointSetAMotorParam(joint->odeMotorID, dParamFMax, max);
			dJointSetAMotorParam(joint->odeMotorID, dParamFMax3, max);
			break;
		case JT_BALL:
			dJointSetAMotorParam(joint->odeMotorID, dParamFMax, max);
			dJointSetAMotorParam(joint->odeMotorID, dParamFMax2, max);
			dJointSetAMotorParam(joint->odeMotorID, dParamFMax3, max);
			break;
	}
}
