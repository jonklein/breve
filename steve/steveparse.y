%{

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
	+ steveparse.y
	= parses the steve text and builds parse trees for each object.
	= the variable parseEngine is set using the stSetParseEngine
	= function before yyparse() is called.  the parseEngine passed
	= in will then be the destination of all the objects parsed.
	=
	= this file should be commented a whole lot more than it is.
*/

#include "steve.h"

brEngine *parseEngine = NULL;

stObject *thisObject = NULL;
stMethod *thisMethod = NULL;

stSteveData *steveData;

extern int lineno;

extern double doubleValue;

extern char *yyfile;

int gReparse;

void stStartParse();
void stEndParse();
void stParseSetSteveData(stSteveData *d);
void stParseSetEngine(brEngine *e);
void stParseSetObject(stObject *o);
void stParseSetObjectAndMethod(stObject *o, stMethod *m);

void yyerror(char *c);

int yylex();

double stDoubleFromIntOrDoubleExp(stExp *e);

int stGetTypeForString(char *name);

%}

/*
	Error checking:

	We could do a YYABORT at every error, but this can leave a lot of memory 
	allocated since there will be structures that haven't been made into 
	parts of objects, etc.

	So instead, we periodically check for errors like this:

		if(parseEngine->error.type) YYABORT;

	when we think that we've got most of the memory stowed away into structures
	that can be neatly freed.
*/

%union {
    double number;
    int integer;
    char *string;

    void *data;
	struct stExp *exp;
	struct brEval *eval;
	struct stVarType *type;
	struct slList *list;
	struct stVar *variable;
	struct stVersionRequirement *requirement;
	struct stKeywordEntry *keyword;
}

%type <integer> vector_element math_assign_operator
%type <number> object_version
%type <string> object_aka method_head 
%type <exp> number vector matrix string
%type <exp> atomic_expression unary_expression mul_expression add_expression comp_expression
%type <exp> log_expression land_expression method_call expression
%type <exp> control_statement simple_statement statement compound_statement
%type <list> keyword_list exp_list code method_code keyword_and_variables keyword_and_variable_list
%type <type> type
%type <requirement> header_version
%type <variable> variable
%type <keyword> keyword_and_variable
%type <eval> matrix_value vector_value default_value
%type <data> variable_list

%type <exp> literal_value

%token <number> FLOAT_VALUE
%token <string> STRING_VALUE WORD_VALUE
%token <eval> ST_EVAL 
%token <integer> INT_VALUE TYPE PLURAL_TYPE TYPE
%token TO_PRIVATE TO_PUBLIC TO_PROTECTED
%token VARIABLES DEFINE
%token IF ELSE WHILE FOREACH ST_IN FOR
%token NEW FREE DIE PRINT PRINTF END RANDOM RETURN
%token GT GE LT LE EQ NE LAND LOR PLUSPLUS MINUSMINUS
%token PLUSEQ SUBEQ MULEQ DIVEQ MODEQ POWEQ
%token INCLUDE USE PATH X_ELEMENT Y_ELEMENT Z_ELEMENT 
%token COPYLIST PREPEND UNPREPEND
%token CONTROLLER VERSION AKA
%token INSERT REMOVE PUSH POP SORT SUPER SELF 
%token NIB_FILE PLUGIN ALL
%token EMPTY_LIST

%token DEBUG

%left GT GE LT LE EQ NE LAND LOR
%left '-' '+'
%left '^' '*' '/' '%'

/* we expect 1 error: the dangling else error */

%expect 1

%%

sucessful_parse
: file {
		if(brGetError(parseEngine)) YYABORT;
	}
| GT { 
		if(!thisMethod || !thisObject) YYERROR;
	} statement {
		if(brGetError(parseEngine)) YYABORT;
		steveData->singleStatement = $3;
	}
;

file
: steve_code
| file steve_code
;


/* 
	the headers lines are lines outside of the objects which control
	the actual parsing and interpretation in some way.  

	@include and @use (which are more or less the same, but @use is not 
	documented) include other files for parsing.  They are actually parsed
	and interpreted before we even get here, so now we just skip over them.

	@path specifies a search path to be added to the engine.  It is also
	parsed before we get here.

	@define connects a word to a string, double, int or vector like the 
	C preprocessor.  When we see a @define, we add an entry to the defines
	table.  The file stevelex.l will consult this table when it encounters
	a symbol in the parsing.
*/

header_version
: 	/* empty */ {
		$$ = NULL;
	}
| '[' VERSION EQ FLOAT_VALUE ']' {
		$$ = stMakeVersionRequirement($4, VR_EQ);
	}
| '[' VERSION GT FLOAT_VALUE ']' {
		$$ = stMakeVersionRequirement($4, VR_GT);
	}
| '[' VERSION GE FLOAT_VALUE ']' {
		$$ = stMakeVersionRequirement($4, VR_GE);
	}
| '[' VERSION LT FLOAT_VALUE ']' {
		$$ = stMakeVersionRequirement($4, VR_LT);
	}
