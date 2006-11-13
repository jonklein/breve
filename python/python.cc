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
#include "python.h"

/**
 * Translation function from a Python object to a breve value.
 * 
 * @param inObject		The Python object to be translated.
 * @param outEval		The breve value output.
 * @return				Either EC_OK or EC_ERROR indicating the success of the translation
 */

inline int brPythonTypeToEval( PyObject *inObject, brEval *outEval ) {
	int result = EC_ERROR;

	if( !inObject ) return result;

	if ( PyInt_Check( inObject ) ) {

		outEval->set( PyInt_AS_LONG( inObject ) );
		result = EC_OK;

	} else if ( PyFloat_Check( inObject ) ) {

		outEval->set( PyFloat_AS_DOUBLE( inObject ) );
		result = EC_OK;

	} else if ( PyString_Check( inObject ) ) {

		outEval->set( PyString_AS_STRING( inObject ) );
		result = EC_OK;

	} else if( PyCObject_Check( inObject ) ) {

		outEval->set( PyCObject_AsVoidPtr( inObject ) );
		result = EC_OK;

	} else if( PySequence_Check( inObject ) ) {
		
		brEvalListHead *list = new brEvalListHead;

		int size = PySequence_Size( inObject );

		for( int n = 0; n < size; n++ ) {
			brEval item;

			brPythonTypeToEval( PySequence_GetItem( inObject, n ), &item );

			brEvalListInsert( list, list->_vector.size(), &item );
		}

		outEval->set( list );
		result = EC_OK;
	}

	return result;
}

/**
 * Translation function from a breve value to a Python object.
 * 
 * @param inEval		The breve value to be translated.
 * @return				A newly created Python object translated from the source value.
 */

inline PyObject *brPythonTypeFromEval( brEval *inEval, PyObject *inBridgeObject ) {
	PyObject *result = NULL;
	brInstance *breveInstance;

	switch ( inEval->type() ) {

		case AT_NULL:
			result = Py_None;
			Py_INCREF( result );
			break;

		case AT_INT:
			result = PyInt_FromLong( BRINT( inEval ) );

			break;

		case AT_DOUBLE:
			result = PyFloat_FromDouble( BRDOUBLE( inEval ) );

			break;

		case AT_STRING:
			result = PyString_FromString( BRSTRING( inEval ) );

			break;

		case AT_INSTANCE:
			breveInstance = BRINSTANCE( inEval );

			// Is this a native type, or should we make a bridge of it?

			if( breveInstance->object->type->_typeSignature == PYTHON_TYPE_SIGNATURE ) {
				result = (PyObject*)breveInstance->userData;
			} else {
				// Create a bridge object, and set the breveInstance field

				result = PyObject_Call( inBridgeObject, PyTuple_New( 0 ), NULL );
				PyObject_SetAttrString( result, "breveInstance", PyCObject_FromVoidPtr( breveInstance, NULL ) );
			}

			break;

		case AT_POINTER:
			result = PyCObject_FromVoidPtr( BRPOINTER( inEval ), NULL );
			break;

		case AT_VECTOR:
			break;

		case AT_MATRIX:
			break;

		case AT_LIST:
			break;

		case AT_ARRAY:
			break;

		case AT_DATA:
			break;

		case AT_HASH:
			break;

		default:
			slMessage( DEBUG_ALL, "Could not convert breve internal type \"%d\" to a Python type\n", inEval->type() );

			break;
	}

	return result;
}

PyObject *brPythonCatchOutput( PyObject *inSelf, PyObject *inArgs ) {
	PyObject *moduleObject;
	char *message;

	if ( !PyArg_ParseTuple( inArgs, "Os", &moduleObject, &message ) ) return NULL;

	slFormattedMessage( DEBUG_ALL, message );

	Py_INCREF( Py_None );
	return Py_None;
}

/**
 * Python callback to add a Python instance to the breve engine.
 */

