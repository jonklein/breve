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

#ifndef _EXPRESSION_H
#define _EXPRESSION_H

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
		int (*stEvalFree)(stFreeExp *s, stRunInstance *i, brEval *t);
		int (*stEvalCodeArray)(stCodeArrayExp *a, stRunInstance *i, brEval *target);
		int (*stEvalMethodCall)(stMethodExp *mexp, stRunInstance *i, brEval *t);
		int (*stEvalWhile)(stWhileExp *w, stRunInstance *i, brEval *target);
		int (*stEvalFor)(stForExp *w, stRunInstance *i, brEval *target);
		int (*stEvalForeach)(stForeachExp *w, stRunInstance *i, brEval *target);
		int (*stEvalIf)(stIfExp *w, stRunInstance *i, brEval *target);
		int (*stEvalListInsert)(stListInsertExp *w, stRunInstance *i, brEval *target);
		int (*stEvalListRemove)(stListRemoveExp *l, stRunInstance *i, brEval *target);
		int (*stEvalAll)(stAllExp *e, stRunInstance *i, brEval *target);
		int (*stEvalSort)(stSortExp *w, stRunInstance *i, brEval *target);
		int (*stEvalListIndexPointer)(stListIndexExp *l, stRunInstance *i, void **pointer, int *type);
		int (*stEvalListIndex)(stListIndexExp *l, stRunInstance *i, brEval *t);
		int (*stEvalListIndexAssign)(stListIndexAssignExp *l, stRunInstance *i, brEval *t);
		int (*stEvalPrint)(stPrintExp *exp, stRunInstance *i, brEval *t);
		int (*stEvalCopyList)(stCopyListExp *exp, stRunInstance *i, brEval *t);
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
		stExp(char *f, int l) {
			block = NULL;
			line = l;
			file = f;
		}

		virtual ~stExp() { 
			if(block) free(block);	
		}

		unsigned char type;
		unsigned char debug;

		int line;
		char *file;
	
		stRtcCodeBlock	*block;
};

class stRandomExp : public stExp {
	public:
		stRandomExp(stExp *e, char *file, int line) : stExp(file, line) {
			expression = e;
			type = ET_RANDOM;
		}

		~stRandomExp() {
			delete expression;
		}

		stExp *expression;
};

class stListExp : public stExp {
	public:
		stListExp(std::vector< stExp* > *e, char *file, int line) : stExp(file, line) {
			if(e) expressions = *e;
			type = ET_LIST;
		}

		std::vector< stExp* > expressions;
};

class stCodeArrayExp : public stExp {
	public:
		stCodeArrayExp(std::vector< stExp* > *e, char *file, int line) : stExp(file, line) {
			if(e) expressions = *e;
			type = ET_CODE_ARRAY;
		}

		std::vector< stExp* > expressions;
};

class stCopyListExp : public stExp {
	public:
		stCopyListExp(stExp *e, char *file, int line);
		~stCopyListExp();

		stExp *expression;
};

class stDieExp : public stExp {
	public:
		stDieExp(stExp *e, char *file, int line) : stExp(file, line) { 
			expression = e;
			type = ET_DIE; 
		}

		stExp *expression;
};

class stSelfExp : public stExp {
	public:
		stSelfExp(char *file, int line) : stExp(file, line) { type = ET_SELF; }
};

class stSuperExp : public stExp {
	public:
		stSuperExp(char *file, int line) : stExp(file, line) { type = ET_SUPER; }
};

class stFreeExp : public stExp {
	public:
		stFreeExp(stExp *e, char *file, int line);
		~stFreeExp();

		stExp *expression;
};

class stEvalExp : public stExp {
	public:
		stEvalExp(brEval *e, char *file, int line);
		~stEvalExp();

		brEval *eval;
};

class stIntExp : public stExp {
	public:
		stIntExp(int i, char *file, int line);

		int intValue;
};

class stReturnExp : public stExp {
	public:
		stReturnExp(stExp *e, char *file, int line);
		~stReturnExp();

