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
	"The book of love if long and boring..." - Magnetic Fields
	
	This file is long and boring.  

	But it used to be worse.  It now contains C++ constructors and 
	destructors for all of the steve parse trees.
*/

#include "steve.h"
#include "expression.h"

stEvalExp::stEvalExp(brEval *e, char *file, int line) : stExp(file, line) {
	eval = e;
	type = ET_ST_EVAL;
}

stEvalExp::~stEvalExp() {
	stGCUnretain(eval);
}

stReturnExp::stReturnExp(stExp *e, char *file, int line) : stExp(file, line) {
	expression = e;
	type = ET_RETURN;
}

stReturnExp::~stReturnExp() {
	delete expression;
}

stFreeExp::stFreeExp(stExp *e, char *file, int line) : stExp(file, line) {
	expression = e;
	type = ET_FREE;
}

stFreeExp::~stFreeExp() {
	delete expression;
}

stCopyListExp::stCopyListExp(stExp *e, char *file, int line) : stExp(file, line) {
	expression = e;
	type = ET_COPYLIST;
}

stCopyListExp::~stCopyListExp() {
	delete expression;
}

stLengthExp::stLengthExp(stExp *e, char *file, int line) : stExp(file, line) {
	expression = e;
	type = ET_LENGTH;
}

stLengthExp::~stLengthExp() {
	delete expression;
}

stIntExp::stIntExp(int i, char *file, int line) : stExp(file, line) {
	intValue = i;
	type = ET_INT;
}

stDoubleExp::stDoubleExp(double d, char *file, int line) : stExp(file, line) {
	doubleValue = d;
	type = ET_DOUBLE;
}

stMethodExp::stMethodExp(stExp *o, char *n, std::vector< stKeyword* > *a, char *f, int l) : stExp(f, l) {
	objectExp = o;
	methodName = strdup(n);
	arguments = *a;
	method = NULL;
	type = ET_METHOD;
}

stMethodExp::~stMethodExp() {
	std::vector< stKeyword* >::iterator ki;

	free(methodName);
	delete objectExp;

	for(ki = arguments.begin(); ki != arguments.end(); ki++ )
		delete *ki;
}

stArrayIndexExp::stArrayIndexExp(stMethod *m, stObject *o, char *word, stExp *i, char *file, int line) : stExp(file, line) {
	stVar *var;

	index = i;
 
	// first find out if this is a local variable 
 
	if((var = stFindLocal(word, m))) {
		local = 1;
	} else {
		var = stObjectLookupVariable(o, word);

		if(!var) {
			stParseError(o->engine, PE_UNKNOWN_SYMBOL, "Unable to locate variable \"%s\" for object \"%s\"", word, o->name);
		}

		local = 0;
	}

	if(var->type->type != AT_ARRAY) {
		stParseError(o->engine, PE_TYPE, "Variable \"%s\" is not an array", word);
	}

	offset = var->offset;
	loadType = var->type->arrayType;
	maxIndex = var->type->arrayCount;
	typeSize = stSizeofAtomic(loadType);
	var->used = 1;

	type = ET_ARRAY_INDEX;
}

stArrayIndexExp::~stArrayIndexExp() {
	delete index;
}

stArrayIndexAssignExp::stArrayIndexAssignExp(stMethod *m, stObject *o, char *word, stExp *i, stExp *rv, char *file, int line) : stExp(file, line) {
	stVar *var;

	index = i;
	rvalue = rv;
 
	/* first find out if this is a local variable */
 
	if((var = stFindLocal(word, m))) {
		local = 1;
	} else {
		var = stObjectLookupVariable(o, word);

		if(!var) {
			stParseError(o->engine, PE_UNKNOWN_SYMBOL, "Unable to locate variable \"%s\" for object \"%s\"", word, o->name);
		}

		local = 0;
	}

	if(var->type->type != AT_ARRAY) {
		stParseError(o->engine, PE_TYPE, "Variable \"%s\" is not an array");
	}

	offset = var->offset;
	assignType = var->type->arrayType;
	maxIndex = var->type->arrayCount;
	typeSize = stSizeofAtomic(assignType);
	var->used = 1;

	type = ET_ARRAY_INDEX_ASSIGN;
}