PyObject *brPythonAddInstance( PyObject *inSelf, PyObject *inArgs ) {
	PyObject *object, *moduleObject, *typeObject, *result;
	brObject *breveObject;


	if ( !PyArg_ParseTuple( inArgs, "OOO", &moduleObject, &typeObject, &object ) ) return NULL;

	PyObject *engineObject = PyObject_GetAttrString( moduleObject, "breveEngine" );

	if( !engineObject ) {
		PyErr_SetString( PyExc_RuntimeError, "Could not locate breve engine in first internal breve call argument" );
		return NULL;
	}

	brEngine *engine = ( brEngine* )PyCObject_AsVoidPtr( engineObject );

	if( PyObject_HasAttrString( typeObject, "breveObject" ) ) {
		// Found an existing brObject type for this Python class

		PyObject *breveTypeObject = PyObject_GetAttrString( typeObject, "breveObject" );

		breveObject = (brObject*)PyCObject_AsVoidPtr( breveTypeObject );

	} else {
		// Clear the missing breveObject error here...

		PyObject *nameObject = PyObject_GetAttrString( typeObject, "__name__" );

		if( !nameObject || !PyString_Check( nameObject ) ) {
			PyErr_SetString( PyExc_RuntimeError, "Internal error while adding breve class type for Python object" );
			return NULL;
		}

		char *name = PyString_AsString( nameObject );

		// This object type has not previously been added to the breve engine.

		PyObject *pythonLanguageType = PyObject_GetAttrString( moduleObject, "breveObjectType" );

		breveObject = brEngineAddObject( engine, (brObjectType*)PyCObject_AsVoidPtr( pythonLanguageType ), name, typeObject );

		PyObject_SetAttrString( typeObject, "breveObject", PyCObject_FromVoidPtr( breveObject, NULL ) );

	}

	brInstance *i = brEngineAddInstance( engine, breveObject, object );

	if( i ) 
		result = PyCObject_FromVoidPtr( i, NULL );
	else
		result = Py_None;

    PyObject_SetAttrString( object, "breveInstance", result );

	Py_INCREF( result );

	return result;
}

/**
 * Python callback to remove a Python instance from the breve engine.
 */

PyObject *brPythonRemoveInstance( PyObject *inSelf, PyObject *inArgs ) {
	PyObject *engineObject, *object;

	if ( !PyArg_ParseTuple( inArgs, "OO", &engineObject, &object ) ) return NULL;

    PyObject *breveObject = PyObject_GetAttrString( object, "breveInstance" );

	brEngine *engine = ( brEngine* )PyCObject_AsVoidPtr( engineObject );
	brInstance *instance = ( brInstance* )PyCObject_AsVoidPtr( breveObject );

	brEngineRemoveInstance( engine, instance );

	Py_INCREF( Py_None );
	return Py_None;
}

/**
 * Python callback to locate a breve internal function.
 * 
 * Arguments are parsed from the Python tuple.
 */

PyObject *brPythonFindInternalFunction( PyObject *inSelf, PyObject *inArgs ) {
	char *name;
	PyObject *breveModule;

	if ( !PyArg_ParseTuple( inArgs, "Os", &breveModule, &name ) ) return NULL;

	brEngine *engine = ( brEngine* )PyCObject_AsVoidPtr( breveModule );

	brInternalFunction *function = brEngineInternalFunctionLookup( engine, name );

	PyObject *result = Py_None;

	if ( !function ) {
		std::string error( "could not locate internal breve method \"" );
		error += name;
		error += "\"";

		PyErr_SetString( PyExc_RuntimeError, error.c_str() );
		return NULL;
	}

	result = PyCObject_FromVoidPtr( ( void* )function, NULL );

	return result;
}

/**
 * Python callback to call a breve internal function. 
 * 
 * The function has already been looked up using \ref brPythonFindInternalFunction.
 */

PyObject *brPythonCallInternalFunction( PyObject *inSelf, PyObject *inArgs ) {
	brEval args[ 128 ], resultEval;

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
			char err[ 1024 ];
			snprintf( err, 1023, "invalid type for argument %d of internal function \"%s\"", n, function->_name.c_str() );

			PyErr_SetString( PyExc_RuntimeError, err );
			return NULL;
		}
	}

	function->_call( args, &resultEval, caller );

	PyObject *result = brPythonTypeFromEval( &resultEval, PyObject_GetAttrString( moduleObject, "bridgeObject" ) );

	return result;
}

/**
 * Python callback to locate a breve object method (in another language frontend).
 * 
 * Arguments are parsed from the Python tuple.
 */

