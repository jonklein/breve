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

/**
 * The GLUT-based breve application.
 * This is a wrapper around the breve gEngine for the GLUT-based
 * steve-frontend version of the breve simulation environment.
 */

#include <signal.h>

#include "simulation.h"
#include "steve.h"
#include "breve.h"
#include "world.h"
#include "movie.h"
#include "gldraw.h"

#include "pyconvert.h"

static void brInterrupt( brEngine * );
static void *workerThread( void * );

static void brCatchSignal( int signal );

#if MINGW
extern char *optarg;
extern int optind;
#endif

static brEngine *gEngine;
static int contextMenu, mainMenu;
static int height = 400, width = 400;
static int gXpos = 0, gYpos = 0;
static stSteveData *gSteveData;
static char keyDown[256];
static char specialDown[256];
static int gPaused = 1;
static int gWaiting = 0;
static int gThreadShouldExit = 0;

static pthread_mutex_t gEngineMutex;
static pthread_mutex_t gThreadMutex;
static pthread_cond_t gThreadPaused;

static char *gOptionArchiveFile = NULL;

static int gConvertToPython = 0;
static int gOptionFormat = 0;
static int gOptionFull = 0;
static int gOptionStdin = 0;
static int gMotionCrosshair = 0;
static int gOptionNoGraphics = 0;

static int gLastX, gLastY, gMods, gSpecial;
static double gStartCamX;

static brInstance *gSelected = NULL;

static int gSlave = 0, gMaster = 0;
static char *gSlaveHost;

static std::map<int, void*> gWindowMap;

int main( int argc, char **argv ) {
	pthread_t thread;
	char *text, *simulationFile;
	int index;
	char wd[MAXPATHLEN];

	interfaceID = "glut/2.6";

	srandom( time( NULL ) );

#if MINGW
	pthread_win32_process_attach_np();
#endif

	pthread_mutex_init( &gEngineMutex, NULL );
	pthread_mutex_init( &gThreadMutex, NULL );
	pthread_cond_init( &gThreadPaused, NULL );

	/* parse the command line args. */

	index = brParseArgs( argc, argv );

	char *execpath = argv[ 0 ];

	// offset argc and argv to account for the options parsed out

	argc -= index;
	argv += index;

	if ( !gOptionStdin ) {
		if ( argc < 1 ) brPrintUsage( execpath );

		text = slUtilReadFile( argv[0] );

		simulationFile = argv[0];
	} else {
		text = slUtilReadStdin();
		simulationFile = "<stdin>";
	}

	if ( !text ) {
		printf( "Error reading file \"%s\": %s\n", simulationFile, strerror( errno ) );
		exit( 1 );
	}

	if ( gOptionFormat ) {
		char *newtext = slFormatText( text );
		slUtilWriteFile( simulationFile, newtext );
		slFree( newtext );
		exit( 0 );
	}

	gEngine = brEngineNewWithArguments( argc, argv );

	brEngineSetIOPath( gEngine, getcwd( wd, sizeof( wd ) ) );

	gSteveData = (stSteveData*)brInitFrontendLanguages( gEngine );

	gEngine->getLoadname 		= getLoadname;
	gEngine->getSavename 		= getSavename;
	gEngine->soundCallback 		= soundCallback;
	gEngine->dialogCallback 	= brCLIDialogCallback;
	gEngine->interfaceTypeCallback	= interfaceVersionCallback;
	gEngine->pauseCallback 		= pauseCallback;

	if ( !gOptionNoGraphics ) 
		slInitGlut( argc, argv, simulationFile );



	gEngine->camera->setBounds( width, height );



	if( gConvertToPython ) {
		brLoadSimulation( gEngine, text, simulationFile );
		std::string pyf = simulationFile;
		std::string plf = simulationFile;
		std::string pyc = stPyConvertFile( gEngine, gSteveData, pyf );
		std::string plc = stPerlConvertFile( gEngine, gSteveData, plf );

		int position = plf.find( ".tz", plf.size() - 3 );

		if( position > 0 ) {
			pyf.replace( position, 3, ".py", 3 );
			plf.replace( position, 3, ".pm", 3 );
		} else {
			pyf += ".py";
			plf += ".pm";
		}

		FILE *fp = fopen( pyf.c_str(), "w" );

		if( !fp ) {
			fprintf( stderr, "Cannot open file \"%s\" for writing\n", pyf.c_str() );
			exit( 1 );
		}

		fprintf( fp, "%s\n", pyc.c_str() );

		fclose( fp );

		fp = fopen( plf.c_str(), "w" );

		if( !fp ) {
			fprintf( stderr, "Cannot open file \"%s\" for writing\n", plf.c_str() );
			exit( 1 );
		}

		fprintf( fp, "%s\n", plc.c_str() );

		exit( 0 );
	}


	if ( gOptionArchiveFile ) {
		if ( brLoadSavedSimulation( gEngine, text, simulationFile, gOptionArchiveFile ) != EC_OK )
			brQuit( gEngine );
	} else if ( brLoadSimulation( gEngine, text, simulationFile ) != EC_OK )
		brQuit( gEngine );





	slFree( text );

	brEngineSetUpdateMenuCallback( gEngine, brGlutMenuUpdate );

	memset( keyDown, 0, sizeof( keyDown ) );

	memset( specialDown, 0, sizeof( specialDown ) );

	pthread_mutex_lock( &gThreadMutex );

	pthread_create( &thread, NULL, workerThread, NULL );

	if ( gMaster )
		gEngine->world->startNetsimServer();

	if ( gSlave ) {
		gEngine->world->startNetsimSlave( gSlaveHost );
		slFree( gSlaveHost );
	}

	if ( gOptionFull ) glutFullScreen();

	if ( gOptionNoGraphics ) {
#ifndef WINDOWS
        	signal( SIGINT, brCatchSignal );
#endif

		gPaused = 0;

		while ( 1 ) 
			brGlutLoop();
	}

	glutMainLoop();

	return 0;
}