| '[' VERSION LE FLOAT_VALUE ']' {
		$$ = stMakeVersionRequirement($4, VR_LE);
	}
| '[' VERSION NE FLOAT_VALUE ']' {
		$$ = stMakeVersionRequirement($4, VR_NE);
	}
;

header
: '@' INCLUDE STRING_VALUE { slFree($3); }
| '@' INCLUDE STRING_VALUE END { slFree($3); }
| '@' PATH STRING_VALUE { slFree($3); }
| '@' PATH STRING_VALUE END { slFree($3); }
| '@' USE WORD_VALUE header_version END { 
		if($4) {
			brObject *o;
			stObject *so;

			o = brObjectFind(parseEngine, $3);
			so = o->userData;

			if(so) {
				if(!stCheckVersionRequirement(so->version, $4)) {
					stParseError(parseEngine, PE_FILE_VERSION, "Incompatible file version for class \"%s\"", $3);
					YYABORT;
				}
			}

			slFree($4);
		}

		slFree($3); 
	}
| '@' PLUGIN STRING_VALUE '(' WORD_VALUE ')' END {
		char *file = slDequote($3);
		brEngineAddDlPlugin(file, $5, parseEngine);

		slFree($3);
		slFree($5);
		slFree(file);
	}
| '@' NIB_FILE STRING_VALUE END {
		char *unquoted = slDequote($3);
		char *fullnib, *nib = slMalloc(strlen(unquoted) + 5);
		sprintf(nib, "%s.nib", unquoted);

		fullnib = brFindFile(parseEngine, unquoted, NULL);

		// check for it with the .nib extension

		if(!fullnib) fullnib = brFindFile(parseEngine, nib, NULL);

		if(!fullnib) {
			slMessage(DEBUG_ALL, "warning: unable to locate nib file \"%s\"\n", nib);
		} else {
			brEngineSetInterface(parseEngine, fullnib);
		}
	
		slFree($3);
		slFree(unquoted);
		slFree(nib);
		if(fullnib) slFree(fullnib);
	}
| '@' DEFINE WORD_VALUE STRING_VALUE END {
		brEval *e;

		e = slMalloc(sizeof(brEval));
		e->type = AT_STRING;
		BRSTRING(e) = slDequote($4);

		brNamespaceStore(steveData->defines, $3, 0, e);

		slFree($3);
		slFree($4);
	}
| '@' DEFINE WORD_VALUE FLOAT_VALUE END {
		brEval *e;

		e = slMalloc(sizeof(brEval));
		e->type = AT_DOUBLE;
		BRDOUBLE(e) = $4;

		brNamespaceStore(steveData->defines, $3, 0, e);
		slFree($3);
	}
| '@' DEFINE WORD_VALUE INT_VALUE END {
		brEval *e;

		e = slMalloc(sizeof(brEval));
		e->type = AT_INT;
		BRINT(e) = $4;

		brNamespaceStore(steveData->defines, $3, 0, e);
		slFree($3);
	}
| '@' DEFINE WORD_VALUE vector_value END {
		brNamespaceStore(steveData->defines, $3, 0, $4);
		slFree($3);
	}
| '@' DEFINE WORD_VALUE matrix_value END {

		brNamespaceStore(steveData->defines, $3, 0, $4);
		slFree($3);
	}
| CONTROLLER WORD_VALUE END {
		if(!gReparse && stSetControllerName(steveData, parseEngine, $2))
			stParseError(parseEngine, PE_INTERNAL, "Error defining \"Controller\" object");

		slFree($2);
	}
;

vector_value
: '(' number ',' number ',' number ')'  {
		brEval *e;

		e = slMalloc(sizeof(brEval));
		e->type = AT_VECTOR;

		BRVECTOR(e).x = stDoubleFromIntOrDoubleExp($2); stExpFree($2);
		BRVECTOR(e).y = stDoubleFromIntOrDoubleExp($4); stExpFree($4);
		BRVECTOR(e).z = stDoubleFromIntOrDoubleExp($6); stExpFree($6);

		$$ = e;
	}
;

matrix_value
: '[' '(' number ',' number ',' number ')' ',' '(' number ',' number ',' number ')' ',' '(' number ',' number ',' number ')' ']' {
		brEval *e;

		e = slMalloc(sizeof(brEval));
		e->type = AT_MATRIX;

		BRMATRIX(e)[0][0] = stDoubleFromIntOrDoubleExp($3); stExpFree($3);
		BRMATRIX(e)[0][1] = stDoubleFromIntOrDoubleExp($5); stExpFree($5);
		BRMATRIX(e)[0][2] = stDoubleFromIntOrDoubleExp($7); stExpFree($7);
		
		BRMATRIX(e)[1][0] = stDoubleFromIntOrDoubleExp($11); stExpFree($11);
		BRMATRIX(e)[1][1] = stDoubleFromIntOrDoubleExp($13); stExpFree($13);
		BRMATRIX(e)[1][2] = stDoubleFromIntOrDoubleExp($15); stExpFree($15);

		BRMATRIX(e)[2][0] = stDoubleFromIntOrDoubleExp($19); stExpFree($19);
		BRMATRIX(e)[2][1] = stDoubleFromIntOrDoubleExp($21); stExpFree($21);
		BRMATRIX(e)[2][2] = stDoubleFromIntOrDoubleExp($23); stExpFree($23);
	}
