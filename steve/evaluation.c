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

/*!
	+ evaluation.c
	= is the heart of steve.  it takes parse trees and executes them.
	= the most basic entity is the stExp which is a wrapper around all
	= the other expression types such as literal numbers and strings,
	= binary expressions, method calls, etc.  see expression.c and 
	= steveparse.y which show what these expressions look like and how
	= the parse trees are created.
	=
	= CRITICAL -- memory management of strings and evalLists
	= the brEval struct holds the values computed and passed around.  
	= ints, doubles and vectors are stored as their actual values,
	= while objects, pointers, strings and evalLists are stored as
	= pointers.
	= 
	= if you receive an brEval through a call to stExp AND if you don't 
	= return it to your calling function, you have to call stUnretainEval
	= on it.
*/

/* 
  ERROR RULES:
	  * call stEvalError if you originate the error 
		(return EC_ERROR for the first time)
	  * otherwise call slMessage for a regular debug output
*/

/* and their human readable string names for printing errors */

char *slAtomicTypeStrings[] = {
	NULL,
	"NULL",
	"int",
	"double",
	"string",
	"object",
	"pointer",
	"vector",
	"matrix",
	"list",
	"array",
	"data",
	"hash"
};

/*!
	\brief Returns a pointer for an evaluation in steve.

	For an array access, list access or regular load, this function gives a 
	pointer to the desired data.  This lets us have a unified approach for 
	variable lookups.
*/

inline int stPointerForExp(stExp *expression, stRunInstance *i, void **pointer, int *type) {
	switch(expression->type) {
		case ET_LOAD:
			return stEvalLoadPointer(expression->values.pValue, i, pointer, type);
			break;
		case ET_ARRAY_INDEX:
			return stEvalArrayIndexPointer(expression->values.pValue, i, pointer, type);
			break;
		case ET_LIST_INDEX:
			return stEvalListIndexPointer(expression->values.pValue, i, pointer, type);
			break;
		default:
			slMessage(DEBUG_ALL, "Invalid value for left-hand side of assignment expression");
			return EC_ERROR;
			break;
	}

	return EC_ERROR;
}

/*!
	\brief Returns a pointer for a regular load expression.
*/

inline int stEvalLoadPointer(stLoadExp *e, stRunInstance *i, void **pointer, int *type) {
	*type = e->type;

	if(e->local) *pointer = &i->instance->type->engine->stack[e->offset];
	else *pointer = &i->instance->variables[e->offset];

	return EC_OK;
}

/*!
	\brief Convert an evaluation an int in steve.
*/

inline int stToInt(brEval *e, brEval *t, stRunInstance *i) {
	char *str;
	brEvalListHead *theList;
	int result;

	switch(e->type) {
		case AT_INT:
			result = BRINT(e);
			break;
		case AT_DOUBLE:
			result = (int)BRDOUBLE(e);
			break;
		case AT_STRING:
			str = BRSTRING(e);
			if(str) result = atoi(BRSTRING(e));
			else result = 0;
			break;
		case AT_LIST:
			/* copy the list to a local variable, so that we can free it */
			/* after assigning to the target (and possibly overwriting   */
			/* the original when e == t) */

			theList = BRLIST(e);
			result = theList->count;
			break;
		case AT_VECTOR:
			stEvalError(i->instance->type->engine, EE_CONVERT, "cannot convert type \"vector\" to type \"int\"");
			return EC_ERROR;
			break;
		case AT_INSTANCE:
			stEvalError(i->instance->type->engine, EE_CONVERT, "cannot convert type \"object\" to type \"int\"");
			return EC_ERROR;
			break;
		case AT_POINTER:
			stEvalError(i->instance->type->engine, EE_CONVERT, "cannot convert type \"pointer\" to type \"int\"");
			return EC_ERROR;
			break;
		case AT_NULL:
			stEvalError(i->instance->type->engine, EE_CONVERT, "cannot convert NULL expression to type \"int\"");
			return EC_ERROR;
			break;
		case AT_HASH:
			stEvalError(i->instance->type->engine, EE_CONVERT, "cannot convert hash expression to type \"int\"");
			return EC_ERROR;
			break;
		default:
			stEvalError(i->instance->type->engine, EE_INTERNAL, "unknown atomic type (%d) in stToInt", e->type);
			return EC_ERROR;
			break;
	}
	
	BRINT(t) = result;
	t->type = AT_INT;

	return EC_OK;
}

/*!
	\brief Convert an evaluation a double in steve.
*/

inline int stToDouble(brEval *e, brEval *t, stRunInstance *i) {
	char *str;
	brEvalListHead *theList;
	double result;

	switch(e->type) {
		case AT_DOUBLE:
			result = BRDOUBLE(e);
			break;
		case AT_INT:
			result = (double)BRINT(e);
			break;
		case AT_STRING:
			str = BRSTRING(e);
			if(str) result = atof(BRSTRING(e));
			else result = 0.0;
			break;
		case AT_LIST:
			theList = BRLIST(e);
			result = (double)theList->count;
			break;
		case AT_VECTOR:
			stEvalError(i->instance->type->engine, EE_CONVERT, "cannot convert vector expression to type \"double\"");
			return EC_ERROR;
			break;
		case AT_NULL:
			stEvalError(i->instance->type->engine, EE_CONVERT, "cannot convert NULL expression to type \"double\"");
			return EC_ERROR;
			break;
		case AT_INSTANCE:
			stEvalError(i->instance->type->engine, EE_CONVERT, "cannot convert object expression to type \"double\"");
			return EC_ERROR;
			break;
		case AT_HASH:
			stEvalError(i->instance->type->engine, EE_CONVERT, "cannot convert hash expression to type \"double\"");
			return EC_ERROR;
			break;
		default:
			stEvalError(i->instance->type->engine, EE_INTERNAL, "unknown atomic type (%d) in stToDouble", e->type);
			return EC_ERROR;
			break;
	}

	BRDOUBLE(e) = result;
	t->type = AT_DOUBLE;

	return EC_OK;
}

/*!
	\brief Attempt to convert between arbitrary types in steve.
*/

inline int stToType(brEval *e, int type, brEval *t, stRunInstance *i) {
	if(e->type == type) return EC_OK;

	switch(type) {
		case AT_DOUBLE:
			return stToDouble(e, t, i);
			break;
		case AT_INT:
			return stToInt(e, t, i);
			break;
		case AT_STRING:
			stEvalError(i->instance->type->engine, EE_CONVERT, "cannot convert type \"%s\" to type \"string\"", slAtomicTypeStrings[e->type]);
			return EC_ERROR;
			break;
		case AT_VECTOR:
			stEvalError(i->instance->type->engine, EE_CONVERT, "cannot convert type \"%s\" to type \"vector\"", slAtomicTypeStrings[e->type]);
			return EC_ERROR;
			break;
		case AT_INSTANCE:
			/* the only legal conversion to object is when the int interpretation is 0--a NULL object */

			if((e->type == AT_DOUBLE && BRDOUBLE(t) == 0.0) || (e->type == AT_INT && BRINT(t) == 0)) {
				BRINSTANCE(t) = NULL;
				t->type = AT_INSTANCE;
				return EC_OK;
			}

			// little hack to get rid of any potential error from stToInt--we want to handle the error here

			i->instance->type->engine->error.type = 0;
			stEvalError(i->instance->type->engine, EE_CONVERT, "cannot convert type \"%s\" to type \"object\"", slAtomicTypeStrings[e->type]);
			return EC_ERROR;
			break;
		case AT_LIST:
			stEvalError(i->instance->type->engine, EE_CONVERT, "cannot convert type \"%s\" to type \"list\"", slAtomicTypeStrings[e->type]);
			return EC_ERROR;
			break;
		case AT_MATRIX:
			stEvalError(i->instance->type->engine, EE_CONVERT, "cannot convert type \"%s\" to type \"matrix\"", slAtomicTypeStrings[e->type]);
			return EC_ERROR;
			break;
		case AT_HASH:
			stEvalError(i->instance->type->engine, EE_CONVERT, "cannot convert type \"%s\" to type \"hash\"", slAtomicTypeStrings[e->type]);
			return EC_ERROR;
			break;
		case AT_POINTER:
			if(t->type == AT_INT && BRINT(t) == 0) {
				t->type = AT_INSTANCE;
				return EC_OK;
			}

			stEvalError(i->instance->type->engine, EE_CONVERT, "cannot convert type \"%s\" to type \"pointer\"", slAtomicTypeStrings[e->type]);
			return EC_ERROR;
			break;
		case AT_DATA:
			stEvalError(i->instance->type->engine, EE_CONVERT, "cannot convert type \"%s\" to type \"data\"", slAtomicTypeStrings[e->type]);
			return EC_ERROR;
			break;
		default:
			stEvalError(i->instance->type->engine, EE_INTERNAL, "INTERNAL ERROR: unknown type in stToType", e->type);
			return EC_ERROR;
			break;
	}

	return EC_OK;
}

int stLoadVariable(void *variable, unsigned char type, brEval *e, stRunInstance *i) {
	e->type = type;

	// figure out which type we're dealing with so we interpret the pointer correctly 

	switch(e->type) {
		case AT_INT:
			BRINT(e) = *(int*)variable;
			break;
		case AT_DOUBLE:
			BRDOUBLE(e) = *(double*)variable;
			break;
		case AT_POINTER:
		case AT_INSTANCE:
		case AT_DATA:
			BRPOINTER(e) = *(void**)variable;
			break;
		case AT_VECTOR:
			slVectorCopy((slVector*)variable, &BRVECTOR(e));
			break;
		case AT_LIST:
			// if there is nothing here (uninitialized list), then we create an empty list.  
			// we retain it, since it's stored as a variable.

			if(!*(brEvalListHead**)variable) {
				*(brEvalListHead**)variable = brEvalListNew();
				stGCRetainPointer(*(void**)variable, AT_LIST);
			}

			BRLIST(e) = *(brEvalListHead**)variable;
			
			break;
		case AT_HASH:
			if(!*(brEvalHash**)variable) {
				// if there is nothing here (uninitialized hash), then we create an empty hash.  
				// we retain it, since it's stored as a variable.

				*(brEvalHash**)variable = brEvalHashNew();
				stGCRetainPointer(*(void**)variable, AT_LIST);
			}

			BRHASH(e) = *(brEvalHash**)variable;
			
			break;
		case AT_STRING:
			if(!(*(char**)variable)) *(char**)variable = slStrdup("");

			BRSTRING(e) = *(char**)variable;

			break;
		case AT_MATRIX:
			slMatrixCopy((double**)variable, BRMATRIX(e));
			break;
		case AT_ARRAY:
			stEvalError(i->instance->type->engine, EE_ARRAY, "Array variables cannot be loaded like normal expressions");
			return EC_ERROR;
			break;
		default:
			slMessage(DEBUG_ALL, "Unknown atomic type in load: %d\n", e->type);
			return EC_ERROR;
			break;
	}

	return EC_OK;
}

/*!
	\brief Sets a variable in steve.
*/

