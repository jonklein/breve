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

/*!
	\brief Holds arbitrary data of a given length.
*/

struct brData {
	unsigned char *data;
	int length;
	int retainCount;
};

#ifdef __cplusplus
extern "C" {
#endif
brData *brDataNew(void *data, int length);
void brDataFree(brData *data);

void brDataRetain(brData *data);
void brDataUnretain(brData *data);
void brDataCollect(brData *data);

char *brDataHexEncode(brData *d);
brData *brDataHexDecode(char *string);
#ifdef __cplusplus
}
#endif
