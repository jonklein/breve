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

// this code does not work.

#include "kernel.h"

int brEngineNextMethodCall(brEngine *e, brInstance **i, brMethod **m) {
    pthread_mutex_lock(&e->scheduleLock);

	*i = NULL;
	*m = NULL;

	pthread_mutex_unlock(&e->scheduleLock);

	return NULL;
}

void *brIterationThread(void *data) {
	stThreadData *threadData = (stThreadData*)data;
	brEngine *e = threadData->engine;

	while(1) {
		brInstance *i;
		brMethod *m;
		int rcode;
		brEval result;

		if(i && m) brEngineNextMethodCall(e, &i, &m);

		if(i && m) rcode = brMethodCall(i, m, NULL, &result);
    }

	return NULL;
}
