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

enum stExpTypes {
	/* basic expression types */

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
	ET_VLENGTH,
	ET_VNORM,
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

struct stExp {
	union {
		int iValue;
		double dValue;
		void *pValue;
	} values;

	unsigned char type;
	unsigned char debug;

	int line;
	char *file;
};

struct stIsaExp {
	stExp *expression;
	stVarType *type;
};

struct stStringExp {
	int baseSize;
	char *string;

	slArray *substrings;
};

/* an stSubstringExp is a variable embedded in an stStringExp */

struct stSubstringExp {
	stExp *loadExp;
	char *string;
	int offset;
	unsigned char retain;
};

struct stBinaryExp {
	unsigned char type;
	stExp *left;
	stExp *right;
};

struct stUnaryExp {
	unsigned char type;
	stExp *exp;
};

struct stVectorExp {
	stExp *x;
	stExp *y;
	stExp *z;
};

struct stMatrixExp {
	stExp *expressions[9];
};

struct stPrintExp {
	slArray *expressions;
	unsigned char newline;
};

struct stVectorElementExp {
	stExp *exp;
	char element;
};

struct stVectorElementAssignExp {
	stExp *exp;
	stExp *assignExp;
	char element;
};

struct stListInsertExp {
	stExp *exp;
	stExp *listExp;
	stExp *index;
};

struct stListRemoveExp {
	stExp *listExp;
	stExp *index;
};

struct stSortExp {
	stExp *listExp;
	char *methodName;
};

struct stListIndexExp {
	stExp *listExp;
	stExp *indexExp;
};

struct stListIndexAssignExp {
	stExp *listExp;
	stExp *indexExp;
	stExp *assignment;
};

struct stMethodExp {
	stExp *objectExp;

	char *methodName;
	stMethod *method;

	slArray *args;
	slStack *arguments;

	stObject *objectCache;
	stObject *objectTypeCache;
};

struct stAssignExp {
	int offset;
	char local;
	unsigned char type;
	stExp *rvalue;
	char *objectName;
	stObject *objectType;
};

struct stLoadExp {
	int offset;
	char local;
	unsigned char type;
};

struct stArrayIndexAssignExp {
	int offset;
	int maxIndex;
	unsigned char local;
	unsigned char type;
	int typeSize;
	stExp *index;

	stExp *rvalue;
};

struct stArrayIndexExp {
	int offset;
	int maxIndex;
	unsigned char local;
	unsigned char type;
	int typeSize;
	stExp *index;
};

struct stWhileExp {
	stExp *cond;
	stExp *code;
};

struct stForeachExp {
	stAssignExp *assignment;
	stExp *list;
	stExp *code;
};

struct stForExp {
	stExp *assignment;
	stExp *condition;
	stExp *iteration;
	stExp *code;
};

struct stIfExp {
	stExp *cond;
	stExp *trueCode;
	stExp *falseCode;
};

struct stAllExp {
	char *name;
	stObject *object;
};

struct stInstanceExp {
	char *name;
	stExp *count;
};

struct stCCallExp {
	brInternalFunction *function;
	slArray *args;
	slArray *evals;
};

struct stKeyword {
	char *word;
	stExp *value;
	int position;
};

int stExpFree(stExp *e);

void stExpFreeList(slList *n);

void stExpFreeArray(slArray *a);

stKeyword *stNewKeyword(char *word, stExp *data);
void stFreeKeywordArray(slArray *a);
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

stExp *stNewCCallExp(brEngine *e, brNamespaceSymbol *s, slList *exps, char *file, int line);

stExp *stNewWhileExp(stExp *cond, stExp *code, char *file, int line);
void stFreeWhileExp(stWhileExp *e);

stExp *stNewForExp(stExp *assignment, stExp *condition, stExp *iteration, stExp *code, char *file, int line);
void stFreeForExp(stForExp *e);

stExp *stInstanceNewExp(char *name, stExp *count, char *file, int line);
void stInstanceFreeExp();

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

stMethodExp *stNewMethodCall(stObject *o, stExp *expression, char *method, slArray *args);
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

stExp *stNewPrintExp(slArray *expressions, int newline, char *file, int line);
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
