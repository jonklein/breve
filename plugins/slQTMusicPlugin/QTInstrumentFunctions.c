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

int slRPlayNote(brEval args[], brEval *result, void *i);
int slRStartNote(brEval args[], brEval *result, void *i);
int slRStopNote(brEval args[], brEval *result, void *i);
int slRPlayChord(brEval args[], brEval *result, void *i);
int slRNewInstrument(brEval args[], brEval *result, void *i);
int slRFreeInstrument(brEval args[], brEval *result, void *i);

void slInitQTInstrumentFuncs(void *n) {
    brNewBreveCall(n, "newInstrument", slRNewInstrument, AT_POINTER, AT_INT, 0);
    brNewBreveCall(n, "freeInstrument", slRFreeInstrument, AT_NULL, AT_POINTER, 0);
    brNewBreveCall(n, "playNote", slRPlayNote, AT_NULL, AT_POINTER, AT_INT, AT_INT, AT_DOUBLE, 0);
    brNewBreveCall(n, "startNote", slRStartNote, AT_NULL, AT_POINTER, AT_INT, AT_INT, 0);
    brNewBreveCall(n, "stopNote", slRStopNote, AT_NULL, AT_POINTER, AT_INT, 0);
    brNewBreveCall(n, "playChord", slRPlayChord, AT_NULL, AT_POINTER, AT_LIST, AT_INT, AT_DOUBLE, 0);
}

slQTInstrumentInfo *slQTNewInstrumentInfo(int instrument) {
    ComponentResult componentError;
    slQTInstrumentInfo *i;
    NoteRequest request;

    /* initialize qt music */

    i = malloc(sizeof(slQTInstrumentInfo));

    i->allocator = OpenDefaultComponent('nota', 0);

    if (!i->allocator) {
        if (i->channel) NADisposeNoteChannel(i->allocator, i->channel);
        if (i->allocator) CloseComponent(i->allocator);
        return NULL;
    }

    request.info.flags = 0;
    request.info.midiChannelAssignment = 0;
    request.info.polyphony = 3; // simultaneous tones
    request.info.typicalPolyphony = 0x00020000;
    componentError = NAStuffToneDescription(i->allocator, instrument, &request.tone);
    componentError = NANewNoteChannel(i->allocator, &request, &i->channel);

    if(componentError || !i->channel) {
        if (i->channel) NADisposeNoteChannel(i->allocator, i->channel);
        if (i->allocator) CloseComponent(i->allocator);

        return NULL;
    }

    return i;
}

int slRFreeInstrument(brEval args[], brEval *result, void *i) {
    slQTInstrumentInfo *info = BRPOINTER(&args[0]);
    if (info->channel) NADisposeNoteChannel(info->allocator, info->channel);
    if (info->allocator) CloseComponent(info->allocator);

    return EC_OK;
}

int slRNewInstrument(brEval args[], brEval *result, void *i) {
    BRPOINTER(result) = slQTNewInstrumentInfo(BRINT(&args[0]));

    return EC_OK;
}

int slRPlayNote(brEval args[], brEval *result, void *i) {
    slQTInstrumentInfo *info = BRPOINTER(&args[0]);

    if(BRDOUBLE(&args[3]) <= 0.0) return EC_OK;

    if(info->allocator && info->channel) {
        NAPlayNote(info->allocator, info->channel, BRINT(&args[1]), BRINT(&args[2]));
        usleep(BRDOUBLE(&args[3]) * 1000000);
        NAPlayNote(info->allocator, info->channel, BRINT(&args[1]), 0);
    }

    return EC_OK;
}

int slRPlayChord(brEval args[], brEval *result, void *i) {
    slQTInstrumentInfo *info = BRPOINTER(&args[0]);
    brEvalList *list, *head = BRLIST(&args[1])->start;

    if(BRDOUBLE(&args[3]) <= 0.0) return EC_OK;

    list = head;

    /* push down */

    while(list) {
        NAPlayNote(info->allocator, info->channel, BRINT(&list->eval), BRINT(&args[2]));
        list = list->next;
    }

    usleep(BRDOUBLE(&args[3]) * 100000);
    list = head;

    /* release */

    while(list) {
        NAPlayNote(info->allocator, info->channel, BRINT(&list->eval), 0);
        list = list->next;
    }

    return EC_OK;
}

int slRStartNote(brEval args[], brEval *result, void *i) {
    slQTInstrumentInfo *info = BRPOINTER(&args[0]);

    if(info->allocator && info->channel) {
        NAPlayNote(info->allocator, info->channel, BRINT(&args[1]), BRINT(&args[2]));
    }

    return EC_OK;
}

int slRStopNote(brEval args[], brEval *result, void *i) {
    slQTInstrumentInfo *info = BRPOINTER(&args[0]);

    if(info->allocator && info->channel) {
        NAPlayNote(info->allocator, info->channel, BRINT(&args[1]), 0);
    }

    return EC_OK;
}
