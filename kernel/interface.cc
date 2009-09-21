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

#include "kernel.h"
#include "world.h"
#include "gldraw.h"

/*!
	\brief Adds an item to an object's menu.

	Adds an item to the object's menu with the specified title, that will
	trigger the given method when selected.
*/

brMenuEntry *brAddMenuItem( brInstance *i, const char *method, const char *title ) {
	brMethod *m;
	// extract the engine from the brInstance 

	m = brMethodFind( i->object, method, NULL, 0 );

	if ( strcmp( title, "" ) && !m ) {
		slMessage( DEBUG_ALL, "Cannot add menu item for method \"%s\"\n", method );
		return NULL;
	}

	if ( m ) brMethodFree( m );

	brMenuEntry *entry = new brMenuEntry;

	entry->method = slStrdup( method );
	entry->title = slStrdup( title );

	entry->instance = i;
	entry->checked = 0;
	entry->enabled = 1;

	i->_menus.push_back( entry );

	if ( i->engine->updateMenu ) 
		i->engine->updateMenu( i );

	return entry;
}

/*
	\brief Called when a menu item is selected.

	This is the callback function that an interface uses to inform the
	engine that a menu has been selected.  The menu item had a
	number associated with it when it was added, so we expect the
	interface to use that number to refer to the menu selection now.
*/

int brMenuCallback( brEngine *e, brInstance *i, unsigned int n ) {
	brEval eval;

	if ( !i ) i = e->controller;

	if ( i->status != AS_ACTIVE ) return EC_OK;

	if ( n >= i->_menus.size() || n < 0 ) {
		brEvalError( e, EE_INTERNAL, "Menu callback #%d out of bounds for instance %x", n, i );
		return EC_ERROR;
	}

	brMenuEntry *menu = ( brMenuEntry* )i->_menus[ n ];

	if ( !menu->enabled ) return EC_OK;

	if ( !strcmp( menu->method, "" ) ) return EC_OK;

	brEngineLock( e );

	int result = brMethodCallByName( i, menu->method, &eval );

	brEngineUnlock( e );

	return result;
}

brInstance *brClickAtLocation( brEngine *e, int x, int y ) {
	return brClickCallback( e, e->camera->select( e->world, x, y ) );
}

/*!
	\brief Called when the user clicks in the breve graphical window.

	The interface detects when an object = has been clicked on in the
	simulation, then calls this function to pass that instance the
	"click" method.

	Be careful calling this method from another thread--the engine is
	not thread safe, so you must externally make sure that the engine
	is not being iterated, preferably by a lock.
*/

brInstance *brClickCallback( brEngine *e, int n ) {
	brEval eval, theArg;
	const brEval *argPtr[1];
	brMethod *method;
	unsigned char types[] = { AT_INSTANCE };

	slWorldObject *o;

	if ( n == -1 ) o = NULL;
	else o = e->world->getObject( n );

	method = brMethodFind( e->controller->object, "click", types, 1 );

	if ( !method ) return NULL;

	if ( o ) theArg.set(( brInstance* )o->getCallbackData() );
	else theArg.set(( brInstance* )NULL );

	argPtr[0] = &theArg;

	brEngineLock( e );

	brMethodCall( e->controller, method, argPtr, &eval );

	brEngineUnlock( e );

	brMethodFree( method );

	if ( !o ) return NULL;

	return ( brInstance* )o->getCallbackData();
}

void brBeginDrag( brEngine *e, brInstance *i ) {
	brEval result;
	brMethodCallByName( e->controller, "get-drag-object", &result );

	if ( !BRINSTANCE( &result ) ) return;

	brMethodCallByName( BRINSTANCE( &result ), "suspend-physics", &result );
}

void brEndDrag( brEngine *e, brInstance *i ) {
	brEval result;
	brMethodCallByName( e->controller, "get-drag-object", &result );

	if ( !BRINSTANCE( &result ) ) return;

	brMethodCallByName( BRINSTANCE( &result ), "resume-physics", &result );
}

/*!
	\brief Handles a drag event.

	Called when the mouse is dragged.  Passes in the x- and y- mouse
	coordinates, calculates the drag-location and then sends a move
	message to the object.
*/

