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
 
/*!
	\brief Used to store temporary data for list-copying.

	Used to correctly copy circular list references.  Instead of recursively
	copying a circular reference, we use the pointer to the previously made 
	copy.
*/

struct brEvalListCopyRecord {
    brEvalListHead *original;
    brEvalListHead *copy;
};

typedef struct brEvalListCopyRecord brEvalListCopyRecord;

#ifdef __cplusplus
extern "C" {
#endif

DLLEXPORT brEvalListHead *brEvalListNew(void);
void brEvalListFree(brEvalListHead *);

int brEvalListCompare(const void *, const void *);

int stDoEvalListIndex(brEvalListHead *, int, brEval *);
inline int stDoEvalListIndexPointer(brEvalListHead *, int, brEval **);

brEvalListHead *brEvalListDeepCopy(brEvalListHead *);
brEvalListHead *stDoEvalListDeepCopy(brEvalListHead *, slList **);
brEvalListHead *brEvalListCopy(brEvalListHead *);

/** 
    \brief Looks up a lists index, using the index if possible.
    
    Updates the index as far as it can as it traverses the list.
*/  

brEvalList *brEvalListIndexLookup(brEvalListHead *, int);

/*!
	\brief Inserts an element into the list, with its value taken from
	the given pointer.
*/  
        
DLLEXPORT int brEvalListInsert(brEvalListHead *, int, brEval *);

/*!
	\brief Removes an arbitrary element from the list and places the removed
	element in the provided brEval.
*/

int brEvalListRemove(brEvalListHead *, int, brEval *);

brEvalListHead *brEvalListDeepCopy(brEvalListHead *);
brEvalListHead *brDoEvalListDeepCopy(brEvalListHead *, slList **);
brEvalListHead *brCopyRecordInList(slList *, brEvalListHead *);
brEvalListCopyRecord *brMakeListCopyRecord(brEvalListHead *, brEvalListHead *);
void brFreeListRecords(slList *);

#ifdef __cplusplus
}
#endif