		stExp *expression;
};

class stLengthExp : public stExp {
	public:
		stLengthExp(stExp *e, char *file, int line);
		~stLengthExp();

		stExp *expression;
};

class stDoubleExp : public stExp {
	public:
		stDoubleExp(double d, char *file, int line);

		double doubleValue;
};

class stStringExp : public stExp {
	public:
		stStringExp(char *str, stMethod *m, stObject *o, char *file, int line);
		~stStringExp();

		int baseSize;
		char *string;

		std::vector< stSubstringExp* > substrings;
};

class stSubstringExp : public stExp {
	public:
		stSubstringExp(char *file, int line) : stExp(file, line) {};

		stExp *loadExp;
		char *string;
		int offset;
		unsigned char retain;
};

class stBinaryExp : public stExp {
	public:
		stBinaryExp(unsigned char o, stExp *le, stExp *re, char *f, int l);
		~stBinaryExp();

		unsigned char op;
		stExp *left;
		stExp *right;
};

class stUnaryExp : public stExp {
	public:
		stUnaryExp(unsigned char o, stExp *e, char *f, int l);
		~stUnaryExp();

		unsigned char op;
		stExp *expression;
};

class stVectorExp : public stExp {
	public:
		stVectorExp(stExp *x, stExp *y, stExp *z, char *f, int l);
		~stVectorExp();

		stExp *_x;
		stExp *_y;
		stExp *_z;
};

class stMatrixExp : public stExp {
	public:
		stMatrixExp(stExp *e00, stExp *e01, stExp *e02, stExp *e10, stExp *e11, stExp *e12, stExp *e20, stExp *e21, stExp *e22, char *file, int line);
		~stMatrixExp();

		stExp *expressions[9];
};

class stPrintExp : public stExp {
	public:
		stPrintExp(std::vector< stExp* > *expressions, int newline, char *file, int lineno);
		~stPrintExp();

		std::vector< stExp* > expressions;
		unsigned char newline;
};

class stVectorElementExp : public stExp {
	public:
		stVectorElementExp(stExp *v, char e, char *file, int line);
		~stVectorElementExp();

		stExp *exp;
		char element;
};

class stDuplicateExp : public stExp {
	public:
		stDuplicateExp(stExp *e, char *file, int l);

		stExp *expression;
};

class stVectorElementAssignExp : public stExp {
	public:
		stVectorElementAssignExp(stExp *v, stExp *rvalue, char element, char *file, int line);
		~stVectorElementAssignExp();

		stExp *exp;
		stExp *assignExp;
		char element;
};

class stListInsertExp : public stExp {
	public:
		stListInsertExp(stExp *le, stExp *ee, stExp *ie, char *file, int l);
		~stListInsertExp();

		stExp *exp;
		stExp *listExp;
		stExp *index;
};

class stListRemoveExp : public stExp {
	public:
		stListRemoveExp(stExp *list, stExp *index, char *file, int lineno);
		~stListRemoveExp();

		stExp *listExp;
		stExp *index;
};

class stSortExp : public stExp {
	public:
		stSortExp(stExp *list, char *method, char *file, int lineno);
		~stSortExp();

		stExp *listExp;
		char *methodName;
};

class stListIndexExp : public stExp {
	public:
		stListIndexExp(stExp *list, stExp *index, char *file, int lineno);
		~stListIndexExp();

		stExp *listExp;
		stExp *indexExp;
};

class stListIndexAssignExp : public stExp {
	public:
		stListIndexAssignExp(stExp *list, stExp *index, stExp *assignment, char *file, int lineno);
		~stListIndexAssignExp();

		stExp *listExp;
		stExp *indexExp;
		stExp *assignment;
};

class stMethodExp : public stExp {
	public:
		stMethodExp(stExp *o, char *n, std::vector<stKeyword*> *a, char *f, int l);

		~stMethodExp();

		stExp *objectExp;

		char *methodName;
	
