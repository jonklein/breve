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
	+ expression.c
	=
	= "The book of love if long and boring..." - Magnetic Fields
	= 
	= This file is long and boring.  The basic idea is that parse trees 
	= are made up of the structures defined in expression.h, and this
	= file just gives "constructor" (and destructor) functions for all of 
	= them.  So when the parser finds a binary expression, it calls 
	= stNewBinaryExp(blah, blah, blah).  These is very little processing
	= done in this file, with the exception of the occasional symbol
	= lookup in order to fill out a field in one of the structures.
	=
	= The only thing remotely interesting here is the stSizeof and stAlign
	= functions which deal with the size of steve types.  They're at the
	= very bottom.
	=
	= Very few comments here.
	=
	= A few notes:
	=   * all expression structures are held within an stExp struct 
	=	 which stores the structure pointer, as well as it's file
	=	 and location.  the last line of most of these functions
	=	 is thus something to the effect of: 
	=	   return stExpNew(exp, type, file, line);
	=   * some expression types don't need a seperate structure
	=	 in the stExp structure, they just use a simple data type.
*/

#include "steve.h"

stExp *stExpNew(void *data, char type, char *file, int line) {
	stExp *e;

	e = slMalloc(sizeof(stExp));

	e->values.pValue = data;
	e->type = type;
	e->line = line;

	e->file = file;
 
	return e;
}

int stExpFree(stExp *e) {
	int result = 0;

	if(!e) return result;

	switch(e->type) {
		/* these elements have nothing additional allocated */

		case ET_SUPER:
		case ET_SELF:
		case ET_INT:
		case ET_DOUBLE:
		case ET_DUPLICATE:
			break;

		/* these elements have only static buffers allocated */

		case ET_ST_EVAL:
		case ET_LOAD:
			slFree(e->values.pValue);
			break;

		/* these elements just have another stExp */

		case ET_RETURN:
		case ET_FREE:
		case ET_VLENGTH:
		case ET_RANDOM:
		case ET_DIE:
		case ET_COPYLIST:
			stExpFree(e->values.pValue);
			break;

		/* these need special freeing */

		case ET_ALL:
			stFreeAllExp(e->values.pValue);
			break;
		case ET_LIST:
			stExpFreeList(e->values.pValue);
			break;
		case ET_VECTOR_ELEMENT:
			stFreeVectorElementExp(e->values.pValue);
			break;
		case ET_VECTOR_ELEMENT_ASSIGN:
			stFreeVectorElementAssignExp(e->values.pValue);
			break;
		case ET_VECTOR:
			stFreeVectorExp(e->values.pValue);
			break;
		case ET_MATRIX:
			stFreeMatrixExp(e->values.pValue);
			break;
		case ET_EXP_ARRAY:
			stExpFreeArray(e->values.pValue);
			break;
		case ET_BINARY:
			stFreeBinaryExp(e->values.pValue);
			break;
		case ET_UNARY:
			stFreeUnaryExp(e->values.pValue);
			break;
		case ET_WHILE:
			stFreeWhileExp(e->values.pValue);
			break;
		case ET_IF:
			stFreeIfExp(e->values.pValue);
			break;
		case ET_FOREACH:
			stFreeForeachExp(e->values.pValue);
			break;
		case ET_FOR:
			stFreeForExp(e->values.pValue);
			break;
		case ET_STRING:
			stFreeStringExp(e->values.pValue);
			break;
		case ET_PRINT:
			stFreePrintExp(e->values.pValue);
			break;

		case ET_CODE_ARRAY:
		case ET_ERROR:
			stExpFreeArray(e->values.pValue);
			break;

		case ET_INSTANCE:
			stInstanceFreeExp(e->values.pValue);
			break;

		case ET_ARRAY_INDEX_ASSIGN:
			stFreeArrayIndexAssignExp(e->values.pValue);
			break;

		case ET_ARRAY_INDEX:
			stFreeArrayIndexExp(e->values.pValue);
			break;

		case ET_ASSIGN:
			stFreeAssignExp(e->values.pValue);
			break;

		case ET_METHOD:
			stFreeMethodExp(e->values.pValue);
			break;

		case ET_FUNC:
			stFreeCCallExp(e->values.pValue);
			break;

		case ET_INSERT:
			stFreeListInsertExp(e->values.pValue);
			break;

		case ET_REMOVE:
			stFreeListRemoveExp(e->values.pValue);
			break;

		case ET_SORT:
			stFreeSortExp(e->values.pValue);
			break;

		case ET_LIST_INDEX:
			stFreeListIndexExp(e->values.pValue);
			break;

		case ET_LIST_INDEX_ASSIGN:
			stFreeListIndexAssignExp(e->values.pValue);
			break;

		case ET_ISA:
			stFreeIsaExp(e->values.pValue);
			break;

		default: 
			slMessage(DEBUG_ALL, "expression type %d not freed\n", e->type);
			break;
	}

	slFree(e);

	return result;
}

