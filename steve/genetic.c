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
	+ genetic.c
	= functions (well, right now only one) which deal with "genetic crossover"
	= of objects variables.  kind of hacky.
*/

#include "steve.h"

/*!
	\brief A simple one-point crossover between two instances, storing the output in a third instance.

	Note that the crossover happens only to the "base" class of the 
	instance, not to it's ancestors.  The ancestor crossovers would
	thus be called explicitly if desired.
*/

int stObjectSimpleCrossover(stInstance *a, stInstance *b, stInstance *child) {
	int crossoverCount = 0, n;
	int varCount = 0;
	slList *list;
	stVar *var;
	stInstance *source;

	if(a->type != b->type || b->type != child->type) {
		slMessage(DEBUG_ALL, "Crossover instances must be of same class\n");
		return -1;
	}

	// we don't always want to use the first half of the first argument
	// passed.  we sometimes want to take the first half of the second 
	// argument... 

	if(random() % 2) {
		stInstance *temp = a;

		a = b;
		b = temp;
	}

	source = a;

	list = a->type->variableList;

	// this has become a little complicated now that we have "array" variables 
	// they only appear internally as a single variable, but may contain more
	// than one piece of information, and naturally we want to be able to
	// crossover in the middle.

	while(list) {
		var = list->data;

		if(var->type->type == AT_ARRAY) varCount += var->type->arrayCount;
		else varCount++;

		list = list->next;
	}

	list = a->type->variableList;

	crossoverCount = random() % (varCount + 1);

	source = a;

	// while(list) {
	for(n=0;n<varCount;n++) {
		brEval value;

		var = list->data;

		if(n >= crossoverCount) source = b;

		if(var->type->type == AT_ARRAY) {
			int index;

			for(index=0;index<var->type->arrayCount;index++) {
				int offset = var->offset + (n * stSizeofAtomic(var->type->arrayType));

				stLoadVariable(&source->variables[offset], var->type->arrayType, &value, NULL);

				// if(var->type->type == AT_LIST) BRLIST(&value) = brEvalListCopy(&value);

				stSetVariable(&child->variables[offset], var->type->arrayType, NULL, &value, NULL);
			}

			n += (var->type->arrayCount - 1);
		} else {
			stLoadVariable(&source->variables[var->offset], var->type->type, &value, NULL);

			// if(var->type->type == AT_LIST) BRLIST(&value) = brEvalListCopy(&value);

			stSetVariable(&child->variables[var->offset], var->type->type, NULL, &value, NULL);
		}

		list = list->next;
	}

	return 0;
}


