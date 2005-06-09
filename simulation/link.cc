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
#include "world.h"
#include "joint.h"
#include "vclip.h"
#include "vclipData.h"

/*!
	\brief Creates a new slLink.
*/

slLink::slLink(slWorld *w) : slWorldObject() {
	_odeBodyID = dBodyCreate(w->_odeWorldID);

	simulate = 0;
	_currentState = 0;

	_moved = 1;
	_justMoved = 1;

	mobile = -1;

	memset(&_stateVector[0], 0, sizeof(slLinkIntegrationPosition));
	memset(&_stateVector[1], 0, sizeof(slLinkIntegrationPosition));

	slQuatIdentity(&_stateVector[0].rotQuat);
	slQuatIdentity(&_stateVector[1].rotQuat);

	multibody = NULL;

	slsVectorZero(&acceleration);
	slsVectorZero(&velocity);
	slsVectorZero(&_externalForce);
}

slLink::~slLink() {
	if(multibody && multibody->_root == this) multibody->setRoot(NULL);

	// This is a bad situation here: slJointBreak modifies the 
	// joint list.  I intend to fix this.

	while(inJoints.size() != 0) (*inJoints.begin())->breakJoint();
	while(outJoints.size() != 0) (*outJoints.begin())->breakJoint();

	dBodyDestroy(_odeBodyID);
}

void slLink::step(slWorld *world, double step) {
	_moved = 1;

	if(simulate) {
		updatePositions();
		applyJointControls();

		if(world->detectCollisions) updateBoundingBox();
	} else {
		if(mobile) { 
			world->integrator(world, this, &step, 0);
			slLinkSwapConfig(this);
			updatePosition();

			if(world->detectCollisions) updateBoundingBox();
		} else {
			// if( !_justMoved) _moved = 0;
			// else _justMoved = 0;
		}
	}
}

/*!
	\brief Set a link to a certain shape.

	Shapes may be shared among several objects.
*/

void slLink::setShape(slShape *s) {
	if(shape) slShapeFree(shape);

	shape = s;
	s->_referenceCount++;

	// The ODE docs call dMatrix3 a 3x3 matrix.  but it's actually 4x3.  
	// go figure.

	dMassSetZero(&_massData);

	_massData.mass = shape->_mass;
	_massData.c[0] = 0.0;
	_massData.c[1] = 0.0;
	_massData.c[2] = 0.0;
	_massData.c[3] = 0.0;

	_massData.I[0] = s->_inertia[0][0];
	_massData.I[1] = s->_inertia[0][1];
	_massData.I[2] = s->_inertia[0][2];
	_massData.I[3] = 0.0;

	_massData.I[4] = s->_inertia[1][0];
	_massData.I[5] = s->_inertia[1][1];
	_massData.I[6] = s->_inertia[1][2];
	_massData.I[7] = 0.0;

	_massData.I[8] = s->_inertia[2][0];
	_massData.I[9] = s->_inertia[2][1];
	_massData.I[10] = s->_inertia[2][2];
	_massData.I[11] = 0.0;

	dBodySetMass(_odeBodyID, &_massData);
}

/*
	\brief Sets the label associated with this link.
*/

void slLink::setLabel(char *l) {
	label = l;
}

/*! 
	\brief Sets the location for a single link. 
*/

void slLink::setLocation(slVector *location) {
	_justMoved = 1;

	slVectorCopy(location, &position.location);

	if(simulate) {
		if(_odeBodyID) dBodySetPosition(_odeBodyID, location->x, location->y, location->z);
	} else {
		slVectorCopy(location, &_stateVector[_currentState].location);
		slVectorCopy(location, &_stateVector[!_currentState].location);
	}

	updateBoundingBox();
}

/*! 
	\brief Sets the rotation for a single link 
*/

