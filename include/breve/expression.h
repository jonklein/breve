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

#define RTC_INLINE

enum stExpTypes {
	// basic expression types 

	ET_INT = 1,
	ET_ST_EVAL,
	ET_DOUBLE,
	ET_VECTOR,
	ET_VECTOR_ELEMENT,
	ET_VECTOR_ELEMENT_ASSIGN,
	ET_MATRIX,
	ET_STRING,
	ET_EXP_ARRAY,	/* 9 */

	/* math/binary expressions */

	ET_BINARY,
	ET_UNARY,

	/* list operators */

	ET_INSERT,
	ET_REMOVE,
	ET_SORT,
	ET_LIST,
	ET_LIST_INDEX,
	ET_LIST_INDEX_ASSIGN, 
	ET_COPYLIST,
	ET_APPEND,
	ET_UNAPPEND, /* 20 */

	/* array operators */

	ET_ARRAY_INDEX,
	ET_ARRAY_INDEX_ASSIGN,

	/* control structures */

	ET_WHILE,
	ET_IF,
	ET_FOREACH,
	ET_FOR,
	ET_RETURN,
	ET_CODE_ARRAY, 	/* 28 */

	/* built in special expressions */

	ET_SUPER,
	ET_SELF,
	ET_RANDOM,
	ET_LENGTH,
	ET_INSTANCE,
	ET_FREE,
	ET_PRINT, 
	ET_ERROR,
	ET_DIE,
	ET_ALL, /* 39 */

	/* assign & load */

	ET_ASSIGN,
	ET_LOAD,

	/* method and function calling */

	ET_METHOD,
	ET_FUNC,

	// getting an expression type

	ET_ISA,

	/* a duplicate is just an expression which holds a pointer */
	/* to another expression which is in use elsewhere.  why?  */
	/* because the primary location will be responsible for	*/
	/* memory management of the expression, but the DUPLICATES */
	/* don't bother. */

	ET_DUPLICATE /* 35 */
};

enum stBinaryExpTypes {
	BT_ADD = 1,
	BT_SUB,
	BT_MUL,
	BT_DIV,
	BT_MOD,
	BT_POW,
	BT_UMN,
	BT_RDEC,
	BT_RINC,
	BT_LDEC,
	BT_LINC,

	BT_EQ,
	BT_NE,
	BT_GT,
	BT_GE,
	BT_LT,
	BT_LE,
	BT_LAND,
	BT_LOR
};

enum stUnaryExpTypes {
	UT_MINUS = 1,
	UT_NOT
};

enum stVectorElements {
	VE_X,
	VE_Y,
	VE_Z
};

enum stRtcBlockType	{
	FRAG,		// code fragment
	FUNC,		// function
	PROCEDURE,	// procedure
	LEAF_FUNC,	// function with no stack frame
	LEAF_PROC	// procedure with no stack frame
};

enum stRtcGPRegs {
	GP0		= 0,
	SP		= 1,
};

enum stRtcFlags {
	RTC_EVAL_ARG_3	= 0x1,
	RTC_EVAL_ARG_4	= 0x2,
};
	
