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
#include "expression.h"

brEngine *parseEngine = NULL;

stObject *currentObject = NULL;
stMethod *currentMethod = NULL;

stSteveData *steveData;

extern int lineno;
extern const char *yyfile;

int gReparse;

void stParseSetSteveData(stSteveData *);
void stParseSetEngine(brEngine *);
void stParseSetObject(stObject *);
void stParseSetObjectAndMethod(stObject *, stMethod *);

void yyerror( const char * );

int yylex(void);

double stDoubleFromIntOrDoubleExp(stExp *);

int stGetTypeForString( char* );

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
	struct stVar *variable;
	struct stVersionRequirement *requirement;
	struct stKeywordEntry *keyword;

	std::vector< stExp* > *exp_vector;
	std::vector< stKeywordEntry* > *keyword_entry_vector;
	std::vector< stKeyword* > *keyword_vector;
}

%type <integer> vector_element math_assign_operator
%type <number> object_version
%type <string> object_aka method_head 
%type <exp> number vector matrix string 
%type <exp> atomic_expression unary_expression mul_expression add_expression comp_expression
%type <exp> log_expression land_expression method_call expression
%type <exp> control_statement simple_statement statement compound_statement
%type <exp_vector> exp_list code method_code 
%type <keyword_vector> keyword_list
%type <keyword_entry_vector> keyword_and_variable_list keyword_and_variables 
%type <type> type
%type <requirement> header_version
%type <variable> variable
%type <keyword> keyword_and_variable
%type <eval> matrix_value vector_value default_value
%type <data> variable_list

%type <exp> literal_value

%token <number> FLOAT_VALUE
%token <string> STRING_VALUE WORD_VALUE COMMENT_STRING DOCUMENTATION_COMMENT_STRING
%token <eval> ST_EVAL 
%token <integer> INT_VALUE TYPE PLURAL_TYPE
%token TO_PRIVATE TO_PUBLIC TO_PROTECTED WITH_INTERFACE
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
		if( !currentMethod || !currentObject ) {
			stParseError( parseEngine, PE_INTERNAL, "Could not locate current object or method for statement" );
			YYERROR;
		}

	} statement {
		if( brGetError( parseEngine ) ) 
			YYABORT;

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

			o = brObjectFindWithPreferredType(parseEngine, $3, STEVE_TYPE_SIGNATURE );
			so = (stObject*)o->userData;

			if(so) {
				if(!stCheckVersionRequirement(so->version, $4)) {
					stParseError(parseEngine, PE_FILE_VERSION, "Incompatible file version for class \"%s\"", $3);
					YYABORT;
				}
			}

			delete $4;
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
		char *fullnib, *nib = new char[strlen(unquoted) + 5];
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
		delete[] nib;
		if(fullnib) slFree(fullnib);
	}
| '@' DEFINE WORD_VALUE STRING_VALUE END {
		brEval *e;

		e = new brEval;
	
		char *str = slDequote($4);
		e->set( str );

		slFree(str);

		steveData->defines[ $3] = e;

		slFree($3);
		slFree($4);
	}
| '@' DEFINE WORD_VALUE FLOAT_VALUE END {
		brEval *e;

		e = new brEval;
		e->set( $4 );

		steveData->defines[ $3] = e;

		slFree($3);
	}
| '@' DEFINE WORD_VALUE INT_VALUE END {
		brEval *e;

		e = new brEval;
		e->set( $4 );

		steveData->defines[ $3] = e;

		slFree($3);
	}
| '@' DEFINE WORD_VALUE vector_value END {
		steveData->defines[ $3 ] = $4;
		slFree( $3 );
	}
| '@' DEFINE WORD_VALUE matrix_value END {
		steveData->defines[ $3 ] = $4;
		slFree( $3 );
	}
| CONTROLLER WORD_VALUE END {
		if( !gReparse && stSetControllerName( steveData, parseEngine, $2 ) )
			stParseError(parseEngine, PE_INTERNAL, "Error defining \"Controller\" object");

		slFree($2);
	}
;

