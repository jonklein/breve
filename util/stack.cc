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
    + stack.c 

    = not (just) a traditional push/pop stack, but can rather be thought
    = of as an array which dynamically resizes.
*/

#include "util.h"

/*!
	\brief Creates a new stack with a default size (8).

    Calls slStackNewWithSize to create a new stack with the default 
	starting size, 8.
*/

slStack *slStackNew() {
    return slStackNewWithSize(8);
}

/*!
    \brief Creates a new stack with a specified starting size.  
*/

slStack *slStackNewWithSize(unsigned int size) {
    slStack *s;

    if(size < 1) size = 1;

    s = new slStack;
    s->count = 0;
    s->maxCount = size;
    s->data = slMalloc(sizeof(void*) * s->maxCount);

    return s;
}

/*!
    \brief Frees the stack structure

	Does not free any data you may have pushed onto the stack.  You have to 
	do that yourself, you lazy git!
*/

void slStackFree(slStack *s) {
	if(!s) return;

    slFree(s->data);
    delete s;
}

/*!
    \brief Pushes data onto a stack.
*/

int slStackPush(slStack *s, void *data) {
    if(s->count == s->maxCount) {
        s->maxCount *= 2;
        s->data = slRealloc(s->data, sizeof(void*) * s->maxCount);
    }

    s->data[s->count] = data;

    s->count++;

    return s->count;
}

/*!
    \brief Pop (and return) the highest element off the stack.  
*/

void *slStackPop(slStack *s) {
    if(s->count == 0) return NULL;

    s->count--;
    return s->data[s->count];
}

/*!
	\brief Remove an element from the middle of a stack.

	What the...?  You can't remove elements from the middle of a stack!
	ARE YOU MAD!?  You'll destroy the stack paradigm upon which all
	of computer science is based!  Won't somebody please think of the
	children!?!
*/

int slStackRemove(slStack *s, void *remove) {
	unsigned int n;

	if(s->count == 0) return 0;

	for(n=0;n<s->count;n++) {
		if(s->data[n] == remove) {
			while(n < (s->count - 1)) {
				s->data[n] = s->data[n+1];
				n++;
			}

			s->count--;
			return 1;
		}
	}

	return 0;
}

/*!
	\brief Clears the stack and resets the stack counter.
*/

void slStackClear(slStack *s) {
    s->count = 0;
}
