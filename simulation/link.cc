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
#include "world.h"
#include "link.h"
#include "multibody.h"
#include "joint.h"
#include "vclip.h"
#include "vclipData.h"

/*!
	\brief Creates a new slLink.
*/

slLink::slLink( slWorld *w ) : slWorldObject() {
	_type = WO_LINK;

	_odeBodyID = dBodyCreate( w->_odeWorldID );

	_simulate = 0;
	_currentState = 0;

	_moved = 1;
	_justMoved = 1;

	_clipNumber = 0;

	_mobile = -1;

	memset( &_stateVector[0], 0, sizeof( slLinkIntegrationPosition ) );
	memset( &_stateVector[1], 0, sizeof( slLinkIntegrationPosition ) );

	slQuatIdentity( &_stateVector[0].rotQuat );
	slQuatIdentity( &_stateVector[1].rotQuat );

	_multibody = NULL;

	slsVectorZero( &_acceleration );
	slsVectorZero( &_velocity );
	slsVectorZero( &_externalForce );
}

slLink::~slLink() {
	if ( _multibody && _multibody->getRoot() == this ) 
		_multibody->setRoot( NULL );

	// This is a bad situation here: slJointBreak modifies the
	// joint list.  I intend to fix this.

	while ( _inJoints.size() != 0 )( *_inJoints.begin() )->breakJoint();
	while ( _outJoints.size() != 0 )( *_outJoints.begin() )->breakJoint();

	dBodyDestroy( _odeBodyID );
}

void slLink::connectedLinks( std::vector<slLink*> *list, int mbOnly ) {
	std::vector<slJoint*>::iterator ji;

	if ( std::find( list->begin(), list->end(), this ) != list->end() ) return;

	list->push_back( this );

	for ( ji = _outJoints.begin(); ji != _outJoints.end(); ji++ ) {
		if (( *ji )->_child && ( !mbOnly || ( *ji )->_isMbJoint ) )
			( *ji )->_child->connectedLinks( list, mbOnly );
	}

	for ( ji = _inJoints.begin(); ji != _inJoints.end(); ji++ ) {
		if (( *ji )->_parent && ( !mbOnly || ( *ji )->_isMbJoint ) )
			( *ji )->_parent->connectedLinks( list, mbOnly );
	}
}

void slLink::step( slWorld *world, double step ) {
	_moved = 1;

	if ( _simulate ) {
		updatePositionFromODE();
		applyJointControls();

		if ( world->_detectCollisions ) updateBoundingBox();
	} else {
		if ( _mobile ) {
			world->integrator( world, this, &step, 0 );
			swapConfig();
			updatePosition();

			if ( world->_detectCollisions ) updateBoundingBox();
		} else {
			// if( !_justMoved) _moved = 0;
			// else _justMoved = 0;
		}
	}
}

/*!
	\brief Set a link to a certain shape.

	Shapes may be shared among several objects.
*/

void slLink::setShape( slShape *s ) {
	slWorldObject::setShape( s );

	// dMatrix3 is actually 4x3.

	dMassSetZero( &_massData );

	_massData.mass = _shape->_mass;
	_massData.c[0] = 0.0;
	_massData.c[1] = 0.0;
	_massData.c[2] = 0.0;
	_massData.c[3] = 0.0;
	_massData.I[0] = s->_inertia[0][0];
	_massData.I[1] = s->_inertia[0][1];
	_massData.I[2] = s->_inertia[0][2];
	_massData.I[3] = 0.0;
	_massData.I[4] = s->_inertia[1][0];
	_massData.I[5] = s->_inertia[1][1];
	_massData.I[6] = s->_inertia[1][2];
	_massData.I[7] = 0.0;
	_massData.I[8] = s->_inertia[2][0];
	_massData.I[9] = s->_inertia[2][1];
	_massData.I[10] = s->_inertia[2][2];
	_massData.I[11] = 0.0;

	dBodySetMass( _odeBodyID, &_massData );
}

void slLink::setLabel( const char *inLabel ) {
	_label = inLabel;
}

void slLink::setLocation( slVector *inLocation ) {
	_justMoved = 1;

	if ( _simulate ) {
		if ( _odeBodyID ) dBodySetPosition( _odeBodyID, inLocation->x, inLocation->y, inLocation->z );
	} else {
		slVectorCopy( inLocation, &_stateVector[ _currentState].location );
		slVectorCopy( inLocation, &_stateVector[!_currentState].location );
	}

	slWorldObject::setLocation( inLocation );
}