void *workerThread( void *data ) {

	for ( ;; ) {
		pthread_mutex_lock( &gThreadMutex );

		if (( !gPaused && !gEngine->drawEveryFrame ) ||
		        !pthread_cond_wait( &gThreadPaused, &gThreadMutex ) ) {
			if ( gThreadShouldExit ) {
				pthread_mutex_unlock( &gThreadMutex );
				return NULL;
			} else if ( brEngineIterate( gEngine ) != EC_OK ) {
				pthread_mutex_unlock( &gThreadMutex );
				brQuit( gEngine );
			}
		}

		pthread_mutex_unlock( &gThreadMutex );
	}
}

void brGlutLoop() {
	static int oldD = 1;

	int newD = gEngine->drawEveryFrame;

	gWaiting = 0;

	if ( !gPaused ) {
		if ( newD && brEngineIterate( gEngine ) != EC_OK )
			brQuit( gEngine );

		if ( !gOptionNoGraphics ) 
			glutPostRedisplay();

		if ( !newD && oldD ) {
			pthread_mutex_unlock( &gThreadMutex );
			pthread_cond_signal( &gThreadPaused );
		} else if ( newD && !oldD )
			pthread_mutex_lock( &gThreadMutex );

//                if ( gEngine->world->detectLightExposure() )
//			gEngine->camera->detectLightExposure(
//			gEngine->world, OSMESA_WINDOW_SIZE, gOffscreenBuffer );

		oldD = newD;
	}
}

void brGlutMenuUpdate( brInstance *i ) {
	brMenuEntry *menu;
	char *message;
	unsigned int n, total;

	if ( i != gEngine->controller )
		return;

	glutSetMenu( mainMenu );

	total = glutGet( GLUT_MENU_NUM_ITEMS );

	for ( n = 0; n < total; ++n )
		glutRemoveMenuItem( 1 );

	for ( n = 0; n < i->_menus.size() ; ++n ) {
		menu = i->_menus[ n ];

		if ( menu->enabled ) {
			message = new char[strlen( menu->title ) + 4];

			if ( menu->checked )
				sprintf( message, "* %s", menu->title );
			else
				sprintf( message, "  %s", menu->title );

			glutAddMenuEntry( message, n );

			delete[] message;
		}
		else glutAddMenuEntry( " ", n );
	}
}