;

steve_code
: objectdef
| header
;

objectdef
: objecttype '{' definitionlist '}' {
	}
| objecttype '{' variabledef '}' {
		stUnusedInstanceVarWarning(thisObject);
	}
| objecttype '{' variabledef definitionlist '}' {
		stUnusedInstanceVarWarning(thisObject);
	}
| objecttype EMPTY_LIST	{
	}
| objecttype '{' '}'
;

definitionlist
: methoddef
| definitionlist methoddef
;

object_version
: '[' VERSION FLOAT_VALUE ']'	{ $$ = $3; }
| /* empty */		{ $$ = 0.0; }
;

object_aka
: '(' AKA WORD_VALUE ')'	{ $$ = $3; }
| /* empty */	{ $$ = NULL; }
;

objecttype
: WORD_VALUE ':' WORD_VALUE	object_aka object_version { 
		stObject *parentObject, *akaObject;
		brObject *o;
		float version;

		if($5) version = $5;
		else version = 1.0;

		o = brObjectFind(parseEngine, $3);

		if(o) thisObject = o->userData;
		else thisObject = NULL;

		if($4) {
			o = brObjectFind(parseEngine, $4);

			if(o) akaObject = o->userData;
			else akaObject = NULL;
		}
		else akaObject = NULL;

		if(thisObject && gReparse) {
			stObjectFree(thisObject);
			thisObject = NULL;
			akaObject = NULL;
		}

		if(thisObject) {
			stParseError(parseEngine, PE_REDEFINITION, "Class \"%s\" already defined", $3);
			slFree($1);
			slFree($3);
			if($4) slFree($4);
		} else if(akaObject) {
			stParseError(parseEngine, PE_REDEFINITION, "Class \"%s\" (defined here as alias for \"%s\") already defined", $4, $3);
			slFree($1);
			slFree($3);
			slFree($4);
		} else {
			brObject *o = brObjectFind(parseEngine, $1);

			if(o) parentObject = o->userData;
			else parentObject = NULL;

			if(!parentObject && strcmp($1, "NULL")) {
				stParseError(parseEngine, PE_UNKNOWN_OBJECT, "Cannot locate parent class \"%s\" for definition of class \"%s\"", $1, $3);
				slFree($1);
				slFree($3);
				if($4) slFree($4);
			} else {
				thisObject = stObjectNew(parseEngine, steveData, $3, $4, parentObject, version);
				thisMethod = NULL;

				slFree($1);
				slFree($3);
				if($4) slFree($4);
			}
		}

		/* an error check */

		if(brGetError(parseEngine)) YYABORT;
	}
;

/* keep in mind that method definitions can be empty */

methoddef
: method_head method_code {
		slList *code;

		code = slListReverse($2);
		thisMethod->code = slListToArray(code);
		slListFree(code);

		stMethodAlignStack(thisMethod);
		
		if(stStoreInstanceMethod(thisObject, $1, thisMethod)) {
			stParseError(parseEngine, PE_REDEFINITION, "Symbol \"%s\" already defined for class \"%s\"", $1, thisObject->name);
			stFreeMethod(thisMethod);
		}

		slFree($1);

		/* if we found an error somewhere and caught it, we continued so */
		/* that the method would be stored before we stop the parsing... */
		/* this way we're sure to free the allocated memory.			 */

		if(brGetError(parseEngine)) YYABORT;
	}
| method_head variable_list method_code {
		slList *code;

		code = slListReverse($3);
		thisMethod->code = slListToArray(code);
		slListFree(code);

		stMethodAlignStack(thisMethod);

		if(stStoreInstanceMethod(thisObject, $1, thisMethod)) {
			stParseError(parseEngine, PE_REDEFINITION, "Symbol \"%s\" already defined for class \"%s\"", $1, thisObject->name);
			stFreeMethod(thisMethod);
		}

		slFree($1);

		if(brGetError(parseEngine)) YYABORT;
	}
;

to
: TO_PRIVATE
| TO_PUBLIC
| TO_PROTECTED
;

method_head
: to WORD_VALUE keyword_and_variables ':' { 
		slArray *keywordArray;
		slList *rev = NULL;

		rev = slListReverse($3);

		keywordArray = slListToArray(rev);

		if($3) slListFree(rev);

		thisMethod = stNewMethod($2, keywordArray, yyfile, lineno);

		$$ = $2; 
	}
;

