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
#include "expression.h"
#include "evaluation.h"

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
	"bridge-object",	// unused
	"pointer",
	"vector",
	"matrix",
	"list",
	"array",
	"data",
	"hash"
};

#define CODE_BLOCK_SIZE	128

/*!
	\brief Creates a new RTC block of size CODE_BLOCK_SIZE (cache line size)
	
	Allocate a new RtcCodeBlock and then allocate the memory for it
*/

stRtcCodeBlock *stNewRtcBlock()
{
	// stRtcCodeBlock	*block = (stRtcCodeBlock *)malloc(sizeof(CODE_BLOCK_SIZE));
	stRtcCodeBlock	*block = (stRtcCodeBlock *)malloc(CODE_BLOCK_SIZE);
	
	block->length	= CODE_BLOCK_SIZE;
	block->calls.ptr		= block->code;
	
	return block;
}

/*!
	\brief update a RtcCodeBlock with more code of size CODE_BLOCK_SIZE (cache line size)
	
	Update a RtcCodeBlock with more code using realloc of size CODE_BLOCK_SIZE (cache line size)
*/

inline void stUpdateBlockSize(stRtcCodeBlock *block) {
	block->length += CODE_BLOCK_SIZE;
	block = (stRtcCodeBlock*)realloc(block, block->length);
}

/*!
	\brief flush instruction and data cache

	Flush instruction and data cache	
*/

inline void stFlushBlock(stRtcCodeBlock *block) {

}

/*!
	\brief Get space from the current block (or add additional space)
*/

inline unsigned int *stAllocInstSpace(stRtcCodeBlock *block, unsigned int word_count)
{
	if ((block->calls.ptr + word_count) > (block->code + block->length))
	{
		stUpdateBlockSize(block);
		
		word_count -= CODE_BLOCK_SIZE;
		
		if (word_count > 0)
			return stAllocInstSpace(block, word_count);
	}
	
	return block->calls.ptr;
}

/*!
	\brief Returns a pointer for an evaluation in steve.

	For an array access, list access or regular load, this function gives a 
	pointer to the desired data.  This lets us have a unified approach for 
	variable lookups.
*/

inline int stPointerForExp(stExp *expression, stRunInstance *i, void **pointer, int *type) {
	switch(expression->type) {
	case ET_LOAD:
		return stEvalLoadPointer((stLoadExp *)expression, i, pointer, type);
	case ET_ARRAY_INDEX:
		return stEvalArrayIndexPointer((stArrayIndexExp *)expression, i, pointer, type);
	case ET_LIST_INDEX:
		return stEvalListIndexPointer((stListIndexExp *)expression, i, pointer, type);
	default:
		slMessage(DEBUG_ALL, "Invalid value for left-hand side of assignment expression");
		return EC_ERROR;
	}
}

typedef struct stRtcVar_t {
	unsigned char		reg;
	unsigned char		type;
	unsigned short		stack_index;
	struct stRtcVar_t	*next;
} stRtcVar;

typedef struct stRtcFunc_t {
	unsigned int	gp_mask;
	unsigned int	fp_mask;
	unsigned int	vp_mask;
	unsigned short	stack_size;
	stRtcVar		*gp_regs[32];
	stRtcVar		*fp_regs[32];
	stRtcVar		*vp_regs[32];
	stRtcVar		*gp_reg_list;
	stRtcVar		*fp_reg_list;
	stRtcVar		*vp_reg_list;
} stRtcFunc;

enum stRtcRegFile {
	GPREG,
	FPREG,
	VPREG,
	UNKNOWN_REG = 0xff
};

void stRtcInitVar(stRtcVar *var)
{
	var->reg			= 0xff;
	var->type			= UNKNOWN_REG;
	var->stack_index	= 0xffff;
	var->next			= NULL;
}

void stRtcInitFunc(stRtcFunc *func)
{
	unsigned int	i;

	func->gp_mask = 0;
	func->fp_mask = 0;
	func->vp_mask = 0;
	func->stack_size = 0;

	for(i=0; i<32; i++)
	{
		func->gp_regs[i] = 0;
		func->fp_regs[i] = 0;
		func->vp_regs[i] = 0;
	}
	
	func->gp_reg_list = 0;
	func->fp_reg_list = 0;
	func->vp_reg_list = 0;
}

void stRtcAllocGPReg(stRtcCodeBlock *bloc, stRtcFunc *func, stRtcVar *var)
{
	unsigned int	i, bit;
	stRtcVar		*temp;
	
	for(i=2; i <32; i++)
	{
		bit = 1 << i;
		
		if (!(func->gp_mask & bit))
		{
			var->reg			= i;
			var->type			= GPREG;
			var->stack_index	= func->stack_size;
			var->next			= func->gp_reg_list;
			
			func->gp_mask		|= bit;
			func->stack_size	+= 4;
			func->gp_regs[i]	= var;
			func->gp_reg_list	= var;
			
			return;
		}
	}

	temp = func->gp_reg_list;
	while(temp->next)
	{
		temp = temp->next;
	}
	
	// save temp to stack
	// set reg to temp->reg
	// set temp->reg to 0xff
	// move reg to head
}

void stLoadGPReg(stRtcCodeBlock *block, stRtcFunc *func, stRtcVar *var)
{
	if (var->reg != 0xff)
		return;

	// load reg from stack
	// move reg to head
}

void stRtcAllocFPReg(stRtcCodeBlock *bloc, stRtcFunc *func, stRtcVar *var)
{
	unsigned int	i, bit;
	stRtcVar		*temp;
	
	for(i=1; i <32; i++)
	{
		bit = 1 << i;
		
		if (!(func->fp_mask & bit))
		{
			var->reg			= i;
			var->type			= FPREG;
			var->stack_index	= func->stack_size;
			var->next			= func->fp_reg_list;
			
			func->fp_mask		|= bit;
			func->stack_size	+= 4;
			func->fp_regs[i]	= var;
			func->fp_reg_list	= var;
			
			return;
		}
	}

	temp = func->fp_reg_list;
	while(temp->next)
	{
		temp = temp->next;
	}
	
	// save temp to stack
	// set reg to temp->reg
	// set temp->reg to 0xff
	// move reg to head
}

/*!
	\brief Load a pointer in steve.
*/

RTC_INLINE int stEvalLoadPointer(stLoadExp *e, stRunInstance *i, void **pointer, int *type) {
	*type = e->loadType;

	if(e->local) *pointer = &i->instance->type->steveData->stack[e->offset];
	else *pointer = &i->instance->variables[e->offset];

	return EC_OK;
}

/*!
	\brief Convert an evaluation an int in steve.
*/

