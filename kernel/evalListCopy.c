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

	head = stDoEvalListDeepCopy(l, &seen);

	if(seen) {
		stFreeListRecords(seen);
		slListFree(seen);
	}

	return head;
}

brEvalListHead *stDoEvalListDeepCopy(brEvalListHead *l, slList **s) {
	brEvalListHead *newList;
	brEval newSubList;
	brEvalList *item = l->start;

	/* we're now officially copying this list -- all future occurences should */
	/* refer to the copy, so we make a record entry for it */

	newList = brEvalListNew();

	// brRetainEvalList(newList);

	*s = slListPrepend(*s, stMakeListCopyRecord(l, newList));

	while(item) {
		brEvalListHead *copy;

		/* is this a list? have we seen it before? */

		if(item->eval.type == AT_LIST) {
			copy = stCopyRecordInList(*s, BRLIST(&item->eval));
			newSubList.type = AT_LIST;

			if(!copy) {
				BRLIST(&newSubList) = stDoEvalListDeepCopy(BRLIST(&item->eval), s);
				brEvalListInsert(newList, newList->count, &newSubList);
			} else {
				BRLIST(&newSubList) = copy;
				brEvalListInsert(newList, newList->count, &newSubList);
			}
		} else {
			brEvalListInsert(newList, newList->count, &item->eval);
		}

		item = item->next;
	}

	return newList;
}

brEvalListCopyRecord *stMakeListCopyRecord(brEvalListHead *original, brEvalListHead *copy) {
	brEvalListCopyRecord *r;

	r = slMalloc(sizeof(brEvalListCopyRecord));

	r->original = original;
	r->copy = copy;

	return r;
}

brEvalListHead *stCopyRecordInList(slList *recordList, brEvalListHead *search) {
	brEvalListCopyRecord *r;

	while(recordList) {
		r = recordList->data;

		if(r->original == search) return r->copy;

		recordList = recordList->next;
	}

	return NULL;
}

void stFreeListRecords(slList *records) {
	while(records) {
		slFree(records->data);
		records = records->next;
	}
}
