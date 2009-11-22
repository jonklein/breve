
/*****************************************************************************
 * The breve Simulation Environment                                          *
 * Copyright (C) 2000-2004 Jonathan Klein                                    *
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
#include "url.h"

/*! \addtogroup InternalFunctions */
/*@{*/

/**
 * brIURLGet( char *url )
 */

int brIURLGet( brEval args[], brEval *target, brInstance *i ) {
	int result = EC_ERROR;

#ifdef HAVE_LIBCURL
	brEngine *e = i -> engine;
	char *data;
	int dataSize;

	e -> _url -> get( BRSTRING( &args[ 0 ] ), &data, &dataSize );

	if( data ) {
		target -> set( data );
		slFree( data );
	
		result = EC_OK;
	}
#endif

	return result;
}

int brIURLPut( brEval args[], brEval *target, brInstance *i ) {
	int result = EC_ERROR;

#ifdef HAVE_LIBCURL
	brEngine *e = i -> engine;
	char *data;
	int dataSize;

	char *putData = BRSTRING( &args[ 1 ] );

	e -> _url -> put( BRSTRING( &args[ 0 ] ), putData, strlen( putData ), &data, &dataSize );

	if( data ) {
		target -> set( data );
		slFree( data );
	
		result = EC_OK;
	}
#endif

	return result;
}

/*@}*/




void breveInitURLFunctions( brNamespace *n ) {
	brNewBreveCall( n, "brIURLGet", brIURLGet, AT_STRING, AT_STRING, 0 );
	brNewBreveCall( n, "brIURLPut", brIURLPut, AT_STRING, AT_STRING, AT_STRING, 0 );
}
