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

#define slLinkSwapConfig(r)		((r)->currentState = !(r)->currentState)

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

#ifdef __cplusplus
#include <vector>

class slLink: public slWorldObject {
	public:
		slLink(slWorld *w) : slWorldObject() {
    		odeBodyID = dBodyCreate(w->odeWorldID);

		    simulate = 0;
			currentState = 0;
			mobile = 0;

			bzero(&stateVector[0], sizeof(slLinkIntegrationPosition));
			bzero(&stateVector[1], sizeof(slLinkIntegrationPosition));

		    slQuatIdentity(&stateVector[0].rotQuat);
		    slQuatIdentity(&stateVector[1].rotQuat);

			multibody = NULL;

			slsVectorZero(&acceleration);
			slsVectorZero(&velocity);

			slVectorZero(&externalForce);
		}

		~slLink();

		slMultibody *multibody;

		slsVector acceleration;
		slsVector velocity;

		char mobile;

		slLinkIntegrationPosition stateVector[2];

		unsigned char currentState;
		unsigned char simulate;

		unsigned int clipNumber;

		dBodyID odeBodyID;
		dMass massData;

		slVector externalForce;

		unsigned char drawOptions;

		std::vector<slJoint*> inJoints;
		std::vector<slJoint*> outJoints;
};

typedef safe_ptr<slLink> slLinkPointer;

#endif

#ifdef __cplusplus
extern "C" {
#endif
slLink *slLinkNew(slWorld *w);
void slLinkSetShape(slLink *l, slShape *s);

slLinkIntegrationPosition *slLinkGetCurrentConfig(slLink *m);
void slLinkFree(slLink *l);

void slLinkSetLocation(slLink *m, slVector *rotation);
void slLinkSetRotation(slLink *m, double rotation[3][3]);

void slLinkGetRotation(slLink *root, double m[3][3]);

int slLinkCheckSelfPenetration(slWorld *w, slLink *l);
int slLinkCheckPenetration(slWorld *w, slLink *l);

void slLinkSetLabel(slLink *m, char *label);

void slLinkApplyForce(slLink *m, slVector *f, slVector *t);

void slLinkApplyJointControls(slLink *l);

void slLinkUpdateBoundingBox(slLink *r);

void slLinkDisableSimulation(slLink *r);
void slLinkEnableSimulation(slLink *r);

void slLinkUpdatePosition(slLink *r);

slJoint *slLinkLinks(slWorld *world, slLink *parent, slLink *child,
		int jointType, slVector *normal, 
		slVector *plinkPoint, slVector *clinkPoint, double m[3][3]);

void slVelocityAtPoint(slVector *vel, slVector *avel, slVector *atPoint, slVector *d);

void slLinkGetAcceleration(slLink *l, slVector *linear, slVector *rotational);
void slLinkSetAcceleration(slLink *m, slVector *linear, slVector *rotational);

void slLinkGetVelocity(slLink *m, slVector *velocity, slVector *rotational);
void slLinkSetVelocity(slLink *m, slVector *velocity, slVector *rotational);

slPosition *slLinkGetPosition(slLink *l);

slMultibody *slLinkGetMultibody(slLink *l);

void slLinkSetTexture(slLink *l, int texture);

void slLinkGetBounds(slLink *l, slVector *min, slVector *max);

void slLinkSetForce(slLink *l, slVector *force);

#ifdef __cplusplus
}
#endif
