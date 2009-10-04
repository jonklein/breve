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

/*! \addtogroup InternalFunctions */
/*@{*/

#include "kernel.h"

int brIsa( brEval args[], brEval *target, brInstance *bi ) {
	brObject *stringType;

	stringType = brObjectFind( bi->engine, BRSTRING( &args[0] ) );

	if( !stringType ) {
		target->set( 0 );
		return EC_OK;
	}

	target->set( brObjectIsSubclass( bi->object, stringType ) );

	return EC_OK;
}

/**
 *	\brief Determines whether a certain instance understand a certain method.
 */

int brORespondsTo( brEval args[], brEval *target, brInstance *i ) {
	brInstance *instance = BRINSTANCE( &args[0] );
	char *method = BRSTRING( &args[1] );

	brMethod *m = brMethodFindWithArgRange( instance->object, method, NULL, 0, 99 );

	if ( m ) {
		target->set( 1 );
		delete m;
	} else 
		target->set( 0 );

	return EC_OK;
}

/**
 * \brief Returns an instance's class name as a string.
 * 
 *  string objectName().
 */

int brIObjectName( brEval args[], brEval *target, brInstance *i ) {

	target->set( BRINSTANCE( &args[0] )->object->name );

	return EC_OK;
}

/*
*/

int brITypeof( brEval args[], brEval *target, brInstance *i ) {

	target->set( brAtomicTypeStrings[ args[ 0 ].type() ] );

	return EC_OK;
}

/*!
	\brief Adds an observer to the calling object's observer list.

	When the calling instance announces a notification matching the
	notification name, the specified instance is executed for the
	observer.

	void addObserver(object dependency, string notification, string method).
*/

int brIAddObserver( brEval args[], brEval *target, brInstance *bi ) {
	brInstance *observedObject = BRINSTANCE( &args[0] );
	char *notification = BRSTRING( &args[1] );
	char *method = BRSTRING( &args[2] );

	target->set( brInstanceAddObserver( observedObject, bi, notification, method ) );

	return EC_OK;
}

/*!
	\brief Removes an observer from the calling object's observer list.

	Removes only the entry for the specified notificationName.

	void removeObserver(object observer, string notificationName).
*/

int brIRemoveObserver( brEval args[], brEval *target, brInstance *i ) {
	brInstance *observer = BRINSTANCE( &args[0] );
	char *notification = BRSTRING( &args[1] );

	if( observer )
		brEngineRemoveInstanceObserver( observer, i, notification );

	return EC_OK;
}

/*!
	\brief Add a collision handler for an object type.

	Note that though this method takes a single instance as input, it will
	add a collision handler for all objects of the given type.

	void addCollisionHandler(object caller, string otherObject, string methodName).
*/

int brIAddCollisionHandler( brEval args[], brEval *target, brInstance *i ) {
	brObject *handler, *collider;
	brInstance *caller;

	caller = BRINSTANCE( &args[0] );

	handler = caller->object;

	collider = brObjectFindWithPreferredType( i->engine, BRSTRING( &args[1] ), i->object->type->_typeSignature );

	if ( !collider ) {
		slMessage( DEBUG_ALL, "addCollisionCallback: Cannot locate class \"%s\"\n", BRSTRING( &args[1] ) );
		return EC_ERROR;
	}

	brObjectAddCollisionHandler( handler, collider, BRSTRING( &args[2] ) );

	return EC_OK;
}

int brISetIgnoreCollisionsWith( brEval args[], brEval *target, brInstance *i ) {
	brObject *handler, *collider;
	brInstance *caller;

	caller = BRINSTANCE( &args[0] );

	handler = caller->object;

	collider = brObjectFind( i->engine, BRSTRING( &args[1] ) );

	if ( !collider ) {
		slMessage( DEBUG_ALL, "addCollisionCallback: Cannot locate class \"%s\"\n", BRSTRING( &args[1] ) );
		return EC_ERROR;
	}

	brObjectSetIgnoreCollisionsWith( handler, collider, BRINT( &args[2] ) );

	return EC_OK;
}

/*!
	\brief Sends a notification, which triggers an event for all observers.

	void notify(string notification).
*/

int brINotify( brEval args[], brEval *target, brInstance *i ) {
	const brEval *newargs[2];
	brEval sEval, iEval, t;
	char *notification = BRSTRING( &args[0] );
	brObserver *observer;
	int r, count;

	sEval.set( notification );
	iEval.set( i );

	newargs[0] = &iEval;
	newargs[1] = &sEval;

	// for each observer, check to see if what it's observing

	for( unsigned int n = 0; n < i->observers.size(); n++ ) {
		observer = i->observers[ n ];

		if ( !strcmp( notification, observer->notification ) ) {
			count = observer->method->argumentCount;

			r = brMethodCall( observer->instance, observer->method, newargs, &t );
		}
	}

	return EC_OK;
}

/*!
	\brief Returns the controller object.

	object getController().
*/

int brIGetController( brEval args[], brEval *target, brInstance *i ) {

	target->set( i->engine-> getController() );

	return EC_OK;
}

/*@}*/

void breveInitObjectFunctions( brNamespace *n ) {
	brNewBreveCall( n, "typeof", brITypeof, AT_STRING, AT_UNDEFINED, 0 );
	brNewBreveCall( n, "isa", brIsa, AT_INT, AT_STRING, 0 );
	brNewBreveCall( n, "respondsTo", brORespondsTo, AT_INT, AT_INSTANCE, AT_STRING, 0 );
	brNewBreveCall( n, "objectName", brIObjectName, AT_STRING, AT_INSTANCE, 0 );
	brNewBreveCall( n, "addObserver", brIAddObserver, AT_INT, AT_INSTANCE, AT_STRING, AT_STRING, 0 );
	brNewBreveCall( n, "removeObserver", brIRemoveObserver, AT_NULL, AT_INSTANCE, AT_STRING, 0 );
	brNewBreveCall( n, "notify", brINotify, AT_NULL, AT_STRING, 0 );

	brNewBreveCall( n, "addCollisionHandler", brIAddCollisionHandler, AT_NULL, AT_INSTANCE, AT_STRING, AT_STRING, 0 );
	brNewBreveCall( n, "setIgnoreCollisionsWith", brISetIgnoreCollisionsWith, AT_NULL, AT_INSTANCE, AT_STRING, AT_INT, 0 );

	brNewBreveCall( n, "getController", brIGetController, AT_INSTANCE, 0 );
}