void brQuit( brEngine *e ) {
	double diff, age;

	if ( !gPaused && !gEngine->drawEveryFrame ) {
		gThreadShouldExit = 1;
		pthread_mutex_lock( &gThreadMutex );
	}

	brPauseTimer( e );

	diff = e->realTime.tv_sec + ( e->realTime.tv_usec / 1000000.0 );

	age = e->world->getAge();

	if ( age != 0.0 ) {
		printf( "%f simulated seconds elapsed\n", age );
		printf( "%f real seconds elapsed\n", diff );
		printf( "%f simulated/real\n", age / diff );
	}

	brEngineFree( gEngine );

	exit( 0 );
}

void brMainMenu( int n ) {
	brMenuCallback( gEngine, gEngine->controller, n );
	glutPostRedisplay();
}

void brContextMenu( int n ) {
	if ( gSelected )
		brMenuCallback( gEngine, gSelected, n );

	glutPostRedisplay();
}

void brClick( int n ) {
	unsigned int m, total;

	gSelected = brClickCallback( gEngine, n );

	glutSetMenu( contextMenu );

	total = glutGet( GLUT_MENU_NUM_ITEMS );

	for ( m = 0; m < total; ++m )
		glutRemoveMenuItem( 1 );

	if ( gSelected ) {
		brMenuEntry *menu;

		for ( m = 0; m < gSelected->_menus.size(); ++m ) {
			menu = gSelected->_menus[ m ];
			glutAddMenuEntry( menu->title, m );
		}
	} else
		glutSetMenu( mainMenu );

	glutAttachMenu( GLUT_RIGHT_BUTTON );
}

int brParseArgs( int argc, char **argv ) {
	int level, r;
	int error = 0;

	const char *name = argv[0];
	const char *optstring = "a:d:fhip:r:s:xuvFMS:Y";

#if HAVE_GETOPT_LONG
	while (( r = getopt_long( argc, argv, optstring, gCLIOptions, NULL ) ) != -1 )
#else
	while (( r = getopt( argc, argv, optstring ) ) != -1 )
#endif
	{

		switch ( r ) {

			case 'a':
				gOptionArchiveFile = slStrdup( optarg );

				break;

			case 'd':
				level = atoi( optarg );

				printf( "debug level: %d\n", level );

				slSetDebugLevel( level );

				break;

			case 'f':
				gOptionFull = 1;

				break;

			case 'i':
				gOptionStdin = 1;

				break;

			case 'p':
				sscanf( optarg, "%d,%d", &gXpos, &gYpos );

				break;

			case 'r':
				srandom( atoi( optarg ) );

				printf( "random seed: %d\n", atoi( optarg ) );

				break;

			case 's':
				sscanf( optarg, "%dx%d\n", &width, &height );

				if ( width <= 0 || width > 1000 ||
				        height <= 0 || height > 1000 ) {
					printf( "-s size flag must be in the form NxM, \
					        where N and M are from 1 to 1000\n" );
					error++;
				}

				break;

			case 'u':
				gPaused = 0;

				break;

			case 'v':
				brPrintVersion();

				break;

			case 'F':
				gOptionFormat = 1;

				break;

			case 'M':
				gMaster = 1;

				break;

			case 'S':
				gSlaveHost = slStrdup( optarg );

				gSlave = 1;

				break;

			case 'x':
				gOptionNoGraphics = 1;
				break;

			case 'Y':	
				gConvertToPython = 1;
				break;

			default:
				printf( "unknown option: '%c'\n", r );

			case 'h':
				error++;

				break;
		}
	}

	if ( error ) brPrintUsage( name );

	return optind;
}

