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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <sys/types.h>
#include <unistd.h>

#include "util.h"

/* 
    memory.c defines a set of malloc wrappers and corresponding 
    utilities.

    the point of having our own malloc is to be able to track
    memory usage more effectively.  one thing to be done is 
    build a simple hash table to relate a pointer with some 
    debugging information like ammount of memory allocated 
*/

#define UM_HASH_SIZE	2048

/* these are some evil global variables */

static int gMallocBytes = 0;

static slHash *gUtilMallocHash = NULL;

/*!
	\brief Initialize the hash table which keeps track of memory allocation.
*/

slHash *slInitUtilMallocHash() {
    return slNewHash(UM_HASH_SIZE, slHashPointer, slCompPointer);
}

/*!
	\brief The breve replacement for malloc().

	When compiled without debugging, this function simply preforms a malloc() and
	sets the memory to 0.  With debugging enabled, this function keeps track of 
	all memory allocated and released during a simulation.

	This function should always be used in place of malloc(), except in situations
	that could cause an infinite loop (which is to say, it cannot be used in other
	functions that track memory usage).
*/


void *slDebugMalloc(int n, int line, char *file) {
    void *m;
    slMallocData *d;

    m = malloc(n);
	bzero(m, n);

    if(!gUtilMallocHash) gUtilMallocHash = slInitUtilMallocHash();

    if(!m) {
        fprintf(stderr, "malloc failed on %d bytes\n", n);
		slStackTrace();
        return NULL;
    }

    if((d = slDehashData(gUtilMallocHash, m))) { 
        if(d->status == MS_MALLOC) {
            fprintf(stderr, "warning: malloc returned already malloced pointer\n");
        }

        d->size = n; 
        d->status = MS_MALLOC;
    } else {
        d = slHashData(gUtilMallocHash, m, slNewUtilMallocData(m, n, line, file));
    }

    gMallocBytes += n;

    return m;
}

#ifndef MEMORY_DEBUG
void *slMalloc(int n) {
	void *m = malloc(n);
	bzero(m, n);
	return m;
}

void *slRealloc(void *p, int n) {
	return realloc(p, n);
}

void slFree(void *p) {
	free(p);
	return;
}
#endif

void *slDebugRealloc(void *p, int n, int line, char *file) {
    void *m;
    slMallocData *d;

    if(!gUtilMallocHash) slInitUtilMallocHash();

    /* find the old pointer */

    d = slDehashData(gUtilMallocHash, p);

    if(!d) {
        fprintf(stderr, "attempting to reallocate unknown pointer %p at line %d of file \"%s\"\n", p, line, file);
		slStackTrace();
        return slMalloc(n);
    }

    if(d->status == MS_FREE) {
        fprintf(stderr, "attempt to reallocate a freed pointer %p at line %d of file \"%s\"\n", p, line, file);
		slStackTrace();
        return slMalloc(n);
    } else if(d->status == MS_REALLOC) {
        fprintf(stderr, "attempt to reallocate a reallocated pointer %p at line %d of file \"%s\"\n", p, line, file);
		slStackTrace();
        return slMalloc(n);
    }

    /* realloc the old pointer */

    m = realloc(p, n);

    if(!m) {
        fprintf(stderr, "realloc of %p failed on %d bytes\n", m, n);
        return NULL;
    }

    gMallocBytes += (n - d->size);

    // if we get the same pointer back, change the size 
    // otherwise, change the status of the old, and create a new entry 

    if(m == p) {
        d->size = n;
    } else {
        d->status = MS_REALLOC;

        d = slDehashData(gUtilMallocHash, m);

        if(d) {
            d->status = MS_MALLOC;
            d->size = n;
			d->line = line;
			d->file = file;
        } else d = slHashData(gUtilMallocHash, m, slNewUtilMallocData(m, n, line, file));
    }

    return m;
}

