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

	All garbage collection is now handled through the brEval 
	class using reference counting hooks in the set() and 
	destructor methods.  This means that GC happens automatically
	when using brEval objects.

	When not using brEval objects, while setting breve instance
	variables directly in memory, for example, the retain
	and unretain methods must be called automatically.
*/

#include "steve.h"

/*!
	\brief Increments the retain count of a brEval.

	Calls type-specific retain functions.
*/

void stGCRetain(brEval *e) {
	stGCRetainPointer( e->getPointer(), e->type() );
}

/*!
	\brief Increments the retain count of a pointer of a given type.

	Calls type-specific retain functions.
*/

void stGCRetainPointer(void *pointer, int type) {
	if( type == AT_NULL 
		|| !pointer ) return;

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
	stGCUnretainPointer( e->getPointer(), e->type() );
}

/*!
	\brief Decrements the retain count of a pointer of a given type.

	Calls type-specific unretain functions.
*/

void stGCUnretainPointer(void *pointer, int type) {
	if(type == AT_NULL || type == AT_INT || type == AT_DOUBLE || type == AT_MATRIX || type == AT_VECTOR || !pointer) return;

	switch(type) {
		case AT_INSTANCE:
			if(!pointer || ((brInstance*)pointer)->status != AS_ACTIVE) return;
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
	stGCCollectPointer( e->getPointer(), e->type() );
}

/*!
	\brief Collects memory if the retain count of a pointer is 0.
*/

void stGCCollectPointer(void *pointer, int type) {
	if(type == AT_NULL || type == AT_INT || type == AT_DOUBLE || type == AT_MATRIX || type == AT_VECTOR || !pointer) return;

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
	stGCUnretainAndCollectPointer( e->getPointer(), e->type() );	
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
	\brief Increments the reference count of an eval-list.
 
	This is so that several structures can reference the same list.
*/
	
void brEvalListRetain(brEvalListHead *lh) {
	lh->retain();
}
	
/*!
	\brief Decrements the retain list count.
	
	If the count is then zero, the list is freed with brEvalListFree.
*/
	
void brEvalListUnretain(brEvalListHead *lh) {
	lh->unretain();
}	   
		
/*! 
	\brief Collects an evalList, if it's ready for GC.
	
	Frees the list if the retainCount is less than 1.
*/  

void brEvalListCollect(brEvalListHead *lh) {
	if( lh->_retainCount < 1 ) delete lh;
}

/*!
	\brief Increments the retain count of the eval hash.
*/

void brEvalHashRetain(brEvalHash *h) {
	h->retain();
}

/*!
	\brief Decrements the retain count of the eval hash.
*/

void brEvalHashUnretain(brEvalHash *h) {
	h->unretain();
}

/*!
	\brief Garbage-collects a brEvalHash.

	If the retain count is less than 1, all keys and values are
	unretained, and the hash table is freed.
*/

void brEvalHashCollect(brEvalHash *h) {
	if( h->_retainCount < 1 ) delete h;
}
