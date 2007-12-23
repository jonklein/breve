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

#define USE_RTC 1

#if USE_RTC
#define EVAL_RTC_CALL_3(_s, _call, _arg0, _arg1, _arg2)		\
	((_s)->block = stNewRtcBlock(),				\
	 (_s)->block->flags |= RTC_EVAL_ARG_3,			\
	 (_s)->block->calls._call = (_call),			\
	 (_s)->block->calls._call((_arg0), (_arg1), (_arg2)))

#define RTC_SET_LOAD_ROUTINE(_loadExp, _call, _type, _variable) \
if (_loadExp) { \
	_loadExp->calls.stEvalLoadFunc = _call; \
	_loadExp->type           = _type; \
	_loadExp->variable       = _variable; \
}
#else
#define EVAL_RTC_CALL_3(_s, _call, _arg0, _arg1, _arg2) _call(_arg0, _arg1, _arg2)
#define RTC_SET_LOAD_ROUTINE(_loadExp, _call, _type, _variable)
#endif

inline int stEvalFree(stFreeExp *, stRunInstance *, brEval *);
inline int stEvalCodeArray(stCodeArrayExp *a, stRunInstance *, brEval *);
inline int stEvalExpVector(std::vector< stExp* > *a, stRunInstance *, brEval *);
inline int stEvalMethodCall(stMethodExp *, stRunInstance *, brEval *);
inline int stRealEvalMethodCall(stMethodExp *, stRunInstance *, stRunInstance *, brEval *);
int stEvalForeignMethodCall(stMethodExp *, brInstance *, stRunInstance *, brEval *);

inline int stEvalCallFunc(stCCallExp *, stRunInstance *, brEval *);

inline int stEvalAssignment(stAssignExp *, stRunInstance *, brEval *);

inline int stEvalListExp(stListExp *, stRunInstance *, brEval *);

inline int stPointerForExp(stExp *, stRunInstance *, void **, int *);
inline int stEvalLoadPointer(stLoadExp *, stRunInstance *, void **, int *);
inline int stEvalArrayIndexPointer(stArrayIndexExp *, stRunInstance *, void **, int *);

int stEvalLoad(stLoadExp *, stRunInstance *, brEval *);

inline int stEvalArrayIndexAssign(stArrayIndexAssignExp *, stRunInstance *, brEval *);
inline int stEvalArrayIndex(stArrayIndexExp *, stRunInstance *, brEval *);

inline int stEvalPrint(stPrintExp *, stRunInstance *, brEval *);
inline int stEvalRandExp(stRandomExp *, stRunInstance *, brEval *);

int stToDouble(brEval *, brEval *, stRunInstance *);
inline int stToInt(brEval *, brEval *, stRunInstance *);
inline int stToType(brEval *, int, brEval *, stRunInstance *);

inline int stEvalVectorExp(stVectorExp *, stRunInstance *, brEval *);
inline int stEvalMatrixExp(stMatrixExp *, stRunInstance *, brEval *);
inline int stEvalBinaryExp(stBinaryExp *, stRunInstance *, brEval *);
inline int stEvalBinaryExpWithEvals(stRunInstance *, unsigned char, brEval *, brEval *, brEval *);

inline int stEvalUnaryExp(stUnaryExp *, stRunInstance *, brEval *);

inline int stEvalBinaryStringExp(char, brEval *, brEval *, brEval *, stRunInstance *);
inline int stEvalBinaryVectorExp(char, brEval *, brEval *, brEval *, stRunInstance *);
inline int stEvalBinaryMatrixExp(char, brEval *, brEval *, brEval *, stRunInstance *);
inline int stEvalBinaryDoubleExp(char, brEval *, brEval *, brEval *, stRunInstance *);
inline int stEvalBinaryIntExp(char, brEval *, brEval *, brEval *, stRunInstance *);

inline int stEvalNewInstance(stInstanceExp *, stRunInstance *, brEval *);

inline int stEvalVectorElementExp(stVectorElementExp *, stRunInstance *, brEval *);
inline int stEvalVectorElementAssignExp(stVectorElementAssignExp *, stRunInstance *, brEval *);

inline int stEvalWhile(stWhileExp *, stRunInstance *, brEval *);
inline int stEvalForeach(stForeachExp *, stRunInstance *, brEval *);
inline int stEvalFor(stForExp *, stRunInstance *, brEval *);
inline int stEvalIf(stIfExp *, stRunInstance *, brEval *);

inline int stEvalListInsert(stListInsertExp *, stRunInstance *, brEval *);
inline int stEvalListRemove(stListRemoveExp *, stRunInstance *, brEval *);
inline int stEvalSort(stSortExp *, stRunInstance *, brEval *);
inline int stEvalListIndexPointer(stListIndexExp *, stRunInstance *, void **, int *);

inline int stEvalIndexAssign(stListIndexAssignExp *, stRunInstance *, brEval *);
inline int stEvalIndexLookup(stListIndexExp *, stRunInstance *, brEval *);

inline int stEvalEvalExp( stEvalExp *inEval, stRunInstance *inInstance, brEval *outResult );

int stCallMethod(stRunInstance *, stRunInstance *, stMethod *, const brEval **, int, brEval *);
int stCallMethodByName(stRunInstance *, char *, brEval *);
extern "C" DLLEXPORT int stCallMethodByNameWithArgs(stRunInstance *, char *, const brEval **, int, brEval *);

int stLoadVariable(void *, unsigned char, brEval *, stRunInstance *);
int stSetVariable(void *, unsigned char, stObject *, brEval *, stRunInstance *);

int stStackTraceFrame(stStackRecord *);
void stStackTrace(stSteveData *);

int stGC(brEval *);

inline int stEvalTruth(brEval *, brEval *, stRunInstance *);

int stPrintEvaluation(brEval *, stRunInstance *);

int stEvalBinaryEvalListExp(char, brEval *, brEval *, brEval *, stRunInstance *);
//brEvalList* stEvalListIndexLookup(brEvalListHead *, int);

// int stExpEval3(stExp *, stRunInstance *, brEval *);

int stExpEval(stExp *, stRunInstance *, brEval *, stObject **);

void stConvertBreveInstanceToSteveInstance(brEval *);

int stEvalListIndexAssign(brEvalListHead *, int, brEval *, stRunInstance *);

void stEvalError( stInstance*, int, char *, ... );

void stConvertSteveInstanceToBreveInstance(brEval *);

void stEvalWarn(stExp *, char *, ...);
