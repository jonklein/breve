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
	\brief A simple stack-like data structure.

	slStack is a simple stack-like structure, but it's used mostly as a 
	dynamically resizing array.
*/

struct slStack {
    void **data;
    unsigned int count;
    unsigned int maxCount;
};

slStack *slStackNew(void);
slStack *slStackNewWithSize(unsigned int);
void slStackFree(slStack *);
int slStackPush(slStack *, void *);
int slStackRemove(slStack *, void *);

void slStackClear(slStack *);
