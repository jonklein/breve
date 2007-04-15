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
#include "world.h"
#include "gldraw.h"
#include "camera.h"
#include "movie.h"

#ifndef WINDOWS
#include <dlfcn.h>
#endif

char *interfaceID;

/** \defgroup breveEngineAPI The breve engine API: using a breve simulation from another program or application frontend */
/*@{*/

void brEngineLock( brEngine *e ) {
	( pthread_mutex_lock( &( e )->lock ) );
}

void brEngineUnlock( brEngine *e ) {
	( pthread_mutex_unlock( &( e )->lock ) );
}

brEvent::brEvent( char *n, double t, double interval, brInstance *i ) {
	_instance = i;
	_name = slStrdup( n );
	_time = t;
	_interval = interval;
}

brEvent::~brEvent() {
	slFree( _name );
}

/**
 * \brief Creates a brEngine structure with argc and argv values filled in.
 *
 * Creates and initializes the bloated brEngine structure.  This is the first step in starting a breve simulation.
 */

brEngine *brEngineNewWithArguments( int inArgc, char **inArgv ) {
	brEngine *e = brEngineNew();

	e->argc = inArgc;
	e->argv = inArgv;

	return e;
}

/**
 * \brief Creates a brEngine structure.
 *
 * Creates and initializes the bloated brEngine structure.  This is the first step in starting a breve simulation.
 */

brEngine *brEngineNew( void ) {
	brEngine *e;
	char *envpath, *dir;
	int n = 0;

	char wd[MAXPATHLEN];

#if MINGW
	pthread_win32_process_attach_np();
#endif

#if WINDOWS
	WSADATA wsaData;
	WSAStartup( 0x0101, &wsaData );
#endif

	// glutInit( &zero, NULL );

	e = new brEngine;

	brClearError( e );

#if HAVE_LIBAVFORMAT
	av_register_all();
#endif

#ifdef HAVE_LIBGSL
	gsl_set_error_handler_off();
#endif

	e->argc = 0;
	e->argv = NULL;

	e->updateMenu = NULL;
	e->getSavename = NULL;
	e->getLoadname = NULL;
	e->dialogCallback = NULL;
	e->soundCallback = NULL;
	e->interfaceTypeCallback = NULL;
	e->interfaceSetStringCallback = NULL;
	e->interfaceSetCallback = NULL;
	e->pauseCallback = NULL;
	e->newWindowCallback = NULL;
	e->freeWindowCallback = NULL;
	e->renderWindowCallback = NULL;
	e->controller = NULL;

	e->RNG = gsl_rng_alloc( gsl_rng_mt19937 );

	e->simulationWillStop = 0;

	e->camera = new slCamera( 400, 400 );

#if HAVE_LIBPORTAUDIO && HAVE_LIBSNDFILE
	Pa_Initialize();
	e->soundMixer = new brSoundMixer();
#endif

	e->nThreads = 1;

	// under OSX we'll connect a file handle to the output queue to allow
	// file handle output to the message logs (useful to plugins).  Under
	// the linux version, we can't and don't really need to do this--they
	// can just use stderr.

#if MACOSX
	e->logFile = funopen( e, NULL, brFileLogWrite, NULL, NULL );
#else
	e->logFile = stderr;
#endif

	e->drawEveryFrame = 1;

	char path[ MAXPATHLEN + 1 ];
	getcwd( path, MAXPATHLEN );
	e->_launchDirectory = path;

	e->world = new slWorld();

	e->world->setCollisionCallbacks( brCheckCollisionCallback, brCollisionCallback );

	gettimeofday( &e->startTime, NULL );

	e->realTime.tv_sec = 0;
	e->realTime.tv_usec = 0;

	if ( pthread_mutex_init( &e->lock , NULL ) ) {
		slMessage( 0, "warning: error creating lock for breve engine\n" );

		if ( e->nThreads > 1 ) {
			slMessage( 0, "cannot start multi-threaded simulation without lock\n" );
			return NULL;
		}
	}

	if ( pthread_mutex_init( &e->scheduleLock, NULL ) ) {
		slMessage( 0, "warning: error creating lock for breve engine\n" );

		if ( e->nThreads > 1 ) {
			slMessage( 0, "cannot start multi-threaded simulation without lock\n" );
			return NULL;
		}
	}

	if ( pthread_mutex_init( &e->conditionLock, NULL ) ) {
		slMessage( 0, "warning: error creating lock for breve engine\n" );

		if ( e->nThreads > 1 ) {
			slMessage( 0, "cannot start multi-threaded simulation without lock\n" );
			return NULL;
		}
	}

	if ( pthread_cond_init( &e->condition, NULL ) ) {
		slMessage( 0, "warning: error creating pthread condition variable\n" );
	}

	// namespaces holding object names and method names

	e->internalMethods = brNamespaceNew();

	// set up the initial search paths

	brEngineSetIOPath( e, getcwd( wd, MAXPATHLEN ) );

	// load all of the internal breve functions

	brLoadInternalFunctions( e );

	// add the default class path, and check the BREVE_CLASS_PATH
	// environment variable to see if it adds any more

	brAddSearchPath( e, "lib/classes" );
	brAddSearchPath( e, "lib/pyclasses" );

	if ( ( envpath = getenv( "BREVE_CLASS_PATH" ) ) ) {
		while (( dir = slSplit( envpath, ":", n++ ) ) ) {
			brAddSearchPath( e, dir );
			slMessage( DEBUG_INFO, "adding \"%s\" to class path\n", dir );
			slFree( dir );
		}
	}

	if (( envpath = getenv( "HOME" ) ) ) brAddSearchPath( e, envpath );

	memset( e->keys, 0, sizeof( e->keys ) );

	for ( n = 1;n < e->nThreads;n++ ) {
		stThreadData *data;

		data = new stThreadData;
		data->engine = e;
		data->number = n;
		pthread_create( &data->thread, NULL, brIterationThread, data );
	}

	return e;
}

