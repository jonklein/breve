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

#ifndef MINGW
#include <dlfcn.h>
#else 
#define dlopen(P,G) (void*)LoadLibrary(P)
#define dlsym(D,F) (void*)GetProcAddress((HMODULE)D, F)
#define dlclose(D) FreeLibrary((HMODULE)D)

__inline const char *dlerror() {
	static char message[256];

	FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM|FORMAT_MESSAGE_IGNORE_INSERTS,
		NULL,
		GetLastError(),
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		message,
		sizeof(message),
		NULL);

	return message;
}
#endif /* MINGW */

#include <errno.h>

/*!
	\brief Opens a plugin and loads it into the engine.
*/

int brEngineAddDlPlugin(char *filename, char *func, brEngine *engine) {
	void *handle;
	char *fullpath;
	brDlPlugin *p;

	p = new brDlPlugin;

	// Linux exports symbols unmolested -- or so I am told.
	// Apparently this is working for OS X too.

	fullpath = brFindFile(engine, filename, NULL);

	if(!fullpath) {
		slMessage(DEBUG_ALL, "unable to locate file \"%s\"\n", filename);
		return -1;
	}

	handle = brDlLoadPlugin(fullpath, func, engine->internalMethods);

	slFree(fullpath);

	if(handle) {
		p->handle = handle;
		p->name = slStrdup(filename);
		engine->dlPlugins.push_back(p);
		return 0;	
	} else {
		return -1;
	}
}

/*!
	\brief Frees and unloads all loaded plugins.
*/

void brEngineRemoveDlPlugins(brEngine *engine) {
	brDlPlugin *p;
	std::vector<brDlPlugin*>::iterator di;

	for(di = engine->dlPlugins.begin(); di != engine->dlPlugins.end(); di++ ) {
		p = *di;

		dlclose(p->handle);
		slFree(p->name);
		delete p;
	}
}

/*!
	\brief Loads internal functions from a given plugin.
*/

void *brDlLoadPlugin(char *filename, char *func, brNamespace *n) {
	void (*f)(brNamespace *n);
	void *handle;

#ifdef WINDOWS
	// can't find RTLD_NOW on Windows
	handle = dlopen(filename, 0);
#else
	handle = dlopen(filename, RTLD_LAZY|RTLD_GLOBAL);
#endif

	if(!handle) {
		slMessage(DEBUG_ALL, "error loading plugin %s: %s\n", filename, dlerror());
		return NULL;
	}

	f = (void (*)(brNamespace *n))dlsym(handle, func);

	if(!f) {
		slMessage(DEBUG_ALL, "error loading function %s from plugin %s: %s\n", func, filename, dlerror());
	}

	if(!f) return NULL;

	// yay!

	f(n);

	return handle;
}

/*!
	\brief A plugin-accessable version of \ref brFindFile.

	Allows plugins to find files using the same file paths as the breve engine does.
*/

char *brPluginFindFile(char *name, void *i) {
	return brFindFile(((brInstance*)i)->engine, name, NULL);
}
