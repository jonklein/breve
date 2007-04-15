#include "simulation.h"
#include "joint.h"

void slJoint::setERP( double inValue ) {
	switch ( _type ) {
		case JT_PRISMATIC:
			if( _odeJointID ) dJointSetSliderParam( _odeJointID, dParamStopERP, inValue );
			break;

		case JT_REVOLUTE:
			if( _odeJointID ) dJointSetHingeParam( _odeJointID, dParamStopERP, inValue );
			break;

		case JT_UNIVERSAL:
		case JT_BALL:
			if( _odeMotorID ) {
				dJointSetAMotorParam( _odeMotorID, dParamStopERP, inValue );
				dJointSetAMotorParam( _odeMotorID, dParamStopERP2, inValue );
				dJointSetAMotorParam( _odeMotorID, dParamStopERP3, inValue );
			}
			break;
	}
}

void slJoint::setCFM( double inValue ) {
	switch ( _type ) {
		case JT_PRISMATIC:
			if( _odeJointID ) dJointSetSliderParam( _odeJointID, dParamCFM, inValue );
			break;

		case JT_REVOLUTE:
			if( _odeJointID ) dJointSetHingeParam( _odeJointID, dParamCFM, inValue );
			break;

		case JT_UNIVERSAL:
			if( _odeJointID ) dJointSetUniversalParam( _odeJointID, dParamCFM, inValue );
		case JT_BALL:
			if( _odeMotorID ) dJointSetAMotorParam( _odeMotorID, dParamCFM, inValue );
			break;
	}
}

/**
 * \brief Applies a force on a joint's DOFs
 * This can be either force or torque, depending on the type of joint.
 * We do this not by setting the torque and force directly, because it will
 * be cleared after the next physics engine step.  Instead, we compute the
 * values and store them in the externalForce vectors for each body.
 */

void slJoint::applyJointForce( slVector *force ) {
	const dReal *t;

	switch ( _type ) {

		case JT_REVOLUTE:
			dJointAddHingeTorque( _odeJointID, force->x );

			break;

		case JT_PRISMATIC:
			dJointAddSliderForce( _odeJointID, force->x );

			break;

		default:
			if ( _odeMotorID ) dJointAddAMotorTorques( _odeMotorID, force->x, force->y, force->z );

			break;
	}

	t = dBodyGetTorque( _parent->_odeBodyID );

	_parent->_externalForce.b.x = t[0];
	_parent->_externalForce.b.y = t[1];
	_parent->_externalForce.b.z = t[2];
	t = dBodyGetForce( _parent->_odeBodyID );
	_parent->_externalForce.a.x = t[0];
	_parent->_externalForce.a.y = t[1];
	_parent->_externalForce.a.z = t[2];

	if ( _child ) {
		t = dBodyGetTorque( _child->_odeBodyID );
		_child->_externalForce.b.x = t[0];
		_child->_externalForce.b.y = t[1];
		_child->_externalForce.b.z = t[2];
		t = dBodyGetForce( _child->_odeBodyID );
		_child->_externalForce.a.x = t[0];
		_child->_externalForce.a.y = t[1];
		_child->_externalForce.a.z = t[2];
	}
}

void slJoint::setNormal( slVector *normal ) {
	slVector tn;

	// transform the normal to the parent's frame

	if ( _parent ) slVectorXform( _parent->_position.rotation, normal, &tn );
	else slVectorCopy( normal, &tn );

	if ( _type == JT_REVOLUTE ) {
		dJointSetHingeAxis( _odeJointID, tn.x, tn.y, tn.z );
	} else if ( _type == JT_PRISMATIC ) {
		dJointSetSliderAxis( _odeJointID, tn.x, tn.y, tn.z );
	}
}

slJoint::~slJoint() {
	breakJoint();
}

/*!
	\brief Breaks an slJoint struct.

	This triggers an automatic recomputation of multibodies.
*/

