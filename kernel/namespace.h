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

#ifndef _NAMESPACE_H
#define _NAMESPACE_H
#include "util.h"

enum symbolTypes {
    ST_METHODS = 1,
    ST_OBJECT,
    ST_OBJECT_ALIAS,
    ST_CLASS,
    ST_FUNC,
    ST_VAR
};

/*!
	\brief A hashtable for holding symbols of different types.

	Used by the steve language, and to hold internal method, object 
	and function names.
*/

#ifdef __cplusplus
#include <map>
#include <string>

struct brNamespace {
    int tableSize;
    slList **buckets;

	std::map<std::string,brNamespaceSymbol*> map;
};
#endif

/*!
	\brief A symbol in a brNamespace.
*/

struct brNamespaceSymbol {
    char *name;
    int type;
    void *data;
};

#ifdef __cplusplus
extern "C" {
#endif


brNamespace *brNamespaceNew(int size);
void brNamespaceFree(brNamespace *ns);
void brNamespaceFreeWithFunction(brNamespace *ns, void (*symFree)(void *s));

brNamespaceSymbol *brNamespaceStore(brNamespace *space, char *name, int type, void *data);
brNamespaceSymbol *brNamespaceLookup(brNamespace *space, char *name);

brNamespaceSymbol *brNamespaceSymbolNew(char *name, int type, void *data);

void brNamespaceSymbolFree(brNamespaceSymbol *s);
void brNamespaceSymbolFreeWithFunction(brNamespaceSymbol *s, void (*symFree)(void *s));

int brNamespaceHashSymbol(char *string);

slList *brNamespaceSymbolList(brNamespace *space);

#ifdef __cplusplus
}
#endif

#endif
