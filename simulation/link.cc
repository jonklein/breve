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
	\brief Creates a new link struct.
*/

slLink *slLinkNew(slWorld *w) {
	// return new slLink(new slLinkObject(w));
	return new slLink(w);
}

/*!
	\brief Set a link to a certain shape.

	Shapes may be shared among several objects.
*/

void slLinkSetShape(slLink *l, slShape *s) {
	if(l->shape) slShapeFree(l->shape);

	l->shape = s;
	s->referenceCount++;

	// The ODE docs call dMatrix3 a 3x3 matrix.  but it's actually 4x3.  
	// go figure.

	l->massData.mass = l->shape->mass;
	l->massData.c[0] = 0.0;
	l->massData.c[1] = 0.0;
	l->massData.c[2] = 0.0;
	l->massData.c[3] = 0.0;

	l->massData.I[0] = s->inertia[0][0];
	l->massData.I[1] = s->inertia[0][1];
	l->massData.I[2] = s->inertia[0][2];
	l->massData.I[3] = 0.0;

	l->massData.I[4] = s->inertia[1][0];
	l->massData.I[5] = s->inertia[1][1];
	l->massData.I[6] = s->inertia[1][2];
	l->massData.I[7] = 0.0;

	l->massData.I[8] = s->inertia[2][0];
	l->massData.I[9] = s->inertia[2][1];
	l->massData.I[10] = s->inertia[2][2];
	l->massData.I[11] = 0.0;

	dBodySetMass(l->odeBodyID, &l->massData);
}

/*
	\brief Sets the label associated with this link.
*/

void slLinkSetLabel(slLink *l, char *label) {
	if(l->label) slFree(l->label);
	l->label = slStrdup(label);
}

/*! 
	\brief Sets the location for a single link. 
*/

void slLinkSetLocation(slLink *m, slVector *location) {
	slVectorCopy(location, &m->position.location);

	if(m->simulate) {
		if(m->odeBodyID) dBodySetPosition(m->odeBodyID, location->x, location->y, location->z);
	} else {
		slVectorCopy(location, &m->stateVector[m->currentState].location);
		slVectorCopy(location, &m->stateVector[!m->currentState].location);
	}

	slLinkUpdateBoundingBox(m);
}

/*! 
	\brief Sets the rotation for a single link 
*/

void slLinkSetRotation(slLink *m, double rotation[3][3]) {
	if(m->simulate) {
		dReal r[16];

		slSlToODEMatrix(rotation, r);
		dBodySetRotation(m->odeBodyID, r);
	} else {
		slQuat q;

		slMatrixToQuat(rotation, &q);

		slQuatCopy(&q, &m->stateVector[m->currentState].rotQuat);
		slQuatCopy(&q, &m->stateVector[!m->currentState].rotQuat);
	}

	slMatrixCopy(rotation, m->position.rotation);
	slLinkUpdateBoundingBox(m);
}

/*!
	\brief Gets the rotational and/or linear velocity of a link.
*/

void slLinkGetVelocity(slLink *m, slVector *velocity, slVector *rotational) {
	slLinkIntegrationPosition *config;

	if(!m->simulate) {
		config = &m->stateVector[m->currentState];

		if(velocity) slVectorCopy(&config->velocity.b, velocity);
		if(rotational) slVectorCopy(&config->velocity.a, rotational);
	} else {
		if(velocity) {
			const dReal *v = dBodyGetLinearVel(m->odeBodyID);
			velocity->x = v[0];
			velocity->y = v[1];
			velocity->z = v[2];
		}

		if(rotational) {
			const dReal *v = dBodyGetAngularVel(m->odeBodyID);
			rotational->x = v[0];
			rotational->y = v[1];
			rotational->z = v[2];
		}
	}
}

/*!
	\brief Gets the rotation of this link.
*/

void slLinkGetRotation(slLink *link, double m[3][3]) {
	const dReal *rotation;

	if(!link->simulate) {
		slMatrixCopy(&link->position.rotation, m);
	} else {
		rotation = dBodyGetRotation(link->odeBodyID);
		slODEToSlMatrix((dReal*)rotation, m);
	}
}

