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

#define YY_NO_UNPUT

#include <string.h>

#include "steveparse.tab.h"

#include "steve.h"

static char *gParseString;
static int gParseLength;
static int gParseLoc = 0;

char *yyfile;
int lineno = 1;

int gErrorLine = 0;

/* yylval might not be big enough for a double */

extern brEngine *parseEngine;
extern stSteveData *gSteveData;

void yyerror(char *c);

FILE *docHandle = NULL;

#define YY_INPUT(buf,result,max_size) \
    result = ((gParseLoc == gParseLength) ? YY_NULL : (buf[0] = gParseString[gParseLoc++], 1))
%}

DIGIT	[0-9]
WORDBREAK	[^0-9a-zA-Z\-_]

%%

#.*						{ /* comment */ };

\+[\ ]*to/{WORDBREAK}	{ return TO_PUBLIC; }
\-[\ ]*to/{WORDBREAK}	{ return TO_PRIVATE; }
\=[\ ]*to/{WORDBREAK}	{ return TO_PROTECTED; }

Controller/{WORDBREAK}	{ return CONTROLLER; }

aka/{WORDBREAK}			{ return AKA; }
version/{WORDBREAK}		{ return VERSION; }
define/{WORDBREAK}		{ return DEFINE; }

@@DEBUG@@				{ return DEBUG; }

int/{WORDBREAK}			{ yylval.integer = AT_INT; return TYPE; }
double/{WORDBREAK}		{ yylval.integer = AT_DOUBLE; return TYPE; }
vector/{WORDBREAK}		{ yylval.integer = AT_VECTOR; return TYPE; }
matrix/{WORDBREAK}		{ yylval.integer = AT_MATRIX; return TYPE; }
object/{WORDBREAK}		{ yylval.integer = AT_INSTANCE; return TYPE; }
pointer/{WORDBREAK}		{ yylval.integer = AT_POINTER; return TYPE; }
list/{WORDBREAK}		{ yylval.integer = AT_LIST; return TYPE; }
string/{WORDBREAK}		{ yylval.integer = AT_STRING; return TYPE; }
float/{WORDBREAK} 		{ yylval.integer = AT_DOUBLE; return TYPE; }
data/{WORDBREAK} 		{ yylval.integer = AT_DATA; return TYPE; }
hash/{WORDBREAK} 		{ yylval.integer = AT_HASH; return TYPE; }

ints/{WORDBREAK}		{ yylval.integer = AT_INT; return PLURAL_TYPE; }
doubles/{WORDBREAK}		{ yylval.integer = AT_DOUBLE; return PLURAL_TYPE; }
vectors/{WORDBREAK}		{ yylval.integer = AT_VECTOR; return PLURAL_TYPE; }
matrices/{WORDBREAK}	{ yylval.integer = AT_MATRIX; return PLURAL_TYPE; }
objects/{WORDBREAK}		{ yylval.integer = AT_INSTANCE; return PLURAL_TYPE; }
pointers/{WORDBREAK}	{ yylval.integer = AT_POINTER; return PLURAL_TYPE; }
lists/{WORDBREAK}		{ yylval.integer = AT_LIST; return PLURAL_TYPE; }
strings/{WORDBREAK}		{ yylval.integer = AT_STRING; return PLURAL_TYPE; }
floats/{WORDBREAK} 		{ yylval.integer = AT_DOUBLE; return PLURAL_TYPE; }
hashes/{WORDBREAK} 		{ yylval.integer = AT_HASH; return PLURAL_TYPE; }

return/{WORDBREAK}		{ return RETURN; }
for[\ \t]*each/{WORDBREAK}		{ return FOREACH; }

push/{WORDBREAK}		{ return PUSH; }
pop/{WORDBREAK}			{ return POP; }
insert/{WORDBREAK}		{ return INSERT; }
remove/{WORDBREAK}		{ return REMOVE; }
sort/{WORDBREAK}		{ return SORT; }

super/{WORDBREAK}		{ return SUPER; }
self/{WORDBREAK}		{ return SELF; }

copylist/{WORDBREAK} 	{ return COPYLIST; }
prepend/{WORDBREAK} 	{ return PREPEND; }
unprepend/{WORDBREAK} 	{ return UNPREPEND; }

\{[\t\ \n]*\}			{ return EMPTY_LIST; }

\+=						{ return PLUSEQ; }
\-=						{ return SUBEQ; }
\/=						{ return DIVEQ; }
\*=						{ return MULEQ; }
\^=						{ return POWEQ; }
\%=						{ return MODEQ; }

while					{ return WHILE; }
for						{ return FOR; }
if						{ return IF; }
else					{ return ELSE; }

in						{ return ST_IN; }

new						{ return NEW; }
free					{ return FREE; }
die						{ return DIE; }
print					{ return PRINT; }
printf					{ return PRINTF; }
random					{ return RANDOM; }
all						{ return ALL; }

use						{ return USE; }
include					{ return INCLUDE; }
plugin					{ return PLUGIN; }
nib_file				{ return NIB_FILE; }
path					{ return PATH; }

\+\+					{ return PLUSPLUS; }
\-\-					{ return MINUSMINUS; }

\+[\ ]*variables/{WORDBREAK}	{ return VARIABLES; }

([a-zA-Z_]|[a-zA-Z_][a-zA-Z0-9\-_]*[a-zA-Z0-9_])	{ 
		brNamespaceSymbol *symbol = brNamespaceLookup(gSteveData->defines, yytext);

		if(symbol) {
			brEval *copy = slMalloc(sizeof(brEval));

			brEvalCopy(symbol->data, copy);
			yylval.eval = copy;

			return ST_EVAL;
		}

		yylval.string = slStrdup(yytext); 
		return WORD_VALUE;
	}

[-]?[0-9]+			{ yylval.integer = atoi(yytext); return INT_VALUE; }
\'.\'				{ yylval.number = yytext[1]; return INT_VALUE; }

[-]?[0-9]*\.[0-9]+	{ yylval.number = atof(yytext); return FLOAT_VALUE; }


\"([^\"]|(\\\"))*\"	{ yylval.string = slStrdup(yytext); return STRING_VALUE; }

^[\ \t]*%.*			{ /* self documenting comment */ };
^[\ \t]*\+[\ ]*section{WORDBREAK}.*	{ /* section */ }

[\t ]				;

::[xX]				{ return X_ELEMENT; }
::[yY]				{ return Y_ELEMENT; }
::[zZ]				{ return Z_ELEMENT; }

"=="				{ return EQ; }
"!="				{ return NE; }
">="				{ return GE; }
"<="				{ return LE; }
"<"					{ return LT; }
">"					{ return GT; }
"&&"				{ return LAND; }
"||"				{ return LOR; }

\.					{ return END; }

\r\n				{ lineno++; }
\r					{ lineno++; }
\n					{ lineno++; }

.					{ return yytext[0]; }

\"[^\"\n]*			{ yyerror("unterminated string constant"); }

%%

int yywrap() {
    return 1;
}

void yyerror(char *c) {
    stParseError(parseEngine, PE_PARSE, "%s in file \"%s\" at line %d near \"%s\"", c, yyfile, lineno, yytext);
    YY_FLUSH_BUFFER;

    gErrorLine = lineno;
}

void stSetParseString(char *string, int length) {
    gParseString = string;
    gParseLength = length;
    gParseLoc = 0;
}