/*!
	\brief Looks up an internal breve function.

	Searches in the engine for a brInternalFunction corresponding with the
	given name.
*/

brInternalFunction *brEngineInternalFunctionLookup( brEngine *e, char *name ) {
	brNamespaceSymbol *s = brNamespaceLookup( e->internalMethods, name );

	if ( s ) return ( brInternalFunction * )s->data;

	// The following could be a way for us to try to load functions without the
	// prototypes.  Not sure it's a good idea.

	// void *dlsymbol;
	// dlsymbol = dlsym(0, name);
	//
	// if(dlsymbol) {
	// 	slMessage(DEBUG_ALL, "registered function %s not found, but symbol was found\n", name);
	// }

	return NULL;
}


/**
 * \brief Frees a breve engine.
 * 
 * Frees the engine and everything inside of it--all objects, all instances, everything.
 */

void brEngineFree( brEngine *e ) {
	delete e;
}


brEngine::~brEngine() {
	std::vector<brInstance*>::iterator bi;
	std::vector<void*>::iterator wi;

#if HAVE_LIBPORTAUDIO && HAVE_LIBSNDFILE

	if ( soundMixer ) delete soundMixer;

#endif

#if HAVE_LIBPORTAUDIO && HAVE_LIBSNDFILE
	Pa_Terminate();

#endif

	gsl_rng_free( RNG );

	for ( bi = instances.begin(); bi != instances.end(); bi++ )
		brInstanceRelease( *bi );

	for ( bi = instancesToAdd.begin(); bi != instancesToAdd.end(); bi++ )
		brInstanceRelease( *bi );

	brEngineRemoveDlPlugins( this );

	if ( camera ) 
		delete camera;

	if ( world ) 
		delete world;

	for ( wi = windows.begin(); wi != windows.end(); wi++ )
		freeWindowCallback( *wi );

	for ( bi = freedInstances.begin(); bi != freedInstances.end(); bi++ )
		delete *bi;

	brNamespaceFreeWithFunction( internalMethods, ( void( * )( void* ) )brFreeBreveCall );

	std::map< std::string, brObject* >::iterator oi;

	for ( oi = objects.begin(); oi != objects.end(); oi++ )
		if ( oi->second ) brObjectFree( oi->second );

	for( unsigned int i = 0; i < objectTypes.size(); i++ ) {
		brObjectType *t = objectTypes[ i ];

		if( t )
			delete t;
	}
}

