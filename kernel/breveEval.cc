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

#include "kernel.h"

int brEvalCopy(brEval *s, brEval *d) {
	d->type = s->type;

	switch(s->type) {
	case AT_NULL:
		break;
	case AT_INT:
		BRINT(d) = BRINT(s);
		break;
	case AT_DOUBLE:
		BRDOUBLE(d) = BRDOUBLE(s);
		break;
	case AT_LIST:
	case AT_DATA:
	case AT_HASH:
	case AT_POINTER:
	case AT_INSTANCE:
		BRPOINTER(d) = BRPOINTER(s);
		break;
	case AT_VECTOR:
		slVectorCopy(&BRVECTOR(s), &BRVECTOR(d));
		break;
	case AT_MATRIX:
		slMatrixCopy(BRMATRIX(s), BRMATRIX(d));
		break;
	case AT_STRING:
		BRSTRING(d) = slStrdup(BRSTRING(s));
		break;
	default:
		slMessage(DEBUG_ALL, "warning: unknown evaluation type (%d) in brEvalCopy\n", s->type);
		return EC_ERROR;
	}

	return EC_OK;
}

/*!
	\brief Returns the description of an object.

	Calls the get-description method for an object.
*/

char *brObjectDescription(brInstance *i) {
	brEval result;
	int r;

	if (!i || i->status != AS_ACTIVE)
		return slStrdup("");

	r = brMethodCallByName(i, "get-description", &result);

	if (r != EC_OK || result.type != AT_STRING)
		return slStrdup("");

	return slStrdup(BRSTRING(&result));
}

/*!
	\brief Formats a brEval as text by calling stFormatEvaluationWithSeenList.

	Returns an slMalloc'd string.
*/

char *brFormatEvaluation(brEval *e, brInstance *i) {
	slList *seen = NULL;
	char *result;

	result = brFormatEvaluationWithSeenList(e, i, &seen);

	if (seen)
		slListFree(seen);

	return result;
}

/*!
	\brief Formats a brEval as text.

	Does the work of \ref stFormatEvaluation, and keeps track of circular
	list references via a "seen" list.  This method is called by
	\ref stFormatEvaluation and shouldn't be called directly.
*/

char *brFormatEvaluationWithSeenList(brEval *e, brInstance *i, slList **seen) {
	char *result;

	switch(e->type) {
	case AT_STRING:
		if (BRSTRING(e))
			return slStrdup(BRSTRING(e));
		else
			return slStrdup("");
	case AT_INT:
		result = (char *)slMalloc(60);
		snprintf(result, 60, "%d", BRINT(e));
		return result;
	case AT_DOUBLE:
		result = (char *)slMalloc(60);
		snprintf(result, 60, "%f", BRDOUBLE(e));
		return result;
	case AT_VECTOR:
		result = (char *)slMalloc(180);
		if (BRVECTOR(e).x > 1.0e10 || BRVECTOR(e).x > 1.0e10 || BRVECTOR(e).z > 1.0e10)
			snprintf(result, 180, "(%.5e, %.5e, %.5e)", BRVECTOR(e).x, BRVECTOR(e).y, BRVECTOR(e).z);
		else
			snprintf(result, 180, "(%.5f, %.5f, %.5f)", BRVECTOR(e).x, BRVECTOR(e).y, BRVECTOR(e).z);
		return result;
	case AT_MATRIX:
		result = (char *)slMalloc(300);
		snprintf(result, 300, "[ (%.5f, %.5f, %.5f), (%.5f, %.5f, %.5f), (%.5f, %.5f, %.5f) ]", BRMATRIX(e)[0][0], BRMATRIX(e)[0][1], BRMATRIX(e)[0][2], BRMATRIX(e)[1][0], BRMATRIX(e)[1][1], BRMATRIX(e)[1][2], BRMATRIX(e)[2][0], BRMATRIX(e)[2][1], BRMATRIX(e)[2][2]);
		return result;
	case AT_INSTANCE:
	{
		brInstance *pi;
		char *desc;
		size_t len;

		if (!(pi = BRINSTANCE(e)))
			return slStrdup("0x0 (NULL)");

		desc = brObjectDescription(pi);

		len = strlen(pi->object->name) + strlen(desc) + sizeof(void *) * 2 + 5;

		result = (char *)slMalloc(len);
		snprintf(result, len, "%s (%p) %s", pi->object->name, pi, desc);
		slFree(desc);

		return result;
	}
	case AT_POINTER:
		if (BRPOINTER(e)) {
			result = (char *)slMalloc(20);
			snprintf(result, 20, "%p", BRPOINTER(e));
			return result;
		} else
			return slStrdup("(NULL pointer)");
	case AT_LIST:
	{
		std::vector<char*> textList;
		std::vector<char*>::iterator ti;
		brEvalListHead *listHead;
		brEvalList *list;
		size_t len = 5;

		listHead = BRLIST(e);
		list = listHead->start;

		if (!list)
			return slStrdup("{ }");

		if (slInList(*seen, listHead)) {
			char text[64];
			snprintf(text, 64, "[circular list reference %p]", listHead);
			return slStrdup(text);
		}

		// update the list of lists seen so that we don't get all circular

		*seen = slListPrepend(*seen, listHead);

		while (list) {
			char *newString;

			newString = brFormatEvaluationWithSeenList(&list->eval, i, seen);
			textList.push_back(newString);

			len += strlen(newString) + 2;
			list = list->next;
		}
		result = (char *)slMalloc(len);

		sprintf(result, "{ ");

		for (ti = textList.begin(); ti != textList.end(); ti++ ) {
			char *newString = *ti;

			strcat(result, newString);
			if (ti + 1 != textList.end())
				strcat(result, ", ");

			slFree(newString);
		}
		strcat(result, " }");

		return result;
	}
	case AT_ARRAY:
		return slStrdup("{ array }");
	case AT_HASH:
		return slStrdup("{ hash table }");
	case AT_NULL:
		return slStrdup("(NULL expression)");
	case AT_DATA:
		return brDataHexEncode(BRDATA(e));
	default:
		if (i)
			brEvalError(i->engine, EE_INTERNAL, "unknown atomic type %d in slFormatEvaluation\n", e->type);
		return slStrdup("");
	}
}
