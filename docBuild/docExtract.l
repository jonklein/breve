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

int level = 0;
int noprint = 0;
%}

WORD	[_a-zA-Z][_a-zA-Z0-9]*
WHITE	[\t\ \n]*

%x OBJECT
%option noyywrap

/* 
  this is a pretty simple hack to take a breve file and spit 
  out 1) object names, 2) method declarations and 3) documenting
  comments.

  this is done in lex to avoid the problem of parenthesis balencing 
  in perl, etc.
*/

%%

{WORD}{WHITE}:{WHITE}{WORD}{WHITE}[^%#]*/\{	{ printf("# %s\n", yytext); BEGIN(OBJECT); level = 0; }

<OBJECT>\{				{ level++; }

<OBJECT>\}				{ level--; if(level == 0) BEGIN(0); }

<OBJECT>\+{WHITE}section{WHITE}.* 	{ puts(yytext); }
<OBJECT>\+{WHITE}to[\t\ \n][^:]*	{ noprint = 0; puts(yytext); }
<OBJECT>\-{WHITE}to[\t\ \n][^:]*	{ noprint = 1; }
<OBJECT>\+{WHITE}variables{WHITE}[^:]*	{ puts(yytext); }
<OBJECT>%.*				{ if(!noprint) puts(yytext); }

<OBJECT>.				;
<OBJECT>\n				;

.					;
\n					;

%%

int main(int argc, char **argv) {
    int n;

    printf("\n");

    for(n=1;n<argc;n++) {
        if((yyin = fopen(argv[n], "r"))) {
            while(yylex());
        } else {
            fprintf(stderr, "cannot open %s, skipping...\n", argv[n]);
        }
    }

	return 0;
}
