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
#include "slutil.h"

enum symbolTypes {
    ST_METHODS = 1,
    ST_VAR
};

/*!
	\brief A hashtable for holding symbols of different types.

	Used by the steve language as a generic hash table.

	This used to be a homegrown hash-table in C, now it's a wrapper
	around an STL map.
*/

#ifdef __cplusplus
#include <map>
#include <string>

class brNamespace {
	public:
		std::map<std::string,brNamespaceSymbol*> map;
};
#endif

/*!
	\brief A symbol in a brNamespace.
*/

struct brNamespaceSymbol {
    int type;
    void *data;
};

#ifdef __cplusplus
extern "C" {
#endif

brNamespace *brNamespaceNew(void);
void brNamespaceFree(brNamespace *);
void brNamespaceFreeWithFunction(brNamespace *, void (*)(void *));

brNamespaceSymbol *brNamespaceStore(brNamespace *, char *, int, void *);
brNamespaceSymbol *brNamespaceLookup(brNamespace *, char *);

brNamespaceSymbol *brNamespaceSymbolNew(int, void *);

void brNamespaceSymbolFree(brNamespaceSymbol *);
void brNamespaceSymbolFreeWithFunction(brNamespaceSymbol *s, void (*)(void *s));

slList *brNamespaceSymbolList(brNamespace *);

#ifdef __cplusplus
}
#endif

#endif
