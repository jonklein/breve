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

char *interfaceID;

/** \defgroup breveEngineAPI The breve engine API: using a breve simulation from another program or application frontend */
/*@{*/

void brEngineLock(brEngine *e) {
	(pthread_mutex_lock(&(e)->lock));
}

void brEngineUnlock(brEngine *e) {
	(pthread_mutex_unlock(&(e)->lock));
}

/*!
    \brief Creates a brEngine structure.

    Creates and initializes the bloated brEngine structure.  This is the 
    first step in starting a breve simulation.
*/

brEngine *brEngineNew(void) {
	brEngine *e;
	char *envpath, *dir;
	int n = 0;

#ifdef MINGW
	pthread_win32_process_attach_np();
#endif

	e = new brEngine;

	e->simulationWillStop = 0;

	e->camera = slCameraNew(400, 400);

#if defined(HAVE_LIBPORTAUDIO) && defined(HAVE_LIBSNDFILE)
	Pa_Initialize();
	e->soundMixer = brNewSoundMixer();
#endif

#ifdef HAVE_LIBOSMESA
	e->osBuffer = NULL;
	e->osContext = NULL;
#endif

	e->nThreads = 1;

	// under OSX we'll connect a file handle to the output queue to allow
	// file handle output to the message logs (useful to plugins).  Under
	// the linux version, we can't and don't really need to do this--they
	// can just use stderr.

#ifdef MACOSX
	e->logFile = funopen(e, NULL, brFileLogWrite, NULL, NULL);
#else
	e->logFile = stderr;
#endif 

	e->path = new char[MAXPATHLEN + 1];
	e->drawEveryFrame = 1;

	getcwd(e->path, MAXPATHLEN);

	e->world = slWorldNew();

	slWorldSetCollisionCallbacks(e->world, brCheckCollisionCallback, brCollisionCallback);

	gettimeofday(&e->startTime, NULL);

	e->realTime.tv_sec = 0;
	e->realTime.tv_usec = 0;

	if(pthread_mutex_init(&e->lock, NULL)) {
        slMessage(0, "warning: error creating lock for breve engine\n");
		if(e->nThreads > 1) {
        	slMessage(0, "cannot start multi-threaded simulation without lock\n");
			return NULL;
		}
	}

	if(pthread_mutex_init(&e->scheduleLock, NULL)) {
        slMessage(0, "warning: error creating lock for breve engine\n");
		if(e->nThreads > 1) {
        	slMessage(0, "cannot start multi-threaded simulation without lock\n");
			return NULL;
		}
	}

	if(pthread_mutex_init(&e->conditionLock, NULL)) {
        slMessage(0, "warning: error creating lock for breve engine\n");
		if(e->nThreads > 1) {
        	slMessage(0, "cannot start multi-threaded simulation without lock\n");
			return NULL;
		}
	}

	if(pthread_cond_init(&e->condition, NULL)) {
		slMessage(0, "warning: error creating pthread condition variable\n");
	}

	// namespaces holding object names and method names 

	e->internalMethods = brNamespaceNew();

	// set up the initial search paths 

	brEngineSetIOPath(e, ".");

	// load all of the internal breve functions

	brLoadInternalFunctions(e);

	// add the default class path, and check the BREVE_CLASS_PATH 
	// environment variable to see if it adds any more 

	brAddSearchPath(e, "lib/classes");

	if((envpath = getenv("BREVE_CLASS_PATH"))) {
		while((dir = slSplit(envpath, ":", n++))) {
			brAddSearchPath(e, dir);
			slMessage(DEBUG_INFO, "adding \"%s\" to class path\n", dir);
			slFree(dir);
		}
	}

	if((envpath = getenv("HOME"))) brAddSearchPath(e, envpath);

	memset(e->keys, 0, sizeof(e->keys));

	for(n=1;n<e->nThreads;n++) {
		stThreadData *data;

		data = new stThreadData;
		data->engine = e;
		data->number = n;
		pthread_create(&data->thread, NULL, brIterationThread, data);
	}

	return e;
}

brInternalFunction *brEngineInternalFunctionLookup(brEngine *e, char *name) {
	brNamespaceSymbol *s = brNamespaceLookup(e->internalMethods, name);

	if(!s) return NULL;

	return (brInternalFunction*)s->data;
}


/*!
	\brief Frees a breve engine.
	
	Frees the engine and everything inside of it--all objects, all instances, everything.
*/