vector_value
: '(' number ',' number ',' number ')'  {
		brEval *e;
		slVector v;

		e = new brEval;

		v.x = stDoubleFromIntOrDoubleExp($2); delete $2;
		v.y = stDoubleFromIntOrDoubleExp($4); delete $4;
		v.z = stDoubleFromIntOrDoubleExp($6); delete $6;

		e->set( v );

		$$ = e;
	}
;

matrix_value
: '[' '(' number ',' number ',' number ')' ',' '(' number ',' number ',' number ')' ',' '(' number ',' number ',' number ')' ']' {
		brEval *e;

		double m[3][3];

		e = new brEval;

		m[0][0] = stDoubleFromIntOrDoubleExp($3); delete $3;
		m[0][1] = stDoubleFromIntOrDoubleExp($5); delete $5;
		m[0][2] = stDoubleFromIntOrDoubleExp($7); delete $7;
		
		m[1][0] = stDoubleFromIntOrDoubleExp($11); delete $11;
		m[1][1] = stDoubleFromIntOrDoubleExp($13); delete $13;
		m[1][2] = stDoubleFromIntOrDoubleExp($15); delete $15;

		m[2][0] = stDoubleFromIntOrDoubleExp($19); delete $19;
		m[2][1] = stDoubleFromIntOrDoubleExp($21); delete $21;
		m[2][2] = stDoubleFromIntOrDoubleExp($23); delete $23;

		e->set( m );

		$$ = e;
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
		stUnusedInstanceVarWarning(currentObject);
	}
