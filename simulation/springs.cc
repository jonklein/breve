#include "simulation.h"

void slSpringApplyForce(slSpring *spring) {
	slVector v1, v2, diff;
	double distance;

	slPositionVertex(&spring->link1->position, &spring->point1, &v1);
	slPositionVertex(&spring->link2->position, &spring->point2, &v2);

	slVectorSub(&v1, &v2, &diff);

	distance = slVectorLength(&diff);
}
