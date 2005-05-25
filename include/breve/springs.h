#ifndef _SPRINGS_H
#define _SPRINGS_H

enum slSpringModes {
	SPRING_MODE_NORMAL = 0,
	SPRING_MODE_EXPAND_ONLY,
	SPRING_MODE_CONTRACT_ONLY
};

#ifdef __cplusplus
class slSpring: public slObjectLine {
	public:
		slSpring(slWorld *w, slLink *l1, slLink *l2, slVector *p1, slVector *p2, double length, double strength, double damping);

		void draw(slCamera *c);

		void step(double step);

		void setLength(double length);
		void setStrength(double strength);
		void setDamping(double damping);

		/*! \brief Sets the action mode for this spring.
		 *
    	 * The mode determines how the spring behaves and should be one of \ref slSpringModes.
		 */

		void setMode(int mode);

		/*! \brief Sets the maximum force that the spring is allowed to apply. 
		 * 
		 * If set to 0, the default value, then the maximum is disabled and the
		 * spring is allowed to apply infinite force.
		 */

		void setMaxForce(double maxForce);

		/*! \brief Returns the current length of the spring.
		 *
		 * The current length of the spring is the actual length of the spring,
		 * stretched or compressed from it "natural" length.  To get the natural
	     * length of the spring, see \ref getLength.
		 */

		double getCurrentLength();

		/*! \brief Returns the "natural" length of the spring.
		 * 
 		 * The natural length of the spring is the spring length at which the
	     * spring applies no force on the objects--it is the "resting" length
    	 * of the spring.  To get the actual current length of the spring, see
		 */

		double getLength();

		/*! \brief Returns the level of force applies by this spring at the most recent timestep.
		 *
		 * The force is returned as a double magnatude.  The direction of the force is defined by 
		 * the vector between the two bodies.  
 		 */

		double getForce();

		double _length;
		double _strength;
		double _damping;

		double _force;
		double _maxForce;

		unsigned char _mode;

		slVector _point1;
		slVector _point2;
};
#endif

void slWorldApplySpringForces(slWorld *w);
void slSpringApplyForce(slSpring *spring);

#endif
