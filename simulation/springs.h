enum slSpringModes {
	SPRING_MODE_NORMAL = 0,
	SPRING_MODE_EXPAND_ONLY,
	SPRING_MODE_CONTRACT_ONLY
};

#ifdef __cplusplus
class slSpring: public slObjectLine {
	public:
		slSpring() {
			slVectorSet(&_color, 0, 0, 0);
			_stipple = 0xaaaa;
		}

		void draw(slCamera *c);

		void step(double step);

		double _length;
		double _strength;
		double _damping;

		double _force;

		unsigned char _mode;

		slVector _point1;
		slVector _point2;
};
#endif

void slWorldApplySpringForces(slWorld *w);
void slSpringApplyForce(slSpring *spring);

#ifdef __cplusplus
extern "C" {
#endif
slSpring *slSpringNew(slWorld *w, slLink *l1, slLink *l2, slVector *v1, slVector *v2, double length, double strength, double damping);

double slSpringGetCurrentLength(slSpring *s);
double slSpringGetLength(slSpring *s);

void slSpringSetLength(slSpring *s, double length);
void slSpringSetStrength(slSpring *s, double strength);
void slSpringSetDamping(slSpring *s, double damping);
void slSpringSetMode(slSpring *s, int mode);

void slWorldRemoveSpring(slWorld *w, slSpring *s);
void slWorldAddSpring(slWorld *w, slSpring *s);
void slWorldDrawSprings(slWorld *w);

void slSpringFree(slSpring *spring);

double slSpringGetForce(slSpring *spring);
#ifdef __cplusplus
}
#endif