void stExpFreeList(slList *n) {
	slList *head = n;

	while(n) {
		stExpFree(n->data);
		n = n->next;
	}

	slListFree(head);
}

void stExpFreeArray(slArray *a) {
	int n;

	if(!a) return;

	for(n=0;n<a->count;n++) stExpFree(a->data[n]);

	slFreeArray(a); 
}

stExp *stNewStEvalExp(brEval *eval, char *file, int line) {
	stExp *e;

	e = slMalloc(sizeof(stExp));

	e->values.pValue = eval;
	e->type = ET_ST_EVAL;
	e->line = line;
	e->file = file;

	return e;
}

stExp *stNewIntExp(int n, char *file, int line) {
	stExp *e;

	e = slMalloc(sizeof(stExp));

	e->values.iValue = n;
	e->type = ET_INT;
	e->line = line;
	e->file = file;

	return e;
}

stExp *stNewDoubleExp(double n, char *file, int line) {
	stExp *e;

	e = slMalloc(sizeof(stExp));

	e->values.dValue = n;
	e->type = ET_DOUBLE;
	e->line = line;
	e->file = file;

	return e;
}

stExp *stNewArrayIndexExp(stMethod *m, stObject *o, char *word, stExp *index, char *file, int line) {
	stVar *var;
	stArrayIndexExp *sa;
	brNamespaceSymbol *varSymbol;

	sa = slMalloc(sizeof(stArrayIndexExp));
	sa->index = index;
 
	/* first find out if this is a local variable */
 
	if((var = stFindLocal(word, m))) {
		sa->local = 1;
	} else {
		varSymbol = stObjectLookup(o, word, ST_VAR);

		if(!varSymbol) {
			stParseError(o->engine, PE_UNKNOWN_SYMBOL, "Unable to locate variable \"%s\" for object \"%s\"", word, o->name);
			slFree(sa);
			return NULL;
		}

		var = varSymbol->data;
		sa->local = 0;
	}

	if(var->type->type != AT_ARRAY) {
		stParseError(o->engine, PE_TYPE, "Variable \"%s\" is not an array", word);
		slFree(sa);
		return NULL;
	}

	sa->offset = var->offset;
	sa->type = var->type->arrayType;
	sa->maxIndex = var->type->arrayCount;
	sa->typeSize = stSizeofAtomic(sa->type);

	var->used = 1;

	return stExpNew(sa, ET_ARRAY_INDEX, file, line);;
}

void stFreeArrayIndexExp(stArrayIndexExp *e) {
	stExpFree(e->index);
	slFree(e);
}

stExp *stNewArrayIndexAssignExp(stMethod *m, stObject *o, char *word, stExp *index, stExp *rvalue, char *file, int line) {
	stVar *var;
	stArrayIndexAssignExp *sa;
	brNamespaceSymbol *varSymbol;

	sa = slMalloc(sizeof(stArrayIndexAssignExp));
	sa->index = index;
	sa->rvalue = rvalue;
 
	/* first find out if this is a local variable */
 
	if((var = stFindLocal(word, m))) {
		sa->local = 1;
	} else {
		varSymbol = stObjectLookup(o, word, ST_VAR);

		if(!varSymbol) {
			stParseError(o->engine, PE_UNKNOWN_SYMBOL, "Unable to locate variable \"%s\" for object \"%s\"", word, o->name);
			slFree(sa);
			return NULL;
		}

		var = varSymbol->data;
		sa->local = 0;
	}

	if(var->type->type != AT_ARRAY) {
		stParseError(o->engine, PE_TYPE, "Variable \"%s\" is not an array");
		slFree(sa);
		return NULL;
	}

	sa->offset = var->offset;
	sa->type = var->type->arrayType;
	sa->maxIndex = var->type->arrayCount;
	sa->typeSize = stSizeofAtomic(sa->type);
	var->used = 1;

	return stExpNew(sa, ET_ARRAY_INDEX_ASSIGN, file, line);;
}