void slJoint::breakJoint() {
	slLink *parent = _parent, *child = _child;
	slMultibody *parentBody = NULL, *childBody, *newMb;
	std::vector<slJoint*>::iterator ji;

	if ( !parent && !child ) return;

	childBody = _child->_multibody;

	if ( parent ) parentBody = parent->_multibody;

	if ( parent ) {
		ji = std::find( parent->_outJoints.begin(), parent->_outJoints.end(), this );

		if ( ji != parent->_outJoints.end() ) parent->_outJoints.erase( ji );
	}

	ji = std::find( child->_inJoints.begin(), child->_inJoints.end(), this );

	if ( ji != child->_inJoints.end() ) child->_inJoints.erase( ji );

	dJointAttach( _odeJointID, NULL, NULL );
	dJointDestroy( _odeJointID );

	_odeJointID = NULL;

	if ( _odeMotorID ) {
		dJointAttach( _odeMotorID, NULL, NULL );
		dJointDestroy( _odeMotorID );
		_odeMotorID = NULL;
	}

	memset( &_feedback, 0, sizeof( dJointFeedback ) );

	_child = NULL;

	_parent = NULL;

	if ( parentBody ) parentBody->update();

	if ( childBody && childBody != parentBody ) childBody->update();

	// figure out if the broken links are still part of those bodies
	// ... if not, then try to adopt the links
	// ... if not, then NULL the multibody entries

	if ( parentBody && ( std::find( parentBody->_links.begin(), parentBody->_links.end(), parent ) == parentBody->_links.end() ) ) {
		if (( newMb = slLinkFindMultibody( parent ) ) ) newMb->update();
		else parent->nullMultibodiesForConnectedLinks();
	}

	if ( childBody && ( std::find( childBody->_links.begin(), childBody->_links.end(), child ) == childBody->_links.end() ) ) {
		if (( newMb = slLinkFindMultibody( child ) ) ) newMb->update();
		else child->nullMultibodiesForConnectedLinks();
	}
}

void slJoint::getPosition( slVector *r ) {
	switch ( _type ) {

		case JT_REVOLUTE:
			r->x = dJointGetHingeAngle( _odeJointID );
			r->y = 0;
			r->z = 0;

			break;

		case JT_PRISMATIC:
			r->x = dJointGetSliderPosition( _odeJointID );
			r->y = 0;
			r->z = 0;

			break;

		case JT_BALL:
			r->x = dJointGetAMotorAngle( _odeMotorID, 0 );
			r->y = dJointGetAMotorAngle( _odeMotorID, 1 );
			r->z = dJointGetAMotorAngle( _odeMotorID, 2 );

			break;

		case JT_UNIVERSAL:
			r->x = dJointGetAMotorAngle( _odeMotorID, 0 );
			r->y = dJointGetAMotorAngle( _odeMotorID, 2 );
			r->z = 0;

			break;

		default:
			break;
	}

	return;
}

void slJoint::getVelocity( slVector *velocity ) {
	switch ( _type ) {

		case JT_REVOLUTE:
			velocity->x = dJointGetHingeAngleRate( _odeJointID );
			break;

		case JT_PRISMATIC:
			velocity->x = dJointGetSliderPositionRate( _odeJointID );
			break;

		case JT_BALL:
		case JT_UNIVERSAL:
			velocity->z = 0.0;
			velocity->x = 0.0;
			velocity->y = 0.0;

			break;
	}
}

void slJoint::setVelocity( slVector *speed ) {
	_targetSpeed = speed->x;

	// if(_type == JT_REVOLUTE) dJointSetHingeParam (_odeJointID, dParamVel, speed->x);
	// else if(_type == JT_PRISMATIC) dJointSetSliderParam (_odeJointID, dParamVel, speed->x);

	if ( _type == JT_UNIVERSAL ) {
		dJointSetAMotorParam( _odeMotorID, dParamVel, speed->x );
		dJointSetAMotorParam( _odeMotorID, dParamVel3, speed->y );
	} else if ( _type == JT_BALL ) {
		dJointSetAMotorParam( _odeMotorID, dParamVel, speed->x );
		dJointSetAMotorParam( _odeMotorID, dParamVel2, speed->y );
		dJointSetAMotorParam( _odeMotorID, dParamVel3, speed->z );
	}
}

