%{

/*
 *  breve simulation environment
 *  Copyright (C) 2001    jon klein (jklein@spiderland.org)
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

/*
    + slQuickparser.l
    = used for the "go to method..." menu in the OSX application.  we 
    = just need a quick way to go through a buffer and grab all of the 
    = steve method names.  the parser should not be fooled by comments
    = or "+ to" within strings or expressions.
    = 
    = the buffer to parse is first set using slObjectParseSetBuffer, and
    = then subsequent calls to yylex() return TZ_NOTHING, TZ_OBJECT or 
    = TZ_METHOD depending on whether a relevant symbol was found or not.
*/

#define YY_NO_UNPUT

int slQPLevel = 0;

enum {
    TZ_NOTHING = 1,
    TZ_OBJECT,
    TZ_METHOD
};

char *slObjectParseText;
char *slObjectParseBuffer;
int slObjectParseLength;
int slObjectParseLocation;
int slObjectParseLine;

void slObjectParseCountNewlines(char *text);

#define YY_INPUT(buf,result,max_size) \
    result = ((slObjectParseLocation == slObjectParseLength) ? YY_NULL : (buf[0] = slObjectParseBuffer[slObjectParseLocation++], 1))

%}


WORD	[_a-zA-Z][\-_a-zA-Z0-9]*
WHITE	[\t\ \n]*

%x OBJECT

%option noyywrap
%option prefix="slYy"

%%

#.*							;

{WORD}{WHITE}:{WHITE}{WORD}{WHITE}/.*\{	{ slObjectParseText = yytext; BEGIN(OBJECT); slQPLevel = 0; return TZ_OBJECT; }

<OBJECT>\{				{ yytext = NULL; slQPLevel++; return TZ_NOTHING; }

<OBJECT>\}				{ yytext = NULL; slQPLevel--; if(slQPLevel == 0) BEGIN(0); return TZ_NOTHING; }
<OBJECT>\"[^\"]\"			;

<OBJECT>\+{WHITE}to[\t\ \n][^:]*	{ slObjectParseText = yytext; slObjectParseCountNewlines(yytext); return TZ_METHOD; }

<OBJECT>.				;
<OBJECT>\n				{ slObjectParseLine++; }

.					;
\n					{ slObjectParseLine++; }
\r					{ slObjectParseLine++; }

%%

void slObjectParseCountNewlines(char *text) {
	while(*text) {
		if(*text == '\n') slObjectParseLine++;
		text++;
	}
}

void slObjectParseSetBuffer(char *b, int n) {
    slObjectParseBuffer = b;
    slObjectParseLength = n;
    slObjectParseLocation = 0;
    slObjectParseLine = 1;
}