default_value
: '=' vector_value { $$ = $2; }
| '=' matrix_value { $$ = $2; }
| '=' number { 
		brEval *e = slMalloc(sizeof(brEval));
		e->type = AT_DOUBLE;

		BRDOUBLE(e) = stDoubleFromIntOrDoubleExp($2); stExpFree($2);

		$$ = e;
	}
| '=' STRING_VALUE {
		brEval *e = slMalloc(sizeof(brEval));
		e->type = AT_STRING;

		BRSTRING(e) = slDequote($2);

		slFree($2);

		$$ = e;
	}
;

variabledef
: VARIABLES ':' variable_list 
| VARIABLES ':' 
;

variable_list
: variable {
		$$ = NULL;
	}
| variable_list variable {
		$$ = NULL;
	}
;

variable
: WORD_VALUE type END {
		stVar *var = stVarNew($1, $2);

		slFree($1);

		if(!thisMethod) {
			if(!stInstanceNewVar(var, thisObject)) {
				stParseError(parseEngine, PE_REDEFINITION, "Redefinition of symbol \"%s\" in class \"%s\"", var->name, thisObject->name);
				stFreeStVar(var);
			}
		} else {
			if(!stMethodAddVar(var, thisMethod)) {
				stParseError(parseEngine, PE_REDEFINITION, "Redefinition of symbol \"%s\" in method \"%s\"", var->name, thisMethod->name);
				stFreeStVar(var);
			}
		}

		$$ = var;
	}
| WORD_VALUE ',' variable {
		// we have to get the variable type from the existing variable

		stVar *otherVar, *thisVar;
		stVarType *newType;

		otherVar = $3;

		newType = stVarTypeCopy(otherVar->type);
		
		thisVar = stVarNew($1, newType);

		slFree($1);

		if(!thisMethod) {
			if(!stInstanceNewVar(thisVar, thisObject)) {
				stParseError(parseEngine, PE_REDEFINITION, "Redefinition of symbol \"%s\" in class \"%s\"", thisVar->name, thisObject->name);
				stFreeStVar(thisVar);
			}
		} else {
			if(!stMethodAddVar(thisVar, thisMethod)) {
				stParseError(parseEngine, PE_REDEFINITION, "Redefinition of symbol \"%s\" in method \"%s\"", thisVar->name, thisMethod->name);
				stFreeStVar(thisVar);
			}
		}

		$$ = thisVar;
	}
;

keyword_and_variables
: keyword_and_variable_list { $$ = $1; }
| /* empty */ { $$ = NULL; }
;

keyword_and_variable_list 
: keyword_and_variable { 
		$$ = slListPrepend(NULL, $1); 
	}
| '[' keyword_and_variable ']' { 
		$$ = slListPrepend(NULL, $2); 
	}
| keyword_and_variable_list keyword_and_variable { 
		$$ = slListPrepend($1, $2); 
	}
;

keyword_and_variable
: WORD_VALUE WORD_VALUE default_value type {
		stVar *v = stVarNew($2, $4);

		$$ = stNewKeywordEntry($1, v, $3);

		slFree($1);
		slFree($2);
		slFree($3);
	}
| WORD_VALUE WORD_VALUE type {
		stVar *v = stVarNew($2, $3);

		$$ = stNewKeywordEntry($1, v, NULL);

		slFree($1);
		slFree($2);
	}
;

method_code 
: code { $$ = $1; }
| /* empty */ { $$ = NULL; }
;

compound_statement
: '{' code '}' { 
		slArray *a;
		slList *rev;

		rev = slListReverse($2);
		a = slListToArray(rev);
		slListFree(rev);

		$$ = stExpNew(a, ET_CODE_ARRAY, yyfile, lineno);
	}
;

code
: statement { 
		$$ = slListPrepend(NULL, $1); 
	}
| code statement { 
		$$ = slListPrepend($1, $2); 
	}
;

statement
: compound_statement { $$ = $1; }
| control_statement { $$ = $1; }
| simple_statement END { $$ = $1; }
| simple_statement error { 
		if($1) stExpFree($1);

		if(brGetError(parseEngine)) YYABORT;

		stParseError(parseEngine, PE_PARSE, "Possible missing period on previous line");
		YYABORT;
		$$ = NULL;
	}
| error {
		if(brGetError(parseEngine)) YYABORT;

		stParseError(parseEngine, PE_PARSE, "Parse error");
		$$ = NULL;
	}
| '{' '}' {
		$$ = NULL;
	}
;

control_statement
: FOREACH WORD_VALUE WORD_VALUE expression ':' statement {
		stExp *ae = stNewAssignExp(thisMethod, thisObject, $2, NULL, yyfile, lineno);
		slFree($2);

		if(strcmp($3, "in")) {
			stParseError(parseEngine, PE_PARSE, "Expected \"in\" after list expression of \"foreach\" statement");
		}

		if(ae) {
			$$ = stNewForeachExp(ae->values.pValue, $4, $6, yyfile, lineno);
			slFree(ae);
		} else {
			stExpFree($4);
			stExpFree($6);
			$$ = NULL;
		}
	}