int stSetVariable(void *variable, unsigned char type, stObject *otype, brEval *e, stRunInstance *i) {
	int result;
	int noRetain = 0;
	char *newstr;

	if(type == AT_INSTANCE && otype) {
		// if they specified an object type in the code, make sure 
		// that this is a valid cast.  this requires a lookup each
		// time we're here, so it could really be improved.

		if(!otype) {
			stEvalError(i->instance->type->engine, EE_TYPE, "Cannot locate class named \"%s\" in assignment expression\n", otype->name);
			return EC_ERROR;
		}
	 
		if(!stIsSubclassOf(STINSTANCE(e)->type, otype)) {
			stEvalError(i->instance->type->engine, EE_TYPE, "Cannot assign instance of class \"%s\" to variable of class \"%s\"\n", STINSTANCE(e)->type->name, otype->name);
			return EC_ERROR;
		}
	}

	if((result = stToType(e, type, e, i)) != EC_OK) return result;

#ifdef MULTITHREAD
	if(i) pthread_mutex_lock(&i->lock);
#endif

	switch(type) {
		case AT_INT:
			*(int*)variable = BRINT(e);
			break;
		case AT_DOUBLE:
			*(double*)variable = BRDOUBLE(e);
			break;
		case AT_VECTOR:
			slVectorCopy(&BRVECTOR(e), (slVector*)variable);
			break;
		case AT_MATRIX:
			slMatrixCopy(BRMATRIX(e), (double**)variable);
			break;
		case AT_INSTANCE:
			*(stInstance**)variable = STINSTANCE(e);
			break;
		case AT_DATA:
			// overwriting an old data variable

			if(*(brData**)variable != BRDATA(e)) stGCUnretainAndCollectPointer(*(void**)variable, AT_DATA);

			// retain and set the new data 

			if(BRDATA(e)) *(brData**)variable = BRDATA(e);

			break;
		case AT_POINTER:
			*(void**)variable = BRPOINTER(e);
			break;
		case AT_STRING:
			newstr = slStrdup(BRSTRING(e));

			// are we overwriting an old string in memory?
			if(*(void**)variable) slFree(*(void**)variable);	

			*(char**)variable = newstr;

			noRetain = 1;

			break;
		case AT_LIST:
			if(!BRLIST(e)) {
				// if this is an empty list, just allocate some space,
				// and mark it.  it will be retained later.

				e->type = AT_LIST;
				BRLIST(e) = brEvalListNew();
			} else if(*(brEvalListHead**)variable != BRLIST(e)) {
				// overwriting an old list at this location

				stGCUnretainAndCollectPointer(*(void**)variable, AT_LIST);
			} else {
				noRetain = 1;
			}

			if(e->type != AT_LIST) {
				slMessage(DEBUG_ALL, "Cannot convert \"%s\" to type \"list\" for assignment\n", slAtomicTypeStrings[e->type]);
#ifdef MULTITHREAD
				if(i) pthread_mutex_unlock(&i->lock);
#endif
				return EC_ERROR;
			}

			*(brEvalListHead**)variable = BRLIST(e);

			break;
		case AT_HASH:
			// if this is an empty hash, just allocate some space...

			if(!BRHASH(e)) {
				e->type = AT_HASH;
				BRHASH(e) = brEvalHashNew();
			} else if(*(brEvalHash**)variable != BRHASH(e)) {
				// overwriting an old hash at this location
				stGCUnretainAndCollectPointer(*(void**)variable, AT_HASH);
			} else {
				noRetain = 1;
			}

			if(e->type != AT_HASH) {
				slMessage(DEBUG_ALL, "Cannot convert \"%s\" to type \"hash\" for assignment\n", slAtomicTypeStrings[e->type]);
#ifdef MULTITHREAD
				if(i) pthread_mutex_unlock(&i->lock);
#endif
				return EC_ERROR;
			}

			*(brEvalHash**)variable = BRHASH(e);

			break;
		case AT_ARRAY:
			stEvalError(i->instance->type->engine, EE_ARRAY, "Array variables cannot be assigned like simple variables");
#ifdef MULTITHREAD
			if(i) pthread_mutex_unlock(&i->lock);
#endif
			return EC_ERROR;
		default:
			slMessage(DEBUG_ALL, "INTERNAL ERROR: unknown variable type %d in set\n", type);
#ifdef MULTITHREAD
			if(i) pthread_mutex_unlock(&i->lock);
#endif
			return EC_ERROR;
			break;
	}

	if(!noRetain) {
		stGCRetain(e);
		if(i) stGCUnmark(i->instance, e);
	}

#ifdef MULTITHREAD
	if(i) pthread_mutex_unlock(&i->lock);
#endif

	return EC_OK;
}

/*!
	\brief Computes the "truth" of an expression in steve.

	Takes any kind of expression and decides whether it is "true" in
	the context of boolean expressions.  It yields an int with a  value 
	of either 1 or 0.
*/

inline int stEvalTruth(brEval *e, brEval *t, stRunInstance *i) {
	/* because e may equal t, put the type in a local variable */
	/* before changing it... */

	char *str;
	int type = e->type;

	double zero[3][3];

	t->type = AT_INT;

	switch(type) {
		case AT_INT:
			BRINT(t) = (BRINT(e) != 0);
			return EC_OK;
			break;
		case AT_DOUBLE:
			BRINT(t) = (BRDOUBLE(e) != 0.0);
			return EC_OK;
			break;
		case AT_STRING:
			str = BRSTRING(e);
			BRINT(t) = (str && strlen(str) > 0);

			return EC_OK;
			break;
		case AT_INSTANCE:
			if(STINSTANCE(e) && (STINSTANCE(e))->status != AS_ACTIVE) BRINT(t) = 0;
			else BRINT(t) = (STINSTANCE(e) != 0x0);

			return EC_OK;
			break;
		case AT_POINTER:
		case AT_HASH:
		case AT_DATA:
			BRINT(t) = (BRPOINTER(e) != 0x0);
			return EC_OK;
			break;
		case AT_LIST:
			BRINT(t) = (BRPOINTER(e) != 0 && (BRLIST(e))->count != 0);
			return EC_OK;
			break;
		case AT_VECTOR:
			BRINT(t) = (slVectorLength(&BRVECTOR(e)) != 0.0);
			return EC_OK;
			break;
		case AT_MATRIX:
			slMatrixZero(zero);
			BRINT(t) = !slMatrixCompare(BRMATRIX(e), zero);
			break;
		case AT_NULL:
			BRINT(t) = 0;
			return EC_OK;
			break;
		default:
			stEvalError(i->instance->type->engine, EE_INTERNAL, "unknown type %d in stEvalTruth", type);
			return EC_ERROR;
			break;
	}

	return EC_OK;
}

/*!
	\brief Evaluates a free expression in steve.
*/

inline int stEvalFree(stExp *s, stRunInstance *i, brEval *t) {
	int result;
	brEval target;
	brEvalList *list;
	int finished = 0;

	t->type = AT_NULL;

	result = stExpEval(s, i, &target, NULL);
	if(result != EC_OK) return EC_ERROR;

	if(target.type == AT_INSTANCE) {
		if(!STINSTANCE(&target)) {
			slMessage(DEBUG_ALL, "warning: attempt to free uninitialized object\n");
			return EC_OK;
		}

		if(STINSTANCE(&target)->status == AS_ACTIVE) stInstanceFree(STINSTANCE(&target));
		else {
			slMessage(DEBUG_ALL, "warning: attempting to free released instance %p\n", STINSTANCE(&target));
		}

		// if we're freeing ourself (the calling instance) then we will return EC_STOP

		if(STINSTANCE(&target) == i->instance) finished = 1;
	} else if(target.type == AT_LIST) {
		list = (BRLIST(&target))->start;

		while(list) {
			if(list->eval.type == AT_INSTANCE) {
				if(!STINSTANCE(&list->eval)) slMessage(DEBUG_ALL, "warning: attempt to free uninitialized object\n");
				else {
					if(STINSTANCE(&list->eval)->status == AS_ACTIVE) stInstanceFree(STINSTANCE(&list->eval));
					else {
						slMessage(DEBUG_ALL, "warning: attempting to free released instance %p\n", STINSTANCE(&list->eval));
						slMessage(DEBUG_ALL, "... error in file \"%s\" at line %d\n", s->file, s->line);
					}

					// if we're freeing ourself (the calling instance) then we will return EC_STOP

					if(STINSTANCE(&list->eval) == i->instance) finished = 1;
				}
			}

			list = list->next;
		}
	}

	if(finished) return EC_STOP;

	return EC_OK;
}

/*!
	\brief Evaluates an array of expressions in steve.
*/

inline int stEvalArray(slArray *a, stRunInstance *i, brEval *target) {
	int n, count;
	stExp *e;

	if(i->instance->status != AS_ACTIVE) return EC_OK;

	if(!i || !target) return EC_ERROR;

	if(!a) return EC_OK;

	for(count=0;count<a->count;count++) {
		if(a->data[count]) {
			n = stExpEval(a->data[count], i, target, NULL);

			/* free the string or evalList, unless it's being returned */

			if(n == EC_ERROR) {
				e = a->data[count];
				i->instance->type->engine->error.line = e->line;

				if(i->instance->type->engine->error.file) slFree(i->instance->type->engine->error.file);

				i->instance->type->engine->error.file = slStrdup(e->file);
				slMessage(DEBUG_ALL, "... error in file \"%s\" at line %d\n", e->file, e->line);
				return EC_ERROR_HANDLED;
			} else if(n == EC_ERROR_HANDLED) return EC_ERROR_HANDLED;

			else if(n == EC_STOP) return EC_STOP;
		}
	}

	/* if we're here, we can't be returning a value.  set the	  */
	/* target type to AT_NULL so nothing gets incorrectly released */

	target->type = AT_NULL;

	return EC_OK;
}

/*!
	\brief Evaluates a hardcoded list expression in steve.

	list is a list of stExp expressions, each of which is evaluated
	and added to the list.
*/

inline int brEvalListExp(slList *list, stRunInstance *i, brEval *target) {
	brEval index;
	int result;

	target->type = AT_LIST;
	BRLIST(target) = brEvalListNew();

	while(list) {
		result = stExpEval(list->data, i, &index, NULL);

		if(result != EC_OK) return result;

		brEvalListInsert(BRLIST(target), BRLIST(target)->count, &index);
		stGCUnmark(i->instance, &index);
		stGCRetain(&index);

		list = list->next;
	}

	stGCMark(i->instance, target);

	return EC_OK;
}

/*!
	\brief Initiates evaluation of a method call expression in steve.

	This function doesn't do the actual method calling--it evaluates the
	calling object or object-list, and then calls \ref stRealEvalMethodCall
	for each object.
*/

inline int stEvalMethodCall(stMethodExp *mexp, stRunInstance *i, brEval *t) {
	brEval obj;
	int r;
	stRunInstance ri;

	r = stExpEval(mexp->objectExp, i, &obj, &ri.type);

	if(r != EC_OK) return r;

	if(obj.type == AT_INSTANCE) {
		ri.instance = STINSTANCE(&obj);

		if(!ri.instance) {
			stEvalError(i->type->engine, EE_NULL_INSTANCE, "method \"%s\" called with uninitialized object", mexp->methodName);
			return EC_ERROR;
		}

		if(!ri.type) ri.type = ri.instance->type;

		r = stRealEvalMethodCall(mexp, &ri, i, t);

		return r;
	}

	if(obj.type == AT_LIST) {
		brEvalList *listStart = BRLIST(&obj)->start;

		while(listStart) {
			ri.instance = STINSTANCE(&listStart->eval);
			ri.type = ri.instance->type;

			r = stRealEvalMethodCall(mexp, &ri, i, t);
			listStart = listStart->next;

			if(r != EC_OK) return r;
		}

		return EC_OK;
	}

	stEvalError(i->instance->type->engine, EE_TYPE, "Method \"%s\" called for an expression that is neither an object nor a list", mexp->methodName);

	return EC_ERROR;
}

