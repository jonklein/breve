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

/*!
	\brief Sets the callback to be triggered when a menu changes.
*/

void brSetUpdateMenuCallback(brInstance *i, void (*updateMenu)(brInstance *l)) {
	i->menu.updateMenu = updateMenu;

	updateMenu(i);
}

/*!
	\brief Sets a callback that will trigger a dialog box.
*/

void brSetDialogCallback(brEngine *e, int (*dc)(void *data, char *t, char *m, char *b1, char *b2)) {
	e->dialogCallback = dc;
}

/*!
	\brief Adds an item to an object's menu.

	Adds an item to the object's menu with the specified title, that will 
	trigger the given method when selected.
*/

brMenuEntry *brAddMenuItem(brInstance *i, char *method, char *title) {
	brMethod *m;
	/* rather than using the global engine variable, we extract */
	/* the value from the brInstance */

	m = brMethodFind(i->class, method, 0);

	if(strcmp(title, "") && !m) return NULL;

	if(m) brMethodFree(m);

	if(i->menu.count == 0) {
		i->menu.maxCount = 8;
		i->menu.list = slMalloc(i->menu.maxCount * sizeof(brMenuEntry*));
	} else if(i->menu.count == i->menu.maxCount) {
		i->menu.maxCount *= 2;
		i->menu.list = slRealloc(i->menu.list, i->menu.maxCount * sizeof(brMenuEntry*));
	}

	i->menu.list[i->menu.count] = slMalloc(sizeof(brMenuEntry));

	i->menu.list[i->menu.count]->instance = i;
	i->menu.list[i->menu.count]->method = slStrdup(method);
	i->menu.list[i->menu.count]->title = slStrdup(title);

	i->menu.list[i->menu.count]->checked = 0;
	i->menu.list[i->menu.count]->enabled = 1;

	if(i->menu.updateMenu) i->menu.updateMenu(i);

	return i->menu.list[i->menu.count++];
}

/*
	\brief Called when a menu item is selected.

	This is the callback function that an interface uses to inform the 
	engine that a menu has been selected.  The menu item had a 
	number associated with it when it was added, so we expect the 
	interface to use that number to refer to the menu selection now.
*/

int brMenuCallback(brEngine *e, brInstance *i, int n) {
	brEval eval;

	if(!i) i = e->controller;

	if(i->status != AS_ACTIVE) return EC_OK;

	if(n >= i->menu.count) {
		brEvalError(e, EE_INTERNAL, "Menu callback #%d not defined for instance %x", n, i);
		return EC_ERROR;
	}

	if(!i->menu.list[n]->enabled) return EC_OK;
	if(!strcmp(i->menu.list[n]->method, "")) return EC_OK;

	return brMethodCallByName(i, i->menu.list[n]->method, &eval);
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

brInstance *brClickCallback(brEngine *e, int n) {
	brEval eval, theArg, *argPtr[1];
	brMethod *method;

	slWorldObject *o;

	if(n == -1) o = NULL;
	else o = e->world->objects[n];

	method = brMethodFind(e->controller->class, "click", 1);
	
	if(!method) return NULL;

	theArg.type = AT_INSTANCE;
	if(o) BRINSTANCE(&theArg) = o->userData;
	else BRINSTANCE(&theArg) = NULL;

	argPtr[0] = &theArg;
	brMethodCall(e->controller, method, argPtr, &eval);

	brMethodFree(method);

	return BRINSTANCE(&theArg);
}

/*!
	\brief Handles a drag event.

	Called when the mouse is dragged.  Passes in the x- and y- mouse 
	coordinates, calculates the drag-location and then sends a move 
	message to the object.
*/

int brDragCallback(brEngine *e, int x, int y) {
	brEval eval, theArg, *argPtr[1];
	brMethod *method;
	brInstance *i;
	int n;

	pthread_mutex_lock(&e->lock);

	method = brMethodFind(e->controller->class, "get-drag-object", 0);
	if(!method) return EC_ERROR;

	n = brMethodCall(e->controller, method, NULL, &eval);

	brMethodFree(method);

	if(n != EC_OK) {
		pthread_mutex_unlock(&e->lock);
		return n;
	}

	i = BRINSTANCE(&eval);
	if(!i) {
		pthread_mutex_unlock(&e->lock);
		return EC_OK;
	}

	method = brMethodFind(i->class, "get-location", 0);
	if(!method) return EC_ERROR;

	n = brMethodCall(i, method, NULL, &eval);

	brMethodFree(method);

	if(n != EC_OK) {
		pthread_mutex_unlock(&e->lock);
		return n;
	}

	slVectorForDrag(e->world, e->camera, &BRVECTOR(&eval), x, y, &BRVECTOR(&theArg));
	method = brMethodFind(i->class, "move", 1);

	if(!method) {
		pthread_mutex_unlock(&e->lock);
		return EC_ERROR;
	}

	theArg.type = AT_VECTOR;
	argPtr[0] = &theArg;
	n = brMethodCall(i, method, argPtr, &eval);

	brMethodFree(method);

	pthread_mutex_unlock(&e->lock);

	return EC_OK;
}

/*!
	\brief Handles a key event.

	Like the other callbacks, this is called by the interface to communicate
	an interface action to the engine.  Takes the ascii code of the pressed 
	key as well as a flag indicating whether the key press was a key down 
	(1) or up (0).
*/

int brKeyCallback(brEngine *e, char keyCode, int isDown) {
	char mname[128];
	brMethod *method;
	brEval eval;
	int r;

	if(isDown) e->keys[keyCode] = 5;

	/* call catch-key-X-down or catch-key-X-up, depending on isDown.  we */
	/* look for both the character and the ascii code, dvs, both		 */
	/* catch-key-A-down and catch-key-0x41-down.  */

	if(isDown) {
		sprintf(mname, "catch-key-%c-down", keyCode);
		method = brMethodFind(e->controller->class, mname, 0);

		if(!method) {
			sprintf(mname, "catch-key-0x%X-down", keyCode);
			method = brMethodFind(e->controller->class, mname, 0);
		}
	} else {
		sprintf(mname , "catch-key-%c-up", keyCode);

		method = brMethodFind(e->controller->class, mname, 0);

		if(!method) {
			sprintf(mname, "catch-key-0x%X-up", keyCode);
			method = brMethodFind(e->controller->class, mname, 0);
		}
	}

	if(!method) return EC_OK;

	r = brMethodCall(e->controller, method, NULL, &eval);

	brMethodFree(method);

	return r;
}

/*!
	\brief Handles a miscellaneous interface event.

	If the frontend supports it, this is used to interface with arbitrary
	interface elements.  Currently only available under Mac OS X.
*/

int brInterfaceCallback(brEngine *e, int interfaceID, char *string) {
	brEval eval, *args, a;
	char mname[128];
	brMethod *method;
	int r;

	args = &a;
	BRSTRING(args) = string;
	args->type = AT_STRING;

	sprintf(mname, "catch-interface-id-%d", interfaceID);

	method = brMethodFind(e->controller->class, mname, 1);

	if(!method) return EC_OK;

	r = brMethodCall(e->controller, method, &args, &eval);

	brMethodFree(method);

	return r;
}
