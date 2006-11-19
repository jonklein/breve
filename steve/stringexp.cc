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
#include "expression.h"
#include "evaluation.h"

/*!
	\brief Preprocesses a string for runtime formating and printing.

	Allows for $xxx perl-style variables embedded in strings.  Prepares
	variable offsets so that the string can be quickly assembled later on.
	Does terrible things to theString.  Too horrible to discuss here.
*/

stStringExp::stStringExp( char *theString, stMethod *m, stObject *o, const char *file, int line ) : stExp( file, line ) {
	char *s;
	char sym, prev = 0;
	char *varstart, *varend, *varname;
	stSubstringExp *sub;

	bool matchBrace = 0;

	s = theString;
	baseSize = 0;

	while (( sym = *s ) ) {
		// if we find a '$' that is NOT preceeded by a '\'
		// then we're dealing with a variable or expression

		if ( sym == '$' && prev != '\\' ) {
			/* if the next character exists... */
			int offset;

			if ( *( ++s ) ) {
				// mark the start, find the end of the variable 

				// if the first char is a '{', then they're using braces to separate the variable
				// skip ahead one character

				if( *s == '{' && *( s + 1 ) ) {
					matchBrace = 1;
					s++;
				}

				varstart = s;

				while ( *s && ( isalnum( *s ) || *s == '_' || *s == '-' ) ) s++;

				varend = s;

				varname = new char[( varend - varstart ) + 1];

				strncpy( varname, varstart, ( varend - varstart ) );

				varname[varend - varstart] = 0;

				sub = new stSubstringExp( file, line );

				sub->offset = baseSize;

				// is this an array or list?

				if ( *varend == '{' || *varend == '[' ) {
					char *numend = varend + 1;
					char *number;
					stExp *index;

					while ( *numend == ' ' || *numend == '\t' || isdigit( *numend ) ) numend++;

					if (( *varend == '{' && *numend == '}' ) || ( *varend == '[' && *numend == ']' ) ) {
						/* we have a valid array or list exp */

						number = new char[numend - varend];
						strncpy( number, varend + 1, numend - ( varend + 1 ) );
						number[numend - ( varend + 1 )] = 0;
						offset = atoi( number );

						index = new stIntExp( offset, file, line );
						delete[] number;

						if ( *varend == '{' ) {
							stExp *listExp = new stLoadExp( m, o, varname, file, line );

							sub->loadExp = new stListIndexExp( listExp, index, file, line );
						} else {
							sub->loadExp = new stArrayIndexExp( m, o, varname, index, file, line );
						}

						s = numend + 1;
					}
				} else {
					// it's a regular variable.  but is it a special case or self or super?

					if ( !strcmp( varname, "super" ) || !strcmp( varname, "self" ) ) {
						sub->loadExp = new stSelfExp( file, line );
					} else {
						sub->loadExp = new stLoadExp( m, o, varname, file, line );
					}
				}

				substrings.push_back( sub );

				if ( !sub->loadExp ) {
					stParseError( o->engine, PE_UNKNOWN_SYMBOL, "Error locating variable \"%s\" while parsing string", varname );
				}

				// skip past the end brace

				if( matchBrace && *s == '}' ) s++;

				delete[] varname;
			}
		} else if ( sym == 'n' && prev == '\\' ) {
			/* if we find a \n, then replace the previous char (\) */
			/* with a newline */

			theString[baseSize - 1] = '\n';

			s++;
			prev = '\n';
		} else if ( sym == 't' && prev == '\\' ) {
			/* if we find a \t, then replace the previous char (\) */
			/* with a tab */

			theString[baseSize - 1] = '\t';

			s++;
			prev = '\t';
		} else if ( sym == '\"' && prev == '\\' ) {
			/* if we find a \", then replace the previous char (\) */
			/* with a quote mark */

			theString[baseSize - 1] = '\"';

			s++;
			prev = '\"';
		} else if ( sym == '\\' && prev == '\\' ) {
			/* if we find a \\, then leave the first \ in place, skip the second */

			s++;

			/* this is an intentional '\', not an escape symbol, so we'll */
			/* make the previous character 0. */

			prev = 0;
		} else {
			theString[baseSize++] = sym;

			s++;
			prev = sym;
		}
	}

	theString[baseSize] = 0;

	string = slStrdup( theString );

	type = ET_STRING;
}

/*!
	\brief Formats a preprocessed string expression into a string.

	Uses the preprocessed string to lookup variable values and assemble
	a string for printing (or whatever) in steve.
*/

int stProcessString( stStringExp *s, stRunInstance *i, brEval *target ) {
	brEval subtar;
	stSubstringExp *ss;
	char *newstr;
	unsigned int n;
	int length, currentPosition;

	if ( s->substrings.size() == 0 ) {

		target->set( s->string );

		return EC_OK;
	}

	length = s->baseSize + 1;

	for ( n = 0;n < s->substrings.size();n++ ) {
		ss = s->substrings[n];
		stExpEval( ss->loadExp, i, &subtar, NULL );

		ss->string = stFormatEvaluation( &subtar, i->instance );

		length += ss->string.size();
	}

	// the substrings are in descending order--start from the last one
	// to find the first position we need to edit

	newstr = ( char* )slMalloc( length );

	newstr[0] = 0;

	// currentPosition is the position in the "template" string
	// so it doesn't count the length of the variable strings

	currentPosition = 0;

	for ( n = 0; n < s->substrings.size(); n++ ) {
		ss = s->substrings[ n ];
		strncat( newstr, &s->string.c_str()[currentPosition], ss->offset - currentPosition );
		strncat( newstr, ss->string.c_str(), ss->string.size() );
		currentPosition = ss->offset;

		ss->string = "";
	}

	strcat( newstr, &s->string[currentPosition] );

	target->set( newstr );

	slFree( newstr );

	return EC_OK;
}

/*!
	\brief Frees a steve string expression.
*/

stStringExp::~stStringExp() {
	unsigned int n;
	stSubstringExp *ss;

	for ( n = 0;n < substrings.size();n++ ) {
		ss = substrings[n];
		delete ss->loadExp;
		delete ss;
	}
}

/*!
	\brief Calls \ref brFormatEvaluation.

	Translates between breve/steve objects.  Returns a slMalloc'd string.
*/

char *stFormatEvaluation( brEval *e, stInstance *i ) {
	char *r;

	r = brFormatEvaluation( e, i->breveInstance );

	return r;
}
