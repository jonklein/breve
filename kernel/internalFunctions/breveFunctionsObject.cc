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

/*!
	\brief Returns an instance's class name as a string.

	string objectName().
*/

int brIObjectName(brEval args[], brEval *target, brInstance *i) {
    BRSTRING(target) = slStrdup(BRINSTANCE(&args[0])->class->name);

    return EC_OK;
}

/*!
	\brief Adds an observer to the calling object's observer list.

	When the calling instance announces a notification matching the 
	notification name, the specified instance is executed for the 
	observer.

	void addObserver(object dependency, string notification, string method).
*/

int brIAddObserver(brEval args[], brEval *target, brInstance *bi) {
	brInstance *observedObject = BRINSTANCE(&args[0]);
	char *notification = BRSTRING(&args[1]);
	char *method = BRSTRING(&args[2]);

	BRINT(target) = brInstanceAddObserver(observedObject, bi, notification, method);

	return EC_OK;
}

/*!
	\brief Removes an observer from the calling object's observer list.

	Removes only the entry for the specified notificationName.

	void removeObserver(object observer, string notificationName).
*/

int brIRemoveObserver(brEval args[], brEval *target, brInstance *i) {
	brInstance *observer = BRINSTANCE(&args[0]);
	char *notification = BRSTRING(&args[1]);

	brEngineRemoveInstanceObserver(i, observer, notification);
	return EC_OK;
}

/*!
	\brief Adds a dependency to the calling object's dependency list.

	void addDependency(object dependency).
*/

int brIAddDependency(brEval args[], brEval *target, brInstance *i) {
    brInstanceAddDependency(i, BRINSTANCE(&args[0]));
    return EC_OK;
}

/*!
	\brief Removes a dependency from the calling object's dependency list.

	void removeDependency(object dependency).
*/

int brIRemoveDependency(brEval args[], brEval *target, brInstance *i) {
    brEngineRemoveInstanceDependency(i, BRINSTANCE(&args[0]));
    return EC_OK;
}

/*!
	\brief Add a collision handler for an object type.

	Note that though this method takes a single instance as input, it will
	add a collision handler for all objects of the given type.

	void addCollisionHandler(object caller, string otherObject, string methodName).
*/

int brIAddCollisionHandler(brEval args[], brEval *target, brInstance *i) {
    brObject *handler, *collider;
	brInstance *caller;

    caller = BRINSTANCE(&args[0]);

    handler = caller->class;

    collider = brObjectFind(i->engine, BRSTRING(&args[1]));

    if(!collider) {
        slMessage(DEBUG_ALL, "addCollisionCallback: Cannot locate class \"%s\"\n", BRSTRING(&args[1]));
        return EC_ERROR;
    }

    brObjectAddCollisionHandler(handler, collider, BRSTRING(&args[2]));

    return EC_OK;
}

/*!
	\brief Sends a notification, which triggers an event for all observers.

	void notify(string notification).
*/

int brINotify(brEval args[], brEval *target, brInstance *i) {
    brEval *newargs[2], sEval, iEval, t;
    char *notification = slStrdup(BRSTRING(&args[0]));
    brObserver *observer;
    int r, count;
    slList *observers;

    observers = i->observers;

    BRSTRING(&sEval) = notification;
    sEval.type = AT_STRING;

    BRINSTANCE(&iEval) = i;
    iEval.type = AT_INSTANCE;

    newargs[0] = &iEval;
    newargs[1] = &sEval;

    /* for each observer, check to see if what it's observing */

    while(observers) {
        observer = observers->data;

        if(!strcmp(notification, observer->notification)) {
            count = observer->method->argumentCount;

            r = brMethodCall(observer->instance, observer->method, newargs, &t);
        }

        observers = observers->next;
    }

    slFree(BRSTRING(&sEval));

    return EC_OK;
}

/*!
	\brief Returns the controller object.

	object getController().
*/

int brIGetController(brEval args[], brEval *target, brInstance *i) {
    BRINSTANCE(target) = i->engine->controller;
    return EC_OK;
}

/*@}*/

void breveInitObjectFunctions(brNamespace *n) {
	brNewBreveCall(n, "objectName", brIObjectName, AT_STRING, AT_INSTANCE, 0);
	brNewBreveCall(n, "addObserver", brIAddObserver, AT_INT, AT_INSTANCE, AT_STRING, AT_STRING, 0);
	brNewBreveCall(n, "removeObserver", brIRemoveObserver, AT_NULL, AT_INSTANCE, AT_STRING, 0);
	brNewBreveCall(n, "notify", brINotify, AT_NULL, AT_STRING, 0);

	brNewBreveCall(n, "addCollisionHandler", brIAddCollisionHandler, AT_INT, AT_INSTANCE, AT_STRING, AT_STRING, 0);

	brNewBreveCall(n, "addDependency", brIAddDependency, AT_NULL, AT_INSTANCE, 0);
	brNewBreveCall(n, "removeDependency", brIRemoveDependency, AT_NULL, AT_INSTANCE, 0);

	brNewBreveCall(n, "getController", brIGetController, AT_INSTANCE, 0);
}
