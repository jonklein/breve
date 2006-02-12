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

/*!
	\brief returns a new empty brEvalListHead.
*/

#define LIST_INDEX_SIZE	16

brEvalListHead *brEvalListNew() {
	brEvalListHead *head;

	head = new brEvalListHead;

	head->start = head->end = NULL;
	head->count = 0;
	head->retainCount = 0;

	head->index = (brEvalList**)slMalloc(sizeof(brEvalList*) * LIST_INDEX_SIZE);
	head->indexSize = LIST_INDEX_SIZE;
	head->indexTop = -1;

	return head;
}

/*!
	\brief Frees an eval-list.

	If the eval-list has been retained using \ref brEvalListRetain, 
	the function has no effect.  If this is the case, then the 
	eval-list will be freed whenever \ref brEvalListUnretain
	sets the retain count to 0.
*/

void brEvalListFree(brEvalListHead *lh) {
	brEvalList *list, *next;

	list = lh->start;

	if(lh->retainCount > 0) return;

	while(list) {
		next = list->next;

		delete list;

		list = next;
	}

	if(lh->index) slFree(lh->index);

	delete lh;
}

/*!
	\brief Adds an arbitrary element from the list, with its value taken from
	the given pointer.
*/

int brEvalListInsert(brEvalListHead *head, int index, brEval *value) {
	brEvalList *start = head->start;
	brEvalList *newList;

	// were moving elements -- update the index top

	if( head->indexTop < index ) {
		// skip to the highest indexed element

		int skip = index;

		int newIndex = 0;

		if( head->indexTop > 0 ) {
			start = head->index[ head->indexTop ];
			skip -= head->indexTop;
			newIndex = head->indexTop;
		}


		// and now step through the list, updating the index because we can

		while( skip-- ) {
			if( !start ) return -1;

			head->index[ newIndex++ ] = start;
			start = start->next;
		}
	} else {
		start = head->index[ index];
	}

	head->indexTop = index - 1;

	head->count++;

	if(head->count >= head->indexSize) {
		head->indexSize += 128;
		head->index = (brEvalList**)slRealloc( head->index, sizeof(brEvalList*) * head->indexSize);
	}

	newList = new brEvalList;

	brEvalCopy(value, &newList->eval);

	if(start) {
		newList->previous = start->previous;
		newList->next = start;

		if(start->previous) start->previous->next = newList;

		start->previous = newList;

		if(start == head->start) head->start = newList;
	} else {
		// after the last element of the list.
		newList->previous = head->end;
		newList->next = NULL;

		if(head->end) head->end->next = newList;

		head->end = newList;

		if(head->start == NULL) head->start = newList;
	}

	return index;
}

int brEvalListRemove(brEvalListHead *head, int index, brEval *value) {
	brEvalList *start = head->start;

	if(head->indexTop >= index) head->indexTop = index - 1;

	while(index--) {
		if(!start) return -1;

		start = start->next;
	}

	if(!start) return -1;

	head->count--;

	if(start->previous) start->previous->next = start->next;
	else head->start = start->next;

	if(start->next) start->next->previous = start->previous;
	else head->end = start->previous;

	brEvalCopy(&start->eval, value);

	delete start;

	return index;
}

brEvalList *brEvalListIndexLookup(brEvalListHead *l, int index) {
	brEvalList *list;

	if(index >= l->count || index < 0) return NULL;

	if(l->indexTop > -1 && l->start != l->index[0]) {
		printf("WARNING!  List index corrupted\n");
	}

	if(index > l->indexTop) {
		if(l->indexTop < 0) {
			l->indexTop = 0;
			l->index[0] = l->start;
		}

		// start at the current top

		list = l->index[l->indexTop];

		// go until we find our index

		while(index > l->indexTop) {
			l->index[++l->indexTop] = list->next;
			list = list->next;
		}
	}

	return l->index[index];
}

brEvalListHead *brEvalListCopy(brEvalListHead *l) {
	brEvalListHead *newList;
	brEvalList *item = l->start;

	newList = brEvalListNew();

	while(item) {
		brEvalListInsert(newList, newList->count, &item->eval);
		item = item->next;
	}

	return newList;
}

brEvalListHead *brEvalListDeepCopy(brEvalListHead *l) {
	slList *seen = NULL;
	brEvalListHead *head;

	head = brDoEvalListDeepCopy(l, &seen);

	if(seen) {
		brFreeListRecords(seen);
		slListFree(seen);
	}

	return head;
}

brEvalListHead *brDoEvalListDeepCopy(brEvalListHead *l, slList **s) {
	brEvalListHead *newList;
	brEval newSubList;
	brEvalList *item = l->start;

	// we're now officially copying this list -- all future occurences should 
	// refer to the copy, so we make a record entry for it 

	newList = brEvalListNew();

	*s = slListPrepend(*s, brMakeListCopyRecord(l, newList));

	while(item) {
		brEvalListHead *copy;

		// is this a list? have we seen it before? 

		if( item->eval.type() == AT_LIST ) {
			copy = brCopyRecordInList(*s, BRLIST(&item->eval));

			if(!copy) {
				newSubList.set( brDoEvalListDeepCopy(BRLIST(&item->eval), s) );
				brEvalListInsert(newList, newList->count, &newSubList);
			} else {
				newSubList.set( copy );
				brEvalListInsert(newList, newList->count, &newSubList);
			}
		} else {
			brEvalListInsert(newList, newList->count, &item->eval);
		}

		item = item->next;
	}

	return newList;
}

brEvalListCopyRecord *brMakeListCopyRecord(brEvalListHead *original, brEvalListHead *copy) {
	brEvalListCopyRecord *r;

	r = new brEvalListCopyRecord;

	r->original = original;
	r->copy = copy;

	return r;
}

brEvalListHead *brCopyRecordInList(slList *recordList, brEvalListHead *search) {
	brEvalListCopyRecord *r;

	while(recordList) {
		r = (brEvalListCopyRecord*)recordList->data;

		if(r->original == search) return r->copy;

		recordList = recordList->next;
	}

	return NULL;
}

void brFreeListRecords(slList *records) {
	while(records) {
		delete (brEvalListCopyRecord*)records->data;
		records = records->next;
	}
}