stArrayIndexAssignExp::~stArrayIndexAssignExp() {
	delete rvalue;
	delete index;
}

stLoadExp::stLoadExp(stMethod *m, stObject *o, char *word, char *file, int line) : stExp( file, line) {
	stVar *var;
 
	// we don't know if this variable is local or not 

	if((var = stFindLocal(word, m))) {
		local = 1;
	} else {
		var = stObjectLookupVariable(o, word);

		if(!var) {
			stParseError(o->engine, PE_UNKNOWN_SYMBOL, "Unable to locate variable \"%s\" for object \"%s\"", word, o->name);
		}

		local = 0;
	}

	offset = var->offset;
	loadType = var->type->type;
	var->used = 1;

	type = ET_LOAD;
}

stBinaryExp::stBinaryExp(unsigned char o, stExp *le, stExp *re, char *f, int l) : stExp(f, l) {
	op = o;
	left = le;
	right = re;
	type = ET_BINARY;
}

stBinaryExp::~stBinaryExp() {
	delete left;
	delete right;
}

stAssignExp::stAssignExp(stMethod *m, stObject *o, char *word, stExp *r, char *file, int line) : stExp(file, line) {
	stVar *var;
 
	// check to see if the variable is local

	if((var = stFindLocal(word, m))) {
		local = 1;
	} else {
		var = stObjectLookupVariable(o, word);

		if(!var) {
			stParseError(o->engine, PE_UNKNOWN_SYMBOL, "Unable to locate variable \"%s\" for object \"%s\"", word, o->name);
		}

		local = 0;
	}

	rvalue = r;

	offset = var->offset;
	assignType = var->type->type;

	if(var->type->objectName) objectName = slStrdup(var->type->objectName);
	else objectName = NULL;

	var->used = 1;

	type = ET_ASSIGN;
}

stAssignExp::~stAssignExp() {
	if(objectName) slFree(objectName);

	delete rvalue;
}

stListInsertExp::stListInsertExp(stExp *l, stExp *e, stExp *i, char *file, int lineno) : stExp(file, lineno) {
	listExp = l;
	exp = e;
	index = i;

	type = ET_INSERT;
}

stListRemoveExp::stListRemoveExp(stExp *l, stExp *i, char *file, int lineno) : stExp(file, lineno) {
	listExp = l;
	index = i;

	type = ET_REMOVE;
}

stListRemoveExp::~stListRemoveExp() {
	delete listExp;

	if(index) delete index;
}

stListInsertExp::~stListInsertExp() {
	delete listExp;
	delete exp;

	if(index) delete index;
}

stSortExp::stSortExp(stExp *list, char *method, char *file, int lineno) : stExp(file, lineno) {
	methodName = slStrdup(method);
	listExp = list;

	type = ET_SORT;
}

stSortExp::~stSortExp() {
	delete listExp;

	slFree(methodName);
}

stListIndexExp::stListIndexExp(stExp *list, stExp *index, char *file, int lineno) : stExp(file, lineno) {
	listExp = list;
	indexExp = index;

	type = ET_LIST_INDEX;
}

stListIndexExp::~stListIndexExp() {
	delete listExp;
	delete indexExp;
}

stListIndexAssignExp::stListIndexAssignExp(stExp *l, stExp *i, stExp *a, char *file, int lineno) : stExp(file, lineno) {
	listExp = l;
	indexExp = i;
	assignment = a;

	type = ET_LIST_INDEX_ASSIGN;
}

stListIndexAssignExp::~stListIndexAssignExp() {
	delete listExp;
	delete indexExp;
	delete assignment;
}

stCCallExp::stCCallExp(brEngine *e, brInternalFunction *s, std::vector< stExp* > *expressions, char *file, int line) : stExp(file, line) {
	function = s;

	if(expressions) arguments = *expressions;

	// parse time check of the number of args passed in. 
	// type checking has to be done at run time. 

	if(function->nargs != arguments.size()) {
		stParseError(e, PE_PROTOTYPE, "invalid number of arguments to internal method \"%s\": expected %d, got %d", s->name, function->nargs, arguments.size()); 
	}

	type = ET_FUNC;
}

