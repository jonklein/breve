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
    + namespace.c
    = a not very exciting and not very commented file.  it's basically
    = a hash table.  i'm sure i had a good reason for not using the 
    = slUtilHash table, but i have no idea what that reason was.
	=
	= The namespace can be used to hash a string to specific data.  It's
	= currently used both in storing steve symbols (variables, functions,
	= objects, etc) and also separately in storing "defines" inside the 
	= engine.
*/

#include "kernel.h"

brNamespace *brNamespaceNew(int size) {
	brNamespace *n;
	int c;
	
	n = slMalloc(sizeof(brNamespace));
	
	n->buckets = slMalloc(sizeof(slList*) * size);
	n->tableSize = size;
	
	for(c=0;c<size;c++) n->buckets[c] = NULL;
	
	return n;
}

void brNamespaceFree(brNamespace *ns) {
	brNamespaceFreeWithFunction(ns, NULL);
}

void brNamespaceFreeWithFunction(brNamespace *ns, void (*symFree)(void *s)) {
	slList *start, *symbols;
	int c;

	start = symbols = brNamespaceSymbolList(ns);

	while(symbols) {
		brNamespaceSymbolFreeWithFunction(symbols->data, symFree);
		symbols = symbols->next;
	}

	slListFree(start);

	for(c=0;c<ns->tableSize;c++) slListFree(ns->buckets[c]);

	slFree(ns->buckets);

	slFree(ns);
}

/* gives all the symbols in a brNamespace as a list of brNamespaceSymbol entries */
/* so that users can free entire symbol tables */

slList *brNamespaceSymbolList(brNamespace *space) {
	slList *theList = NULL, *bucketList;
	int n;
 

	for(n=0;n<space->tableSize;n++) {
		bucketList = space->buckets[n];

		while(bucketList) {
			theList = slListPrepend(theList, bucketList->data);
			bucketList = bucketList->next;
		}
	}

	return theList;
}

brNamespaceSymbol *brNamespaceStore(brNamespace *space, char *name, int type, void *data) {
	brNamespaceSymbol *s;
	int hash;
	slList *l;
	
	hash = brNamespaceHashSymbol(name) % space->tableSize;
	
	/* check to see if this name is already in the table */
	/* traverse down the slList to the end, or til we hit  */
	/* an entry with this name.  if we stop traversing   */
	/* before the end of the slList, return NULL		   */
	
	l = space->buckets[hash];
	
	while(l && strcmp(((brNamespaceSymbol*)l->data)->name, name)) l = l->next;
	
	if(l) return NULL;
	
	s = brNamespaceSymbolNew(name, type, data);

	space->buckets[hash] = slListPrepend(space->buckets[hash], s);

	return s;
}

brNamespaceSymbol *brNamespaceLookup(brNamespace *space, char *name) {
	slList *l;
	int hash;
	
	hash = brNamespaceHashSymbol(name) % space->tableSize;

	l = space->buckets[hash];
	
	while(l && strcmp(((brNamespaceSymbol*)l->data)->name, name)) l = l->next;	

	if(!l) return NULL;
    
	return l->data;
}

brNamespaceSymbol *brNamespaceSymbolNew(char *name, int type, void *data) {
	brNamespaceSymbol *s;
	
	s = slMalloc(sizeof(brNamespaceSymbol));
	
	s->name = slStrdup(name);
	s->type = type;
	s->data = data;
	
	return s;
}

void brNamespaceSymbolFreeWithFunction(brNamespaceSymbol *s, void (*symFree)(void *s)) {
	if(symFree && s->data) symFree(s->data);
	slFree(s->name);
	slFree(s);
}

/* i know this isn't a 'good' hash function.  */

/* but it's *A* hash function, which was better than the last */
/* one, which just always returned zero */

int brNamespaceHashSymbol(char *string) {
	char c;

	int r = 0;

	while((c = *string++)) r += c;

	return r;
}
