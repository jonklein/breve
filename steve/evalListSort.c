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

#include "steve.h"
#include "evaluation.h"

/*!
    + evalListSort.c
    sorts an brEvalList based on the result of passing the elements
    of the brEvalList to a certain method in the caller object.  This is 
    similar to using the C function qsort--we use qsort to do the actual 
    sorting.

    A problem with qsort is that we can only pass in the function that 
    we are sorting with and not any other variables we want to use with 
    it.  That is to say that we cannot simply pass in the object and 
    method we want to use.  We place them in global variables temporarily.
   
	NON-THREADSAFE: the threadsafe fix is to implement our own
    sort algorithm.  But I'm a laaaaaaaaaaaazy man.

	The global gSortVector is the first time a sort is called and then 
    reused at each subsequent sorting, extending the length of the 
    vector using realloc if needed.
*/

static brEvalList **gSortVector = NULL;
static int gSortVectorLength = 0;
static stMethod *gSortMethod;
static int gEvalListSortError = 0;
static stInstance *gSortObject = NULL;

/*!
	\brief Sorts an evalList with a given method.

    Takes a brEvalListHead to sort, an stInstance and an stMethod to be
    called inside the given stInstance.  The brEvalListHead will be 
    sorted, comparing two elements by calling the given method when 
	necessary.
*/

int stSortEvalList(brEvalListHead *head, stInstance *caller, stMethod *method) {
	brEvalList *l = head->start;
	int n, length = head->count;

	if (head->count < 2)
		return EC_OK;

	/* extend the vector if needed */

	if (gSortVectorLength < length) {
		if (gSortVector)
			gSortVector = (brEvalList **)slRealloc(gSortVector, sizeof(brEvalList *) * length);
        	else
			gSortVector = (brEvalList **)slMalloc(sizeof(brEvalList *) * length);

		gSortVectorLength = length;
	}

	/* load the evalList into the sort vector */

	for (n = 0; n < length; ++n) {
		gSortVector[n] = l;
		l = l->next;
	}

	gEvalListSortError = 0;

	/* set the global method and object pointers and call qsort */

	gSortObject = caller;
	gSortMethod = method;
	qsort(gSortVector, length, sizeof(brEvalList *), brEvalListCompare);
	gSortObject = NULL;
	gSortMethod = NULL;

	/* now reload the sortVector back into the list structure */

	head->start = gSortVector[0];
	head->end = gSortVector[length - 1];

	gSortVector[0]->previous = NULL;
	gSortVector[0]->next = gSortVector[1];

	gSortVector[length - 1]->previous = gSortVector[length - 2];
	gSortVector[length - 1]->next = NULL;

	head->index[0] = head->start;
	head->index[length - 1] = head->end;

	for (n = 1; n < length - 1; ++n) {
		gSortVector[n]->previous = gSortVector[n - 1];
		gSortVector[n]->next = gSortVector[n + 1];
		head->index[n] = gSortVector[n];
	}

	head->indexTop = length - 1;

	if (gEvalListSortError)
		return EC_ERROR;

	/* victoire! */

	return EC_OK;
}

/*
    \brief The evalList qsort comparison function.

    Uses the global variables we've set to call the specified method 
	in the specified instance, with the two elements we are comparing
	as arguments.
*/

int brEvalListCompare(const void *a, const void *b) {
	stRunInstance ri;
	brEval result;
	brEval *args[2];
	int rcode;

	/* get the two brEvalList elements */

	brEvalList *al = *(brEvalList **)a;
	brEvalList *bl = *(brEvalList **)b;

	args[0] = &al->eval;
	args[1] = &bl->eval;

	// if we encounted an error at a previous iteration, no need to continue

	if (gEvalListSortError)
		return 0;

	/* pass the two evals as arguments to the sort method */

	result.type = AT_NULL;
	
	ri.instance = gSortObject;
	ri.type = gSortObject->type;

	rcode = stCallMethod(&ri, &ri, gSortMethod, args, 2, &result);

	if (rcode == EC_ERROR)
		gEvalListSortError = 1;

	/* convert the answer to an AT_DOUBLE */

	if (result.type != AT_DOUBLE) {
		rcode = stToDouble(&result, &result, &ri);
		if (rcode == EC_ERROR)
			gEvalListSortError = 1;
	}

	/* return the value of the method call */

	if (BRDOUBLE(&result) > 0.0)
		return 1;
	if (BRDOUBLE(&result) < 0.0)
		return -1;

	return 0;
}

/*
	\brief Frees memory associated with the evalList sorting routines.
*/

void brEvalListFreeSortVars() {
	if (gSortVector) {
		slFree(gSortVector);
		gSortVectorLength = 0;
		gSortVector = NULL;
	}
}
