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

/*
	+ callback.c
	= contains two callback functions used in conjunction with collisions. 
	= one function takes two steve instances and determines whether they
	= have the possibility of colliding, the other is called during the 
	= actual collision
*/

#include "kernel.h"

/*!
	\brief Checks whether collisions should be detected between two objects.

	In this function, the world asks us if we want to handle collisions 
	between two instances.  We return 1 if a collision handler is in place 
	or if all collision resolution is on, otherwise 0.
*/

brCollisionHandler *brCheckCollisionCallback(brInstance *o1, brInstance *o2) {
	int n;
	brCollisionHandler *h;

	if(!o1 || !o2) return NULL;

	/* check all of the collision handlers for both instances */
	/* to see if there exists code to collide with eachother  */

	for(n=0;n<o1->class->collisionHandlers->count;n++) {
		h = o1->class->collisionHandlers->data[n];

		if(o2->class->type == h->object->type && o2->class->type->isSubclass(h->object, o2->class)) return h;
	}

	for(n=0;n<o2->class->collisionHandlers->count;n++) {
		h = o2->class->collisionHandlers->data[n];

		if(o1->class->type == h->object->type && o1->class->type->isSubclass(h->object, o1->class)) return h;
	}

	return NULL;
}

/*
	+ brCollisionCallback
	= is called when two instances actually collide.  this callback
	= is triggered by the world code, and passes back the "userData"
	= from the world objects.
	= 
	= for each instance we'll step through it's collision handlers.
	= if the instance handles a collision with the other instance
	= (depending on the type of the other instance), then we send 
	= the proper "collision" method, specifiying the instance
	= that is colliding with it as the argument.
*/

void brCollisionCallback(brInstance *o1, brInstance *o2, int type) {
	int n;
	brMethod *meth;
	brEval collider, result;
	brCollisionHandler *h;

	brEval *argPtr[1];

	if(!o1 || !o2) return;

	for(n=0;n<o1->class->collisionHandlers->count;n++) {
		h = o1->class->collisionHandlers->data[n];

		if(o2->class->type == h->object->type && o2->class->type->isSubclass(o2->class, h->object)) {
			meth = h->method;

			collider.type = AT_INSTANCE;
			BRINSTANCE(&collider) = o2;
			argPtr[0] = &collider;

			if(brMethodCall(o1, meth, argPtr, &result) == EC_ERROR) {
				slMessage(DEBUG_ALL, "Error during collision callback\n");
				return;
			}

			if(result.type == AT_INT && BRINT(&result)) return;
		}
	}

	/* it's possible that one of the objects will be destroyed */
	/* by the interaction, so check to see if we need to stop  */

	if(o1->status != AS_ACTIVE || o2->status != AS_ACTIVE) return;
	
	for(n=0;n<o2->class->collisionHandlers->count;n++) {
		h = o2->class->collisionHandlers->data[n];

		if(o1->class->type == h->object->type && o1->class->type->isSubclass(o1->class, h->object)) {
			meth = h->method;

			collider.type = AT_INSTANCE;
			BRINSTANCE(&collider) = o1;
			argPtr[0] = &collider;

			if(brMethodCall(o2, meth, argPtr, &result) == EC_ERROR) {
				slMessage(DEBUG_ALL, "Error during collision callback\n");
				return;
			}

			if(result.type == AT_INT && BRINT(&result)) return;
		}
	}
}
