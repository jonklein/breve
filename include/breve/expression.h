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

#include <vector>

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
		int (*stRtcEval3)(void *, void *, void *);
		int (*stRtcEval4)(void *, void *, void *, void *);
		int (*stExpEval3)(stExp *, stRunInstance *, brEval *);
		int (*stExpEval)(stExp *, stRunInstance *, brEval *, stObject **);
		int (*stEvalLoadPointer)(stLoadExp *, stRunInstance *, void **, int *);
		int (*stEvalTruth)(brEval *, brEval *, stRunInstance *);
		int (*stEvalFree)(stFreeExp *, stRunInstance *, brEval *);
		int (*stEvalCodeArray)(stCodeArrayExp *, stRunInstance *, brEval *);
		int (*stEvalMethodCall)(stMethodExp *, stRunInstance *, brEval *);
		int (*stEvalWhile)(stWhileExp *, stRunInstance *, brEval *);
		int (*stEvalFor)(stForExp *, stRunInstance *, brEval *);
		int (*stEvalForeach)(stForeachExp *, stRunInstance *, brEval *);
		int (*stEvalIf)(stIfExp *, stRunInstance *, brEval *);
		int (*stEvalListInsert)(stListInsertExp *, stRunInstance *, brEval *);
		int (*stEvalListRemove)(stListRemoveExp *, stRunInstance *, brEval *);
		int (*stEvalAll)(stAllExp *, stRunInstance *, brEval *);
		int (*stEvalSort)(stSortExp *, stRunInstance *, brEval *);
		int (*stEvalListIndexPointer)(stListIndexExp *, stRunInstance *, void **, int *);
		int (*stEvalListIndex)(stListIndexExp *, stRunInstance *, brEval *);
		int (*stEvalListIndexAssign)(stListIndexAssignExp *, stRunInstance *, brEval *);
		int (*stEvalPrint)(stPrintExp *, stRunInstance *, brEval *);
		int (*stEvalCopyList)(stCopyListExp *, stRunInstance *, brEval *);
		int (*stEvalVectorElementExp)(stVectorElementExp *, stRunInstance *, brEval *);
		int (*stEvalVectorElementAssignExp)(stVectorElementAssignExp *, stRunInstance *, brEval *);
		int (*stEvalCallFunc)(stCCallExp *, stRunInstance *, brEval *);
		int (*stEvalArrayIndexPointer)(stArrayIndexExp *, stRunInstance *, void **, int *);
		int (*stEvalArrayIndex)(stArrayIndexExp *, stRunInstance *, brEval *);
		int (*stEvalArrayIndexAssign)(stArrayIndexAssignExp *, stRunInstance *, brEval *);
		int (*stEvalAssignment)(stAssignExp *, stRunInstance *, brEval *);
		int (*stEvalLoad)(stLoadExp *, stRunInstance *, brEval *);
		int (*stEvalLoadInt)(stLoadExp *, stRunInstance *, brEval *);
		int (*stEvalLoadDouble)(stLoadExp *, stRunInstance *, brEval *);
		int (*stEvalLoadIndirect)(stLoadExp *, stRunInstance *, brEval *);
		int (*stEvalLoadList)(stLoadExp *, stRunInstance *, brEval *);
		int (*stEvalLoadHash)(stLoadExp *, stRunInstance *, brEval *);
		int (*stEvalLoadString)(stLoadExp *, stRunInstance *, brEval *);
		int (*stEvalLoadVector)(stLoadExp *, stRunInstance *, brEval *);
		int (*stEvalLoadMatrix)(stLoadExp *, stRunInstance *, brEval *);
		int (*stEvalUnaryExp)(stUnaryExp *, stRunInstance *, brEval *);
		int (*stEvalBinaryStringExp)(char, brEval *, brEval *, brEval *, stRunInstance *);
		int (*stEvalBinaryMatrixExp)(char, brEval *, brEval *, brEval *, stRunInstance *);
		int (*stEvalBinaryVectorExp)(char, brEval *, brEval *, brEval *, stRunInstance *);
		int (*stEvalBinaryDoubleExp)(char, brEval *, brEval *, brEval *, stRunInstance *);
		int (*stEvalBinaryIntExp)(char, brEval *, brEval *, brEval *, stRunInstance *);
		int (*stEvalBinaryExp)(stBinaryExp *, stRunInstance *i, brEval *);
		int (*stEvalRandExp)(stRandomExp *, stRunInstance *, brEval *);
		int (*stEvalBinaryExpWithEvals)(stRunInstance *, unsigned char, brEval *, brEval *, brEval *);
		int (*stEvalVectorExp)(stVectorExp *, stRunInstance *, brEval *);
		int (*stEvalMatrixExp)(stMatrixExp *, stRunInstance *, brEval *);
		int (*stEvalNewInstance)(stInstanceExp *, stRunInstance *, brEval *);
		int (*stEvalBinaryEvalListExp)(char, brEval *, brEval *, brEval *, stRunInstance *);
	} calls;

	unsigned int	code[0];
} stRtcCodeBlock;