void slLink::setRotation( double inRotation[3][3] ) {
	_justMoved = 1;

	if ( _simulate ) {
		dReal r[ 16 ];

		slSlToODEMatrix( inRotation, r );
		dBodySetRotation( _odeBodyID, r );
	} else {
		slQuat q;

		slMatrixToQuat( inRotation, &q );

		slQuatCopy( &q, &_stateVector[  _currentState ].rotQuat );
		slQuatCopy( &q, &_stateVector[ !_currentState ].rotQuat );
	}

	slWorldObject::setRotation( inRotation );
}

void slLink::getVelocity( slVector *linear, slVector *rotational ) {
	slLinkIntegrationPosition *config;

	if ( !_simulate ) {
		config = &_stateVector[_currentState];

		if ( linear ) slVectorCopy( &config->velocity.b, linear );

		if ( rotational ) slVectorCopy( &config->velocity.a, rotational );
	} else {
		if ( linear ) {
			const dReal *v = dBodyGetLinearVel( _odeBodyID );

			linear->x = v[0];
			linear->y = v[1];
			linear->z = v[2];
		}

		if ( rotational ) {
			const dReal *v = dBodyGetAngularVel( _odeBodyID );

			rotational->x = v[0];
			rotational->y = v[1];
			rotational->z = v[2];
		}
	}
}

void slLink::getRotation( double m[3][3] ) {
	const dReal *rotation;

	if ( !_simulate ) {
		slMatrixCopy( &_position.rotation, m );
	} else {
		rotation = dBodyGetRotation( _odeBodyID );
		slODEToSlMatrix(( dReal* )rotation, m );
	}
}

void slLink::setVelocity( slVector *velocity, slVector *rotational ) {
	slLinkIntegrationPosition *config;

	if ( !_simulate ) {
		config = ( slLinkIntegrationPosition* ) & _stateVector[_currentState];

		if ( velocity ) {
			slVectorCopy( velocity, &config->velocity.b );

			if ( !slVectorIsZero( velocity ) ) _mobile = 1;
		}

		if ( rotational ) {
			slVectorCopy( rotational, &config->velocity.a );

			if ( !slVectorIsZero( rotational ) ) _mobile = 1;
		}

	} else {
		if ( velocity ) dBodySetLinearVel( _odeBodyID, velocity->x, velocity->y, velocity->z );

		if ( rotational ) dBodySetAngularVel( _odeBodyID, rotational->x, rotational->y, rotational->z );
	}
}

void slLink::setAcceleration( slVector *linear, slVector *rotational ) {
	if ( linear ) {
		slVectorCopy( linear, &_acceleration.b );

		if ( !slVectorIsZero( linear ) ) _mobile = 1;
	}

	if ( rotational ) {
		slVectorCopy( rotational, &_acceleration.a );

		if ( !slVectorIsZero( rotational ) ) _mobile = 1;
	}
}

void slLink::applyForce( slVector *f, slVector *t ) {
	if ( f ) 
		dBodySetForce( _odeBodyID, f->x, f->y, f->z );

	if ( t ) 
		dBodySetTorque( _odeBodyID, t->x, t->y, t->z );
}

void slLink::updatePositionFromODE() {
	const dReal *positionV;

	const dReal *rotationV;

	if ( !_simulate || !_odeBodyID ) return;

	positionV = dBodyGetPosition( _odeBodyID );
	rotationV = dBodyGetRotation( _odeBodyID );

	_position.location.x = positionV[0];
	_position.location.y = positionV[1];
	_position.location.z = positionV[2];

	slODEToSlMatrix( ( dReal* )rotationV, _position.rotation );
}

/*!
	\brief Checks for collisions between this link and all other links
	in the same \ref slMultibody.

	Returns 1 if any such collisions are occurring.
*/