typedef struct stRtcCodeBlock_t {
	unsigned char	type;
	unsigned char	flags;
	unsigned short	length;
	union {
		unsigned int	*ptr;
		int (*stRtcEval3)(void *arg0, void *arg1, void *arg2);
		int (*stRtcEval4)(void *arg0, void *arg1, void *arg2, void *arg3);
		int (*stExpEval3)(stExp *s, stRunInstance *i, brEval *target);
		int (*stExpEval)(stExp *s, stRunInstance *i, brEval *target, stObject **tClass);
		int (*stEvalLoadPointer)(stLoadExp *e, stRunInstance *i, void **pointer, int *type);
		int (*stEvalTruth)(brEval *e, brEval *t, stRunInstance *i);
		int (*stEvalFree)(stExp *s, stRunInstance *i, brEval *t);
		int (*stEvalArray)(std::vector< stExp* > *a, stRunInstance *i, brEval *target);
		int (*stEvalMethodCall)(stMethodExp *mexp, stRunInstance *i, brEval *t);
		int (*stEvalWhile)(stWhileExp *w, stRunInstance *i, brEval *target);
		int (*stEvalFor)(stForExp *w, stRunInstance *i, brEval *target);
		int (*stEvalForeach)(stForeachExp *w, stRunInstance *i, brEval *target);
		int (*stEvalIf)(stIfExp *w, stRunInstance *i, brEval *target);
		int (*stEvalListInsert)(stListInsertExp *w, stRunInstance *i, brEval *target);
		int (*stEvalListRemove)(stListRemoveExp *l, stRunInstance *i, brEval *target);
		int (*stEvalCopyList)(stExp *l, stRunInstance *i, brEval *target);
		int (*stEvalAll)(stAllExp *e, stRunInstance *i, brEval *target);
		int (*stEvalSort)(stSortExp *w, stRunInstance *i, brEval *target);
		int (*stEvalListIndexPointer)(stListIndexExp *l, stRunInstance *i, void **pointer, int *type);
		int (*stEvalListIndex)(stListIndexExp *l, stRunInstance *i, brEval *t);
		int (*stEvalListIndexAssign)(stListIndexAssignExp *l, stRunInstance *i, brEval *t);
		int (*stEvalPrint)(stPrintExp *exp, stRunInstance *i, brEval *t);
		int (*stEvalVectorElementExp)(stVectorElementExp *s, stRunInstance *i, brEval *target);
		int (*stEvalVectorElementAssignExp)(stVectorElementAssignExp *s, stRunInstance *i, brEval *target);
		int (*stEvalCallFunc)(stCCallExp *c, stRunInstance *i, brEval *target);
		int (*stEvalArrayIndexPointer)(stArrayIndexExp *a, stRunInstance *i, void **pointer, int *type);
		int (*stEvalArrayIndex)(stArrayIndexExp *a, stRunInstance *i, brEval *target);
		int (*stEvalArrayIndexAssign)(stArrayIndexAssignExp *a, stRunInstance *i, brEval *rvalue);
		int (*stEvalAssignment)(stAssignExp *a, stRunInstance *i, brEval *t);
		int (*stEvalLoad)(stLoadExp *e, stRunInstance *i, brEval *target);
		int (*stEvalLoadInt)(stLoadExp *e, stRunInstance *i, brEval *target);
		int (*stEvalLoadDouble)(stLoadExp *e, stRunInstance *i, brEval *target);
		int (*stEvalLoadIndirect)(stLoadExp *e, stRunInstance *i, brEval *target);
		int (*stEvalLoadList)(stLoadExp *e, stRunInstance *i, brEval *target);
		int (*stEvalLoadHash)(stLoadExp *e, stRunInstance *i, brEval *target);
		int (*stEvalLoadString)(stLoadExp *e, stRunInstance *i, brEval *target);
		int (*stEvalLoadVector)(stLoadExp *e, stRunInstance *i, brEval *target);
		int (*stEvalLoadMatrix)(stLoadExp *e, stRunInstance *i, brEval *target);
		int (*stEvalUnaryExp)(stUnaryExp *b, stRunInstance *i, brEval *target);
		int (*stEvalBinaryStringExp)(char op, brEval *l, brEval *r, brEval *target, stRunInstance *i);
		int (*stEvalBinaryMatrixExp)(char op, brEval *l, brEval *r, brEval *t, stRunInstance *i);
		int (*stEvalBinaryVectorExp)(char op, brEval *l, brEval *r, brEval *t, stRunInstance *i);
		int (*stEvalBinaryDoubleExp)(char op, brEval *l, brEval *r, brEval *t, stRunInstance *i);
		int (*stEvalBinaryIntExp)(char op, brEval *l, brEval *r, brEval *t, stRunInstance *i);
		int (*stEvalBinaryExp)(stBinaryExp *b, stRunInstance *i, brEval *target);
		int (*stEvalRandExp)(stExp *r, stRunInstance *i, brEval *target);
		int (*stEvalBinaryExpWithEvals)(stRunInstance *i, unsigned char op, brEval *tl, brEval *tr, brEval *target);
		int (*stEvalVectorExp)(stVectorExp *v, stRunInstance *i, brEval *target);
		int (*stEvalMatrixExp)(stMatrixExp *v, stRunInstance *i, brEval *target);
		int (*stEvalNewInstance)(stInstanceExp *ie, stRunInstance *i, brEval *t);
		int (*stEvalBinaryEvalListExp)(char op, brEval *l, brEval *r, brEval *target, stRunInstance *i);
	} calls;

	unsigned int	code[0];
} stRtcCodeBlock;