| WHILE expression ':' statement {
		$$ = stNewWhileExp($2, $4, yyfile, lineno);
	}
| IF expression ':' statement {
		$$ = stNewIfExp($2, $4, NULL, yyfile, lineno);
	}
| IF expression ':' statement else statement {
		$$ = stNewIfExp($2, $4, $6, yyfile, lineno);
	}
| IF expression ':' EMPTY_LIST else statement {
		$$ = stNewIfExp($2, NULL, $6, yyfile, lineno);
	}
| FOR expression ',' expression ',' expression ':' statement {
		$$ = stNewForExp($2, $4, $6, $8, yyfile, lineno);
	}
;

else
: ELSE
| ELSE ':'
;

simple_statement
: method_call 		{ $$ = $1; }
| PRINT exp_list { 
		slArray *a;
		slList *rev;

		rev = slListReverse($2);

		a = slListToArray(rev);

		slListFree(rev);

		$$ = stNewPrintExp(a, 1, yyfile, lineno); 
	}
| PRINTF exp_list { 
		slArray *a;
		slList *rev;

		rev = slListReverse($2);

		a = slListToArray(rev);

		slListFree(rev);

		$$ = stNewPrintExp(a, 0, yyfile, lineno); 
	}
| DIE { 
		stParseError(parseEngine, PE_SYNTAX, "'die' requires an error message string");
	}
| DIE expression	{ $$ = stExpNew($2, ET_DIE, yyfile, lineno); }
| expression		{ $$ = $1; }
| RETURN expression { 
		$$ = stExpNew($2, ET_RETURN, yyfile, lineno); 
	}
| RETURN {
		$$ = stExpNew(NULL, ET_RETURN, yyfile, lineno); 
	}
| FREE expression { 
		$$ = stExpNew($2, ET_FREE, yyfile, lineno); 
	}
| WORD_VALUE '=' method_call {
		$$ = stNewAssignExp(thisMethod, thisObject, $1, $3, yyfile, lineno);

		if(!$$) stExpFree($3);
		
		slFree($1);
	}
; 

exp_list
: expression			{ $$ = slListPrepend(NULL, $1); }
| exp_list ',' expression	{ $$ = slListPrepend($1, $3); }
;

method_call
: atomic_expression WORD_VALUE keyword_list { 
		slList *l = $3;
		slArray *a;
		stMethodExp *m;

		a = slListToArray(l);
		slListFree(l);

		m = stNewMethodCall(thisObject, $1, $2, a);
		if(m) $$ = stExpNew(m, ET_METHOD, yyfile, lineno);
	else $$ = NULL;

		slFree($2);
	}
| atomic_expression WORD_VALUE {
		stMethodExp *m;
		m = stNewMethodCall(thisObject, $1, $2, slListToArray(NULL));
		if(m) $$ = stExpNew(m, ET_METHOD, yyfile, lineno);
	else $$ = NULL;

		slFree($2);
	}
;

keyword_list 
: WORD_VALUE expression {
		stKeyword *k = stNewKeyword((char *)$1, (stExp*)$2);

		$$ = slListPrepend(NULL, k);

		slFree($1);
	}
| keyword_list WORD_VALUE expression {
		stKeyword *k = stNewKeyword($2, (stExp*)$3);

		$$ = slListPrepend($1, k); 

		slFree($2);
	}
;

expression
: log_expression
| WORD_VALUE math_assign_operator expression {
		stExp *loadExp, *binExp;

		loadExp = stNewLoadExp(thisMethod, thisObject, $1, yyfile, lineno);

		binExp = stNewBinaryExp($2, loadExp, $3, yyfile, lineno);

		$$ = stNewAssignExp(thisMethod, thisObject, $1, binExp, yyfile, lineno);

		slFree($1);
	}
| WORD_VALUE '=' expression {
		$$ = stNewAssignExp(thisMethod, thisObject, $1, $3, yyfile, lineno);

		if(!$$) {
			stExpFree($3);
		}

		slFree($1);
	}
| WORD_VALUE PLUSPLUS {
		stExp *var = stNewLoadExp(thisMethod, thisObject, $1, yyfile, lineno);
		stExp *one = stNewIntExp(1, yyfile, lineno);
		stExp *newExp = stNewBinaryExp(BT_ADD, var, one, yyfile, lineno); 

		$$ = stNewAssignExp(thisMethod, thisObject, $1, newExp, yyfile, lineno);

		slFree($1);
	}
| WORD_VALUE MINUSMINUS {
		stExp *var = stNewLoadExp(thisMethod, thisObject, $1, yyfile, lineno);
		stExp *one = stNewIntExp(1, yyfile, lineno);
		stExp *newExp = stNewBinaryExp(BT_SUB, var, one, yyfile, lineno); 

		$$ = stNewAssignExp(thisMethod, thisObject, $1, newExp, yyfile, lineno);

		slFree($1);
	}