| objecttype '{' variabledef definitionlist '}' {
		stUnusedInstanceVarWarning(currentObject);
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
: WORD_VALUE ':' WORD_VALUE object_aka object_version { 
		stObject *parentObject, *akaObject;
		brObject *o;
		float version;

		if($5) version = $5;
		else version = 1.0;

		o = brObjectFindWithTypeSignature( parseEngine, $3, STEVE_TYPE_SIGNATURE );

		if(o) currentObject = (stObject*)o->userData;
		else currentObject = NULL;

		if($4) {
			o = brObjectFindWithTypeSignature( parseEngine, $4, STEVE_TYPE_SIGNATURE );

			if(o) akaObject = (stObject*)o->userData;
			else akaObject = NULL;
		}
		else akaObject = NULL;

		if(currentObject && gReparse) {
			stObjectFree(currentObject);
			currentObject = NULL;
			akaObject = NULL;
		}

		if(currentObject) {
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
			brObject *o = brObjectFindWithTypeSignature( parseEngine, $1, STEVE_TYPE_SIGNATURE );

			if(o) parentObject = (stObject*)o->userData;
			else parentObject = NULL;

			if(!parentObject && strcmp($1, "NULL")) {
				stParseError(parseEngine, PE_UNKNOWN_OBJECT, "Cannot locate parent class \"%s\" for definition of class \"%s\"", $1, $3);
				slFree($1);
				slFree($3);
				if($4) slFree($4);
			} else {
				currentObject = stObjectNew( parseEngine, steveData, $3, $4, parentObject, version, yyfile );
				currentMethod = NULL;

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
		if($2) currentMethod->code = *$2;

		delete $2;

		stMethodAlignStack(currentMethod);
		
		if( stStoreInstanceMethod(currentObject, $1, currentMethod) ) {
			stParseError(parseEngine, PE_REDEFINITION, "Symbol \"%s\" already defined for class \"%s\"", $1, currentObject->name.c_str() );
			delete currentMethod;
		}

		slFree( $1 );

		// if we found an error somewhere and caught it, we continued so 
		// that the method would be stored before we stop the parsing... 
		// this way we're sure to free the allocated memory.			 

		if( brGetError( parseEngine ) ) YYABORT;
	}
| method_head variable_list method_code {
		if($3) currentMethod->code = *$3;

		delete $3;

		stMethodAlignStack(currentMethod);

		if(stStoreInstanceMethod(currentObject, $1, currentMethod)) {
			stParseError( parseEngine, PE_REDEFINITION, "Symbol \"%s\" already defined for class \"%s\"", $1, currentObject->name.c_str() );
			delete currentMethod;
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
		currentMethod = new stMethod($2, $3, yyfile, lineno);

		delete $3;

		$$ = $2; 
	}
;

default_value
: '=' vector_value { $$ = $2; }
| '=' matrix_value { $$ = $2; }
| '=' number { 
		brEval *e = new brEval;

		e->set( stDoubleFromIntOrDoubleExp($2) ); delete $2;

		$$ = e;
	}
| '=' STRING_VALUE {
		brEval *e = new brEval;
		char *str;

		str = slDequote($2);

		e->set( str );

		slFree( str );

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
		stVar *var = new stVar($1, $2);

		slFree($1);

		if(!currentMethod) {
			if(!stInstanceNewVar(var, currentObject)) {
				stParseError(parseEngine, PE_REDEFINITION, "Redefinition of symbol \"%s\" in class \"%s\"", var->name.c_str(), currentObject->name.c_str() );
				delete var;
			}
		} else {
			if(!stMethodAddVar(var, currentMethod)) {
				stParseError(parseEngine, PE_REDEFINITION, "Redefinition of symbol \"%s\" in method \"%s\"", var->name.c_str(), currentMethod->name.c_str() );
				delete var;
			}
		}

		$$ = var;
	}
| WORD_VALUE ',' variable {
		// we have to get the variable type from the existing variable

		stVar *otherVar, *thisVar;
		stVarType *newType;

		otherVar = $3;

		newType = otherVar->type->copy();
		
		thisVar = new stVar($1, newType);

		slFree($1);

		if(!currentMethod) {
			if(!stInstanceNewVar(thisVar, currentObject)) {
				stParseError(parseEngine, PE_REDEFINITION, "Redefinition of symbol \"%s\" in class \"%s\"", thisVar->name.c_str(), currentObject->name.c_str() );
				delete thisVar;
			}
		} else {
			if(!stMethodAddVar(thisVar, currentMethod)) {
				stParseError(parseEngine, PE_REDEFINITION, "Redefinition of symbol \"%s\" in method \"%s\"", thisVar->name.c_str() , currentMethod->name.c_str() );
				delete thisVar;
			}
		}

		$$ = thisVar;
	}
;

keyword_and_variables
: keyword_and_variable_list { $$ = $1; }
| /* empty */ { $$ = new std::vector< stKeywordEntry* >(); }
;

keyword_and_variable_list 
: keyword_and_variable { 
		$$ = new std::vector< stKeywordEntry* >();
		$$->push_back($1);
	}
| keyword_and_variable_list keyword_and_variable { 
		$$ = $1;
		$1->push_back($2);
	}
;

keyword_and_variable
: WORD_VALUE WORD_VALUE default_value type {
		stVar *v = new stVar($2, $4);

		$$ = stNewKeywordEntry( $1, v, $3 );

		slFree($1);
		slFree($2);
		delete $3;
	}
| WORD_VALUE WORD_VALUE type {
		stVar *v = new stVar($2, $3);

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
		$$ = new stCodeArrayExp($2, yyfile, lineno);
		delete $2;
	}
;

code
: statement { 
		$$ = new std::vector< stExp* >();
		$$->push_back($1);
	}
| code statement { 
		$1->push_back($2);
		$$ = $1;
	}
;

statement
: compound_statement { $$ = $1; }
| control_statement { $$ = $1; }
| simple_statement END { $$ = $1; }
| simple_statement error { 
		if($1) delete $1;

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
		stAssignExp *ae = new stAssignExp(currentMethod, currentObject, $2, NULL, yyfile, lineno);
		slFree($2);

		if(strcmp($3, "in")) {
			stParseError(parseEngine, PE_PARSE, "Expected \"in\" after list expression of \"foreach\" statement");
		}

		slFree($3);

		if(ae) {
			$$ = new stForeachExp(ae, $4, $6, yyfile, lineno);
		} else {
			delete $4;
			delete $6;
			$$ = NULL;
		}

	}
| WHILE expression ':' statement {
		$$ = new stWhileExp($2, $4, yyfile, lineno);
	}
| IF expression ':' statement {
		$$ = new stIfExp($2, $4, NULL, yyfile, lineno);
	}
| IF expression ':' statement else statement {
		$$ = new stIfExp($2, $4, $6, yyfile, lineno);
	}
| IF expression ':' EMPTY_LIST else statement {
		$$ = new stIfExp($2, NULL, $6, yyfile, lineno);
	}
| FOR expression ',' expression ',' expression ':' statement {
		$$ = new stForExp($2, $4, $6, $8, yyfile, lineno);
	}
;

else
: ELSE
| ELSE ':'
;

simple_statement
: method_call 		{ $$ = $1; }
| PRINT exp_list { 
		$$ = new stPrintExp($2, 1, yyfile, lineno); 
		delete $2;
	}
| PRINTF exp_list { 
		$$ = new stPrintExp($2, 0, yyfile, lineno); 
		delete $2;
	}
| DIE { 
		stParseError(parseEngine, PE_SYNTAX, "'die' requires an error message string");
	}
| DIE expression	{ $$ = new stDieExp($2, yyfile, lineno); }
| expression		{ $$ = $1; }
| RETURN expression { 
		$$ = new stReturnExp($2, yyfile, lineno); 
	}
| RETURN {
		$$ = new stReturnExp(NULL, yyfile, lineno); 
	}
| FREE expression { 
		$$ = new stFreeExp($2, yyfile, lineno); 
	}
| WORD_VALUE '=' method_call {
		$$ = new stAssignExp(currentMethod, currentObject, $1, $3, yyfile, lineno);

		if(!$$) delete $3;
		
		slFree($1);
	}
; 

exp_list
: expression			{ 
		$$ = new std::vector< stExp* >();
		$$->push_back($1);
	}
| exp_list ',' expression	{ 
		$$ = $1; 
		$$->push_back($3);
	}
;

method_call
: atomic_expression WORD_VALUE keyword_list { 
		$$ = new stMethodExp($1, $2, $3, yyfile, lineno);

		delete $3;

		slFree($2);
	}
| atomic_expression WORD_VALUE {
		std::vector< stKeyword* > keywords;

		$$ = new stMethodExp($1, $2, &keywords, yyfile, lineno);

		slFree($2);
	}
;

keyword_list 
: WORD_VALUE expression {
		$$ = new std::vector< stKeyword* >();
		$$->push_back(new stKeyword((char *)$1, (stExp*)$2));

		slFree($1);
	}
| keyword_list WORD_VALUE expression {
		$$->push_back(new stKeyword($2, (stExp*)$3));

		slFree($2);
	}
;

expression
: log_expression
| WORD_VALUE math_assign_operator expression {
		stExp *loadExp, *binExp;

		loadExp = new stLoadExp(currentMethod, currentObject, $1, yyfile, lineno);

		binExp = new stBinaryExp($2, loadExp, $3, yyfile, lineno);

		$$ = new stAssignExp(currentMethod, currentObject, $1, binExp, yyfile, lineno);

		slFree($1);
	}
| WORD_VALUE '=' expression {
		$$ = new stAssignExp(currentMethod, currentObject, $1, $3, yyfile, lineno);

		if(!$$) delete $3;

		slFree($1);
	}
| WORD_VALUE PLUSPLUS {
		stExp *var = new stLoadExp(currentMethod, currentObject, $1, yyfile, lineno);
		stExp *one = stIntExp(1, yyfile, lineno);
		stExp *newExp = new stBinaryExp(BT_ADD, var, one, yyfile, lineno); 

		$$ = new stAssignExp(currentMethod, currentObject, $1, newExp, yyfile, lineno);

		slFree($1);
	}
| WORD_VALUE MINUSMINUS {
		stExp *var = new stLoadExp(currentMethod, currentObject, $1, yyfile, lineno);
		stExp *one = stIntExp(1, yyfile, lineno);
		stExp *newExp = new stBinaryExp(BT_SUB, var, one, yyfile, lineno); 

		$$ = new stAssignExp(currentMethod, currentObject, $1, newExp, yyfile, lineno);

		slFree($1);
	}
| atomic_expression vector_element '=' expression {
		$$ = new stVectorElementAssignExp($1, $4, $2, yyfile, lineno);
	}
| atomic_expression vector_element math_assign_operator expression {
		stExp *element = new stVectorElementExp(new stDuplicateExp($1, yyfile, lineno), $2, yyfile, lineno);
		stExp *binaryExp = new stBinaryExp($3, element, $4, yyfile, lineno);

		$$ = new stVectorElementAssignExp($1, binaryExp, $2, yyfile, lineno);
	}
| atomic_expression '{' expression'}' '=' expression {
		$$ = new stListIndexAssignExp($1, $3, $6, yyfile, lineno);
	}
| atomic_expression '{' expression'}' math_assign_operator expression {
		stExp *loadExp, *binExp;

		loadExp = new stListIndexExp($1, $3, yyfile, lineno);
		binExp = new stBinaryExp($5, loadExp, $6, yyfile, lineno);

		// we're reusing $1 and $3 -- so we have to "duplicate" them

		$$ = new stListIndexAssignExp(new stDuplicateExp($1, yyfile, lineno), new stDuplicateExp($3, yyfile, lineno), binExp, yyfile, lineno);
	}
| WORD_VALUE '[' expression']' '=' expression {
		$$ = new stArrayIndexAssignExp(currentMethod, currentObject, $1, $3, $6, yyfile, lineno);
		slFree($1);
	}
| WORD_VALUE '[' expression']' math_assign_operator expression {
		stExp *loadExp, *binExp;

		loadExp = new stArrayIndexExp(currentMethod, currentObject, $1, $3, yyfile, lineno);
		binExp = new stBinaryExp($5, loadExp, $6, yyfile, lineno);

		$$ = new stArrayIndexAssignExp(currentMethod, currentObject, $1, new stDuplicateExp($3, yyfile, lineno), binExp, yyfile, lineno);

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
		$$ = new stBinaryExp(BT_LOR, $1, $3, yyfile, lineno); 
	}
| land_expression
;

land_expression
: land_expression LAND land_expression { 
		$$ = new stBinaryExp(BT_LAND, $1, $3, yyfile, lineno);
	}
| comp_expression
; 

comp_expression
: comp_expression EQ comp_expression { 
		$$ = new stBinaryExp(BT_EQ, $1, $3, yyfile, lineno); 
	}
| comp_expression NE comp_expression { 
		$$ = new stBinaryExp(BT_NE, $1, $3, yyfile, lineno); 
	}
| comp_expression GE comp_expression { 
		$$ = new stBinaryExp(BT_GE, $1, $3, yyfile, lineno); 
	}
| comp_expression GT comp_expression { 
		$$ = new stBinaryExp(BT_GT, $1, $3, yyfile, lineno); 
	}
| comp_expression LE comp_expression { 
		$$ = new stBinaryExp(BT_LE, $1, $3, yyfile, lineno); 
	}
| comp_expression LT comp_expression { 
		$$ = new stBinaryExp(BT_LT, $1, $3, yyfile, lineno); 
	}
| add_expression
;

add_expression 
: add_expression '+' add_expression { 
		$$ = new stBinaryExp(BT_ADD, $1, $3, yyfile, lineno); 
	}
| add_expression '-' add_expression { 
		$$ = new stBinaryExp(BT_SUB, $1, $3, yyfile, lineno); 
	}
| mul_expression			{ $$ = $1; }
;

mul_expression
: mul_expression '*' mul_expression { 
		$$ = new stBinaryExp(BT_MUL, $1, $3, yyfile, lineno); 
	}
| mul_expression '/' mul_expression { 
		$$ = new stBinaryExp(BT_DIV, $1, $3, yyfile, lineno); 
	}
| mul_expression '%' mul_expression { 
		$$ = new stBinaryExp(BT_MOD, $1, $3, yyfile, lineno); 
	}
| mul_expression '^' mul_expression { 
		$$ = new stBinaryExp(BT_POW, $1, $3, yyfile, lineno); 
	}
| unary_expression			{ $$ = $1; }
;

unary_expression 
: '!' unary_expression	{ $$ = new stUnaryExp(UT_NOT, $2, yyfile, lineno); }
| '-' unary_expression	{ $$ = new stUnaryExp(UT_MINUS, $2, yyfile, lineno); }
| atomic_expression 	{ $$ = $1; }
| POP atomic_expression {
		$$ = new stListRemoveExp($2, NULL, yyfile, lineno);
	}
| UNPREPEND atomic_expression {
		$$ = new stListRemoveExp($2, stIntExp(0, yyfile, lineno), yyfile, lineno);
	}
| PUSH expression WORD_VALUE atomic_expression {
		if(strcmp($3, "onto")) {
			stParseError(parseEngine, PE_SYNTAX, "Expecting \"onto <expression>\" after \"push\" operator");
			slFree($3);
			delete $2;
			delete $4;
			$$ = NULL;
		} else {
			$$ = new stListInsertExp($4, $2, NULL, yyfile, lineno);
			slFree($3);
		}
	}
| PREPEND expression WORD_VALUE atomic_expression {
		if(strcmp($3, "onto")) {
			stParseError(parseEngine, PE_SYNTAX, "Expecting \"onto <expression>\" after \"append\" operator");
			slFree($3);
			delete $2;
			delete $4;
			$$ = NULL;
		} else {
			$$ = new stListInsertExp($4, $2, stIntExp( 0, yyfile, lineno ), yyfile, lineno);
			slFree($3);
		}
	}
| INSERT expression WORD_VALUE atomic_expression '{' expression '}' {
		if(strcmp($3, "at")) {
			stParseError(parseEngine, PE_SYNTAX, "Expecting \"at <expression>\" after \"insert\" operator");
			slFree($3);
			delete $2;
			delete $4;
			delete $6;
		} else {
			$$ = new stListInsertExp($4, $2, $6, yyfile, lineno);
			slFree($3);
		}
	}
| REMOVE atomic_expression '{' expression '}' {
		$$ = new stListRemoveExp($2, $4, yyfile, lineno);
	}
| SORT atomic_expression WORD_VALUE WORD_VALUE {
		if(strcmp($3, "with")) {
			stParseError(parseEngine, DEBUG_ALL, "Expecting \"with <method_name>\" after \"sort\" operator");
			slFree($3);
			slFree($4);
			delete $2;
			$$ = NULL;
		} else {
			$$ = new stSortExp($2, $4, yyfile, lineno);
			slFree($3);
			slFree($4);
		}
	}
| COPYLIST atomic_expression {
	$$ = new stCopyListExp($2, yyfile, lineno);
}
;

atomic_expression
: literal_value
| atomic_expression vector_element { $$ = new stVectorElementExp($1, $2, yyfile, lineno); }
| '|' expression '|' { 
		$$ = new stLengthExp($2, yyfile, lineno); 
	}
| ST_EVAL		{ $$ = new stEvalExp($1, yyfile, lineno); }
| atomic_expression '{' expression '}' { 
		$$ = new stListIndexExp($1, $3, yyfile, lineno); 
	}
| WORD_VALUE '[' expression ']' {
		$$ = new stArrayIndexExp(currentMethod, currentObject, $1, $3, yyfile, lineno);

		slFree($1);

		if(!$$) delete $3;
	}
| WORD_VALUE { 
		$$ = new stLoadExp(currentMethod, currentObject, $1, yyfile, lineno);

		slFree($1);

		if(!$$) {
			/* this parse error is handled by new stLoadExp */
		}
	}
| SUPER {
		$$ = new stSuperExp(yyfile, lineno);
	}
| SELF {
		$$ = new stSelfExp(yyfile, lineno);
	}
| NEW WORD_VALUE { 
		$$ = new stInstanceExp($2, stIntExp(1, yyfile, lineno), yyfile, lineno); 
		slFree($2);
	}
| NEW STRING_VALUE { 
		char *unquoted = slDequote($2);;
		$$ = new stInstanceExp(unquoted, stIntExp(1, yyfile, lineno), yyfile, lineno); 
		slFree($2);
		slFree(unquoted);
	}
| atomic_expression NEW WORD_VALUE { 
		$$ = new stInstanceExp($3, $1, yyfile, lineno); 
		slFree($3);
	}
| '(' expression ')'		{ $$ = $2; }
| DEBUG '(' expression ')'	{ 
		((stExp*)$3)->debug = 1;
		$$ = $3; 
	}
| '(' method_call ')'		{ $$ = $2; }
| RANDOM '[' expression	']' { 
		$$ = new stRandomExp($3, yyfile, lineno); 
	}
| '{' exp_list '}' {
		$$ = new stListExp($2, yyfile, lineno);
		delete $2;
	}
| EMPTY_LIST {
		$$ = new stListExp(NULL, yyfile, lineno);
	}
| WORD_VALUE '(' exp_list ')' { 
		brInternalFunction *func = brEngineInternalFunctionLookup(parseEngine, $1);

		if(!func) {	  
			unsigned int n;
			stParseError(parseEngine, PE_UNKNOWN_FUNCTION, "Internal function \"%s\" not found", $1);

			for(n=0; n < $3->size(); n++) delete (*$3)[n];

			delete $3;
			slFree($1);
			$$ = NULL;
		} else {
			$$ = new stCCallExp(parseEngine, func, $3, yyfile, lineno); 
			delete $3;
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
			$$ = new stCCallExp(parseEngine, func, NULL, yyfile, lineno); 

			slFree($1);

			if(!$$) {
				/* this error is handled by new stCCallExp */
			}
		}
	}
| ALL WORD_VALUE {
		$$ = new stAllExp($2, yyfile, lineno);
		slFree($2);
	}
| WORD_VALUE '.' WORD_VALUE '(' exp_list ')' {
		// hey, this could be a java call! 

		delete $3;
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
		$$ = new stMatrixExp($3, $5, $7, $11, $13, $15, $19, $21, $23, yyfile, lineno);
	}
;

vector
: '(' expression ',' expression ',' expression ')' { 
		$$ = new stVectorExp($2, $4, $6, yyfile, lineno); 
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
		$$ = new stStringExp(unquoted, currentMethod, currentObject, yyfile, lineno);

		slFree( $1 );
		slFree( unquoted );

		if( !$$ ) {
			// this error is handled by new stStringExp 
		}
	}
;

type
: '(' TYPE ')' {
		$$ = new stVarType($2, AT_NULL, 0, NULL);
	}
| '(' WORD_VALUE TYPE ')' {
		if($3 != AT_INSTANCE) {
			stParseError(parseEngine, PE_SYNTAX, "Expected \"object\" after class name");
			slFree($2);
		} else {
			$$ = new stVarType($3, AT_NULL, 0, $2);
			slFree($2);
		}
	}
| '(' INT_VALUE PLURAL_TYPE ')' {
		$$ = new stVarType(AT_ARRAY, $3, $2, NULL);
	}
| '(' INT_VALUE WORD_VALUE PLURAL_TYPE ')' {
		if($4 != AT_INSTANCE) {
			stParseError(parseEngine, PE_SYNTAX, "Expected \"objects\" after class name");
			slFree($3);
		} else {
			$$ = new stVarType(AT_ARRAY, $4, $2, $3);
			slFree($3);
		}
	}
;

number
: INT_VALUE	 		{ $$ = stIntExp($1, yyfile, lineno); }
| FLOAT_VALUE	   		{ $$ = stDoubleExp($1, yyfile, lineno); }
;

%%

void stParseSetEngine(brEngine *e) {
	parseEngine = e;
	stParseSetObjectAndMethod( NULL, NULL );
	brClearError( e );
}

void stParseSetObjectAndMethod(stObject *o, stMethod *m) {
	currentMethod = m;
	currentObject = o;
}

void stParseSetSteveData(stSteveData *data) {
	steveData = data;
}

double stDoubleFromIntOrDoubleExp( stExp *inExp ) {
	brEval *e = &( ( stEvalExp* )inExp ) -> _eval;

	if( e->type() == AT_DOUBLE ) return BRDOUBLE( e );
	if( e->type() == AT_INT ) return (double)BRINT( e );

	return 0.0;
}

int stGetTypeForString( const char *type ) {
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

void stParseAddDocumentingComment( const char *comment ) {
	while( *comment && ( *comment == '#' || *comment == '%' || isspace( *comment ) ) ) comment++;

	std::string *target = NULL;

	if( currentMethod ) {
		target = &currentMethod->_comment;
	} else if( currentObject ) {
		target = &currentObject->_comment;
	}

	if( target ) {
		if( target->size() != 0 ) (*target) += " ";
		(*target) += comment;
	}
}
