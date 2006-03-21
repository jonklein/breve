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
#include "vclip.h"
#include "vclipData.h"

/*!
	\brief Checks whether collisions should be detected between two objects.

	In this function, the world asks us if we want to handle collisions 
	between two instances.  We return 1 if a collision handler is in place 
	or if all collision resolution is on, otherwise 0.
*/

int brCheckCollisionCallback(void *p1, void *p2, int type) {
	unsigned int n;
	brCollisionHandler *h;

	brInstance *o1 = (brInstance*)p1;
	brInstance *o2 = (brInstance*)p2;

	if(!o1 || !o2) return 0;

	/* check all of the collision handlers for both instances */
	/* to see if there exists code to collide with eachother  */

	for(n=0;n<o1->object->collisionHandlers->count;n++) {
		h = (brCollisionHandler*)o1->object->collisionHandlers->data[n];

		if(o2->object->type == h->object->type && o2->object->type->isSubclass(h->object->userData, o2->object->userData)) {
			if(type == CC_CALLBACK) {
				if(h->method) return 1;
				return 0;
			}

			if(type == CC_SIMULATE) {
				// simulate is the default -- only return 0 if they want to explicity ignore
				if(h->ignore) return 0;
				return 1;
			}
		}
	}

	for(n=0;n<o2->object->collisionHandlers->count;n++) {
		h = (brCollisionHandler*)o2->object->collisionHandlers->data[n];

		if(o1->object->type == h->object->type && o1->object->type->isSubclass(h->object->userData, o1->object->userData)) {
			if(type == CC_CALLBACK) {
				if(h->method) return 1;
				return 0;
			}

			if(type == CC_SIMULATE) {
				// simulate is the default -- only return 0 if they want to explicity ignore
				if(h->ignore) return 0;
				return 1;
			}
		}
	}

	if(type == CC_CALLBACK) return 0;
	
	return 1;
}


/*!
	\brief Called when two instances actually collide, this triggers 
	a frontend method call.

	For each instance we'll step through it's collision handlers.
	if the instance handles a collision with the other instance
	(depending on the type of the other instance), then we send 
	the proper "collision" method, specifiying the instance
	that is colliding with it as the argument.
*/

void brCollisionCallback(void *p1, void *p2, int type, slVector *pos, slVector *face) {
	unsigned int n;
	brMethod *meth;
	brEval collider, result;
	brEval position, facing;
	brCollisionHandler *h;
	brEval *argPtr[3];//Sometimes we only need brEval *argPtr[1];


	brInstance *o1 = (brInstance*)p1;
	brInstance *o2 = (brInstance*)p2;
	//brInstance *o3 = (brInstance*)pos;
	//brInstance *o4 = (brInstance*)face;

	if(!o1 || !o2 || (o1->status != AS_ACTIVE) || (o2->status != AS_ACTIVE)) return;

	for(n=0;n<o1->object->collisionHandlers->count;n++) {
		h = (brCollisionHandler*)o1->object->collisionHandlers->data[n];

		if(o2->object->type == h->object->type && o2->object->type->isSubclass(o2->object->userData, h->object->userData)) {
			meth = h->method;
			// Only call methods with the right count of parameters
			if(meth->argumentCount==3){
					
				collider.set( o2 );
				position.set((*pos));
				facing.set((*face));
				argPtr[0] = &collider;
				argPtr[1] = &position;
				argPtr[2] = &facing;
			}
			// Only 1 paramerter so not position and facing direction is needed
			else if(meth->argumentCount==1){
				collider.set( o2 );
				argPtr[0] = &collider;
			}
			else{
				slMessage(DEBUG_ALL, "Error during collision callback: wrong method arguments count\n");
			}
			if(brMethodCall(o1, meth, argPtr, &result) == EC_ERROR) {
				slMessage(DEBUG_ALL, "Error during collision callback\n");
				return;
			}

			if( result.type() == AT_INT && BRINT(&result) ) return;
		}
	}

	/* it's possible that one of the objects will be destroyed */
	/* by the interaction, so check to see if we need to stop  */

	if(o1->status != AS_ACTIVE || o2->status != AS_ACTIVE) return;
	
	for(n=0;n<o2->object->collisionHandlers->count;n++) {
		h = (brCollisionHandler*)o2->object->collisionHandlers->data[n];

		if(o1->object->type == h->object->type && o1->object->type->isSubclass(o1->object->userData, h->object->userData)) {
			meth = h->method;

			// Only call methods with the right count of parameters
			if(meth->argumentCount==3){
					
				collider.set( o2 );
				position.set((*pos));
				facing.set((*face));
				argPtr[0] = &collider;
				argPtr[1] = &position;
				argPtr[2] = &facing;
			}
			// Only 1 paramerter so not position and facing direction is needed
			else if(meth->argumentCount==1){
				collider.set( o2 );
				argPtr[0] = &collider;
			}
			
			if(brMethodCall(o2, meth, argPtr, &result) == EC_ERROR) {
				slMessage(DEBUG_ALL, "Error during collision callback4\n");
				return;
			}

			if( result.type() == AT_INT && BRINT(&result) ) return;
		}
	}
	
}