/*!
	\brief Evaluates a method call expression in steve.
*/

inline int stRealEvalMethodCall(stMethodExp *mexp, stRunInstance *caller, stRunInstance *i, brEval *t) {
	int result;
	int argCount, keyCount;
	int n, m;
	brEval *args, **argps;
	stKeyword *key, *tmpkey;
	stKeywordEntry *keyEntry;

	if(caller->instance == NULL) {
		stEvalError(i->instance->type->engine, EE_NULL_INSTANCE, "method \"%s\" called with uninitialized object", mexp->methodName);
		return EC_ERROR;
	} 

	if(caller->instance->status != AS_ACTIVE) {
		stEvalError(i->instance->type->engine, EE_FREED_INSTANCE, "method \"%s\" called with freed object (%p)", mexp->methodName, caller->instance);
		return EC_ERROR;
	}

	// when we look up a method, we can remember its address for next time
	// if the same object type is being used.  otherwise, we need to look 
	// it up again.  there is a fair amount of computation here looking up
	// the method and argument order, so caching it is good.

	if(mexp->objectCache != caller->type) {
		stMethod *method;
		stObject *newType;

		method = stFindInstanceMethodWithMinArgs(caller->type, mexp->methodName, mexp->arguments->count, &newType);

		caller->type = newType;

		if(!method) {
			// can't find the method!

			char *kstring = "keywords";

			if(mexp->arguments && mexp->arguments->count == 1) kstring = "keyword";

			caller->type = caller->instance->type;
			stEvalError(caller->type->engine, EE_UNKNOWN_METHOD, "object type \"%s\" does not respond to method \"%s\" with %d %s", caller->type->name, mexp->methodName, mexp->arguments->count, kstring);
			mexp->objectCache = NULL;
			return EC_ERROR;
		}

		mexp->method = method;
		mexp->objectCache = caller->instance->type;
		mexp->objectTypeCache = caller->type;

		if(mexp->method->keywords) keyCount = mexp->method->keywords->count;
		else keyCount = 0;

		if(mexp->arguments) argCount = mexp->arguments->count;
		else argCount = 0;

		// we have to lookup the order of the keywords.  initialize them to -1 first.

		for(n=0;n<argCount;n++) ((stKeyword*)mexp->arguments->data[n])->position = -1;

		slStackClear(mexp->positionedArguments);

		for(n=0;n<keyCount;n++) {
			// look for the method's Nth keyword

			key = NULL;
			keyEntry = mexp->method->keywords->data[n];

			for(m=0;m<argCount;m++) {
				// go through all the arguments, checking what was passed in.

				tmpkey = mexp->arguments->data[m];
				if(tmpkey->position == -1 && !strcmp(tmpkey->word, keyEntry->keyword)) {
					tmpkey->position = n;
					m = keyCount;
					key = tmpkey;
				}
			}

			if(!key && keyEntry->defaultKey)  key = keyEntry->defaultKey;

			if(!key) {
				stEvalError(caller->type->engine, EE_MISSING_KEYWORD, "Call to method %s of class %s missing keyword \"%s\"", mexp->method->name, caller->type->name, keyEntry->keyword);
				mexp->objectCache = NULL;
				return EC_ERROR;
			}

			slStackPush(mexp->positionedArguments, key);
		}

		for(n=0;n<argCount;n++) {
			if(((stKeyword*)mexp->arguments->data[n])->position == -1) {
				tmpkey = mexp->arguments->data[n];

				stEvalError(caller->type->engine, EE_UNKNOWN_KEYWORD, "unknown keyword \"%s\" in call to method \"%s\"", tmpkey->word, mexp->method->name);
				mexp->objectCache = NULL;
				return EC_ERROR;
			}
		}
	} else {
		if(mexp->method->keywords) keyCount = mexp->method->keywords->count;
		else keyCount = 0;

		caller->type = mexp->objectTypeCache;
	}

	if(mexp->method->inlined) {
		// The method is inlined if it has no local variables, and no arguments

		int result;
		slStack *oldStack = caller->instance->gcStack;
		caller->instance->gcStack = slStackNew();

		result = stEvalArray(mexp->method->code, caller, t);

		// unmark the return value -- we'll make it the current instance's problem

		if(t->type != AT_NULL) stGCUnmark(caller->instance, t);

		// collect and reset the gcStack

		stGCCollectStack(caller->instance->gcStack);
		slStackFree(caller->instance->gcStack);
		caller->instance->gcStack = oldStack;

		// mark the return value for the caller's GC

		if(i->instance->gcStack) stGCMark(i->instance, t);

		if(result == EC_STOP) return EC_OK;
	
		return result;
	}

	// we don't want to reuse the same argps in the case of a recursive function 
	// so we create some local storage for them.

	if(keyCount) {
		args = alloca(sizeof(brEval) * keyCount);
		argps = alloca(sizeof(brEval*) * keyCount);

		for(n=0;n<keyCount;n++) argps[n] = &args[n];
	} else {
		args = NULL;
		argps = NULL;
	}

	for(n=0;n<keyCount;n++) {
		key = mexp->positionedArguments->data[n];

		if(!key) {
			slMessage(DEBUG_ALL, "Missing keyword for method \"%s\"\n", mexp->method->name);
			return EC_ERROR;
		}

		// evaluate the key into the eval...

		result = stExpEval(key->value, i, argps[n], NULL);

		if(result != EC_OK) {
			slMessage(DEBUG_ALL, "Error evaluating keyword \"%s\" for method \"%s\"\n", key->word, mexp->method->name);
			return EC_ERROR;
		}
	}

	if(!mexp->method) return EC_OK;

	result = stCallMethod(i, caller, mexp->method, argps, keyCount, t);

	return result;
}

/*!
	\brief Evaluates a while expression in steve.
*/

inline int stEvalWhile(stWhileExp *w, stRunInstance *i, brEval *target) {
	brEval conditionExp, condition;
	int evaluations = 0;
	int result;

	target->type = AT_NULL;

	result = stExpEval(w->cond, i, &conditionExp, NULL);
	if(result != EC_OK) return result;

	result = stEvalTruth(&conditionExp, &condition, i);
	if(result != EC_OK) return result;

	while(BRINT(&condition)) {
		evaluations++;

		result = stExpEval(w->code, i, target, NULL); 
		if(result != EC_OK) return result;

		result = stExpEval(w->cond, i, &conditionExp, NULL);
		if(result != EC_OK) {
			return result;
		}

		result = stEvalTruth(&conditionExp, &condition, i);
		if(result != EC_OK) return result;
	} 

	target->type = AT_NULL;

	return EC_OK;
}

inline int stEvalFor(stForExp *w, stRunInstance *i, brEval *target) {
	brEval condition, conditionExp, assignment, iteration;
	int evaluations = 0;
	int result;

	target->type = AT_NULL;

	result = stExpEval(w->assignment, i, &assignment, NULL);
	if(result != EC_OK) {
		return result;
	}

	result = stExpEval(w->condition, i, &conditionExp, NULL);
	if(result != EC_OK) {
		return result;
	}

	result = stEvalTruth(&conditionExp, &condition, i);

	if(result != EC_OK) return result;

	while(BRINT(&condition)) {
		evaluations++;

		result = stExpEval(w->code, i, target, NULL); 
		if(result != EC_OK) return result;

		result = stExpEval(w->iteration, i, &iteration, NULL);
		if(result != EC_OK) return result;

		result = stExpEval(w->condition, i, &conditionExp, NULL);
		if(result != EC_OK) {
			return result;
		}

		result = stEvalTruth(&conditionExp, &condition, i);

		if(result != EC_OK) return result;
	} 

	target->type = AT_NULL;

	return EC_OK;
}

inline int stEvalForeach(stForeachExp *w, stRunInstance *i, brEval *target) {
	brEval list;
	int evaluations = 0;
	brEvalList *el;
	void *iterationPointer;
	int result;

	stAssignExp *assignExp = w->assignment;

	target->type = AT_NULL;

	if(assignExp->local) iterationPointer = &i->instance->type->engine->stack[assignExp->offset];
	else iterationPointer = &i->instance->variables[assignExp->offset];

	stExpEval(w->list, i, &list, NULL);

	if(list.type != AT_LIST) {
		stEvalError(i->instance->type->engine, EE_TYPE, "expected type \"list\" in foreach evaluation");
		return EC_ERROR;
	}

	el = BRLIST(&list)->start;

	while(el) {
		brEval eval;

		if((result = brEvalCopy(&el->eval, &eval)) != EC_OK) return result;

		// this does a retain

        if(assignExp->objectName && !assignExp->objectType)
			assignExp->objectType = stObjectFind(i->instance->type->engine->objects, assignExp->objectName);

		if((result = stSetVariable(iterationPointer, assignExp->type, assignExp->objectType, &eval, i)) != EC_OK) {
			return result;
		}

		result = stExpEval(w->code, i, target, NULL); 

		if(result != EC_OK) {
			return result;
		}

		evaluations++;
		el = el->next;
	} 

	target->type = AT_NULL;

	return EC_OK;
}

inline int stEvalIf(stIfExp *w, stRunInstance *i, brEval *target) {
	brEval condition, conditionExp;
	int result, v;

	result = stExpEval(w->cond, i, &conditionExp, NULL);
	if(result != EC_OK) return result;

	result = stEvalTruth(&conditionExp, &condition, i);
	if(result != EC_OK) return result;

	v = BRINT(&condition);

	if(v) {
		if(!w->trueCode) return EC_OK;
		return stExpEval(w->trueCode, i, target, NULL); 
	} else {
		if(!w->falseCode) return EC_OK;
		return stExpEval(w->falseCode, i, target, NULL);
	}
}

inline int stEvalListInsert(stListInsertExp *w, stRunInstance *i, brEval *target) {
	brEval pushEval, index;
	int result;

	result = stExpEval(w->listExp, i, target, NULL);
	if(result != EC_OK) return result;

	result = stExpEval(w->exp, i, &pushEval, NULL);
	if(result != EC_OK) return result;

	if(target->type != AT_LIST) {
		stEvalError(i->instance->type->engine, EE_TYPE, "expected type \"list\" in \"push\" evaluation");
		return EC_ERROR;
	}

	if(BRLIST(target) == NULL) {
		stEvalError(i->instance->type->engine, EE_INTERNAL, "attempt to insert value into uninitialized list\n");
		return EC_ERROR;
	}

	if(w->index) {
		result = stExpEval(w->index, i, &index, NULL);
		if(result != EC_OK) return result;
	} else {
		index.type = AT_INT;
		BRINT(&index) = BRLIST(target)->count;
	}

	brEvalListInsert(BRPOINTER(target), BRINT(&index), &pushEval);

	stGCRetain(&pushEval);

	return EC_OK;
}

