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
		case AT_STRING:
			BRSTRING(d) = slStrdup(BRSTRING(s));
			break;
		default:
			slMessage(DEBUG_ALL, "warning: unknown evaluation type (%d) in brEvalCopy\n", s->type);
			return EC_ERROR;
			break;
	}

	return EC_OK;
}

int brEvalPointer(brEval *s, void **pointer) {
	*pointer = &s->values;

	switch(s->type) {
		case AT_INT:
			*pointer = &BRINT(s);
			break;
		case AT_DOUBLE:
			*pointer = &BRDOUBLE(s);
			break;
		case AT_VECTOR:
			*pointer = &BRVECTOR(s);
			break;
		default:
			return EC_ERROR;
			break;
	}

	return EC_OK;
}
