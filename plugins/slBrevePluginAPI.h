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
	The breve Simulation Environment plugin API version 2.3

	Documentation on using the breve plugin API is included in
	the documentation distributed with breve.
*/

/* These are deprecated symbol names, 
   included here for backwards compatability */

#define stNewBreveCall brNewBreveCall
#define stNewSteveCall brNewBreveCall
#define stEval brEval
#define stHash brEvalHash
#define stEvalList brEvalList
#define stEvalListHead brEvalListHead
#define stNewEvalList brEvalListNew
#define stPluginFindFile brPluginFindFile

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

#include <stdio.h>
#include <stdlib.h>

/* Plugin entry-point functions should be declared as DLLEXPORT */

#if defined(_MSC_VER) || defined(__WIN32__) || defined(WINDOWS)
#ifdef __cplusplus
#define DLLEXPORT extern "C" __declspec(dllexport)
#else
#define DLLEXPORT __declspec(dllexport)
#endif
#elif defined(__cplusplus)
#define DLLEXPORT extern "C"
#else
#define DLLEXPORT extern
#endif

#ifdef __cplusplus
extern "C" {
#endif

typedef struct slVector slVector;
typedef struct brEval brEval;
typedef struct brEvalListHead brEvalListHead;
typedef struct brEvalList brEvalList;
typedef struct brEvalHash brEvalHash;
typedef struct brData brData;
typedef struct stInstance stInstance;
typedef struct brInstance brInstance;

/* These entries appear in the brEval type field. */

enum atomicTypes {
    AT_INVALID = 0,
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
    AT_TYPE,
    AT_UNDEFINED
};

struct slVector {
	double x, y, z;
};

typedef double slMatrix[3][3];

struct brEval {
    union {
        double doubleValue;
        int intValue;
        slVector vectorValue;
        slMatrix matrixValue;
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
	Lists in breve are held internally by the brEvalListHead which
	holds a doubly-linked list of brEvalList structures.  

	Do not modify this structure.
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
 * The brData struct and associated functions.
 */

struct brData {
    void *data;
    int length;
    int retainCount;
};

brData *brDataNew(void *data, int length);
void brDataFree(brData *data);

/* 
	The following functions are used to create & edit evaluation lists.
	You can create brEvalLists to be returned by your functions.
	Don't worry about freeing brEvalLists, this is done by the engine.
*/

brEvalListHead *brEvalListNew(void);
int brEvalListInsert(brEvalListHead *head, int index, brEval *value);
#define brEvalListAppend(a, eval) brEvalListInsert((a), (a)->count, (eval))

/* Use these macros to treat brEval pointers as specific types. */

#define BRINT(e)	((e)->values.intValue)
#define BRFLOAT(e)	((e)->values.doubleValue)
#define BRDOUBLE(e)	((e)->values.doubleValue)
#define BRSTRING(e)	((e)->values.stringValue)
#define BRVECTOR(e)	((e)->values.vectorValue)
#define BRMATRIX(e)	((e)->values.matrixValue)
#define BRINSTANCE(e)	((e)->values.instanceValue)
#define BRPOINTER(e)	((e)->values.pointerValue)
#define BRDATA(e)	((e)->values.dataValue)
#define BRHASH(e)	((e)->values.hashValue)
#define BRLIST(e)	((e)->values.listValue)

#define EC_ERROR -1
#define EC_OK 1

int brNewBreveCall(void *n, char *name, int (*call)(brEval argumentArray[],
	brEval *returnValue, void *callingInstance), int rtype, ...);

	/*
	 * The brMethodCallByNameWithArgs() function calls the breve method
	 * named by the string _name_ with _argcount_ number of arguments
	 * in the array pointed to by _args_ and stores the return value of the
	 * method (if any) in the brEval pointed to by _result_.
	 *
	 * The function returns EC_OK if successful.
	 * If an error occurs while calling the specified method, EC_ERROR is
	 * returned and the simulation is halted.
	 */

int brMethodCallByNameWithArgs(void *instance, char *name, brEval **args, int count, brEval *result);

	/*
	 * The slMessage() function prints a formatted error message to the
	 * breve console. The _level_ argument must always be DEBUG_ALL.
	 * The _fmt_ specification (and associated arguments) may be any format
	 * allowed by printf(3) or a simple string.
	 */

#define DEBUG_ALL	0

void slMessage(int level, const char *fmt, ...);

	/*
	 * The slMalloc() function allocates space for an object of _size_ bytes
	 * and initializes the space to all bits zero.
	 *
	 * The slFree() function makes the space allocated to the object
	 * pointed to by _ptr_ available for further allocation.
	 *
	 * The slRealloc() function changes the size of the object pointed to
	 * by _ptr_ to _size_ bytes and returns a pointer to the object.
	 *
	 * It is an error to call slFree() or slRealloc() for an object which
	 * was not returned by a previous call to slMalloc() or slRealloc().
	 *
	 * The slMalloc() and slRealloc() functions return NULL on error.
	 */

#define slMalloc(n) calloc(1,n)
#define slRealloc(p,n) realloc(p,n)
#define slFree(p) free(p)

	/*
	 * The slStrdup() function allocates space for a copy of the string
	 * pointed to by _s_, copies the string, and a returns a pointer
	 * to the copied string. The returned pointer may subsequently be
	 * used as an argument to the slFree() and slRealloc() functions.
	 *
	 * If the string cannot be copied, NULL is returned.
	 */

char *slStrdup(const char *s);

	/*
	 * The slGetLogFilePointer() function returns a FILE pointer
	 * referring to the breve output log, typically stderr.
	 */

FILE *slGetLogFilePointer(void *callingInstance);

	/*
	 * The brPluginFindFile() function finds a file in the engine's
	 * search path and returns the pathname to that file.
	 *
	 * The returned pointer, if it is not NULL, should be deallocated
	 * with slFree() when it is no longer needed by the plugin.
	 */

char *brPluginFindFile(char *file, void *callingInstance);

#ifdef __cplusplus
}
#endif