class stExp {
	public:
		stExp( const char *file, int l ) {
			block = NULL;
			line = l;
			file = file;

			debug = false;
			block = NULL;
		}

		virtual ~stExp() { 
			if (block)
				free(block);	
		}

		unsigned char type;

		bool debug;

		int line;
		const char *file;
	
		stRtcCodeBlock	*block;
};

class stRandomExp : public stExp {
	public:
		stRandomExp(stExp *e, const char *file, int line) : stExp(file, line) {
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
		stListExp(std::vector< stExp* > *e, const char *file, int line) : stExp(file, line) {
			if(e) expressions = *e;
			type = ET_LIST;
		}

		std::vector< stExp* > expressions;
};

class stCodeArrayExp : public stExp {
	public:
		stCodeArrayExp(std::vector< stExp* > *e, const char *file, int line) : stExp(file, line) {
			if(e) expressions = *e;
			type = ET_CODE_ARRAY;
		}

		std::vector< stExp* > expressions;
};

class stCopyListExp : public stExp {
	public:
		stCopyListExp(stExp *e, const char *file, int line);
		~stCopyListExp();

		stExp *expression;
};

class stDieExp : public stExp {
	public:
		stDieExp(stExp *e, const char *file, int line) : stExp(file, line) { 
			expression = e;
			type = ET_DIE; 
		}

		stExp *expression;
};

class stSelfExp : public stExp {
	public:
		stSelfExp(const char *file, int line) : stExp(file, line) { type = ET_SELF; }
};

class stSuperExp : public stExp {
	public:
		stSuperExp(const char *file, int line) : stExp(file, line) { type = ET_SUPER; }
};

class stFreeExp : public stExp {
	public:
		stFreeExp(stExp *e, const char *file, int line);
		~stFreeExp();

		stExp *expression;
};

class stEvalExp : public stExp {
	public:
		stEvalExp(brEval *e, const char *file, int line);
		~stEvalExp();

		brEval *eval;
};

class stIntExp : public stExp {
	public:
		stIntExp(int i, const char *file, int line);

		int intValue;
};

class stReturnExp : public stExp {
	public:
		stReturnExp(stExp *e, const char *file, int line);
		~stReturnExp();

		stExp *expression;
};

class stLengthExp : public stExp {
	public:
		stLengthExp(stExp *e, const char *file, int line);
		~stLengthExp();

		stExp *expression;
};

class stDoubleExp : public stExp {
	public:
		stDoubleExp(double d, const char *file, int line);

		double doubleValue;
};

class stStringExp : public stExp {
	public:
		stStringExp( char *str, stMethod *m, stObject *o, const char *file, int line );
		~stStringExp();

		int baseSize;
		std::string string;

		std::vector< stSubstringExp* > substrings;
};

class stSubstringExp : public stExp {
	public:
		stSubstringExp( const char *file, int line ) : stExp(file, line) {};

		stExp *loadExp;
		std::string string;
		int offset;
		unsigned char retain;
};

class stBinaryExp : public stExp {
	public:
		stBinaryExp( unsigned char o, stExp *le, stExp *re, const char *file, int line );
		~stBinaryExp();

		unsigned char op;
		stExp *left;
		stExp *right;
};

class stUnaryExp : public stExp {
	public:
		stUnaryExp( unsigned char o, stExp *e, const char *file, int line );
		~stUnaryExp();

		unsigned char op;
		stExp *expression;
};

class stVectorExp : public stExp {
	public:
		stVectorExp( stExp *x, stExp *y, stExp *z, const char *file, int line );
		~stVectorExp();

		stExp *_x;
		stExp *_y;
		stExp *_z;
};

class stMatrixExp : public stExp {
	public:
		stMatrixExp( stExp *e00, stExp *e01, stExp *e02, stExp *e10, stExp *e11, stExp *e12, stExp *e20, stExp *e21, stExp *e22, const char *file, int line );
		~stMatrixExp();

		stExp *expressions[9];
};

class stPrintExp : public stExp {
	public:
		stPrintExp(std::vector< stExp* > *expressions, int newline, const char *file, int lineno);
		~stPrintExp();

		std::vector< stExp* > expressions;
		unsigned char newline;
};

class stVectorElementExp : public stExp {
	public:
		stVectorElementExp(stExp *v, char e, const char *file, int line);
		~stVectorElementExp();

		stExp *exp;
		char element;
};

class stDuplicateExp : public stExp {
	public:
		stDuplicateExp(stExp *e, const char *file, int l);

