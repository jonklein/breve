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

/*!
	\brief A runtime instance record which tracks the current context class
	(ie, which superclass is active).

	This structure is used to keep track of the current instance, as well
	as which superclass of the instance is being used for variable and 
	method lookups.
*/

struct stRunInstance {
	stInstance *instance;
	stObject *type;
};

/*!
	\brief Holds information about a function-frame of steve execution.

	Holds information about the current method call, as well as a pointer
	to the previous stack record.  Used for proper method call GC/cleanup
	as well as stacktraces, etc.
*/

struct stStackRecord {
	stInstance *instance;
	stMethod *method;
	stStackRecord *previousStackRecord;
	slStack *gcStack;
};

int brEvalCopy(brEval *s, brEval *d);

__inline__ int stEvalFree(stExp *e, stRunInstance *i, brEval *t);
__inline__ int stEvalArray(slArray *l, stRunInstance *i, brEval *target);
__inline__ int stEvalMethodCall(stMethodExp *m, stRunInstance *i, brEval *t);
__inline__ int stRealEvalMethodCall(stMethodExp *mexp, stRunInstance *caller, stRunInstance *i, brEval *t);

__inline__ int stEvalCallFunc(stCCallExp *c, stRunInstance *i, brEval *target);

__inline__ int stEvalAssignment(stAssignExp *e, stRunInstance *i, brEval *rvalue);

__inline__ int stEvalListExp(slList *l, stRunInstance *i, brEval *rvalue);

__inline__ int stPointerForExp(stExp *expression, stRunInstance *i, void **pointer, int *type);
__inline__ int stEvalLoadPointer(stLoadExp *e, stRunInstance *i, void **pointer, int *type);
__inline__ int stEvalArrayIndexPointer(stArrayIndexExp *a, stRunInstance *i, void **pointer, int *type);

int stEvalLoad(stLoadExp *e, stRunInstance *i, brEval *target);

__inline__ int stEvalArrayIndexAssign(stArrayIndexAssignExp *e, stRunInstance *i, brEval *target);
__inline__ int stEvalArrayIndex(stArrayIndexExp *e, stRunInstance *i, brEval *target);

__inline__ int stEvalPrint(stPrintExp *printExp, stRunInstance *i, brEval *target);
__inline__ int stEvalRandExp(stExp *e, stRunInstance *i, brEval *target);

__inline__ int stToDouble(brEval *e, brEval *t, stRunInstance *i);
__inline__ int stToInt(brEval *e, brEval *t, stRunInstance *i);
__inline__ int stToType(brEval *e, int type, brEval *t, stRunInstance *i);

__inline__ int stEvalVectorExp(stVectorExp *s, stRunInstance *i, brEval *target);
__inline__ int stEvalMatrixExp(stMatrixExp *s, stRunInstance *i, brEval *target);
__inline__ int stEvalBinaryExp(stBinaryExp *b, stRunInstance *i, brEval *target);
__inline__ int stEvalBinaryExpWithEvals(stRunInstance *i, unsigned char op, brEval *tr, brEval *tl, brEval *target);

__inline__ int stEvalUnaryExp(stUnaryExp *b, stRunInstance *i, brEval *target);

__inline__ int stEvalBinaryStringExp(char op, brEval *l, brEval *r, brEval *t, stRunInstance *i);
__inline__ int stEvalBinaryVectorExp(char op, brEval *l, brEval *r, brEval *t, stRunInstance *i);
__inline__ int stEvalBinaryMatrixExp(char op, brEval *l, brEval *r, brEval *t, stRunInstance *i);
__inline__ int stEvalBinaryDoubleExp(char op, brEval *l, brEval *r, brEval *t, stRunInstance *i);
__inline__ int stEvalBinaryIntExp(char op, brEval *l, brEval *r, brEval *t, stRunInstance *i);

__inline__ int stEvalNewInstance(stInstanceExp *name, stRunInstance *i, brEval *t);

__inline__ int stEvalVectorElementExp(stVectorElementExp *s, stRunInstance *i, brEval *t);
__inline__ int stEvalVectorElementAssignExp(stVectorElementAssignExp *s, stRunInstance *i, brEval *target);

__inline__ int stEvalWhile(stWhileExp *w, stRunInstance *i, brEval *target);
__inline__ int stEvalForeach(stForeachExp *w, stRunInstance *i, brEval *target);
__inline__ int stEvalFor(stForExp *w, stRunInstance *i, brEval *target);
__inline__ int stEvalIf(stIfExp *w, stRunInstance *i, brEval *target);

__inline__ int stEvalListInsert(stListInsertExp *w, stRunInstance *i, brEval *target);
__inline__ int stEvalListRemove(stListRemoveExp *l, stRunInstance *i, brEval *target);
__inline__ int stEvalSort(stSortExp *w, stRunInstance *i, brEval *target);
__inline__ int stEvalListIndex(stListIndexExp *l, stRunInstance *i, brEval *t);
__inline__ int stEvalListIndexPointer(stListIndexExp *l, stRunInstance *i, void **pointer, int *type);
__inline__ int stEvalListIndexAssign(stListIndexAssignExp *l, stRunInstance *i, brEval *t);

int stCallMethodByNameWithArgs(stRunInstance *newI, char *name, brEval **args, int argCount, brEval *result);
int stCallMethodByName(stRunInstance *newI, char *name, brEval *result);
__inline__ int stCallMethod(stRunInstance *old, stRunInstance *newI, stMethod *method, brEval **args, int count, brEval *result);

int stLoadVariable(void *variable, unsigned char type, brEval *e, stRunInstance *i);
int stSetVariable(void *variable, unsigned char type, stObject *otype, brEval *e, stRunInstance *i);

int stStackTraceFrame(stStackRecord *r);
void stStackTrace(brEngine *r);

int stGC(brEval *e);

__inline__ int stEvalTruth(brEval *e, brEval *t, stRunInstance *i);

int stPrintEvaluation(brEval *e, stRunInstance *i);

int stEvalBinaryEvalListExp(char op, brEval *l, brEval *r, brEval *target, stRunInstance *i);
brEvalList *stEvalListIndexLookup(brEvalListHead *l, int index);

int stExpEval(stExp *s, stRunInstance *i, brEval *target, stObject **tclass);

void stConvertBreveInstanceToSteveInstance(brEval *e);

int stDoEvalListIndexAssign(brEvalListHead *l, int n, brEval *newVal, stRunInstance *ri);

void stEvalError(brEngine *e, int type, char *proto, ...);

void stConvertSteveInstanceToBreveInstance(brEval *e);

void stEvalWarn(stExp *exp, char *proto, ...);

