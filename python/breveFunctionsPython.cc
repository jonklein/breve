/*****************************************************************************
 *                                                                           *
 * The breve Simulation Environment                                          *
 * Copyright (C) 2000-2006 Jonathan Klein                                    *
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

#ifdef HAVE_LIBPYTHON

#include <python2.3/Python.h>

int brPythonLoadFile( brEval arguments[], brEval *result, brInstance *instance ) {
	char *file = brFindFile( instance->engine, BRSTRING( &arguments[ 0 ] ), NULL );
	char import[ 10240 ];

	if( !file ) {
		throw slException( "cannot locate file \"" + std::string( BRSTRING( &arguments[ 0 ] ) ) + "\"" );
	}

	snprintf( import, 10239, "execfile( '''%s''' )", file );

	result->set( PyRun_SimpleString( import ) );

	slFree( file );

	if( BRINT( result ) != 0 ) {
		slMessage( DEBUG_ALL, "Python load of file \"%s\" failed\n", file );
		return EC_ERROR;
	}

	return EC_OK;
}

int brPythonInternalTest( brEval arguments[], brEval *result, brInstance *instance ) {
	printf( "Internal test function called\n" );

	return EC_OK;
}

#endif

void breveInitPythonFunctions( brNamespace *n ) {
#ifdef HAVE_LIBPYTHON
	brNewBreveCall( n, "pythonLoadFile", brPythonLoadFile, AT_INT, AT_STRING, 0 );
	brNewBreveCall( n, "pythonInternalTest", brPythonInternalTest, AT_INT, AT_STRING, 0 );
#endif
}