/*!
	\brief Set the controller object for the simulation.

	This function should be called as soon as the controller instance
	is created <b>before the controller's initialization methods are
	even called</b>.
*/

int brEngineSetController( brEngine *e, brInstance *instance ) {
	if ( e->controller ) {
		slMessage( DEBUG_ALL, "Error: redefinition of \"Controller\" object\n" );
		return -1;
	}

	e->controller = instance;

	return 0;
}

brInstance *brEngineGetController( brEngine *e ) {
	return e->controller;
}

/**
 * \brief Sets the output path, and adds the path to the search path.
 */

void brEngineSetIOPath( brEngine *inEngine, const char *inPath ) {
	inEngine->_outputPath = inPath;
	brAddSearchPath( inEngine, inPath );
}

/** 
 * Takes a filename, and returns a slMalloc'd string with the full output path for that file.
 */

char *brOutputPath( brEngine *e, const char *filename ) {
	char *f;

	if ( *filename == '/' )
		return slStrdup( filename );

	f = ( char * )slMalloc( strlen( filename ) + e->_outputPath.length() + 2 );

	sprintf( f, "%s/%s", e->_outputPath.c_str(), filename );

	return f;
}

/**
 * Pause the simulation timer.
 * Optional call to be made when the engine is paused,
 * so that information about simulation speed can be measured.
 * Used in conjunction with \ref brUnpauseTimer.
 */

void brPauseTimer( brEngine *e ) {

	struct timeval tv;

	if ( e->startTime.tv_sec == 0 && e->startTime.tv_usec == 0 )
		return;

	gettimeofday( &tv, NULL );

	e->realTime.tv_sec += ( tv.tv_sec - e->startTime.tv_sec );
	e->realTime.tv_usec += ( tv.tv_usec - e->startTime.tv_usec );

	e->startTime.tv_sec = 0;
	e->startTime.tv_usec = 0;
}

/*!
    \brief Unpause the simulation timer.

    Optional call to be made when the engine is running at fullspeed,
    so that information about simulation speed can be measured.

	Used in conjunction with \ref brPauseTimer.
*/

void brUnpauseTimer( brEngine *e ) {
	gettimeofday( &e->startTime, NULL );
}

/*!
	\brief Adds a call to a method for an instance at a given time.
*/

brEvent *brEngineAddEvent( brEngine *e, brInstance *i, char *methodName, double time, double interval ) {
	brEvent *event;
	std::vector<brEvent*>::iterator ei;

	event = new brEvent( methodName, time, interval, i );

	// insert the event where it belongs according to the time it will be called

	ei = e->events.end() - 1;

	if ( e->events.size() == 0 ) {
		e->events.push_back( event );
		return event;
	}

	while ( ei != e->events.begin() && ( *ei )->_time < time ) ei--;

	// we want to insert AFTER the current event...

	if ((( *ei )->_time ) > time ) ei++;

	e->events.insert( ei, event );

	return event;
}

/**
 * Iterates the breve engine.
 * Iterates an engine by:
 * - checking for freed objects
 * - calling the iterate method for all objects in the engine
 * - checking to see if the time has come for an event to be called
 */

