
#include "simulation.h"

/*!
	\brief Applies the spring forces between two bodies.
*/

void slSpringApplyForce(slSpring *spring) {
	slVector pos1, pos2, toV1, vel1, vel2, velocity;
	double distance, normVelocity, damping;
	slVector linearVel, angularVel;

	slPositionVertex(&spring->link1->position, &spring->point1, &pos1);
	slPositionVertex(&spring->link2->position, &spring->point2, &pos2);

	slVectorSub(&pos1, &pos2, &toV1);

	distance = slVectorLength(&toV1);

	/////////////////////

	slLinkGetVelocity(spring->link1, &linearVel, &angularVel);
	slVelocityAtPoint(&linearVel, &angularVel, &spring->point1, &vel1);
	// slVectorPrint(&linearVel);

	slLinkGetVelocity(spring->link2, &linearVel, &angularVel);
	slVelocityAtPoint(&linearVel, &angularVel, &spring->point2, &vel2);
	// slVectorPrint(&linearVel);

	slVectorSub(&vel1, &vel2, &velocity);

	normVelocity = slVectorDot(&velocity, &toV1);

	/////////////////////

	damping = spring->dampingConstant * normVelocity;

	distance -= spring->naturalLength;

	distance = (distance * spring->springConstant) + damping;

	slVectorNormalize(&toV1);
	slVectorMul(&toV1, distance, &toV1);

	dBodyAddForceAtPos(spring->link2->odeBodyID, 
						toV1.x, toV1.y, toV1.z, pos2.x, pos2.y, pos2.z);

	dBodyAddForceAtPos(spring->link1->odeBodyID, 
						-toV1.x, -toV1.y, -toV1.z, pos1.x, pos1.y, pos1.z);
}

/*!
	\brief Creates a new spring.
*/

slSpring *slSpringNew(slLink *l1, slLink *l2, slVector *p1, slVector *p2, double length, double strength, double damping) {
	slSpring *spring;

	spring = slMalloc(sizeof(slSpring));

	slVectorCopy(p1, &spring->point1);
	slVectorCopy(p2, &spring->point2);

	spring->link1 = l1;
	spring->link2 = l2;

	spring->naturalLength = length;
	spring->springConstant = strength;
	spring->dampingConstant = damping;

	return spring;
}

/*!
	\brief Adds a spring to the world.
*/

void slWorldAddSpring(slWorld *w, slSpring *s) {
	slStackPush(w->springObjects, s);
}
