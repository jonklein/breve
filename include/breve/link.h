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

#ifndef _LINK_H
#define _LINK_H

#include <ode/ode.h>

#include "worldObject.h"
#include "multibody.h"

#define slLinkSwapConfig(r)		((r)->_currentState = !(r)->_currentState)

/*!
	\brief Holds link state data in an integratable structure.

	The integration position is a position structure that can be 
	integrated as a vector of doubles.  

	We use one copy of this structure to specify rotational velocity, 
	velocity and acceleration, then can integrate the values directly
	into this structure.
*/

struct slLinkIntegrationPosition {
    slQuat rotQuat;
    slVector location;
    slsVector velocity;
};

/*!
	\brief Holds a link -- a single simulated body.

	A link is a single simulated physical body.  These are 
	called links because they are frequently used in conjunction
	with joints and other links to build up larger logically 
	linked bodies called multibodies.
*/

#include <vector>

class slLink: public slWorldObject {
	public:
		slLink(slWorld *w);

		~slLink();

		void draw() {
		}

		void setLocation(slVector *rotation);
		void setRotation(double rotation[3][3]);
		void getRotation(double m[3][3]);
		int checkSelfPenetration(slWorld *w);

		std::vector< void* > userDataForPenetratingObjects(slWorld *w);

		void applyForce(slVector *f, slVector *t);
		void applyJointControls();
		void updateBoundingBox();
		void disableSimulation();
		void enableSimulation();
		void updatePosition();
		void updatePositions();

		void setForce(slVector *f);
		void setTorque(slVector *t);

		void getVelocity(slVector *velocity, slVector *rotational);
		void setVelocity(slVector *velocity, slVector *rotational);

		void getAcceleration(slVector *linear, slVector *rotational);
		void setAcceleration(slVector *linear, slVector *rotational);

		/**
		 * Sets the multibody field to NULL for all connected links.  This 
		 * is done when a joint is broken which dissociates the links from
		 * the multibody.
		 */

		void nullMultibodiesForConnectedLinks();

		/**
		 * Returns the multibody this link is associated with, if any.
		 */

		slMultibody *getMultibody();

		/*
		 * Returns an \ref slPosition pointer holding this link's position.
		 */

		slPosition *getPosition();

		/**
		 * Returns the minimum and maximum reach of this link, taking current
		 * rotation into account.
		 */

		void getBounds(slVector *min, slVector *max);

		/**
		 * Sets the shape of this link.
		 */

		void setShape(slShape *s);
		void step(slWorld *world, double step);

		/**
		 * Sets a label to be drawn with this link.
		 */

		void setLabel(char *label);

		/**
		 * Recursively builds a list of all directly and indirectly 
		 * connected links.  
		 * @param mbOnly determines whether only multibody links should followed.
		 */

		void connectedLinks(std::vector<slLink*> *list, int mbOnly);


		slMultibody *multibody;

		slsVector acceleration;
		slsVector velocity;

		char mobile;

		bool _justMoved;

		slLinkIntegrationPosition _stateVector[2];

		unsigned char _currentState;

		unsigned int clipNumber;

		dBodyID _odeBodyID;
		dMass _massData;

		slsVector _externalForce;

		unsigned char drawOptions;

		std::vector<slJoint*> inJoints;
		std::vector<slJoint*> outJoints;

};

slJoint *slLinkLinks(slWorld *world, slLink *parent, slLink *child,
		int jointType, slVector *normal, 
		slVector *plinkPoint, slVector *clinkPoint, double m[3][3]);

void slVelocityAtPoint(slVector *vel, slVector *avel, slVector *atPoint, slVector *d);


void slLinkSetTexture(slLink *l, int texture);

void slSlToODEMatrix(double m[3][3], dReal *r);
void slODEToSlMatrix(dReal *r, double m[3][3]);

#endif /* LINK_H */