/*!
	\brief Sets minima and maxima for a joint.

	The minima and maxima are relative to the joint's natural state.
	Since the joint may be 1-, 2- or 3-DOF, only the relevant field
	of the vectors are used.
*/

void slJoint::setLimits( slVector *min, slVector *max ) {
	switch ( _type ) {
		case JT_PRISMATIC:
			dJointSetSliderParam( _odeJointID, dParamStopERP, .1 );
			dJointSetSliderParam( _odeJointID, dParamLoStop, min->x );
			dJointSetSliderParam( _odeJointID, dParamHiStop, max->x );

			break;

		case JT_REVOLUTE:
			dJointSetHingeParam( _odeJointID, dParamStopERP, .1 );
			dJointSetHingeParam( _odeJointID, dParamLoStop, min->x );
			dJointSetHingeParam( _odeJointID, dParamHiStop, max->x );

			break;

		case JT_BALL:
			// The ODE user manual states that there is a singularity when
			// the a1 angle goes outside of the range (-pi / 2, pi / 2).  I
			// see clearly incorrect behaviors already at -pi / 2 + 0.2 and
			// oddities even earlier, so we'll stay far away from that area.

			if ( max->y >= ( M_PI / 2.0 ) - 0.35 ) max->y = ( M_PI / 2.0 ) - 0.35;
			if ( min->y <= ( -M_PI / 2.0 ) + 0.35 ) min->y = -( M_PI / 2.0 ) + 0.35;

			dJointSetAMotorParam( _odeMotorID, dParamLoStop, min->x );
			dJointSetAMotorParam( _odeMotorID, dParamLoStop2, min->y );
			dJointSetAMotorParam( _odeMotorID, dParamLoStop3, min->z );
			dJointSetAMotorParam( _odeMotorID, dParamStopERP, .1 );
			dJointSetAMotorParam( _odeMotorID, dParamStopERP2, .1 );
			dJointSetAMotorParam( _odeMotorID, dParamStopERP3, .1 );
			dJointSetAMotorParam( _odeMotorID, dParamHiStop, max->x );
			dJointSetAMotorParam( _odeMotorID, dParamHiStop2, max->y );
			dJointSetAMotorParam( _odeMotorID, dParamHiStop3, max->z );

			break;

		case JT_UNIVERSAL:
			dJointSetAMotorParam( _odeMotorID, dParamLoStop, min->x );
			dJointSetAMotorParam( _odeMotorID, dParamLoStop3, min->y );
			dJointSetAMotorParam( _odeMotorID, dParamStopERP, .1 );
			dJointSetAMotorParam( _odeMotorID, dParamStopERP3, .1 );
			dJointSetAMotorParam( _odeMotorID, dParamHiStop, max->x );
			dJointSetAMotorParam( _odeMotorID, dParamHiStop3, max->y );
			break;
	}
}

/*!
	\brief Set the maximum torque that a joint can affect.
*/

void slJoint::setMaxTorque( double max ) {
	switch ( _type ) {

		case JT_REVOLUTE:
			dJointSetHingeParam( _odeJointID, dParamFMax, max );
			break;

		case JT_PRISMATIC:
			dJointSetSliderParam( _odeJointID, dParamFMax, max );
			break;

		case JT_UNIVERSAL:
			dJointSetAMotorParam( _odeMotorID, dParamFMax, max );
			dJointSetAMotorParam( _odeMotorID, dParamFMax3, max );
			break;

		case JT_BALL:
			dJointSetAMotorParam( _odeMotorID, dParamFMax, max );
			dJointSetAMotorParam( _odeMotorID, dParamFMax2, max );
			dJointSetAMotorParam( _odeMotorID, dParamFMax3, max );
			break;
	}
}