void brEngineFree(brEngine *e) {
	std::vector<brInstance*>::iterator bi;
	std::vector<void*>::iterator wi;

#if defined(HAVE_LIBPORTAUDIO) && defined(HAVE_LIBSNDFILE)
	if(e->soundMixer) brFreeSoundMixer(e->soundMixer);
#endif

#if defined(HAVE_LIBPORTAUDIO) && defined(HAVE_LIBSNDFILE)
	Pa_Terminate();
#endif

	for(bi = e->instances.begin(); bi != e->instances.end(); bi++ )
		brInstanceRelease(*bi);

	for(bi = e->instancesToAdd.begin(); bi != e->instancesToAdd.end(); bi++ )
		brInstanceRelease(*bi);

	if(e->path) delete[] e->path;

	brEngineRemoveDlPlugins(e);

	if(e->error.file) {
		slFree(e->error.file);
		e->error.file = NULL;
		e->error.type = 0;
	}

	if(e->camera) slCameraFree(e->camera);
	if(e->world) slWorldFree(e->world);
	if(e->outputPath) slFree(e->outputPath);
	if(e->iTunesData) delete e->iTunesData;

	for(wi = e->windows.begin(); wi != e->windows.end(); wi++ ) 
		e->freeWindowCallback(*wi);

	for(bi = e->freedInstances.begin(); bi != e->freedInstances.end(); bi++ )
		delete *bi;

#ifdef HAVE_LIBOSMESA
	slFree(e->osBuffer);
	OSMesaDestroyContext(e->osContext);
#endif

	brNamespaceFreeWithFunction(e->internalMethods, (void(*)(void*))brFreeBreveCall);

	std::map<std::string,brObject*>::iterator oi;

	for(oi = e->objects.begin(); oi != e->objects.end(); oi++ )
		if(oi->second) brObjectFree(oi->second);

	brFreeSearchPath(e);

	delete e;
}

/*!
	\brief Set the controller object for the simulation.

	This function should be called as soon as the controller instance 
	is created <b>before the controller's initialization methods are
	even called</b>.  
*/

int brEngineSetController(brEngine *e, brInstance *instance) {
	if(e->controller) {
		slMessage(DEBUG_ALL, "Error: redefinition of \"Controller\" object\n");
		return -1;
	}

	e->controller = instance;

	return 0;
}

brInstance *brEngineGetController(brEngine *e) {
	return e->controller;
}

slStack *brEngineGetAllInstances(brEngine *e) {
	std::vector<brInstance*>::iterator ii;
	slStack *s = slStackNew();
	
	for(ii = e->instances.begin(); ii != e->instances.end(); ii++ ) {
			slStackPush(s, *ii);
	}

	return s;
}

/*!
	\brief Sets the output path, and adds the path to the search path.
*/

void brEngineSetIOPath(brEngine *e, char *path) {
	if(e->outputPath) slFree(e->outputPath);
	e->outputPath = slStrdup(path);
	brAddSearchPath(e, path);
}

/*!
	\brief Takes a filename, and returns a slMalloc'd string with the full output path for that file.
*/

char *brOutputPath(brEngine *e, char *filename) {
	char *f;

	if(*filename == '/') return slStrdup(filename);

	f = (char*)slMalloc(strlen(filename) + strlen(e->outputPath) + 3);

	sprintf(f, "%s/%s", e->outputPath, filename);

	return f;
}

/*!
	\brief Pause the simulation timer.

    Optional call to be made when the engine is paused,
    so that information about simulation speed can be measured.

	Used in conjunction with \ref brUnpauseTimer.
*/

void brPauseTimer(brEngine *e) {
	struct timeval tv;

	if(e->startTime.tv_sec == 0 && e->startTime.tv_usec == 0) return;

	gettimeofday(&tv, NULL);

	e->realTime.tv_sec += (tv.tv_sec - e->startTime.tv_sec);
	e->realTime.tv_usec += (tv.tv_usec - e->startTime.tv_usec);

	e->startTime.tv_sec = 0;
	e->startTime.tv_usec = 0;
}

/*!
    \brief Unpause the simulation timer.

    Optional call to be made when the engine is running at fullspeed,
    so that information about simulation speed can be measured.

	Used in conjunction with \ref brPauseTimer.
*/

void brUnpauseTimer(brEngine *e) {
	gettimeofday(&e->startTime, NULL);
}

/*!  
	\brief Adds a call to a method for an instance at a given time.
*/