int slLink::checkSelfPenetration( slWorld *world ) {
	slVclipData *vc;
	std::vector<slLink*> links;
	std::vector<slLink*>::iterator li;

	if ( !world->_initialized ) slVclipDataInit( world );

	vc = world->_clipData;

	this->connectedLinks( &links, 0 );

	for ( li = links.begin(); li != links.end(); li++ ) {
		slLink *link2 = *li;

		if ( this != link2 ) {
			slCollisionCandidate c( vc, _clipNumber, link2->_clipNumber );

			slPairFlags *flags = slVclipPairFlags( vc, _clipNumber, link2->_clipNumber );

			if (( slVclipFlagsShouldTest( *flags ) ) && vc->testPair( &c, NULL, 0 ) ) {
				return 1;
			}
		}
	}

	return 0;
}

/*!
	\brief Checks for collisions between this link and all other objects.

	Returns 1 if any such collisions are occurring.
*/

std::vector< void* > slLink::userDataForPenetratingObjects( slWorld *w ) {
	slVclipData *vc;
	unsigned int ln;
	unsigned int n;
	std::vector< void* > penetrations;

	if ( !w->_initialized ) slVclipDataInit( w );

	ln = _clipNumber;

	vc = w->_clipData;

	for ( n = 0;n < vc->_count;n++ ) {
		if ( ln != n ) {
			slCollisionCandidate c( vc, ln, n );
			slPairFlags *flags = slVclipPairFlags( vc, ln, n );

			if (( slVclipFlagsShouldTest( *flags ) && *flags & BT_SIMULATE ) && vc->testPair( &c, NULL, 0 ) ) {
				penetrations.push_back( w->_objects[ n ]->getCallbackData() );
			}
		}
	}

	// who wants to make a penetration joke?

	return penetrations;
}

/*!
	\brief Applies external torques and forces to a link and its joints.
*/

void slLink::applyJointControls() {
	slJoint *joint;
	double angle, speed;
	double newSpeed;
	std::vector<slJoint*>::iterator ji;

	dBodySetTorque( _odeBodyID, 0, 0, 0 );
	dBodySetForce( _odeBodyID, 0, 0, 0 );

	applyForce( &_externalForce.a, &_externalForce.b );

	for ( ji = _inJoints.begin(); ji != _inJoints.end(); ji++ ) {
		joint = *ji;

		if ( joint->_type == JT_REVOLUTE ) {
			angle = dJointGetHingeAngle( joint->_odeJointID );
			speed = dJointGetHingeAngleRate( joint->_odeJointID );
		} else if ( joint->_type == JT_PRISMATIC ) {
			angle = dJointGetSliderPosition( joint->_odeJointID );
			speed = dJointGetSliderPositionRate( joint->_odeJointID );
		} else {
			angle = 0;
			speed = 0;
		}

		newSpeed = joint->_targetSpeed;

		if ( joint->_kSpring != 0.0 ) {
			double delta = 0;

			if ( angle > joint->_sMax ) {
				delta = joint->_kSpring * ( joint->_sMax - angle );
			} else if ( angle < joint->_sMin ) {
				delta = joint->_kSpring * ( joint->_sMin - angle );
			}

			newSpeed += delta;
		}

		if ( joint->_kDamp != 0.0 ) dJointAddHingeTorque( joint->_odeJointID, -speed * joint->_kDamp );

		if ( joint->_type == JT_REVOLUTE ) dJointSetHingeParam( joint->_odeJointID, dParamVel, newSpeed );
		else if ( joint->_type == JT_PRISMATIC ) dJointSetSliderParam( joint->_odeJointID, dParamVel, newSpeed );
	}
}

/*!
	\brief Enables physical simulation for a link.
*/

void slLink::enableSimulation() {
	if ( _simulate == 1 ) return;

	_justMoved = 1;

	_simulate = 1;

	dBodySetLinearVel( _odeBodyID, _velocity.b.x, _velocity.b.y, _velocity.b.z );

	dBodySetAngularVel( _odeBodyID, _velocity.a.x, _velocity.a.y, _velocity.a.z );

	dBodySetPosition( _odeBodyID, _position.location.x, _position.location.y, _position.location.z );
}

/*!
	\brief Disables physical simulation for a link.
*/

