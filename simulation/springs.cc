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

double slSpringGetCurrentLength(slSpring *spring) {
	slVector pos1, pos2, toV1;
	slPositionVertex(&spring->_src->position, &spring->point1, &pos1);
	slPositionVertex(&spring->_dst->position, &spring->point2, &pos2);

	slVectorSub(&pos1, &pos2, &toV1);

	return slVectorLength(&toV1);
}

double slSpringGetLength(slSpring *spring) {
	return spring->length;
}

/*!
	\brief Applies the spring forces between two bodies.
*/

void slSpringApplyForce(slSpring *spring) {
	slVector pos1, pos2, toV1, vel1, vel2, velocity, point, force;
	double distance, normVelocity, damping;
	slVector linearVel, angularVel;
	slLink *l1, *l2;

	l1 = spring->_src;
	l2 = spring->_dst;

	slPositionVertex(&spring->_src->position, &spring->point1, &pos1);
	slPositionVertex(&spring->_dst->position, &spring->point2, &pos2);

	slVectorSub(&pos1, &pos2, &toV1);

	distance = slVectorLength(&toV1);

	////////////////////////////////////////////////

	slVectorXform(spring->_src->position.rotation, &spring->point1, &point);
	slLinkGetVelocity(spring->_src, &linearVel, &angularVel);
	slVelocityAtPoint(&linearVel, &angularVel, &point, &vel1);

	slVectorXform(spring->_dst->position.rotation, &spring->point2, &point);
	slLinkGetVelocity(spring->_dst, &linearVel, &angularVel);
	slVelocityAtPoint(&linearVel, &angularVel, &point, &vel2);

	slVectorSub(&vel1, &vel2, &velocity);

	normVelocity = slVectorDot(&velocity, &toV1);

	////////////////////////////////////////////////

	distance -= spring->length;

	if(spring->mode == SPRING_MODE_CONTRACT_ONLY && distance < 0.0) return;
	else if(spring->mode == SPRING_MODE_EXPAND_ONLY && distance > 0.0) return;

	damping = spring->damping * normVelocity;

	distance = (distance * spring->strength) + damping;

	slVectorNormalize(&toV1);
	slVectorMul(&toV1, distance, &force);

	////////////////////////////////////////////////

	dBodyAddForceAtPos(l2->odeBodyID, 
			force.x, force.y, force.z, pos1.x, pos1.y, pos1.z);

	dBodyAddForceAtPos(l1->odeBodyID, 
			-force.x, -force.y, -force.z, pos2.x, pos2.y, pos2.z);
}

void slSpringSetStrength(slSpring *s, double strength) {
	s->strength = strength;
}

void slSpringSetDamping(slSpring *s, double damping) {
	s->strength = damping;
}

void slSpringSetLength(slSpring *s, double length) {
	s->length = length;
}

void slSpringSetMode(slSpring *s, int mode) {
	s->mode = mode;
}

/*!
	\brief Creates a new spring.
*/

slSpring *slSpringNew(slLink *l1, slLink *l2, slVector *p1, slVector *p2, double length, double strength, double damping) {
	slSpring *spring;

	spring = new slSpring;

	slVectorCopy(p1, &spring->point1);
	slVectorCopy(p2, &spring->point2);

	spring->_src = l1;
	spring->_dst = l2;

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

void slWorldDrawSprings(slWorld *w) {
	std::vector<slSpring*>::iterator i;

	for(i = w->springs.begin(); i != w->springs.end(); i++ ) (*i)->draw();
}

void slSpring::draw() {
	slVector pos1, pos2;

	glLineStipple(1, 0xaaaa);
	glEnable(GL_LINE_STIPPLE);
	
	glEnable(GL_BLEND);
	glLineWidth(1.8);
	glColor4f(0.0, 0.0, 0.0, 0.8);

	glBegin(GL_LINES);

	slPositionVertex(&_src->position, &point1, &pos1);
	slPositionVertex(&_dst->position, &point2, &pos2);

	glVertex3f(pos1.x, pos1.y, pos1.z);
	glVertex3f(pos2.x, pos2.y, pos2.z);

	glEnd();

	glDisable(GL_LINE_STIPPLE);
}

void slSpringFree(slSpring *s) {
	delete s;
}