inline int stEvalListRemove(stListRemoveExp *l, stRunInstance *i, brEval *target) {
	brEval listEval, index;
	int result;

	result = stExpEval(l->listExp, i, &listEval, NULL);
	if(result != EC_OK) return result;

	if(listEval.type != AT_LIST) {
		stEvalError(i->instance->type->engine, EE_TYPE, "expected type \"list\" during \"pop\" evaluation"); 
		return EC_ERROR;
	}

	if(!BRLIST(&listEval)) {
		target->type = AT_NULL;
		return EC_OK;
	}

	if(l->index) {
		result = stExpEval(l->index, i, &index, NULL);
		if(result != EC_OK) return result;
	} else {
		index.type = AT_INT;
		BRINT(&index) = BRLIST(&listEval)->count - 1;
	}

	brEvalListRemove(BRLIST(&listEval), BRINT(&index), target);

	// this is unretained when we pop it from the list, but we cannot collect 
	// it immediately, since we're returning it.

	stGCUnretain(target);
	stGCMark(i->instance, target);

	return EC_OK;
}

inline int stEvalCopyList(stExp *l, stRunInstance *i, brEval *target) {
	brEval listEval;
	int result;

	result = stExpEval(l, i, &listEval, NULL);
	if(result != EC_OK) return result;

	if(listEval.type != AT_LIST) {
		stEvalError(i->instance->type->engine, EE_TYPE, "expected type \"list\" during \"copylist\" evaluation"); 
		return EC_ERROR;
	}

	target->type = AT_LIST;
	BRLIST(target) = brEvalListDeepCopy(BRLIST(&listEval));

	return EC_OK;
}

inline int stEvalAll(stAllExp *e, stRunInstance *i, brEval *target) {
	slList *l;
	brEval instance;

	if(!e->object) {
		e->object = stObjectFind(i->instance->type->engine->objects, e->name);

		if(!e->object) {
			stEvalError(i->instance->type->engine, EE_UNKNOWN_OBJECT, "unknown object type \"%s\" in \"all\" expression\n", e->name);
			return EC_ERROR;
		}
	}

	l = e->object->allInstances;

	BRLIST(target) = brEvalListNew();
	target->type = AT_LIST;

	stGCMark(i->instance, target);

	instance.type = AT_INSTANCE;

	while(l) {
		STINSTANCE(&instance) = l->data;
		brEvalListInsert(BRLIST(target), 0, &instance);
		l = l->next;
	}

	return EC_OK;
}

inline int stEvalSort(stSortExp *w, stRunInstance *i, brEval *target) {
	brEval listEval;
	stMethod *method;
	int result;

	result = stExpEval(w->listExp, i, &listEval, NULL);
	if(result != EC_OK) return result;

	method = stFindInstanceMethod(i->instance->type, w->methodName, 2, NULL);
	
	if(!method) {
		stEvalError(i->instance->type->engine, EE_UNKNOWN_METHOD, "object type \"%s\" does not respond to method \"%s\"", i->instance->type->name, w->methodName); 
		return EC_ERROR;
	}

	result = stSortEvalList(BRLIST(&listEval), i->instance, method); 

	return result;
}

inline int stEvalListIndexPointer(stListIndexExp *l, stRunInstance *i, void **pointer, int *type) {
	brEval list, index, *target;
	int result;

	result = stExpEval(l->listExp, i, &list, NULL);
	if(result != EC_OK) return result;

	result = stExpEval(l->indexExp, i, &index, NULL);
	if(result != EC_OK) return result;

	if(list.type == AT_LIST) {
		if(index.type != AT_INT && stToInt(&index, &index, i) == EC_ERROR) {
			stEvalError(i->instance->type->engine, EE_TYPE, "expected type \"int\" in list element evaluation (index component)");
			return EC_ERROR;
		}

		if(stDoEvalListIndexPointer(BRPOINTER(&list), BRINT(&index), &target)) {
			stEvalError(i->instance->type->engine, EE_BOUNDS, "list index \"%d\" out of bounds in list element evaluation", BRINT(&index));
			return EC_ERROR;
		}

		*pointer = &target->values;
		*type = target->type;
	} else if(list.type == AT_HASH) {
		return EC_ERROR;
	} else {
		return EC_ERROR;
	}

	return EC_OK;
}

inline int stEvalListIndex(stListIndexExp *l, stRunInstance *i, brEval *t) {
	brEval list, index;
	int result;

	result = stExpEval(l->listExp, i, &list, NULL);
	if(result != EC_OK) return result;

	result = stExpEval(l->indexExp, i, &index, NULL);
	if(result != EC_OK) return result;

	if(list.type == AT_LIST) {
		if(index.type != AT_INT && stToInt(&index, &index, i) == EC_ERROR) {
			stEvalError(i->instance->type->engine, EE_TYPE, "expected type \"int\" in list element evaluation (index component)");
			return EC_ERROR;
		}

		if(stDoEvalListIndex(BRPOINTER(&list), BRINT(&index), t)) {
			stEvalError(i->instance->type->engine, EE_BOUNDS, "list index \"%d\" out of bounds in list element evaluation", BRINT(&index));
			return EC_ERROR;
		}
	} else if(list.type == AT_HASH) {
		brEvalHashLookup(BRHASH(&list), &index, t);
	} else {
		stEvalError(i->instance->type->engine, EE_TYPE, "expected list or hash in lookup expression");
		return EC_ERROR;
	}

	stGCMark(i->instance, t);

	return EC_OK;
}

inline int stEvalListIndexAssign(stListIndexAssignExp *l, stRunInstance *i, brEval *t) {
	brEval list, index;
	int result;

	result = stExpEval(l->listExp, i, &list, NULL);
	if(result != EC_OK) return result;

	result = stExpEval(l->indexExp, i, &index, NULL);
	if(result != EC_OK) return result;

	result = stExpEval(l->assignment, i, t, NULL);
	if(result != EC_OK) return result;

	if(list.type == AT_LIST) {
		if(index.type != AT_INT && stToInt(&index, &index, i) == EC_ERROR) {
			stEvalError(i->instance->type->engine, EE_TYPE, "expected type \"int\" in list element assignment evaluation (index component)");
			return EC_ERROR;
		}

		if(stDoEvalListIndexAssign(BRLIST(&list), BRINT(&index), t, i)) {
			stEvalError(i->instance->type->engine, EE_BOUNDS, "list index \"%d\" out of bounds in list element assignment evaluation", BRINT(&index));
			return EC_ERROR;
		}
	} else if(list.type == AT_HASH) {
		brEval old;

		brEvalHashStore(BRHASH(&list), &index, t, &old);

		stGCUnretainAndCollect(&old);
	} else {
		stEvalError(i->instance->type->engine, EE_TYPE, "expected type \"list\" in list element assignment evaluation (list component)");
		return EC_ERROR;
	}

	// since the variable now has a home, we have to unmark it.

	stGCUnmark(i->instance, t);
	stGCRetain(t);

	return EC_OK;
}

inline int stEvalPrint(stPrintExp *exp, stRunInstance *i, brEval *t) {
	brEval arg;
	int n, result;
	slArray *a = exp->expressions;

	t->type = AT_NULL;

	for(n=0;n<a->count;n++) {
		result = stExpEval((stExp*)a->data[n], i, &arg, NULL);
		if(result != EC_OK) return result;

		result = stPrintEvaluation(&arg, i);

		if(result != EC_OK) return result;

		if(n != a->count - 1) slMessage(NORMAL_OUTPUT, " ");

	}

	if(exp->newline) slMessage(NORMAL_OUTPUT, "\n");

	return EC_OK;
}

inline int stEvalVectorElementExp(stVectorElementExp *s, stRunInstance *i, brEval *target) {
	int result;

	result = stExpEval(s->exp, i, target, NULL);
	if(result != EC_OK) return EC_ERROR;

	if(target->type != AT_VECTOR) {
		stEvalError(i->instance->type->engine, EE_TYPE, "expected type \"vector\" in vector element evaluation");
		return EC_ERROR;
	}

	target->type = AT_DOUBLE;

	switch(s->element) {
		case VE_X:
			BRDOUBLE(target) = BRVECTOR(target).x;
			return EC_OK;
			break;
		case VE_Y:
			BRDOUBLE(target) = BRVECTOR(target).y;
			return EC_OK;
			break;
		case VE_Z:
			BRDOUBLE(target) = BRVECTOR(target).z;
			return EC_OK;
			break;
		default:
			stEvalError(i->instance->type->engine, EE_INTERNAL, "unknown vector element (%d) in stEvalVectorElementExp", s->element);
			return EC_ERROR;
			break;
	}

	return EC_OK;
}

inline int stEvalVectorElementAssignExp(stVectorElementAssignExp *s, stRunInstance *i, brEval *target) {
	int result;
	slVector *vector;
	int type;

	result = stPointerForExp(s->exp, i, (void*)&vector, &type);

	if(result != EC_OK) return EC_ERROR;

	if(type != AT_VECTOR) {
		stEvalError(i->instance->type->engine, EE_TYPE, "expected vector expression in vector element assign");
		return EC_ERROR;
	}

	result = stExpEval(s->assignExp, i, target, NULL);
	if(result != EC_OK) return EC_ERROR;
	result = stToDouble(target, target, i); 
	if(result != EC_OK) return EC_ERROR;

	switch(s->element) {
		case VE_X:
			vector->x = BRDOUBLE(target);
			break;
		case VE_Y:
			vector->y = BRDOUBLE(target);
			break;
		case VE_Z:
			vector->z = BRDOUBLE(target);
			break;
		default:
			stEvalError(i->instance->type->engine, EE_INTERNAL, "unknown vector element (%d) in stEvalVectorElementAssignExp", s->element);
			break;
	}

	return EC_OK;
}

/*!
	\brief Calls a C-style internal function in steve.
*/

inline int stEvalCallFunc(stCCallExp *c, stRunInstance *i, brEval *target) {
	brEval e[ST_CMAX_ARGS];
	int n, result;

	for(n=0;n<c->function->nargs;n++) {
		result = stExpEval(c->args->data[n], i, &e[n], NULL);

		if(result != EC_OK) return result;

		/* if the types don't match, try to convert them */

		if(e[n].type != c->function->argtypes[n]) {
			result = stToType(&e[n], c->function->argtypes[n], &e[n], i);

		}

		if(c->function->argtypes[n] == AT_INSTANCE && STINSTANCE(&e[n]))
			BRINSTANCE(&e[n]) = STINSTANCE(&e[n])->breveInstance;

		if(result != EC_OK) {
			stEvalError(i->instance->type->engine, EE_TYPE, "expected type \"%s\" for argument #%d to internal method \"%s\", got type \"%s\"", slAtomicTypeStrings[c->function->argtypes[n]], n + 1, c->function->name, slAtomicTypeStrings[e[n].type]);
			return result;
		}
	}

#ifdef MULTITHREAD
	pthread_mutex_lock(&(i->instance->lock));
#endif
	result = c->function->call(e, target, i->instance->breveInstance);
#ifdef MULTITHREAD
	pthread_mutex_unlock(&(i->instance->lock));
#endif

	// special case--if the define type is undefined, any type may be returned. */

	if(c->function->rtype != AT_UNDEFINED) target->type = c->function->rtype;

	stConvertBreveInstanceToSteveInstance(target);

	stGCMark(i->instance, target);

	return result;
}

/*!
	\brief Gives the pointer for an array-load expression in steve.
*/

