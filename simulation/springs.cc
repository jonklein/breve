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

#include "simulation.h"

void slWorldApplySpringForces(slWorld *w) {
	std::vector<slSpring*>::iterator i;
    for(i = w->springs.begin(); i != w->springs.end(); i++) {
		slSpringApplyForce(*i);
	}
}

/*!
	\brief Applies the spring forces between two bodies.
*/

void slSpringApplyForce(slSpring *spring) {
	slVector pos1, pos2, toV1, vel1, vel2, velocity, point, force;
	double distance, normVelocity, damping;
	slVector linearVel, angularVel;

	slPositionVertex(&spring->link1->position, &spring->point1, &pos1);
	slPositionVertex(&spring->link2->position, &spring->point2, &pos2);

	slVectorSub(&pos1, &pos2, &toV1);

	distance = slVectorLength(&toV1);

	////////////////////////////////////////////////

	slVectorXform(spring->link1->position.rotation, &spring->point1, &point);
	slLinkGetVelocity(spring->link1, &linearVel, &angularVel);
	slVelocityAtPoint(&linearVel, &angularVel, &point, &vel1);

	slVectorXform(spring->link2->position.rotation, &spring->point2, &point);
	slLinkGetVelocity(spring->link2, &linearVel, &angularVel);
	slVelocityAtPoint(&linearVel, &angularVel, &point, &vel2);

	slVectorSub(&vel1, &vel2, &velocity);

	normVelocity = slVectorDot(&velocity, &toV1);

	////////////////////////////////////////////////

	distance -= spring->length;

	damping = spring->damping * normVelocity;

	distance = (distance * spring->strength) + damping;

	slVectorNormalize(&toV1);
	slVectorMul(&toV1, distance, &force);

	////////////////////////////////////////////////

	dBodyAddForceAtPos(spring->link2->odeBodyID, 
			force.x, force.y, force.z, pos1.x, pos1.y, pos1.z);

	dBodyAddForceAtPos(spring->link1->odeBodyID, 
			-force.x, -force.y, -force.z, pos2.x, pos2.y, pos2.z);
}

/*!
	\brief Creates a new spring.
*/

slSpring *slSpringNew(slLink *l1, slLink *l2, slVector *p1, slVector *p2, double length, double strength, double damping) {
	slSpring *spring;

	spring = new slSpring;

	slVectorCopy(p1, &spring->point1);
	slVectorCopy(p2, &spring->point2);

	spring->link1 = l1;
	spring->link2 = l2;

	spring->length = length;
	spring->strength = strength;
	spring->damping = damping;

	return spring;
}

/*!
	\brief Adds a spring to the world.
*/

void slWorldAddSpring(slWorld *w, slSpring *s) {
	w->springs.push_back(s);
}

/*!
	\brief Removes a spring from the world.
*/

void slWorldRemoveSpring(slWorld *w, slSpring *s) {
	std::vector<slSpring*>::iterator i;

	if(!s) return;

	i = std::find(w->springs.begin(), w->springs.end(), s);
	slSpringFree(*i);
	w->springs.erase(i);
}

/*!
	\brief Draws the springs.
*/

void slWorldDrawSprings(slWorld *w) {
	slVector pos1, pos2;
	std::vector<slSpring*>::iterator i = w->springs.begin();

	glLineStipple(1, 0xaaaa);
	glEnable(GL_LINE_STIPPLE);
	
	glEnable(GL_BLEND);
	glLineWidth(1.8);
	glColor4f(0.0, 0.0, 0.0, 0.8);

	glBegin(GL_LINES);

	for(i = w->springs.begin(); i != w->springs.end(); i++) {
		slSpring *spring = *i;

		slPositionVertex(&spring->link1->position, &spring->point1, &pos1);
		slPositionVertex(&spring->link2->position, &spring->point2, &pos2);

		glVertex3f(pos1.x, pos1.y, pos1.z);
		glVertex3f(pos2.x, pos2.y, pos2.z);
	}

	glEnd();

	glDisable(GL_LINE_STIPPLE);
}

void slSpringFree(slSpring *s) {
	delete s;
}