PyObject *brPythonFindBridgeMethod( PyObject *inSelf, PyObject *inArgs ) {
	PyObject *object, *attrObject;
	char *name;
	char err[ 1024 ];

	if ( !PyArg_ParseTuple( inArgs, "Os", &object, &name ) ) return NULL;

	attrObject = PyObject_GetAttrString( object, "breveInstance" );

	if( !attrObject ) {
		snprintf( err, 1023, "could not locate breve instance for Python bridge instance" );
		PyErr_SetString( PyExc_RuntimeError, err );
		return NULL;
	}

	for( unsigned int n = 0; n < strlen( name ); n++ ) {
		if( name[ n ] == '_' ) name[ n ] = '-';
	}

	brInstance *breveInstance = (brInstance*)PyCObject_AsVoidPtr( attrObject );

	// if( !PyObject_HasAttrString( object, name ) ) {
		brMethod *method = brMethodFindWithArgRange( breveInstance->object, name, NULL, 0, 9999 );

		if( !method ) {
			Py_INCREF( Py_None );
			return Py_None;
		}

		PyObject *methodObject = PyCObject_FromVoidPtr( method, NULL );
		// PyObject_SetAttrString( object, name, methodObject );
	// }

	return methodObject;

	// return PyObject_GetAttrString( object, name );
}

/**
 * Python callback to call a breve object method (for another language frontend)
 * 
 * The method has already been located using \ref brPythonFindBridgeMethod.
 */

PyObject *brPythonCallBridgeMethod( PyObject *inSelf, PyObject *inArgs ) {
	PyObject *object, *method, *arguments, *module;
	char err[ 1024 ];
	const brEval **args;
	brEval resultEval;

	args = new const brEval*[ 128 ];

	if ( !PyArg_ParseTuple( inArgs, "OOOO", &module, &object, &method, &arguments ) ) return NULL;

	PyObject *attrObject = PyObject_GetAttrString( object, "breveInstance" );

	if( !attrObject ) {
		snprintf( err, 1023, "could not locate breve instance for Python bridge instance" );
		PyErr_SetString( PyExc_RuntimeError, err );
		return NULL;
	}

	brInstance *breveInstance = (brInstance*)PyCObject_AsVoidPtr( attrObject );

	brMethod *breveMethod = (brMethod*)PyCObject_AsVoidPtr( method );

	for( int n = 0; n < PyTuple_GET_SIZE( arguments ); n++ ) {
		brEval *newEval = new brEval;
		brPythonTypeToEval( PyTuple_GET_ITEM( arguments, n ), newEval );
		args[ n ] = newEval;
	}

	brMethodCall( breveInstance, breveMethod, args, &resultEval );
	
	PyObject *result = brPythonTypeFromEval( &resultEval, PyObject_GetAttrString( module, "bridgeObject" ) );

	for( int n = 0; n < PyTuple_GET_SIZE( arguments ); n++ ) {
		delete args[ n ];
	}

	delete args;


	return result;
}



/************************************************************************
 * Python language frontend callbacks for the breveObjectType structure *
 ************************************************************************/


/**
 * A breveObjectType callback to locate a method in a Python object.
 * 
 * @param inObject 	A void pointer to a Python object.
 * @param inName	The name of the method to locate.
 * 
 * Unused parameters: argument types and argument count.
 * 
 * @return 			A void pointer to a Python method object. 
 */

void *brPythonFindMethod( void *inObject, const char *inName, unsigned char *, int ) {
	PyObject *type = ( PyObject* )inObject;

	PyObject *method = PyObject_GetAttrString( type, (char*)inName );

	if ( !method ) {
		PyErr_Clear();
		return NULL;
	}

	// Py_INCREF( method );

	return method;
}

void *brPythonFindObject( void *inData, const char *inName ) {
	PyObject *module = ( PyObject* )inData;

	PyObject *object = PyObject_GetAttrString( module, (char*)inName );

	if ( !object ) {
		PyErr_Clear();
		return NULL;
	}

	// Py_INCREF( object );

	return object;
}

brInstance *brPythonInstantiate( brEngine *inEngine, brObject* inObject, const brEval **, int ) {
	PyObject *object = ( PyObject* )inObject->userData;

	PyObject *result = PyObject_Call( object, PyTuple_New( 0 ), NULL );

	if ( !result ) {
		PyErr_Print();
		return NULL;
	}

    PyObject *breveObject = PyObject_GetAttrString( result, "breveInstance" );

	if( !breveObject ) {
		slMessage( DEBUG_ALL, "New Python object was not added to breve engine\n" );
		slMessage( DEBUG_ALL, "breve Python objects must inherit from class breve.object\n" );
		return NULL;
	}

	Py_INCREF( result );

	return (brInstance*)PyCObject_AsVoidPtr( breveObject );
}

