%{
#include "steve.h"

#define YY_NO_UNPUT

int tlevel = 0;
char *result = NULL;
unsigned int length = 0;

void slFormatAppend(char *text, int tlevel);
char *slFormatText();

char *slFormatParseBuffer;
int slFormatParseLength;
int slFormatParseLocation;
int midline = 0;

#define YY_INPUT(buf,result,max_size) \
	result = ((slFormatParseLocation == slFormatParseLength) ? YY_NULL : (buf[0] = slFormatParseBuffer[slFormatParseLocation++], 1))
%}

%option noyywrap
%option prefix="slFormat"

%%

^[\t\ ]*					;

#.*					{ slFormatAppend(yytext, tlevel); }

[+-][\t\ ]*to.*				{ tlevel = 1; slFormatAppend(yytext, tlevel); tlevel++; }
[+-][\t\ ]*variables.*		{ tlevel = 1; slFormatAppend(yytext, tlevel); tlevel++; }

\{[\t\ ]*			{ 
						if(!midline) slFormatAppend(yytext, tlevel); 
						else slFormatAppend(yytext, 0);
						tlevel++; 
						midline = 1;
					}

\}[\t\ ]*			{ 
						if(tlevel == 2) tlevel--; tlevel--; 
						if(!midline) slFormatAppend(yytext, tlevel); 
						else slFormatAppend(yytext, 0);
						midline = 1;
					}

[^\t\n\}\{\ ][^\{\}\n]*	{ 
						if(!midline) slFormatAppend(yytext, tlevel); 
						else slFormatAppend(yytext, 0); 

						midline = 1; 
					}

\n					{ slFormatAppend("\n", 0); midline = 0; }

%%

char *slFormatText(char *text) {
	slFormatParseBuffer = text;
	slFormatParseLength = strlen(text);
	slFormatParseLocation = 0;

	length = 1024;
	result = (char*)slMalloc(length);
	result[0] = 0;
	while(slFormatlex());

	return result;
}

void slFormatAppend(char *text, int tlevel) {
	if(strlen(text) + strlen(result) + 2 + tlevel > length) {
		length *= 2;
		result = (char*)slRealloc(result, length);
	}

	while(tlevel--) strcat(result, "\t");
	strcat(result, text);
}
