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
    = hash.c -- a general purpose hash table utility.  used mainly by the
    = spiderland memory subsystem to store the states of malloc/freed
    = pointers.  no surprises here in the implementation.
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "slutil.h"

/*
    = slNewHash generates a new hash table, given a size, a function
    = which will be used as the hash function and a "compare" function
    = used to compare two entries when doing a lookup.

    = the advantage of having the function pointers inside the
    = hash structure is that we can hash structure to hash different
    = kinds of data--for example, we can hash normal null-terminated
    = c strings or 4-byte pointers.

    = upon success, this function returns the new hash table.
*/

/*!
	\brief Create a new empty hash table of the size specified.

	Uses the function pointer argument hf() for the hash function,
	and cf() for the compare function.
*/

slHash *slNewHash( unsigned int size, unsigned int( *hf )( void *p, unsigned int n ), unsigned int( *cf )( void *a, void *b ) ) {
	slHash *h;
	unsigned int n;

	h = new slHash;

	h->buckets = new slList*[size];
	h->size = size;

	for ( n = 0;n < h->size;n++ ) h->buckets[n] = 0;

	h->hashFunc = hf;

	h->compFunc = cf;

	return h;
}

/*!
	\brief Frees an slHash hash-table.

	Does not free the data inside the hash-table -- this must be done manually.
*/

void slFreeHash( slHash *h ) {
	unsigned int n;

	for ( n = 0;n < h->size;n++ ) {
		slList *buckets;

		buckets = h->buckets[n];

		while ( buckets ) {
			delete( slHashEntry* )buckets->data;
			buckets = buckets->next;
		}

		slListFree( h->buckets[n] );
	}

	delete[] h->buckets;

	delete h;
}

/*!
	\brief Add data to a hash table.

    Takes a hash table, a key and data and preforms a hash.  The data is
	hashed using the hashFunc inside the hash table.

    If an error occurs, or if a record with this key already exists
	(according to the compFunc inside the hash table), then NULL is
	returned.
*/

void *slHashData( slHash *h, void *key, void *data ) {
	slList *activeList, *origList;
	int number;

	slHashEntry *newEntry;

	if ( !h->hashFunc || !h->compFunc ) return NULL;

	/* go through the bucket to see if this entry already exists */

	number = h->hashFunc( key, h->size );

	origList = activeList = h->buckets[number];

	while ( activeList ) {
		if ( !h->compFunc( key, (( slHashEntry* )activeList->data )->key ) )
			return NULL;

		activeList = activeList->next;
	}

	newEntry = new slHashEntry;

	newEntry->data = data;
	newEntry->key = key;

	h->buckets[h->hashFunc( key, h->size )] = slListPrepend( origList, newEntry );

	return data;
}

/**
 * \brief Looks up an element in a hash table.
 *
 *   Takes a hash and a key, and returns the data associated with the key.
 * If the data cannot be found, NULL is returned.
 */

void *slDehashDataAndKey( slHash *h, void *key, void **outkey, int inRemove ) {
	slList *activeList;
	int number;

	if ( !h->hashFunc || !h->compFunc ) return NULL;

	number = h->hashFunc( key, h->size );

	activeList = h -> buckets[ number ];

	while ( activeList ) {
		if ( !h->compFunc( key, (( slHashEntry* )activeList->data )->key ) ) {
			slHashEntry *entry = (( slHashEntry* )activeList->data );
			void *result = entry -> data;

			if ( outkey ) 
				*outkey = entry->key;

			if( inRemove ) 
				h -> buckets[ number ] = slListRemoveData( h -> buckets[ number ], entry );

			return result;
		}

		activeList = activeList->next;
	}

	if ( outkey ) *outkey = NULL;

	return NULL;
}


/*!
	\brief Returns a list of pointers to all the values in a hash-table.
*/

slList *slHashValues( slHash *h ) {
	slList *all = NULL;
	unsigned int n;

	for ( n = 0;n < h->size;n++ ) {
		slList *buckets;

		buckets = h->buckets[n];

		while ( buckets ) {
			all = slListPrepend( all, (( slHashEntry* )buckets->data )->data );
			buckets = buckets->next;
		}
	}

	return all;
}

/*!
	\brief Returns a list of pointers to all the keys in a hash-table.
*/

slList *slHashKeys( slHash *h ) {
	slList *all = NULL;
	unsigned int n;

	for ( n = 0;n < h->size;n++ ) {
		slList *buckets;

		buckets = h->buckets[n];

		while ( buckets ) {
			all = slListPrepend( all, (( slHashEntry* )buckets->data )->key );
			buckets = buckets->next;
		}
	}

	return all;
}

/*!
	\brief A simple hash function for strings.
*/

unsigned int slHashString( void *d, unsigned int s ) {
	char *string = ( char* )d;
	char c;
	int r = 0;

	while (( c = *( string++ ) ) ) r += c;

	return r % s;
}

/*!
	\brief A compare function for strings.
*/

unsigned int slCompString( void *a, void *b ) {
	return strcmp(( char* )a, ( char* )b );
}

/*!
	\brief A simple hash function for pointers.
*/

unsigned int slHashPointer( void *p, unsigned int n ) {
	unsigned int total = 0;
	void *localP = p;
	unsigned char *start = ( unsigned char* ) & localP;
	unsigned int c;

	for ( c = 0;c < sizeof( void* );c++ ) total += start[c];

	return (( unsigned int )total % n );
}

/*!
	\brief A compare function for pointers.
*/

unsigned int slCompPointer( void *a, void *b ) {
	return ( unsigned int )a - ( unsigned int )b;
}
