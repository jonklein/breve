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

#ifndef _PLUGINS_H
#define _PLUGINS_H
#ifdef MINGW

#include <windows.h>
#define dlopen(P,G) (void*)LoadLibrary(P)
#define dlsym(D,F) (void*)GetProcAddress((HMODULE)D, F)
#define dlclose(D) FreeLibrary((HMODULE)D)

const char *dlerror(void);

#else

#include <dlfcn.h>

#endif /* MINGW */



/*!
	\brief Holds data about a loaded plugin.
*/
	
struct brDlPlugin {
	char *name;
	void *handle;
};

int brEngineAddDlPlugin(char *, char *, brEngine *);
void brEngineRemoveDlPlugins(brEngine *);
void *brDlLoadPlugin(char *, char *, brNamespace *);

DLLEXPORT char *brPluginFindFile(char *, void *);

#endif