/*!
	\brief Sets the linear and/or rotational velocity of a link.
*/

void slLinkSetVelocity(slLink *m, slVector *velocity, slVector *rotational) {
	slLinkIntegrationPosition *config;
	int vmobile = 0, rmobile = 0;

	if(!m->simulate) {
		config = (slLinkIntegrationPosition*)&m->stateVector[m->currentState];

		if(velocity) {
			slVectorCopy(velocity, &config->velocity.b);
			if(!slVectorZeroTest(velocity)) vmobile = 1;
		}

		if(rotational) {
			slVectorCopy(rotational, &config->velocity.a);
			if(!slVectorZeroTest(rotational)) rmobile = 1;
		}

		if(m->mobile && !rmobile && !vmobile) m->mobile = -1;
		else if(vmobile || rmobile) m->mobile = 1;
	} else {
		if(velocity) dBodySetLinearVel(m->odeBodyID, velocity->x, velocity->y, velocity->z);
		if(rotational) dBodySetAngularVel(m->odeBodyID, rotational->x, rotational->y, rotational->z);
	}
}

/*!
	\brief Sets the linear and/or rotational acceleration of a link.
*/

void slLinkSetAcceleration(slLink *m, slVector *linear, slVector *rotational) {
	int lmobile = 0, rmobile = 0;

	if(linear) {
		slVectorCopy(linear, &m->acceleration.b);
		if(!slVectorZeroTest(linear)) lmobile = 1;
	}

	if(rotational) {
		slVectorCopy(rotational, &m->acceleration.a);
		if(!slVectorZeroTest(rotational)) rmobile = 1;
	}

	if(m->mobile && !rmobile && !lmobile) m->mobile = -1;
	else if(rmobile || lmobile) m->mobile = 1;
}

/*!
	\brief Apply a linear/rotational force to a link.
*/

void slLinkApplyForce(slLink *m, slVector *f, slVector *t) {
	if(f) dBodyAddForce(m->odeBodyID, f->x, f->y, f->z);
	if(t) dBodyAddTorque(m->odeBodyID, t->x, t->y, t->z);
}

/*!
	\brief Updates the link positions from the ODE positions.
*/

void slLinkUpdatePositions(slLink *l) {
	const dReal *positionV;
	const dReal *rotationV;
	
	if(!l->simulate || !l->odeBodyID) return;

	positionV = dBodyGetPosition(l->odeBodyID);
	rotationV = dBodyGetRotation(l->odeBodyID);

	l->position.location.x = positionV[0];
	l->position.location.y = positionV[1];
	l->position.location.z = positionV[2];

	slODEToSlMatrix((dReal*)rotationV, l->position.rotation);
}

/*!
	\brief Checks for collisions between this link and all other links 
	in the same \ref slMultibody.

	Returns 1 if any such collisions are occurring.
*/

int slLinkCheckSelfPenetration(slWorld *world, slLink *l) {
	slVclipData *vc;
	int x, y;
	std::vector<slLink*> links;
	std::vector<slLink*>::iterator li;

	if(!world->initialized) slVclipDataInit(world);
	vc = world->clipData;

	// slLinkList(l, &links, 0);

	for(li = links.begin(); li != links.end(); li++ ) {
		slPairEntry *pe;

		slLink *link2 = *li;

		if(l != link2) {
			x = l->clipNumber;
			y = link2->clipNumber;

			pe = slVclipPairEntry(vc->pairList, x, y);

			if((slVclipFlagsShouldTest(pe->flags)) && slVclipTestPair(vc, pe, NULL)) {
				return 1;
			}
		}
	}

	return 0;
}

/*!
	\brief Checks for collisions between this link and all other objects.

	Returns 1 if any such collisions are occurring.
*/

