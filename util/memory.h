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

#ifdef MEMORY_DEBUG
#define slFree(p)		slDebugFree((p), __LINE__, __FILE__)
#define slMalloc(p) 	slDebugMalloc((p), __LINE__, __FILE__)
#define slRealloc(p, s) slDebugRealloc((p), (s), __LINE__, __FILE__)
#else
#define slFree(p) free(p)
#define slMalloc(p) slNormalMalloc(p)
#define slRealloc(p, s) realloc(p, s)
#endif /* MEMORY_DEBUG */

enum mallocStatus {
    MS_NULL = 0,
    MS_MALLOC,
    MS_REALLOC,
    MS_FREE
};

/*!
	\brief Data used to track a memory allocation, only used when memory 
	debugging is enabled.
*/

struct slMallocData {
    void *pointer;
    int size;

    char status;

	int line;
	char *file;
};

void *slNormalMalloc(int n);

void *slDebugMalloc(int n, int line, char *file);
void *slDebugRealloc(void *p, int n, int line, char *file);
void slDebugFree(void *p, int line, char *file);

int slMallocHash(void *hash, void *p, slMallocData *d);
slMallocData *slMallocLookup(void *hash, void *p);
slMallocData *slNewUtilMallocData(void *p, int size, int line, char *file);

int slUtilMemoryUnfreed();
void slUtilMemoryFreeAll();

void slMessage(int level, char *format, ...);
char *slDequeueMessage();
int slCheckMessageQueue();

void slFatal(char *format, ...);
void utilPause();
double randomDouble();

void slMemoryReport();
int slMemoryAllocated();

void slDebugMatrix(int level, double m[3][3]);

slHash *slInitUtilMallocHash();

void slPrintIfString(char *pointer, int length, int maxlen);

unsigned int slUtilPointerCompare(void *a, void *b);
unsigned int slUtilPointerHash(void *a, unsigned int n);
#endif
