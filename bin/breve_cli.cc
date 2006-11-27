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

/*
	+ breve_cli.cc
	= this is a hack-ish wrapper around the breve engine for a command line
	= based GLUT version of breve.
*/

#define OSMESA_WINDOW_SIZE 300

#include <signal.h>

#include "steve.h"
#include "world.h"
#include "camera.h"

#include "breve.h"
#include "glIncludes.h"
#include "gldraw.h"
#include "lightdetector.h"
#include "world.h"

#ifndef WINDOWS
#include <libgen.h>
#endif


double gNotify = 40.0;
double gSimMax = 0.0;

char *gSimFile;

unsigned char *gOffscreenBuffer;

int gSlave = 0, gMaster = 0;
char *gSlaveHost;

int gShouldQuit = 0;

breveFrontend *frontend;

static int activateContext( void );

static void brCatchSignal( int );

static void renderContext( slWorld *, slCamera * );

int slLoadOSMesaPlugin( char *execPath );

int main( int argc, char **argv ) {
	int index;
	char *text;
	double nextNotify;
	char wd[MAXPATHLEN];

#if MINGW
	pthread_win32_process_attach_np();
#endif

	srandom( time( NULL ) );

	interfaceID = "cli/2.5";

	gSimFile = NULL;

	//
	// Parse the command line args.
	//

	index = brParseArgs( argc, argv );

	char *execpath = argv[ 0 ];

	// offset argc and argv to account for the options parsed out.

	argc -= index;
	argv += index;

	if ( argc < 1 ) brPrintUsage( execpath );

	//
	// Initialize the frontend and engine and their associated callbacks.
	//

	frontend = breveFrontendInit( argc, argv );

	frontend->data = breveFrontendInitData( frontend->engine );

	brEngineSetIOPath( frontend->engine, getcwd( wd, MAXPATHLEN ) );

	frontend->engine->camera->setActivateContextCallback( activateContext );
	frontend->engine->camera->_renderContextCallback = renderContext;

	frontend->engine->getLoadname = getLoadname;

	frontend->engine->getSavename = getSavename;

	frontend->engine->dialogCallback = brCLIDialogCallback;

	frontend->engine->interfaceTypeCallback = interfaceVersionCallback;

	signal( SIGINT, brCatchSignal );

	//
	// Attempt to load an offscreen Mesa buffer.
	//

	gOffscreenBuffer = ( GLubyte * )slMalloc( OSMESA_WINDOW_SIZE * OSMESA_WINDOW_SIZE * 4 * sizeof( GLubyte ) );

	if ( slLoadOSMesaPlugin( execpath ) ) {
		slFree( gOffscreenBuffer );
		gOffscreenBuffer = NULL;
	}

	frontend->engine->camera->setBounds( OSMESA_WINDOW_SIZE, OSMESA_WINDOW_SIZE );

	//
	// Load the input file and prepare the simulation.
	//

	text = slUtilReadFile( argv[ 0 ] );

	if ( !text ) {
		fprintf( stderr, "Error reading file \"%s\"\n", argv[0] );
		exit( 1 );
	}

	if ( gSimFile ) {
		if ( breveFrontendLoadSavedSimulation( frontend, text, argv[0], gSimFile ) != EC_OK )
			brQuit( frontend->engine );
	} else if ( breveFrontendLoadSimulation( frontend, text, argv[0] ) != EC_OK )
		brQuit( frontend->engine );

	slFree( text );

	nextNotify = gNotify;

	if ( gMaster )
		frontend->engine->world->startNetsimServer();

	if ( gSlave ) {
		frontend->engine->world->startNetsimSlave( gSlaveHost );
		slFree( gSlaveHost );
	}

	while ( !gShouldQuit && brEngineIterate( frontend->engine ) == EC_OK ) {
		if ( gNotify &&
		        frontend->engine->world->getAge() > nextNotify ) {
			printf( "%f seconds elapsed\n",
			        frontend->engine->world->getAge() );
			nextNotify += gNotify;
		}

		if ( gSimMax > 0.0 &&
		        frontend->engine->world->getAge() >= gSimMax ) {
			printf( "%f simulation seconds completed\n", gSimMax );
			brQuit( frontend->engine );
		}

		if ( frontend->engine->world->detectLightExposure() )
			frontend->engine->camera->detectLightExposure(
			    frontend->engine->world, OSMESA_WINDOW_SIZE, gOffscreenBuffer );
	}

	brQuit( frontend->engine );
}

void brCatchSignal( int signal ) {

	static int waiting = 0;

	char *line, staticLine[10240];

	if ( waiting )
		return;

	waiting = 1;

	brPauseTimer( frontend->engine );

	printf( "\n\nSimulation interupted.  Type a steve command, 'x' to quit, or hit enter to continue\n" );

	fflush( stdout );

#if HAVE_LIBREADLINE && HAVE_LIBHISTORY
	line = readline( "breve> " );

	if ( line && *line )
		add_history( line );

#else
	printf( "breve> " );

	fflush( stdout );

	line = fgets( staticLine, sizeof( staticLine ), stdin );

#endif

	if ( !line || *line == 'x' ) {
		brUnpauseTimer( frontend->engine );
		gShouldQuit = 1;
		waiting = 0;
		return;
	}

	if ( *line == '\n' ) {
		waiting = 0;
		return;
	}

	stRunSingleStatement(( stSteveData * )frontend->data, frontend->engine, line );

	if ( line != staticLine )
		free( line );

	brUnpauseTimer( frontend->engine );

	waiting = 0;
}