int brDragCallback( brEngine *e, int x, int y ) {
	brEval eval, theArg;
	const brEval *argPtr[ 1 ];
	brMethod *method;
	brInstance *i;
	int n;
	unsigned char types[] = { AT_VECTOR };
	slVector v;

	pthread_mutex_lock( &e->lock );

	method = brMethodFind( e->controller->object, "get-drag-object", NULL, 0 );

	if ( !method ) return EC_ERROR;

	n = brMethodCall( e->controller, method, NULL, &eval );

	brMethodFree( method );

	if ( n != EC_OK ) {

		pthread_mutex_unlock( &e->lock );

		return n;
	}

	i = BRINSTANCE( &eval );

	if ( !i ) {

		pthread_mutex_unlock( &e->lock );

		return EC_OK;
	}

	method = brMethodFind( i->object, "get-location", NULL, 0 );

	if ( !method ) return EC_ERROR;

	n = brMethodCall( i, method, NULL, &eval );

	brMethodFree( method );

	if ( n != EC_OK ) {

		pthread_mutex_unlock( &e->lock );

		return n;
	}

	e->camera->vectorForDrag( e->world, &BRVECTOR( &eval ), x, y, &v );

	theArg.set( v );

	method = brMethodFind( i->object, "move", types, 1 );

	if ( !method ) {

		pthread_mutex_unlock( &e->lock );

		return EC_ERROR;
	}

	argPtr[0] = &theArg;

	n = brMethodCall( i, method, argPtr, &eval );

	brMethodFree( method );

	pthread_mutex_unlock( &e->lock );

	return EC_OK;
}

/*!
	\brief Handles a key event.

	Like the other callbacks, this is called by the interface to communicate
	an interface action to the engine.  Takes the ascii code of the pressed
	key as well as a flag indicating whether the key press was a key down
	(1) or up (0).
*/

int brKeyCallback( brEngine *e, unsigned char keyCode, int isDown ) {
	brEval eval;
	brMethod *method;
	int r;
	char mname[128];

	if ( isDown ) e->keys[keyCode] = 5;

	/*
	 * call catch-key-X-down or catch-key-X-up, depending on isDown.
	 * we look for both the character and the ascii code, dvs, both
	 * catch-key-A-down and catch-key-0x41-down.
	 */

	if ( isDown ) {
		snprintf( mname, sizeof( mname ), "catch-key-%c-down", keyCode );
		method = brMethodFind( e->controller->object, mname, NULL, 0 );

		if ( !method ) {
			snprintf( mname, sizeof( mname ), "catch-key-0x%X-down", keyCode );
			method = brMethodFind( e->controller->object, mname, NULL, 0 );
		}
	} else {
		snprintf( mname, sizeof( mname ), "catch-key-%c-up", keyCode );

		method = brMethodFind( e->controller->object, mname, NULL, 0 );

		if ( !method ) {
			snprintf( mname, sizeof( mname ), "catch-key-0x%X-up", keyCode );
			method = brMethodFind( e->controller->object, mname, NULL, 0 );
		}
	}

	if ( !method )
		return EC_OK;

	r = brMethodCall( e->controller, method, NULL, &eval );

	brMethodFree( method );

	return r;
}

/*!
	\brief Handles a special key event.

	Handles key events which do not have ASCII codes.  This method
	will accept any "name" for a key, but the officially supported
	keys are, at the moment, the arrow keys: up, down, left and right.

	The breve interface is expected to relay these key events to the
	simulation.
*/

int brSpecialKeyCallback( brEngine *e, const char *name, int isDown ) {
	brEval eval;
	brMethod *method;
	int r;
	char mname[128];

	if ( isDown ) {
		snprintf( mname, sizeof( mname ), "catch-key-%s-down", name );
		method = brMethodFind( e->controller->object, mname, NULL, 0 );
	} else {
		snprintf( mname, sizeof( mname ), "catch-key-%s-up", name );
		method = brMethodFind( e->controller->object, mname, NULL, 0 );
	}

	if ( !method ) return EC_OK;

	r = brMethodCall( e->controller, method, NULL, &eval );

	brMethodFree( method );

	return r;
}

/**
 * \brief Handles a miscellaneous interface event.
 * 
 * If the frontend supports it, this is used to interface with arbitrary
 * interface elements.  Currently only available under Mac OS X.
 */

int brInterfaceCallback( brEngine *e, int interfaceID, const char *string ) {
	brEval eval, a;
	const brEval *args;
	brMethod *method;
	int r;
	unsigned char types[] = { AT_UNDEFINED };
	char mname[128];

	args = &a;
	a.set( string );

	snprintf( mname, sizeof( mname ), "catch-interface-id-%d", interfaceID );

	method = brMethodFind( e->controller->object, mname, types, 1 );

	if ( !method ) return EC_OK;

	r = brMethodCall( e->controller, method, &args, &eval );

	brMethodFree( method );

	return r;
}

const char *brEngine::runSaveDialog() {
	return getSavename ? getSavename() : NULL;
}

const char *brEngine::runLoadDialog() {
	return getLoadname ? getLoadname() : NULL;
}

void brEngine::setMouseLocation( int inX, int inY ) {
	_mouseX = inX;
	_mouseY = inY;
}