stCCallExp::~stCCallExp() {
	for(unsigned int n=0; n < arguments.size(); n++) delete arguments[n];
}

stAllExp::stAllExp(char *objectName, char *file, int line) : stExp(file, line) {
	name = slStrdup(objectName);

	type = ET_ALL;
}

stAllExp::~stAllExp() {
	slFree(name);
}

stWhileExp::stWhileExp(stExp *cn, stExp *cd, char *file, int lineno) : stExp(file, line) {
	cond = cn;
	code = cd;

	type = ET_WHILE;
}

stWhileExp::~stWhileExp() {
	delete cond;
	delete code;
}

stForeachExp::stForeachExp(stAssignExp *a, stExp *l, stExp *c, char *file, int lineno) : stExp(file, lineno) {
	assignment = a;
	list = l;
	code = c;

	type = ET_FOREACH;
}

stForeachExp::~stForeachExp() {
	delete assignment;
	delete list;
	delete code;
}

stForExp::stForExp(stExp *a, stExp *cn, stExp *i, stExp *cd, char *file, int lineno) : stExp(file, lineno) {
	condition = cn;
	assignment = a;
	iteration = i;
	code = cd;

	type = ET_FOR;
}

stForExp::~stForExp() {
	delete condition;
	delete iteration;
	delete assignment;
	delete code;
}

stIfExp::stIfExp(stExp *c, stExp *t, stExp *f, char *file, int lineno) : stExp(file, lineno) {
	cond = c;
	trueCode = t;
	falseCode = f;

	type = ET_IF;
}

stIfExp::~stIfExp() {
	delete trueCode;
	delete falseCode;
	delete cond;
}

stPrintExp::stPrintExp(std::vector< stExp* > *e, int n, char *file, int lineno) : stExp(file, lineno) {
	expressions = *e;
	newline = n;

	type = ET_PRINT;
}

stPrintExp::~stPrintExp() {
	unsigned int n;

	for(n=0; n < expressions.size(); n++) delete expressions[n];
}

stUnaryExp::stUnaryExp(unsigned char o, stExp *e, char *f, int l) : stExp(f, l) {
	expression = e;
	op = o;
	type = ET_UNARY;
}

stUnaryExp::~stUnaryExp() {
	delete expression;
}

stVectorExp::stVectorExp(stExp *x, stExp *y, stExp *z, char *file, int line) : stExp(file, line) {
	_x = x;
	_y = y;
	_z = z;

	type = ET_VECTOR;
}

stVectorExp::~stVectorExp() {
	delete _x;
	delete _y;
	delete _z;
}

stMatrixExp::stMatrixExp(stExp *e00, stExp *e01, stExp *e02, stExp *e10, stExp *e11, stExp *e12, stExp *e20, stExp *e21, stExp *e22, char *file, int line) : stExp(file, line) {
	expressions[0] = e00;
	expressions[1] = e01;
	expressions[2] = e02;

	expressions[3] = e10;
	expressions[4] = e11;
	expressions[5] = e12;

	expressions[6] = e20;
	expressions[7] = e21;
	expressions[8] = e22;

	type = ET_MATRIX;
}

stMatrixExp::~stMatrixExp() {
	for(unsigned int n=0;n<9;n++) delete (expressions[n]);
}

stVectorElementExp::stVectorElementExp(stExp *v, char e, char *file, int line) : stExp( file, line) {
	exp = v;
	element = e;

	type = ET_VECTOR_ELEMENT;
}

stVectorElementExp::~stVectorElementExp() {
	delete exp;
}

stVectorElementAssignExp::stVectorElementAssignExp(stExp *v, stExp *r, char e, char *file, int line) : stExp(file, line) {
	exp = v;
	element = e;
	assignExp = r;

	type = ET_VECTOR_ELEMENT_ASSIGN;
}

stVectorElementAssignExp::~stVectorElementAssignExp() {
	delete exp;
	delete assignExp;
}

stDuplicateExp::stDuplicateExp(stExp *e, char *file, int line) : stExp(file, line) {
	expression = e;
	type = ET_DUPLICATE;
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

