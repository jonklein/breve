/*****************************************************************************
 *                                                                           *
 * The breve Simulation Environment                                          *
 * Copyright (C) 2000-2004 Jonathan Klein                                    *
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

/*!
	The GC system in steve is an under the hood retain-count
	system.  The collectable data types are hash, list, object
	and data.  GC can be turned on/off on a per-object basis, 
	since objects can sometimes be "revived" (because of physical
	interaction or an "all" statement) even if they are not 
	referenced in memory.

	Any time new collectable data is allocated, it is given a 
	retain count of 0 and is added to a local variable collection 
	pool.  The collection pool is traversed at the end of the 
	local method.  If the retain count is still at 0, the data 
	is released.

	When any kind of variable assignment occurs, the retain count 
	is incremented.  When any kind of variable overwrite or release
	is preformed, the retain count is decremented.  If a variable
	overwrite causes a retain count to reach 0, the data is 
	released.


	Strings are a bit of a special case.  They piggy-back on the
	GC system, but do not work like the other types.  When a string 
	is set, it is always copied.  When it is unretained, it does
	not have a retain count--it is simply freed.  This system relies
	on the fact that a string will never be referenced by more than
	one memory location.


	all variable sets require a retain (hash, simple, list):
		- hash set 
		- simple set
		- list set
		- list push

	all variable overwrites or releases require immediate unretain and 
	collection:
		- hash set 
		- simple set
		- list set
		- method arguments at end of method call
		- local variables at end of method call

	creation of new collectable variables requires marking for collection:
		- new Object
		- new list
		- C-style function calls

	special treatment is needed for list operators which remove objects 
	and return them -- they are unretained and marked, but not immediately 
	collected.
		- list pop
*/

#include "steve.h"

/*!
	\brief Increments the retain count of a brEval.

	Calls type-specific retain functions.
*/

void stGCRetain(brEval *e) {
	stGCRetainPointer(e->values.pointerValue, e->type);
}

/*!
	\brief Increments the retain count of a pointer of a given type.

	Calls type-specific retain functions.
*/

void stGCRetainPointer(void *pointer, int type) {
	if(type == AT_NULL || !pointer) return;

	switch(type) {
		case AT_INSTANCE:
			if(((brInstance*)pointer)->status != AS_ACTIVE) return;
			stInstanceRetain((stInstance*)((brInstance*)pointer)->userData);
			break;
		case AT_LIST:
			brEvalListRetain((brEvalListHead*)pointer);
			break; 
		case AT_HASH: 
			brEvalHashRetain((brEvalHash*)pointer);
			break;
		case AT_DATA:
			brDataRetain((brData*)pointer);
			break;
		default:
			break;
	} 
}

/*!
	\brief Decrements the retain count of a brEval.

	Calls \ref stGCUnretainPointer.
*/

void stGCUnretain(brEval *e) {
	stGCUnretainPointer(BRPOINTER(e), e->type);
}

/*!
	\brief Decrements the retain count of a pointer of a given type.

	Calls type-specific unretain functions.
*/

void stGCUnretainPointer(void *pointer, int type) {
	if(type == AT_NULL || !pointer) return;

	switch(type) {
		case AT_INSTANCE:
			if(((brInstance*)pointer)->status != AS_ACTIVE) return;
			stInstanceUnretain((stInstance*)((brInstance*)pointer)->userData);
			break;
		case AT_LIST:
			brEvalListUnretain((brEvalListHead*)pointer);
			break;
		case AT_HASH:
			brEvalHashUnretain((brEvalHash*)pointer);
			break;
		case AT_DATA:
			brDataUnretain((brData*)pointer);
			break;
		case AT_STRING:
			break;
		default:
			break;
	}
}

/*!
	\brief Collects memory if the retain count of an eval is 0.
*/

void stGCCollect(brEval *e) {
	stGCCollectPointer(e->values.pointerValue, e->type);
}

/*!
	\brief Collects memory if the retain count of a pointer is 0.
*/

inline void stGCCollectPointer(void *pointer, int type) {
	if(type == AT_NULL || !pointer) return;

	switch(type) {
		case AT_INSTANCE:
			if(((brInstance*)pointer)->status != AS_ACTIVE) return;
			stInstanceCollect((stInstance*)((brInstance*)pointer)->userData);
			break;
		case AT_LIST:
			brEvalListCollect((brEvalListHead*)pointer);
			break;
		case AT_STRING:
			slFree((char*)pointer);
			break;
		case AT_HASH:
			brEvalHashCollect((brEvalHash*)pointer);
			break;
		case AT_DATA:
			brDataCollect((brData*)pointer);
			break;
		default:
			break;
	}
}

/*!
	\brief Decrements the retain count and collects memory if the retain
	count is 0.

	calls \ref stGCUnretain and \ref stGCCollect.
*/

void stGCUnretainAndCollect(brEval *e) {
	stGCUnretainAndCollectPointer(e->values.pointerValue, e->type);	
}

/*!
	\brief Decrements the retain count and collect memory if the retain
	count is 0.
*/