/*!
	\brief Modifies the link points of a joint.
*/

void slJoint::setLinkPoints( slVector *plinkPoint, slVector *clinkPoint, double rotation[3][3], int first = 0 ) {
	const double *childR;
	const dReal *childP, *linkP;
	dReal idealR[16];
	dReal savedChildR[16], savedChildP[3], offset[3];
	slVector hingePosition, childPosition;
	double ideal[3][3];

	std::vector< slLink*> childChain;
	std::vector< slLink*>::iterator li;

	if ( !_parent || !_child ) return;

	childP = dBodyGetPosition( _child->_odeBodyID );
	childR = ( double* )dBodyGetRotation( _child->_odeBodyID );

	memcpy( savedChildR, childR, sizeof( savedChildR ) );
	memcpy( savedChildP, childP, sizeof( savedChildP ) );

	_child->connectedLinks( &childChain, 0 );

	if ( _parent )
		slMatrixMulMatrix( _parent->_position.rotation, rotation, ideal );
	else
		slMatrixCopy( rotation, ideal );

	slSlToODEMatrix( ideal, idealR );

	// compute the hinge position--the plinkPoint in world coordinates

	if ( _parent ) {
		slVectorXform( _parent->_position.rotation, plinkPoint, &hingePosition );
		slVectorAdd( &hingePosition, &_parent->_position.location, &hingePosition );
	} else {
		slVectorCopy( plinkPoint, &hingePosition );
	}

	// set the ideal positions, so that the anchor
	// command registers the native position

	slVectorXform( ideal, clinkPoint, &childPosition );

	slVectorSub( &hingePosition, &childPosition, &childPosition );

	dJointAttach( _odeJointID, NULL, NULL );

	dBodySetRotation( _child->_odeBodyID, idealR );

	dBodySetPosition( _child->_odeBodyID, childPosition.x, childPosition.y, childPosition.z );

	//
	// NOTE: the child is first in the attachment because of a bug in an older version
	// of ODE.
	//

	if ( _parent ) dJointAttach( _odeJointID, _child->_odeBodyID, _parent->_odeBodyID );
	else dJointAttach( _odeJointID, _child->_odeBodyID, NULL );

	switch ( _type ) {

		case JT_REVOLUTE:
			dJointSetHingeAnchor( _odeJointID, hingePosition.x, hingePosition.y, hingePosition.z );
			break;

		case JT_FIX:
			dJointSetFixed( _odeJointID );
			break;

		case JT_UNIVERSAL:
			dJointSetUniversalAnchor( _odeJointID, hingePosition.x, hingePosition.y, hingePosition.z );
			break;

		case JT_BALL:
			dJointSetBallAnchor( _odeJointID, hingePosition.x, hingePosition.y, hingePosition.z );
			break;

		default:
			break;
	}

	// set the proper positions where the link should actually be at this time

	slVectorXform( _child->_position.rotation, clinkPoint, &childPosition );

	slVectorSub( &hingePosition, &childPosition, &childPosition );

	if ( !first ) 
		dBodySetRotation( _child->_odeBodyID, savedChildR );

	offset[0] = childPosition.x - savedChildP[0];
	offset[1] = childPosition.y - savedChildP[1];
	offset[2] = childPosition.z - savedChildP[2];

	if ( _repositionAll ) {
		for ( li = childChain.begin(); li != childChain.end(); li++ ) {
			if ( *li != _parent ) {
				linkP = dBodyGetPosition(( *li )->_odeBodyID );

				dBodySetPosition(( *li )->_odeBodyID, linkP[0] + offset[0], linkP[1] + offset[1], linkP[2] + offset[2] );

				( *li )->updatePositionFromODE();
			}
		}
	}

	dBodySetPosition( _child->_odeBodyID, childPosition.x, childPosition.y, childPosition.z );

	if ( _parent ) _parent->updatePositionFromODE();

	_child->updatePositionFromODE();
}
