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

#include <sys/types.h>

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "slutil.h"

static int gDebugLevel;

static void( *gMessageOutputFunction )( const char * ) = slStdoutMessageCallback;
static void( *gErrorOutputFunction )( const char * ) = slStderrMessageCallback;

void slSetDebugLevel( int level ) {
	gDebugLevel = level;
}

void slSetMessageCallbackFunctions( void( out )( const char * ), void( err )( const char * ) ) {
	gMessageOutputFunction = out;
	gErrorOutputFunction   = err;
}

void slSetMessageCallbackFunction( void( f )( const char * ) ) {
	gMessageOutputFunction = f;
	gErrorOutputFunction   = f;
}

void slStderrMessageCallback( const char *string ) {
	fprintf( stderr, "%s", string );
	fflush( stderr );
}

void slStdoutMessageCallback( const char *string ) {
	fprintf( stdout, "%s", string );
	fflush( stdout );
}

/*!
	\brief Prints a fatal error to stderr and quits.

	slFatal is an exception to the normal error handling rules.
	Regardless of the message-output callback function, we'll
	print the error to stderr before quitting.
*/

void slFatal( char *format, ... ) {
	va_list vp;

	fprintf( stderr, "Fatal error: " );
	va_start( vp, format );
	vfprintf( stderr, format, vp );
	va_end( vp );

	fprintf( stderr, "[hit enter to exit]\n" );
	getchar();

	exit( 0 );
}

void slFormattedMessage( int level, const char *format ) {
	if( level > gDebugLevel )
		return;

	if( level <= 0 ) {
		if( gMessageOutputFunction ) gMessageOutputFunction( format );
	} else {
		if( gErrorOutputFunction ) gErrorOutputFunction( format );
	}
}

void slMessage( int level, const char *format, ... ) {
	if ( level > gDebugLevel )
		return;

	va_list vp;

	char *queueMessage;

	va_start( vp, format );

#if HAVE_VASPRINTF
	if ( vasprintf( &queueMessage, format, vp ) == -1 )
		return;

#else
	const size_t len = 1024 + strlen( format ) * 10;

	queueMessage = ( char * )malloc( len );

	vsnprintf( queueMessage, len, format, vp );

#endif
	va_end( vp );

	if( level <= 0 ) {
		if( gMessageOutputFunction ) gMessageOutputFunction( queueMessage );
	} else {
		if( gErrorOutputFunction ) gErrorOutputFunction( queueMessage );
	}

	free( queueMessage );
}