void slLink::disableSimulation() {
	const dReal *odePos, *odeRot;

	const dReal *odeLinear, *odeAngular;

	if ( _simulate == 0 ) return;

	_simulate = 0;

	odePos = dBodyGetPosition( _odeBodyID );
	odeRot = dBodyGetRotation( _odeBodyID );

	odeLinear = dBodyGetLinearVel( _odeBodyID );
	odeAngular = dBodyGetAngularVel( _odeBodyID );

	_position.location.x = odePos[0];
	_position.location.y = odePos[1];
	_position.location.z = odePos[2];

	slMatrix newRotation;

	slODEToSlMatrix(( dReal* )odeRot, newRotation );

	setLocation( &_position.location );
	setRotation( newRotation );

	_velocity.a.x = odeAngular[0];
	_velocity.a.y = odeAngular[1];
	_velocity.a.z = odeAngular[2];
	_velocity.b.x = odeLinear[0];
	_velocity.b.y = odeLinear[1];
	_velocity.b.z = odeLinear[2];
}

/*!
	\brief Updates the position field from the integration data.
*/

void slLink::updatePosition() {
	// copy the velocity and position info from the state slVector

	slLinkIntegrationPosition *f = &_stateVector[_currentState];

	slVectorCopy( &f->location, &_position.location );
	slQuatNormalize( &f->rotQuat );
	slQuatToMatrix( &f->rotQuat, _position.rotation );

	slVectorCopy( &f->velocity.a, &_velocity.a );
	slVectorInvXform( _position.rotation, &f->velocity.b, &_velocity.b );
}

/*!
	\brief Creates a joint between two links.
*/

slJoint *slLink::link( slWorld *world, slLink *parent, int jointType, slVector *normal, slVector *plinkPoint, slVector *clinkPoint, double rotation[3][3], bool useCurrentRotation ) {
	slJoint *joint;
	slVector tp, tn, axis2;
	dBodyID pBodyID, cBodyID;

	// I had to switch the order of arguments to dJointAttach to work-around a problem
	// in ODE, so now we have to reverse the normal to preserve the behavior of the
	// previous versions of breve.

	slVectorMul( normal, -1.0, normal );

	joint = new slJoint;

	// figure out if this is a multibody joint or not.

	if ( !parent || ( _multibody && parent->_multibody ) ) {
		joint->_isMbJoint = 0;
	} else {
		if ( _multibody ) parent->_multibody = _multibody;
		else _multibody = parent->_multibody;

		joint->_isMbJoint = 1;
	}

	if ( parent ) pBodyID = parent->_odeBodyID;
	else pBodyID = NULL;

	cBodyID = _odeBodyID;

	switch ( jointType ) {

		case JT_BALL:
			joint->_odeJointID = dJointCreateBall( world->_odeWorldID, NULL );

			joint->_odeMotorID = dJointCreateAMotor( world->_odeWorldID, NULL );

			dJointAttach( joint->_odeMotorID, cBodyID, pBodyID );

			break;

		case JT_UNIVERSAL:
			joint->_odeJointID = dJointCreateUniversal( world->_odeWorldID, NULL );

			joint->_odeMotorID = dJointCreateAMotor( world->_odeWorldID, NULL );

			dJointAttach( joint->_odeMotorID, cBodyID, pBodyID );

			break;

		case JT_REVOLUTE:
			joint->_odeJointID = dJointCreateHinge( world->_odeWorldID, NULL );

			break;

		case JT_PRISMATIC:
			joint->_odeJointID = dJointCreateSlider( world->_odeWorldID, NULL );

			dJointSetSliderParam( joint->_odeJointID, dParamHiStop, 2.0 );

			dJointSetSliderParam( joint->_odeJointID, dParamLoStop, -2.0 );

			break;

		case JT_FIX:
			joint->_odeJointID = dJointCreateFixed( world->_odeWorldID, NULL );

			break;
	}

	dJointAttach( joint->_odeJointID, cBodyID, pBodyID );

	// transform the normal to the parent coordinates.

	if ( parent ) {
		slVectorXform( parent->_position.rotation, normal, &tn );
		slVectorXform( parent->_position.rotation, plinkPoint, &tp );
	} else {
		slVectorCopy( normal, &tn );
		slVectorCopy( plinkPoint, &tp );
	}

	// Note: most of the joint configuration is now done with by calling
	// setNormal and setLinkPoints below, instead of doing it in this method.

	switch ( jointType ) {

		case JT_BALL:
			slVectorCross( &tp, &tn, &axis2 );

			slVectorNormalize( &axis2 );

			slVectorNormalize( &tp );

			dJointSetAMotorNumAxes( joint->_odeMotorID, 3 );
			dJointSetAMotorAxis( joint->_odeMotorID, 0, 1, tn.x, tn.y, tn.z );
			dJointSetAMotorAxis( joint->_odeMotorID, 1, 0, 0, 0, 0 );
			dJointSetAMotorAxis( joint->_odeMotorID, 2, 2, tp.x, tp.y, tp.z );
			dJointSetAMotorMode( joint->_odeMotorID, dAMotorEuler );

			// See the note in \ref slJoint::setLimits.  These odd stops are required for stability.

			dJointSetAMotorParam( joint->_odeMotorID, dParamLoStop2, -( M_PI / 2.0 ) + 0.35 );
			dJointSetAMotorParam( joint->_odeMotorID, dParamHiStop2, ( M_PI / 2.0 ) - 0.35 );

			break;

		case JT_UNIVERSAL:
			slVectorCross( &tp, &tn, &axis2 );

			slVectorNormalize( &axis2 );

			dJointSetUniversalAxis1( joint->_odeJointID, tn.x, tn.y, tn.z );
			dJointSetUniversalAxis2( joint->_odeJointID, axis2.x, axis2.y, axis2.z );
			dJointSetAMotorAxis( joint->_odeMotorID, 0, 2, tn.x, tn.y, tn.z );
			dJointSetAMotorAxis( joint->_odeMotorID, 2, 2, axis2.x, axis2.y, axis2.z );
			dJointSetAMotorMode( joint->_odeMotorID, dAMotorEuler );

			break;

		case JT_FIX:
			dJointSetFixed( joint->_odeJointID );

			break;
	}

	if ( parent ) parent->_outJoints.push_back( joint );

	_inJoints.push_back( joint );

	joint->_parent = parent;

	joint->_child = this;

	joint->_type = jointType;

	joint->setLinkPoints( plinkPoint, clinkPoint, rotation, !useCurrentRotation );

	joint->setNormal( normal );

	if ( parent && parent->_multibody ) parent->_multibody->update();

	if ( _multibody && ( !parent || ( _multibody != parent->_multibody ) ) ) _multibody->update();

//	bzero( &joint->_feedback, sizeof( dJointFeedback ) );
	dJointSetFeedback( joint->_odeJointID, &joint->_feedback );

	return joint;
}