void stFreeArrayIndexAssignExp(stArrayIndexAssignExp *e) {
	stExpFree(e->rvalue);
	stExpFree(e->index);
	slFree(e);
}

stExp *stNewLoadExp(stMethod *m, stObject *o, char *word, char *file, int line) {
	stVar *var;
	brNamespaceSymbol *varSymbol;
	stLoadExp *le;
 
	le = slMalloc(sizeof(stLoadExp));

	/* we don't know if this variable is local or not */

	if((var = stFindLocal(word, m))) {
		le->local = 1;
	} else {
		varSymbol = stObjectLookup(o, word, ST_VAR);

		if(!varSymbol) {
			stParseError(o->engine, PE_UNKNOWN_SYMBOL, "Unable to locate variable \"%s\" for object \"%s\"", word, o->name);
			slFree(le);
			return NULL;
		}

		var = varSymbol->data;
		le->local = 0;
	}

	le->offset = var->offset;
	le->type = var->type->type;
	var->used = 1;

	return stExpNew(le, ET_LOAD, file, line);
}

void stFreeAssignExp(stAssignExp *a) {
	if(a->objectName) slFree(a->objectName);
	if(a->rvalue) stExpFree(a->rvalue);
	slFree(a);
}

stExp *stNewAssignExp(stMethod *m, stObject *o, char *word, stExp *rvalue, char *file, int line) {
	stVar *var;
	brNamespaceSymbol *varSymbol;
	stAssignExp *ae;
 
	ae = slMalloc(sizeof(stAssignExp));
	ae->rvalue = rvalue;

	/* we don't know if this variable is local or not */

	if((var = stFindLocal(word, m))) {
		ae->local = 1;
	} else {
		varSymbol = stObjectLookup(o, word, ST_VAR);

		if(!varSymbol) {
			stParseError(o->engine, PE_UNKNOWN_SYMBOL, "Unable to locate variable \"%s\" for object \"%s\"", word, o->name);
			slFree(ae);
			return NULL;
		}

		var = varSymbol->data;
		ae->local = 0;
	}

	ae->offset = var->offset;
	ae->type = var->type->type;

	if(var->type->objectName) ae->objectName = slStrdup(var->type->objectName);
	else var->type->objectName = NULL;

	var->used = 1;

	return stExpNew(ae, ET_ASSIGN, file, line);
}

stMethodExp *stNewMethodCall(stObject *o, stExp *callingObject, char *method, slArray *args) {
	stMethodExp *m;

	if(!strcmp(method, "init")) {
		stParseError(o->engine, PE_SYNTAX, "Method '%s' may not be called manually", method);
		return NULL;
	}

	m = slMalloc(sizeof(stMethodExp));

	m->methodName = slStrdup(method);
	m->method = NULL;
	m->objectExp = callingObject;
	m->arguments = args;
	m->positionedArguments = slStackNew();

	return m;
}

void stFreeMethodExp(stMethodExp *m) {
	slFree(m->methodName);
	stExpFree(m->objectExp);
	stFreeKeywordArray(m->arguments);
	slStackFree(m->positionedArguments);
	slFree(m);
}

// these methods involve creating LIST operator expressions

stExp *stNewListInsertExp(stExp *list, stExp *exp, stExp *index, char *file, int lineno) {
	stListInsertExp *p;

	p = slMalloc(sizeof(stListInsertExp));

	p->listExp = list;
	p->exp = exp;
	p->index = index;

	return stExpNew(p, ET_INSERT, file, lineno);
}

stExp *stNewListRemoveExp(stExp *list, stExp *index, char *file, int lineno) {
	stListRemoveExp *p;

	p = slMalloc(sizeof(stListRemoveExp));
	p->listExp = list;
	p->index = index;

	return stExpNew(p, ET_REMOVE, file, lineno);
}

void stFreeListRemoveExp(stListRemoveExp *p) {
	stExpFree(p->listExp);
	if(p->index) stExpFree(p->index);
	slFree(p);
}