void slLink::setRotation(double rotation[3][3]) {
	_justMoved = 1;

	if(simulate) {
		dReal r[16];

		slSlToODEMatrix(rotation, r);
		dBodySetRotation(_odeBodyID, r);
	} else {
		slQuat q;

		slMatrixToQuat(rotation, &q);

		slQuatCopy(&q, &_stateVector[_currentState].rotQuat);
		slQuatCopy(&q, &_stateVector[!_currentState].rotQuat);
	}

	slMatrixCopy(rotation, position.rotation);
	updateBoundingBox();
}

/*!
	\brief Gets the rotational and/or linear velocity of a link.
*/

void slLink::getVelocity(slVector *velocity, slVector *rotational) {
	slLinkIntegrationPosition *config;

	if(!simulate) {
		config = &_stateVector[_currentState];

		if(velocity) slVectorCopy(&config->velocity.b, velocity);
		if(rotational) slVectorCopy(&config->velocity.a, rotational);
	} else {
		if(velocity) {
			const dReal *v = dBodyGetLinearVel(_odeBodyID);
			velocity->x = v[0];
			velocity->y = v[1];
			velocity->z = v[2];
		}

		if(rotational) {
			const dReal *v = dBodyGetAngularVel(_odeBodyID);
			rotational->x = v[0];
			rotational->y = v[1];
			rotational->z = v[2];
		}
	}
}

/*!
	\brief Gets the rotation of this link.
*/

void slLink::getRotation(double m[3][3]) {
	const dReal *rotation;

	if(!simulate) {
		slMatrixCopy(&position.rotation, m);
	} else {
		rotation = dBodyGetRotation(_odeBodyID);
		slODEToSlMatrix((dReal*)rotation, m);
	}
}

/*!
	\brief Sets the linear and/or rotational velocity of a link.
*/

void slLink::setVelocity(slVector *velocity, slVector *rotational) {
	slLinkIntegrationPosition *config;

	if(!simulate) {
		config = (slLinkIntegrationPosition*)&_stateVector[_currentState];

		if(velocity) {
			slVectorCopy(velocity, &config->velocity.b);
			if(!slVectorIsZero(velocity)) mobile = 1;
		}

		if(rotational) {
			slVectorCopy(rotational, &config->velocity.a);
			if(!slVectorIsZero(rotational)) mobile = 1;
		}

	} else {
		if(velocity) dBodySetLinearVel(_odeBodyID, velocity->x, velocity->y, velocity->z);
		if(rotational) dBodySetAngularVel(_odeBodyID, rotational->x, rotational->y, rotational->z);
	}
}

/*!
	\brief Sets the linear and/or rotational acceleration of a link.
*/

void slLink::setAcceleration(slVector *linear, slVector *rotational) {
	if(linear) {
		slVectorCopy(linear, &acceleration.b);
		if(!slVectorIsZero(linear)) mobile = 1;
	}

	if(rotational) {
		slVectorCopy(rotational, &acceleration.a);
		if(!slVectorIsZero(rotational)) mobile = 1;
	}
}

/*!
	\brief Apply a linear/rotational force to a link.
*/

void slLink::applyForce(slVector *f, slVector *t) {
	if(f) dBodyAddForce(_odeBodyID, f->x, f->y, f->z);
	if(t) dBodyAddTorque(_odeBodyID, t->x, t->y, t->z);
}

/*!
	\brief Updates the link positions from the ODE positions.
*/

void slLink::updatePositions() {
	const dReal *positionV;
	const dReal *rotationV;
	
	if(!simulate || !_odeBodyID) return;

	positionV = dBodyGetPosition(_odeBodyID);
	rotationV = dBodyGetRotation(_odeBodyID);

	position.location.x = positionV[0];
	position.location.y = positionV[1];
	position.location.z = positionV[2];

	slODEToSlMatrix((dReal*)rotationV, position.rotation);
}

/*!
	\brief Checks for collisions between this link and all other links 
	in the same \ref slMultibody.

	Returns 1 if any such collisions are occurring.
*/

