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

/*!
    \brief Creates a brEngine structure.

    Creates and initializes the bloated brEngine structure.  This is the 
    first step in starting a breve simulation.
*/

brEngine *brEngineNew() {
	brEngine *e;
	char *envpath, *dir;
	int n = 0;

#ifdef MINGW
	// windows requires that we do some initialization before using threads.  bitches.
	pthread_win32_process_attach_np();
#endif

#if defined(HAVE_LIBPORTAUDIO) && defined(HAVE_LIBSNDFILE)
	Pa_Initialize();
#endif

	e = slMalloc(sizeof(brEngine));
	e->speedFactor = 1.0;

	e->simulationWillStop = 0;

	e->objectTypes = slStackNew();

#if defined(HAVE_LIBPORTAUDIO) && defined(HAVE_LIBSNDFILE)
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
#endif /* MACOSX */

	e->path = slMalloc(MAXPATHLEN + 1);
	e->drawEveryFrame = 1;

	e->dlPlugins = NULL;

	getcwd(e->path, MAXPATHLEN);

	e->world = slWorldNew();

	e->world->resolveCollisions = 0;
	e->world->detectCollisions = 0;
	e->world->collisionCallback = (void*)brCollisionCallback;
	e->world->collisionCheckCallback = (void*)brCheckCollisionCallback;

	gettimeofday(&e->startTime, NULL);

	e->realTime.tv_sec = 0;
	e->realTime.tv_usec = 0;

	e->stackRecord = NULL;

	if(!e->world) {
		slFree(e);
		return NULL;
	}

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

	e->controllerName = NULL;

	// allocate self-growing stacks for the instances and the 
	// iteration methods

	e->instances = slStackNew();
	e->postIterationInstances = slStackNew();
	e->iterationInstances = slStackNew();

	// namespaces holding object names and method names 

	e->objects = brNamespaceNew(128);
	e->internalMethods = brNamespaceNew(128);

	// set up the initial search paths 

	e->searchPath = NULL;

	brEngineSetIOPath(e, "");

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

	if((envpath = getenv("HOME"))) {
		// the user's home directory as a search path
		brAddSearchPath(e, envpath);
	}

	e->windows = NULL;

	bzero(e->keys, 256);

	for(n=1;n<e->nThreads;n++) {
		stThreadData *data;

		data = slMalloc(sizeof(stThreadData));
		data->engine = e;
		data->number = n;
		pthread_create(&data->thread, NULL, stIterationThread, data);
	}

#ifdef HAVE_LIBJAVA
	// brJavaInit(e);
#endif

	return e;
}

/*!
	\brief Frees a breve engine.
	
	Frees the engine and everything inside of it--all objects, all instances, everything.
*/

void brEngineFree(brEngine *e) {
	slList *l;
	int n;

#if defined(HAVE_LIBPORTAUDIO) && defined(HAVE_LIBSNDFILE)
	if(e->soundMixer) brFreeSoundMixer(e->soundMixer);
#endif

#if defined(HAVE_LIBPORTAUDIO) && defined(HAVE_LIBSNDFILE)
	Pa_Terminate();
#endif

#ifdef HAVE_LIBOSMESA
	slFree(e->osBuffer);
	OSMesaDestroyContext(e->osContext);
#endif

	for(n=0;n<e->instances->count;n++) 
		brInstanceRelease(e->instances->data[n]);

	for(n=0;n<e->instances->count;n++) {
		brInstance *i = e->instances->data[n];
		if(i->object->type->destroyInstance) i->object->type->destroyInstance(i);
		i->pointer = NULL;
	}

	for(n=0;n<e->instances->count;n++) 
		brInstanceFree(e->instances->data[n]);

	slStackFree(e->instances);
	slStackFree(e->iterationInstances);
	slStackFree(e->postIterationInstances);

	if(e->path) slFree(e->path);
	if(e->controllerName) slFree(e->controllerName);

	if(e->dlPlugins) {
		brEngineRemoveDlPlugins(e);
		slListFree(e->dlPlugins);	
	}

	if(e->error.file) {
		slFree(e->error.file);
		e->error.file = NULL;
		e->error.type = 0;
	}

	if(e->camera) slCameraFree(e->camera);
	if(e->world) slWorldFree(e->world);
	if(e->outputPath) slFree(e->outputPath);
	if(e->iTunesData) slFree(e->iTunesData);

	l = e->windows;

	while(l) {
		e->freeWindowCallback(l->data);
		l = l->next;
	}

	slListFree(e->windows);

	brNamespaceFreeWithFunction(e->internalMethods, (void(*)(void*))brFreeInternalFunction);
	brFreeObjectSpace(e->objects);

	brFreeSearchPath(e);

	slFree(e);
}
/*!
	\brief Set the controller object for the simulation.

	This function should be called as soon as the controller instance 
	is created <b>before the controller's initialization methods are
	even called</b>.  
*/