void brPrintUsage( const char *name ) {
	fprintf( stderr, "usage: %s [options] simulation_file\n", name );
	fprintf( stderr, "options:\n" );
	fprintf( stderr, "  -x              Run the simulation without graphical display.\n" );
	fprintf( stderr, "  -r <seed>       Sets the random seed to <seed>.\n" );
	fprintf( stderr, "  -a <xml_file>   Dearchive simulation from <xml_file>.  <xml_file> should be\n" );
	fprintf( stderr, "                  an archive of the simulation contained in the input file.\n" );
	fprintf( stderr, "  -f              Start breve in fullscreen mode.\n" );
	fprintf( stderr, "  -u              Unpause: begin running simulation immediately.\n" );
	fprintf( stderr, "  -F              Format the simulation file and quit.\n" );
	fprintf( stderr, "  -p X,Y          Move the window to coordinates (X, Y).\n" );
	fprintf( stderr, "  -s NxM          Create a window of size (N, M).\n" );
	fprintf( stderr, "  -v              Display the current version number.\n" );
	fprintf( stderr, "  -h              Display this information.\n" );
	fprintf( stderr, "\n" );
	fprintf( stderr, "For full documentation, or to submit a bug report, visit the breve homepage:\n" );
	fprintf( stderr, "http://www.spiderland.org/breve\n\n" );
	exit( 1 );
}

void slInitGlut( int argc, char **argv, char *title ) {

	glutInitWindowSize( width, height );
	glutInit( &argc, argv );
	glutInitDisplayMode( GLUT_DOUBLE | GLUT_RGBA | GLUT_DEPTH | GLUT_STENCIL );

	if ( gXpos || gYpos ) 
		glutInitWindowPosition( gXpos, gYpos );

	glutCreateWindow( title );
	glutMouseFunc( slDemoMouse );
	glutMotionFunc( slDemoMotion );
	glutPassiveMotionFunc( slDemoPassiveMotion );
	glutDisplayFunc( slDemoDisplay );
	glutSpecialFunc( slDemoSpecial );
	glutSpecialUpFunc( slDemoSpecialUp );
	glutReshapeFunc( slDemoReshape );
	glutKeyboardUpFunc( slDemoKeyboardUp );
	glutKeyboardFunc( slDemoKeyboard );

	mainMenu = glutCreateMenu( brMainMenu );

	glutAttachMenu( GLUT_RIGHT_BUTTON );

	contextMenu = glutCreateMenu( brContextMenu );

	glutAttachMenu( GLUT_MIDDLE_BUTTON );

	slInitGL( gEngine->world, gEngine->camera );

	glutIgnoreKeyRepeat( 1 );

	glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );

	if ( !gPaused ) glutIdleFunc( brGlutLoop );
}

void slDemoReshape( int x, int y ) {
	gEngine->camera->setBounds( x, y );

	glViewport( 0, 0, x, y );
	glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
}

void slDemoDisplay() {
	brEngineLock( gEngine );
	brEngineRenderWorld( gEngine, gMotionCrosshair );
	brEngineUnlock( gEngine );

	glutSwapBuffers();
}

void slDemoMouse( int button, int state, int x, int y ) {
	gMods = glutGetModifiers();

	if ( state == GLUT_DOWN ) {
		gLastX = x;
		gLastY = y;
		gStartCamX = gEngine->camera->_rx;

		brClick( gEngine->camera->select( gEngine->world, x, y ) );
		brGlutMenuUpdate( gEngine->controller );

		gMotionCrosshair = 1;

		// if(gMods & GLUT_ACTIVE_SHIFT) brBeginDrag( gEngine, gSelected );

	} else {
		gLastX = 0;
		gLastY = 0;
		gMotionCrosshair = 0;
	}

	if( state == GLUT_UP ) { 
		// mouse wheel 
		if ( button == 3 )
			gEngine->camera->zoomWithMouseMovement( 0, 15 );
		else if ( button == 4 )
			gEngine->camera->zoomWithMouseMovement( 0, -15 );
	}

	slDemoDisplay();
}

void slDemoPassiveMotion( int x, int y ) {
	gEngine->mouseX = x;
	gEngine->mouseY = gEngine->camera->_height - y;
}

void slDemoMotion( int x, int y ) {
	if ( gLastX || gLastY ) {
		if (( gMods & GLUT_ACTIVE_SHIFT ) || ( gSpecial == GLUT_KEY_F4 ) ) {
			brDragCallback( gEngine, x, y );
		} else if (( gMods & GLUT_ACTIVE_ALT ) || ( gSpecial == GLUT_KEY_F2 ) ) {
			gEngine->camera->zoomWithMouseMovement( x - gLastX, y - gLastY );
		} else if (( gMods & GLUT_ACTIVE_CTRL ) || ( gSpecial == GLUT_KEY_F3 ) ) {
			gEngine->camera->moveWithMouseMovement( x - gLastX, y - gLastY );
		} else {
			gEngine->camera->rotateWithMouseMovement( x - gLastX, y - gLastY );
		}

		gLastX = x;

		gLastY = y;
	}

	glutPostRedisplay();
}