void slDebugFree(void *p, int line, char *file) {
    slMallocData *d;

    d = slDehashData(gUtilMallocHash, p);

    if(!d) {
        fprintf(stderr, "attempt to free an unallocated pointer %p at line %d of file \"%s\"\n", p, line, file);
		slStackTrace();
        return;
    }

    if(d->status == MS_FREE) {
        fprintf(stderr, "attempt to free a freed pointer %p at line %d of file \"%s\"\n", p, line, file);
		slStackTrace();
        return;
    } else if (d->status == MS_REALLOC) {
        fprintf(stderr, "attempt to free a reallocateded pointer %p at line %d of file \"%s\"\n", p, line, file);
		slStackTrace();
        return;
	}

    gMallocBytes -= d->size;

    d->status = MS_FREE;

    free(p);
}

/*!
	\brief Print memory allocation data to the log.

	Only enabled when MEMORY_DEBUG is defined.
*/

void slMemoryReport() {
    slMessage(DEBUG_ALL, "%d bytes allocated\n", gMallocBytes);
}

/*!
	\brief Returns the number of bytes currently allocated.

	Only enabled when MEMORY_DEBUG is defined.
*/


int slMemoryAllocated() {
    return gMallocBytes;
}

/*!
	\brief Print out all unfreed memory allocations.
*/

int slUtilMemoryUnfreed() {
    unsigned int n;
    int count = 0;
	int debug = 0;
    slList *l;
    slMallocData *d;

    if(!gUtilMallocHash) return 0;
    
	debug = (getenv("MallocStackLogging") || getenv("MallocStackLoggingNoCompact"));

    for(n=0;n<gUtilMallocHash->size;n++) {
        l = gUtilMallocHash->buckets[n];

        while(l) {
            d = ((slHashEntry*)l->data)->data;

            if(d->status == MS_MALLOC) {
				char command[1024];

                slMessage(DEBUG_ALL, "%p: %d bytes still allocated at line %d of file \"%s\" ", d->pointer, d->size, d->line, d->file);
				slPrintIfString(d->pointer, 30, d->size);
				slMessage(DEBUG_ALL, "\n");

				if(debug) {
					sprintf(command, "malloc_history %d %p 2> /dev/null", getpid(), d->pointer);
					system(command);
				}

                count += d->size;
            }

            l = l->next;
        }
    }

    return count;
}

/*!
	\brief If the string is a pointer, print out a portion of it.

	For memory debugging messages.
*/

void slPrintIfString(char *pointer, int length, int maxlen) {
	char *output;
	int n;

	if(length > maxlen) length = maxlen;

	output = alloca(length + 20);

	for(n=0;n<maxlen;n++) {
		if(pointer[n] < 0) return;
	}

	n = strlen(pointer);

	if(n >= length) {
		strncpy(output, pointer, length);
		output[length] = output[length + 1] = output[length + 2] = '.';
		output[length + 3] = 0;
	} else sprintf(output, "%s", pointer);
	
	slMessage(DEBUG_ALL, "\"%s\"", output);
}

/*!
	\brief Create a new allocation record.

	When memory debugging is enabled, \ref slMallocData structs are 
	used to track all allocations.  This function creates a record 
	of a single allocation.
*/

slMallocData *slNewUtilMallocData(void *p, int size, int line, char *file) {
    slMallocData *d;

    d = malloc(sizeof(slMallocData));

    d->pointer = p;
    d->size = size;
    d->status = MS_MALLOC;
    d->line = line;
    d->file = file;

    return d;
}

/*!
	\brief Free all memory allocated by the slMalloc routine.  

	I can't say i really recommend calling this method, but you're 
	a grown up now so you can make your own decissions. 
*/

void slUtilMemoryFreeAll() {
	unsigned int n;
	slList *l;
	slMallocData *d;
					    
	if(!gUtilMallocHash) return;

	for(n=0;n<gUtilMallocHash->size;n++) {
		l = gUtilMallocHash->buckets[n];

		while(l) {
			d = ((slHashEntry*)l->data)->data;
			if(d->status == MS_MALLOC) slFree(d->pointer);
			l = l->next;
		}
	}
}
