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

#include "java.h"

#ifdef HAVE_LIBJAVA
int brEvalToJValue(brJavaBridgeData *bridge, brEval *e, jvalue *v, char javaType) {
	switch(javaType) {
		case 'V':	
			e->type = AT_NULL;
			break;
		case 'I':
			v->i = BRINT(e);
			break;
		case 'D':
			v->d = BRDOUBLE(e);
			break;
		case 'F':
			v->f = BRDOUBLE(e);
			break;
		case 'T':
			v->l = brMakeJavaString(bridge, BRSTRING(e));
			break;
		case 'O':
			// extract the java instance
			v->l = ((brJavaInstance*)(BRINSTANCE(e)->userData))->instance;
			break;
		default:
			return EC_ERROR;
	}

	return EC_OK;
}

char brJTypeForType(unsigned char breveType) {
	switch(breveType) {
		case AT_NULL:
			return 'V';
			break;
		case AT_INT:
			return 'I';
			break;
		case AT_DOUBLE:
			return 'D';
			break;
		case AT_INSTANCE:
			return 'O';
			break;
		case AT_STRING:
			return 'L';
			break;
		default:
			return 'V';
			break;
	}

}

jstring brMakeJavaString(brJavaBridgeData *bridge, char *string) {
	jchar *characters;
	jstring s;
	unsigned int n;

	characters = new jchar[sizeof(jchar) * (strlen(string) + 1)];

	for(n=0;n<strlen(string);n++) characters[n] = string[n];

	s = (*bridge->env).NewString(characters, strlen(string));

	delete[] characters;

	return s;
}

char *brReadJavaString(brJavaBridgeData *bridge, jstring string) {
	char *result;
	const jchar *characters;
	int n, length;

	length = (*bridge->env).GetStringLength(string);

	result = (char*)slMalloc(length + 1);

	characters = (*bridge->env).GetStringChars(string, NULL);

	for(n=0;n<length;n++) 
		result[n] = characters[n];

	result[n] = 0;

	(*bridge->env).ReleaseStringChars(string, characters);

	return result;
}
#endif
