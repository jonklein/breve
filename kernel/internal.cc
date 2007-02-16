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

/*! \defgroup InternalFunctions The C-Style Internal Function Simulation API: the simulation API

	The functions outlined in this section comprise the C-Style
	Internal Function Simulation API through which all of the
	breve simulation functionality is accessed.

	The functions here are not simple C-functions as in the other
	APIs---they are designed for simple interaction with "steve"
	and other interpreted languages.  This is done by encapsulating
	the input arguments and return values as \ref brEval structures.

	As a result, all of the simulation API functions have the same
	function definition:

	int simulationFunction(brEval args[], brEval *target, brInstance *i)

	To call the functions in this API, you must encode the inputs in
	an array of \ref brEval structures, and decode the output from
	a \ref brEval as well.
*/

/*@{*/

#include "internal.h"

/*!
	\brief Adds an internal C-style function to the breve engine.

	This function adds an internal function to the specified namespace.
	The added function must have the function definition:
	int function(brEval *inputArgs, brEval *result, brInstance *caller);

	The 4th argument is the steve return type of the function--the
	type that the function will place into "result".

	Subsequent arguments represent the steve types that the function
	accepts as arguments, up to 16 currently.  after all the types are
	given, the final argument must be 0 to signify the end of the argument
	type list.

	So, for example, a function called "zorch" which a vector and a double
	and returns a pointer might be defined like this:

	brNewBreveCall(myNamespace, "zorch", zorchPtr, AT_POINTER, AT_VECTOR, AT_DOUBLE, 0);

	There are many examples of this in the steveFunctions*.c files.
*/

int brNewBreveCall( brNamespace *n, char *name, int( *f )( brEval *a, brEval *r, brInstance *i ), int rtype, ... ) {
	brInternalFunction *c;
	va_list ap;
	int value;

	c = new brInternalFunction;

	c->_call = f;
	c->_returnType = rtype;
	c->_name = name;

	va_start( ap, rtype );

	c->_argCount = 0;

	while ( ( value = va_arg( ap, int ) ) != 0 ) c->_argTypes[ c->_argCount++ ] = value;

	va_end( ap );

	if ( !brNamespaceStore( n, name, 0, c ) ) {
		slMessage( DEBUG_ALL, "Internal error: internal function \"%s\" redefined\n", name );
		return -1;
	}

	return 0;
}

void brFreeBreveCall( void *d ) {
	brInternalFunction *i = ( brInternalFunction* )d;
	delete i;
}

/*!
	\brief Returns a FILE pointer associated with the output log.

	Returns a FILE pointer referring to the output log.  Under command-line
	based breve implementations, this is typically stderr.  Under Mac OS X,
	however, output to this file handle will be routed through slMessage.
*/

FILE *slGetLogFilePointer( brInstance *i ) {
	return i->engine->logFile;
}

/*!
	\brief Loads internal functions into the breve engine.

	Calls all of the loader functions for different groups of functions.
*/

void brLoadInternalFunctions( brEngine *e ) {
	breveInitWorldFunctions( e->internalMethods );
	breveInitStationaryFunctions( e->internalMethods );
	breveInitObjectFunctions( e->internalMethods );
	breveInitControlFunctions( e->internalMethods );
	breveInitMathFunctions( e->internalMethods );
	breveInitMenuFunctions( e->internalMethods );
	breveInitPhysicsFunctions( e->internalMethods );
	breveInitJointFunctions( e->internalMethods );
	breveInitMultibodyFunctions( e->internalMethods );
	breveInitShapeFunctions( e->internalMethods );
	breveInitLinkFunctions( e->internalMethods );
	breveInitRealFunctions( e->internalMethods );
	breveInitNetworkFunctions( e->internalMethods );
	breveInitPatchFunctions( e->internalMethods );
	breveInitNeuralFunctions( e->internalMethods );
	breveInitNeuralNetworkFunctions( e->internalMethods );
	breveInitCameraFunctions( e->internalMethods );
	breveInitFileFunctions( e->internalMethods );
	breveInitSoundFunctions( e->internalMethods );
	breveInitGraphFunctions( e->internalMethods );
	breveInitTerrainFunctions( e->internalMethods );
	breveInitImageFunctions( e->internalMethods );
	breveInitMovieFunctions( e->internalMethods );
	breveInitPushFunctions( e->internalMethods );
	breveInitSpringFunctions( e->internalMethods );
	breveInitPushCallbackFunctions( e->internalMethods );
	breveInitMatrixFunctions( e->internalMethods );
	breveInitVectorFunctions( e->internalMethods );
	breveInitStatisticsFunctions( e->internalMethods );
	breveInitQGAMEFunctions( e->internalMethods );
	breveInitDrawFunctions( e->internalMethods );
}

/*@}*/