int brEngineIterate( brEngine *e ) {
	brEval result;
	brEvent *event;
	std::vector<brInstance*>::iterator bi;
	int n = 0;

	brInstance *i;

	pthread_mutex_lock( &e->lock );

	e->lastScheduled = -1;

	for ( bi = e->instancesToAdd.begin(); bi != e->instancesToAdd.end(); bi++ ) {
		i = *bi;

		e->instances.push_back( i );

		if ( i->iterate ) e->iterationInstances.push_back( i );

		if ( i->postIterate ) e->postIterationInstances.push_back( i );
	}

	e->instancesToAdd.clear();

	for ( bi = e->instancesToRemove.begin(); bi != e->instancesToRemove.end(); bi++ ) {
		i = *bi;

		brEngineRemoveInstance( e, i );
	}

	e->instancesToRemove.clear();

	for ( bi = e->iterationInstances.begin(); bi != e->iterationInstances.end(); bi++ ) {
		i = *bi;

		n++;

		if ( i->status == AS_ACTIVE ) {
			if ( brMethodCall( i, i->iterate, NULL, &result ) != EC_OK ) {

				pthread_mutex_unlock( &e->lock );

				return EC_ERROR;
			}
		}
	}

	for ( bi = e->postIterationInstances.begin(); bi != e->postIterationInstances.end(); bi++ ) {
		i = *bi;

		if ( i->status == AS_ACTIVE ) {
			if ( brMethodCall( i, i->postIterate, NULL, &result ) != EC_OK ) {

				pthread_mutex_unlock( &e->lock );

				return EC_ERROR;
			}
		}
	}

	double oldAge = e->world->getAge();

	while ( !e->events.empty() && ( oldAge + e->iterationStepSize ) >= e->events.back()->_time ) {
		event = e->events.back();

		if ( event->_instance->status == AS_ACTIVE ) {
			e->world->setAge( event->_time );

			int rcode = brMethodCallByName( event->_instance, event->_name, &result );

			e->world->setAge( oldAge );

			if ( rcode != EC_OK ) {

				pthread_mutex_unlock( &e->lock );

				return rcode;
			}

		}

		if ( event->_interval != 0.0 ) {
			brEngineAddEvent( e, event->_instance, event->_name, event->_time + event->_interval, event->_interval );
		}

		delete event;

		e->events.pop_back();
	}

	pthread_mutex_unlock( &e->lock );

	fflush( e->logFile );

	if ( e->simulationWillStop ) return EC_STOP;

	return EC_OK;
}

/*!
	\brief Adds a search path for the breve engine.

	The search path is used whenever the engine needs to find a
	simulation file or resource.
*/

void replace_substring( std::string *inStr, char *sub, char *repl ) { 
	unsigned int pos = 0;

	while( 1 ) {
    		pos = inStr->find( sub, pos ); 

		if ( pos == std::string::npos ) 
			return;

		inStr->replace( pos, strlen( sub ), repl ); 

		pos += strlen( repl );
	}
 } 

void brAddSearchPath( brEngine *e, const char *path ) {
	std::string newPath( path );

	replace_substring( &newPath, "\\", "\\\\" );
	slMessage( DEBUG_INFO, "adding search path %s\n", path );
	e->_searchPaths.push_back( newPath );
}

std::vector< std::string > brEngineGetAllObjectNames( brEngine *e ) {
	std::vector< std::string > names;

	std::map< std::string, brObject >::iterator oi;

	// for( oi = e->objects.begin(); oi != e->objects.end(); oi++ ) {
	// 
	// }

	return names;
}

const std::vector< std::string > &brEngineGetSearchPaths( brEngine *e ) {
	return e->_searchPaths;
}

/**
 * \brief Finds a file in the engine's file paths.
 * 
 * Takes an engine and a relative filename, and looks for the file
 * in the engine's search path.  If the file is found, it is returned
 * as an slMalloc'd string which must be freed by the caller.
 */

char *brFindFile( brEngine *e, const char *file, struct stat *st ) {
	struct stat localStat;
	std::vector< std::string >::iterator pi;
	char path[ MAXPATHLEN ];

	if ( !file || *file == '\0' )
		return NULL;

	if ( !st )
		st = &localStat;

	if ( *file == '/' )
		return stat( file, st ) ? NULL : slStrdup( file );

	for ( pi = e->_searchPaths.begin(); pi != e->_searchPaths.end(); pi++ ) {
		snprintf( path, sizeof( path ), "%s/%s", (*pi).c_str(), file );

		if ( !stat( path, st ) )
			return slStrdup( path );
	}

	return NULL;
}

/**
 * \brief Render the current simulation world to an active OpenGL context.
 * Requires that a valid OpenGL context is active.
 */

void brEngineRenderWorld( brEngine *e, int crosshair ) {
	e->camera->renderScene( e->world, crosshair );
}

/*@}*/

/**
 * Prints the current version and build timestamp.
 */

void brPrintVersion() {
	fprintf( stderr, "breve version %s (%s)\n", interfaceID, __DATE__ );
	exit( 1 );
}

/**
 * \brief Triggers a run-time simulation error. 
 * 
 * Takes an engine, a type (one of the \ref parseErrorMessageCodes), and
 * a set of printf-style arguments (format string and data).
 *
 * Exactly how the error is handled depends on the simulation frontend,
 * but this will typically cause a simulation to die.
 */