int brEngineSetController(brEngine *e, brInstance *instance) {
	if(e->controller) {
		stParseError(e, PE_REDEFINITION, "Redefinition of \"Controller\" object");
		return -1;
	}

	e->controller = instance;

	return 0;
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
	\brief Sets the output path.
*/

char *brOutputPath(brEngine *e, char *filename) {
	char *f;

	f = slMalloc(strlen(filename) + strlen(e->outputPath) + 3);

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
	slList *l, *eventEntry;

	event = slMalloc(sizeof(brEvent));

	event->name = slStrdup(methodName);
	event->time = time;

	eventEntry = slListPrepend(NULL, event);

	/* insert the event where it belongs according to the time it will be called */

	if(i->events) {
		l = i->events;

		/* are we less than the first entry? */

		if(((brEvent*)l->data)->time > time) {
			eventEntry->next = l;
			i->events = eventEntry;
		} else {
			while(l->next && ((brEvent*)l->next->data)->time < time) l = l->next;

			eventEntry->next = l->next;
			l->next = eventEntry;
		}
	} else {
		i->events = eventEntry;
	}

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
	int n, rcode;
	brEvent *event;
	slList *eventList;

	brInstance *i;

	pthread_mutex_lock(&e->lock);

	e->lastScheduled = -1;

	for(n=0;n<e->iterationInstances->count;n++) {
		i = e->iterationInstances->data[n];

		if(i->status == AS_ACTIVE) {
			rcode = brMethodCall(i, i->iterate, NULL, &result);

			if(rcode < 0) {
				pthread_mutex_unlock(&e->lock);
				return rcode;
			}
		}
	}

	for(n=0;n<e->postIterationInstances->count;n++) {
		i = e->postIterationInstances->data[n];

		if(i->status == AS_ACTIVE) {
			rcode = brMethodCall(i, i->postIterate, NULL, &result);

			if(rcode < 0) {
				pthread_mutex_unlock(&e->lock);
				return rcode;
			}
		}
	}

	for(n=0;n<e->instances->count;n++) {
		i = e->instances->data[n];

		if(i->status == AS_RELEASED) {
			brEngineRemoveInstance(e, i);
			brInstanceFree(i);

			// all the other objects are shifted down, so we 
			// need to check number n again 

			n--;
		} else if(i->status == AS_ACTIVE && i->events) {
			double oldAge;

			eventList = i->events;
			event = eventList->data;

			if((e->world->age + e->iterationStepSize) >= event->time) {
				i->events = eventList->next;

				oldAge = e->world->age;
				e->world->age = event->time;

				if(i->status == AS_ACTIVE) rcode = brMethodCallByName(i, event->name, &result);

				brEventFree(event);
				slListFreeHead(eventList);

				if(rcode != EC_OK) {
					pthread_mutex_unlock(&e->lock);
					return rcode;
				}

				e->world->age = oldAge;
			}
		}
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
	e->searchPath = slListAppend(e->searchPath, slStrdup(path));
}

/*!
	\brief Finds a file in the engine's file paths.

	Takes an engine and a relative filename, and looks for the file 
	in the engine's search path.  If the file is found, it is returned 
	as an slMalloc'd string which must be freed by the caller.
*/

char *brFindFile(brEngine *e, char *file, struct stat *st) {
	slList *start;
	char path[4096];
	struct stat localStat, *sp;

	if(!file || !*file) return NULL;

	if(st) sp = st;
	else sp = &localStat;

	start = e->searchPath;

	/* first try the file as an absolute path */

	if(!stat(file, sp)) return slStrdup(file);

	while(start) {
		snprintf(path, 4095, "%s/%s", (char*)start->data, file);   

		if(!stat(path, sp)) return slStrdup(path);

		start = start->next;
	}

	return NULL;
}

/*!
	\brief Render the current simulation world to an active OpenGL context.

	Requires that a valid OpenGL context is active.
*/

void brEngineRenderWorld(brEngine *e, int crosshair) {
    slRenderWorld(e->world, e->camera, 0, GL_RENDER, crosshair, 0);
}

/*@}*/

/*!
	\brief Frees the whole search path list.

	Used internally when the brEngine struct is destroyed.
*/

void brFreeSearchPath(brEngine *e) {
	slList *path;

	path = e->searchPath;

	while(path) { 
		slFree(path->data); 
		path = path->next; 
	}

	slListFree(e->searchPath);
}

/*!
	\brief Prints the current version and build timestamp.
*/

void brPrintVersion() {
	fprintf(stderr, "breve version %s (%s)\n", interfaceID, __DATE__);
	exit(1);
}

/*!
	\brief Frees all of the breve objects in a namespace.

	Frees all of the breve objects in a namespace.
*/

void brFreeObjectSpace(brNamespace *ns) {
    slList *objects, *start;
    brNamespaceSymbol *symbol;

    start = objects = brNamespaceSymbolList(ns);
    
    while(objects) {
        symbol = objects->data;

        if(symbol->type != ST_OBJECT) {
            if(symbol->type != ST_OBJECT_ALIAS) slMessage(DEBUG_ALL, "warning: found unknown type %d while freeing object list!\n", symbol->type);
        } else brObjectFree(symbol->data);

        objects = objects->next;
    }

	slListFree(start);
    
    brNamespaceFree(ns);
}

/*!
	\brief Frees a single brEvent.  

	Used internally when the \ref brEngine is finished with an event.
*/

void brEventFree(brEvent *e) {
	if(e->name) slFree(e->name);
	slFree(e);
}

/*!
	\brief Allocates memory for iTunes plugin data.

	Used only when breve is being built as an iTunes plugin.  Which
	is to say, not a whole lot.
*/

void brMakeiTunesData(brEngine *e) {
	e->iTunesData = slMalloc(sizeof(briTunesData));
	e->iTunesData->data = NULL;
}

/*!
	\brief Frees memory associated with an internal function.
	
	Used internally when the brEngine struct is destroyed.
*/

void brFreeInternalFunction(void *d) {
	brInternalFunction *i = d;

	slFree(i->name);
	slFree(i);
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

/*!
	\brief A wrapper for slMessage.

	This function is a callback used by funopen() to associate a 
	FILE* pointer with the slMessage logging system.  This allows
	output written to a file pointer to be directed to the breve
	log.  This function is not typically called manually.
*/

int brFileLogWrite(void *m, const char *buffer, int length) {
	char *s = alloca(length + 1);
	strncpy(s, buffer, length);

	s[length] = 0;

	slMessage(DEBUG_ALL, s);

	return length;
}

/*!
    \brief Pause the simulation timer.

    Optional call to be made when the engine is paused,
    so that we can track the realtime required for the simulation.
*/