class stExp {
	public:
		union {
			int iValue;
			double dValue;
			void *pValue;
		} values;

		unsigned char type;
		unsigned char debug;

		int line;
		char *file;
	
		stRtcCodeBlock	*block;
};

class stIsaExp : public stExp {
	public:
		stExp *expression;
		stVarType *type;
};

class stStringExp : public stExp {
	public:
		int baseSize;
		char *string;

		std::vector< stSubstringExp* > substrings;
};

class stSubstringExp : public stExp {
	public:
		stExp *loadExp;
		char *string;
		int offset;
		unsigned char retain;
};

class stBinaryExp : public stExp {
	public:
		unsigned char type;
		stExp *left;
		stExp *right;
};

class stUnaryExp : public stExp {
	public:
		unsigned char type;
		stExp *exp;
};

class stVectorExp : public stExp {
	public:
		stExp *x;
		stExp *y;
		stExp *z;
};

class stMatrixExp : public stExp {
	public:
		stExp *expressions[9];
};

class stPrintExp : public stExp {
	public:
		std::vector< stExp* > expressions;
		unsigned char newline;
};

class stVectorElementExp : public stExp {
	public:
		stExp *exp;
		char element;
};

class stVectorElementAssignExp : public stExp {
	public:
		stExp *exp;
		stExp *assignExp;
		char element;
};

class stListInsertExp : public stExp {
	public:
		stExp *exp;
		stExp *listExp;
		stExp *index;
};

class stListRemoveExp : public stExp {
	public:
		stExp *listExp;
		stExp *index;
};

class stSortExp : public stExp {
	public:
		stExp *listExp;
		char *methodName;
};

class stListIndexExp : public stExp {
	public:
		stExp *listExp;
		stExp *indexExp;
};

class stListIndexAssignExp : public stExp {
	public:
		stExp *listExp;
		stExp *indexExp;
		stExp *assignment;
};

class stMethodExp : public stExp {
	public:
		stExp *objectExp;

		char *methodName;
		stMethod *method;
	
		std::vector< stKeyword* > arguments;
		std::vector< stKeyword* > positionedArguments;

		stObject *objectCache;
		stObject *objectTypeCache;
};

class stAssignExp : public stExp {
	public:
		int offset;
		char local;
		unsigned char type;
		stExp *rvalue;
		char *objectName;
		stObject *objectType;
};

class stLoadExp : public stExp {
	public:
		int offset;
		char local;
		unsigned char type;
};

class stArrayExp : public stExp {
	public:
		stArrayExp(std::vector< stExp* > *e) { expressions = *e; }

		~stArrayExp() { 
			unsigned int n;

			for(n = 0; n < expressions.size(); n++)  
				delete expressions[n];
		}

		std::vector< stExp* > expressions;
};

class stArrayIndexAssignExp : public stExp {
	public:
		int offset;
		int maxIndex;
		unsigned char local;
		unsigned char type;
		int typeSize;
		stExp *index;
	
		stExp *rvalue;
};

class stArrayIndexExp : public stExp {
	public:
		int offset;
		int maxIndex;
		unsigned char local;
		unsigned char type;
		int typeSize;
		stExp *index;
};

class stWhileExp : public stExp {
	public:
		stExp *cond;
		stExp *code;
};

class stForeachExp : public stExp {
	public:
		stAssignExp *assignment;
		stExp *list;
		stExp *code;
};

class stForExp : public stExp {
	public:
		stExp *assignment;
		stExp *condition;
		stExp *iteration;
		stExp *code;
};

class stIfExp : public stExp {
	public:
		stExp *cond;
		stExp *trueCode;
		stExp *falseCode;
};

class stAllExp : public stExp {
	public:
		char *name;
		stObject *object;
};

class stInstanceExp : public stExp {
	public:
		char *name;
		stExp *count;
};

class stCCallExp : public stExp {
	public:
		brInternalFunction *function;
		std::vector< stExp* > arguments;
};

class stKeyword : public stExp {
	public:
		char *word;
		stExp *value;
		int position;
};

void stExpFree(stExp *e);

void stExpFreeList(slList *n);

stKeyword *stNewKeyword(char *word, stExp *data);
void stFreeKeyword(stKeyword *k);

stExp *stNewAssignExp(stMethod *m, stObject *o, char *word, stExp *rvalue, char *file, int line);

void stFreeAssignExp(stAssignExp *a);