void slDemoSpecial( int key, int x, int y ) {
	gSpecial = key;

	if ( specialDown[key] ) return;

	specialDown[key] = 1;

	switch ( key ) {

		case GLUT_KEY_F1:
			if ( brEngineIterate( gEngine ) != EC_OK )
				brQuit( gEngine );

			break;

		case GLUT_KEY_UP:
			brSpecialKeyCallback( gEngine, "up", 1 );
			break;

		case GLUT_KEY_DOWN:
			brSpecialKeyCallback( gEngine, "down", 1 );
			break;

		case GLUT_KEY_LEFT:
			brSpecialKeyCallback( gEngine, "left", 1 );
			break;

		case GLUT_KEY_RIGHT:
			brSpecialKeyCallback( gEngine, "right", 1 );
			break;
	}

	glutPostRedisplay();
}

void slDemoSpecialUp( int key, int x, int y ) {
	specialDown[key] = 0;

	switch ( key ) {

		case GLUT_KEY_UP:
			brSpecialKeyCallback( gEngine, "up", 0 );

			break;

		case GLUT_KEY_DOWN:
			brSpecialKeyCallback( gEngine, "down", 0 );

			break;

		case GLUT_KEY_LEFT:
			brSpecialKeyCallback( gEngine, "left", 0 );

			break;

		case GLUT_KEY_RIGHT:
			brSpecialKeyCallback( gEngine, "right", 0 );

			break;
	}

	gSpecial = 0;
}

void slDemoKeyboard( unsigned char key, int x, int y ) {
	if ( keyDown[key] ) return;

	keyDown[key] = 1;

	switch ( key ) {

		case ' ':
			gPaused = !gPaused;

			if ( gPaused ) {
				brPauseTimer( gEngine );
				glutIdleFunc( NULL );
			} else {
				brUnpauseTimer( gEngine );
				pthread_cond_signal( &gThreadPaused );
				glutIdleFunc( brGlutLoop );
			}

			break;

		case 0x1b:
			if ( !gWaiting )
				brInterrupt( gEngine );

			break;

		default:
			brKeyCallback( gEngine, key, 1 );

			break;
	}
}


void brCatchSignal( int signal ) {
	brInterrupt( gEngine );
}

void brInterrupt( brEngine *gEngine ) {
	char *line;
	char staticLine[MAXPATHLEN];

	gWaiting = 1;

	if ( gOptionFull ) {
		glutReshapeWindow( 400, 400 );
		gOptionFull = 0;
		return;
	}

	brPauseTimer( gEngine );

	fflush( stdin );
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

	if ( !line || *line == 'x' )
		brQuit( gEngine );

	if ( *line != '\0' && *line != '\n' )
		stRunSingleStatement( gSteveData, gEngine, line );

	if ( line != staticLine )
		free( line );

	if ( gOptionFull )
		glutFullScreen();

	brUnpauseTimer( gEngine );
}

/*!
	\brief The GLUT key-up callback.  Calls \ref brKeyCallback.
*/

void slDemoKeyboardUp( unsigned char key, int x, int y ) {
	brKeyCallback( gEngine, key, 0 );
	keyDown[key] = 0;
}

/*!
	\brief A lame way to get a yes or no from the user.

	This would normally be done with a dialog box, but
	this is GLUT and we don't have one...
*/

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
		name[strlen( name ) - 1] = 0;

	return slStrdup( name );
}

char *getLoadname() {
	char name[MAXPATHLEN];

	printf( "filename to load: " );
	fgets( name, sizeof( name ), stdin );

	if ( *name )
		name[strlen( name ) - 1] = '\0';

	return slStrdup( name );
}

int soundCallback() {
	putchar( 0x07 );
	fflush( stdout );
	return 0;
}

int pauseCallback() {
	gPaused = 1;
	brPauseTimer( gEngine );
	glutIdleFunc( NULL );
	return 0;
}