void stGCUnretainAndCollectPointer(void *pointer, int type) {
	stGCUnretainPointer(pointer, type);
	stGCCollectPointer(pointer, type);
}

/*!
	\brief Collects multiple expressions in an slStack structure.

	Uses \ref stGCCollect.
*/

void stGCCollectStack(slStack *gc) {
	unsigned int n;

	for(n=0;n<gc->count;n++) {
		stGCRecord *r = (stGCRecord*)gc->data[n];

		stGCCollectPointer(r->pointer, r->type);
		delete r;
	}

	gc->count = 0;
}

/*!
	\brief Marks a brEval for collection, by adding it to an instance's
	collection stack.
*/

void stGCMark(stInstance *i, brEval *collect) {
	stGCMarkPointer(i, BRPOINTER(collect), collect->type);
}

/*!
	\brief Marks a pointer for collect by adding it to an instance's
	collection stack.
*/

inline void stGCMarkPointer(stInstance *i, void *pointer, int type) {
	stGCRecord *r;
	stInstance *collect;

	if(type == AT_NULL || !pointer) return;

	switch(type) {
		case AT_INSTANCE:
			if(((brInstance*)pointer)->status != AS_ACTIVE) return;
			collect = (stInstance*)((brInstance*)pointer)->userData;
			if(!collect->gc) return;
			break;
		default:
			break;
	}


	r = new stGCRecord;
	r->pointer = pointer;
	r->type = type;
	
	slStackPush(i->gcStack, r);
}

/*!
	\brief Unmark a brEval for collection, by removing it from an instance's
	collection stack.
*/

void stGCUnmark(stInstance *i, brEval *collect) {
	unsigned int n;
	stGCRecord *r;

	if(!i->gcStack || collect->type == AT_NULL || collect->type == AT_INT || 
		collect->type == AT_DOUBLE || collect->type == AT_MATRIX || 
		collect->type == AT_VECTOR || !collect->values.pointerValue) return;

	if(collect->type == AT_INSTANCE) {
		stInstance *i;

		if(BRINSTANCE(collect)->status != AS_ACTIVE) return;

		i = (stInstance*)BRINSTANCE(collect)->userData;

		if(!i->gc) return;
	} 

	for(n=0;n<i->gcStack->count;n++) {
		r = (stGCRecord*)i->gcStack->data[n];
	
		if(r->pointer == collect->values.pointerValue) {
			switch(r->type) {
				case AT_HASH:
				case AT_STRING:
				case AT_DATA:
				case AT_LIST:
				case AT_INSTANCE:
					r->pointer = NULL;
					break;
				default:
					break;
			}
		}
	}
}

/*!
	\brief Increments the reference count of an eval-list.
 
	This is so that several structures can reference the same list.
*/
	
void brEvalListRetain(brEvalListHead *lh) {
	if(!lh) return;

	lh->retainCount++;
}
	
/*!
	\brief Decrements the retain list count.
	
	If the count is then zero, the list is freed with brEvalListFree.
*/
	
void brEvalListUnretain(brEvalListHead *lh) {
	if(!lh) return;

	lh->retainCount--;
}	   
		
/*! 
	\brief Collects an evalList, if it's ready for GC.
	
	Frees the list if the retainCount is less than 1.
*/  

void brEvalListCollect(brEvalListHead *lh) {
	if(lh->retainCount < 1) brEvalListFreeGC(lh);
}

/*!
	\brief Garbage-collects and frees a brEvalList.

	Unretains and collects all list members, and then preforms a 
	regular dealocation.	
*/

void brEvalListFreeGC(brEvalListHead *lh) {
	brEvalList *list = lh->start;

	while(list) {
		stGCUnretainAndCollect(&list->eval);
		list = list->next;
	}

	brEvalListFree(lh);
}

/*!
	\brief Increments the retain count of the eval hash.
*/

void brEvalHashRetain(brEvalHash *h) {
	h->retainCount++;
}

/*!
	\brief Decrements the retain count of the eval hash.
*/

void brEvalHashUnretain(brEvalHash *h) {
	h->retainCount--;
}

/*!
	\brief Garbage-collects a brEvalHash.

	If the retain count is less than 1, all keys and values are
	unretained, and the hash table is freed.
*/

void brEvalHashCollect(brEvalHash *h) {
	if(h->retainCount < 1) brEvalHashFreeGC(h);
}

/*!
	\brief Garbage-collects and frees a brEvalHash.

	Unretains and collects all hash keys and value, then 
	preforms a regular dealocation.	
*/

void brEvalHashFreeGC(brEvalHash *h) {
	slList *all, *start;

	start = all = slHashValues(h->table);

	while(all) {
		stGCUnretainAndCollect((brEval*)all->data);
		all = all->next;
	}

	slListFree(start);

	start = all = slHashKeys(h->table);

	while(all) {
		stGCUnretainAndCollect((brEval*)all->data);
		all = all->next;
	}

	slListFree(start);

	brEvalHashFree(h);
}
