#include "kernel.h"

#ifdef HAVE_LIBPERL

#include "perlInterface.h"

/**
 * Perl callback to add a Perl instance to the breve engine.
 */
brInstance *brPerlAddInstance( SV* instance ) {

	// Adding the instance requires a brObject type.
	SvREFCNT_inc(instance);
	slMessage(DEBUG_INFO, "Adding a perl object to Breve engine.\n");
	
	if(SvOK(instance)) {
		slMessage(DEBUG_INFO, "Instance %08x appears to be valid.\n", instance);
	}

	HV* package_stash =
		SvSTASH(SvRV(instance)); // finding the package stash from the blessed ref SV*s
	
	brObject *newPerlObj      = brEngineAddObject(breveEngine, brevePerlType, "foobaz", package_stash);
	brInstance *newPerlInst   = brEngineAddInstance(breveEngine, newPerlObj, (void*) instance );
	
	return newPerlInst;
}

/**
 * Perl callback to find a breve internal function. 
 */
brInternalFunction *brPerlFindInternalFunction( SV *inSelf, char *name ) {

	brInternalFunction *function = brEngineInternalFunctionLookup( breveEngine, name );

	if ( !function ) {
		slMessage(DEBUG_ALL, "Could not locate internal breve method.\n");
		return NULL;
	}

	slMessage(DEBUG_INFO, "Found internal method: %s\n\n",name);

	return function;
}

/**
 * Perl callback to call a breve internal function. 
 */

void *brPerlCallInternalFunction( SV *inSelf, void *inArgs ) {
/*	brEval args[ 128 ], resultEval;

	PyObject *callerObject, *functionObject, *arguments, *moduleObject;

	if ( !PyArg_ParseTuple( inArgs, "OOOO", &moduleObject, &callerObject, &functionObject, &arguments ) ) return NULL;

	brInternalFunction *function = ( brInternalFunction* )PyCObject_AsVoidPtr( functionObject );

	if ( !function ) {
		PyErr_SetString( PyExc_RuntimeError, "Could not execute internal breve function" );
		return NULL;
	}

	PyObject *breveObject = PyObject_GetAttrString( callerObject, "breveInstance" );

	if( !breveObject ) {
		PyErr_SetString( PyExc_RuntimeError, "Internal breve function called for Python object not in breve engine (missing call to __init__ for the parent class?)" );
		return NULL;
	}

	brInstance *caller = ( brInstance* )PyCObject_AsVoidPtr( breveObject );

	Py_DECREF( breveObject );

	if( caller->status != AS_ACTIVE ) {
		slMessage( DEBUG_ALL, "Warning: internal function called for freed instance %p\n", caller );
		return NULL;
	}

	if( function->_argCount != 0 ) {

		if( !PyTuple_Check( arguments ) ) {
			PyErr_SetString( PyExc_RuntimeError, "Invalid arguments passed to internal breve function" );
			return NULL;
		}

		int argCount = PyTuple_GET_SIZE( arguments );

		if( argCount != function->_argCount ) {
			char err[ 1024 ];
			snprintf( err, 1023, "internal function \"%s\" given %d arguments, expects %d", function->_name.c_str(), argCount, function->_argCount );
			PyErr_SetString( PyExc_RuntimeError, err );
			return NULL;
		}

		for( int n = 0; n < argCount; n++ ) {
			brPythonTypeToEval( PyTuple_GET_ITEM( arguments, n ), &args[ n ] );

			if( args[ n ].type() != function->_argTypes[ n ] ) {

				// int <=> double conversion 

				if( args[ n ].type() == AT_INT && function->_argTypes[ n ] == AT_DOUBLE ) {
					 args[ n ].set( (double)BRINT( &args[ n ] ) );
				} else if( args[ n ].type() == AT_DOUBLE && function->_argTypes[ n ] == AT_INT) {
					 args[ n ].set( (int)BRDOUBLE( &args[ n ] ) );
				} else {
					char err[ 1024 ];
					snprintf( err, 1023, "invalid type for argument %d of internal function \"%s\" (got \"%s\", expected \"%s\")", n, function->_name.c_str(), brAtomicTypeStrings[ args[ n ].type() ], brAtomicTypeStrings[ function->_argTypes[ n ] ] );

					PyErr_SetString( PyExc_RuntimeError, err );
					return NULL;
				}
			}
		}
	}

	function->_call( args, &resultEval, caller );

	return brPythonTypeFromEval( &resultEval, moduleObject );*/
	return NULL;
}


void *brPerlSetController( SV *controller ) {
	slMessage(DEBUG_ALL,"brPerlSetController() perl invocation.\n");

   brEngineSetController(breveEngine, brPerlAddInstance(controller));

	SvREFCNT_inc(controller);

	return NULL;
}

#endif
