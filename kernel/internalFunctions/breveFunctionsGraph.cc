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

/*! \addtogroup InternalFunctions */
/*@{*/

#include "kernel.h"
#include "graph.h"

#define BRGRAPHPOINTER(p)	((slGraph*)BRPOINTER(p))

/*!
	\brief Creates a new graph.

	void graphNew().
*/

int brIGraphNew( brEval args[], brEval *target, brInstance *i ) {
	slVector color;
	slGraph *graph;
	void *p;

	slVectorSet( &color, .8, .8, 1 );

	graph = slGraphNew( &color, 0, 0, 200, 200 );

	if ( !i->engine->newWindowCallback ) {
		slMessage( DEBUG_ALL, "cannot create new window\n" );

		target->set(( void* )NULL );

		return EC_OK;
	}

	p = i->engine->newWindowCallback( BRSTRING( &args[0] ), graph );

	if ( !p ) {
		slMessage( DEBUG_ALL, "warning: new graph window callback failed--graphs not available for this interface\n" );
	} else
		i->engine->windows.push_back( p );

	target->set( graph );

	graph->userData = p;

	return EC_OK;
}

/*!
	\brief Sets the graph's title.

	void graphSetTitle(slGraph pointer, string).
*/

int brIGraphSetTitle( brEval args[], brEval *target, brInstance *i ) {
	slGraph *g = BRGRAPHPOINTER( &args[0] );
	char *string = BRSTRING( &args[1] );

	if ( g )
		slGraphSetTitle( g, string );

	return EC_OK;
}

/*!
	\brief Sets the graph's x-axis name.

	void graphSetXAxisName(slGraph pointer, string).
*/

int brIGraphSetXAxisName( brEval args[], brEval *target, brInstance *i ) {
	slGraph *g = BRGRAPHPOINTER( &args[0] );
	char *string = BRSTRING( &args[1] );

	if ( g )
		slGraphSetXAxisName( g, string );

	return EC_OK;
}

/*!
	\brief Sets the graph's y-axis name.

	void graphSetYAxisName(slGraph pointer, string).
*/

int brIGraphSetYAxisName( brEval args[], brEval *target, brInstance *i ) {
	slGraph *g = BRGRAPHPOINTER( &args[0] );
	char *string = BRSTRING( &args[1] );

	if ( g )
		slGraphSetYAxisName( g, string );

	return EC_OK;
}

/*!
	\brief Adds a line to a graph.

	int graphAddLine(slGraph pointer, vector).

	Adds a new line to the graph with the given color.  Returns an
	integer identifying the line.
*/

int brIGraphAddLine( brEval args[], brEval *target, brInstance *i ) {
	slGraph *g = BRGRAPHPOINTER( &args[0] );
	slVector *color = &BRVECTOR( &args[1] );

	if ( g ) target->set(( void* )slGraphAddLine( g, color ) );

	return EC_OK;
}

/*!
	\brief Adds a value to the given line in the given graph.

	void graphAddLineValue(slGraph pointer, int, double).
*/

int brIGraphAddLineValue( brEval args[], brEval *target, brInstance *i ) {
	slGraph *g = BRGRAPHPOINTER( &args[0] );
	unsigned int line = BRINT( &args[1] );
	double yValue = BRDOUBLE( &args[2] );

	if ( !g )
		return EC_OK;

	if ( line >= g->lines.size() ) {
		/* error message */
		return EC_OK;
	}

	slGraphAddLineValue( g, line, g->lines[line]->lastX + 1, yValue );

	i->engine->renderWindowCallback( g->userData );

	return EC_OK;
}

/*@}*/

void breveInitGraphFunctions( brNamespace *n ) {
	brNewBreveCall( n, "graphNew", brIGraphNew, AT_POINTER, AT_STRING, 0 );
	brNewBreveCall( n, "graphSetTitle", brIGraphSetTitle, AT_NULL, AT_POINTER, AT_STRING, 0 );
	brNewBreveCall( n, "graphSetXAxisName", brIGraphSetXAxisName, AT_NULL, AT_POINTER, AT_STRING, 0 );
	brNewBreveCall( n, "graphSetYAxisName", brIGraphSetYAxisName, AT_NULL, AT_POINTER, AT_STRING, 0 );
	brNewBreveCall( n, "graphAddLine", brIGraphAddLine, AT_INT, AT_POINTER, AT_VECTOR, 0 );
	brNewBreveCall( n, "graphAddLineValue", brIGraphAddLineValue, AT_NULL, AT_POINTER, AT_INT, AT_DOUBLE, 0 );
}
