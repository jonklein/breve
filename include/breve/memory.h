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

#ifndef _MEMORY_H
#define _MEMORY_H
#include <stdio.h>

void *slMalloc(int);
void *slRealloc(void *, int);
void slFree(void *);

char *slDequeueMessage(void);
int slCheckMessageQueue(void);

void slFatal(char *, ...);
void utilPause(void);

void slMemoryReport(void);
int slMemoryAllocated(void);

void slDebugMatrix(int, double [3][3]);

slHash *slInitUtilMallocHash(void);

void slPrintIfString(char *, int, int);

unsigned int slUtilPointerCompare(void *, void *);
unsigned int slUtilPointerHash(void *, unsigned int);
#endif
