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
	+ string.c
	= the purpose of this file is to allow perl-like parsing of strings,
	= such as extracting $variable.
*/

#include <ctype.h>
#include "steve.h"

/*!
	\brief Preprocesses a string for runtime formating and printing.

	Allows for $xxx perl-style variables embedded in strings.  Prepares 
	variable offsets so that the string can be quickly assembled later on.
	Does terrible things to theString.  Too horrible to discuss here.
*/

stExp *stNewStringExp(char *theString, stMethod *m, stObject *o, char *file, int line) {
	stStringExp *se;
	char *s;
	char sym, prev = 0;
	char *varstart, *varend, *varname;
	stSubstringExp *sub;

	slList *expList = NULL;

	se = slMalloc(sizeof(stStringExp));

	se->baseSize = 0;

	s = theString;

	while((sym = *s)) {
		// if we find a '$' that is NOT preceeded by a '\'
		// then we're dealing with a variable or expression

		if(sym == '$' && prev != '\\') {
			/* if the next character exists... */
			int offset;

			if(*(++s)) {
				/* mark the start, find the end of the variable */

				varstart = s;
				while(*s && (isalnum(*s) || *s == '_' || *s == '-')) s++;
				varend = s;

				varname = slMalloc((varend - varstart) + 1);
				strncpy(varname, varstart, (varend - varstart));
				varname[varend - varstart] = 0;

				sub = slMalloc(sizeof(stSubstringExp));
			   	sub->offset = se->baseSize;

				// is this an array or list? 

				if(*varend == '{' || *varend == '[') {
					char *numend = varend + 1;
					char *number;
					stExp *index;

					while(*numend == ' ' || *numend == '\t' || isdigit(*numend)) numend++;

					if((*varend == '{' && *numend == '}') || (*varend == '[' && *numend == ']')) {
						/* we have a valid array or list exp */

						number = slMalloc(numend - varend);
						strncpy(number, varend + 1, numend - (varend + 1));
						number[numend - (varend + 1)] = 0;
						offset = atoi(number);

						index = stNewIntExp(offset, file, line);
						slFree(number);

						if(*varend == '{') {
							stExp *listExp = stNewLoadExp(m, o, varname, file, line);

							sub->loadExp = stNewListIndexExp(listExp, index, file, line);
						} else {
							sub->loadExp = stNewArrayIndexExp(m, o, varname, index, file, line);
						}

						s = numend + 1;
					}
				} else { 
					// it's a regular variable.  but is it a special case or self or super?

					if(!strcmp(varname, "super") || !strcmp(varname, "self")) {
						sub->loadExp = stExpNew(NULL, ET_SELF, file, line);
					} else {
						sub->loadExp = stNewLoadExp(m, o, varname, file, line);
					}
				}

				expList = slListPrepend(expList, sub);

				if(!sub->loadExp) {
					stParseError(o->engine, PE_UNKNOWN_SYMBOL, "Error locating variable \"%s\" while parsing string", varname);
					return NULL;
				}

				slFree(varname);
			}
		} else if(sym == 'n' && prev == '\\') {
			/* if we find a \n, then replace the previous char (\) */
			/* with a newline */

			theString[se->baseSize - 1] = '\n';

			s++;
			prev = '\n';
		} else if(sym == 't' && prev == '\\') {
			/* if we find a \t, then replace the previous char (\) */
			/* with a tab */

			theString[se->baseSize - 1] = '\t';

			s++;
			prev = '\t';
		} else if(sym == '\"' && prev == '\\') {
			/* if we find a \", then replace the previous char (\) */
			/* with a quote mark */

			theString[se->baseSize - 1] = '\"';

			s++;
			prev = '\"';
		} else if(sym == '\\' && prev == '\\') {
			/* if we find a \\, then leave the first \ in place, skip the second */

			s++;

			/* this is an intentional '\', not an escape symbol, so we'll */
			/* make the previous character 0. */

			prev = 0;
		} else {
			theString[se->baseSize++] = sym;

			s++;
			prev = sym;
		}
	}

	theString[se->baseSize] = 0;

	se->string = slStrdup(theString);
	se->substrings = slListToArray(expList);

	slListFree(expList);

	return stExpNew(se, ET_STRING, file, line);
}

/*!
	\brief Formats a preprocessed string expression into a string.

	Uses the preprocessed string to lookup variable values and assemble
	a string for printing (or whatever) in steve.
*/

int stProcessString(stStringExp *s, stRunInstance *i, brEval *target) {
	brEval subtar;
	stSubstringExp *ss;
	char *newstr;
	int n, length, currentPosition;

	target->type = AT_STRING;

	if(!s->substrings || s->substrings->count == 0) {
		BRSTRING(target) = s->string;
		return EC_OK;
	}

	length = s->baseSize + 1;

	for(n=0;n<s->substrings->count;n++) {
		ss = s->substrings->data[n];
		stExpEval(ss->loadExp, i, &subtar, NULL);

		ss->string = stFormatEvaluation(&subtar, i->instance);

		length += strlen(ss->string);
	}

	/* the substrings are in descending order--start from the last one */
	/* to find the first position we need to edit					*/

	n = s->substrings->count - 1;

	newstr = slMalloc(length);
	newstr[0] = 0;

	/* currentPosition is the position in the "template" string */
	/* so it doesn't count the length of the variable strings   */

	currentPosition = 0;

	while(n > -1) {
		ss = s->substrings->data[n];
		strncat(newstr, &s->string[currentPosition], ss->offset - currentPosition);
		strncat(newstr, ss->string, strlen(ss->string));
		currentPosition = ss->offset;
		n--;

		slFree(ss->string);
		ss->string = NULL;
	}

	strcat(newstr, &s->string[currentPosition]);

	BRSTRING(target) = newstr;

	stGCMark(i->instance, target);

	return EC_OK;
}

/*!
	\brief Frees a steve string expression.
*/

void stFreeStringExp(stStringExp *s) {
	int n;
	stSubstringExp *ss;

	if(s->substrings) {
		for(n=0;n<s->substrings->count;n++) {
			ss = s->substrings->data[n];
			stExpFree(ss->loadExp);
			slFree(ss);
		}

		slFreeArray(s->substrings);
	}

	slFree(s->string);
	slFree(s);
}

/*!
	\brief Calls \ref brFormatEvaluation.

	Translates between breve/steve objects.  Returns a slMalloc'd string.
*/

char *stFormatEvaluation(brEval *e, stInstance *i) {
	char *r;

	stConvertSteveInstanceToBreveInstance(e);
	r = brFormatEvaluation(e, i->breveInstance);
	stConvertBreveInstanceToSteveInstance(e);

	return r;
}