| atomic_expression vector_element '=' expression {
		$$ = stNewVectorElementAssignExp($1, $4, $2, yyfile, lineno);
	}
| atomic_expression vector_element math_assign_operator expression {
		stExp *loadExp = $1;

		if(loadExp->type != ET_LOAD) {
			stParseError(parseEngine, PE_SYNTAX, "Invalid vector element assignment");
			YYERROR;
		}

		// $$ = stNewVectorElementAssign($1, $2, $4, yyfile, lineno);
	}
| atomic_expression '{' expression'}' '=' expression {
		$$ = stNewListIndexAssignExp($1, $3, $6, yyfile, lineno);
	}
| atomic_expression '{' expression'}' math_assign_operator expression {
		stExp *loadExp, *binExp;

		loadExp = stNewListIndexExp($1, $3, yyfile, lineno);
		binExp = stNewBinaryExp($5, loadExp, $6, yyfile, lineno);

		// we're reusing $1 and $3 -- so we have to "duplicate" them

		$$ = stNewListIndexAssignExp(stNewDuplicateExp($1, yyfile, lineno), stNewDuplicateExp($3, yyfile, lineno), binExp, yyfile, lineno);
	}
| WORD_VALUE '[' expression']' '=' expression {
		$$ = stNewArrayIndexAssignExp(thisMethod, thisObject, $1, $3, $6, yyfile, lineno);
		slFree($1);
	}
| WORD_VALUE '[' expression']' math_assign_operator expression {
		stExp *loadExp, *binExp;

		loadExp = stNewArrayIndexExp(thisMethod, thisObject, $1, $3, yyfile, lineno);
		binExp = stNewBinaryExp($5, loadExp, $6, yyfile, lineno);

		$$ = stNewArrayIndexAssignExp(thisMethod, thisObject, $1, stNewDuplicateExp($3, yyfile, lineno), binExp, yyfile, lineno);

		slFree($1);
	}
;

math_assign_operator
: PLUSEQ { $$ = BT_ADD; }
| SUBEQ { $$ = BT_SUB; }
| DIVEQ { $$ = BT_DIV; }
| MULEQ { $$ = BT_MUL; }
| MODEQ { $$ = BT_MOD; }
| POWEQ { $$ = BT_POW; }
;

log_expression
: log_expression LOR log_expression { 
		$$ = stNewBinaryExp(BT_LOR, $1, $3, yyfile, lineno); 
	}
| land_expression
;

land_expression
: land_expression LAND land_expression { 
		$$ = stNewBinaryExp(BT_LAND, $1, $3, yyfile, lineno);
	}
| comp_expression
; 

comp_expression
: comp_expression EQ comp_expression { 
		$$ = stNewBinaryExp(BT_EQ, $1, $3, yyfile, lineno); 
	}
| comp_expression NE comp_expression { 
		$$ = stNewBinaryExp(BT_NE, $1, $3, yyfile, lineno); 
	}
| comp_expression GE comp_expression { 
		$$ = stNewBinaryExp(BT_GE, $1, $3, yyfile, lineno); 
	}
| comp_expression GT comp_expression { 
		$$ = stNewBinaryExp(BT_GT, $1, $3, yyfile, lineno); 
	}
| comp_expression LE comp_expression { 
		$$ = stNewBinaryExp(BT_LE, $1, $3, yyfile, lineno); 
	}
| comp_expression LT comp_expression { 
		$$ = stNewBinaryExp(BT_LT, $1, $3, yyfile, lineno); 
	}
| add_expression
;

add_expression 
: add_expression '+' add_expression { 
		$$ = stNewBinaryExp(BT_ADD, $1, $3, yyfile, lineno); 
	}
| add_expression '-' add_expression { 
		$$ = stNewBinaryExp(BT_SUB, $1, $3, yyfile, lineno); 
	}
| mul_expression			{ $$ = $1; }
;

mul_expression
: mul_expression '*' mul_expression { 
		$$ = stNewBinaryExp(BT_MUL, $1, $3, yyfile, lineno); 
	}
| mul_expression '/' mul_expression { 
		$$ = stNewBinaryExp(BT_DIV, $1, $3, yyfile, lineno); 
	}
| mul_expression '%' mul_expression { 
		$$ = stNewBinaryExp(BT_MOD, $1, $3, yyfile, lineno); 
	}
| mul_expression '^' mul_expression { 
		$$ = stNewBinaryExp(BT_POW, $1, $3, yyfile, lineno); 
	}
| unary_expression			{ $$ = $1; }
;

unary_expression 
: '!' unary_expression	{ $$ = stNewUnaryExp(UT_NOT, $2, yyfile, lineno); }
| '-' unary_expression	{ $$ = stNewUnaryExp(UT_MINUS, $2, yyfile, lineno); }
| atomic_expression 	{ $$ = $1; }
| POP atomic_expression {
		$$ = stNewListRemoveExp($2, NULL, yyfile, lineno);
	}
