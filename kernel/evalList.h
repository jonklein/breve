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

brEvalListHead *brEvalListNew();

/*!
	\brief A breve/steve list datatype.

	This is the "head" structure which contains a doubly-linked list, 
	used to store lists of breve/steve expressions.  Indexed for fast 
	random-access.  Reference counting enabled.
*/

struct brEvalListHead {
	int count;
	int retainCount;
	brEvalList *start;
	brEvalList *end;

	int indexSize;
	int indexTop;
	brEvalList **index;
};

/*!
	\brief An element in an eval-list.

	Used in conjunction with brEvalListHead, these are the elements of the 
	doubly-linked list.
*/

struct brEvalList {
	brEval eval;

	brEvalList *next;
	brEvalList *previous;
};

void brEmptyEvalList(brEvalListHead *lh);

int stCountEvalList(brEvalListHead *a);

void brEvalListFree(brEvalListHead *a);

int brEvalListCompare(const void *a, const void *b);

int stDoEvalListIndex(brEvalListHead *l, int n, brEval *newLoc);
inline int stDoEvalListIndexPointer(brEvalListHead *l, int n, brEval **eval);

brEvalListHead *brEvalListDeepCopy(brEvalListHead *l);
brEvalListHead *stDoEvalListDeepCopy(brEvalListHead *l, slList **seen);
brEvalListHead *brEvalListCopy(brEvalListHead *l);

brEvalList *brEvalListIndexLookup(brEvalListHead *l, int index);

int brEvalListInsert(brEvalListHead *head, int index, brEval *value);
int brEvalListRemove(brEvalListHead *head, int index, brEval *value);

brEvalList *brEvalListIndexLookup(brEvalListHead *l, int index);