		stExp *expression;
};

class stVectorElementAssignExp : public stExp {
	public:
		stVectorElementAssignExp(stExp *v, stExp *rvalue, char element, const char *file, int line);
		~stVectorElementAssignExp();

		stExp *exp;
		stExp *assignExp;
		char element;
};

class stListInsertExp : public stExp {
	public:
		stListInsertExp(stExp *le, stExp *ee, stExp *ie, const char *file, int l);
		~stListInsertExp();

		stExp *exp;
		stExp *listExp;
		stExp *index;
};

class stListRemoveExp : public stExp {
	public:
		stListRemoveExp(stExp *list, stExp *index, const char *file, int lineno);
		~stListRemoveExp();

		stExp *listExp;
		stExp *index;
};

class stSortExp : public stExp {
	public:
		stSortExp(stExp *list, char *method, const char *file, int lineno);
		~stSortExp();

		stExp *listExp;
		std::string methodName;
};

class stListIndexExp : public stExp {
	public:
		stListIndexExp(stExp *list, stExp *index, const char *file, int lineno);
		~stListIndexExp();

		stExp *listExp;
		stExp *indexExp;
};

class stListIndexAssignExp : public stExp {
	public:
		stListIndexAssignExp(stExp *list, stExp *index, stExp *assignment, const char *file, int lineno);
		~stListIndexAssignExp();

		stExp *listExp;
		stExp *indexExp;
		stExp *assignment;
};

class stMethodExp : public stExp {
	public:
		stMethodExp( stExp *o, char *n, std::vector<stKeyword*> *a, const char *file, int line );

		~stMethodExp();

		stExp *objectExp;

		std::string methodName;
	
		std::vector< stKeyword* > arguments;
		std::vector< stKeyword* > positionedArguments;

		stMethod *method;
		stObject *objectCache;
		stObject *objectTypeCache;
};

class stAssignExp : public stExp {
	public:
		stAssignExp(stMethod *m, stObject *o, char *word, stExp *rvalue, const char *file, int line);
		~stAssignExp();

		int _offset;
		bool _local;
		unsigned char _assignType;
		stExp *_rvalue;
		std::string _objectName;
		stObject *_objectType;
};

class stLoadExp : public stExp {
	public:
		stLoadExp(stMethod *m, stObject *o, char *word, const char *file, int l);

		int offset;
		char local;
		unsigned char loadType;
};

class stArrayExp : public stExp {
	public:
		stArrayExp( std::vector< stExp* > *e, const char *file, int l ) : stExp( file, l ) {
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
		stArrayIndexAssignExp(stMethod *m, stObject *o, char *word, stExp *i, stExp *rv, const char *file, int line);
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
		stArrayIndexExp(stMethod *m, stObject *o, char *word, stExp *i, const char *file, int line);
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
		stWhileExp(stExp *cn, stExp *cd, const char *file, int lineno);
		~stWhileExp();

		stExp *cond;
		stExp *code;
};

class stForeachExp : public stExp {
	public:
		stForeachExp(stAssignExp *a, stExp *l, stExp *c, const char *file, int lineno);
		~stForeachExp();

		stAssignExp *assignment;
		stExp *list;
		stExp *code;
};

class stForExp : public stExp {
	public:
		stForExp(stExp *a, stExp *cn, stExp *i, stExp *cd, const char *file, int lineno);
		~stForExp();

		stExp *assignment;
		stExp *condition;
		stExp *iteration;
		stExp *code;
};

class stIfExp : public stExp {
	public:
		stIfExp(stExp *c, stExp *t, stExp *f, const char *file, int lineno);
		~stIfExp();

		stExp *cond;
		stExp *trueCode;
		stExp *falseCode;
};

class stAllExp : public stExp {
	public:
		stAllExp(char *objectName, const char *file, int line);
		~stAllExp();

		std::string name;
		stObject *object;
};

class stInstanceExp : public stExp {
	public:
		stInstanceExp( char *n, stExp *c, const char *file, int line ) : stExp( file, line ) {
			name = n;
			count = c;

			type = ET_INSTANCE;
		}

		~stInstanceExp() {
			delete count;
		}

		std::string name;
		stExp *count;
};

class stCCallExp : public stExp {
	public:
		stCCallExp(brEngine *e, brInternalFunction *s, std::vector< stExp* > *expressions, const char *file, int line);
		~stCCallExp();

		brInternalFunction *_function;
		std::vector< stExp* > _arguments;
};

class stKeyword {
	public:
		stKeyword( const char *w, stExp *v ) {
			word = w;
			value = v;
		}

		~stKeyword() {
			delete value;
		}

		std::string word;
		stExp *value;
		int position;
};

char *dequote(char *c);

int stSizeof(stVarType *v);
int stSizeofAtomic(int a);

int stAlign(stVarType *v);
int stAlignAtomic(int a);

#endif
