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

#include <unistd.h>
#include <QuickTime/QuickTime.h>
#include "slBrevePluginAPI.h"

struct slQTInstrumentInfo {
	NoteAllocator allocator;
	NoteChannel channel;
	NoteRequest request;
};

typedef struct slQTInstrumentInfo slQTInstrumentInfo;

slQTInstrumentInfo *slQTNewInstrumentInfo(int instrument) {
	ComponentResult componentError;
	slQTInstrumentInfo *i;
	NoteRequest request;

	/* initialize qt music */

	i = new slQTInstrumentInfo;

	i->allocator = OpenDefaultComponent('nota', 0);

	if (!i->allocator) {
		if (i->channel) NADisposeNoteChannel(i->allocator, i->channel);
		if (i->allocator) CloseComponent(i->allocator);
		return NULL;
	}

	BigEndianShort wtf;
	BigEndianFixed againwtf;

	request.info.flags = 0;
	request.info.midiChannelAssignment = 0;

	wtf.bigEndianValue = 3;
	request.info.polyphony = wtf; // wtf?

	againwtf.bigEndianValue = 0x00020000;
	request.info.typicalPolyphony = againwtf; // again, wtf?

	componentError = NAStuffToneDescription(i->allocator, instrument, &request.tone);
	componentError = NANewNoteChannel(i->allocator, &request, &i->channel);

	if(componentError || !i->channel) {
		if (i->channel) NADisposeNoteChannel(i->allocator, i->channel);
		if (i->allocator) CloseComponent(i->allocator);

		return NULL;
	}

	return i;
}

int brQTInstrumentFree(brEval args[], brEval *result, void *i) {
	slQTInstrumentInfo *info = (slQTInstrumentInfo*)BRPOINTER(&args[0]);

	printf("freeing %p and %p\n", info->channel, info->allocator);
	if (info->channel) NADisposeNoteChannel(info->allocator, info->channel);
	if (info->allocator) CloseComponent(info->allocator);

	return EC_OK;
}

int brQTInstrumentNew(brEval args[], brEval *result, void *i) {
	BRPOINTER(result) = slQTNewInstrumentInfo(BRINT(&args[0]));

	return EC_OK;
}

int brQTInstrumentPlayNote(brEval args[], brEval *result, void *i) {
	slQTInstrumentInfo *info = (slQTInstrumentInfo*)BRPOINTER(&args[0]);

	if(BRDOUBLE(&args[3]) <= 0.0) return EC_OK;

	if(info->allocator && info->channel) {
		NAPlayNote(info->allocator, info->channel, BRINT(&args[1]), BRINT(&args[2]));
		usleep( ( int )( BRDOUBLE(&args[3]) * 1000000 ) );
		NAPlayNote(info->allocator, info->channel, BRINT(&args[1]), 0);
	}

	return EC_OK;
}

int brQTInstrumentSetController(brEval args[], brEval *result, void *i) {
	slQTInstrumentInfo *info = (slQTInstrumentInfo*)BRPOINTER(&args[0]);
	long number = BRINT(&args[1]);
	long value = BRINT(&args[2]);

	printf("setting %d for %d\n", value, number);
	
	NASetController(info->allocator, info->channel, number, value);

	return EC_OK;
}

int brQTInstrumentPlayChord(brEval args[], brEval *result, void *i) {
	slQTInstrumentInfo *info = (slQTInstrumentInfo*)BRPOINTER(&args[0]);
	brEvalList *list, *head = BRLIST(&args[1])->start;

	if(BRDOUBLE(&args[3]) <= 0.0) return EC_OK;

	list = head;

	/* push down */

	while(list) {
		NAPlayNote(info->allocator, info->channel, BRINT(&list->eval), BRINT(&args[2]));
		list = list->next;
	}

	usleep( ( int )( BRDOUBLE(&args[3]) * 100000 ) );
	list = head;

	/* release */

	while(list) {
		NAPlayNote(info->allocator, info->channel, BRINT(&list->eval), 0);
		list = list->next;
	}

	return EC_OK;
}

int brQTInstrumentStartNote(brEval args[], brEval *result, void *i) {
	slQTInstrumentInfo *info = (slQTInstrumentInfo*)BRPOINTER(&args[0]);

	if(info->allocator && info->channel) {
		NAPlayNote(info->allocator, info->channel, BRINT(&args[1]), BRINT(&args[2]));
	}

	return EC_OK;
}

int brQTInstrumentStopNote(brEval args[], brEval *result, void *i) {
	slQTInstrumentInfo *info = (slQTInstrumentInfo*)BRPOINTER(&args[0]);

	if(info->allocator && info->channel) {
		NAPlayNote(info->allocator, info->channel, BRINT(&args[1]), 0);
	}

	return EC_OK;
}

void slInitQTInstrumentFuncs(void *n) {
	brNewBreveCall(n, "instrumentNew", brQTInstrumentNew, AT_POINTER, AT_INT, 0);
	brNewBreveCall(n, "instrumentFree", brQTInstrumentFree, AT_NULL, AT_POINTER, 0);
	brNewBreveCall(n, "instrumentPlayNote", brQTInstrumentPlayNote, AT_NULL, AT_POINTER, AT_INT, AT_INT, AT_DOUBLE, 0);
	brNewBreveCall(n, "instrumentStartNote", brQTInstrumentStartNote, AT_NULL, AT_POINTER, AT_INT, AT_INT, 0);
	brNewBreveCall(n, "instrumentStopNote", brQTInstrumentStopNote, AT_NULL, AT_POINTER, AT_INT, 0);
	brNewBreveCall(n, "instrumentPlayChord", brQTInstrumentPlayChord, AT_NULL, AT_POINTER, AT_LIST, AT_INT, AT_DOUBLE, 0);
	brNewBreveCall(n, "instrumentSetController", brQTInstrumentSetController, AT_NULL, AT_POINTER, AT_INT, AT_INT, 0);
}

