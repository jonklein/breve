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

#include "steve.h"
#include "expression.h"

int brDataCopyObject(stInstance *src, stInstance *dst) {
	std::map< std::string, stVar* >::iterator vi;

	if(src->type != dst->type) return -1;

	for(vi = src->type->variables.begin(); vi != src->type->variables.end(); vi++) {
		brDataCopyVar(vi->second, src, dst);
	}

	return 0;
}

int brDataCopyVar(stVar *v, stInstance *src, stInstance *dst) {
	brEval load, assign;
	stRunInstance ri;

	ri.instance = src;
	ri.type = src->type;

	switch(v->type->type) {
		case AT_ARRAY:
			return brDataCopyArray(v, src, dst);
			break;

		case AT_INT:
		case AT_DOUBLE:
		case AT_VECTOR:
		case AT_MATRIX:
			memcpy(&dst->variables[v->offset], &src->variables[v->offset], stSizeofAtomic(v->type->type));
			break;

		case AT_STRING:
			stLoadVariable(&src->variables[v->offset], v->type->type, &load, &ri);
			stSetVariable(&dst->variables[v->offset], v->type->type, NULL, &load, &ri);

			slFree(BRSTRING(&load));

			break;

		case AT_LIST:
			stLoadVariable(&src->variables[v->offset], v->type->type, &load, &ri);
			assign.type = AT_LIST;
			BRLIST(&assign) = brEvalListCopy(BRLIST(&load));
			stSetVariable(&dst->variables[v->offset], v->type->type, NULL, &assign, &ri);

			brEvalListUnretain(BRLIST(&load));
			brEvalListUnretain(BRLIST(&assign));

			break;

		default:	
			return -1;
	}

	return 0;
}

int brDataCopyArray(stVar *v, stInstance *src, stInstance *dst) {
	brEval load, assign;
	stRunInstance ri;
	int n;

	ri.instance = src;
	ri.type = src->type;

	switch(v->type->arrayType) {
		case AT_INT:
		case AT_DOUBLE:
		case AT_VECTOR:
		case AT_MATRIX:
			memcpy(&dst->variables[v->offset], &src->variables[v->offset], stSizeofAtomic(v->type->arrayType) * v->type->arrayCount);
			break;
		case AT_STRING:
			for(n=0;n<v->type->arrayCount;n++) {
				stLoadVariable(&src->variables[v->offset + n * stSizeofAtomic(v->type->arrayType)], v->type->type, &load, &ri);
				brEvalCopy(&load, &assign);
				stSetVariable(&dst->variables[v->offset + n * stSizeofAtomic(v->type->arrayType)], v->type->type, NULL, &assign, &ri);
			}
			break;
		case AT_LIST:
			for(n=0;n<v->type->arrayCount;n++) {

			}
			break;

		default:
			break;
	}

	return 0;
}
