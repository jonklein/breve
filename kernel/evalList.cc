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
	+ evalList.c
	= an brEvalList is a perl style list of brEvals.  this is completely
	= independent of the "slList" type of the util library.  the point
	= here is to have an array of data in which each element can be any
	= data type.
	=
	= unlike the slList type, the brEvalList is referenced by
	= a constant "head", a brEvalListHead.
	=
	= evalList.c provides the interface to this data type.
*/

/**
	Creates a new brEvalListHead
*/

brEvalListHead *brEvalListNew() {
	return new brEvalListHead();
}

brEvalListHead::brEvalListHead() : brEvalObject() {}

/*!
	\brief Frees an eval-list.

	If the eval-list has been retained using \ref brEvalListRetain,
	the function has no effect.  If this is the case, then the
	eval-list will be freed whenever \ref brEvalListUnretain
	sets the retain count to 0.
*/

brEvalListHead::~brEvalListHead() {
}

/*!
	\brief Adds an arbitrary element from the list, with its value taken from
	the given pointer.
*/

int brEvalListInsert( brEvalListHead *head, int index, brEval *inEval ) {
	int size = (int)head->_vector.size();

	if ( index > size ) {
		return EC_ERROR;
	} else if ( index == size ) {
		head->_vector.push_back( *inEval );
	} else {
		std::vector< brEval >::iterator i = head->_vector.begin() + index;

		head->_vector.insert( i, 1, *inEval );
	}

	return EC_OK;
}

int brEvalListAppend( brEvalListHead *inList, brEval *inEval ) {
	inList -> _vector.push_back( *inEval );
	return EC_OK;
}

int brEvalListRemove( brEvalListHead *head, int index, brEval *value ) {
	if ( index < 0 || index >= ( int )head->_vector.size() ) return index;

	std::vector< brEval >::iterator i = head->_vector.begin() + index;

	brEvalCopy( &(*i), value );

	head->_vector.erase( i );

	return index;
}

int brEvalListLength( brEvalListHead *inHead ) {
	return inHead->_vector.size();
}

brEval *brEvalListIndexLookup( brEvalListHead *head, int index ) {
	if ( index < 0 || index >= ( int )head->_vector.size() ) return NULL;

	std::vector< brEval >::iterator i = head->_vector.begin() + index;

	return &(*i);
}

brEvalListHead *brEvalListCopy( brEvalListHead *head ) {
	brEvalListHead *newList;
	std::vector< brEval >::iterator i;

	newList = new brEvalListHead();

	newList->_vector = head->_vector;

	return newList;
}

brEvalListHead *brEvalListDeepCopy( brEvalListHead *l ) {
	slList *seen = NULL;
	brEvalListHead *head;

	head = brDoEvalListDeepCopy( l, &seen );

	if ( seen ) {
		brFreeListRecords( seen );
		slListFree( seen );
	}

	return head;
}

brEvalListHead *brDoEvalListDeepCopy( brEvalListHead *l, slList **s ) {
	brEvalListHead *newList;
	brEval newSubList;
	std::vector< brEval >::iterator i;

	// we're now officially copying this list -- all future occurences should
	// refer to the copy, so we make a record entry for it

	newList = new brEvalListHead();

	*s = slListPrepend( *s, brMakeListCopyRecord( l, newList ) );

	for ( i = l->_vector.begin(); i != l->_vector.end(); i++ ) {
		brEvalListHead *copy;
		brEval *eval = &(*i);

		// is this a list? have we seen it before?

		if ( eval->type() == AT_LIST ) {
			copy = brCopyRecordInList( *s, BRLIST( eval ) );

			if ( !copy ) {
				newSubList.set( brDoEvalListDeepCopy( BRLIST( eval ), s ) );
				brEvalListInsert( newList, newList->_vector.size(), &newSubList );
			} else {
				newSubList.set( copy );
				brEvalListInsert( newList, newList->_vector.size(), &newSubList );
			}
		} else {
			brEvalListInsert( newList, newList->_vector.size(), eval );
		}
	}

	return newList;
}

brEvalListCopyRecord *brMakeListCopyRecord( brEvalListHead *original, brEvalListHead *copy ) {
	brEvalListCopyRecord *r;

	r = new brEvalListCopyRecord;

	r->original = original;
	r->copy = copy;

	return r;
}

brEvalListHead *brCopyRecordInList( slList *recordList, brEvalListHead *search ) {
	brEvalListCopyRecord *r;

	while ( recordList ) {
		r = ( brEvalListCopyRecord* )recordList->data;

		if ( r->original == search ) return r->copy;

		recordList = recordList->next;
	}

	return NULL;
}

void brFreeListRecords( slList *records ) {
	while ( records ) {
		delete( brEvalListCopyRecord* )records->data;
		records = records->next;
	}
}
