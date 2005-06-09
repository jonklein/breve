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

#include "ode/ode.h"

enum jointTypes {
	JT_REVOLUTE = 1,
	JT_PRISMATIC,
	JT_BALL,
	JT_UNIVERSAL,
	JT_FIX
};

class slLink;

/*!
	\brief A joint connecting two links.
*/

class slJoint {
	public:
		~slJoint();
	
		/*!
		 * Updates the link points and relative rotation between two joined objects.
		 */

		void setLinkPoints(slVector *plinkPoint, slVector *clinkPoint, double rotation[3][3]);

		/*!
		 * Sets the normal vector of a prismatic or revolute joint.
		 */

		void setNormal(slVector *normal);

		/*!
		 * Sets the min and max limits of the joint.  Depending on the joint 
		 * type, these may be 1, 2 or 3 values.
		 */

		void setLimits(slVector *min, slVector *max);

		/*!
		 * Gets the velocity of the joint.  Depending on the joint type, this may be 1, 2 or 3 values.
		 */

		void getVelocity(slVector *velocity);

		/*!
		 * Sets the velocity of the joint.  Depending on the joint type, this may be 1, 2 or 3 values.
		 */

		void setVelocity(slVector *velocity);

		/*!
		 * Gets the position of the joint.  Depending on the joint type, this may be 1, 2 or 3 values.
		 */

		void getPosition(slVector *v);

		/*
		 * Sets the maximum torque for this joint.  Depending on the joint type, this may be 1, 2 or 3 values.
		 */

		void setMaxTorque(double max);

		/* 
		 * Breaks (but does not destroy) the joint.
		 */

		void breakJoint();

		/*
		 * Applies torque to the joint.  Depending on the joint type, this may be 1, 2 or 3 values.
		 */
	
		void applyTorque(slVector *torque);

		slLink *_parent;
		slLink *_child;
		dJointID _odeJointID;
		dJointID _odeMotorID;

		double _kDamp;
		double _kSpring;
		double _sMax;
		double _sMin;
		double _torque;
		double _targetSpeed;
	
		unsigned char _type;
		unsigned char _isMbJoint;
	
		void *userData;
};
