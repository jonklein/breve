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

brNamespace *brNamespaceNew() {
	return new brNamespace;
}

void brNamespaceFree(brNamespace *ns) {
	brNamespaceFreeWithFunction(ns, NULL);
}

void brNamespaceFreeWithFunction(brNamespace *ns, void (*symFree)(void *s)) {
	std::map< std::string, brNamespaceSymbol*>::iterator mi;

	for(mi = ns->map.begin(); mi != ns->map.end(); mi++) {
		if(mi->second) brNamespaceSymbolFreeWithFunction(mi->second, symFree);
	}

	ns->map.clear();

	delete ns;
}

/*!
	\brief Returns a slList of all of the data in the namespace.
*/

slList *brNamespaceSymbolList(brNamespace *space) {
	slList *list = NULL;
	std::map< std::string, brNamespaceSymbol*>::iterator mi;

	for(mi = space->map.begin(); mi != space->map.end(); mi++) 
		if(mi->second) list = slListPrepend(list, mi->second);

	return list;
}

brNamespaceSymbol *brNamespaceStore(brNamespace *space, char *name, int type, void *data) {
	std::string nameS = name;

	if(space->map[ nameS]) return NULL;

	space->map[ nameS] = brNamespaceSymbolNew(type, data);

	return space->map[ nameS];
}

brNamespaceSymbol *brNamespaceLookup(brNamespace *space, char *name) {
	std::string nameS = name;
	return space->map[ nameS];
}

brNamespaceSymbol *brNamespaceSymbolNew(int type, void *data) {
	brNamespaceSymbol *s;
	
	s = new brNamespaceSymbol;
	
	s->type = type;
	s->data = data;
	
	return s;
}

void brNamespaceSymbolFreeWithFunction(brNamespaceSymbol *s, void (*symFree)(void *s)) {
	if(symFree && s->data) symFree(s->data);
	delete s;
}
