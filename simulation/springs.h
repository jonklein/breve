struct slSpring {
	double naturalLength;
	double springConstant;

	slLink *link1;
	slLink *link2;

	slVector point1;
	slVector point2;
};