inline int stEvalArrayIndexPointer(stArrayIndexExp *a, stRunInstance *i, void **pointer, int *type) {
	brEval indexExp;
	int r;
	int offset;

	r = stExpEval(a->index, i, &indexExp, NULL);

	*type = a->type;

	if(r != EC_OK) {
		slMessage(DEBUG_ALL, "Error evaluating index of array expression.\n");
		return r;
	}

	if(indexExp.type != AT_INT && !(r = stToInt(&indexExp, &indexExp, i))) {
		slMessage(DEBUG_ALL, "Array index must be of type \"int\"\n");
		return r;
	}

	offset = BRINT(&indexExp);

	if(offset >= a->maxIndex || offset < 0) {
		stEvalError(i->instance->type->engine, EE_BOUNDS, "array index \"%d\" out of bounds", BRINT(&indexExp));
		return EC_ERROR;
	}

	if(a->local) *pointer = &i->instance->type->engine->stack[a->offset];
	else *pointer = &i->instance->variables[a->offset];

	*pointer += a->typeSize * offset;

	return EC_OK;
}

inline int stEvalArrayIndex(stArrayIndexExp *a, stRunInstance *i, brEval *target) {
	void *pointer;
	int r, type;

	r = stEvalArrayIndexPointer(a, i, &pointer, &type);

	if(r != EC_OK) return r;

	return stLoadVariable(pointer, a->type, target, i);
}

inline int stEvalArrayIndexAssign(stArrayIndexAssignExp *a, stRunInstance *i, brEval *rvalue) {
	brEval indexExp;
	char *pointer;
	int r;

	r = stExpEval(a->index, i, &indexExp, NULL);
	if(r != EC_OK) {
		slMessage(DEBUG_ALL, "Error evaluating index of array expression.\n");
		return r;
	}

	r = stExpEval(a->rvalue, i, rvalue, NULL);
	if(r != EC_OK) {
		slMessage(DEBUG_ALL, "Error evaluating right hand side of assignment expression.\n");
		return r;
	}

	if(indexExp.type != AT_INT && !(r = stToInt(&indexExp, &indexExp, i))) {
		slMessage(DEBUG_ALL, "Array index must be of type \"int\".\n");
		return r;
	}

	if(BRINT(&indexExp) >= a->maxIndex || BRINT(&indexExp) < 0) {
		stEvalError(i->instance->type->engine, EE_BOUNDS, "array index \"%d\" out of bounds", BRINT(&indexExp));
		return EC_ERROR;
	}

	if(a->local) pointer = &i->instance->type->engine->stack[a->offset];
	else pointer = &i->instance->variables[a->offset];

	pointer += a->typeSize * BRINT(&indexExp);

	return stSetVariable(pointer, a->type, NULL, rvalue, i);
}

inline int stEvalAssignment(stAssignExp *a, stRunInstance *i, brEval *t) {
	char *pointer;
	int result;

	result = stExpEval(a->rvalue, i, t, NULL);

	if(result != EC_OK) {
		slMessage(DEBUG_ALL, "Error evaluating right hand side of assignment expression.\n");
		return result;
	}

	if(a->local) pointer = &i->instance->type->engine->stack[a->offset];
	else pointer = &i->instance->variables[a->offset];

	if(a->objectName && !a->objectType)
		a->objectType = stObjectFind(i->instance->type->engine->objects, a->objectName);

	result = stSetVariable(pointer, a->type, a->objectType, t, i);

	return result;
}


int stEvalLoad(stLoadExp *e, stRunInstance *i, brEval *target) {
	void *pointer;
	int type;

	stEvalLoadPointer(e, i, &pointer, &type);

	return stLoadVariable(pointer, e->type, target, i);
}

inline int stEvalUnaryExp(stUnaryExp *b, stRunInstance *i, brEval *target) {
	int result;
	char *str;
	brEval truth;

	/* This particular code is not very robust. */

	result = stExpEval(b->exp, i, target, NULL);

	if(result != EC_OK) return result;

	if(b->type == UT_NOT) {
		stEvalTruth(target, &truth, i);
		target->type = truth.type;
		BRINT(target) = !BRINT(&truth);

		return EC_OK;
	}

	// if it's a string, switch to a number before continuing.

	if(target->type == AT_STRING) {
		str = BRSTRING(target);
		result = stToDouble(target, target, i); 

		if(result != EC_OK) return result;
	}

	if(target->type == AT_LIST) {
		switch(b->type) {
			case UT_MINUS:
				stEvalError(i->instance->type->engine, EE_TYPE, "type \"list\" unexpected during evaluation of unary operator \"-\"");
				return EC_ERROR;
				break;
		}
	}

	if(target->type == AT_VECTOR) {
		switch(b->type) {
			case UT_MINUS:
				slVectorMul(&BRVECTOR(target), -1, &BRVECTOR(target));
				return EC_OK;
				break;
			default:
				stEvalError(i->instance->type->engine, EE_INTERNAL, "unknown unary operator (%d) in stEvalUnaryExp", b->type);
				return EC_ERROR;
				break;
		}
	}

	if(target->type == AT_INT) {
		switch(b->type) {
			case UT_MINUS:
				BRINT(target) *= -1;
				return EC_OK;
				break;
			default:
				stEvalError(i->instance->type->engine, EE_INTERNAL, "unknown unary operator (%d) in stEvalUnaryExp", b->type);
				return EC_ERROR;
				break;
		}
	}

	if(target->type == AT_DOUBLE) {
		switch(b->type) {
			case UT_MINUS:
				BRDOUBLE(target) *= -1;
				return EC_OK;
				break;
			default:
				stEvalError(i->instance->type->engine, EE_INTERNAL, "unknown unary operator (%d) in stEvalUnaryExp", b->type);
				return EC_ERROR;
				break;
		}
	}

	if(target->type == AT_INSTANCE) {
		switch(b->type) {
			case UT_MINUS:
				stEvalError(i->instance->type->engine, EE_TYPE, "type \"object\" unexpected during evaluation of unary operator \"-\"");
				return EC_ERROR;
				break;
			default:
				stEvalError(i->instance->type->engine, EE_INTERNAL, "unknown unary operator (%d) in stEvalUnaryExp", b->type);
				return EC_ERROR;
				break;
		}
	}

	if(target->type == AT_POINTER) {
		switch(b->type) {
			case UT_MINUS:
				stEvalError(i->instance->type->engine, EE_TYPE, "type \"pointer\" unexpected during evaluation of unary operator \"-\"");
				return EC_ERROR;
				break;
			default:
				stEvalError(i->instance->type->engine, EE_INTERNAL, "unknown unary operator (%d) in stEvalUnaryExp", b->type);
				return EC_ERROR;
				break;
		}
	}

	stEvalError(i->instance->type->engine, EE_INTERNAL, "unknown expression type (%d) in stEvalUnaryExp", target->type);

	return EC_ERROR;
}

inline int stEvalBinaryStringExp(char op, brEval *l, brEval *r, brEval *target, stRunInstance *i) {
	char *sl, *sr;

	sl = BRSTRING(l);
	sr = BRSTRING(r);

	target->type = AT_INT;

	switch(op) {
		case BT_EQ:
			BRINT(target) = !strcmp(sl, sr);
			break;
		case BT_NE:
			BRINT(target) = strcmp(sl, sr);
			break;
		default:
			stEvalError(i->instance->type->engine, EE_INTERNAL, "unknown string operator (%d) in stEvalBinaryStringExp", op);
			return EC_ERROR;
	}

	return EC_OK;
}

inline int stEvalBinaryMatrixExp(char op, brEval *l, brEval *r, brEval *t, stRunInstance *i) {
	double temp[3][3];

	if(l->type == AT_MATRIX && r->type == AT_MATRIX) {
		t->type = AT_MATRIX;

		switch(op) {
			case BT_MUL:
				slMatrixMulMatrix(BRMATRIX(l), BRMATRIX(r), BRMATRIX(t));
				break;
			case BT_DIV:
				slMatrixInvert(BRMATRIX(r), temp);
				slMatrixMulMatrix(BRMATRIX(l), temp, BRMATRIX(t));
				break;
			case BT_ADD:
				slMatrixAdd(BRMATRIX(l), BRMATRIX(r), BRMATRIX(t));
				break;
			case BT_SUB:
				slMatrixSub(BRMATRIX(l), BRMATRIX(r), BRMATRIX(t));
				break;
			case BT_EQ:
				t->type = AT_INT;
				BRINT(t) = slMatrixCompare(BRMATRIX(l), BRMATRIX(r));
				break;
			case BT_NE:
				t->type = AT_INT;
				BRINT(t) = !slMatrixCompare(BRMATRIX(l), BRMATRIX(r));
				break;
			default:
				stEvalError(i->instance->type->engine, EE_INTERNAL, "invalid matrix binary operator (%d)", op);
				return EC_ERROR;
		}

		return EC_OK;
	}

	/* transforming a vector... */

	if(l->type == AT_MATRIX && r->type == AT_VECTOR) {
		t->type = AT_VECTOR;

		switch(op) {
			case BT_MUL:
				slVectorXform(BRMATRIX(l), &BRVECTOR(r), &BRVECTOR(t));
				break;
			default:
				break;
				return EC_ERROR;
		}

		return EC_OK;
	}

	/* inverse transform */

	if(l->type == AT_VECTOR && r->type == AT_MATRIX) {
		t->type = AT_VECTOR;

		switch(op) {
			case BT_MUL:
				slVectorInvXform(BRMATRIX(l), &BRVECTOR(r), &BRVECTOR(t));
				break;
			default:
				break;
				return EC_ERROR;
		}

		return EC_OK;
	}

	if(l->type == AT_MATRIX) {
		t->type = AT_MATRIX;

		if(stToDouble(r, r, i) != EC_OK) {
			return EC_ERROR;
		}

		switch(op) {
			case BT_MUL:
				slMatrixMulScalar(BRMATRIX(l), BRDOUBLE(r), BRMATRIX(t));
				break;
			case BT_DIV:
				slMatrixMulScalar(BRMATRIX(l), 1.0/BRDOUBLE(r), BRMATRIX(t));
				break;
			default:
				return EC_ERROR;
				break;
		}

		return EC_OK;
	}

	if(r->type == AT_MATRIX) {
		t->type = AT_MATRIX;

		if(stToDouble(l, l, i) != EC_OK) {
			return EC_ERROR;
		}

		switch(op) {
			case BT_MUL:
				slMatrixMulScalar(BRMATRIX(r), BRDOUBLE(l), BRMATRIX(t));
				break;
			case BT_DIV:
				slMatrixInvert(BRMATRIX(r), temp);
				slMatrixMulScalar(temp, BRDOUBLE(l), BRMATRIX(t));
				break;
			default:
				return EC_ERROR;
				break;
		}

		return EC_OK;
	}

	return EC_ERROR;
}