brEvent *brEngineAddEvent(brEngine *e, brInstance *i, char *methodName, double time) {
	brEvent *event;
	std::vector<brEvent*>::iterator ei;

	event = new brEvent;

	event->name = slStrdup(methodName);
	event->time = time;
	event->instance = i;

	// insert the event where it belongs according to the time it will be called 

	if(e->events.size() == 0) {
		e->events.push_back(event);
		return event;
	}

	ei = e->events.end() - 1;

	while(ei != e->events.begin() && (*ei)->time < time) ei--;

	e->events.insert(ei, event);

	return event;
}

/*!
	\brief Iterates the breve engine.

	Iterates an engine by:
		- checking for freed objects 
		- calling the iterate method for all objects in the engine
		- checking to see if the time has come for an event to be called
*/ 

int brEngineIterate(brEngine *e) {
	brEval result;
	int rcode;
	brEvent *event;
	std::vector<brInstance*>::iterator bi;
	int n = 0;

	brInstance *i;

	pthread_mutex_lock(&e->lock);

	e->lastScheduled = -1;

	for(bi = e->instancesToAdd.begin(); bi != e->instancesToAdd.end(); bi++ ) {
		i = *bi;

		e->instances.push_back(i);
    
		if(i->iterate) e->iterationInstances.push_back(i);
		if(i->postIterate) e->postIterationInstances.push_back(i);
	}

	e->instancesToAdd.clear();

	for(bi = e->instancesToRemove.begin(); bi != e->instancesToRemove.end(); bi++ ) {
		 i = *bi;

		brEngineRemoveInstance(e, i);
	}

	e->instancesToRemove.clear();

	for(bi = e->iterationInstances.begin(); bi != e->iterationInstances.end(); bi++ ) {
		i = *bi;

		n++;

		if(i->status == AS_ACTIVE) {
			rcode = brMethodCall(i, i->iterate, NULL, &result);

			if(rcode < 0) {
				pthread_mutex_unlock(&e->lock);
				return rcode;
			}
		}
	}

	for(bi = e->postIterationInstances.begin(); bi != e->postIterationInstances.end(); bi++ ) {
		i = *bi;

		if(i->status == AS_ACTIVE) {
			rcode = brMethodCall(i, i->postIterate, NULL, &result);

			if(rcode < 0) {
				pthread_mutex_unlock(&e->lock);
				return rcode;
			}
		}
	}

	double oldAge = slWorldGetAge(e->world);

	while(!e->events.empty() && (oldAge + e->iterationStepSize) >= e->events.back()->time) {
		event = e->events.back();

		if(event->instance->status == AS_ACTIVE) {
			slWorldSetAge(e->world, event->time);

			rcode = brMethodCallByName(event->instance, event->name, &result);

			brEventFree(event);

			slWorldSetAge(e->world, oldAge);

			if(rcode != EC_OK) {
				pthread_mutex_unlock(&e->lock);
				return rcode;
			}
		}

		e->events.pop_back();
	}

	pthread_mutex_unlock(&e->lock);

	fflush(e->logFile);

	if(e->simulationWillStop) return EC_STOP;

	return EC_OK;
}

/*!
	\brief Adds a search path for the breve engine.

	The search path is used whenever the engine needs to find a 
	simulation file or resource.
*/

void brAddSearchPath(brEngine *e, char *path) {
	e->searchPath.push_back(slStrdup(path));
}

/*!
	\brief Finds a file in the engine's file paths.

	Takes an engine and a relative filename, and looks for the file 
	in the engine's search path.  If the file is found, it is returned 
	as an slMalloc'd string which must be freed by the caller.
*/

char *brFindFile(brEngine *e, char *file, struct stat *st) {
	char path[4096];
	struct stat localStat, *sp;
	std::vector<char*>::iterator pi;

	if(!file || !*file) return NULL;

	if(st) sp = st;
	else sp = &localStat;

	// first try the file as an absolute path 

	if(!stat(file, sp)) return slStrdup(file);

	for(pi = e->searchPath.begin(); pi != e->searchPath.end(); pi++ ) {
		snprintf(path, 4095, "%s/%s", *pi, file);   

		if(!stat(path, sp)) return slStrdup(path);
	}

	return NULL;
}

/*!
	\brief Render the current simulation world to an active OpenGL context.

	Requires that a valid OpenGL context is active.
*/

void brEngineRenderWorld(brEngine *e, int crosshair) {
    slRenderScene(e->world, e->camera, crosshair);
}

/*@}*/

/*!
	\brief Frees the whole search path list.

	Used internally when the brEngine struct is destroyed.
*/

void brFreeSearchPath(brEngine *e) {
	std::vector<char*>::iterator pi;

	for(pi = e->searchPath.begin(); pi != e->searchPath.end(); pi++ ) slFree(*pi); 
}

