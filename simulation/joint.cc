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