int slLinkCheckPenetration(slWorld *w, slLink *l) {
	slVclipData *vc;
	slPairEntry *pe;
	unsigned int ln;
	unsigned int n;

	if(!w->initialized) slVclipDataInit(w);

	ln = l->clipNumber;

	vc = w->clipData;

	for(n=0;n<vc->count;n++) {
		if(ln != n) {
			pe = slVclipPairEntry(vc->pairList, ln, n);

			if((slVclipFlagsShouldTest(pe->flags)) && slVclipTestPair(vc, pe, NULL)) 
				return 1;
		}
	}

	return 0;
}

/*!
	\brief Applies external torques and forces to a link and its joints.
*/

void slLinkApplyJointControls(slLink *m) {
	slJoint *joint;
	double angle, speed;
	double newSpeed;
	std::vector<slJoint*>::iterator ji;

	slLinkApplyForce(m, &m->externalForce, NULL);

	for(ji = m->inJoints.begin(); ji != m->inJoints.end(); ji++ ) {
		joint = *ji;

		if(joint->type == JT_REVOLUTE) {
			angle = dJointGetHingeAngle(joint->odeJointID);
			speed = dJointGetHingeAngleRate(joint->odeJointID);
		} else if(joint->type == JT_PRISMATIC) {
			angle = dJointGetSliderPosition(joint->odeJointID);
				speed = dJointGetSliderPositionRate(joint->odeJointID);
		} else {
			angle = 0;
			speed = 0;
		}

		newSpeed = joint->targetSpeed;

		if(joint->kSpring != 0.0) {
			double delta = 0;
	
			if(angle > joint->sMax) {
				delta = joint->kSpring * (joint->sMax - angle);
			} else if(angle < joint->sMin) {
				delta = joint->kSpring * (joint->sMin - angle);
			}
	
			newSpeed += delta;
		}

		if(joint->kDamp != 0.0) dJointAddHingeTorque(joint->odeJointID, -speed * joint->kDamp);

		if(joint->type == JT_REVOLUTE) dJointSetHingeParam (joint->odeJointID, dParamVel, newSpeed);
		else if(joint->type == JT_PRISMATIC) dJointSetSliderParam (joint->odeJointID, dParamVel, newSpeed);
	}
}

/*!
	\brief Updates the bounding box minima and maxima for a link.
*/

void slLinkUpdateBoundingBox(slLink *link) {
	slShapeBounds(link->shape, &link->position, &link->min, &link->max);
}

/*!
	\brief Enables physical simulation for a link.
*/

void slLinkEnableSimulation(slLink *r) {
	if(r->simulate == 1) return;

	r->simulate = 1;

	dBodySetLinearVel(r->odeBodyID, r->velocity.b.x, r->velocity.b.y, r->velocity.b.z);
	dBodySetAngularVel(r->odeBodyID, r->velocity.a.x, r->velocity.a.y, r->velocity.a.z);

	dBodySetPosition(r->odeBodyID, r->position.location.x, r->position.location.y, r->position.location.z);
}

/*!
	\brief Disables physical simulation for a link.
*/

void slLinkDisableSimulation(slLink *r) {
	const dReal *position, *rotation;
	const dReal *linearV, *angularV;

	if(r->simulate == 0) return;

	r->simulate = 0;

	position = dBodyGetPosition(r->odeBodyID);
	rotation = dBodyGetRotation(r->odeBodyID);
	linearV = dBodyGetLinearVel(r->odeBodyID);
	angularV = dBodyGetAngularVel(r->odeBodyID);
	
	r->position.location.x = position[0];
	r->position.location.y = position[1];
	r->position.location.z = position[2];

	slODEToSlMatrix((dReal*)rotation, r->position.rotation);

	r->velocity.a.x = angularV[0];
	r->velocity.a.y = angularV[1];
	r->velocity.a.z = angularV[2];

	r->velocity.b.x = linearV[0];
	r->velocity.b.y = linearV[1];
	r->velocity.b.z = linearV[2];
}

/*!
	\brief Updates the position field from the integration data.
*/