int brParseArgs( int argc, char **argv ) {
	int level, r;
	int error = 0;

	const char *optstring = "a:d:hn:r:t:vMS:";

#if HAVE_GETOPT_LONG
	while (( r = getopt_long( argc, argv, optstring, gCLIOptions, NULL ) ) != -1 )
#else
	while (( r = getopt( argc, argv, optstring ) ) != -1 )
#endif
	{

		switch ( r ) {

			case 'a':
				gSimFile = slStrdup( optarg );

				break;

			case 'd':
				level = atoi( optarg );

				printf( "debug level: %d\n", level );

				slSetDebugLevel( level );

				break;

			case 'n':
				gNotify = atof( optarg );

				if ( gNotify )
					printf( "Notification every %f seconds\n", gNotify );
				else
					printf( "Notifications disabled\n" );

				break;

			case 'r':
				srandom( atoi( optarg ) );

				printf( "random seed: %d\n", atoi( optarg ) );

				break;

			case 't':
				gSimMax = atof( optarg );

				printf( "running for %f seconds\n", gSimMax );

				break;

			case 'v':
				brPrintVersion();

				break;

			case 'M':
				gMaster = 1;

				break;

			case 'S':
				gSlaveHost = slStrdup( optarg );

				gSlave = 1;

				break;

			default:
				printf( "unknown option: '%c'\n", r );

			case 'h':
				error++;
		}
	}

	if ( error )
		brPrintUsage( argv[0] );

	return optind;
}

void brPrintUsage( const char *name ) {
	fprintf( stderr, "usage: %s [options] simulation_file\n", name );
	fprintf( stderr, "options:\n" );
	fprintf( stderr, "  -t, --terminate <seconds>  Stops the simulation after <seconds>\n" );
	fprintf( stderr, "  -n, --notify <seconds>     Prints a message every time <seconds>\n" );
	fprintf( stderr, "                             have passed\n" );
	fprintf( stderr, "  -r, --random <seed>        Sets the random seed to <seed>.\n" );
	fprintf( stderr, "  -a, --archive <xml_file>   Dearchive simulation from <xml_file>.  <xml_file>\n" );
	fprintf( stderr, "                             should be an archive of the simulation contained \n" );
	fprintf( stderr, "                             in the input file.\n" );
	fprintf( stderr, "  -v, --version              Display the current version number.\n" );
	fprintf( stderr, "  -h, --help                 Display this information.\n" );
	fprintf( stderr, "\n" );
	fprintf( stderr, "For full documentation, or to submit a bug report, visit the breve homepage:\n" );
	fprintf( stderr, "http://www.spiderland.org/breve\n\n" );
	exit( 1 );
}

void brQuit( brEngine *e ) {

	double diff, age;

	brPauseTimer( e );

	diff = e->realTime.tv_sec + ( e->realTime.tv_usec / 1000000.0 );

	age = e->world->getAge();

	if ( age != 0.0 ) {
		printf( "%f simulated seconds elapsed\n", age );
		printf( "%f real seconds elapsed\n", diff );
		printf( "%f simulated/real\n", age / diff );
	}

	brEngineFree( frontend->engine );

	breveFrontendCleanupData( frontend->data );
	breveFrontendDestroy( frontend );

	exit( 0 );
}

int brCLIDialogCallback( char *title, char *message, char *b1, char *b2 ) {
	int result;

	for ( ;; ) {
		printf( "%s\n\n%s\n[Y/N]? ", title, message );

		result = getchar();
		( void )getchar();

		switch ( result ) {

			case EOF:

			case 'n':

			case 'N':
				return 0;

			case 'y':

			case 'Y':
				return 1;
		}
	}
}

char *interfaceVersionCallback() {
	return interfaceID;
}

char *getSavename() {
	char name[MAXPATHLEN];

	printf( "filename to save: " );
	fgets( name, sizeof( name ), stdin );

	if ( *name )
		name[strlen( name ) - 1] = '\0';

	return slStrdup( name );
}

char *getLoadname() {
	char name[MAXPATHLEN];

	printf( "filename to load: " );
	fgets( name, sizeof( name ), stdin );

	if ( *name )
		name[strlen( name ) - 1] = 0;

	return slStrdup( name );
}

int activateContext() {
	return -1;
}

void renderContext( slWorld *w, slCamera *c ) {
	c->renderScene( w, 0 );
}

int slLoadOSMesaPlugin( char *execPath ) {
#ifdef MINGW
	// I'm not even going to try.
	return -1;
#else
	void *handle;
	void( *create )( unsigned char *, int );
	int( *activate )();

	std::string path = dirname( execPath );

	path += "/osmesaloader.o";

	handle = dlopen( path.c_str(), RTLD_NOW );

	if ( !handle ) return -1;

	create = ( void( * )( unsigned char*, int ) )dlsym( handle, "slOSMesaCreate" );

	if ( !create ) return -1;

	create( gOffscreenBuffer, OSMESA_WINDOW_SIZE );

	activate = ( int( * )() )dlsym( handle, "slOSMesaMakeCurrentContext" );

	if ( !activate ) return -1;

	frontend->engine->camera->setActivateContextCallback( activate );

	activate();

	slInitGL( frontend->engine->world, frontend->engine->camera );

	return 0;

#endif
}