void stFreeListInsertExp(stListInsertExp *p) {
	stExpFree(p->listExp);
	stExpFree(p->exp);
	if(p->index) stExpFree(p->index);
	slFree(p);
}

stExp *stNewSortExp(stExp *list, char *method, char *file, int lineno) {
	stSortExp *s;
 
	s = slMalloc(sizeof(stSortExp));
	s->methodName = slStrdup(method);
	s->listExp = list;

	return stExpNew(s, ET_SORT, file, lineno);
}

void stFreeSortExp(stSortExp *s) {
	stExpFree(s->listExp);

	slFree(s->methodName);
	slFree(s);
}

stExp *stNewListIndexExp(stExp *list, stExp *index, char *file, int lineno) {
	stListIndexExp *e;

	e = slMalloc(sizeof(stListIndexExp));

	e->listExp = list;
	e->indexExp = index;

	return stExpNew(e, ET_LIST_INDEX, file, lineno);
}

void stFreeListIndexExp(stListIndexExp *e) {
	stExpFree(e->listExp);
	stExpFree(e->indexExp);

	slFree(e);
}

stExp *stNewListIndexAssignExp(stExp *list, stExp *index, stExp *assignment, char *file, int lineno) {
	stListIndexAssignExp *s;

	s = slMalloc(sizeof(stListIndexAssignExp));

	s->listExp = list;
	s->indexExp = index;
	s->assignment = assignment;

	return stExpNew(s, ET_LIST_INDEX_ASSIGN, file, lineno);
}

void stFreeListIndexAssignExp(stListIndexAssignExp *s) {
	stExpFree(s->listExp);
	stExpFree(s->indexExp);
	stExpFree(s->assignment);

	slFree(s);
}

stExp *stNewCCallExp(brEngine *e, brNamespaceSymbol *s, slList *exps, char *file, int line) {
	stCCallExp *c;
	int passedArgs;
  
	c = slMalloc(sizeof(stCCallExp));

	c->function = s->data;

	if(exps) {
		c->args = slListToArray(exps);

		passedArgs = c->args->count;
	} else {
		c->args = NULL;
		passedArgs = 0;
	}

	/* parse time check of the number of args passed in. */
	/* type checking has to be done at run time. */

	if(c->function->nargs != passedArgs) {
		stParseError(e, PE_PROTOTYPE, "invalid number of arguments to internal method \"%s\": expected %d, got %d", s->name, c->function->nargs, passedArgs); 
		slFree(c);

		return NULL;
	}

	return stExpNew(c, ET_FUNC, file, line);
}

void stFreeCCallExp(stCCallExp *c) {
	/* UNFREED ... */

	stExpFreeArray(c->args);

	slFree(c);
}

stExp *stNewAllExp(char *object, char *file, int line) {
	stAllExp *e;

	e = slMalloc(sizeof(stAllExp));
	e->name = slStrdup(object);

	return stExpNew(e, ET_ALL, file, line);
}

void stFreeAllExp(stAllExp *e) {
	slFree(e->name);
	slFree(e);
}

stExp *stNewWhileExp(stExp *cond, stExp *code, char *file, int lineno) {
	stWhileExp *w;

	w = slMalloc(sizeof(stWhileExp));

	w->cond = cond;
	w->code = code;

	return stExpNew(w, ET_WHILE, file, lineno);
}

void stFreeWhileExp(stWhileExp *w) {
	stExpFree(w->cond);
	stExpFree(w->code);

	slFree(w);
}

stExp *stNewForeachExp(stAssignExp *assignment, stExp *list, stExp *code, char *file, int lineno) {
	stForeachExp *w;

	w = slMalloc(sizeof(stForeachExp));

	w->assignment = assignment;
	w->list = list;
	w->code = code;

	return stExpNew(w, ET_FOREACH, file, lineno);
}

void stFreeForeachExp(stForeachExp *f) {
	stFreeAssignExp(f->assignment);
	stExpFree(f->list);
	stExpFree(f->code);

	slFree(f);
}

stExp *stNewForExp(stExp *assignment, stExp *condition, stExp *iteration, stExp *code, char *file, int lineno) {
	stForExp *f;

	f = slMalloc(sizeof(stForExp));

	f->condition = condition;
	f->assignment = assignment;
	f->iteration = iteration;
	f->code = code;

	return stExpNew(f, ET_FOR, file, lineno);
}