void slLinkUpdatePosition(slLink *r) {
	// copy the velocity and position info from the state slVector

	slLinkIntegrationPosition *f = &r->stateVector[r->currentState];

	slVectorCopy(&f->location, &r->position.location);
	slQuatNormalize(&f->rotQuat);
	slQuatToMatrix(&f->rotQuat, r->position.rotation);

	slVectorCopy(&f->velocity.a, &r->velocity.a);
	slVectorInvXform(r->position.rotation, &f->velocity.b, &r->velocity.b);
}

/*!
	\brief Creates a joint between two links.
*/

slJoint *slLinkLinks(slWorld *world, slLink *parent, slLink *child, int jointType, slVector *normal, slVector *plinkPoint, slVector *clinkPoint, double rotation[3][3]) {
	slJoint *joint;
	slVector position, newPosition, tp, tc, tn, axis2;
	dReal R[16];
	double pR[3][3], cR[3][3];
	dBodyID pBodyID, cBodyID;

	if(!child) return NULL;

	// I had to switch the order of arguments to dJointAttach to work-around a problem
	// in ODE, so now we have to reverse the normal to preserve the behavior of the 
	// previous versions of breve.

	slVectorMul(normal, -1.0, normal);

	slLinkUpdatePositions(child);

	if(parent) {
		slLinkUpdatePositions(parent);
		slMatrixCopy(parent->position.rotation, pR);
		slVectorCopy(&parent->position.location, &position);
	} else {
		slMatrixIdentity(pR);
		slVectorSet(&position, 0, 0, 0);
	}

	slMatrixMulMatrix(pR, rotation, cR);

	slMatrixCopy(cR, child->position.rotation);

	slSlToODEMatrix(cR, R);

	slVectorXform(pR, plinkPoint, &tp);
	slVectorXform(pR, normal, &tn);
	slVectorXform(cR, clinkPoint, &tc);

	slVectorAdd(&position, &tp, &newPosition);
	slVectorSub(&newPosition, &tc, &newPosition);

	dBodySetRotation(child->odeBodyID, R);
	dBodySetPosition(child->odeBodyID, newPosition.x, newPosition.y, newPosition.z);
	slVectorCopy(&newPosition, &child->position.location);

	/* normalize the normal vector, just in case */

	slVectorNormalize(&tn);

	joint = new slJoint;
	bzero(joint, sizeof(slJoint));

	/* for both parent and child:
		if it does NOT exist, OR if it already has a mb, then 
		this is not an MB joint */ 

	if(!parent || (child->multibody && parent->multibody)) {
		joint->isMbJoint = 0;
	} else {
		if(child->multibody) parent->multibody = child->multibody;
		else child->multibody = parent->multibody;

		joint->isMbJoint = 1;
	}

	if(parent) pBodyID = parent->odeBodyID;
	else pBodyID = NULL;

	cBodyID = child->odeBodyID;

	switch(jointType) {
		case JT_BALL:
			joint->odeJointID = dJointCreateBall(world->odeWorldID, world->odeJointGroupID);
			joint->odeMotorID = dJointCreateAMotor(world->odeWorldID, world->odeJointGroupID);
			// dJointSetAMotorMode(joint->odeMotorID, dAMotorEuler);
			dJointAttach(joint->odeMotorID, cBodyID, pBodyID);
			break;
		case JT_UNIVERSAL:
			joint->odeJointID = dJointCreateUniversal(world->odeWorldID, world->odeJointGroupID);
			joint->odeMotorID = dJointCreateAMotor(world->odeWorldID, world->odeJointGroupID);
			dJointSetAMotorMode(joint->odeMotorID, dAMotorEuler);
			dJointAttach(joint->odeMotorID, cBodyID, pBodyID);
			break;
		case JT_REVOLUTE:
			joint->odeJointID = dJointCreateHinge(world->odeWorldID, world->odeJointGroupID);
			break;
		case JT_PRISMATIC:
			joint->odeJointID = dJointCreateSlider(world->odeWorldID, world->odeJointGroupID);
			dJointSetSliderParam(joint->odeJointID, dParamHiStop, 2.0);
			dJointSetSliderParam(joint->odeJointID, dParamLoStop, -2.0);
			break;
		case JT_FIX:
			joint->odeJointID = dJointCreateFixed(world->odeWorldID, world->odeJointGroupID);
			break;
	}

	dJointAttach(joint->odeJointID, cBodyID, pBodyID);

	switch(jointType) {
		case JT_BALL:
			slVectorCross(&tp, &tn, &axis2);
			slVectorNormalize(&axis2);
			dJointSetBallAnchor(joint->odeJointID, tp.x + position.x, tp.y + position.y, tp.z + position.z);

			dJointSetAMotorNumAxes(joint->odeMotorID, 3);
			dJointSetAMotorAxis(joint->odeMotorID, 0, 1, 1, 0, 0);
			dJointSetAMotorAxis(joint->odeMotorID, 1, 1, 0, 1, 0);
			dJointSetAMotorAxis(joint->odeMotorID, 2, 1, 0, 0, 1);

			// slVectorCross(&tp, &tn, &axis2);
			// slVectorNormalize(&axis2);

			// dJointSetBallAnchor(joint->odeJointID, tp.x + position.x, tp.y + position.y, tp.z + position.z);

			// dJointSetAMotorAxis(joint->odeMotorID, 0, 1, tn.x, tn.y, tn.z);
			// dJointSetAMotorAxis(joint->odeMotorID, 2, 2, axis2.x, axis2.y, axis2.z);
			break;
		case JT_UNIVERSAL:
			slVectorCross(&tp, &tn, &axis2);
			slVectorNormalize(&axis2);

			dJointSetUniversalAxis1(joint->odeJointID, tn.x, tn.y, tn.z);
			dJointSetUniversalAxis2(joint->odeJointID, axis2.x, axis2.y, axis2.z);
			dJointSetUniversalAnchor(joint->odeJointID, tp.x + position.x, tp.y + position.y, tp.z + position.z);

			dJointSetAMotorAxis(joint->odeMotorID, 0, 1, tn.x, tn.y, tn.z);
			dJointSetAMotorAxis(joint->odeMotorID, 2, 2, axis2.x, axis2.y, axis2.z);
			break;
		case JT_REVOLUTE:
			dJointSetHingeAxis(joint->odeJointID, tn.x, tn.y, tn.z);
			dJointSetHingeAnchor(joint->odeJointID, tp.x + position.x, tp.y + position.y, tp.z + position.z);
			break;
		case JT_PRISMATIC:
			dJointSetSliderAxis(joint->odeJointID, tn.x, tn.y, tn.z);
			break;
		case JT_FIX:
			dJointSetFixed(joint->odeJointID);
			break;
	}

	if(parent) parent->outJoints.push_back(joint);
	child->inJoints.push_back(joint);

	joint->parent = parent;
	joint->child = child;
	joint->type = jointType;

	if(parent && parent->multibody) slMultibodyUpdate(parent->multibody);
	if(child->multibody && (!parent || (child->multibody != parent->multibody))) slMultibodyUpdate(child->multibody);

	return joint;
}

void slVelocityAtPoint(slVector *vel, slVector *avel, slVector *atPoint, slVector *d) {
	slVectorCross(avel, atPoint, d);
	slVectorAdd(d, vel, d);
}

slPosition *slLinkGetPosition(slLink *l) {
	return &l->position;
}

slMultibody *slLinkGetMultibody(slLink *l) {
	return l->multibody;
}

void slLinkSetTexture(slLink *l, int texture) {
	l->texture = texture;
}

void slLinkGetAcceleration(slLink *l, slVector *linear, slVector *rotational) {
	if(linear) slVectorCopy(&l->acceleration.b, linear);
	if(rotational) slVectorCopy(&l->acceleration.a, rotational);
}

void slLinkGetBounds(slLink *l, slVector *min, slVector *max) {
	if(min) slVectorCopy(&l->min, min);
	if(max) slVectorCopy(&l->max, max);
}

void slLinkSetForce(slLink *l, slVector *force) {
	slVectorCopy(force, &l->externalForce);
}