void brEvalError( brEngine *e, int type, char *proto, ... ) {
	va_list vp;
	char localMessage[ BR_ERROR_TEXT_SIZE ];

	if ( e->error.type == 0 ) {
		e->error.type = type;

		va_start( vp, proto );
		vsnprintf( e->error.message, BR_ERROR_TEXT_SIZE, proto, vp );
		va_end( vp );

		slMessage( DEBUG_ALL, e->error.message );
	} else {
		va_start( vp, proto );
		vsnprintf( localMessage, BR_ERROR_TEXT_SIZE, proto, vp );
		va_end( vp );

		slMessage( DEBUG_ALL, localMessage );
	}

	slMessage( DEBUG_ALL, "\n" );
}

/**
 * Clears the current error data.
 */

void brClearError( brEngine *e ) {
	e->error.clear();
}

/**
 * Returns the current error code.
 */

int brGetError( brEngine *e ) {
	return e->error.type;
}


/**
 * \brief A wrapper for slMessage.
 * This function is a callback used by funopen() to associate a
 * FILE* pointer with the slMessage logging system.  This allows
 * output written to a file pointer to be directed to the breve
 * log.  This function is not typically called manually.
 */

int brFileLogWrite( void *m, const char *buffer, int length ) {
	char *s = ( char* )alloca( length + 1 );

	strncpy( s, buffer, length );
	s[length] = 0;

	slMessage( DEBUG_ALL, s );

	return length;
}

/**
 * \brief Calls the interfaceSetCallback.
 */

int brEngineSetInterface( brEngine *e, char *name ) {
	if ( !e->interfaceSetCallback )
		return -1;

	e->interfaceSetCallback( name );

	return 0;
}

/**
 * \brief Returns a pointer to the error info structure.
 */

brErrorInfo *brEngineGetErrorInfo( brEngine *e ) {
	return &e->error;
}

/**
 * Returns the working directory where breve was started.
 */

const char *brEngineGetPath( brEngine *e ) {
	return e->_launchDirectory.c_str();
}

/**
 * Returns the slCamera associated with this breve engine.
 */

slCamera *brEngineGetCamera( brEngine *e ) {
	return e->camera;
}

/**
 * Returns the slWorld associated with this breve engine.
 */

slWorld *brEngineGetWorld( brEngine *e ) {
	return e->world;
}

/**
 * \brief Returns the internal methods namespace.
 */

brNamespace *brEngineGetInternalMethods( brEngine *e ) {
	return e->internalMethods;
}

void brEngineSetSoundCallback( brEngine *e, int( *callback )( void ) ) {
	e->soundCallback = callback;
}

void brEngineSetDialogCallback( brEngine *e, int( *callback )( char *, char *, char *, char * ) ) {
	e->dialogCallback = callback;
}

void brEngineSetGetSavenameCallback( brEngine *e, char *( *callback )( void ) ) {
	e->getSavename = callback;
}

void brEngineSetGetLoadnameCallback( brEngine *e, char *( *callback )( void ) ) {
	e->getLoadname = callback;
}

void brEngineSetPauseCallback( brEngine *e, int( callback )( void ) ) {
	e->pauseCallback = callback;
}

void brEngineSetUnpauseCallback( brEngine *e, int( callback )( void ) ) {
	e->unpauseCallback = callback;
}

void brEngineSetInterfaceInterfaceTypeCallback( brEngine *e, char *( *callback )( void ) ) {
	e->interfaceTypeCallback = callback;
}

void brEngineSetInterfaceSetStringCallback( brEngine *e, int( *callback )( char *, int ) ) {
	e->interfaceSetStringCallback = callback;
}

void brEngineSetInterfaceSetNibCallback( brEngine *e, void( *callback )( char * ) ) {
	e->interfaceSetCallback = callback;
}

void brEngineSetUpdateMenuCallback( brEngine *e, void( *updateMenu )( brInstance * ) ) {
	e->updateMenu = updateMenu;
}

int brEngineGetDrawEveryFrame( brEngine *e ) {
	return e->drawEveryFrame;
}
