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

	head = slMalloc(sizeof(brEvalListHead));

	head->start = head->end = NULL;
	head->count = 0;
	head->retainCount = 0;

	head->index = slMalloc(sizeof(brEvalList*) * LIST_INDEX_SIZE);
	head->indexSize = LIST_INDEX_SIZE;
	head->indexTop = -1;

	return head;
}

/*!
	\brief Returns the number of elements in an eval-list.
*/

int brEvalListCount(brEvalListHead *lh) {
	return lh->count;
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

		slFree(list);

		list = next;
	}

	if(lh->index) slFree(lh->index);

	slFree(lh);
}

/*!
	\brief Adds an arbitrary element from the list, with its value taken from
	the given pointer.
*/

int brEvalListInsert(brEvalListHead *head, int index, brEval *value) {
	brEvalList *start = head->start;
	brEvalList *newList;

	// were moving elements -- update the index top

	if(head->indexTop >= index) head->indexTop = index - 1;

	while(index--) {
		if(!start) return -1;

		start = start->next;
	}

	head->count++;

	if(head->count >= head->indexSize) {
		head->indexSize += 128;
		head->index = slRealloc(head->index, sizeof(brEvalList*) * head->indexSize);
	}

	newList = slMalloc(sizeof(brEvalList));

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

/*!
	\brief Removes an arbitrary element from the list, with its value placed in
	the given pointer.
*/

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

	slFree(start);

	return index;
}

/*!
	\brief Looks up a lists index, using the index if possible.

	Updates the index as far as it can as it traverses the list.
*/

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