/*!
	\brief Prints the current version and build timestamp.
*/

void brPrintVersion() {
	fprintf(stderr, "breve version %s (%s)\n", interfaceID, __DATE__);
	exit(1);
}

/*!
	\brief Frees a single brEvent.  

	Used internally when the \ref brEngine is finished with an event.
*/

void brEventFree(brEvent *e) {
	slFree(e->name);
	delete e;
}

/*!
	\brief Allocates memory for iTunes plugin data.

	Used only when breve is being built as an iTunes plugin.  Which
	is to say, not a whole lot.
*/

void brMakeiTunesData(brEngine *e) {
	e->iTunesData = new briTunesData;
	e->iTunesData->data = NULL;
}

/*!
	\brief Triggers a run-time simulation error.

	Takes an engine, a type (one of the \ref parseErrorMessageCodes), and 
	a set of printf-style arguments (format string and data).

	Exactly how the error is handled depends on the simulation frontend,
	but this will typically cause a simulation to die.
*/

void brEvalError(brEngine *e, int type, char *proto, ...) {
    va_list vp;
    char localMessage[BR_ERROR_TEXT_SIZE];

    if(e->error.type == 0) {
        e->error.type = type;

        va_start(vp, proto);
        vsnprintf(e->error.message, BR_ERROR_TEXT_SIZE, proto, vp);
        va_end(vp);

        slMessage(DEBUG_ALL, e->error.message);
    } else {
        va_start(vp, proto);
        vsnprintf(localMessage, BR_ERROR_TEXT_SIZE, proto, vp);
        va_end(vp);

        slMessage(DEBUG_ALL, localMessage);
    }

    slMessage(DEBUG_ALL, "\n");
}

void brClearError(brEngine *e) {
	e->error.type = 0;
}

int brGetError(brEngine *e) {
	return e->error.type;
}


/*!
	\brief A wrapper for slMessage.

	This function is a callback used by funopen() to associate a 
	FILE* pointer with the slMessage logging system.  This allows
	output written to a file pointer to be directed to the breve
	log.  This function is not typically called manually.
*/

int brFileLogWrite(void *m, const char *buffer, int length) {
	char *s = (char*)alloca(length + 1);
	strncpy(s, buffer, length);

	s[length] = 0;

	slMessage(DEBUG_ALL, s);

	return length;
}

/*!
	\brief Calls the interfaceSetCallback.
*/

int brEngineSetInterface(brEngine *e, char *name) {
	if(!e->interfaceSetCallback) return -1;

	e->interfaceSetCallback(name);
	return 0;
}

/*!
	\brief Returns a pointer to the error info structure.
*/

brErrorInfo *brEngineGetErrorInfo(brEngine *e) {
	return &e->error;
}

/*!
	\brief Returns the working directory where breve was started.
*/

char *brEngineGetPath(brEngine *e) {
	return e->path;
}

slCamera *brEngineGetCamera(brEngine *e) {
	return e->camera;
}

slWorld *brEngineGetWorld(brEngine *e) {
	return e->world;
}

/*!
	\brief Returns the internal methods namespace.
*/

brNamespace *brEngineGetInternalMethods(brEngine *e) {
	return e->internalMethods;
}

void brEngineSetSoundCallback(brEngine *e, int (*callback)()) {
	e->soundCallback = callback;
}

void brEngineSetDialogCallback(brEngine *e, int (*callback)(char *, char *, char *, char *)) {
	e->dialogCallback = callback;
}

void brEngineSetGetSavenameCallback(brEngine *e, char *(*callback)()) {
	e->getSavename = callback;
}

void brEngineSetGetLoadnameCallback(brEngine *e, char *(*callback)()) {
	e->getLoadname = callback;
}

void brEngineSetPauseCallback(brEngine *e, int (callback)()) {
	e->pauseCallback = callback;
}

void brEngineSetInterfaceInterfaceTypeCallback(brEngine *e, char *(*callback)()) {
	e->interfaceTypeCallback = callback;
}

void brEngineSetInterfaceSetStringCallback(brEngine *e, int (*callback)(char*, int)) {
	e->interfaceSetStringCallback = callback;
}

void brEngineSetInterfaceSetNibCallback(brEngine *e, void (*callback)(char*)) {
	e->interfaceSetCallback = callback;
}

void brEngineSetUpdateMenuCallback(brEngine *e, void (*updateMenu)(brInstance *l)) {
	e->updateMenu = updateMenu;
}

int brEngineGetDrawEveryFrame(brEngine *e) {
	return e->drawEveryFrame;
}