stExp *stNewLoadExp(stMethod *m, stObject *o, char *word, char *file, int line);

stExp *stNewArrayIndexExp(stMethod *m, stObject *o, char *word, stExp *index, char *file, int line);
void stFreeArrayIndexExp(stArrayIndexExp *e);

stExp *stNewArrayIndexAssignExp(stMethod *m, stObject *o, char *word, stExp *index, stExp *rvalue, char *file, int line);
void stFreeArrayIndexAssignExp(stArrayIndexAssignExp *e);

stExp *stExpNew(void *data, char type, char *file, int line);
stExp *stNewIntExp(int n, char *file, int line);
stExp *stNewDoubleExp(double n, char *file, int line);

stExp *stNewStEvalExp(brEval *e, char *file, int line);

stExp *stNewAllExp(char *object, char *file, int line);
void stFreeAllExp(stAllExp *e);

stExp *stNewCCallExp(brEngine *e, brInternalFunction *s, std::vector< stExp* > *, char *file, int line);

stExp *stNewWhileExp(stExp *cond, stExp *code, char *file, int line);
void stFreeWhileExp(stWhileExp *e);

stExp *stNewForExp(stExp *assignment, stExp *condition, stExp *iteration, stExp *code, char *file, int line);
void stFreeForExp(stForExp *e);

stExp *stInstanceNewExp(char *name, stExp *count, char *file, int line);
void stInstanceFreeExp(stInstanceExp *i);

stExp *stNewForeachExp(stAssignExp *assignment, stExp *list, stExp *code, char *file, int lineno);
void stFreeForeachExp(stForeachExp *e);

stExp *stNewSortExp(stExp *list, char *method, char *file, int lineno);
void stFreeSortExp(stSortExp *e);

stExp *stNewListIndexExp(stExp *list, stExp *index, char *file, int lineno);
void stFreeListIndexExp(stListIndexExp *e);

stExp *stNewListIndexAssignExp(stExp *list, stExp *index, stExp *assignment, char *file, int lineno);
void stFreeListIndexAssignExp(stListIndexAssignExp *e);

stExp *stNewListInsertExp(stExp *list, stExp *exp, stExp *index, char *file, int lineno);
void stFreeListInsertExp(stListInsertExp *e);

stExp *stNewListRemoveExp(stExp *list, stExp *index, char *file, int lineno);
void stFreeListRemoveExp(stListRemoveExp *e);

stExp *stNewIfExp(stExp *cond, stExp *trueCode, stExp *falseCode, char *file, int lineno);
void stFreeIfExp(stIfExp *e);

stMethodExp *stNewMethodCall(stObject *o, stExp *expression, char *method, std::vector< stKeyword* > *arguments);
void stFreeMethodExp(stMethodExp *m);

void stFreeCCallExp(stCCallExp *m);
 
stExp *stNewBinaryExp(int type, stExp *left, stExp *right, char *f, int line);
void stFreeBinaryExp(stBinaryExp *e);

stExp *stNewUnaryExp(int type, stExp *exp, char *f, int line);
void stFreeUnaryExp(stUnaryExp *e);

stExp *stNewVectorExp(stExp *x, stExp *y, stExp *z, char *file, int line);
void stFreeVectorExp(stVectorExp *v);

stExp *stNewMatrixExp(stExp *e00, stExp *e01, stExp *e02, stExp *e10, stExp *e11, stExp *e12, stExp *e20, stExp *e21, stExp *e22, char *file, int line);
void stFreeMatrixExp(stMatrixExp *m);

stExp *stNewPrintExp(std::vector< stExp* > *expressions, int newline, char *file, int line);
void stFreePrintExp(stPrintExp *m);

stExp *stNewVectorElementExp(stExp *v, char element, char *file, int line);
void stFreeVectorElementExp(stVectorElementExp *v);

stExp *stNewVectorElementAssignExp(stExp *v, stExp *rvalue, char element, char *file, int line);
void stFreeVectorElementAssignExp(stVectorElementAssignExp *v);

stExp *stNewDuplicateExp(stExp *dupe, char *file, int line);

stExp *stNewIsaExp(stExp *exp, stVarType *type, char *file, int line);
void stFreeIsaExp(stIsaExp *isa);

char *dequote(char *c);

int stSizeof(stVarType *v);
int stSizeofAtomic(int a);

int stAlign(stVarType *v);
int stAlignAtomic(int a);