inline int stToInt(brEval *e, brEval *t, stRunInstance *i) {
	char *str;
	int resultCode;

	switch(e->type) {
		case AT_INT:
			resultCode = BRINT(e);
			break;
		case AT_DOUBLE:
			resultCode = (int)BRDOUBLE(e);
			break;
		case AT_STRING:
			str = BRSTRING(e);
			if(str) resultCode = atoi(BRSTRING(e));
			else resultCode = 0;
			break;
		case AT_LIST:
			resultCode = BRLIST(e)->count;
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
	
	BRINT(t) = resultCode;
	t->type = AT_INT;

	return EC_OK;
}

/*!
	\brief Convert an evaluation a double in steve.
*/

int stToDouble(brEval *e, brEval *t, stRunInstance *i) {
	char *str;
	brEvalListHead *theList;
	double resultCode;

	switch(e->type) {
		case AT_DOUBLE:
			resultCode = BRDOUBLE(e);
			break;
		case AT_INT:
			resultCode = (double)BRINT(e);
			break;
		case AT_STRING:
			str = BRSTRING(e);
			if(str) resultCode = atof(BRSTRING(e));
			else resultCode = 0.0;
			break;
		case AT_LIST:
			theList = BRLIST(e);
			resultCode = (double)theList->count;
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

	BRDOUBLE(e) = resultCode;
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

static int stEvalLoadInt(stLoadExp *e, stRunInstance *i, brEval *target) {
	void *pointer;
	
	target->type = e->loadType;

	if (e->local)
		pointer = &i->instance->type->steveData->stack[e->offset];
	else
		pointer = &i->instance->variables[e->offset];
	
	BRINT(target) = *(int *)pointer;

	return EC_OK;
}

static int stEvalLoadDouble(stLoadExp *e, stRunInstance *i, brEval *target) {
	void *pointer;
	
	target->type = AT_DOUBLE;

	if (e->local)
		pointer = &i->instance->type->steveData->stack[e->offset];
	else
		pointer = &i->instance->variables[e->offset];
	
	BRDOUBLE(target) = *(double *)pointer;

	return EC_OK;
}

static int stEvalLoadIndirect(stLoadExp *e, stRunInstance *i, brEval *target) {
	void *pointer;
	
	target->type = e->loadType;

	if (e->local)
		pointer = &i->instance->type->steveData->stack[e->offset];
	else
		pointer = &i->instance->variables[e->offset];
	
	BRPOINTER(target) = *(void **)pointer;

	return EC_OK;
}

static int stEvalLoadVector(stLoadExp *e, stRunInstance *i, brEval *target) {
	void *pointer;
	
	target->type = AT_VECTOR;

	if (e->local)
		pointer = &i->instance->type->steveData->stack[e->offset];
	else
		pointer = &i->instance->variables[e->offset];
	
	slVectorCopy((slVector *)pointer, &BRVECTOR(target));

	return EC_OK;
}

static int stEvalLoadList(stLoadExp *e, stRunInstance *i, brEval *target) {
	void *pointer;
	
	target->type = AT_LIST;

	if (e->local)
		pointer = &i->instance->type->steveData->stack[e->offset];
	else
		pointer = &i->instance->variables[e->offset];
	
	if (!*(brEvalListHead **)pointer) {
		// if there is nothing here (uninitialized list), then we create an empty one.  
		// we retain it, since it's already stored as a variable.

		*(brEvalListHead **)pointer = brEvalListNew();
		stGCRetainPointer(*(void **)pointer, AT_LIST);
	}

	BRLIST(target) = *(brEvalListHead **)pointer;

	return EC_OK;
}

static int stEvalLoadHash(stLoadExp *e, stRunInstance *i, brEval *target) {
	void *pointer;
	
	target->type = AT_HASH;

	if (e->local)
		pointer = &i->instance->type->steveData->stack[e->offset];
	else
		pointer = &i->instance->variables[e->offset];
	
	if (!*(brEvalHash **)pointer) {
		// if there is nothing here (uninitialized hash), then we create an empty one.  
		// we retain it, since it's already stored as a variable.

		*(brEvalHash **)pointer = brEvalHashNew();
		stGCRetainPointer(*(void **)pointer, AT_LIST);
	}

	BRHASH(target) = *(brEvalHash **)pointer;

	return EC_OK;
}

static int stEvalLoadString(stLoadExp *e, stRunInstance *i, brEval *target) {
	void *pointer;
	
	target->type = AT_STRING;

	if (e->local)
		pointer = &i->instance->type->steveData->stack[e->offset];
	else
		pointer = &i->instance->variables[e->offset];
	
	if (!(*(char **)pointer))
		*(char **)pointer = slStrdup("");

	BRSTRING(target) = slStrdup(*(char **)pointer);

	stGCMark(i->instance, target);

	return EC_OK;
}

static int stEvalLoadMatrix(stLoadExp *e, stRunInstance *i, brEval *target) {
	void *pointer;
	
	target->type = AT_MATRIX;

	if (e->local)
		pointer = &i->instance->type->steveData->stack[e->offset];
	else
		pointer = &i->instance->variables[e->offset];
	
	slMatrixCopy((double **)pointer, BRMATRIX(target));

	return EC_OK;
}

int stLoadVariable(void *variable, unsigned char type, brEval *e, stRunInstance *i) {
	e->type = type;

	// figure out which type we're dealing with so we interpret the pointer correctly 

	switch(e->type) {
		case AT_INT:
			BRINT(e) = *(int *)variable;
			break;
		case AT_DOUBLE:
			BRDOUBLE(e) = *(double *)variable;
			break;
		case AT_POINTER:
		case AT_INSTANCE:
		case AT_DATA:
			BRPOINTER(e) = *(void **)variable;
			break;
		case AT_VECTOR:
			slVectorCopy((slVector *)variable, &BRVECTOR(e));
			break;
		case AT_LIST:
			// if there is nothing here (uninitialized list), then we create an empty list.  
			// we retain it, since it's now stored as a variable.

			if (!*(brEvalListHead **)variable) {
				*(brEvalListHead **)variable = brEvalListNew();
				stGCRetainPointer(*(void **)variable, AT_LIST);
			}

			BRLIST(e) = *(brEvalListHead **)variable;
			break;
		case AT_HASH:
			if (!*(brEvalHash **)variable) {
				// if there is nothing here (uninitialized hash), then we create an empty hash.  
				// we retain it, since it's stored as a variable.

				*(brEvalHash **)variable = brEvalHashNew();
				stGCRetainPointer(*(void **)variable, AT_HASH);
			}

			BRHASH(e) = *(brEvalHash **)variable;			
			break;
		case AT_STRING:
			if (!(*(char**)variable))
				*(char **)variable = slStrdup("");

			BRSTRING(e) = slStrdup(*(char **)variable);
			break;
		case AT_MATRIX:
			slMatrixCopy((double **)variable, BRMATRIX(e));
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
	stInstance *instance;
	char *newstr;
	int resultCode;

	if (type == AT_INSTANCE && otype && e->type == AT_INSTANCE && BRINSTANCE(e)) {
		// if they specified an object type in the code, make sure 
		// that this is a valid cast.  this requires a lookup each
		// time we're here, so it could really be improved.

		instance = (stInstance *)BRINSTANCE(e)->userData;
		
		if (!stIsSubclassOf(instance->type, otype)) {
			stEvalError(i->instance->type->engine, EE_TYPE, "Cannot assign instance of class \"%s\" to variable of class \"%s\"\n", instance->type->name, otype->name);
			return EC_ERROR;
		}
	}

	if ((resultCode = stToType(e, type, e, i)) != EC_OK)
		return resultCode;

#ifdef MULTITHREAD
	if(i) pthread_mutex_lock(&i->lock);
#endif

	switch(type) {
		case AT_INT:
			*(int *)variable = BRINT(e);
			break;
		case AT_DOUBLE:
			*(double *)variable = BRDOUBLE(e);
			break;
		case AT_VECTOR:
			slVectorCopy(&BRVECTOR(e), (slVector *)variable);
			break;
		case AT_MATRIX:
			slMatrixCopy(BRMATRIX(e), (double **)variable);
			break;
		case AT_INSTANCE:
			*(brInstance **)variable = BRINSTANCE(e);
			break;
		case AT_DATA:
			// overwriting an old data variable?

			if (*(brData **)variable != BRDATA(e))
				stGCUnretainAndCollectPointer(*(void **)variable, AT_DATA);

			// retain and set the new data 

			if (BRDATA(e))
				*(brData **)variable = BRDATA(e);

			break;
		case AT_POINTER:
			*(void **)variable = BRPOINTER(e);
			break;
		case AT_STRING:
			// overwriting an old string?
			if (*(char **)variable)
				slFree(*(void **)variable);	

			newstr = slStrdup(BRSTRING(e));

			*(char **)variable = newstr;

			break;
		case AT_LIST:
			if (!BRLIST(e)) {
				// if this is an empty list, just allocate some space,
				// and mark it.  it will be retained later.

				e->type = AT_LIST;
				BRLIST(e) = brEvalListNew();
			} else if (*(brEvalListHead **)variable != BRLIST(e)) {
				// overwriting an old list at this location

				stGCUnretainAndCollectPointer(*(void **)variable, AT_LIST);
			} 

			*(brEvalListHead **)variable = BRLIST(e);

			break;
		case AT_HASH:
			// if this is an empty hash, just allocate some space...

			if (!BRHASH(e)) {
				e->type = AT_HASH;
				BRHASH(e) = brEvalHashNew();
			} else if(*(brEvalHash **)variable != BRHASH(e)) {
				// overwriting an old hash at this location
				stGCUnretainAndCollectPointer(*(void **)variable, AT_HASH);
			} 

			*(brEvalHash **)variable = BRHASH(e);

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

	stGCRetain(e);
	if (i) stGCUnmark(i->instance, e);

#ifdef MULTITHREAD
	if(i) pthread_mutex_unlock(&i->lock);
#endif

	return EC_OK;
}

/*!
	\brief Computes the "truth" of an expression in steve.

	Takes any kind of expression and decides whether it is "true" in
	the context of boolean expressions.  It yields an int with a value 
	of either 1 or 0.
*/

RTC_INLINE int stEvalTruth(brEval *e, brEval *t, stRunInstance *i) {
	// e may equal t, so put the type in a local variable before changing it... 

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
			if (BRINSTANCE(e) && (BRINSTANCE(e))->status != AS_ACTIVE)
				BRINT(t) = 0;
			else
				BRINT(t) = (BRINSTANCE(e) != NULL);

			return EC_OK;
			break;
		case AT_POINTER:
		case AT_HASH:
		case AT_DATA:
			BRINT(t) = (BRPOINTER(e) != NULL);
			return EC_OK;
			break;
		case AT_LIST:
			BRINT(t) = (BRPOINTER(e) != NULL && (BRLIST(e))->count != 0);
			return EC_OK;
			break;
		case AT_VECTOR:
			BRINT(t) = !(slVectorIsZero(&BRVECTOR(e)));
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

RTC_INLINE int stEvalFree(stFreeExp *s, stRunInstance *i, brEval *t) {
	brEval result;
	brEvalList *list;
	int resultCode;
	int finished = 0;

	t->type = AT_NULL;

	resultCode = stExpEval3(s->expression, i, &result);
	if (resultCode != EC_OK)
		return EC_ERROR;

	if (result.type == AT_INSTANCE) {
		if (!BRINSTANCE(&result)) {
			slMessage(DEBUG_ALL, "warning: attempt to free uninitialized object\n");
			return EC_OK;
		}

		// if we're freeing ourself (the calling instance) then we will return EC_STOP

		if (BRINSTANCE(&result)->userData == i->instance)
			finished = 1;

		if (BRINSTANCE(&result)->status == AS_ACTIVE)
			brInstanceRelease(BRINSTANCE(&result));
	} else if (result.type == AT_LIST) {
		list = (BRLIST(&result))->start;

		while (list) {
			if (list->eval.type == AT_INSTANCE) {
				if (!BRINSTANCE(&list->eval))
					slMessage(DEBUG_ALL, "warning: attempt to free uninitialized object\n");
				else {
					// if we're freeing ourself (the calling instance) then we will return EC_STOP
					if (BRINSTANCE(&list->eval)->userData == i->instance) finished = 1;

					if (BRINSTANCE(&list->eval)->status == AS_ACTIVE) brInstanceRelease(BRINSTANCE(&list->eval));
				}
			}

			list = list->next;
		}
	}

	if (finished)
		return EC_STOP;

	return EC_OK;
}

/*!
	\brief Evaluates an array of expressions in steve.
*/

RTC_INLINE int stEvalCodeArray(stCodeArrayExp *a, stRunInstance *i, brEval *result) {
	return stEvalExpVector(&a->expressions, i, result);
}

int stEvalExpVector(std::vector< stExp* > *a, stRunInstance *i, brEval *result) {
	unsigned int count;
	int resultCode;

	if (!i)
		return EC_ERROR;
	if (i->instance->status != AS_ACTIVE)
		return EC_OK;
	if (!result)
		return EC_ERROR;

	for (count = 0; count < (*a).size(); ++count) {
		stExp *expression = (*a)[count];

		if (expression) {
			resultCode = stExpEval3(expression, i, result);

			if (resultCode == EC_ERROR) {
				brErrorInfo *error = brEngineGetErrorInfo(i->type->engine);

				error->line = expression->line;

				if (error->file)
					slFree(error->file);
				error->file = slStrdup(expression->file);

				slMessage(DEBUG_ALL, "... error in file \"%s\" at line %d\n", expression->file, expression->line);

				return EC_ERROR_HANDLED;
			} else if (resultCode == EC_ERROR_HANDLED) {
				return EC_ERROR_HANDLED;
			} else if (resultCode == EC_STOP) {
				// an EC_STOP indicates a return value or other interruption
				return EC_STOP;
			} else {
				// collect the leftovers.
				stGCCollect(result);
			}
		}
	}

	// if we're here, we can't be returning a value.  set the
	// result type to AT_NULL so nothing gets incorrectly released.

	result->type = AT_NULL;

	return EC_OK;
}

/*!
	\brief Evaluates a hardcoded list expression in steve.

	list is a list of stExp expressions, each of which is evaluated
	and added to the list.
*/

inline int brEvalListExp(stListExp *le, stRunInstance *i, brEval *result) {
	brEval index;
	int resultCode;
	unsigned int n;

	result->type = AT_LIST;
	BRLIST(result) = brEvalListNew();

	for(n = 0; n < le->expressions.size(); n++ ) {
		resultCode = stExpEval3(le->expressions[n], i, &index);

		if(resultCode != EC_OK) return resultCode;

		brEvalListInsert(BRLIST(result), BRLIST(result)->count, &index);
		stGCRetain(&index);
		stGCUnmark(i->instance, &index);
	}

	stGCMark(i->instance, result);

	return EC_OK;
}

/*!
	\brief Initiates evaluation of a method call expression in steve.

	This function doesn't do the actual method calling--it evaluates the
	calling object or object-list, and then calls \ref stRealEvalMethodCall
	for each object.
*/

RTC_INLINE int stEvalMethodCall(stMethodExp *mexp, stRunInstance *i, brEval *t) {
	stRunInstance ri;
	brEval obj;
	int r;

	r = stExpEval(mexp->objectExp, i, &obj, &ri.type);

	if (r != EC_OK)
		return r;

	if (obj.type == AT_INSTANCE) {
		if(!BRINSTANCE(&obj) || BRINSTANCE(&obj)->status != AS_ACTIVE) {
			stEvalError(i->type->engine, EE_NULL_INSTANCE, "method \"%s\" called with uninitialized object", mexp->methodName);
			return EC_ERROR;
		}

		// if the new instance is not a steve object, it's a foreign method call

		if(BRINSTANCE(&obj)->object->type != &i->instance->type->steveData->steveObjectType)
			return stEvalForeignMethodCall(mexp, BRINSTANCE(&obj), i, t);

		ri.instance = (stInstance *)BRINSTANCE(&obj)->userData;

		if (!ri.type)
			ri.type = ri.instance->type;

		r = stRealEvalMethodCall(mexp, &ri, i, t);

		return r;
	}

	if (obj.type == AT_LIST) {
		brEvalList *listStart = BRLIST(&obj)->start;

		while (listStart) {
			if (!BRINSTANCE(&listStart->eval) || BRINSTANCE(&listStart->eval)->status != AS_ACTIVE) {
				stEvalError(i->type->engine, EE_NULL_INSTANCE, "method \"%s\" called with uninitialized object", mexp->methodName);
				return EC_ERROR;
			}

			// if the new instance is not a steve object, it's a foreign method call

			if (BRINSTANCE(&listStart->eval)->object->type != &i->instance->type->steveData->steveObjectType)
				return stEvalForeignMethodCall(mexp, BRINSTANCE(&listStart->eval), i, t);

			ri.instance = (stInstance *)BRINSTANCE(&listStart->eval)->userData;
			ri.type = ri.instance->type;

			r = stRealEvalMethodCall(mexp, &ri, i, t);
			listStart = listStart->next;
		}

		return r;
	}

	stEvalError(i->instance->type->engine, EE_TYPE, "Method \"%s\" called for an expression that is neither an object nor a list", mexp->methodName);

	return EC_ERROR;
}

int stEvalForeignMethodCall(stMethodExp *mexp, brInstance *caller, stRunInstance *i, brEval *t) {
	brEval args[mexp->arguments.size()];
	brEval *argps[mexp->arguments.size()];
	unsigned int n;
	int resultCode;

	for(n = 0; n < mexp->arguments.size(); ++n) {
		stKeyword *k = mexp->arguments[n];

		argps[n] = &args[n];

		resultCode = stExpEval(k->value, i, argps[n], NULL);
		if (resultCode != EC_OK)
			return resultCode;
	}

	brMethodCallByNameWithArgs(caller, mexp->methodName, argps, mexp->arguments.size(), t);

	return EC_OK;
}

/*!
	\brief Evaluates a method call expression in steve.
*/

inline int stRealEvalMethodCall(stMethodExp *mexp, stRunInstance *target, stRunInstance *caller, brEval *t) {
	stKeywordEntry *keyEntry;
	stKeyword *key, *tmpkey;
	unsigned int n, m, argCount;
	int resultCode;

	if (!target->instance) {
		stEvalError(caller->instance->type->engine, EE_NULL_INSTANCE, "method \"%s\" called with uninitialized object", mexp->methodName);
		return EC_ERROR;
	} 
	if (target->instance->status != AS_ACTIVE) {
		stEvalError(caller->instance->type->engine, EE_FREED_INSTANCE, "method \"%s\" called with freed object (%p)", mexp->methodName, target->instance);
		return EC_ERROR;
	}

	// when we look up a method, we can remember its address for next time
	// if the same object type is being used.  otherwise, we need to look 
	// it up again.  there is a fair amount of computation here looking up
	// the method and argument order, so caching it is good.

	if (mexp->objectCache != target->type) {
		stMethod *method;
		stObject *newType;

		method = stFindInstanceMethodWithMinArgs(target->type, mexp->methodName, mexp->arguments.size(), &newType);

		if (!method) {
			// can't find the method!

			char *kstring = "keywords";

			if (mexp->arguments.size() == 1)
				kstring = "keyword";

			target->type = target->instance->type;
			stEvalError(target->type->engine, EE_UNKNOWN_METHOD, "object type \"%s\" does not respond to method \"%s\" with %d %s", target->type->name, mexp->methodName, mexp->arguments.size(), kstring);
			mexp->objectCache = NULL;
			return EC_ERROR;
		}

		// target->type = newType;
		// target->type = newType;

		mexp->method = method;
		mexp->objectCache = target->instance->type;
		mexp->objectTypeCache = target->type;

		argCount = mexp->arguments.size();

		// we have to lookup the order of the keywords.  initialize them to -1 first.

		for (n = 0; n < argCount; ++n)
			mexp->arguments[n]->position = -1;

		mexp->positionedArguments.clear();
		mexp->positionedArguments.reserve(mexp->method->keywords.size());

		for (n = 0; n < mexp->method->keywords.size(); ++n) {
			// look for the method's Nth keyword

			key = NULL;
			keyEntry = mexp->method->keywords[n];

			for (m = 0; m < argCount; ++m) {
				// go through all the arguments, checking what was passed in.

				tmpkey = mexp->arguments[m];
				if (tmpkey->position == -1 && !strcmp(tmpkey->word, keyEntry->keyword)) {
					tmpkey->position = n;
					key = tmpkey;
				}
			}

			if (!key && keyEntry->defaultKey)
				key = keyEntry->defaultKey;

			if (!key) {
				stEvalError(target->type->engine, EE_MISSING_KEYWORD, "Call to method %s of class %s missing keyword \"%s\"", mexp->method->name, target->type->name, keyEntry->keyword);
				mexp->objectCache = NULL;
				return EC_ERROR;
			}

			mexp->positionedArguments.push_back( key);
		}

		for (n = 0; n < argCount; ++n) {
			if (mexp->arguments[n]->position == -1) {
				tmpkey = mexp->arguments[n];

				stEvalError(target->type->engine, EE_UNKNOWN_KEYWORD, "unknown keyword \"%s\" in call to method \"%s\"", tmpkey->word, mexp->method->name);
				mexp->objectCache = NULL;
				return EC_ERROR;
			}
		}
	} else
		target->type = mexp->objectTypeCache;

	if (mexp->method->inlined) {
		// The method is inlined if it has no local variables, and no arguments

		slStack newStack, *oldStack = target->instance->gcStack;
		int resultCode;

		target->instance->gcStack = &newStack;

		resultCode = stEvalExpVector(&mexp->method->code, target, t);

		// unmark the return value -- we'll make it the current instance's problem

		if (t->type != AT_NULL)
			stGCUnmark(target->instance, t);

		// collect and reset the gcStack

		stGCCollectStack(target->instance->gcStack);
		target->instance->gcStack = oldStack;

		// mark the return value for the target's GC, if it exists

		if (caller->instance->gcStack)
			stGCMark(caller->instance, t);
		else {
			// stGCUnretainAndCollect(t);
		}

		if (resultCode == EC_STOP)
			return EC_OK;
	
		return resultCode;
	}

	// we don't want to reuse the same argps in the case of a recursive function 
	// so we create some local storage for them.

	brEval args[ mexp->method->keywords.size() ];
	brEval *argps[ mexp->method->keywords.size() ];

	for (n = 0; n < mexp->method->keywords.size(); ++n)
		argps[n] = &args[n];

	for (n = 0; n < mexp->method->keywords.size(); ++n) {
		key = mexp->positionedArguments[n];

		if (!key) {
			slMessage(DEBUG_ALL, "Missing keyword for method \"%s\"\n", mexp->method->name);
			return EC_ERROR;
		}

		// evaluate the key into the eval...

		resultCode = stExpEval3(key->value, caller, argps[n]);

		if (resultCode != EC_OK) {
			slMessage(DEBUG_ALL, "Error evaluating keyword \"%s\" for method \"%s\"\n", key->word, mexp->method->name);
			return EC_ERROR;
		}
	}

	if (!mexp->method)
		return EC_OK;

	resultCode = stCallMethod(caller, target, mexp->method, argps, mexp->method->keywords.size(), t);

	return resultCode;
}

/*!
	\brief Evaluates a while expression in steve.
*/

RTC_INLINE int stEvalWhile(stWhileExp *w, stRunInstance *i, brEval *result) {
	brEval condition, conditionExp;
	int r;

	result->type = AT_NULL;

	if ((r = stExpEval3(w->cond, i, &conditionExp)) != EC_OK ||
	    (r = stEvalTruth(&conditionExp, &condition, i)) != EC_OK)
		return r;

	while (BRINT(&condition)) {
		if ((r = stExpEval3(w->code, i, result)) != EC_OK ||
		    (r = stExpEval3(w->cond, i, &conditionExp)) != EC_OK ||
		    (r = stEvalTruth(&conditionExp, &condition, i)) != EC_OK)
			return r;
	}

	result->type = AT_NULL;

	return EC_OK;
}

RTC_INLINE int stEvalFor(stForExp *w, stRunInstance *i, brEval *result) {
	brEval condition, conditionExp;
	brEval assignment, iteration;
	int r;

	result->type = AT_NULL;

	if ((r = stExpEval3(w->assignment, i, &assignment)) != EC_OK ||
	    (r = stExpEval3(w->condition, i, &conditionExp)) != EC_OK ||
	    (r = stEvalTruth(&conditionExp, &condition, i)) != EC_OK)
		return r;

	while (BRINT(&condition)) {
		if ((w->code && (r = stExpEval3(w->code, i, result)) != EC_OK) ||
		    (r = stExpEval3(w->iteration, i, &iteration)) != EC_OK ||
		    (r = stExpEval3(w->condition, i, &conditionExp)) != EC_OK ||
		    (r = stEvalTruth(&conditionExp, &condition, i)) != EC_OK)
			return r;
	} 

	result->type = AT_NULL;

	return EC_OK;
}

RTC_INLINE int stEvalForeach(stForeachExp *w, stRunInstance *i, brEval *result) {
	brEval list;
	brEvalList *el;
	void *iterationPointer;
	int resultCode;

	stAssignExp *assignExp = w->assignment;

	result->type = AT_NULL;

	if (assignExp->local)
		iterationPointer = &i->instance->type->steveData->stack[assignExp->offset];
	else
		iterationPointer = &i->instance->variables[assignExp->offset];

	stExpEval3(w->list, i, &list);

	if (list.type != AT_LIST) {
		stEvalError(i->instance->type->engine, EE_TYPE, "expected type \"list\" in foreach evaluation");
		return EC_ERROR;
	}

	el = BRLIST(&list)->start;

	while (el) {
		brEval eval;

		if ((resultCode = brEvalCopy(&el->eval, &eval)) != EC_OK)
			return resultCode;

		if (assignExp->objectName && !assignExp->objectType) {
			brObject *object = brObjectFind(i->instance->type->engine, assignExp->objectName);

			if (object)
				assignExp->objectType = (stObject*)object->userData;
			else
				assignExp->objectType = NULL;
		}

		if ((resultCode = stSetVariable(iterationPointer, assignExp->assignType, assignExp->objectType, &eval, i)) != EC_OK)
			return resultCode;

		resultCode = stExpEval3(w->code, i, result); 
		if (resultCode != EC_OK)
			return resultCode;

		el = el->next;
	} 

	result->type = AT_NULL;

	return EC_OK;
}

RTC_INLINE int stEvalIf(stIfExp *w, stRunInstance *i, brEval *result) {
	brEval condition, conditionExp;
	int resultCode;

	if ((resultCode = stExpEval3(w->cond, i, &conditionExp)) != EC_OK ||
	    (resultCode = stEvalTruth(&conditionExp, &condition, i)) != EC_OK)
		return resultCode;

	if (BRINT(&condition)) {
		if (w->trueCode)
			return stExpEval3(w->trueCode, i, result); 
		else
			return EC_OK;
	} else {
		if (w->falseCode)
			return stExpEval3(w->falseCode, i, result);
		else
			return EC_OK;
	}
}

RTC_INLINE int stEvalListInsert(stListInsertExp *w, stRunInstance *i, brEval *result) {
	brEval pushEval, index;
	int resultCode;

	if ((resultCode = stExpEval3(w->listExp, i, result)) != EC_OK ||
	    (resultCode = stExpEval3(w->exp, i, &pushEval)) != EC_OK)
		return resultCode;

	if (result->type != AT_LIST) {
		stEvalError(i->instance->type->engine, EE_TYPE, "expected type \"list\" in \"push\" evaluation");
		return EC_ERROR;
	}
	if (!BRLIST(result)) {
		stEvalError(i->instance->type->engine, EE_INTERNAL, "attempt to insert value into uninitialized list\n");
		return EC_ERROR;
	}

	if (w->index) {
		resultCode = stExpEval3(w->index, i, &index);
		if (resultCode != EC_OK)
			return resultCode;
	} else {
		index.type = AT_INT;
		BRINT(&index) = BRLIST(result)->count;
	}

	brEvalListInsert(BRLIST(result), BRINT(&index), &pushEval);

	// the pushEval now has a home, so unmark it.

	stGCRetain(&pushEval);
	stGCUnmark(i->instance, &pushEval);

	return EC_OK;
}

RTC_INLINE int stEvalListRemove(stListRemoveExp *l, stRunInstance *i, brEval *result) {
	brEval listEval, index;
	int resultCode;

	resultCode = stExpEval3(l->listExp, i, &listEval);
	if (resultCode != EC_OK)
		return resultCode;

	if (listEval.type != AT_LIST) {
		stEvalError(i->instance->type->engine, EE_TYPE, "expected type \"list\" during \"pop\" evaluation"); 
		return EC_ERROR;
	}
	if(!BRLIST(&listEval)) {
		result->type = AT_NULL;
		return EC_OK;
	}

	if (l->index) {
		resultCode = stExpEval3(l->index, i, &index);
		if (resultCode != EC_OK)
			return resultCode;
	} else {
		index.type = AT_INT;
		BRINT(&index) = BRLIST(&listEval)->count - 1;
	}

	brEvalListRemove(BRLIST(&listEval), BRINT(&index), result);

	// this is unretained when we pop it from the list, but we cannot 
	// collect it immediately, since we're returning it.

	stGCUnretain(result);
	stGCMark(i->instance, result);

	return EC_OK;
}

RTC_INLINE int stEvalCopyList(stCopyListExp *l, stRunInstance *i, brEval *result) {
	brEval listEval;
	int resultCode;

	if ((resultCode = stExpEval3(l->expression, i, &listEval)) != EC_OK) return resultCode;

	if (listEval.type != AT_LIST) {
		stEvalError(i->instance->type->engine, EE_TYPE, "expected type \"list\" during \"copylist\" evaluation"); 
		return EC_ERROR;
	}

	result->type = AT_LIST;
	BRLIST(result) = brEvalListDeepCopyGC(BRLIST(&listEval));

	return EC_OK;
}

RTC_INLINE int stEvalAll(stAllExp *e, stRunInstance *i, brEval *result) {
	brEval instance;
	std::set< stInstance*, stInstanceCompare>::iterator ii;

	if (!e->object) {
		brObject *type = brObjectFind(i->instance->type->engine, e->name);

		if (!type) {
			stEvalError(i->instance->type->engine, EE_UNKNOWN_OBJECT, "unknown object type \"%s\" in \"all\" expression\n", e->name);
			return EC_ERROR;
		}

		e->object = (stObject *)type->userData;
	}

	BRLIST(result) = brEvalListNew();
	result->type = AT_LIST;

	stGCMark(i->instance, result);

	instance.type = AT_INSTANCE;

	for (ii = e->object->allInstances.begin(); ii != e->object->allInstances.end(); ii++ ) {
		stInstance *i = *ii;
		BRINSTANCE(&instance) = i->breveInstance;
		brEvalListInsert(BRLIST(result), 0, &instance);

		// adding it to the list -- retain it.  it will be unretained later.
		stGCRetain(&instance);
	}

	return EC_OK;
}

RTC_INLINE int stEvalSort(stSortExp *w, stRunInstance *i, brEval *result) {
	brEval listEval;
	stMethod *method;
	int resultCode;

	resultCode = stExpEval3(w->listExp, i, &listEval);
	if (resultCode != EC_OK)
		return resultCode;

	method = stFindInstanceMethod(i->instance->type, w->methodName, 2, NULL);
	
	if (!method) {
		stEvalError(i->instance->type->engine, EE_UNKNOWN_METHOD, "object type \"%s\" does not respond to method \"%s\"", i->instance->type->name, w->methodName); 
		return EC_ERROR;
	}

	resultCode = stSortEvalList(BRLIST(&listEval), i->instance, method); 

	return resultCode;
}

RTC_INLINE int stEvalListIndexPointer(stListIndexExp *l, stRunInstance *i, void **pointer, int *type) {
	brEval list, index;
	brEval *result;
	int resultCode;

	if ((resultCode = stExpEval3(l->listExp, i, &list)) != EC_OK ||
	    (resultCode = stExpEval3(l->indexExp, i, &index)) != EC_OK)
		return resultCode;

	if (list.type != AT_LIST)
		return EC_ERROR;

	if (index.type != AT_INT && stToInt(&index, &index, i) == EC_ERROR) {
		stEvalError(i->instance->type->engine, EE_TYPE, "expected type \"int\" in list element evaluation (index component)");
		return EC_ERROR;
	}

	if (stDoEvalListIndexPointer(BRLIST(&list), BRINT(&index), &result)) {
		stEvalError(i->instance->type->engine, EE_BOUNDS, "list index \"%d\" out of bounds", BRINT(&index));
		return EC_ERROR;
	}

	*pointer = &result->values;
	*type = result->type;

	return EC_OK;
}

RTC_INLINE int stEvalListIndex(stListIndexExp *l, stRunInstance *i, brEval *t) {
	brEval list, index;
	int resultCode;

	if ((resultCode = stExpEval3(l->listExp, i, &list)) != EC_OK ||
	    (resultCode = stExpEval3(l->indexExp, i, &index)) != EC_OK)
		return resultCode;

	if (list.type == AT_LIST) {
		if (index.type != AT_INT && stToInt(&index, &index, i) == EC_ERROR) {
			stEvalError(i->instance->type->engine, EE_TYPE, "expected type \"int\" in list index");
			return EC_ERROR;
		}
		if (stDoEvalListIndex(BRLIST(&list), BRINT(&index), t)) {
			stEvalError(i->instance->type->engine, EE_BOUNDS, "list index \"%d\" out of bounds", BRINT(&index));
			return EC_ERROR;
		}
	} else if (list.type == AT_HASH) {
		brEvalHashLookup(BRHASH(&list), &index, t);
	} else if (list.type == AT_STRING) {
		char *newstring, *oldstring;

		oldstring = BRSTRING(&list);

		if ((int)strlen(oldstring) <= BRINT(&index)) {
			stEvalError(i->instance->type->engine, EE_BOUNDS, "string index \"%d\" out of bounds", BRINT(&index));
			return EC_ERROR;
		}

		newstring = (char *)slMalloc(2);

		newstring[0] = oldstring[BRINT(&index)];
		newstring[1] = '\0';

		BRSTRING(t) = newstring;
		t->type = AT_STRING;
	} else {
		stEvalError(i->instance->type->engine, EE_TYPE, "expected type \"list\" or \"hash\" in lookup expression");
		return EC_ERROR;
	}

	return EC_OK;
}

RTC_INLINE int stEvalListIndexAssign(stListIndexAssignExp *l, stRunInstance *i, brEval *t) {
	brEval list, index;
	int resultCode;

	if ((resultCode = stExpEval3(l->listExp, i, &list)) != EC_OK ||
	    (resultCode = stExpEval3(l->indexExp, i, &index)) != EC_OK ||
	    (resultCode = stExpEval3(l->assignment, i, t)) != EC_OK)
		return resultCode;

	if (list.type == AT_LIST) {
		if (index.type != AT_INT && stToInt(&index, &index, i) == EC_ERROR) {
			stEvalError(i->instance->type->engine, EE_TYPE, "expected type \"int\" for list element assignment index");
			return EC_ERROR;
		}

		if(stDoEvalListIndexAssign(BRLIST(&list), BRINT(&index), t, i)) {
			stEvalError(i->instance->type->engine, EE_BOUNDS, "list index \"%d\" out of bounds", BRINT(&index));
			return EC_ERROR;
		}
	} else if (list.type == AT_HASH) {
		brEval old;

		brEvalHashStore(BRHASH(&list), &index, t, &old);

		stGCUnretainAndCollect(&old);
	} else if (list.type == AT_STRING) {
		char **stringptr, *newstring, *oldstring, *substring;
		unsigned int n;
		int type;

		n = BRINT(&index);

		resultCode = stPointerForExp(l->listExp, i, (void **)&stringptr, &type);

		oldstring = *stringptr;

		if (!oldstring || n < 0 || n > strlen(oldstring) + 1) {
			stEvalError(i->instance->type->engine, EE_BOUNDS, "string index \"%d\" out of bounds", BRINT(&index));
			return EC_ERROR;
		}
		if (t->type != AT_STRING) {
			stEvalError(i->instance->type->engine, EE_TYPE, "expected type \"string\" for string index assignment", BRINT(&index));
			return EC_ERROR;
		}

		substring = BRSTRING(t);

		newstring = (char *)slMalloc(strlen(oldstring) + strlen(substring) + 1);

		strncpy(newstring, oldstring, n);
		strcpy(&newstring[n], substring);

		if (n != strlen(oldstring)) {
			strcpy(&newstring[n + strlen(substring)], &oldstring[n + 1]);
			newstring[strlen(oldstring) + strlen(substring) - 1] = 0;
		} else
			newstring[strlen(oldstring) + strlen(substring)] = 0;

		BRSTRING(t) = *stringptr;
		stGCUnmark(i->instance, t);

		if (*stringptr)
			slFree(*stringptr);
		*stringptr = newstring;

		BRSTRING(t) = slStrdup(newstring);
	} else {
		stEvalError(i->instance->type->engine, EE_TYPE, "expected type \"list\" or \"hash\" in index assignment");
		return EC_ERROR;
	}

	// since the variable now has a home, we have to unmark it.

	stGCUnmark(i->instance, t);
	stGCRetain(t);

	return EC_OK;
}

RTC_INLINE int stEvalPrint(stPrintExp *exp, stRunInstance *i, brEval *t) {
	brEval arg;
	unsigned int n;
	int resultCode;

	t->type = AT_NULL;

	for (n = 0; n < exp->expressions.size(); ++n) {
		resultCode = stExpEval3(exp->expressions[n], i, &arg);
		if (resultCode != EC_OK)
			return resultCode;

		resultCode = stPrintEvaluation(&arg, i);
		if (resultCode != EC_OK)
			return resultCode;

		if (n != exp->expressions.size() - 1)
			slMessage(NORMAL_OUTPUT, " ");

		// stGCCollect(&arg);
	}

	if (exp->newline)
		slMessage(NORMAL_OUTPUT, "\n");

	return EC_OK;
}

RTC_INLINE int stEvalVectorElementExp(stVectorElementExp *s, stRunInstance *i, brEval *result) {
	int resultCode;

	resultCode = stExpEval3(s->exp, i, result);
	if (resultCode != EC_OK)
		return EC_ERROR;

	if (result->type != AT_VECTOR) {
		stEvalError(i->instance->type->engine, EE_TYPE, "expected type \"vector\" in vector element evaluation");
		return EC_ERROR;
	}

	result->type = AT_DOUBLE;

	switch(s->element) {
		case VE_X:
			BRDOUBLE(result) = BRVECTOR(result).x;
			return EC_OK;
			break;
		case VE_Y:
			BRDOUBLE(result) = BRVECTOR(result).y;
			return EC_OK;
			break;
		case VE_Z:
			BRDOUBLE(result) = BRVECTOR(result).z;
			return EC_OK;
			break;
		default:
			stEvalError(i->instance->type->engine, EE_INTERNAL, "unknown vector element (%d) in stEvalVectorElementExp", s->element);
			return EC_ERROR;
			break;
	}

	return EC_OK;
}

RTC_INLINE int stEvalVectorElementAssignExp(stVectorElementAssignExp *s, stRunInstance *i, brEval *result) {
	slVector *vector;
	int resultCode;
	int type = AT_NULL;

	resultCode = stPointerForExp(s->exp, i, (void **)&vector, &type);

	if (resultCode != EC_OK)
		return EC_ERROR;

	if(type != AT_VECTOR) {
		stEvalError(i->instance->type->engine, EE_TYPE, "expected vector expression in vector element assign");
		return EC_ERROR;
	}

	resultCode = stExpEval3(s->assignExp, i, result);
	if(resultCode != EC_OK) return EC_ERROR;
	resultCode = stToDouble(result, result, i); 
	if(resultCode != EC_OK) return EC_ERROR;

	switch(s->element) {
		case VE_X:
			vector->x = BRDOUBLE(result);
			break;
		case VE_Y:
			vector->y = BRDOUBLE(result);
			break;
		case VE_Z:
			vector->z = BRDOUBLE(result);
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

RTC_INLINE int stEvalCallFunc(stCCallExp *c, stRunInstance *i, brEval *result) {
	brEval e[ST_CMAX_ARGS];
	int n, resultCode;

	for(n = 0; n < c->function->nargs; ++n) {
		resultCode = stExpEval3(c->arguments[n], i, &e[n]);
		if (resultCode != EC_OK)
			return resultCode;

		/* if the types don't match, try to convert them */

		if (e[n].type != c->function->argtypes[n])
			resultCode = stToType(&e[n], c->function->argtypes[n], &e[n], i);

		if(c->function->argtypes[n] == AT_POINTER && BRPOINTER(&e[n]) == NULL) {
			stEvalError(i->instance->type->engine, EE_TYPE, "NULL pointer passed as argument %d to internal function %s", n, c->function->name);
			return EC_ERROR;
		}

		if (resultCode != EC_OK) {
			stEvalError(i->instance->type->engine, EE_TYPE, "expected type \"%s\" for argument #%d to internal method \"%s\", got type \"%s\"", slAtomicTypeStrings[c->function->argtypes[n]], n + 1, c->function->name, slAtomicTypeStrings[e[n].type]);
			return resultCode;
		}
	}

	memset(result, 0, sizeof(brEval));

#ifdef MULTITHREAD
	pthread_mutex_lock(&(i->instance->lock));
#endif

	try {
		resultCode = c->function->call(e, result, i->instance->breveInstance);
	} catch(slException &error) {
		stEvalError(i->instance->type->engine, EE_SIMULATION, "an error occurred executing the internal function \"%s\": %s", c->function->name, error._message.c_str());

		return EC_ERROR;
	}

#ifdef MULTITHREAD
	pthread_mutex_unlock(&(i->instance->lock));
#endif

	// special case--if the define type is undefined, any type may be returned.

	if(resultCode != EC_OK) result->type = AT_NULL;
	else if(c->function->rtype != AT_UNDEFINED) result->type = c->function->rtype;

	stGCMark(i->instance, result);

	return resultCode;
}

/*!
	\brief Gives the pointer for an array-load expression in steve.
*/

RTC_INLINE int stEvalArrayIndexPointer(stArrayIndexExp *a, stRunInstance *i, void **pointer, int *type) {
	brEval indexExp;
	int r;
	int offset;

	r = stExpEval3(a->index, i, &indexExp);

	*type = a->loadType;

	if (r != EC_OK) {
		slMessage(DEBUG_ALL, "Error evaluating index of array expression.\n");
		return r;
	}
	if(indexExp.type != AT_INT && !(r = stToInt(&indexExp, &indexExp, i))) {
		slMessage(DEBUG_ALL, "Array index must be of type \"int\"\n");
		return r;
	}

	offset = BRINT(&indexExp);

	if (offset >= a->maxIndex || offset < 0) {
		stEvalError(i->instance->type->engine, EE_BOUNDS, "array index \"%d\" out of bounds", BRINT(&indexExp));
		return EC_ERROR;
	}

	if (a->local)
		*pointer = &i->instance->type->steveData->stack[a->offset];
	else
		*pointer = &i->instance->variables[a->offset];

	*pointer = (char *)*pointer + a->typeSize * offset;

	return EC_OK;
}

RTC_INLINE int stEvalArrayIndex(stArrayIndexExp *a, stRunInstance *i, brEval *result) {
	void *pointer;
	int r, type;

	r = stEvalArrayIndexPointer(a, i, &pointer, &type);

	if (r != EC_OK)
		return r;

	return stLoadVariable(pointer, a->loadType, result, i);
}

RTC_INLINE int stEvalArrayIndexAssign(stArrayIndexAssignExp *a, stRunInstance *i, brEval *rvalue) {
	brEval indexExp;
	char *pointer;
	int r;

	r = stExpEval3(a->index, i, &indexExp);
	if (r != EC_OK) {
		slMessage(DEBUG_ALL, "Error evaluating index of array expression.\n");
		return r;
	}

	r =  stExpEval3(a->rvalue, i, rvalue);
	if (r != EC_OK) {
		slMessage(DEBUG_ALL, "Error evaluating right hand side of assignment expression.\n");
		return r;
	}

	if(indexExp.type != AT_INT && !(r = stToInt(&indexExp, &indexExp, i))) {
		slMessage(DEBUG_ALL, "Array index must be of type \"int\".\n");
		return r;
	}

	if (BRINT(&indexExp) >= a->maxIndex || BRINT(&indexExp) < 0) {
		stEvalError(i->instance->type->engine, EE_BOUNDS, "array index \"%d\" out of bounds", BRINT(&indexExp));
		return EC_ERROR;
	}

	if (a->local)
		pointer = &i->instance->type->steveData->stack[a->offset];
	else
		pointer = &i->instance->variables[a->offset];

	pointer += a->typeSize * BRINT(&indexExp);

	return stSetVariable(pointer, a->assignType, NULL, rvalue, i);
}

RTC_INLINE int stEvalAssignment(stAssignExp *a, stRunInstance *i, brEval *t) {
	char *pointer;
	int resultCode;

	resultCode = stExpEval3(a->rvalue, i, t);

	if (resultCode != EC_OK) {
		slMessage(DEBUG_ALL, "Error evaluating right hand side of assignment expression.\n");
		return resultCode;
	}

	if (a->local)
		pointer = &i->instance->type->steveData->stack[a->offset];
	else
		pointer = &i->instance->variables[a->offset];

	if (a->objectName && !a->objectType) {
		brObject *object = brObjectFind(i->instance->type->engine, a->objectName);

		if (object)
			a->objectType = (stObject *)object->userData;
	}

	resultCode = stSetVariable(pointer, a->assignType, a->objectType, t, i);

	return resultCode;
}

RTC_INLINE int stEvalUnaryExp(stUnaryExp *b, stRunInstance *i, brEval *result) {
	brEval truth;
	char *str;
	int resultCode;

	/* This particular code is not very robust. */

	resultCode = stExpEval3(b->expression, i, result);

	if (resultCode != EC_OK)
		return resultCode;

	if (b->op == UT_NOT) {
		stEvalTruth(result, &truth, i);
		result->type = truth.type;
		BRINT(result) = !BRINT(&truth);

		return EC_OK;
	}

	// if it's a string, switch to a number before continuing.

	if (result->type == AT_STRING) {
		str = BRSTRING(result);
		if ((resultCode = stToDouble(result, result, i)) != EC_OK)
			return resultCode;
	}

	if(result->type == AT_LIST) {
		switch(b->op) {
			case UT_MINUS:
				stEvalError(i->instance->type->engine, EE_TYPE, "type \"list\" unexpected during evaluation of unary operator \"-\"");
				return EC_ERROR;
				break;
		}
	}

	if(result->type == AT_VECTOR) {
		switch(b->op) {
			case UT_MINUS:
				slVectorMul(&BRVECTOR(result), -1, &BRVECTOR(result));
				return EC_OK;
				break;
			default:
				stEvalError(i->instance->type->engine, EE_INTERNAL, "unknown unary operator (%d) in stEvalUnaryExp", b->op);
				return EC_ERROR;
				break;
		}
	}

	if(result->type == AT_INT) {
		switch(b->op) {
			case UT_MINUS:
				BRINT(result) *= -1;
				return EC_OK;
				break;
			default:
				stEvalError(i->instance->type->engine, EE_INTERNAL, "unknown unary operator (%d) in stEvalUnaryExp", b->op);
				return EC_ERROR;
				break;
		}
	}

	if(result->type == AT_DOUBLE) {
		switch(b->op) {
			case UT_MINUS:
				BRDOUBLE(result) *= -1;
				return EC_OK;
				break;
			default:
				stEvalError(i->instance->type->engine, EE_INTERNAL, "unknown unary operator (%d) in stEvalUnaryExp", b->type);
				return EC_ERROR;
				break;
		}
	}

	if(result->type == AT_INSTANCE) {
		switch(b->op) {
			case UT_MINUS:
				stEvalError(i->instance->type->engine, EE_TYPE, "type \"object\" unexpected during evaluation of unary operator \"-\"");
				return EC_ERROR;
				break;
			default:
				stEvalError(i->instance->type->engine, EE_INTERNAL, "unknown unary operator (%d) in stEvalUnaryExp", b->op);
				return EC_ERROR;
				break;
		}
	}

	if(result->type == AT_POINTER) {
		switch(b->op) {
			case UT_MINUS:
				stEvalError(i->instance->type->engine, EE_TYPE, "type \"pointer\" unexpected during evaluation of unary operator \"-\"");
				return EC_ERROR;
				break;
			default:
				stEvalError(i->instance->type->engine, EE_INTERNAL, "unknown unary operator (%d) in stEvalUnaryExp", b->op);
				return EC_ERROR;
				break;
		}
	}

	stEvalError(i->instance->type->engine, EE_INTERNAL, "unknown expression type (%d) in stEvalUnaryExp", result->type);

	return EC_ERROR;
}

RTC_INLINE int stEvalBinaryStringExp(char op, brEval *l, brEval *r, brEval *result, stRunInstance *i) {
	char *sl, *sr;

	sl = BRSTRING(l);
	sr = BRSTRING(r);

	result->type = AT_INT;

	switch(op) {
		case BT_EQ:
			BRINT(result) = !strcmp(sl, sr);
			break;
		case BT_NE:
			BRINT(result) = strcmp(sl, sr);
			break;
		default:
			stEvalError(i->instance->type->engine, EE_INTERNAL, "unknown string operator (%d) in stEvalBinaryStringExp", op);
			return EC_ERROR;
	}

	return EC_OK;
}

RTC_INLINE int stEvalBinaryMatrixExp(char op, brEval *l, brEval *r, brEval *t, stRunInstance *i) {
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

RTC_INLINE int stEvalBinaryVectorExp(char op, brEval *l, brEval *r, brEval *t, stRunInstance *i) {
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

RTC_INLINE int stEvalBinaryDoubleExp(char op, brEval *l, brEval *r, brEval *t, stRunInstance *i) {
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

RTC_INLINE int stEvalBinaryIntExp(char op, brEval *l, brEval *r, brEval *t, stRunInstance *i) {
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
			BRINT(t) = (int)pow(BRINT(l), BRINT(r));
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

RTC_INLINE int stEvalBinaryExp(stBinaryExp *b, stRunInstance *i, brEval *result) {
	brEval tl, tr;
	brEval truthResult;
	int c;

	if(b->op == BT_LAND) {
		result->type = AT_INT;
		BRINT(result) = 1;

		/* LAND is treated as a short circuit op, do not eval both sides */

		if((c = stExpEval3(b->left, i, &tl)) != EC_OK) return c;
		if((c = stEvalTruth(&tl, &truthResult, i)) != EC_OK) return c;

		if(!BRINT(&truthResult)) {
			/* short circuit -- we know that the left is false, so return */

			BRINT(result) = 0;
			return EC_OK;
		}

		if((c = stExpEval3(b->right, i, &tr)) != EC_OK) return c;
		if((c = stEvalTruth(&tr, &truthResult, i)) != EC_OK) return c;

		if(!BRINT(&truthResult)) BRINT(result) = 0;

		return EC_OK;
	}

	if(b->op == BT_LOR) {
		result->type = AT_INT;
		BRINT(result) = 0;

		/* LOR is treated as a short circuit op, do not eval both sides */

		if((c =  stExpEval3(b->left, i, &tl)) != EC_OK) return c;
		if((c = stEvalTruth(&tl, &truthResult, i)) != EC_OK) return c;

		if(BRINT(&truthResult)) {
			/* short circuit -- we know that the left is true, so return */

			BRINT(result) = 1;
			return EC_OK;
		}

		if((c = stExpEval3(b->right, i, &tr)) != EC_OK) return c;
		if((c = stEvalTruth(&tr, &truthResult, i)) != EC_OK) return c;

		if(BRINT(&truthResult)) BRINT(result) = 1;

		return EC_OK;
	}

	if((c =  stExpEval3(b->left, i, &tl)) != EC_OK) return c;
	if((c =  stExpEval3(b->right, i, &tr)) != EC_OK) return c;

	return stEvalBinaryExpWithEvals(i, b->op, &tl, &tr, result);
}

RTC_INLINE int stEvalBinaryExpWithEvals(stRunInstance *i, unsigned char op, brEval *tl, brEval *tr, brEval *result) {
	int c;

	if(tl->type == AT_INT && tr->type == AT_INT) {
		result->type = AT_INT;
 		return stEvalBinaryIntExp(op, tl, tr, result, i);
 	}

	/* if either expression is a matrix... */

	if(tl->type == AT_MATRIX || tr->type == AT_MATRIX) {
		return stEvalBinaryMatrixExp(op, tl, tr, result, i);
	}

	/* if either expression is a vector... */

	if(tl->type == AT_VECTOR || tr->type == AT_VECTOR) {
		result->type = AT_VECTOR;
		return stEvalBinaryVectorExp(op, tl, tr, result, i);
	}

	/* if we have two strings and they're testing for equality or inequality */
	/* we do a string compare--otherwise we'll convert to doubles and handle */
	/* the expression that way */

	if(tr->type == AT_STRING && tl->type == AT_STRING && (op == BT_EQ || op == BT_NE)) {
		return stEvalBinaryStringExp(op, tl, tr, result, i);
	}

	if(tr->type == AT_LIST && tl->type == AT_LIST && (op == BT_EQ || op == BT_NE)) {
		return stEvalBinaryEvalListExp(op, tl, tr, result, i);
	}

	if(tr->type == AT_STRING) if((c = stToDouble(tr, tr, i)) != EC_OK) return c;
	if(tl->type == AT_STRING) if((c = stToDouble(tl, tl, i)) != EC_OK) return c;

	if(tr->type == AT_LIST) if((c = stToInt(tr, tr, i)) != EC_OK) return c;
	if(tl->type == AT_LIST) if((c = stToInt(tl, tl, i)) != EC_OK) return c;

	if(tl->type == AT_DOUBLE || tr->type == AT_DOUBLE) {
		result->type = AT_DOUBLE;
		return stEvalBinaryDoubleExp(op, tl, tr, result, i);
	}

	result->type = AT_INT;
	return stEvalBinaryIntExp(op, tl, tr, result, i);
}

RTC_INLINE int stEvalRandExp(stRandomExp *r, stRunInstance *i, brEval *result) {
	stExpEval3(r->expression, i, result);

	switch(result->type) {
		case AT_INT:
			if(BRINT(result) < 0) BRINT(result) = 0;
			else BRINT(result) = (random() % (BRINT(result) + 1));
			return EC_OK;
			break;
		case AT_DOUBLE:
			BRDOUBLE(result) = slRandomDouble() * BRDOUBLE(result);
			return EC_OK;
			break;
		case AT_VECTOR:
			BRVECTOR(result).x = slRandomDouble() * BRVECTOR(result).x;
			BRVECTOR(result).y = slRandomDouble() * BRVECTOR(result).y;
			BRVECTOR(result).z = slRandomDouble() * BRVECTOR(result).z;
			return EC_OK;
			break;
		default:
			stEvalError(i->instance->type->engine, EE_TYPE, "expected type \"int\", \"double\" or \"vector\" in evaluation of \"random\"");
			return EC_ERROR;
	}

	return EC_OK;
}

/* turn a vector stExp tree into a real vector */

RTC_INLINE int stEvalVectorExp(stVectorExp *v, stRunInstance *i, brEval *result) {
	brEval tx, ty, tz;
	int resultCode;

	result->type = AT_VECTOR;

	stExpEval3(v->_x, i, &tx);
	stExpEval3(v->_y, i, &ty);
	stExpEval3(v->_z, i, &tz);

	if(tx.type != AT_DOUBLE) {
		if((resultCode = stToDouble(&tx, &tx, i)) != EC_OK) return resultCode;
	}

	if(ty.type != AT_DOUBLE) {
		if((resultCode = stToDouble(&ty, &ty, i)) != EC_OK) return resultCode;
	}

	if(tz.type != AT_DOUBLE) {
		if((resultCode = stToDouble(&tz, &tz, i)) != EC_OK) return resultCode;
	}

	BRVECTOR(result).x = BRDOUBLE(&tx);
	BRVECTOR(result).y = BRDOUBLE(&ty);
	BRVECTOR(result).z = BRDOUBLE(&tz);

	return EC_OK;
}

RTC_INLINE int stEvalMatrixExp(stMatrixExp *v, stRunInstance *i, brEval *result) {
	int n, resultCode;
	brEval t;

	result->type = AT_MATRIX;

	for(n=0;n<9;n++) {
		stExpEval3(v->expressions[n], i, &t);
	
		if(t.type != AT_DOUBLE)
			if((resultCode = stToDouble(&t, &t, i)) != EC_OK) return resultCode;

		BRMATRIX(result)[n / 3][n % 3] = BRDOUBLE(&t);
	}

	return EC_OK;
}

/*!
	\brief Calls a method once the method has been found and the arguments have been processed.
*/

int stCallMethod(stRunInstance *caller, stRunInstance *target, stMethod *method, brEval **args, int argcount, brEval *result) {
	int n;
	stKeywordEntry *keyEntry;
	stStackRecord record;

	char *savedStackPointer, *newStStack;
	int resultCode;

	stSteveData *steveData;

	if(target->instance->status != AS_ACTIVE) {
		stEvalError(target->instance->type->engine, EE_FREED_INSTANCE, "method \"%s\" being called with freed instance of class \"%s\" (%p)", method->name, target->instance->type->name, target->instance);
		return EC_ERROR;
	}

	steveData = target->instance->type->steveData;

	savedStackPointer = steveData->stack;

	// if there is no current stackpointer (outermost frame), start at the end of the stack 

	if(savedStackPointer == NULL) steveData->stack = &steveData->stackBase[ST_STACK_SIZE];

	// step down the stack enough to make room for the current calling method

	newStStack = steveData->stack - method->stackOffset;

	if(newStStack < steveData->stackBase) {
		slMessage(DEBUG_ALL, "Stack overflow in class \"%s\" method \"%s\"\n", target->instance->type->name, method->name);
		return EC_ERROR;
	}

	memset(newStStack, 0, method->stackOffset);

	// go through the arguments and place them on the stack.

	for(n=0;n<argcount;n++) {
		keyEntry = method->keywords[n];

		if(keyEntry->var->type->objectName && !keyEntry->var->type->objectType) {
			brObject *o = brObjectFind(target->instance->type->engine, keyEntry->var->type->objectName);

			if(o) keyEntry->var->type->objectType = (stObject*)o->userData;
		}

		resultCode = stSetVariable(&newStStack[keyEntry->var->offset], keyEntry->var->type->type, keyEntry->var->type->objectType, args[n], caller);

		// most things will be retained by the stSetVariable so the collect does 
		// nothing.  the exception is strings, which are copied -- we need to free
		// the original now.

		// stGCCollect(args[n]);
		// BRPOINTER(args[n]) = 0xdeadbeef;

		if(resultCode != EC_OK) {
			slMessage(DEBUG_ALL, "Error evaluating keyword \"%s\" for method \"%s\"\n", keyEntry->keyword, method->name);
			return resultCode;
		}
	}

	record.instance = target->instance;
	record.method = method;
	record.previousStackRecord = steveData->stackRecord;
	steveData->stackRecord = &record;
	record.gcStack = target->instance->gcStack;

	slStack newStack;

	target->instance->gcStack = &newStack;

	// prepare for the actual method call 

	result->type = AT_NULL;
	steveData->stack = newStStack;

	resultCode = stEvalExpVector(&method->code, target, result);

	// we don't want the return value released, so we'll retain it.
	// this will keep it alive through the releasing stage.
	
	if(result->type != AT_NULL) {
		stGCUnmark(target->instance, result);
		stGCRetain(result);
	}

	// unretain the local variables

	std::vector< stVar* >::iterator vi;
	
	for(vi = method->variables.begin(); vi != method->variables.end(); vi++ ) {
		brEval e;

		if((*vi)->type->type != AT_STRING || *(void**)&newStStack[(*vi)->offset] != result->values.stringValue) stGCUnretainAndCollectPointer(*(void**)&newStStack[(*vi)->offset], (*vi)->type->type);

		e.values.pointerValue = *(void**)&newStStack[(*vi)->offset];
		e.type = (*vi)->type->type;

		stGCUnmark(target->instance, &e);
	}

	if(resultCode == EC_STOP) resultCode = EC_OK;

	// collect and destroy the current gcStack.

	stGCCollectStack(target->instance->gcStack);

	// reset the previous gcStack.

	target->instance->gcStack = record.gcStack;

	// unretain the input arguments

	std::vector< stKeywordEntry* >::iterator ki;

	for(ki = method->keywords.begin(); ki != method->keywords.end(); ki++ ) {
		keyEntry = *ki;

		stGCUnretainAndCollectPointer(*(void**)&newStStack[keyEntry->var->offset], keyEntry->var->type->type);
	}

	// remember when we retained the return value before?
	// unretain it, and make it the caller's problem.

	if(caller && caller->instance->gcStack) {
		stGCUnretain(result);
		stGCMark(caller->instance, result);
	} else {
		if (result->type != AT_NULL) stGCUnretainAndCollect(result);
	}

	// restore the previous stack and stack records

	steveData->stack = savedStackPointer;
	steveData->stackRecord = record.previousStackRecord;

	return resultCode;
}

int stCallMethodByName(stRunInstance *target, char *name, brEval *resultCode) {
	return stCallMethodByNameWithArgs(target, name, NULL, 0, resultCode);
}

int stCallMethodByNameWithArgs(stRunInstance *target, char *name, brEval **args, int argcount, brEval *resultCode) {
	stMethod *method;
	stRunInstance ri;

	ri.instance = target->instance;
	ri.type = ri.instance->type;

	method = stFindInstanceMethod(target->instance->type, name, argcount, NULL);
	resultCode->type = AT_NULL;

	if(!method) {
		stEvalError(target->instance->type->engine, EE_UNKNOWN_METHOD, "object type \"%s\" does not respond to method \"%s\"", target->instance->type->name, name); 
		return EC_ERROR;
	}

	return stCallMethod(NULL, &ri, method, args, argcount, resultCode);
}

void stStackTrace(stSteveData *d) {
	stStackRecord *r = d->stackRecord;

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

	slMessage(NORMAL_OUTPUT, "%s", evalString);

	slFree(evalString);

	return EC_OK;
}

/*!
	\brief Evaluates a new instance expression in steve.

	Produces either a single object or a list.
*/

RTC_INLINE int stEvalNewInstance(stInstanceExp *ie, stRunInstance *i, brEval *t) {
	brObject *object;
	brEvalListHead *list = NULL;
	brEval listItem, count;
	int n;

	object = brObjectFind(i->instance->type->engine, ie->name);

	if(!object) {
		stEvalError(i->instance->type->engine, EE_UNKNOWN_OBJECT, "unknown object type \"%s\" during new instance evaluation", ie->name);
		return EC_ERROR;
	}

	// how many instances are we creating? 

	 stExpEval3(ie->count, i, &count);

	if(count.type != AT_INT) {
		stEvalError(i->instance->type->engine, EE_TYPE, "expected integer count for \"new\" expression");
		return EC_ERROR;
	}

	if(BRINT(&count) == 1) {
		t->type = AT_INSTANCE;

		BRINSTANCE(t) = stInstanceCreateAndRegister(i->instance->type->steveData, i->instance->type->engine, object);

		if(BRINSTANCE(t) == NULL) return EC_ERROR;
	} else {
		listItem.type = AT_INSTANCE;
		list = brEvalListNew();

		for(n=0;n<BRINT(&count);n++) {
			BRINSTANCE(&listItem) = stInstanceCreateAndRegister(i->instance->type->steveData, i->instance->type->engine, object);

			if(BRINSTANCE(&listItem) == NULL) return EC_ERROR;

			brEvalListInsert(list, 0, &listItem);
		}

		t->type = AT_LIST;
		BRLIST(t) = list;
	}

	stGCMark(i->instance, t);

	return EC_OK;
}

int stExpEval3(stExp *s, stRunInstance *i, brEval *result) {
	if (s->block)
		return s->block->calls.stRtcEval3(s, i, result);
	else
		return stExpEval(s, i, result, NULL);
}

int stExpEval(stExp *s, stRunInstance *i, brEval *result, stObject **tClass) {
	brEval t;
	int resultCode = EC_OK;

	if (tClass) *tClass = NULL;

	result->type = AT_NULL;

	if (!i) {
		stEvalError(i->instance->type->engine, EE_INTERNAL, "Expression evaluated with uninitialized instance");
		return EC_ERROR;
	}

	// check active instance status here rather than burden stExpEval3 with a test every time
	if (i->instance->status == AS_ACTIVE) {
		if (!s)
			return EC_OK;

		if (s->debug == 1)
			slDebug("debug called from within steve evaluation\n");

		if (s->block)
			return s->block->calls.stRtcEval3(s, i, result);
	} else {
		// we don't allow execution with freed instances, unless it's
		// a return value, in which case it's okay.

		if (s->type == ET_RETURN) {
			result->type = AT_NULL;

			resultCode = stExpEval3(s, i, result);

			resultCode = EC_STOP;
		}

		stEvalError(i->instance->type->engine, EE_FREED_INSTANCE, "Expression evaluated using freed instance");
		return EC_ERROR;
	}

	if (s->debug == 1)
		slDebug("debug called from within steve evaluation\n");

	if (s->block)
		return s->block->calls.stRtcEval3(s, i, result);
	
	switch(s->type) {
	case ET_LOAD:
		{
			stLoadExp *e = (stLoadExp *)s;

			switch(e->loadType) {
			case AT_INT:
				resultCode = EVAL_RTC_CALL_3(s, stEvalLoadInt, e, i, result);
				break;
			case AT_DOUBLE:
				resultCode = EVAL_RTC_CALL_3(s, stEvalLoadDouble, e, i, result);
				break;
			case AT_POINTER:
			case AT_INSTANCE:
			case AT_DATA:
				resultCode = EVAL_RTC_CALL_3(s, stEvalLoadIndirect, e, i, result);
				break;
			case AT_VECTOR:
				resultCode = EVAL_RTC_CALL_3(s, stEvalLoadVector, e, i, result);
				break;
			case AT_LIST:
				resultCode = EVAL_RTC_CALL_3(s, stEvalLoadList, e, i, result);
				break;
			case AT_HASH:
				resultCode = EVAL_RTC_CALL_3(s, stEvalLoadHash, e, i, result);
				break;
			case AT_STRING:
				resultCode = EVAL_RTC_CALL_3(s, stEvalLoadString, e, i, result);
				break;
			case AT_MATRIX:
				resultCode = EVAL_RTC_CALL_3(s, stEvalLoadMatrix, e, i, result);
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
		}
		break;
	case ET_ASSIGN:
		resultCode = EVAL_RTC_CALL_3(s, stEvalAssignment, (stAssignExp *)s, i, result);
		break;
	case ET_BINARY:
		resultCode = EVAL_RTC_CALL_3(s, stEvalBinaryExp, (stBinaryExp *)s, i, result);
		break;
	case ET_METHOD:
		resultCode = EVAL_RTC_CALL_3(s, stEvalMethodCall, (stMethodExp *)s, i, result);
		break;
	case ET_FUNC:
		resultCode = EVAL_RTC_CALL_3(s, stEvalCallFunc, (stCCallExp *)s, i, result);
		break;
	case ET_RETURN:
		if (!((stReturnExp *)s)->expression) {
			result->type = AT_NULL;
			resultCode = EC_STOP;
		} else {
			resultCode = stExpEval3(((stReturnExp *)s)->expression, i, result);
			if (resultCode == EC_OK)
				resultCode = EC_STOP;
		}

		break;
	case ET_ARRAY_INDEX:
		resultCode = EVAL_RTC_CALL_3(s, stEvalArrayIndex, (stArrayIndexExp *)s, i, result);
		break;
	case ET_ARRAY_INDEX_ASSIGN:
		resultCode = EVAL_RTC_CALL_3(s, stEvalArrayIndexAssign, (stArrayIndexAssignExp *)s, i, result);
		break;
	case ET_INT:
		result->type = AT_INT;
		BRINT(result) = ((stIntExp *)s)->intValue;
		break;
	case ET_DOUBLE:
		result->type = AT_DOUBLE;
		BRDOUBLE(result) = ((stDoubleExp *)s)->doubleValue;
		break;
	case ET_SUPER:
		result->type = AT_INSTANCE;

		if(i->type->super) {
			BRINSTANCE(result) = i->instance->breveInstance;
			if( tClass) *tClass = i->type->super;
		} else {
			BRINSTANCE(result) = NULL;
		}

		break;
	case ET_SELF:
		result->type = AT_INSTANCE;
		BRINSTANCE(result) = i->instance->breveInstance;
		break;
	case ET_IF:
		resultCode = EVAL_RTC_CALL_3(s, stEvalIf, (stIfExp *)s, i, result);
		break;
	case ET_VECTOR:
		resultCode = EVAL_RTC_CALL_3(s, stEvalVectorExp, (stVectorExp *)s, i, result);
		break;
	case ET_MATRIX:
		resultCode = EVAL_RTC_CALL_3(s, stEvalMatrixExp, (stMatrixExp *)s, i, result);
		break;
	case ET_UNARY:
		resultCode = EVAL_RTC_CALL_3(s, stEvalUnaryExp, (stUnaryExp *)s, i, result);
		break;
	case ET_ST_EVAL:
		memcpy(result, ((stEvalExp *)s)->eval, sizeof(brEval));
		break;
	case ET_LIST:
		resultCode = brEvalListExp((stListExp *)s, i, result);
		break;
	case ET_CODE_ARRAY:
		resultCode = EVAL_RTC_CALL_3(s, stEvalCodeArray, (stCodeArrayExp *)s, i, result);
		break;
	case ET_LENGTH:
		resultCode = stExpEval3(((stLengthExp *)s)->expression, i, result);
		switch(result->type) {
		case AT_VECTOR:
			result->type = AT_DOUBLE;
			BRDOUBLE(result) = slVectorLength(&BRVECTOR(result));
			resultCode = EC_OK;
			break;
		case AT_LIST:
			result->type = AT_INT;
			BRINT(result) = BRLIST(result)->count;
			resultCode = EC_OK;
			break;
		case AT_INT:
			result->type = AT_INT;
			BRINT(result) = abs(BRINT(result));
			resultCode = EC_OK;
			break;
		case AT_DOUBLE:
			result->type = AT_DOUBLE;
			BRDOUBLE(result) = fabs(BRDOUBLE(result));
			resultCode = EC_OK;
			break;
		case AT_STRING:
			result->type = AT_INT;
			if (!BRSTRING(result))
				BRINT(result) = 0;
			else
				BRINT(result) = strlen(BRSTRING(result));
			break;
		default:
			stEvalError(i->instance->type->engine, EE_TYPE, "Cannot give magnitude of %s expression", slAtomicTypeStrings[result->type]);
			return EC_ERROR;
			break;				
		}
		break;
	case ET_WHILE:
		resultCode = EVAL_RTC_CALL_3(s, stEvalWhile, (stWhileExp *)s, i, result);
		break;
	case ET_FOREACH:
		resultCode = EVAL_RTC_CALL_3(s, stEvalForeach, (stForeachExp *)s, i, result);
		break;
	case ET_FOR:
		resultCode = EVAL_RTC_CALL_3(s, stEvalFor, (stForExp *)s, i, result);
		break;
	case ET_INSERT:
		resultCode = EVAL_RTC_CALL_3(s, stEvalListInsert, (stListInsertExp *)s, i, result);
		break;
	case ET_REMOVE:
		resultCode = EVAL_RTC_CALL_3(s, stEvalListRemove, (stListRemoveExp *)s, i, result);
		break;
	case ET_COPYLIST:
		resultCode = EVAL_RTC_CALL_3(s, stEvalCopyList, (stCopyListExp *)s, i, result);
		break;
	case ET_ALL:
		resultCode = EVAL_RTC_CALL_3(s, stEvalAll, (stAllExp *)s, i, result);
		break;
	case ET_SORT:
		resultCode = EVAL_RTC_CALL_3(s, stEvalSort, (stSortExp *)s, i, result);
		break;
	case ET_LIST_INDEX:
		resultCode = EVAL_RTC_CALL_3(s, stEvalListIndex, (stListIndexExp *)s, i, result);
		break;
	case ET_LIST_INDEX_ASSIGN:
		resultCode = EVAL_RTC_CALL_3(s, stEvalListIndexAssign, (stListIndexAssignExp *)s, i, result);
		break;
	case ET_PRINT:
		resultCode = stEvalPrint((stPrintExp *)s, i, &t);
		break;
	case ET_STRING:
		resultCode = stProcessString((stStringExp *)s, i, result);
		break; 
	case ET_INSTANCE:
		resultCode = EVAL_RTC_CALL_3(s, stEvalNewInstance, (stInstanceExp *)s, i, result);
		break;
	case ET_VECTOR_ELEMENT:
		resultCode = EVAL_RTC_CALL_3(s, stEvalVectorElementExp, (stVectorElementExp *)s, i, result);
		break;
	case ET_VECTOR_ELEMENT_ASSIGN:
		resultCode = EVAL_RTC_CALL_3(s, stEvalVectorElementAssignExp, (stVectorElementAssignExp *)s, i, result);
		break;
	case ET_RANDOM:
		resultCode = EVAL_RTC_CALL_3(s, stEvalRandExp, (stRandomExp *)s, i, result);
		break;
	case ET_DUPLICATE:
		// resultCode = EVAL_RTC_CALL_3(s, stExpEval3, ((stDuplicateExp *)s)->expression, i, result);
		resultCode = stExpEval(((stDuplicateExp *)s)->expression, i, result, NULL);
		break;
	case ET_FREE:
		resultCode = EVAL_RTC_CALL_3(s, stEvalFree, (stFreeExp *)s, i, result);
		break;
	case ET_DIE:
		resultCode = stExpEval3(((stDieExp *)s)->expression, i, &t);
		stEvalError(i->instance->type->engine, EE_USER, "execution stopped from within simulation: %s", BRSTRING(&t));
		return EC_ERROR;
		break;
	default:
		stEvalError(i->instance->type->engine, EE_INTERNAL, "unknown expression type (%d) in stExpEval", s->type);
		resultCode = EC_ERROR;
		break;
	}

	return resultCode;
}

/*!
	\brief Gives a pointer to the eval-value of a list at a specified index. 
    
	The index is 0-based.
*/  


int stDoEvalListIndexPointer(brEvalListHead *l, int n, brEval **eval) {
	brEvalList *list;

	if (n > (l->count - 1) || n < 0) {
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

	if (!eval) {
		newLoc->type = AT_NULL;
		return -1;
	}
    
	brEvalCopy(eval, newLoc);
	// stGCRetain(newLoc);
    
	return 0;
}
   
int stDoEvalListIndexAssign(brEvalListHead *l, int n, brEval *newVal, stRunInstance *ri) {
	brEvalList *list;

	if (n > l->count || n < 0)
		return -1;

	// if this is a new entry at the end, append it instead

	if (n == l->count || l->count == 0) {
		if (brEvalListInsert(l, l->count, newVal) > -1)
			return 0;
		else
			return -1;
	}

	// overwriting a previous evaluation -- lookup, unretain and replace

	list = brEvalListIndexLookup(l, n);

	stGCUnretainAndCollect(&list->eval);

	brEvalCopy(newVal, &list->eval);

	return 0;
}

RTC_INLINE int stEvalBinaryEvalListExp(char op, brEval *l, brEval *r, brEval *result, stRunInstance *i) {
	brEvalListHead *h1, *h2;
	brEvalList *l1, *l2;
	int same;
	int ret;

	h1 = BRLIST(l);
	h2 = BRLIST(r);

	if (h1->count != h2->count)
		same = 0;
	else {
		l1 = h1->start;
		l2 = h2->start;

		same = 1;

		while (l1 && l2 && same) {
			ret = stEvalBinaryExpWithEvals(i, BT_EQ, &l1->eval, &l2->eval, result);

			if (ret != EC_OK || !BRINT(result))
				same = 0;

			l1 = l1->next;
			l2 = l2->next;
		}
	}

	switch(op) {
	case BT_EQ:
		BRINT(result) = same;
		break;
	case BT_NE:
		BRINT(result) = !same;
		break;
	default:
		stEvalError(i->instance->type->engine, EE_INTERNAL, "unknown binary expression operator (%d) in stEvalBinaryDoubleExp", op);
		return EC_ERROR;
	}

	return EC_OK;
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
	brErrorInfo *error = brEngineGetErrorInfo(e);
	char *m, localMessage[BR_ERROR_TEXT_SIZE];

	if (!error->type) {
		m = error->message;

		// if this is the first stEvalError, this is the primary error
		// print out all of the information

		error->type = type;
	} else
		m = localMessage;

	va_start(vp, proto);
	vsnprintf(m, BR_ERROR_TEXT_SIZE, proto, vp);
	va_end(vp);

	slMessage(DEBUG_ALL, m);
	slMessage(DEBUG_ALL, "\n");
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
