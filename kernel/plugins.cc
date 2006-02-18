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

#ifdef MINGW
const char *dlerror(void) {
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

/*!
	\brief Opens a plugin and loads it into the engine.
*/

int brEngineAddDlPlugin(char *filename, char *func, brEngine *e) {
	char *fullpath;
	void *handle;
	brDlPlugin *p;

	if (!(fullpath = brFindFile(e, filename, NULL))) {
		slMessage(DEBUG_ALL, "Cannot find plugin \"%s\"\n", filename);
		return -1;
	}
	handle = brDlLoadPlugin(fullpath, func, e->internalMethods);
	slFree(fullpath);

	if (!handle)
		return -1;

	p = new brDlPlugin;
	p->handle = handle;
	p->name = slStrdup(filename);
	e->dlPlugins.push_back(p);

	return 0;	
}

/*!
	\brief Frees and unloads all loaded plugins.
*/

void brEngineRemoveDlPlugins(brEngine *e) {
	brDlPlugin *p;
	std::vector<brDlPlugin*>::iterator di;

	for (di = e->dlPlugins.begin(); di != e->dlPlugins.end(); di++ ) {
		p = *di;

		dlclose(p->handle);
		slFree(p->name);
		delete p;
	}
}

/*!
	\brief Loads internal functions from a given plugin.
*/

void *brDlLoadPlugin(char *filename, char *symname, brNamespace *n) {
	void (*f)(brNamespace *);
	void *handle;

	if (!(handle = dlopen(filename, RTLD_LAZY | RTLD_GLOBAL))) {
		slMessage(DEBUG_ALL, "error loading plugin %s: %s\n",
		    filename, dlerror());
		return NULL;
	}
	if (!(f = (void (*)(brNamespace *))dlsym(handle, symname))) {
		slMessage(DEBUG_ALL, "error resolving %s in %s: %s\n",
		    symname, filename, dlerror());
		return NULL;
	}

	slMessage(DEBUG_INFO, "Calling %s() in %s\n", symname, filename);
	f(n);

	return handle;
}

/*!
	\brief A plugin-accessible version of \ref brFindFile.

	Allows plugins to find files using the same file paths as the
	breve engine does.
*/

char *brPluginFindFile(char *name, void *i) {
	return brFindFile(((brInstance *)i)->engine, name, NULL);
}
