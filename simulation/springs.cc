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
#include "glIncludes.h"
#include "springs.h"
#include "world.h"
#include "link.h"
#include "shape.h"

/*!
	\brief Draws the spring connecting two objects.
*/

void slSpring::draw(slCamera *camera) {
	slVector x, y;

	if(!_src || !_dst) return;
	if(!_stipple) return;

	slPositionVertex(&_src->_position, &_point1, &x);
	slPositionVertex(&_dst->_position, &_point2, &y);

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
	\brief Steps the spring forward in time and applies the spring forces 
	between the attached bodies.
*/

void slSpring::step(double step) {
	slVector pos1, pos2, toV1, vel1, vel2, velocity, point, force;
	double distance, normVelocity, damping;
	slVector linearVel, angularVel;
	slLink *l1, *l2;

	if(!_src || !_dst) return;

	l1 = (slLink*)_src;
	l2 = (slLink*)_dst;

	slPositionVertex(&_src->_position, &_point1, &pos1);
	slPositionVertex(&_dst->_position, &_point2, &pos2);
	slVectorSub(&pos1, &pos2, &toV1);
	distance = slVectorLength(&toV1);

	////////////////////////////////////////////////

	slVectorXform(l1->_position.rotation, &_point1, &point);
	l1->getVelocity(&linearVel, &angularVel);
	slVelocityAtPoint(&linearVel, &angularVel, &point, &vel1);

	slVectorXform(l2->_position.rotation, &_point2, &point);
	l2->getVelocity(&linearVel, &angularVel);
	slVelocityAtPoint(&linearVel, &angularVel, &point, &vel2);

	slVectorSub(&vel1, &vel2, &velocity);

	normVelocity = slVectorDot(&velocity, &toV1);

	////////////////////////////////////////////////

	distance -= _length;

	if(_mode == SPRING_MODE_CONTRACT_ONLY && distance < 0.0) return;
	else if(_mode == SPRING_MODE_EXPAND_ONLY && distance > 0.0) return;

	damping = _damping * normVelocity;

	_force = (distance * _strength) + damping;

	if(_maxForce != 0.0) {
		 if(_force > _maxForce) _force = _maxForce;
		else if(_force < -_maxForce) _force = -_maxForce;
	}

	slVectorNormalize(&toV1);
	slVectorMul(&toV1, _force, &force);

	////////////////////////////////////////////////

	dBodyAddForceAtPos(l2->_odeBodyID, 
			force.x, force.y, force.z, pos1.x, pos1.y, pos1.z);

	dBodyAddForceAtPos(l1->_odeBodyID, 
			-force.x, -force.y, -force.z, pos2.x, pos2.y, pos2.z);
}

/*!
	\brief Sets the strength of this spring.
*/

void slSpring::setStrength(double strength) {
	_strength = strength;
}

/*!
	\brief Sets the damping constant for this spring.
*/

void slSpring::setDamping(double damping) {
	_damping = damping;
}

/*!
	\brief Sets the natural length for this spring.  

	The natural length is the length at which this spring applies no 
	forces to the objects it is attached to.
*/

void slSpring::setLength(double length) {
	_length = length;
}

void slSpring::setMode(int mode) {
	_mode = mode;
}

/*!
	\brief Creates a new spring.
*/

slSpring::slSpring(slWorld *w, slLink *l1, slLink *l2, slVector *p1, slVector *p2, double length, double strength, double damping) {
	slVectorSet(&_color, 0, 0, 0);

	_stipple = 0xaaaa;
	_maxForce = 0;

	_mode = 0;

	slVectorCopy(p1, &_point1);
	slVectorCopy(p2, &_point2);

	_src = l1;
	_dst = l2;

	_length = length;
	_strength = strength;
	_damping = damping;
	_force = 0;

	w->addConnection( this );
}

double slSpring::getCurrentLength() {
	slVector pos1, pos2, toV1;

	if(!_src || !_dst) return 0.0;

	slPositionVertex(&_src->_position, &_point1, &pos1);
	slPositionVertex(&_dst->_position, &_point2, &pos2);

	slVectorSub(&pos1, &pos2, &toV1);

	return slVectorLength(&toV1);
}

void slSpring::setMaxForce(double f) {
	_maxForce = f;
}

double slSpring::getLength() {
	return _length;
}

double slSpring::getForce() {
	return _force;
}
