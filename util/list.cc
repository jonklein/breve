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
    = list.c -- a set of general purpose linked list functions.  used 
    = quite frequently by many different areas of the project

    = it also includes functions dealing with the slArray datatype.  
    = an slArray is like a list in that it stores a set of (void*)
    = data, but unlike a list in that it is a set number of entries
    = which cannot be changed.  it has the advantage that it offers
    = faster random access of variables.

    = often data sets will be creates as lists (during text parsing,
    = for example) and then converted to arrays when they contain
    = their final contents.
*/

#include "util.h"

/* 
    = slListPrepend prepends an element (of type void*) onto a list.
    = the function returns the NEW list head.  functions using this
    = function should thus reassign the existing list pointer, such as:
    = myList = slListPrepend(myList, newElement);

    = if the list is NULL, then the element returned is the only 
    = element of a brand new list.
*/

slList *slListPrepend(slList *l, void *d) {
    slList *newList;

    newList = malloc(sizeof(slList));
   
    if(!newList) return NULL;

    newList->data = d;
    newList->next = l;

    return newList;
}

/*
    = slListAppend appends an element (of type void*) onto the end
    = of a list.  the list may once again be NULL, in which case
    = a new list is returned with a single element.

    = the function always returns the head of the list whether
    = it is a new list, or simply the same list head that was
    = passed in.  

    = not as efficient as a list prepend because the whole
    = list must be traversed before the elment is added.  
    = consider using a series of slListPrepend()s followed
    = by an slReverseList.
*/

slList *slListAppend(slList *l, void *d) {
    slList *newEntry;
    slList *orig = l;

    newEntry = slListPrepend(NULL, d);

    if(!newEntry) return NULL;

    if(!l) return newEntry;

    while(l->next) l = l->next;

    l->next = newEntry;

    return orig;
}

/*
	= slListRemoveData removes the element of the list
	= which has the specified pointer.  note that this
	= returns a pointer to the "new" list structure.  
	= the new list structure may be different than
	= the old (in the event that the first element is 
	= removed)!
*/

slList *slListRemoveData(slList *s, void *p) {
	slList *previous = NULL;
	slList *list = s;

	while(s && s->data != p) {
		previous = s;
		s = s->next;
	}

	if(previous) {
		if(s) previous->next = s->next;
	} else {
		if(s) list = s->next;
		else list = NULL;
	}

	if(s) free(s);
	
	return list;
}

slList *slInList(slList *s, void *p) {
	while(s) {
		if(s->data == p) return s;
		s = s->next;
	}

	return NULL;
}

/*
    = slListReverse takes a list and reverses its order,
    = making the first element the last and the last element 
    = the first.  
*/

slList *slListReverse(slList *s) {
    slList *previous = NULL, *next;

    while(s) {
        next = s->next;

        s->next = previous;

        previous = s;

        s = next;
    }

    return previous;
}

/*!
	\brief Makes a shalow copy of a list.
*/

slList *slListCopy(slList *s) {
	slList *result = NULL;

	while(s) {
		result = slListAppend(result, s->data);
		s = s->next;
	}

	return result;
}

/*
	\brief Frees a list, but not its contents.

	Since this function does not free the contents of the list,
	make sure to iterate through the list and free the data pointers 
	first.
*/

void slListFree(slList *l) {
    slList *next;

	if(!l) return;

    while(l) {
        next = l->next;
        free(l);    
        l = next;
    }
}

/*!
	\brief Free a list head.

    Since the lists use free() and malloc() instead of slFree() and 
    slMalloc(), we give the users an interface to just free a single
    element of the list themselves so that they are not expected to 
    know to do a raw free() on something they did not explicitly
    allocate themselves.
*/

void slListFreeHead(slList *l) {
    if(!l) return;

    free(l);
}

/*!
    \brief Returns the number of elements in a list.
*/

int slListCount(slList *l) {
    int n = 0;

    while(l) {
        n++;
        l = l->next;
    }

    return n;
}

/*!
    \brief Converts an slList to an slArray containing pointers to the same data.  
	
	This function does not free the original slList--you should do this yourself.  
	However, you should NOT free the list's data in this case, since the new slArray
    will be pointing to the same memory.
*/

slArray *slListToArray(slList *l) {
    slArray *a;
    int n;

    a = malloc(sizeof(slArray));

	if(!l) {
		a->count = 0;
		a->data = NULL;
		return a;
	}

    a->count = slListCount(l);

    a->data = malloc(sizeof(void*) * a->count);

    n = 0;

    while(l) {
        a->data[n++] = l->data;
        l = l->next;
    }

    return a;
}

/*!
    \brief Creates an empty array of a specific size.
*/

slArray *slNewArray(int count, int size) {
    slArray *a;
    int n;

    a = malloc(sizeof(slArray));

    a->count = count;

    a->data = malloc(sizeof(void*) * a->count);

    if(size) {
        for(n=0;n<count;n++) a->data[n] = malloc(size);
    }

    return a;
}

/*!
	\brief Frees an slArray structure, 

	This function does not free the slArray data contents.  If you use this 
	function, make sure you iterate through the array and free the data first!
*/

void slFreeArray(slArray *a) {
    if(a->data) free(a->data);
    free(a);
}

void slPrintList(slList *l) {
	int first = 1;

	printf(" { ");

	while(l) { 
		if(first) printf("%p", l->data);
		else printf(", %p", l->data);

		first = 0;

		l = l->next;
	}

	printf(" } ");
}