void stFreeForExp(stForExp *f) {
	stExpFree(f->condition);
	stExpFree(f->iteration);
	stExpFree(f->assignment);
	stExpFree(f->code);

	slFree(f);
}

stExp *stNewIfExp(stExp *cond, stExp *trueCode, stExp *falseCode, char *file, int lineno) {
	stIfExp *i;

	i = slMalloc(sizeof(stIfExp));

	i->cond = cond;
	i->trueCode = trueCode;
	i->falseCode = falseCode;

	return stExpNew(i, ET_IF, file, lineno);
}

void stFreeIfExp(stIfExp *f) {
	stExpFree(f->trueCode);
	stExpFree(f->falseCode);
	stExpFree(f->cond);

	slFree(f);
}

stExp *stNewPrintExp(slArray *array, int newline, char *file, int lineno) {
	stPrintExp *pe;

	pe = slMalloc(sizeof(stPrintExp));
	pe->expressions = array;
	pe->newline = newline;

	return stExpNew(pe, ET_PRINT, file, lineno);
}

void stFreePrintExp(stPrintExp *pe) {
	stExpFreeArray(pe->expressions);
	slFree(pe);
}

stKeyword *stNewKeyword(char *word, stExp *data) {
	stKeyword *k;

	k = slMalloc(sizeof(struct stKeyword));

	k->word = slStrdup(word);
	k->value = data;

	return k;
}

void stFreeKeywordArray(slArray *a) {
	int n;

	if(!a) return;

	for(n=0;n<a->count;n++) stFreeKeyword(a->data[n]);

	slFreeArray(a);
}

void stFreeKeyword(stKeyword *k) {
	slFree(k->word);
	stExpFree(k->value);

	slFree(k);
}

stExp *stNewBinaryExp(int type, stExp *left, stExp *right, char *file, int line) {
	stBinaryExp *be;

	be = slMalloc(sizeof(stBinaryExp));

	be->left = left;
	be->right = right;

	be->type = type;

	return stExpNew(be, ET_BINARY, file, line);
}

void stFreeBinaryExp(stBinaryExp *b) {
	stExpFree(b->left);
	stExpFree(b->right);

	slFree(b);
}

stExp *stNewUnaryExp(int type, stExp *exp, char *file, int line) {
	stUnaryExp *be;

	be = slMalloc(sizeof(stUnaryExp));

	be->exp = exp;

	be->type = type;

	return stExpNew(be, ET_UNARY, file, line);
}

void stFreeUnaryExp(stUnaryExp *u) {
	stExpFree(u->exp);

	slFree(u);
}

stExp *stNewVectorExp(stExp *x, stExp *y, stExp *z, char *file, int line) {
	stVectorExp *ve;

	ve = slMalloc(sizeof(stVectorExp));

	ve->x = x;
	ve->y = y;
	ve->z = z;

	return stExpNew(ve, ET_VECTOR, file, line);
}

stExp *stNewMatrixExp(stExp *e00, stExp *e01, stExp *e02, stExp *e10, stExp *e11, stExp *e12, stExp *e20, stExp *e21, stExp *e22, char *file, int line) {
	stMatrixExp *me;

	me = slMalloc(sizeof(stMatrixExp));

	me->expressions[0] = e00;
	me->expressions[1] = e01;
	me->expressions[2] = e02;

	me->expressions[3] = e10;
	me->expressions[4] = e11;
	me->expressions[5] = e12;

	me->expressions[6] = e20;
	me->expressions[7] = e21;
	me->expressions[8] = e22;

	return stExpNew(me, ET_MATRIX, file, line);
}

void stFreeVectorExp(stVectorExp *v) {
	stExpFree(v->x);
	stExpFree(v->y);
	stExpFree(v->z);

	slFree(v);
}

void stFreeMatrixExp(stMatrixExp *m) {
	int n;

	for(n=0;n<9;n++) stExpFree(m->expressions[n]);

	slFree(m);
}


stExp *stNewVectorElementExp(stExp *v, char element, char *file, int line) {
	stVectorElementExp *ve;

	ve = slMalloc(sizeof(stVectorElementExp));

	ve->exp = v;
	ve->element = element;

	return stExpNew(ve, ET_VECTOR_ELEMENT, file, line);
}

