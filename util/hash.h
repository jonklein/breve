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
	\brief A simple reusable hash-table structure.
*/

struct slHash {
    unsigned int size;
    slList **buckets;

    unsigned int (*hashFunc)(void *p, unsigned int n);
    unsigned int (*compFunc)(void *a, void *b);
};

/*!
	\brief An entry in an slHash.
*/

struct slHashEntry {
    void *data;
    void *key;
};

#define slDehashData(h, k) slDehashDataAndKey((h), (k), NULL)

slHash *slNewHash(unsigned int size, unsigned int (*hf)(void *p, unsigned int n), unsigned int (*cf)(void *a, void *b));
void slFreeHash(slHash *h);

void *slHashData(slHash *h, void *data, void *key);
void *slDehashDataAndKey(slHash *h, void *key, void **outkey);

slList *slHashKeys(slHash *h);
slList *slHashValues(slHash *h);

unsigned int slHashPointer(void *p, unsigned int n);
unsigned int slCompPointer(void *a, void *b);

unsigned int slHashString(void *p, unsigned int n);
unsigned int slCompString(void *a, void *b);
