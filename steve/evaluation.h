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

__inline__ int stEvalFree(stExp *, stRunInstance *, brEval *);
__inline__ int stEvalArray(slArray *, stRunInstance *, brEval *);
__inline__ int stEvalMethodCall(stMethodExp *, stRunInstance *, brEval *);
__inline__ int stRealEvalMethodCall(stMethodExp *, stRunInstance *, stRunInstance *, brEval *);
int stEvalForeignMethodCall(stMethodExp *, brInstance *, stRunInstance *, brEval *);

__inline__ int stEvalCallFunc(stCCallExp *, stRunInstance *, brEval *);

__inline__ int stEvalAssignment(stAssignExp *, stRunInstance *, brEval *);

__inline__ int stEvalListExp(slList *, stRunInstance *, brEval *);

__inline__ int stPointerForExp(stExp *, stRunInstance *, void **, int *);
__inline__ int stEvalLoadPointer(stLoadExp *, stRunInstance *, void **, int *);
__inline__ int stEvalArrayIndexPointer(stArrayIndexExp *, stRunInstance *, void **, int *);

int stEvalLoad(stLoadExp *, stRunInstance *, brEval *);

__inline__ int stEvalArrayIndexAssign(stArrayIndexAssignExp *, stRunInstance *, brEval *);
__inline__ int stEvalArrayIndex(stArrayIndexExp *, stRunInstance *, brEval *);

__inline__ int stEvalPrint(stPrintExp *, stRunInstance *, brEval *);
__inline__ int stEvalRandExp(stExp *, stRunInstance *, brEval *);

__inline__ int stToDouble(brEval *, brEval *, stRunInstance *);
__inline__ int stToInt(brEval *, brEval *, stRunInstance *);
__inline__ int stToType(brEval *, int, brEval *, stRunInstance *);

__inline__ int stEvalVectorExp(stVectorExp *, stRunInstance *, brEval *);
__inline__ int stEvalMatrixExp(stMatrixExp *, stRunInstance *, brEval *);
__inline__ int stEvalBinaryExp(stBinaryExp *, stRunInstance *, brEval *);
__inline__ int stEvalBinaryExpWithEvals(stRunInstance *, unsigned char, brEval *, brEval *, brEval *);

__inline__ int stEvalUnaryExp(stUnaryExp *, stRunInstance *, brEval *);

__inline__ int stEvalBinaryStringExp(char, brEval *, brEval *, brEval *, stRunInstance *);
__inline__ int stEvalBinaryVectorExp(char, brEval *, brEval *, brEval *, stRunInstance *);
__inline__ int stEvalBinaryMatrixExp(char, brEval *, brEval *, brEval *, stRunInstance *);
__inline__ int stEvalBinaryDoubleExp(char, brEval *, brEval *, brEval *, stRunInstance *);
__inline__ int stEvalBinaryIntExp(char, brEval *, brEval *, brEval *, stRunInstance *);

__inline__ int stEvalNewInstance(stInstanceExp *, stRunInstance *, brEval *);

__inline__ int stEvalVectorElementExp(stVectorElementExp *, stRunInstance *, brEval *);
__inline__ int stEvalVectorElementAssignExp(stVectorElementAssignExp *, stRunInstance *, brEval *);

__inline__ int stEvalWhile(stWhileExp *, stRunInstance *, brEval *);
__inline__ int stEvalForeach(stForeachExp *, stRunInstance *, brEval *);
__inline__ int stEvalFor(stForExp *, stRunInstance *, brEval *);
__inline__ int stEvalIf(stIfExp *, stRunInstance *, brEval *);

__inline__ int stEvalListInsert(stListInsertExp *, stRunInstance *, brEval *);
__inline__ int stEvalListRemove(stListRemoveExp *, stRunInstance *, brEval *);
__inline__ int stEvalSort(stSortExp *, stRunInstance *, brEval *);
__inline__ int stEvalListIndex(stListIndexExp *, stRunInstance *, brEval *);
__inline__ int stEvalListIndexPointer(stListIndexExp *, stRunInstance *, void **, int *);
__inline__ int stEvalListIndexAssign(stListIndexAssignExp *, stRunInstance *, brEval *);

int stCallMethodByNameWithArgs(stRunInstance *, char *, brEval **, int, brEval *);
int stCallMethodByName(stRunInstance *, char *, brEval *);
__inline__ int stCallMethod(stRunInstance *, stRunInstance *, stMethod *, brEval **, int, brEval *);

int stLoadVariable(void *, unsigned char, brEval *, stRunInstance *);
int stSetVariable(void *, unsigned char, stObject *, brEval *, stRunInstance *);

int stStackTraceFrame(stStackRecord *);
void stStackTrace(stSteveData *);

int stGC(brEval *);

__inline__ int stEvalTruth(brEval *, brEval *, stRunInstance *);

int stPrintEvaluation(brEval *, stRunInstance *);

int stEvalBinaryEvalListExp(char, brEval *, brEval *, brEval *, stRunInstance *);
brEvalList *stEvalListIndexLookup(brEvalListHead *, int);

int stExpEval(stExp *, stRunInstance *, brEval *, stObject **);

void stConvertBreveInstanceToSteveInstance(brEval *);

int stDoEvalListIndexAssign(brEvalListHead *, int, brEval *, stRunInstance *);

void stEvalError(brEngine *, int, char *, ...);

void stConvertSteveInstanceToBreveInstance(brEval *);

void stEvalWarn(stExp *, char *, ...);