int slLink::checkSelfPenetration(slWorld *world) {
	slVclipData *vc;
	std::vector<slLink*> links;
	std::vector<slLink*>::iterator li;

	if(!world->initialized) slVclipDataInit(world);
	vc = world->clipData;

	slLinkList(this, &links, 0);

	for(li = links.begin(); li != links.end(); li++ ) {
		slLink *link2 = *li;

		if(this != link2) {
			slCollisionCandidate c( vc, clipNumber, link2->clipNumber);

			slPairFlags *flags = slVclipPairFlags(vc, clipNumber, link2->clipNumber);

			if((slVclipFlagsShouldTest(*flags)) && vc->testPair(&c, NULL)) {
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

int slLink::checkPenetration(slWorld *w) {
	slVclipData *vc;
	unsigned int ln;
	unsigned int n;

	if(!w->initialized) slVclipDataInit(w);

	ln = clipNumber;

	vc = w->clipData;

	for(n=0;n<vc->count;n++) {
		if(ln != n) {
			slCollisionCandidate c( vc, ln, n);
			slPairFlags *flags = slVclipPairFlags(vc, ln, n);

			if((slVclipFlagsShouldTest(*flags) && *flags & BT_SIMULATE) && vc->testPair(&c, NULL)) return 1;
		}
	}

	return 0;
}

/*!
	\brief Applies external torques and forces to a link and its joints.
*/

void slLink::applyJointControls() {
	slJoint *joint;
	double angle, speed;
	double newSpeed;
	std::vector<slJoint*>::iterator ji;

	applyForce(&_externalForce.a, &_externalForce.b);

	for(ji = inJoints.begin(); ji != inJoints.end(); ji++ ) {
		joint = *ji;

		if(joint->_type == JT_REVOLUTE) {
			angle = dJointGetHingeAngle(joint->_odeJointID);
			speed = dJointGetHingeAngleRate(joint->_odeJointID);
		} else if(joint->_type == JT_PRISMATIC) {
			angle = dJointGetSliderPosition(joint->_odeJointID);
				speed = dJointGetSliderPositionRate(joint->_odeJointID);
		} else {
			angle = 0;
			speed = 0;
		}

		newSpeed = joint->_targetSpeed;

		if(joint->_kSpring != 0.0) {
			double delta = 0;
	
			if(angle > joint->_sMax) {
				delta = joint->_kSpring * (joint->_sMax - angle);
			} else if(angle < joint->_sMin) {
				delta = joint->_kSpring * (joint->_sMin - angle);
			}
	
			newSpeed += delta;
		}

		if(joint->_kDamp != 0.0) dJointAddHingeTorque(joint->_odeJointID, -speed * joint->_kDamp);

		if(joint->_type == JT_REVOLUTE) dJointSetHingeParam (joint->_odeJointID, dParamVel, newSpeed);
		else if(joint->_type == JT_PRISMATIC) dJointSetSliderParam (joint->_odeJointID, dParamVel, newSpeed);
	}
}

/*!
	\brief Updates the bounding box minima and maxima for a link.
*/

void slLink::updateBoundingBox() {
	shape->bounds(&position, &min, &max);
}

/*!
	\brief Enables physical simulation for a link.
*/

void slLink::enableSimulation() {
	if(simulate == 1) return;

	_justMoved = 1;

	simulate = 1;

	dBodySetLinearVel(_odeBodyID, velocity.b.x, velocity.b.y, velocity.b.z);
	dBodySetAngularVel(_odeBodyID, velocity.a.x, velocity.a.y, velocity.a.z);

	dBodySetPosition(_odeBodyID, position.location.x, position.location.y, position.location.z);
}

/*!
	\brief Disables physical simulation for a link.
*/

void slLink::disableSimulation() {
	const dReal *odePos, *odeRot;
	const dReal *odeLinear, *odeAngular;

	if(simulate == 0) return;

	simulate = 0;

	odePos = dBodyGetPosition(_odeBodyID);
	odeRot = dBodyGetRotation(_odeBodyID);
	odeLinear = dBodyGetLinearVel(_odeBodyID);
	odeAngular = dBodyGetAngularVel(_odeBodyID);
	
	position.location.x = odePos[0];
	position.location.y = odePos[1];
	position.location.z = odePos[2];

	slODEToSlMatrix((dReal*)odeRot, position.rotation);

	setLocation(&position.location);
	setRotation(position.rotation);

	velocity.a.x = odeAngular[0];
	velocity.a.y = odeAngular[1];
	velocity.a.z = odeAngular[2];

	velocity.b.x = odeLinear[0];
	velocity.b.y = odeLinear[1];
	velocity.b.z = odeLinear[2];
}

/*!
	\brief Updates the position field from the integration data.
*/

void slLink::updatePosition() {
	// copy the velocity and position info from the state slVector

	slLinkIntegrationPosition *f = &_stateVector[_currentState];

	slVectorCopy(&f->location, &position.location);
	slQuatNormalize(&f->rotQuat);
	slQuatToMatrix(&f->rotQuat, position.rotation);

	slVectorCopy(&f->velocity.a, &velocity.a);
	slVectorInvXform(position.rotation, &f->velocity.b, &velocity.b);
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

	// No parent is okay.  No child is bad news.

	if(!child) return NULL;

	// I had to switch the order of arguments to dJointAttach to work-around a problem
	// in ODE, so now we have to reverse the normal to preserve the behavior of the 
	// previous versions of breve.

	slVectorMul(normal, -1.0, normal);

	joint = new slJoint;

	// figure out if this is a multibody joint or not.

	if(!parent || (child->multibody && parent->multibody)) {
		joint->_isMbJoint = 0;
	} else {
		if(child->multibody) parent->multibody = child->multibody;
		else child->multibody = parent->multibody;

		joint->_isMbJoint = 1;
	}

	if(parent) pBodyID = parent->_odeBodyID;
	else pBodyID = NULL;

	cBodyID = child->_odeBodyID;

	switch(jointType) {
		case JT_BALL:
			joint->_odeJointID = dJointCreateBall(world->_odeWorldID, world->_odeJointGroupID);
			joint->_odeMotorID = dJointCreateAMotor(world->_odeWorldID, world->_odeJointGroupID);
			// dJointSetAMotorMode(joint->_odeMotorID, dAMotorEuler);
			dJointAttach(joint->_odeMotorID, cBodyID, pBodyID);
			break;
		case JT_UNIVERSAL:
			joint->_odeJointID = dJointCreateUniversal(world->_odeWorldID, world->_odeJointGroupID);
			joint->_odeMotorID = dJointCreateAMotor(world->_odeWorldID, world->_odeJointGroupID);
			dJointSetAMotorMode(joint->_odeMotorID, dAMotorEuler);
			dJointAttach(joint->_odeMotorID, cBodyID, pBodyID);
			break;
		case JT_REVOLUTE:
			joint->_odeJointID = dJointCreateHinge(world->_odeWorldID, world->_odeJointGroupID);
			break;
		case JT_PRISMATIC:
			joint->_odeJointID = dJointCreateSlider(world->_odeWorldID, world->_odeJointGroupID);
			dJointSetSliderParam(joint->_odeJointID, dParamHiStop, 2.0);
			dJointSetSliderParam(joint->_odeJointID, dParamLoStop, -2.0);
			break;
		case JT_FIX:
			joint->_odeJointID = dJointCreateFixed(world->_odeWorldID, world->_odeJointGroupID);
			break;
	}

	dJointAttach(joint->_odeJointID, cBodyID, pBodyID);

	// transform the normal to the parent coordinates.

    if(parent) slVectorXform(parent->position.rotation, normal, &tn);
    else slVectorCopy(normal, &tn);

	switch(jointType) {
		case JT_BALL:
			slVectorCross(&tp, &tn, &axis2);
			slVectorNormalize(&axis2);
			dJointSetBallAnchor(joint->_odeJointID, tp.x + position.x, tp.y + position.y, tp.z + position.z);

			dJointSetAMotorNumAxes(joint->_odeMotorID, 3);
			dJointSetAMotorAxis(joint->_odeMotorID, 0, 1, 1, 0, 0);
			dJointSetAMotorAxis(joint->_odeMotorID, 1, 1, 0, 1, 0);
			dJointSetAMotorAxis(joint->_odeMotorID, 2, 1, 0, 0, 1);

			break;
		case JT_UNIVERSAL:
			slVectorCross(&tp, &tn, &axis2);
			slVectorNormalize(&axis2);

			dJointSetUniversalAxis1(joint->_odeJointID, tn.x, tn.y, tn.z);
			dJointSetUniversalAxis2(joint->_odeJointID, axis2.x, axis2.y, axis2.z);
			dJointSetUniversalAnchor(joint->_odeJointID, tp.x + position.x, tp.y + position.y, tp.z + position.z);

			dJointSetAMotorAxis(joint->_odeMotorID, 0, 1, tn.x, tn.y, tn.z);
			dJointSetAMotorAxis(joint->_odeMotorID, 2, 2, axis2.x, axis2.y, axis2.z);
			break;
		case JT_REVOLUTE:
			dJointSetHingeAxis(joint->_odeJointID, tn.x, tn.y, tn.z);
			dJointSetHingeAnchor(joint->_odeJointID, tp.x + position.x, tp.y + position.y, tp.z + position.z);
			break;
		case JT_PRISMATIC:
			dJointSetSliderAxis(joint->_odeJointID, tn.x, tn.y, tn.z);
			break;
		case JT_FIX:
			dJointSetFixed(joint->_odeJointID);
			break;
	}

	if(parent) parent->outJoints.push_back(joint);
	child->inJoints.push_back(joint);

	joint->_parent = parent;
	joint->_child = child;
	joint->_type = jointType;

	joint->setLinkPoints(plinkPoint, clinkPoint, rotation);
	joint->setNormal(normal);

	if(parent && parent->multibody) parent->multibody->update();

	if(child->multibody && (!parent || (child->multibody != parent->multibody))) child->multibody->update();

	return joint;
}

void slVelocityAtPoint(slVector *vel, slVector *avel, slVector *atPoint, slVector *d) {
	slVectorCross(avel, atPoint, d);
	slVectorAdd(d, vel, d);
}

slPosition *slLink::getPosition() {
	return &position;
}

slMultibody *slLink::getMultibody() {
	return multibody;
}

void slLinkSetTexture(slLink *l, int texture) {
	l->texture = texture;
}

void slLink::getAcceleration(slVector *linear, slVector *rotational) {
	if(linear) slVectorCopy(&acceleration.b, linear);
	if(rotational) slVectorCopy(&acceleration.a, rotational);
}

void slLink::getBounds(slVector *minBounds, slVector *maxBounds) {
	if(minBounds) slVectorCopy(&min, minBounds);
	if(maxBounds) slVectorCopy(&max, maxBounds);
}

void slLink::setForce(slVector *force) {
	slVectorCopy(force, &_externalForce.a);
}

void slLink::setTorque(slVector *torque) {
	slVectorCopy(torque, &_externalForce.b);
}


/*!
	\brief Converts a breve matrix to an ODE matrix.

	Used when getting/setting rotation from the ODE body.
*/


void slSlToODEMatrix(double m[3][3], dReal *r) {
	r[0] = m[0][0];
	r[1] = m[0][1];
	r[2] = m[0][2];

	r[4] = m[1][0];
	r[5] = m[1][1];
	r[6] = m[1][2];

	r[8] = m[2][0];
	r[9] = m[2][1];
	r[10] = m[2][2];
}

/*!
	\brief Converts an ODE matrix to a breve matrix.

	Used when getting/setting rotation from the ODE body.
*/

void slODEToSlMatrix(dReal *r, double m[3][3]) {
	m[0][0] = r[0];
	m[0][1] = r[1];
	m[0][2] = r[2];

	m[1][0] = r[4];
	m[1][1] = r[5];
	m[1][2] = r[6];

	m[2][0] = r[8];
	m[2][1] = r[9];
	m[2][2] = r[10];
}
