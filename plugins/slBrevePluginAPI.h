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
			the breve simulation environment plugin API version 2.0

	Documentation on using the breve plugin API is included in
	the documentation distributed with breve.
*/

// These are depreciated symbol names, 
// included here for backwards compatability

#define stNewBreveCall brNewBreveCall
#define stEval brEval
#define stHash brEvalHash
#define stEvalList brEvalList
#define stEvalListHead brEvalListHead
#define stNewEvalList brEvalListNew

#define stEvalListAppend(list, element) brEvalListInsert((list), (list)->count, (element))

#include <stdio.h>
#include <stdlib.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

typedef struct slVector slVector;
typedef struct brEval brEval;
typedef struct brEvalListHead brEvalListHead;
typedef struct brEvalList brEvalList;
typedef struct brEvalHash brEvalHash;
typedef struct brData brData;
typedef struct stInstance stInstance;
typedef struct brInstance brInstance;

// breve output error codes 

#define EC_ERROR -1		/* causes the simulation to terminate */
#define EC_OK 1			/* normal output--simulation continues */

// these entries appear in the brEval type field

enum atomicTypes {
	AT_UNDEFINED = 0,
	AT_NULL,
	AT_INT,
    AT_DOUBLE,
	AT_STRING,
	AT_INSTANCE,
	AT_BRIDGE_INSTANCE,
	AT_POINTER,
	AT_VECTOR,
	AT_MATRIX,
	AT_LIST,
	AT_ARRAY,
	AT_DATA,
	AT_HASH,
	AT_TYPE
};

struct slVector {
	double x;
	double y;
	double z;
};

struct brEval {
    union {
        double doubleValue;
        int intValue;
        slVector vectorValue;
        double matrixValue[3][3];
        void *pointerValue;
        char *stringValue;
        brEvalHash *hashValue;
        brData *dataValue;
        brInstance *instanceValue;
        brEvalListHead *listValue;
    } values;

    unsigned char type;
};

/*
	lists in steve are held internally by the brEvalListHead which
	holds a doubly-linked list of brEvalList structures.  

	do not modify this structure.
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

struct brEvalList {
    brEval eval;

    brEvalList *next;
    brEvalList *previous;
};

/*
 * the brData struct and associated functions.
 */

struct brData {
    void *data;
    int length;
    int retainCount;
};

brData *brDataNew(void *data, int length);
void brDataFree(brData *data);

char *slStrdup(char *string);

/* 
	the following functions are used to create & edit evaluation lists.
	you can create brEvalLists to be returned by your functions.  don't
	worry about freeing brEvalLists, this is done by the engine.
*/

brEvalListHead *brEvalListNew();
brEval *brEvalListPrepend(brEvalListHead *a, brEval *eval);
brEval *brEvalListAppend(brEvalListHead *a, brEval *eval);

/* use these macros to treat brEval pointers like specific types */

#define BRINT(e)        ((e)->values.intValue)
#define BRFLOAT(e)      ((e)->values.doubleValue)
#define BRDOUBLE(e)     ((e)->values.doubleValue)
#define BRSTRING(e)     ((e)->values.stringValue)
#define BRVECTOR(e)     ((e)->values.vectorValue)
#define BRMATRIX(e)     ((e)->values.matrixValue)
#define BRINSTANCE(e)     ((e)->values.instanceValue)
#define BRPOINTER(e)    ((e)->values.pointerValue)
#define BRDATA(e)       ((e)->values.dataValue)
#define BRHASH(e)       ((e)->values.hashValue)
#define BRLIST(e)       ((e)->values.listValue)

#define STINT		BRINT
#define STFLOAT		BRFLOAT
#define STDOUBLE	BRDOUBLE
#define STSTRING	BRSTRING
#define STVECTOR	BRVECTOR
#define STMATRIX	BRMATRIX
#define STINSTANCE	BRINSTANCE
#define STPOINTER	BRPOINTER
#define STDATA		BRDATA
#define STHASH		BRHASH
#define STLIST		BRLIST

int brNewBreveCall(void *n, char *name, int (*call)(brEval *argumentArray, brEval *returnValue, void *callingInstance), int rtype, ...);

/*
	to call breve methods from your plugin, use the following method.  
	the arguments to the method should be placed in an array of brEval
	structs in the order that they appear in the methods definition
	(keywords are not used with this interface).  you must also supply
	the number of arguments supplied.

	the brEval pointed to in result will return the return value (if 
	any) of the method.

	the return value of the function itself is either EC_OK or EC_ERROR
	depending on whether an error occurred while calling the specified
	method.
*/

int stCallMethodByNameWithArgs(void *instance, char *name, brEval **args, int argcount, brEval *result);

/*
	to print error messages to the breve console, use the function slMessage.
	the first argument should always be DEBUG_ALL.  the second argument is a 
	"printf" style format string, and subsequent arguments correspond to the 
	arguments referenced in the format string.
*/

#define DEBUG_ALL	0

int slMessage(int level, char *format, ...);

/*
	slMalloc and slFree are plugin replacements for malloc and free which 
	do some simple memory management.  Their use is optional, but if you 
	choose to use one, then you must use the other as well.  An slMalloc
	call must be balanced by an slFree.
*/

void *slMalloc(int size);
void *slRealloc(void *pointer, int size);
void slFree(void *pointer);

/*
	pass in the callingInstance pointer, and this function returns 
	a FILE* to which fprintf output can be directed in order to send 
	it to the breve log.
*/

FILE *slGetLogFilePointer(void *callingInstance);

char *stPluginFindFile(char *file, void *i);

#ifdef __cplusplus
}
#endif /* __cplusplus */