inline int stEvalBinaryVectorExp(char op, brEval *l, brEval *r, brEval *t, stRunInstance *i) {
	brEval *temp;

	if(op == BT_MUL || op == BT_DIV) {
		if(l->type == AT_VECTOR && r->type != AT_DOUBLE && stToDouble(r, r, i) == EC_ERROR) {
			if(op == BT_MUL) stEvalError(i->instance->type->engine, EE_TYPE, "expected type \"double\" for right expression in vector multiplication evaluation");
			else stEvalError(i->instance->type->engine, EE_TYPE, "expected type \"vector\" for left expression in vector division evaluation");
			return EC_ERROR;
		}

		if(r->type == AT_VECTOR && l->type != AT_DOUBLE && stToDouble(l, l, i) == EC_ERROR) {
			if(op == BT_MUL) stEvalError(i->instance->type->engine, EE_TYPE, "expected type \"vector\" for left expression in vector multiplication evaluation");
			else stEvalError(i->instance->type->engine, EE_TYPE, "expected type \"vector\" for left expression in vector division evaluation");
			return EC_ERROR;
		}

		/* so now we know that we have the right types, but we want the	*/
		/* number to be the right argument, so switch them if necessary... */

		if(r->type == AT_VECTOR) {
			temp = r;
			r = l;
			l = temp;
		}

	} else if(l->type != AT_VECTOR || r->type != AT_VECTOR) {
		stEvalError(i->instance->type->engine, EE_TYPE, "expected type \"vector\" for vector binary operation evaluation");
		return EC_ERROR;
	}

	switch(op) {
		case BT_ADD:
			slVectorAdd(&BRVECTOR(l), &BRVECTOR(r), &BRVECTOR(t));
			return EC_OK;
			break;
		case BT_SUB:
			slVectorSub(&BRVECTOR(l), &BRVECTOR(r), &BRVECTOR(t));
			return EC_OK;
			break;
		case BT_MUL:
			slVectorMul(&BRVECTOR(l), BRDOUBLE(r), &BRVECTOR(t));
			return EC_OK;
			break;
		case BT_DIV:
			if(BRDOUBLE(r) == 0.0) {
				stEvalError(i->instance->type->engine, EE_MATH, "division by zero error in vector division");
				return EC_ERROR;
			}

			slVectorMul(&BRVECTOR(l), 1.0/BRDOUBLE(r), &BRVECTOR(t));
			return EC_OK;
			break;
		case BT_EQ:
			if(BRVECTOR(l).x == BRVECTOR(r).x && 
				BRVECTOR(l).y == BRVECTOR(r).y && 
					BRVECTOR(l).z == BRVECTOR(r).z) BRINT(t) = 1;
			else BRINT(t) = 0;

			t->type = AT_INT;

			return EC_OK;
			break;
		case BT_NE:
			if(BRVECTOR(l).x == BRVECTOR(r).x && 
				BRVECTOR(l).y == BRVECTOR(r).y && 
					BRVECTOR(l).z == BRVECTOR(r).z) BRINT(t) = 0;
			else BRINT(t) = 1;

			t->type = AT_INT;

			return EC_OK;
			break;
		default:
			stEvalError(i->instance->type->engine, EE_INTERNAL, "unknown binary expression operator (%d) in stEvalBinaryVectorExp", op);
			return EC_ERROR;
			break;
	}
}

inline int stEvalBinaryDoubleExp(char op, brEval *l, brEval *r, brEval *t, stRunInstance *i) {
	int c;

	if(l->type != AT_DOUBLE) if((c = stToDouble(l, l, i)) != EC_OK) return c;
	if(r->type != AT_DOUBLE) if((c = stToDouble(r, r, i)) != EC_OK) return c;

	switch(op) {
		case BT_ADD:
			BRDOUBLE(t) = BRDOUBLE(l) + BRDOUBLE(r);
			return EC_OK;
			break;
		case BT_SUB:
			BRDOUBLE(t) = BRDOUBLE(l) - BRDOUBLE(r);
			return EC_OK;
			break;
		case BT_MUL:
			BRDOUBLE(t) = BRDOUBLE(l) * BRDOUBLE(r);
			return EC_OK;
			break;
		case BT_DIV:
			if(BRDOUBLE(r) == 0.0) {
				stEvalError(i->instance->type->engine, EE_MATH, "Division by zero error");
				return EC_ERROR;
			}

			BRDOUBLE(t) = BRDOUBLE(l) / BRDOUBLE(r);
			return EC_OK;
			break;
		case BT_EQ:
			BRDOUBLE(t) = (BRDOUBLE(l) == BRDOUBLE(r));
			return EC_OK;
			break;
		case BT_NE:
			BRDOUBLE(t) = (BRDOUBLE(l) != BRDOUBLE(r));
			return EC_OK;
			break;
		case BT_GT:
			BRDOUBLE(t) = BRDOUBLE(l) > BRDOUBLE(r);
			return EC_OK;
			break;
		case BT_GE:
			BRDOUBLE(t) = BRDOUBLE(l) >= BRDOUBLE(r);
			return EC_OK;
			break;
		case BT_LT:
			BRDOUBLE(t) = BRDOUBLE(l) < BRDOUBLE(r);
			return EC_OK;
			break;
		case BT_LE:
			BRDOUBLE(t) = BRDOUBLE(l) <= BRDOUBLE(r);
			return EC_OK;
			break;
		case BT_MOD:
			if(BRDOUBLE(r) == 0.0) {
				stEvalError(i->instance->type->engine, EE_MATH, "Modulus by zero error");
				return EC_ERROR;
			}

			BRDOUBLE(t) = fmod(BRDOUBLE(l), BRDOUBLE(r));
			return EC_OK;
			break;
		case BT_POW:
			BRDOUBLE(t) = pow(BRDOUBLE(l), BRDOUBLE(r));
			return EC_OK;
			break;
		default:
			stEvalError(i->instance->type->engine, EE_INTERNAL, "unknown binary expression operator (%d) in stEvalBinaryDoubleExp", op);
			return EC_ERROR;
			break;
	}
}

inline int stEvalBinaryIntExp(char op, brEval *l, brEval *r, brEval *t, stRunInstance *i) {
	 switch(op) {
		case BT_ADD:
			BRINT(t) = BRINT(l) + BRINT(r);
			return EC_OK;
			break;
		case BT_SUB:
			BRINT(t) = BRINT(l) - BRINT(r);
			return EC_OK;
			break;
		case BT_MUL:
			BRINT(t) = BRINT(l) * BRINT(r);
			return EC_OK;
			break;
		case BT_DIV:
			if(BRINT(r) == 0) {
				stEvalError(i->instance->type->engine, EE_MATH, "division by zero error");
				return EC_ERROR;
			}
			BRINT(t) = BRINT(l) / BRINT(r);
			return EC_OK;
			break;
		case BT_POW:
			BRINT(t) = pow(BRINT(l), BRINT(r));
			return EC_OK;
			break;
		case BT_MOD:
			if(BRINT(r) == 0) {
				stEvalError(i->instance->type->engine, EE_MATH, "modulus by zero error");
				return EC_ERROR;
			}
			BRINT(t) = BRINT(l) % BRINT(r);
			return EC_OK;
			break;
		case BT_EQ:
			BRINT(t) = (BRINT(l) == BRINT(r));
			return EC_OK;
			break;
		case BT_NE:
			BRINT(t) = (BRINT(l) != BRINT(r));
			return EC_OK;
			break;
		case BT_GT:
			BRINT(t) = BRINT(l) > BRINT(r);
			return EC_OK;
			break;
		case BT_GE:
			BRINT(t) = BRINT(l) >= BRINT(r);
			return EC_OK;
			break;
		case BT_LT:
			BRINT(t) = BRINT(l) < BRINT(r);
			return EC_OK;
			break;
		case BT_LE:
			BRINT(t) = BRINT(l) <= BRINT(r);
			return EC_OK;
			break;
		default:
			stEvalError(i->instance->type->engine, EE_INTERNAL, "unknown binary expression operator (%d) in stEvalBinaryIntExp", op);
			return EC_ERROR;
			break;
	}
}

inline int stEvalBinaryExp(stBinaryExp *b, stRunInstance *i, brEval *target) {
	brEval tl, tr;
	brEval result;
	int c;

	if(b->type == BT_LAND) {
		target->type = AT_INT;
		BRINT(target) = 1;

		/* LAND is treated as a short circuit op, do not eval both sides */

		if((c = stExpEval(b->left, i, &tl, NULL)) != EC_OK) return c;
		if((c = stEvalTruth(&tl, &result, i)) != EC_OK) return c;

		if(!BRINT(&result)) {
			/* short circuit -- we know that the left is false, so return */

			BRINT(target) = 0;
			return EC_OK;
		}

		if((c = stExpEval(b->right, i, &tr, NULL)) != EC_OK) return c;
		if((c = stEvalTruth(&tr, &result, i)) != EC_OK) return c;

		if(!BRINT(&result)) BRINT(target) = 0;

		return EC_OK;
	}

	if(b->type == BT_LOR) {
		target->type = AT_INT;
		BRINT(target) = 0;

		/* LOR is treated as a short circuit op, do not eval both sides */

		if((c = stExpEval(b->left, i, &tl, NULL)) != EC_OK) return c;
		if((c = stEvalTruth(&tl, &result, i)) != EC_OK) return c;

		if(BRINT(&result)) {
			/* short circuit -- we know that the left is true, so return */

			BRINT(target) = 1;
			return EC_OK;
		}

		if((c = stExpEval(b->right, i, &tr, NULL)) != EC_OK) return c;
		if((c = stEvalTruth(&tr, &result, i)) != EC_OK) return c;

		if(BRINT(&result)) BRINT(target) = 1;

		return EC_OK;
	}

	if((c = stExpEval(b->left, i, &tl, NULL)) != EC_OK) return c;
	if((c = stExpEval(b->right, i, &tr, NULL)) != EC_OK) return c;

	return stEvalBinaryExpWithEvals(i, b->type, &tl, &tr, target);
}

inline int stEvalBinaryExpWithEvals(stRunInstance *i, unsigned char operator, brEval *tl, brEval *tr, brEval *target) {
	int c;

	if(tl->type == AT_INT && tr->type == AT_INT) {
		target->type = AT_INT;
 		return stEvalBinaryIntExp(operator, tl, tr, target, i);
 	}

	/* if either expression is a matrix... */

	if(tl->type == AT_MATRIX || tr->type == AT_MATRIX) {
		return stEvalBinaryMatrixExp(operator, tl, tr, target, i);
	}

	/* if either expression is a vector... */

	if(tl->type == AT_VECTOR || tr->type == AT_VECTOR) {
		target->type = AT_VECTOR;
		return stEvalBinaryVectorExp(operator, tl, tr, target, i);
	}

	/* if we have two strings and they're testing for equality or inequality */
	/* we do a string compare--otherwise we'll convert to doubles and handle */
	/* the expression that way */

	if(tr->type == AT_STRING && tl->type == AT_STRING && (operator == BT_EQ || operator == BT_NE)) {
		return stEvalBinaryStringExp(operator, tl, tr, target, i);
	}

	if(tr->type == AT_LIST && tl->type == AT_LIST && (operator == BT_EQ || operator == BT_NE)) {
		return stEvalBinaryEvalListExp(operator, tl, tr, target, i);
	}

	if(tr->type == AT_STRING) if((c = stToDouble(tr, tr, i)) != EC_OK) return c;
	if(tl->type == AT_STRING) if((c = stToDouble(tl, tl, i)) != EC_OK) return c;

	if(tr->type == AT_LIST) if((c = stToInt(tr, tr, i)) != EC_OK) return c;
	if(tl->type == AT_LIST) if((c = stToInt(tl, tl, i)) != EC_OK) return c;

	if(tl->type == AT_DOUBLE || tr->type == AT_DOUBLE) {
		target->type = AT_DOUBLE;
		return stEvalBinaryDoubleExp(operator, tl, tr, target, i);
	}

	target->type = AT_INT;
	return stEvalBinaryIntExp(operator, tl, tr, target, i);
}