		std::vector< stKeyword* > arguments;
		std::vector< stKeyword* > positionedArguments;

		stMethod *method;
		stObject *objectCache;
		stObject *objectTypeCache;
};

class stAssignExp : public stExp {
	public:
		stAssignExp(stMethod *m, stObject *o, char *word, stExp *rvalue, char *file, int line);
		~stAssignExp();

		int offset;
		char local;
		unsigned char assignType;
		stExp *rvalue;
		char *objectName;
		stObject *objectType;
};

class stLoadExp : public stExp {
	public:
		stLoadExp(stMethod *m, stObject *o, char *word, char *f, int l);

		int offset;
		char local;
		unsigned char loadType;
};

class stArrayExp : public stExp {
	public:
		stArrayExp(std::vector< stExp* > *e, char *f, int l) : stExp(f, l) {
			expressions = *e; 
			type = ET_EXP_ARRAY;
		}

		~stArrayExp() { 
			unsigned int n;

			for(n = 0; n < expressions.size(); n++)  
				delete expressions[n];
		}

		std::vector< stExp* > expressions;
};

class stArrayIndexAssignExp : public stExp {
	public:
		stArrayIndexAssignExp(stMethod *m, stObject *o, char *word, stExp *i, stExp *rv, char *file, int line);
		~stArrayIndexAssignExp();

		int offset;
		int maxIndex;
		unsigned char local;
		unsigned char assignType;
		int typeSize;
		stExp *index;
	
		stExp *rvalue;
};

class stArrayIndexExp : public stExp {
	public:
		stArrayIndexExp(stMethod *m, stObject *o, char *word, stExp *i, char *file, int line);
		~stArrayIndexExp();

		int offset;
		int maxIndex;
		unsigned char local;
		unsigned char loadType;
		int typeSize;
		stExp *index;
};

class stWhileExp : public stExp {
	public:
		stWhileExp(stExp *cn, stExp *cd, char *file, int lineno);
		~stWhileExp();

		stExp *cond;
		stExp *code;
};

class stForeachExp : public stExp {
	public:
		stForeachExp(stAssignExp *a, stExp *l, stExp *c, char *file, int lineno);
		~stForeachExp();

		stAssignExp *assignment;
		stExp *list;
		stExp *code;
};

class stForExp : public stExp {
	public:
		stForExp(stExp *a, stExp *cn, stExp *i, stExp *cd, char *file, int lineno);
		~stForExp();

		stExp *assignment;
		stExp *condition;
		stExp *iteration;
		stExp *code;
};

class stIfExp : public stExp {
	public:
		stIfExp(stExp *c, stExp *t, stExp *f, char *file, int lineno);
		~stIfExp();

		stExp *cond;
		stExp *trueCode;
		stExp *falseCode;
};

class stAllExp : public stExp {
	public:
		stAllExp(char *objectName, char *file, int line);
		~stAllExp();

		char *name;
		stObject *object;
};

class stInstanceExp : public stExp {
	public:
		stInstanceExp(char *n, stExp *c, char *f, int l) : stExp(f, l) {
			name = strdup(n);
			count = c;

			type = ET_INSTANCE;
		}

		~stInstanceExp() {
			free(name);
			delete count;
		}

		char *name;
		stExp *count;
};

class stCCallExp : public stExp {
	public:
		stCCallExp(brEngine *e, brInternalFunction *s, std::vector< stExp* > *expressions, char *file, int line);
		~stCCallExp();

		brInternalFunction *function;
		std::vector< stExp* > arguments;
};

class stKeyword {
	public:
		stKeyword(char *w, stExp *v) {
			word = strdup(w);
			value = v;
		}

		~stKeyword() {
			free(word);
			delete value;
		}

		char *word;
		stExp *value;
		int position;
};

char *dequote(char *c);

int stSizeof(stVarType *v);
int stSizeofAtomic(int a);

int stAlign(stVarType *v);
int stAlignAtomic(int a);

#endif
