struct slSpring {
	double naturalLength;
	double springConstant;
	double dampingConstant;

	slLink *link1;
	slLink *link2;

	slVector point1;
	slVector point2;
};

void slSpringApplyForce(slSpring *spring);
slSpring *slSpringNew(slLink *l1, slLink *l2, slVector *v1, slVector *v2, double length, double strength, double damping);

