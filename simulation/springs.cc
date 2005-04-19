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

double slSpringGetCurrentLength(slSpring *spring) {
	slVector pos1, pos2, toV1;

	if(!spring->_src || !spring->_dst) return 0.0;

	slPositionVertex(&spring->_src->position, &spring->_point1, &pos1);
	slPositionVertex(&spring->_dst->position, &spring->_point2, &pos2);

	slVectorSub(&pos1, &pos2, &toV1);

	return slVectorLength(&toV1);
}

void slSpring::draw(slCamera *camera) {
	slVector x, y;

	if(!_src || !_dst) return;
	if(!_stipple) return;

	slPositionVertex(&_src->position, &_point1, &x);
	slPositionVertex(&_dst->position, &_point2, &y);

	glLineStipple(2, _stipple);
	glEnable(GL_LINE_STIPPLE);

	glColor4f(_color.x, _color.y, _color.z, 0.8);

	glBegin(GL_LINES);

	glVertex3f(x.x, x.y, x.z);
	glVertex3f(y.x, y.y, y.z);

	glEnd();

	glDisable(GL_LINE_STIPPLE);
}


/*!
	\brief Applies the spring forces between two bodies.
*/

void slSpring::step(double step) {
	slVector pos1, pos2, toV1, vel1, vel2, velocity, point, force;
	double distance, normVelocity, damping;
	slVector linearVel, angularVel;
	slLink *l1, *l2;

	if(!_src || !_dst) return;

	l1 = (slLink*)_src;
	l2 = (slLink*)_dst;

	slPositionVertex(&_src->position, &_point1, &pos1);
	slPositionVertex(&_dst->position, &_point2, &pos2);

	slVectorSub(&pos1, &pos2, &toV1);

	distance = slVectorLength(&toV1);

	////////////////////////////////////////////////

	slVectorXform(l1->position.rotation, &_point1, &point);
	slLinkGetVelocity(l1, &linearVel, &angularVel);
	slVelocityAtPoint(&linearVel, &angularVel, &point, &vel1);

	slVectorXform(l2->position.rotation, &_point2, &point);
	slLinkGetVelocity(l2, &linearVel, &angularVel);
	slVelocityAtPoint(&linearVel, &angularVel, &point, &vel2);

	slVectorSub(&vel1, &vel2, &velocity);

	normVelocity = slVectorDot(&velocity, &toV1);

	////////////////////////////////////////////////

	distance -= _length;

	if(_mode == SPRING_MODE_CONTRACT_ONLY && distance < 0.0) return;
	else if(_mode == SPRING_MODE_EXPAND_ONLY && distance > 0.0) return;

	damping = _damping * normVelocity;

	_force = (distance * _strength) + damping;

	slVectorNormalize(&toV1);
	slVectorMul(&toV1, _force, &force);

	////////////////////////////////////////////////

	dBodyAddForceAtPos(l2->odeBodyID, 
			force.x, force.y, force.z, pos1.x, pos1.y, pos1.z);

	dBodyAddForceAtPos(l1->odeBodyID, 
			-force.x, -force.y, -force.z, pos2.x, pos2.y, pos2.z);
}

void slSpringSetStrength(slSpring *s, double strength) {
	s->_strength = strength;
}

void slSpringSetDamping(slSpring *s, double damping) {
	s->_damping = damping;
}

void slSpringSetLength(slSpring *s, double length) {
	s->_length = length;
}

void slSpringSetMode(slSpring *s, int mode) {
	s->_mode = mode;
}

/*!
	\brief Creates a new spring.
*/

slSpring *slSpringNew(slWorld *w, slLink *l1, slLink *l2, slVector *p1, slVector *p2, double length, double strength, double damping) {
	slSpring *spring;

	spring = new slSpring;

	slVectorCopy(p1, &spring->_point1);
	slVectorCopy(p2, &spring->_point2);

	spring->_src = l1;
	spring->_dst = l2;

	spring->_length = length;
	spring->_strength = strength;
	spring->_damping = damping;
	spring->_force = 0;

	slWorldAddConnection(w, spring);

	return spring;
}

double slSpringGetLength(slSpring *spring) {
	return spring->_length;
}

double slSpringGetForce(slSpring *spring) {
	return spring->_force;
}

void slSpringFree(slSpring *s) {
	delete s;
}