void slVelocityAtPoint( slVector *vel, slVector *avel, slVector *atPoint, slVector *d ) {
	slVectorCross( avel, atPoint, d );
	slVectorAdd( d, vel, d );
}

void slLink::getAcceleration( slVector *linear, slVector *rotational ) {
	if ( linear ) slVectorCopy( &_acceleration.b, linear );

	if ( rotational ) slVectorCopy( &_acceleration.a, rotational );
}

void slLink::setForce( slVector *force ) {
	slVectorCopy( force, &_externalForce.a );
}

void slLink::setTorque( slVector *torque ) {
	slVectorCopy( torque, &_externalForce.b );
}

void slLink::nullMultibodiesForConnectedLinks() {
	std::vector< slLink* > links;
	std::vector< slLink* >::iterator li;

	connectedLinks( &links, 1 );

	for ( li = links.begin(); li != links.end(); li++ ) {
		( *li )->_multibody = NULL;
	}

	_multibody = NULL;
}

/*!
	\brief Converts a breve matrix to an ODE matrix.

	Used when getting/setting rotation from the ODE body.
*/


void slSlToODEMatrix( double m[3][3], dReal *r ) {
	r[0] = m[0][0];
	r[1] = m[0][1];
	r[2] = m[0][2];

	r[4] = m[1][0];
	r[5] = m[1][1];
	r[6] = m[1][2];

	r[8] = m[2][0];
	r[9] = m[2][1];
	r[10] = m[2][2];
}

/*!
	\brief Converts an ODE matrix to a breve matrix.

	Used when getting/setting rotation from the ODE body.
*/

void slODEToSlMatrix( dReal *r, double m[3][3] ) {
	m[0][0] = r[0];
	m[0][1] = r[1];
	m[0][2] = r[2];

	m[1][0] = r[4];
	m[1][1] = r[5];
	m[1][2] = r[6];

	m[2][0] = r[8];
	m[2][1] = r[9];
	m[2][2] = r[10];
}
