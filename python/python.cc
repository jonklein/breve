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

#include "python.h"


/**
 * Translation function from a Python object to a breve value.
 * 
 * @param inObject		The Python object to be translated.
 * @param outEval		The breve value output.
 * @return			Either EC_OK or EC_ERROR indicating the success of the translation
 */

inline int brPythonTypeToEval( PyObject *inObject, brEval *outEval ) {
	int result = EC_ERROR;

	if( !inObject ) return result;

	if( inObject == Py_None ) {
		return EC_OK;
	}

	if( 0 && PyObject_GetAttrString( inObject, "isVector" ) != Py_None ) {
		slVector v = { 0.0, 0.0, 0.0 };

		PyObject *x = PyObject_GetAttrString( inObject, "x" );
		PyObject *y = PyObject_GetAttrString( inObject, "y" );
		PyObject *z = PyObject_GetAttrString( inObject, "z" );

		if( x ) v.x = PyFloat_AS_DOUBLE( x );
		if( y ) v.y = PyFloat_AS_DOUBLE( y );
		if( z ) v.z = PyFloat_AS_DOUBLE( z );

		outEval->set( v );
		result = EC_OK;

	} else if ( PyInt_Check( inObject ) ) {

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
 * @param inBridgeObject
 * @return			A newly created Python object translated from the source value.
 */

inline PyObject *brPythonTypeFromEval( const brEval *inEval, PyObject *inBridgeObject ) {
	PyObject *result = Py_None;
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

			if( breveInstance && breveInstance->object->type->_typeSignature == PYTHON_TYPE_SIGNATURE ) {
				result = (PyObject*)breveInstance->userData;
			} else if( breveInstance ) {
				// Create a bridge object, and set the breveInstance field

				result = PyObject_Call( inBridgeObject, PyTuple_New( 0 ), NULL );
				PyObject_SetAttrString( result, "breveInstance", PyCObject_FromVoidPtr( breveInstance, NULL ) );
			} else {
				result = Py_None;
			}

			break;

		case AT_POINTER:
			result = PyCObject_FromVoidPtr( BRPOINTER( inEval ), NULL );
			break;

		case AT_VECTOR:
			result = PyTuple_New( 3 );
			PyTuple_SetItem( result, 0, PyFloat_FromDouble( BRVECTOR( inEval ).x ) );
			PyTuple_SetItem( result, 1, PyFloat_FromDouble( BRVECTOR( inEval ).y ) );
			PyTuple_SetItem( result, 2, PyFloat_FromDouble( BRVECTOR( inEval ).z ) );
			break;

		case AT_MATRIX:
			{ 
				slMatrix &m = BRMATRIX( inEval );

				result = PyTuple_New( 3 );

				PyObject *rowx = PyTuple_New( 3 );
				PyObject *rowy = PyTuple_New( 3 );
				PyObject *rowz = PyTuple_New( 3 );

				PyTuple_SetItem( rowx, 0, PyFloat_FromDouble( m[ 0 ][ 0 ] ) );
				PyTuple_SetItem( rowx, 1, PyFloat_FromDouble( m[ 0 ][ 1 ] ) );
				PyTuple_SetItem( rowx, 2, PyFloat_FromDouble( m[ 0 ][ 2 ] ) );
				PyTuple_SetItem( rowy, 0, PyFloat_FromDouble( m[ 1 ][ 0 ] ) );
				PyTuple_SetItem( rowy, 1, PyFloat_FromDouble( m[ 1 ][ 1 ] ) );
				PyTuple_SetItem( rowy, 2, PyFloat_FromDouble( m[ 1 ][ 2 ] ) );
				PyTuple_SetItem( rowz, 0, PyFloat_FromDouble( m[ 2 ][ 0 ] ) );
				PyTuple_SetItem( rowz, 1, PyFloat_FromDouble( m[ 2 ][ 1 ] ) );
				PyTuple_SetItem( rowz, 2, PyFloat_FromDouble( m[ 2 ][ 2 ] ) );

				PyTuple_SetItem( result, 0, rowx );
				PyTuple_SetItem( result, 1, rowy );
				PyTuple_SetItem( result, 2, rowz );
			}

			break;

		case AT_LIST:
			{
				brEvalListHead *list = BRLIST( inEval );

				result = PyList_New( list->_vector.size() );

				for( unsigned int n = 0; n < list->_vector.size(); n++ ) {
					PyObject *element = brPythonTypeFromEval( &list->_vector[ n ], inBridgeObject );
					PyList_SET_ITEM( result, n, element );
				}
			}


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

/**
 * Callback to handle stdout/stderr from Python and redirect it to breve output.
 */

PyObject *brPythonCatchOutput( PyObject *inSelf, PyObject *inArgs ) {
	PyObject *moduleObject;
	char *message;

	if( !PyArg_ParseTuple( inArgs, "Os", &moduleObject, &message ) ) return NULL;

	slFormattedMessage( DEBUG_ALL, message );

	Py_INCREF( Py_None );
	return Py_None;
}

/**
 * Callback to set the controller object
 */

PyObject *brPythonSetController( PyObject *inSelf, PyObject *inArgs ) {
	PyObject *moduleObject, *pythonInstance;

	if ( !PyArg_ParseTuple( inArgs, "OO", &moduleObject, &pythonInstance) ) return NULL;

	PyObject *engineObject = PyObject_GetAttrString( moduleObject, "breveEngine" );
	PyObject *instanceObject = PyObject_GetAttrString( pythonInstance, "breveInstance" );

	brEngine *engine = (brEngine*)PyCObject_AsVoidPtr( engineObject );
	brInstance *instance = (brInstance*)PyCObject_AsVoidPtr( instanceObject );

        brEngineSetController( engine, instance );

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

	// Extract the breveEngine from the module data

	PyObject *engineObject = PyObject_GetAttrString( moduleObject, "breveEngine" );

	if( !engineObject ) {
		PyErr_SetString( PyExc_RuntimeError, "Could not locate breve engine in first internal breve call argument" );
		return NULL;
	}

	brEngine *engine = ( brEngine* )PyCObject_AsVoidPtr( engineObject );

	// Adding the instance requires a brObject type.
	// Figure out the brObject type for this instance.  We may need to create it ourselves.

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

	// Now the engine has a copy of object -- increment the reference count!
	
	Py_INCREF( object );

	if( i ) 
		result = PyCObject_FromVoidPtr( i, NULL );
	else
		result = Py_None;

	// Set the breveInstance field for this object to the newly created engine instance

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
			snprintf( err, 1023, "invalid type for argument %d of internal function \"%s\" (got \"%s\", expected \"%s\")", n, function->_name.c_str(), brAtomicTypeStrings[ args[ n ].type() ], brAtomicTypeStrings[ function->_argTypes[ n ] ] );

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
		brMethod *method = brMethodFindWithArgRange( breveInstance->object, name, NULL, 0, 50 );

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

	delete[] args;

	return result;
}






/////////////////////////////////////////////////////////////////////////////////////                                                
//  Python language frontend callbacks for the breveObjectType structure           //
/////////////////////////////////////////////////////////////////////////////////////                                                  


/**
 * A breveObjectType callback to locate a method in a Python object.
 * 
 * @param inObject 	A void pointer to a Python object.
 * @param inName	The name of the method to locate.
 * @param inTypes 	Argument types (currently unused)
 * @param inCount	The argument count
 * 
 * @return 			A void pointer to a Python method object. 
 */

void *brPythonFindMethod( void *inObject, const char *inName, unsigned char *inTypes, int inCount ) {
	PyObject *type = ( PyObject* )inObject;

	PyObject *method = PyObject_GetAttrString( type, (char*)inName );

	if ( !method ) {
		PyErr_Clear();
		return NULL;
	}

	return method;
}

/**
 * A breveObjectType callback to locate a class in Python.
 * 
 * @param inData 	The userdata callback pointer to a __main__ module 
 * @param inName	The name of the desired object
 */

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

/**
 * A breveObjectType callback to instantiate a class in Python
 * 
 * @param inEngine	The breve engine creating the instancce.
 * @param inObject	The object to be instantiated.
 * @param inArgs	Constructor arguments -- not currently used.
 * @param inArgCount	Constructor argument count -- not currently used.
 */

brInstance *brPythonInstantiate( brEngine *inEngine, brObject* inObject, const brEval **inArgs, int inArgCount ) {
	PyObject *object = ( PyObject* )inObject->userData;

	PyObject *result = PyObject_Call( object, PyTuple_New( 0 ), NULL );

	if ( !result ) {
		PyErr_Print();
		return NULL;
	}

	PyObject *breveObject = PyObject_GetAttrString( result, "breveInstance" );

	if( !breveObject ) {
		// This is a fundamental rule of the Python frontend: objects must add themselves 
		// to the breve engine, or else they cannot be used

		slMessage( DEBUG_ALL, "New Python object was not added to breve engine\n" );
		slMessage( DEBUG_ALL, "breve Python objects must inherit from class breve.object\n" );
		return NULL;
	}

	Py_INCREF( result );

	// Note: the brInstance we return will 

	return (brInstance*)PyCObject_AsVoidPtr( breveObject );
}

/**
 * A breveObjectType callback to call a method in Python
 * 
 * @param inInstance	The object instance data (as found with \ref brPythonInstantiate)
 * @param inMethod	The method callback data (as found with \ref brPythonFindMethod )
 * @param inArguments	NOT CURRENTLY IMPLEMENTED
 * @param outResult	The brEval result of the method calll
 */

int brPythonCallMethod( void *inInstance, void *inMethod, const brEval **inArguments, brEval *outResult ) {
	PyObject *instance = ( PyObject* )inInstance;
	PyObject *method = ( PyObject* )inMethod;

	if ( !PyCallable_Check( method ) ) {
		slMessage( DEBUG_ALL, "Warning: called method is not callable\n" );
		return EC_ERROR;
	}

	// Get the number of arguments to this method

	PyObject *code = PyObject_GetAttrString( method, "func_code" );
	PyObject *argumentCount = PyObject_GetAttrString( code, "co_argcount" );

	if( !argumentCount ) {
		slMessage( DEBUG_ALL, "Warning: cannot find argument count for method %p\n", method );
		return EC_ERROR;
	}

	// we subtract 1 for the "self" argument

	int count = PyInt_AS_LONG( argumentCount ) - 1;

	PyObject *tuple = PyTuple_New( count + 1 );

	// the instance we're calling the method for has the breveInternal module pointer

	PyObject *module = PyObject_GetAttrString( instance, "breveModule" );

	if( !module ) {
		slMessage( DEBUG_ALL, "Could not locate breveModule for %p\n", instance );
		PyErr_Print();
		return EC_ERROR;
	}


	PyObject *internal = PyObject_GetAttrString( module, "bridgeObject" );

	if( !internal ) {
		PyErr_Print();
		return EC_ERROR;
	}


	// Set the self argument

	Py_INCREF( instance );
	PyTuple_SetItem( tuple, 0, instance );

	// Set the rest of the arguments

	for( int n = 0; n < count; n++ ) {
		PyObject *argument = brPythonTypeFromEval( inArguments[ n ], internal );
		Py_INCREF( argument );
		PyTuple_SetItem( tuple, n + 1, argument );
	}

	PyObject *result = PyObject_Call( method, tuple, NULL );

	// Py_DECREF( tuple );

	brPythonTypeToEval( result, outResult );

	if( !result ) {
		PyErr_Print();
		return EC_ERROR;
	}

	Py_DECREF( result );

	return EC_OK;
}

/**
 * A brObjectType callback to determine whether one object is a subclass of another.  Used by 
 * collision detection to determine if a handler is installed for an object pair.
 * 
 * Clearly not implemented at the moment.
 */

int brPythonIsSubclass( void *inClassA, void *inClassB ) {
	return 0;
}

/**
 * The Python canLoad breve object callback
 */

int brPythonCanLoad( void *inObjectData, const char *inExtension ) {
	if( !strcasecmp( inExtension, "py" ) ||
		!strcasecmp( inExtension, "pybreve" ) )
			return 1;

	return 0;
}

/**
 * The Python load breve object callback
 */

int brPythonLoad( brEngine *inEngine, void *inObjectTypeUserData, const char *inFilename, const char *inFiletext ) {
	if( PyRun_SimpleString( inFiletext ) )
		return EC_ERROR;

	return EC_OK;
}

/**
 * A brObjectType callback to clean up a generic Python object.  This method is used as the 
 * destructor for both objects and methods
 *
 * @param inObject		A void pointer to a Python object.
 */

void brPythonDestroyGenericPythonObject( void *inObject ) {
	if( !inObject ) return;

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
	static int pyInitialized = 0;

	brObjectType *brevePythonType = new brObjectType();

	static PyMethodDef methods[] = {
		{ "setController", 		brPythonSetController, 	METH_VARARGS, "" }, 
		{ "findInternalFunction", 	brPythonFindInternalFunction, 	METH_VARARGS, "" }, 
		{ "callInternalFunction", 	brPythonCallInternalFunction, 	METH_VARARGS, "" },
		{ "addInstance", 			brPythonAddInstance, 			METH_VARARGS, "" },
		{ "removeInstance", 		brPythonRemoveInstance, 		METH_VARARGS, "" },
		{ "findBridgeMethod", 		brPythonFindBridgeMethod, 		METH_VARARGS, "" },
		{ "callBridgeMethod", 		brPythonCallBridgeMethod, 		METH_VARARGS, "" },
		{ "catchOutput", 			brPythonCatchOutput, 			METH_VARARGS, "" },
		{ NULL, NULL, 0, NULL }
	};

	if( !pyInitialized ) {
		pyInitialized = 1;
		Py_Initialize();
	}

	brevePythonType->userData = ( void* )PyImport_ImportModule( "__main__" );

	Py_INCREF( ( PyObject* )brevePythonType->userData );

	PyObject *internal = Py_InitModule( "breveInternal", methods );

	PyObject_SetAttrString( internal, "breveEngine", PyCObject_FromVoidPtr( ( void* )breveEngine, NULL ) );
	PyObject_SetAttrString( internal, "breveObjectType", PyCObject_FromVoidPtr( ( void* )brevePythonType, NULL ) );

	PyRun_SimpleString( "import sys" );

	std::string setpath, path;

	for( unsigned int n = 0; n < breveEngine->_searchPaths.size(); n++ ) {
		path = breveEngine->_searchPaths[ n ];
		setpath = "sys.path.append( '" + path + "' ) ";

		PyRun_SimpleString( setpath.c_str() );

		path += "/PythonLibraries/";
		setpath = "sys.path.append( '" + path + "' ) ";

		PyRun_SimpleString( setpath.c_str() );
	}

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
	brevePythonType->canLoad		= brPythonCanLoad;
	brevePythonType->load			= brPythonLoad;
	brevePythonType->_typeSignature 	= PYTHON_TYPE_SIGNATURE;

	brEngineRegisterObjectType( breveEngine, brevePythonType );

 	breveInitPythonFunctions( breveEngine->internalMethods );

#ifndef WINDOWS
	// Those meddling Python kids mess with the signal handlers!

	signal( SIGINT, NULL );
#endif
}

#endif
