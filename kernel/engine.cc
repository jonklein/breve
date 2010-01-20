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

#include "config.h"

#include "kernel.h"
#include "world.h"
#include "camera.h"
#include "movie.h"
#include "render.h"
#include "url.h"
#include "sound.h"

#ifndef WINDOWS
#include <dlfcn.h>
#endif

char *interfaceID;

/** \defgroup breveEngineAPI The breve engine API: using a breve simulation from another program or application frontend */
/*@{*/

void brEngine::lock() {
	pthread_mutex_lock( &_engineLock );
}

void brEngine::unlock() {
	pthread_mutex_unlock( &_engineLock );
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
 * \brief Creates a brEngine structure.
 *
 * Creates and initializes the bloated brEngine structure.  This is the first step in starting a breve simulation.
 */

brEngine::brEngine( int inArgc, const char **inArgv ) {
	_argc = inArgc;
	_argv = inArgv;

	updateMenu = NULL;
	getSavename = NULL;
	getLoadname = NULL;
	dialogCallback = NULL;
	soundCallback = NULL;
	interfaceTypeCallback = NULL;
	interfaceSetStringCallback = NULL;
	interfaceSetCallback = NULL;
	pauseCallback = NULL;
	newWindowCallback = NULL;
	freeWindowCallback = NULL;
	renderWindowCallback = NULL;

	_url = NULL;
	_soundMixer = NULL;
	_controller = NULL;

	_renderer = new slRenderGL;

	memset( keys, 0, sizeof( keys ) );

	// wxWindows change change the locale -- I'm not cool with that
	setlocale( LC_ALL, "C" );

#ifdef HAVE_LIBCURL
	_url = new brURLFetcher;
#endif

#if defined(HAVE_LIBPORTAUDIO) && defined(HAVE_LIBSNDFILE)
	_soundMixer = new brSoundMixer;
#endif

#if WINDOWS
	pthread_win32_process_attach_np();
	WSADATA wsaData;
	WSAStartup( 0x0101, &wsaData );
#endif

	gettimeofday( &unpauseTime, NULL );
	accumulatedTime.tv_sec = 0;
	accumulatedTime.tv_usec = 0;

	brClearError( this );

#if HAVE_LIBAVCODEC && HAVE_LIBAVFORMAT && HAVE_LIBAVUTIL && HAVE_LIBSWSCALE
	av_register_all();
#endif

	gsl_set_error_handler_off();
	RNG = gsl_rng_alloc( gsl_rng_mt19937 );

	_simulationWillStop = 0;

	camera = new slCamera( 400, 400 );

	nThreads = 1;

	// under OSX we'll connect a file handle to the output queue to allow
	// file handle output to the message logs (useful to plugins).  Under
	// the linux version, we can't and don't really need to do this--they
	// can just use stderr.

#if MACOSX
	_logFile = funopen( this, NULL, brFileLogWrite, NULL, NULL );
#else
	_logFile = stderr;
#endif

	drawEveryFrame = 1;

	char path[ MAXPATHLEN + 1 ];
	getcwd( path, MAXPATHLEN );
	_launchDirectory = path;
	brEngineSetIOPath( this, path );

	world = new slWorld();

	world -> setCollisionCallbacks( brCheckCollisionCallback, brCollisionCallback );

	if ( pthread_mutex_init( &_engineLock , NULL ) ) {
		slMessage( 0, "warning: error creating lock for breve engine\n" );

		if ( nThreads > 1 ) {
			slMessage( 0, "cannot start multi-threaded simulation without lock\n" );
			return;
		}
	}

	// namespaces holding object names and method names

	internalMethods = brNamespaceNew();
	brLoadInternalFunctions( this );

	// add the default class path, and check the BREVE_CLASS_PATH
	// environment variable to see if it adds any more

	addSearchPath( "lib/classes" );
	addSearchPath( "lib" );
	addSearchPath( "." );

#if WINDOWS
	#define PATHSEP	";"
#else
	#define PATHSEP ":"
#endif

	char *envpath, *dir;
	int n = 0;

	if ( ( envpath = getenv( "BREVE_CLASS_PATH" ) ) ) {
		while ( ( dir = slSplit( envpath, PATHSEP, n++ ) ) ) {
			addSearchPath( dir );
			slMessage( DEBUG_INFO, "adding \"%s\" to class path\n", dir );
			slFree( dir );
		}
	}

	if ( ( envpath = getenv( "HOME" ) ) ) addSearchPath( envpath );

	for ( int t = 1; t < nThreads; t++ ) {
		stThreadData *data;

		data = new stThreadData;
		data -> engine = this;
		data -> number = t;
		pthread_create( &data->thread, NULL, brIterationThread, data );
	}
}

/**
 * \brief Looks up an internal breve function.
 *
 * Searches in the engine for a brInternalFunction corresponding with the
 * given name.
 */

brInternalFunction *brEngineInternalFunctionLookup( brEngine *e, char *name ) {
	brNamespaceSymbol *s = brNamespaceLookup( e->internalMethods, name );

	if ( s ) 
		return ( brInternalFunction * )s->data;

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

	delete _renderer;

#ifdef HAVE_LIBCURL
	if( _url )
		delete _url;
#endif

#if defined(HAVE_LIBPORTAUDIO) && defined(HAVE_LIBSNDFILE)
	if( _soundMixer )
		delete _soundMixer;
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

	for ( bi = _freedInstances.begin(); bi != _freedInstances.end(); bi++ )
		delete *bi;

	brNamespaceFreeWithFunction( internalMethods, ( void( * )( void* ) )brFreeBreveCall );

	std::map< std::string, brObject* >::iterator oi;

	for ( oi = objects.begin(); oi != objects.end(); oi++ )
		if ( oi->second ) brObjectFree( oi->second );

	for( unsigned int i = 0; i < _objectTypes.size(); i++ ) {
		brObjectType *t = _objectTypes[ i ];

		if( t )
			delete t;
	}

	for( unsigned int e = 0; e < events.size(); e++ ) {
		delete events[ e ];
		events[ e ] = NULL;
	}
}

/**
 * \brief Set the controller object for the simulation.
 *
 * This function should be called as soon as the controller instance
 * is created <b>before the controller's initialization methods are
 * even called</b>.
 */
int brEngine::setController( brInstance *instance ) {
	if ( _controller ) {
		slMessage( DEBUG_ALL, "Error: redefinition of \"Controller\" object\n" );
		return -1;
	}

	_controller = instance;

	return 0;
}

/**
 * \brief Sets the output path, and adds the path to the search path.
 */

void brEngineSetIOPath( brEngine *inEngine, const char *inPath ) {
	inEngine -> _outputPath = inPath;
	inEngine -> addSearchPath( inPath );
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

void brEngine::pauseTimer() {
	if ( unpauseTime.tv_sec == 0 && unpauseTime.tv_usec == 0 )
		return;

	struct timeval tv;
	gettimeofday( &tv, NULL );

	accumulatedTime.tv_sec += ( tv.tv_sec - unpauseTime.tv_sec );
	accumulatedTime.tv_usec += ( tv.tv_usec - unpauseTime.tv_usec );

	unpauseTime.tv_sec = 0;
	unpauseTime.tv_usec = 0;
}

/**
 * \brief Unpause the simulation timer.
 *
 * Optional call to be made when the engine is running at fullspeed,
 * so that information about simulation speed can be measured.
 *
 * Used in conjunction with \ref brPauseTimer.
 */

void brEngine::unpauseTimer() {
	gettimeofday( &unpauseTime, NULL );
}

/**
 * Gets the accumulated running time.
 */

double brEngine::runningTime() {
	struct timeval tv, current;

	gettimeofday( &tv, NULL );
	current.tv_sec  = accumulatedTime.tv_sec  + ( tv.tv_sec  - unpauseTime.tv_sec );
	current.tv_usec = accumulatedTime.tv_usec + ( tv.tv_usec - unpauseTime.tv_usec );

	return (double)current.tv_sec + current.tv_usec / 1000000.0;
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

/*!
	\brief Removes a call to a method for an instance at a given time.
*/

int brEngineRemoveEvent( brEngine *e, brInstance *i, char *methodName, double time ) {
        std::vector<brEvent*>::iterator ei, nei;

	ei = e->events.begin();

	if ( e->events.size() == 0 ) {
		return EC_OK;
	}

	// Loop through the list as long as the time of the event to remove is after NOW or we want to remove all events with this name (time==0)
	while ( ei != e->events.end()-1 && (time >= e->world->getAge( ) || time == 0) ) {

	  // Find the right instance with time >= NOW
	  if( ( *ei )->_instance == i && ( *ei )->_time >= e->world->getAge( ) ) {

	    // Cases: matching time & matching name, matching name for all time, or method to remove is empty
	    if(( ( *ei )->_time == time && 
		 strcmp(( *ei )->_name,methodName)==0) ||
	       ( time == 0 && strcmp(( *ei )->_name,methodName)==0 ) ||
	       ( strcmp("",methodName)==0 )) {

	      nei = e->events.erase(ei);

	    } else {
	      ei++;
	    }
	  } else {
	    ei++;
	  }
	}

	return EC_OK;
}

/**
 * Iterates the breve engine.
 * Iterates an engine by:
 * - checking for freed objects
 * - calling the iterate method for all objects in the engine
 * - checking to see if the time has come for an event to be called
 */

int brEngine::iterate() {
	brEval result;
	brEvent *event;
	std::vector<brInstance*>::iterator bi;
	int n = 0;

	brInstance *i;

	lock();

	lastScheduled = -1;

	for ( bi = instancesToAdd.begin(); bi != instancesToAdd.end(); bi++ ) {
		i = *bi;

		instances.push_back( i );

		if ( i->iterate ) iterationInstances.push_back( i );

		if ( i->postIterate ) postIterationInstances.push_back( i );
	}

	instancesToAdd.clear();

	for ( bi = instancesToRemove.begin(); bi != instancesToRemove.end(); bi++ ) {
		i = *bi;

		brEngineRemoveInstance( this, i );
	}

	instancesToRemove.clear();

	for ( bi = iterationInstances.begin(); bi != iterationInstances.end(); bi++ ) {
		i = *bi;

		n++;

		if ( i->status == AS_ACTIVE ) {
			if ( brMethodCall( i, i->iterate, NULL, &result ) != EC_OK ) {
				unlock();
				return EC_ERROR;
			}
		}
	}

	for ( bi = postIterationInstances.begin(); bi != postIterationInstances.end(); bi++ ) {
		i = *bi;

		if ( i->status == AS_ACTIVE ) {
			if ( brMethodCall( i, i->postIterate, NULL, &result ) != EC_OK ) {
				unlock();
				return EC_ERROR;
			}
		}
	}

	double oldAge = world->getAge();

	while ( !events.empty() && ( oldAge + _iterationStepSize ) >= events.back()->_time ) {
		event = events.back();

		if ( event->_instance->status == AS_ACTIVE ) {
			world -> setAge( event->_time );

			int rcode = brMethodCallByName( event->_instance, event->_name, &result );

			world->setAge( oldAge );

			if ( rcode != EC_OK ) {
				unlock();
				return rcode;
			}

		}

		if ( event->_interval != 0.0 ) {
			brEngineAddEvent( this, event->_instance, event->_name, event->_time + event->_interval, event->_interval );
		}

		delete event;

		events.pop_back();
	}

	unlock();

	fflush( _logFile );

	if ( _simulationWillStop ) 
		return EC_STOP;

	return EC_OK;
}

/*!
	\brief Adds a search path for the breve engine.

	The search path is used whenever the engine needs to find a
	simulation file or resource.
*/

void brReplaceSubstring( std::string *inStr, const char *sub, const char *repl ) { 
	unsigned int pos = 0;

	while( 1 ) {
    		pos = inStr->find( sub, pos ); 

		if ( pos == std::string::npos || pos >=inStr->size() ) 
			return;

		inStr->replace( pos, strlen( sub ), repl ); 

		pos += strlen( repl );
	}
 } 

void brEngine::addSearchPath( const char *inPath ) {
	std::string newPath( inPath );

	brReplaceSubstring( &newPath, "\\", "\\\\" );
	slMessage( DEBUG_INFO, "adding search path %s\n", inPath );
	_searchPaths.push_back( newPath );
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

void brEngine::draw() {
	lock();
	world -> draw( *_renderer, camera );
	unlock();
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

void brEvalError( brEngine *e, int type, const char *proto, ... ) {
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

void brEngineSetGetSavenameCallback( brEngine *e, const char *( *callback )( void ) ) {
	e->getSavename = callback;
}

void brEngineSetGetLoadnameCallback( brEngine *e, const char *( *callback )( void ) ) {
	e->getLoadname = callback;
}

void brEngineSetPauseCallback( brEngine *e, int( callback )( void ) ) {
	e->pauseCallback = callback;
}

void brEngineSetUnpauseCallback( brEngine *e, int( callback )( void ) ) {
	e->unpauseCallback = callback;
}

void brEngineSetInterfaceInterfaceTypeCallback( brEngine *e, const char *( *callback )( void ) ) {
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
