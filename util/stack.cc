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
	\brief Creates a new stack with a default size (0).

	Creates an empty stack with the default capacity 0.  Capacity 
	is automatically increased to 8 on the first "push".
*/

slStack *slStackNew() {
	slStack *s = new slStack;

	s->maxCount = 0;
	s->data = NULL;
	s->count = 0;

	return s;
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
	s->data = (void**)slMalloc(sizeof(void*) * s->maxCount);

	return s;
}

/*!
	\brief Pushes data onto a stack.
*/

int slStackPush(slStack *s, void *data) {
	if(s->count == s->maxCount) {
		if(s->maxCount == 0) {
			s->maxCount = 8;
			s->data = (void**)slMalloc(sizeof(void*) * s->maxCount);
		} else {
			s->maxCount *= 2;
			s->data = (void**)slRealloc(s->data, sizeof(void*) * s->maxCount);
		}
	}

	s->data[s->count] = data;

	s->count++;

	return s->count;
}

/*!
	\brief Returns an element from the stack.
*/

void *slStackGet(slStack *s, int index) {
	return s->data[index];
}

/*!
	\brief Returns the number of elements in the stack.
*/

unsigned int slStackSize(slStack *s) {
	return s->count;
}