inline int stEvalRandExp(stExp *r, stRunInstance *i, brEval *target) {
	stExpEval(r, i, target, NULL);

	switch(target->type) {
		case AT_INT:
			if(BRINT(target) < 0) BRINT(target) = 0;
			else BRINT(target) = (random() % (BRINT(target) + 1));
			return EC_OK;
			break;
		case AT_DOUBLE:
			BRDOUBLE(target) = slRandomDouble() * BRDOUBLE(target);
			return EC_OK;
			break;
		case AT_VECTOR:
			BRVECTOR(target).x = slRandomDouble() * BRVECTOR(target).x;
			BRVECTOR(target).y = slRandomDouble() * BRVECTOR(target).y;
			BRVECTOR(target).z = slRandomDouble() * BRVECTOR(target).z;
			return EC_OK;
			break;
		default:
			stEvalError(i->instance->type->engine, EE_TYPE, "expected type \"int\", \"double\" or \"vector\" in evaluation of \"random\"");
			return EC_ERROR;
	}

	return EC_OK;
}

/* turn a vector stExp tree into a real vector */

inline int stEvalVectorExp(stVectorExp *v, stRunInstance *i, brEval *target) {
	brEval tx, ty, tz;
	int result;

	target->type = AT_VECTOR;

	stExpEval(v->x, i, &tx, NULL);
	stExpEval(v->y, i, &ty, NULL);
	stExpEval(v->z, i, &tz, NULL);

	if(tx.type != AT_DOUBLE) {
		if((result = stToDouble(&tx, &tx, i)) != EC_OK) return result;
	}

	if(ty.type != AT_DOUBLE) {
		if((result = stToDouble(&ty, &ty, i)) != EC_OK) return result;
	}

	if(tz.type != AT_DOUBLE) {
		if((result = stToDouble(&tz, &tz, i)) != EC_OK) return result;
	}

	BRVECTOR(target).x = BRDOUBLE(&tx);
	BRVECTOR(target).y = BRDOUBLE(&ty);
	BRVECTOR(target).z = BRDOUBLE(&tz);

	return EC_OK;
}

inline int stEvalMatrixExp(stMatrixExp *v, stRunInstance *i, brEval *target) {
	int n, result;
	brEval t;

	target->type = AT_MATRIX;

	for(n=0;n<9;n++) {
		stExpEval(v->expressions[n], i, &t, NULL);
	
		if(t.type != AT_DOUBLE)
			if((result = stToDouble(&t, &t, i)) != EC_OK) return result;

		BRMATRIX(target)[n / 3][n % 3] = BRDOUBLE(&t);
	}

	return EC_OK;
}

/*!
	\brief Calls a method once the method has been found and the arguments have been processed.
*/

int stCallMethod(stRunInstance *old, stRunInstance *new, stMethod *method, brEval **args, int argcount, brEval *target) {
	int n;
	stKeywordEntry *keyEntry;
	stStackRecord record;
	slStack *currentGCStack;

	char *savedStackPointer, *newStStack;
	int result;

	if(new->instance->status != AS_ACTIVE) {
		stEvalError(new->instance->type->engine, EE_FREED_INSTANCE, "method \"%s\" being called with freed instance of class \"%s\" (%p)", method->name, new->instance->type->name, new->instance);
		return EC_ERROR;
	}

	savedStackPointer = new->instance->type->engine->stack;

	// if there is no current stackpointer (outermost frame), start at the end of the stack 

	if(savedStackPointer == NULL) new->instance->type->engine->stack = &new->instance->type->engine->stackBase[ST_STACK_SIZE];

	// step down the stack enough to make room for the current calling method

	newStStack = new->instance->type->engine->stack - method->stackOffset;

	if(newStStack < new->instance->type->engine->stackBase) {
		slMessage(DEBUG_ALL, "Stack overflow in class \"%s\" method \"%s\"\n", new->instance->type->name, method->name);
		return EC_ERROR;
	}

	bzero(newStStack, method->stackOffset);

	// go through the arguments and place them on the stack.

	for(n=0;n<argcount;n++) {
		keyEntry = method->keywords->data[n];

        if(keyEntry->var->type->objectName && !keyEntry->var->type->objectType)
			keyEntry->var->type->objectType = stObjectFind(new->instance->type->engine->objects, keyEntry->var->type->objectName);

		result = stSetVariable(&newStStack[keyEntry->var->offset], keyEntry->var->type->type, keyEntry->var->type->objectType, args[n], old);

		if(result != EC_OK) {
			slMessage(DEBUG_ALL, "Error evaluating keyword \"%s\" for method \"%s\"\n", keyEntry->keyword, method->name);
			new->instance->type->engine->stackRecord = new->instance->type->engine->stackRecord->previousStackRecord;
			return result;
		}
	}

	record.instance = new->instance;
	record.method = method;
	record.previousStackRecord = new->instance->type->engine->stackRecord;
	new->instance->type->engine->stackRecord = &record;
	record.gcStack = new->instance->gcStack;

	currentGCStack = new->instance->gcStack = slStackNew();

	// prepare for the actual method call 

	target->type = AT_NULL;
	new->instance->type->engine->stack = newStStack;

	result = stEvalArray(method->code, new, target);

	// we don't want the return value released, so we'll retain it.
	// this will keep it alive through the releasing stage.
	
	if(new->instance->gcStack) {
		if(target->type != AT_NULL) stGCUnmark(new->instance, target);
		stGCRetain(target);
	}

	// unretain the input arguments

	if(method->keywords) {
		for(n=0;n<method->keywords->count;n++) {
			keyEntry = method->keywords->data[n];

			stGCUnretainAndCollectPointer(*(void**)&newStStack[keyEntry->var->offset], keyEntry->var->type->type);
		}
	}

	// unretain the local variables

	if(method->variables) {
		slList *variables = method->variables;

		while(variables) {
			stVar *var = variables->data;

			stGCUnretainAndCollectPointer(*(void**)&newStStack[var->offset], var->type->type);

			variables = variables->next;
		}
	}

	if(result == EC_STOP) result = EC_OK;

	// collect the current gcStack.

	if(new->instance->gcStack) {
		stGCCollectStack(new->instance->gcStack);
		slStackFree(new->instance->gcStack);
	}

	// reset the previous gcStack.

	new->instance->gcStack = record.gcStack;

	// remember when we retained the return value before?
	// unretain it, and make it the caller's problem.

	if(new->instance->gcStack) {
		if(target->type != AT_NULL) stGCUnretain(target);
		if(old) stGCMark(old->instance, target);
	}

	// restore the previous stack and stack records

	new->instance->type->engine->stack = savedStackPointer;
	new->instance->type->engine->stackRecord = record.previousStackRecord;

	return result;
}

int stCallMethodByName(stRunInstance *new, char *name, brEval *result) {
	return stCallMethodByNameWithArgs(new, name, NULL, 0, result);
}

int stCallMethodByNameWithArgs(stRunInstance *new, char *name, brEval **args, int argcount, brEval *result) {
	stMethod *method;
	stRunInstance ri;

	ri.instance = new->instance;

	method = stFindInstanceMethod(new->instance->type, name, argcount, &ri.type);
	result->type = AT_NULL;

	if(!method) {
		stEvalError(new->instance->type->engine, EE_UNKNOWN_METHOD, "object type \"%s\" does not respond to method \"%s\"", new->instance->type->name, name); 
		return EC_ERROR;
	}

	return stCallMethod(NULL, &ri, method, args, argcount, result);
}

void stStackTrace(brEngine *e) {
	stStackRecord *r = e->stackRecord;

	slMessage(NORMAL_OUTPUT, "breve engine tack trace:\n");

	stStackTraceFrame(r);
}

int stStackTraceFrame(stStackRecord *r) {
	int n;

	if(!r) return 0;

	n = stStackTraceFrame(r->previousStackRecord);

	slMessage(NORMAL_OUTPUT, "%d) %s (%p) %s (line %d of \"%s\")\n", n, r->instance->type->name, r->instance, r->method->name, r->method->lineno, r->method->filename);

	return n + 1;
}

int stPrintEvaluation(brEval *e, stRunInstance *i) {
	char *evalString;

	evalString = stFormatEvaluation(e, i->instance);

	slFormattedMessage(NORMAL_OUTPUT, evalString);

	slFree(evalString);

	return EC_OK;
}

/*!
	\brief Evaluates a new instance expression in steve.

	Produces either a single object or a list.
*/

inline int stEvalNewInstance(stInstanceExp *ie, stRunInstance *i, brEval *t) {
	brObject *class;
	brEvalListHead *list = NULL;
	brEval listItem, count;
	int n;

	class = brObjectFind(i->instance->type->engine, ie->name);

	if(!class) {
		stEvalError(i->instance->type->engine, EE_UNKNOWN_OBJECT, "unknown object type \"%s\" during new instance evaluation\n", ie->name);
		return EC_ERROR;
	}

	/* how many instances are we creating? */

	stExpEval(ie->count, i, &count, NULL);

	if(count.type != AT_INT) {
		stEvalError(i->instance->type->engine, EE_TYPE, "expected integer count for \"new\" expression\n");
		return EC_ERROR;
	}

	if(BRINT(&count) == 1) {
		t->type = AT_INSTANCE;

		STINSTANCE(t) = stInstanceCreateAndRegister(i->instance->type->engine, class);

		if(STINSTANCE(t) == NULL) return EC_ERROR_HANDLED;
	} else {
		listItem.type = AT_INSTANCE;
		list = brEvalListNew();

		for(n=0;n<BRINT(&count);n++) {
			STINSTANCE(&listItem) = stInstanceCreateAndRegister(i->instance->type->engine, class);

			if(STINSTANCE(&listItem) == NULL) return EC_ERROR_HANDLED;

			brEvalListInsert(list, 0, &listItem);
		}

		t->type = AT_LIST;
		BRLIST(t) = list;
	}

	stGCMark(i->instance, t);

	return EC_OK;
}