void stFreeVectorElementExp(stVectorElementExp *v) {
	stExpFree(v->exp);
	slFree(v);
}

stExp *stNewVectorElementAssignExp(stExp *v, stExp *rvalue, char element, char *file, int line) {
	stVectorElementAssignExp *ve;

	ve = slMalloc(sizeof(stVectorElementAssignExp));

	ve->exp = v;
	ve->element = element;
	ve->assignExp = rvalue;

	return stExpNew(ve, ET_VECTOR_ELEMENT_ASSIGN, file, line);
}

void stFreeVectorElementAssignExp(stVectorElementAssignExp *v) {
	stExpFree(v->exp);
	stExpFree(v->assignExp);
	slFree(v);
}

stExp *stInstanceNewExp(char *name, stExp *count, char *file, int line) {
	stInstanceExp *ie;

	ie = slMalloc(sizeof(stInstanceExp));

	ie->count = count;
	ie->name = slStrdup(name);

	return stExpNew(ie, ET_INSTANCE, file, line);
}

void stInstanceFreeExp(stInstanceExp *i) {
	stExpFree(i->count);
	slFree(i->name);
	slFree(i);
}

stExp *stNewDuplicateExp(stExp *e, char *file, int line) {
	return stExpNew(e, ET_DUPLICATE, file, line);
}

stExp *stNewIsaExp(stExp *e, stVarType *type, char *file, int line) {
	stIsaExp *isa;

	isa = slMalloc(sizeof(stIsaExp));
	isa->expression = e;
	isa->type = type;

	return stExpNew(isa, ET_ISA, file, line);
}

void stFreeIsaExp(stIsaExp *e) {
	stExpFree(e->expression);
	if(e->type->objectName) slFree(e->type->objectName);
	slFree(e->type);
}

int stSizeof(stVarType *v) {
	if(v->type == AT_ARRAY) 
		return v->arrayCount * stSizeofAtomic(v->arrayType);

	return stSizeofAtomic(v->type);
}

int stSizeofAtomic(int type) {
	switch(type) {
		case AT_INT:
			return sizeof(int);
			break;
		case AT_DOUBLE:
			return sizeof(double);
			break;
		case AT_VECTOR:
			return sizeof(slVector);
			break;
		case AT_INSTANCE:
			return sizeof(stObject*);
			break;
		case AT_HASH:
			return sizeof(brEvalHash*);
			break;
		case AT_POINTER:
		case AT_DATA:
			return sizeof(void*);
			break;
		case AT_STRING:
			return sizeof(char*);
			break;
		case AT_LIST:
			return sizeof(brEvalList*);
			break;
		case AT_MATRIX:
			return sizeof(double) * 9;
			break;
		case AT_ARRAY:
			slMessage(DEBUG_ALL, "INTERNAL ERROR: stSizeof: no atomic size for AT_ARRAY\n", type);
			return 0;
			break;
		default:
			slMessage(DEBUG_ALL, "INTERNAL ERROR: stSizeof: unknown type %d\n", type);
			return 0;
			break;
	}
}

/* when we make our own space for variable storage, we need to be */
/* aware of the alignment of our pointers.  for example, doubles  */
/* need to be 8 aligned on platforms like sparc */

int stAlign(stVarType *var) {
	if(var->type == AT_ARRAY) return stAlignAtomic(var->arrayType);
	return stAlignAtomic(var->type);
}

int stAlignAtomic(int type) {
	switch(type) {
		case AT_INT:
			return sizeof(int);
			break;
		case AT_DOUBLE:
			return sizeof(double);
			break;
		case AT_VECTOR:
			return sizeof(double);
			break;
		case AT_INSTANCE:
			return sizeof(stObject*);
			break;
		case AT_HASH:
			return sizeof(brEvalHash*);
			break;
		case AT_DATA:
		case AT_POINTER:
			return sizeof(void*);
			break;
		case AT_STRING:
			return sizeof(char*);
			break;
		case AT_LIST:
			return sizeof(brEvalList*);
			break;
		case AT_MATRIX:
			return sizeof(double);
			break;
		default:
			slMessage(DEBUG_ALL, "INTERNAL ERROR: stAlign: unknown type %d\n", type);
			return 0;
			break;
	}
}