| UNPREPEND atomic_expression {
		$$ = stNewListRemoveExp($2, stNewIntExp(1, yyfile, lineno), yyfile, lineno);
	}
| PUSH expression WORD_VALUE atomic_expression {
		if(strcmp($3, "onto")) {
			stParseError(parseEngine, PE_SYNTAX, "Expecting \"onto <expression>\" after \"push\" operator");
			slFree($3);
			stExpFree($2);
			stExpFree($4);
			$$ = NULL;
		} else {
			$$ = stNewListInsertExp($4, $2, NULL, yyfile, lineno);
			slFree($3);
		}
	}
| PREPEND expression WORD_VALUE atomic_expression {
		if(strcmp($3, "onto")) {
			stParseError(parseEngine, PE_SYNTAX, "Expecting \"onto <expression>\" after \"append\" operator");
			slFree($3);
			stExpFree($2);
			stExpFree($4);
			$$ = NULL;
		} else {
			$$ = stNewListInsertExp($4, $2, stNewIntExp(1, yyfile, lineno), yyfile, lineno);
			slFree($3);
		}
	}
| INSERT expression WORD_VALUE atomic_expression '{' expression '}' {
		if(strcmp($3, "at")) {
			stParseError(parseEngine, PE_SYNTAX, "Expecting \"at <expression>\" after \"insert\" operator");
			slFree($3);
			stExpFree($2);
			stExpFree($4);
			stExpFree($6);
		} else {
			$$ = stNewListInsertExp($4, $2, $6, yyfile, lineno);
			slFree($3);
		}
	}
| REMOVE atomic_expression '{' expression '}' {
		$$ = stNewListRemoveExp($2, $4, yyfile, lineno);
	}
| SORT atomic_expression WORD_VALUE WORD_VALUE {
		if(strcmp($3, "with")) {
			stParseError(parseEngine, DEBUG_ALL, "Expecting \"with <method_name>\" after \"sort\" operator");
			slFree($3);
			slFree($4);
			stExpFree($2);
			$$ = NULL;
		} else {
			$$ = stNewSortExp($2, $4, yyfile, lineno);
			slFree($3);
			slFree($4);
		}
	}
| COPYLIST atomic_expression {
	$$ = stExpNew($2, ET_COPYLIST, yyfile, lineno);
}
;

atomic_expression
: literal_value
| atomic_expression vector_element { $$ = stNewVectorElementExp($1, $2, yyfile, lineno); }
| '|' expression '|' { 
		$$ = stExpNew($2, ET_VLENGTH, yyfile, lineno); 
	}
| ST_EVAL		{ $$ = stNewStEvalExp($1, yyfile, lineno); }
| atomic_expression '{' expression '}' { 
		$$ = stNewListIndexExp($1, $3, yyfile, lineno); 
	}
| WORD_VALUE '[' expression ']' {
		$$ = stNewArrayIndexExp(thisMethod, thisObject, $1, $3, yyfile, lineno);

		slFree($1);

		if(!$$) {
			/* this parse error is handled by stNewLoadExp */
			stExpFree($3);
		}
	}
| WORD_VALUE { 
		$$ = stNewLoadExp(thisMethod, thisObject, $1, yyfile, lineno);

		slFree($1);

		if(!$$) {
			/* this parse error is handled by stNewLoadExp */
		}
	}
| SUPER {
		$$ = stExpNew(NULL, ET_SUPER, yyfile, lineno);
	}
| SELF {
		$$ = stExpNew(NULL, ET_SELF, yyfile, lineno);
	}
| NEW WORD_VALUE { 
		$$ = stInstanceNewExp($2, stNewIntExp(1, yyfile, lineno), yyfile, lineno); 
		slFree($2);
	}
| NEW STRING_VALUE { 
		char *unquoted = slDequote($2);;
		$$ = stInstanceNewExp(unquoted, stNewIntExp(1, yyfile, lineno), yyfile, lineno); 
		slFree($2);
		slFree(unquoted);
	}
| atomic_expression NEW WORD_VALUE { 
		$$ = stInstanceNewExp($3, $1, yyfile, lineno); 
		slFree($3);
	}
| '(' expression ')'		{ $$ = $2; }
| DEBUG '(' expression ')'	{ 
		((stExp*)$3)->debug = 1;
		$$ = $3; 
	}
| '(' method_call ')'		{ $$ = $2; }
| RANDOM '[' expression	']' { 
		$$ = stExpNew($3, ET_RANDOM, yyfile, lineno); 
	}
| '{' exp_list '}' {
		slList *rev = slListReverse($2);
		$$ = stExpNew(rev, ET_LIST, yyfile, lineno);
	}
| EMPTY_LIST {
		$$ = stExpNew(NULL, ET_LIST, yyfile, lineno);
	}
