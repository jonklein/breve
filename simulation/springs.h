enum slSpringModes {
	SM_NORMAL = 0,
	SM_EXPAND_ONLY,
	SM_CONTRACT_ONLY
};

struct slSpring {
	double length;
	double strength;
	double damping;

	unsigned char mode;

	slLink *link1;
	slLink *link2;

	slVector point1;
	slVector point2;
};

void slSpringApplyForce(slSpring *spring);
slSpring *slSpringNew(slLink *l1, slLink *l2, slVector *v1, slVector *v2, double length, double strength, double damping);