int stExpEval(stExp *s, stRunInstance *i, brEval *target, stObject **tClass) {
	brEval t;
	int result = EC_OK;

	if(tClass) *tClass = NULL;

	target->type = AT_NULL;

	if(!i) {
		stEvalError(i->instance->type->engine, EE_INTERNAL, "Expression evaluated with uninitialized instance");
		return EC_ERROR;
	}

	if(i->instance->status != AS_ACTIVE) {
		// we don't allow execution with freed instances, unless it's a return
		// value, in which case it's okay.

		if(s->type == ET_RETURN) {
			if(!s->values.pValue) {
				target->type = AT_NULL;
				result = EC_STOP;
			}

			result = stExpEval(s->values.pValue, i, target, NULL);

			if(result != EC_OK) result = result;
			result = EC_STOP;
		}

		stEvalError(i->instance->type->engine, EE_FREED_INSTANCE, "Expression evaluated using freed instance");
		return EC_ERROR;
	}

	if(!s) return EC_OK;

	if(s->debug == 1) slDebug("debug called from within steve evaluation\n");

	switch(s->type) {
		case ET_LOAD:
			result = stEvalLoad(s->values.pValue, i, target);
			break;
		case ET_ASSIGN:
			result = stEvalAssignment(s->values.pValue, i, target);
			break;
		case ET_BINARY:
			result = stEvalBinaryExp(s->values.pValue, i, target);
			break;
		case ET_METHOD:
			result = stEvalMethodCall(s->values.pValue, i, target);
			break;
		case ET_FUNC:
			result = stEvalCallFunc(s->values.pValue, i, target);
			break;
		case ET_RETURN:
			if(!s->values.pValue) {
				target->type = AT_NULL;
				result = EC_STOP;
			}

			result = stExpEval(s->values.pValue, i, &t, NULL);

			brEvalCopy(&t, target);

			if(result != EC_OK) result = result;
			result = EC_STOP;
			break;
		case ET_ARRAY_INDEX:
			result = stEvalArrayIndex(s->values.pValue, i, target);
			break;
		case ET_ARRAY_INDEX_ASSIGN:
			result = stEvalArrayIndexAssign(s->values.pValue, i, target);
			break;
		case ET_INT:
			target->type = AT_INT;
			BRINT(target) = s->values.iValue;
			break;
		case ET_DOUBLE:
			target->type = AT_DOUBLE;
			BRDOUBLE(target) = s->values.dValue;
			break;
		case ET_SUPER:
			target->type = AT_INSTANCE;
			STINSTANCE(target) = i->instance;
			*tClass = i->type->super;
			break;
		case ET_SELF:
			target->type = AT_INSTANCE;
			STINSTANCE(target) = i->instance;
			break;
		case ET_IF:
			result = stEvalIf(s->values.pValue, i, target);
			break;
		case ET_VECTOR:
			result = stEvalVectorExp(s->values.pValue, i, target);
			break;
		case ET_MATRIX:
			result = stEvalMatrixExp(s->values.pValue, i, target);
			break;
		case ET_UNARY:
			result = stEvalUnaryExp(s->values.pValue, i, target);
			break;
		case ET_ST_EVAL:
			bcopy(s->values.pValue, target, sizeof(brEval));
			break;
		case ET_LIST:
			result = brEvalListExp(s->values.pValue, i, target);
			break;
		case ET_CODE_ARRAY:
			result = stEvalArray(s->values.pValue, i, target);
			break;
		case ET_VLENGTH:
			result = stExpEval(s->values.pValue, i, target, NULL);
			if(result != EC_OK) result = result;

			switch(target->type) {
				brEvalListHead *l;

				case AT_VECTOR:
					target->type = AT_DOUBLE;
					BRDOUBLE(target) = slVectorLength(&BRVECTOR(target));
					result = EC_OK;
					break;
				case AT_LIST:
					l = BRLIST(target);
					target->type = AT_INT;
					BRINT(target) = l->count;
					result = EC_OK;
					break;
				case AT_INT:
					target->type = AT_INT;
					BRINT(target) = abs(BRINT(target));
					result = EC_OK;
					break;
				case AT_DOUBLE:
					target->type = AT_DOUBLE;
					BRDOUBLE(target) = fabs(BRDOUBLE(target));
					result = EC_OK;
					break;
				default:
					stEvalError(i->instance->type->engine, EE_TYPE, "Cannot give magnitude of %s expression", slAtomicTypeStrings[target->type]);
					return EC_ERROR;
					break;
				
			}
			

			break;
		case ET_VNORM:
			result = stExpEval(s->values.pValue, i, target, NULL);

			if(result != EC_OK) result = result;

			if(target->type != AT_VECTOR) {
				stEvalError(i->instance->type->engine, EE_TYPE, "expected type \"vector\" in vector normalization evaluation");
				return EC_ERROR;
			}

			target->type = AT_VECTOR;
			slVectorMul(&BRVECTOR(target), 1.0/slVectorLength(&BRVECTOR(target)), &BRVECTOR(target));
			result = EC_OK;
			break;
		case ET_ERROR:
			stEvalPrint(s->values.pValue, i, &t);
			return EC_ERROR;
			break;
		case ET_WHILE:
			result = stEvalWhile(s->values.pValue, i, target);
			break;
		case ET_FOREACH:
			result = stEvalForeach(s->values.pValue, i, target);
			break;
		case ET_FOR:
			result = stEvalFor(s->values.pValue, i, target);
			break;
		case ET_INSERT:
			result = stEvalListInsert(s->values.pValue, i, target);
			break;
		case ET_REMOVE:
			result = stEvalListRemove(s->values.pValue, i, target);
			break;
		case ET_COPYLIST:
			result = stEvalCopyList(s->values.pValue, i, target);
			break;
		case ET_ALL:
			result = stEvalAll(s->values.pValue, i, target);
			break;
		case ET_SORT:
			result = stEvalSort(s->values.pValue, i, target);
			break;
		case ET_LIST_INDEX:
			result = stEvalListIndex(s->values.pValue, i, target);
			break;
		case ET_LIST_INDEX_ASSIGN:
			result = stEvalListIndexAssign(s->values.pValue, i, target);
			break;
		case ET_PRINT:
			result = stEvalPrint(s->values.pValue, i, &t);
			break;
		case ET_STRING:
			result = stProcessString(s->values.pValue, i, target);
			break; 
		case ET_INSTANCE:
			result = stEvalNewInstance(s->values.pValue, i, target);
			break;
		case ET_VECTOR_ELEMENT:
			result = stEvalVectorElementExp(s->values.pValue, i, target);
			break;
		case ET_VECTOR_ELEMENT_ASSIGN:
			result = stEvalVectorElementAssignExp(s->values.pValue, i, target);
			break;
		case ET_RANDOM:
			result = stEvalRandExp(s->values.pValue, i, target);
			break;
		case ET_DUPLICATE:
			result = stExpEval(s->values.pValue, i, target, NULL);
			break;
		case ET_FREE:
			result = stEvalFree(s->values.pValue, i, target);
			break;
		case ET_DIE:
			result = stExpEval(s->values.pValue, i, &t, NULL);
			stEvalError(i->instance->type->engine, EE_USER, "execution stopped from within simulation: %s", BRSTRING(&t));
			return EC_ERROR;
			break;
		default:
			stEvalError(i->instance->type->engine, EE_INTERNAL, "unknown expression type (%d) in stExpEval", s->type);
			result = EC_ERROR;
			break;
	}

	return result;
}

/*!
	\brief Gives a pointer to the eval-value of a list at a specified index. 
    
	The index is 0-based.
*/  


int stDoEvalListIndexPointer(brEvalListHead *l, int n, brEval **eval) {
    brEvalList *list;

    if(n > (l->count - 1) || n < 0) {
        *eval = NULL;
        return -1;
    }

    list = brEvalListIndexLookup(l, n);
    *eval = &list->eval;
    return 0;
}

/*!
	\brief Assigns a value to a specified index of an eval list.

    The index is 0-based.  If the index is out of bounds, the brEval 
	pointer is set to type AT_NULL and -1 is returned.  Upon success, 
	0 is returned.
*/  

int stDoEvalListIndex(brEvalListHead *l, int n, brEval *newLoc) {
    brEval *eval;

    stDoEvalListIndexPointer(l, n, &eval);
    
    if(!eval) {
        newLoc->type = AT_NULL;
        return -1;
    }
    
    brEvalCopy(eval, newLoc);
    
    return 0;
}
   
int stDoEvalListIndexAssign(brEvalListHead *l, int n, brEval *newVal, stRunInstance *ri) {
    brEvalList *list;
    
    if(n > l->count) {
        return -1;
    }   
    
    // if this is a new entry at the end, append it instead
    
    if(n == l->count || n < 0) {
        if(!brEvalListInsert(l, l->count, newVal)) return -1;
        
        return 0;
    }   

    list = brEvalListIndexLookup(l, n);

	stGCUnretainAndCollect(&list->eval);

    brEvalCopy(newVal, &list->eval);

    return 0;
}

int stEvalBinaryEvalListExp(char op, brEval *l, brEval *r, brEval *target, stRunInstance *i) {
    brEvalListHead *h1, *h2;
    brEvalList *l1, *l2;
    int same;
    int ret;

    h1 = BRLIST(l);
    h2 = BRLIST(r);

    if(h1->count != h2->count) {
        same = 0;
    } else {
        l1 = h1->start;
        l2 = h2->start;

        same = 1;

        while(l1 && l2 && same) {
            ret = stEvalBinaryExpWithEvals(i, BT_EQ, &l1->eval, &l2->eval, target);

            if(ret != EC_OK || !BRINT(target)) same = 0;

            l1 = l1->next;
            l2 = l2->next;
        }
    }

    switch(op) {
        case BT_EQ:
            BRINT(target) = same;
            break;
        case BT_NE:
            BRINT(target) = !same;
            break;
        default:
            stEvalError(i->instance->type->engine, EE_INTERNAL, "unknown binary expression operator (%d) in stEvalBinaryDoubleExp", op);
            return EC_ERROR;
            break;
    }

    return EC_OK;
}

/*!
	\brief Convert breveInstances to stInstsances using the pointer field.

	Sigh.  The breveObjectAPI gives a great deal of freedom when it comes to porting the
	breve engine to other languages, but it requires this ugliness of converting between
	steve objects and breve objects.
*/

void stConvertBreveInstanceToSteveInstance(brEval *e) {
	if(e->type == AT_INSTANCE) {
		if(BRINSTANCE(e)) STINSTANCE(e) = BRINSTANCE(e)->pointer;
	}

	if(e->type == AT_LIST) {
		brEvalListHead *head = BRLIST(e);
		brEvalList *l;

		l = head->start;

		while(l) {
			stConvertBreveInstanceToSteveInstance(&l->eval);
			l = l->next;
		}
	}
}

/*!
	\brief Convert stInstances to brInstsances using the breveInstance field.

	Once again, not something to be proud of.
*/

void stConvertSteveInstanceToBreveInstance(brEval *e) {
	if(e->type == AT_INSTANCE) {
		if(STINSTANCE(e)) BRINSTANCE(e) = STINSTANCE(e)->breveInstance;
	}

	if(e->type == AT_LIST) {
		brEvalListHead *head = BRLIST(e);
		brEvalList *l;

		l = head->start;

		while(l) {
			stConvertSteveInstanceToBreveInstance(&l->eval);
			l = l->next;
		}
	}
}

/*!
	\brief Triggers a run-time simulation error.

	A copy of \ref brEvalError that includes a steve stack trace.

	Takes an engine, a type (one of the \ref parseErrorMessageCodes), and
	a set of printf-style arguments (format string and data).

	Exactly how the error is handled depends on the simulation frontend,
	but this will typically cause a simulation to die.
*/

void stEvalError(brEngine *e, int type, char *proto, ...) {
	va_list vp;
	char localMessage[BR_ERROR_TEXT_SIZE];

	if(e->error.type == 0) {
		e->error.type = type;
        
		va_start(vp, proto);
		vsnprintf(e->error.message, BR_ERROR_TEXT_SIZE, proto, vp);
		va_end(vp); 

		slMessage(DEBUG_ALL, e->error.message);
	} else {
		va_start(vp, proto);
		vsnprintf(localMessage, BR_ERROR_TEXT_SIZE, proto, vp);
		va_end(vp); 

		slMessage(DEBUG_ALL, localMessage);
	}
    
	slMessage(DEBUG_ALL, "\n");
    
	stStackTrace(e);
}

/*!
	\brief Prints an evaluation warning.
*/

void stEvalWarn(stExp *exp, char *proto, ...) {
	va_list vp;
	char localMessage[BR_ERROR_TEXT_SIZE];

	va_start(vp, proto);
	vsnprintf(localMessage, BR_ERROR_TEXT_SIZE, proto, vp);
	va_end(vp); 
	slMessage(DEBUG_ALL, localMessage);
	slMessage(DEBUG_ALL, "... at in file \"%s\" at line %d\n", exp->file, exp->line);
}