| WORD_VALUE '(' exp_list ')' { 
		brInternalFunction *func = brEngineInternalFunctionLookup(parseEngine, $1);

		if(!func) {	  
			slList *l = $3;

			stParseError(parseEngine, PE_UNKNOWN_FUNCTION, "Internal function \"%s\" not found", $1);

			while(l) {
				stExpFree(l->data);
				l = l->next;
			}

			slListFree($3);
			slFree($1);
			$$ = NULL;
		} else {
			slList *l = slListReverse($3);

			$$ = stNewCCallExp(parseEngine, func, l, yyfile, lineno); 
			slListFree(l);

			slFree($1);
		}
	}
| WORD_VALUE '(' ')' {
		brInternalFunction *func = brEngineInternalFunctionLookup(parseEngine, $1);

		if(!func) {
			stParseError(parseEngine, PE_UNKNOWN_FUNCTION, "Internal function \"%s\" not found", $1);
			slFree($1);
			$$ = NULL;
		} else {
			$$ = stNewCCallExp(parseEngine, func, NULL, yyfile, lineno); 

			slFree($1);

			if(!$$) {
				/* this error is handled by stNewCCallExp */
			}
		}
	}
| ALL WORD_VALUE {
		$$ = stNewAllExp($2, yyfile, lineno);
		slFree($2);
	}
| WORD_VALUE '.' WORD_VALUE '(' exp_list ')' {
	/* hey, this could be a java call! */
	$$ = NULL;
	}
;

literal_value
: matrix
| vector
| string
| number
;

matrix
: '[' '(' expression ',' expression ',' expression ')' ',' '(' expression ',' expression ',' expression ')' ',' '(' expression ',' expression ',' expression ')' ']' {
		$$ = stNewMatrixExp($3, $5, $7, $11, $13, $15, $19, $21, $23, yyfile, lineno);
	}
;

vector
: '(' expression ',' expression ',' expression ')' { 
		$$ = stNewVectorExp($2, $4, $6, yyfile, lineno); 
	}
;

vector_element
: X_ELEMENT { $$ = VE_X; }
| Y_ELEMENT { $$ = VE_Y; }
| Z_ELEMENT { $$ = VE_Z; }
; 

string
: STRING_VALUE { 
		char *unquoted = slDequote($1);
		$$ = stNewStringExp(unquoted, thisMethod, thisObject, yyfile, lineno);

		slFree($1);
		slFree(unquoted);

		if(!$$) {
			// this error is handled by stNewStringExp 
		}
	}
;

type
: '(' TYPE ')' {
		// int type = stGetTypeForString($2);
		// slFree($2);
		// $$ = stVarTypeNew(type, AT_NULL, 0, NULL);

		$$ = stVarTypeNew($2, AT_NULL, 0, NULL);
	}
| '(' WORD_VALUE TYPE ')' {
		if($3 != AT_INSTANCE) {
			stParseError(parseEngine, PE_SYNTAX, "Expected \"object\" after class name");
			slFree($2);
		} else {
			$$ = stVarTypeNew($3, AT_NULL, 0, $2);
			slFree($2);
		}
	}
| '(' INT_VALUE PLURAL_TYPE ')' {
		$$ = stVarTypeNew(AT_ARRAY, $3, $2, NULL);
	}
| '(' INT_VALUE WORD_VALUE PLURAL_TYPE ')' {
		if($4 != AT_INSTANCE) {
			stParseError(parseEngine, PE_SYNTAX, "Expected \"objects\" after class name");
			slFree($3);
		} else {
			$$ = stVarTypeNew(AT_ARRAY, $4, $2, $3);
			slFree($3);
		}
	}
;

number
: INT_VALUE	 		{ $$ = stNewIntExp($1, yyfile, lineno); }
| FLOAT_VALUE	   		{ $$ = stNewDoubleExp($1, yyfile, lineno); }
;

%%

void stStartParse() {

}

void stParseSetEngine(brEngine *e) {
	parseEngine = e;
	brClearError(e);
}

void stParseSetObjectAndMethod(stObject *o, stMethod *m) {
	thisMethod = m;
	thisObject = o;
}

void stParseSetSteveData(stSteveData *data) {
	steveData = data;
}

double stDoubleFromIntOrDoubleExp(stExp *e) {
	if(e->type == ET_DOUBLE) return e->values.dValue;
	if(e->type == ET_INT) return (double)e->values.iValue;

	return 0.0;
}

int stGetTypeForString(char *type) {
	if(!strcmp(type, "int")) return AT_INT;
	if(!strcmp(type, "double")) return AT_DOUBLE;
	if(!strcmp(type, "float")) return AT_DOUBLE;
	if(!strcmp(type, "vector")) return AT_VECTOR;
	if(!strcmp(type, "matrix")) return AT_MATRIX;
	if(!strcmp(type, "object")) return AT_INSTANCE;
	if(!strcmp(type, "pointer")) return AT_POINTER;
	if(!strcmp(type, "list")) return AT_LIST;
	if(!strcmp(type, "hash")) return AT_HASH;
	if(!strcmp(type, "string")) return AT_STRING;
	if(!strcmp(type, "data")) return AT_STRING;

	return AT_UNDEFINED;
}
