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

/*!
	\brief Creates a new brData struct from a pointer and a data length.
*/

brData *brDataNew(void *info, int length) {
	brData *d;

	d = new brData;

	d->length = length;
	d->retainCount = 0;
	d->data = new unsigned char[length];
	bcopy(info, d->data, d->length);

	return d;
}

/*!
	\brief Frees a brData struct.
*/

void brDataFree(brData *d) {
	delete[] d->data;
	delete d;
}

/*!
    \brief Increments the retain count of a brData struct.
*/

void brDataRetain(brData *d) {
	if(!d) return;
	d->retainCount++;
}

/*!
	\brief Decrements the retain count of a brData struct.

	Frees the struct if the retain count hits 0.
*/

void brDataUnretain(brData *d) {
	if(!d) return;
	d->retainCount--;
}

/*!
	\brief Attempts to collect a brData struct.

	If the retain count is less than 1, the data is freed.
*/

void brDataCollect(brData *d) {
	if(d->retainCount < 1) brDataFree(d);
}

/*!
	\brief Encodes a brData struct into a hex string.

	Used for archiving and XML networking.
*/

char *brDataHexEncode(brData *d) {
    char *string;
    int n;

    if(!d || d->length < 1) return slStrdup("");

    string = slMalloc((d->length * 2) + 1);

    for(n=0;n<d->length;n++)
        sprintf(&string[n*2], "%02x", ((unsigned char*)d->data)[n]);

    string[(d->length * 2)] = 0;

    return string;
}

/*!
	\brief Decodes a brData struct from a hex string.

	Used for archiving and XML networking.
*/

brData *brDataHexDecode(char *string) {
    brData *d;
    int length;
    int n;
    int l;

    if(!string) return NULL;

    length = strlen(string);
    if((length % 2) || length < 0) {
        slMessage(DEBUG_ALL, "warning: error decoding hex data string (length = %d)\n", length);
        return NULL;
    }

    d = slMalloc(sizeof(brData));

    d->length = length / 2;
    d->data = slMalloc(d->length);
	d->retainCount = 0;

    for(n=0;n<d->length;n++) {
        sscanf(&string[n * 2], "%2x", &l);
        ((char*)d->data)[n] = l & 0xff;
    }

    return d;
}
