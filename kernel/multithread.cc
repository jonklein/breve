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

void *stIterationThread(void *data) {
	stThreadData *threadData = data;
	brEngine *e = threadData->engine;

	while(1) {
		int r;

		r = pthread_cond_wait(&e->condition, &e->conditionLock);

		printf("condition waited (%d) for thread %d at time %f\n", r, threadData->number, slWorldGetAge(e->world));

		printf("result from unlocking condition %d for thread %d\n", pthread_mutex_unlock(&e->conditionLock), threadData->number);

		while(e->lastScheduled < e->instances.size()) {
			printf("locked mutex %p %d, thread %d\n", &e->scheduleLock, pthread_mutex_lock(&e->scheduleLock), threadData->number);
			printf("locked mutex %p %d, thread %d\n", &e->lock, pthread_mutex_lock(&e->lock), threadData->number);
		
			if(e->lastScheduled < e->instances.size()) {
				int n;

				e->lastScheduled++;
				n = e->lastScheduled;

				printf("got schedule lock for %d, thread %d at time %f\n", n, threadData->number, slWorldGetAge(e->world));

				if(e->instances[n]) {
					// rcode = brMethodCall(e->instances[n], e->instances[n]->iterate, NULL, &result);
				}

			}

        	pthread_mutex_unlock(&e->scheduleLock);

		}
    }

	return NULL;
}
