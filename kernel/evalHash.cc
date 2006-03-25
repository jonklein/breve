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

/*
bool brEval::operator<(brEval *b) {
	if(type < b->type) return true;

	if(type > b->type) return false;

	switch(type) {
		case AT_INT:
			return BRINT(this) < BRINT(b);
			break;
		case AT_DOUBLE:
			return BRDOUBLE(this) < BRDOUBLE(b);
			break;

		case AT_DATA:
		case AT_POINTER:
		case AT_INSTANCE:
		case AT_HASH:
		case AT_LIST:
			return BRPOINTER(this) < BRPOINTER(b);
			break;
	}
}
*/

/*!
	\brief Creates a breve eval hash table.

	The breve eval hash structure is typically used only in the steve language.
*/

brEvalHash *brEvalHashNew() {
	return new brEvalHash;
}

brEvalHash::brEvalHash() {
	table = slNewHash(1024, brEvalHashFunction, brEvalHashCompareFunction);
	retainCount = 0;
}

/*!
	\brief Frees an eval hash.

	Does not garbage collect.  The steve version of this function, 
	\ref brEvalHashFreeGC does garbage collection, but is part of 
	the steve library, not the breve library.
*/

void brEvalHashFree(brEvalHash *h) {
	slList *all, *start;

	start = all = slHashValues(h->table);

	while(all) {
		delete (brEval*)all->data;
		all = all->next;
	}

	slListFree(start);

	start = all = slHashKeys(h->table);

	while(all) {
		delete (brEval*)all->data;
		all = all->next;
	}

	slListFree(start);

	slFreeHash(h->table);

	delete h;
}

/**
 * Stores an entry in a breve hash table.
 */

void brEvalHashStore(brEvalHash *h, brEval *key, brEval *value) {
	brEval *v, *k;

	v = (brEval*)slDehashDataAndKey( h->table, key, (void**)&k );

	if(!v) {
		v = new brEval;
		k = new brEval;
		brEvalCopy( key, k );
	} 

	brEvalCopy(value, v);

	slHashData( h->table, k, v );
}

/*!
	\brief Returns a brEvalList of all of the keys in the hash table.
*/

brEvalListHead *brEvalHashKeys(brEvalHash *h) {
	brEvalListHead *el;
	slList *l;

	l = slHashKeys(h->table);
	el = brEvalListNew();

	while(l) {
		brEvalListInsert(el, 0, (brEval*)l->data);
		l = l->next;
	}

	return el;
}

/*!
	\brief Returns a brEvalList of all of the values in the hash table.
*/

brEvalListHead *brEvalHashValues(brEvalHash *h) {
	brEvalListHead *el;
	slList *l;

	l = slHashKeys(h->table);
	el = brEvalListNew();

	while(l) {
		brEvalListInsert(el, 0, (brEval*)l->data);
		l = l->next;
	}

	return el;
}

/*!
	\brief Returns a brEvalList of all of the values in the hash table.
*/

int brEvalHashLookup(brEvalHash *h, brEval *key, brEval *value) {
	brEval *v;

	v = (brEval*)slDehashData(h->table, key);

	if(!v) return -1;

	brEvalCopy(v, value);

	return 0;
}

unsigned int brEvalHashFunction(void *e, unsigned int hsize) {
	brEval *ee = (brEval*)e;
	int i;
	int dataSize = 0;
	int total = 0;
	unsigned char *p;
	void *vp;
	double d;

	switch( ee->type() ) {
		case AT_INSTANCE:
		case AT_POINTER:
		case AT_DATA:
		case AT_HASH:
		case AT_LIST:
			vp = ee->getPointer();
			p = (unsigned char*)&vp;
			dataSize = sizeof(void*);
			break;
		case AT_INT:
			i = ee->getInt();
			p = (unsigned char*)&i;
			dataSize = sizeof(int);
			break;
		case AT_DOUBLE:
			d = ee->getDouble();
			p = (unsigned char*)&d;
			dataSize = sizeof(double);
			break;
		case AT_STRING:
			p = (unsigned char*)BRSTRING(ee);
			dataSize = strlen(BRSTRING(ee));
			break;
		case AT_VECTOR:
			dataSize = sizeof(slVector);
			p = (unsigned char*)&BRVECTOR(ee);
			break;
		case AT_MATRIX:
			dataSize = sizeof(double) * 9;
			p = (unsigned char*)&BRMATRIX(ee);
			break;
		default:
			dataSize = 0;
			p = 0x0;
			break;

	}

	if(p) for(i=0;i<dataSize;i++) total += p[i];

	return total % hsize;
}

unsigned int brEvalHashCompareFunction(void *a, void *b) {
	brEval *ae = (brEval*)a, *be = (brEval*)b;

	if( ae->type() != be->type() ) return 1;

	switch( ae->type() ) {
		case AT_LIST:
		case AT_DATA:
		case AT_POINTER:
		case AT_INSTANCE:
		case AT_HASH:
			return !(BRPOINTER(ae) == BRPOINTER(be));
			break;
		case AT_INT:
			return !(BRINT(ae) == BRINT(be));
			break;
		case AT_DOUBLE:
			return !(BRDOUBLE(ae) == BRDOUBLE(be));
			break;
		case AT_VECTOR:
			return slVectorCompare(&BRVECTOR(ae), &BRVECTOR(be));
			break;
		case AT_MATRIX:
			return slMatrixCompare(BRMATRIX(ae), BRMATRIX(be));
			break;
		case AT_STRING:
			return strcmp(BRSTRING(ae), BRSTRING(be));
			break;
		default:
			slMessage( 0, "unknown expression type in brEvalCompareFunction: %d\n", ae->type() );
			return 0;
			break;
	}
}
