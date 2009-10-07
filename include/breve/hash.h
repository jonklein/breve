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

struct slList;

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

#define slDehashData(h, k) slDehashDataAndKey( (h), (k), NULL, 0 )

slHash *slNewHash(unsigned int, unsigned int (*)(void *, unsigned int), unsigned int (*)(void *, void *));
void slFreeHash(slHash *);

void *slHashData(slHash *, void *, void *);
void *slDehashDataAndKey(slHash *, void *, void **, int inRemove );

slList *slHashKeys(slHash *);
slList *slHashValues(slHash *);

unsigned int slHashPointer(void *, unsigned int);
unsigned int slCompPointer(void *, void *);

unsigned int slHashString(void *, unsigned int);
unsigned int slCompString(void *, void *);