int brPythonCallMethod( void *inInstance, void *inMethod, const brEval **, brEval *outResult ) {
	PyObject *instance = ( PyObject* )inInstance;
	PyObject *method = ( PyObject* )inMethod;

	if ( !PyCallable_Check( method ) ) {
		slMessage( DEBUG_ALL, "Warning: called method is not callable\n" );
		return EC_ERROR;
	}

	PyObject *tuple = PyTuple_New( 1 );

	Py_INCREF( instance );
	PyTuple_SetItem( tuple, 0, instance );

	PyObject *result = PyObject_Call( method, tuple, NULL );

	Py_DECREF( tuple );

	brPythonTypeToEval( result, outResult );

	if( !result ) {
		PyErr_Print();
		return EC_ERROR;
	}

	Py_DECREF( result );

	return EC_OK;
}

/**
 * A brObjectType callback to determine whether one object is a subclass of another.
 */

int brPythonIsSubclass( void *, void * ) {
	return 0;
}

/**
 * A brObjectType callback to clean up a generic Python object.
 *
 * @param inObject		A void pointer to a Python object.
 */

void brPythonDestroyGenericPythonObject( void *inObject ) {
	if( !inObject ) return;

	// printf(" DECREF %p\n", inObject );

	PyObject *object = ( PyObject* )inObject;
	Py_DECREF( object );
}

/**
 * A function to initialize the Python frontend.  
 * 
 * Creates necessary Python structures and initializes a brObjectType for
 * all of the required language callbacks.
 */

void brPythonInit( brEngine *breveEngine ) {
	brObjectType *brevePythonType = new brObjectType();

	static PyMethodDef methods[] = {
		{ "findInternalFunction", 	brPythonFindInternalFunction, 	METH_VARARGS, "" }, 
		{ "callInternalFunction", 	brPythonCallInternalFunction, 	METH_VARARGS, "" },
		{ "addInstance", 			brPythonAddInstance, 			METH_VARARGS, "" },
		{ "removeInstance", 		brPythonRemoveInstance, 		METH_VARARGS, "" },
		{ "findBridgeMethod", 		brPythonFindBridgeMethod, 		METH_VARARGS, "" },
		{ "callBridgeMethod", 		brPythonCallBridgeMethod, 		METH_VARARGS, "" },
		{ "catchOutput", 			brPythonCatchOutput, 			METH_VARARGS, "" },
		{ NULL, NULL, 0, NULL }
	};

	Py_Initialize();

	brevePythonType->userData = ( void* )PyImport_ImportModule( "__main__" );

	Py_INCREF( ( PyObject* )brevePythonType->userData );

	PyObject *internal = Py_InitModule( "breveInternal", methods );

	PyObject_SetAttrString( internal, "breveEngine", PyCObject_FromVoidPtr( ( void* )breveEngine, NULL ) );
	PyObject_SetAttrString( internal, "breveObjectType", PyCObject_FromVoidPtr( ( void* )brevePythonType, NULL ) );

	PyRun_SimpleString( "import sys" );

	const std::vector< std::string > &paths = brEngineGetSearchPaths( breveEngine );

	for( unsigned int n = 0; n < paths.size(); n++ ) {
		char path[ MAXPATHLEN + 1024 ];
		snprintf( path, MAXPATHLEN + 1023, "sys.path.append( '%s' )", paths[ n ].c_str() );

		PyRun_SimpleString( path );
	}

	PyRun_SimpleString( "import breve" );

	brevePythonType->findMethod 		= brPythonFindMethod;
	brevePythonType->findObject 		= brPythonFindObject;
	brevePythonType->instantiate 		= brPythonInstantiate;
	brevePythonType->callMethod 		= brPythonCallMethod;
	brevePythonType->isSubclass 		= brPythonIsSubclass;
	brevePythonType->destroyObject 		= brPythonDestroyGenericPythonObject;
	brevePythonType->destroyMethod 		= brPythonDestroyGenericPythonObject;
	brevePythonType->destroyInstance 	= brPythonDestroyGenericPythonObject;
	brevePythonType->_typeSignature 	= PYTHON_TYPE_SIGNATURE;

	brEngineRegisterObjectType( breveEngine, brevePythonType );

 	breveInitPythonFunctions( breveEngine->internalMethods );

#ifndef WINDOWS
	// Those meddling Python kids mess with the signal handlers!

	signal( SIGINT, NULL );
#endif
}

#endif
