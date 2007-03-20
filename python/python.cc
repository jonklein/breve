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

std::string brPythonToSteveName( const char *in ) {
	std::string result;

	for( unsigned int n = 0; n < strlen( in ); n++ ) {
		if( isupper( in[ n ] ) ) {
			result += "-";
			result += tolower( in[ n ] );
		} else  {
			result += in[ n ];
		}
	}

	return result;
}


/**
 * Translates a PyInt or PyFloat to a double.  For breve vectors and matrices,
 * which can come in either as ints or doubles
 */

double PyNumber_AS_DOUBLE( PyObject *inObject ) {

	if( PyInt_Check( inObject ) ) 
       		return (double)PyInt_AS_LONG( inObject );

	return PyFloat_AS_DOUBLE( inObject );
}

/**
 * Does PyObject_GetAttrString 
 *
 * Super-suprising Python optimization note: HasAttrString actually calls GetAttrString.
 * So when I thought I was being smart by checking for the Attr before getting it, it 
 * actually was taking up more time!
 */

inline PyObject *PyObject_GetAttrStringSafe( PyObject *inObject, const char *inString  ) {
	if( !inObject ) 
		return NULL;

	PyObject *result = PyObject_GetAttrString( inObject, (char*) inString );

	if( !result )
		PyErr_Clear();

	return result;
}



/**
 * Translation function from a Python object to a breve value.
 * 
 * @param inObject		The Python object to be translated.
 * @param outEval		The breve value output.
 * @return			Either EC_OK or EC_ERROR indicating the success of the translation
 */

inline int brPythonTypeToEval( PyObject *inObject, brEval *outEval ) {
	int result = EC_ERROR;
	PyObject *breveInstance;

	if( !inObject ) 
		return result;

	if( inObject == Py_None )
		return EC_OK;

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

	} else if( PyObject_HasAttrString( inObject, "isVector" ) ) {
		slVector v = { 0.0, 0.0, 0.0 };

		PyObject *x = PyObject_GetAttrString( inObject, "x" );
		PyObject *y = PyObject_GetAttrString( inObject, "y" );
		PyObject *z = PyObject_GetAttrString( inObject, "z" );

		if( x ) v.x = PyNumber_AS_DOUBLE( x );
		if( y ) v.y = PyNumber_AS_DOUBLE( y );
		if( z ) v.z = PyNumber_AS_DOUBLE( z );

		Py_DECREF( x );
		Py_DECREF( y );
		Py_DECREF( z );

		outEval->set( v );
		result = EC_OK;

	} else if ( ( breveInstance = PyObject_GetAttrStringSafe( inObject, "breveInstance" ) ) )  {

		outEval->set( (brInstance*)PyCObject_AsVoidPtr( breveInstance ) );
		Py_DECREF( breveInstance );
		result = EC_OK;

	} else if( PyObject_HasAttrString( inObject, "isMatrix" ) ) {
		slMatrix m;

		PyObject *x1 = PyObject_GetAttrString( inObject, "x1" );
		PyObject *x2 = PyObject_GetAttrString( inObject, "x2" );
		PyObject *x3 = PyObject_GetAttrString( inObject, "x3" );
		PyObject *y1 = PyObject_GetAttrString( inObject, "y1" );
		PyObject *y2 = PyObject_GetAttrString( inObject, "y2" );
		PyObject *y3 = PyObject_GetAttrString( inObject, "y3" );
		PyObject *z1 = PyObject_GetAttrString( inObject, "z1" );
		PyObject *z2 = PyObject_GetAttrString( inObject, "z2" );
		PyObject *z3 = PyObject_GetAttrString( inObject, "z3" );

		m[ 0 ][ 0 ] = PyNumber_AS_DOUBLE( x1 );
		m[ 0 ][ 1 ] = PyNumber_AS_DOUBLE( x2 );
		m[ 0 ][ 2 ] = PyNumber_AS_DOUBLE( x3 );
		m[ 1 ][ 0 ] = PyNumber_AS_DOUBLE( y1 );
		m[ 1 ][ 1 ] = PyNumber_AS_DOUBLE( y2 );
		m[ 1 ][ 2 ] = PyNumber_AS_DOUBLE( y3 );
		m[ 2 ][ 0 ] = PyNumber_AS_DOUBLE( z1 );
		m[ 2 ][ 1 ] = PyNumber_AS_DOUBLE( z2 );
		m[ 2 ][ 2 ] = PyNumber_AS_DOUBLE( z3 );

		Py_DECREF( x1 );
		Py_DECREF( x2 );
		Py_DECREF( x3 );
		Py_DECREF( y1 );
		Py_DECREF( y2 );
		Py_DECREF( y3 );
		Py_DECREF( z1 );
		Py_DECREF( z2 );
		Py_DECREF( z3 );

		outEval->set( m );
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
 * @param inModuleObject	The Python breveInternal module object
 * @return			A newly created Python object translated from the source value.
 */

inline PyObject *brPythonTypeFromEval( const brEval *inEval, PyObject *inModuleObject ) {
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

			if( breveInstance && breveInstance->object->type->_typeSignature == PYTHON_TYPE_SIGNATURE ) {
				result = (PyObject*)breveInstance->userData;
				Py_INCREF( result );
			} else if( breveInstance ) {
				// Create a bridge object, and set the breveInstance field

				PyObject *bridgeObject = PyObject_GetAttrString( inModuleObject, "bridgeObject" );

				PyObject *args = PyTuple_New( 0 );
				result = PyObject_Call( bridgeObject, args, NULL );
				Py_DECREF( args );

				if( !result ) {
					PyErr_Print();
				
					Py_INCREF( Py_None );
					return Py_None;
				}

				PyObject_SetAttrString( result, "breveInstance", PyCObject_FromVoidPtr( breveInstance, NULL ) );
			} else {
				result = Py_None;
				Py_INCREF( result );
			}

			break;

		case AT_POINTER:
			result = PyCObject_FromVoidPtr( BRPOINTER( inEval ), NULL );
			break;

		case AT_VECTOR:
			{
				PyObject *vectorType = PyObject_GetAttrString( inModuleObject, "vectorType" );
				const slVector &v = BRVECTOR( inEval );

				if( vectorType ) {
					PyObject *args = PyTuple_New( 3 );

					PyTuple_SET_ITEM( args, 0, PyFloat_FromDouble( v.x ) );
					PyTuple_SET_ITEM( args, 1, PyFloat_FromDouble( v.y ) );
					PyTuple_SET_ITEM( args, 2, PyFloat_FromDouble( v.z ) );

					result = PyObject_Call( vectorType, args, NULL );

					Py_DECREF( args );
					Py_DECREF( vectorType );
				}
			}

			break;

		case AT_MATRIX:
			{ 
				const slMatrix &m = BRMATRIX( inEval );

				PyObject *matrixType = PyObject_GetAttrString( inModuleObject, "matrixType" );

				if( matrixType ) {
					PyObject *args = PyTuple_New( 9 );

					PyTuple_SET_ITEM( args, 0, PyFloat_FromDouble( m[ 0 ][ 0 ] ) );
					PyTuple_SET_ITEM( args, 1, PyFloat_FromDouble( m[ 0 ][ 1 ] ) );
					PyTuple_SET_ITEM( args, 2, PyFloat_FromDouble( m[ 0 ][ 2 ] ) );
					PyTuple_SET_ITEM( args, 3, PyFloat_FromDouble( m[ 1 ][ 0 ] ) );
					PyTuple_SET_ITEM( args, 4, PyFloat_FromDouble( m[ 1 ][ 1 ] ) );
					PyTuple_SET_ITEM( args, 5, PyFloat_FromDouble( m[ 1 ][ 2 ] ) );
					PyTuple_SET_ITEM( args, 6, PyFloat_FromDouble( m[ 2 ][ 0 ] ) );
					PyTuple_SET_ITEM( args, 7, PyFloat_FromDouble( m[ 2 ][ 1 ] ) );
					PyTuple_SET_ITEM( args, 8, PyFloat_FromDouble( m[ 2 ][ 2 ] ) );

					result = PyObject_Call( matrixType, args, NULL );

					Py_DECREF( args );
					Py_DECREF( matrixType );
				}
			}

			break;

		case AT_LIST:
			{
				brEvalListHead *list = BRLIST( inEval );

				result = PyList_New( list->_vector.size() );

				for( unsigned int n = 0; n < list->_vector.size(); n++ ) {
					PyObject *element = brPythonTypeFromEval( &list->_vector[ n ], inModuleObject );
					PyList_SET_ITEM( result, n, element );
				}
			}


break;

		case AT_ARRAY:
		case AT_DATA:
		case AT_HASH:
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


	Py_DECREF( engineObject );
	Py_DECREF( instanceObject );

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

	Py_DECREF( engineObject );

	// Adding the instance requires a brObject type.
	// We may need to create it ourselves.

	PyObject *breveTypeObject = PyObject_GetAttrString( typeObject, "breveObject" );

	if( !breveTypeObject ) {
		// Clear the missing attr error 
		PyErr_Clear();

		PyObject *nameObject = PyObject_GetAttrString( typeObject, "__name__" );

		if( !nameObject || !PyString_Check( nameObject ) ) {
			PyErr_SetString( PyExc_RuntimeError, "Internal error while adding breve class type for Python object" );
			return NULL;
		}

		char *name = PyString_AsString( nameObject );

		Py_DECREF( nameObject );

		// This object type has not previously been added to the breve engine.

		PyObject *pythonLanguageType = PyObject_GetAttrString( moduleObject, "breveObjectType" );

		breveObject = brObjectFind( engine, name );

		if( !breveObject )
			breveObject = brEngineAddObject( engine, (brObjectType*)PyCObject_AsVoidPtr( pythonLanguageType ), name, typeObject );

		PyObject_SetAttrString( typeObject, "breveObject", PyCObject_FromVoidPtr( breveObject, NULL ) );

		Py_DECREF( pythonLanguageType );
	} else {
		// Found an existing brObject type for this Python class

		PyObject *breveTypeObject = PyObject_GetAttrString( typeObject, "breveObject" );

		breveObject = (brObject*)PyCObject_AsVoidPtr( breveTypeObject );

		Py_DECREF( breveTypeObject );
	}

	brInstance *i = brEngineAddInstance( engine, breveObject, object );

	// Now the engine has a copy of object -- increment the reference count
	
	Py_INCREF( object );

	if( i ) {
		result = PyCObject_FromVoidPtr( i, NULL );
	} else {
		result = Py_None;
		Py_INCREF( result );
	}

	// Set the breveInstance field for this object to the newly created engine instance

	PyObject_SetAttrString( object, "breveInstance", result );

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

	Py_DECREF( breveObject );

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


	if ( !function ) {
		std::string error( "could not locate internal breve method \"" );
		error += name;
		error += "\"";

		PyErr_SetString( PyExc_RuntimeError, error.c_str() );
		return NULL;
	}

	PyObject *result = PyCObject_FromVoidPtr( ( void* )function, NULL );

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

	return brPythonTypeFromEval( &resultEval, moduleObject );
}

/**
 * Python callback to locate a breve object method (in another language frontend).
 * 
 * Arguments are parsed from the Python tuple.
 */

PyObject *brPythonFindBridgeMethod( PyObject *inSelf, PyObject *inArgs ) {
	PyObject *object, *attrObject;
	char *name;
	std::string altname;
	char err[ 1024 ];

	if ( !PyArg_ParseTuple( inArgs, "Os", &object, &name ) ) return NULL;

	attrObject = PyObject_GetAttrString( object, "breveInstance" );

	if( !attrObject ) {
		snprintf( err, 1023, "could not locate breve instance for Python bridge instance" );
		PyErr_SetString( PyExc_RuntimeError, err );
		return NULL;
	}

	altname = brPythonToSteveName( name );

	for( unsigned int n = 0; n < strlen( name ); n++ ) {
		if( name[ n ] == '_' )
			name[ n ] = '-';
	}

	brInstance *breveInstance = (brInstance*)PyCObject_AsVoidPtr( attrObject );

	Py_DECREF( attrObject );

	brMethod *method;

	// try the original name

	method = brMethodFindWithArgRange( breveInstance->object, name, NULL, 0, 50 );
	if( method ) return PyCObject_FromVoidPtr( method, NULL );

	// try the alternative name

	method = brMethodFindWithArgRange( breveInstance->object, altname.c_str(), NULL, 0, 50 );
	if( method ) return PyCObject_FromVoidPtr( method, NULL );

	// nope

	Py_INCREF( Py_None );
	return Py_None;
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

	Py_DECREF( attrObject );

	brMethod *breveMethod = (brMethod*)PyCObject_AsVoidPtr( method );

	for( int n = 0; n < PyTuple_GET_SIZE( arguments ); n++ ) {
		brEval *newEval = new brEval;
		brPythonTypeToEval( PyTuple_GET_ITEM( arguments, n ), newEval );
		args[ n ] = newEval;
	}

	brMethodCall( breveInstance, breveMethod, args, &resultEval );
	
	for( int n = 0; n < PyTuple_GET_SIZE( arguments ); n++ ) {
		delete args[ n ];
	}

	delete[] args;

	return brPythonTypeFromEval( &resultEval, module );

}






/////////////////////////////////////////////////////////////////////////////////////                                                
//  Python language frontend callbacks for the breveObjectType structure           //
/////////////////////////////////////////////////////////////////////////////////////                                                  

std::string brPyConvertSymbol( std::string &inValue ) {
        std::string result; 
        unsigned int n = 0, m = 0;
        bool upper = false;
        
        // Hardcode some troublesome symbols -- they're reserved keywords in Python, but not steve
        
        if( inValue == "is" )
                return std::string( "isA" );
        
        if( inValue == "break" )
                return std::string( "snap" ); // hehe, this is fun
        
        for( n = 0; n < inValue.size(); n++, m++ ) {
                if( inValue[ n ] == '-' ) {
                        m--;
                        upper = true;
                } else {
                        
                        if( upper ) {
                                result += toupper( inValue[ n ] );
                        } else {
                                result += inValue[ n ];
                        }
                        
                        upper = false;
                }
        }
        
        return result;
}


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

	std::string symbol = std::string( inName );
	const char *name = brPyConvertSymbol( symbol ).c_str();

	/*
	// Translate steve to Python method names
	for( unsigned int n = 0; n < strlen( name ); n++ ) {
		if( name[ n ] == '-' ) name[ n ] = '_';
	}
	*/

	PyObject *method = PyObject_GetAttrStringSafe( type, name );

	if ( !method ) {
		return NULL;
	}

	PyObject *code = PyObject_GetAttrString( method, "func_code" );
	
	if( !code ) {
		PyErr_Clear();
		return NULL;
	}

	Py_DECREF( code );

	PyObject *argumentCount = PyObject_GetAttrString( code, "co_argcount" );

	if( !argumentCount || PyInt_AS_LONG( argumentCount ) != ( inCount + 1 ) ) {
		PyErr_Clear();
		return NULL;
	}

	Py_DECREF( argumentCount );

	printf("findMethod returning: %08x\n",(unsigned)method);
	return method;
}

/**
 * A breveObjectType callback to locate a class in Python.
 * 
 * @param inData 	The userdata callback pointer to a __main__ module 
 * @param inName	The name of the desired object
 */

void *brPythonFindObject( void *inData, const char *inName ) {
	PyObject *mainModule = ( PyObject* )inData;

	PyObject *obj = PyObject_GetAttrStringSafe( mainModule, (char*)inName );

	if( !obj ) {
		PyObject *breveModule = PyObject_GetAttrString( mainModule, "breve" );
	
		obj = PyObject_GetAttrStringSafe( breveModule, (char*)inName );
	}

	return obj;
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

	PyObject *args = PyTuple_New( 0 );
	PyObject *result = PyObject_Call( object, args, NULL );

	Py_DECREF( args );

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
	Py_DECREF( breveObject );

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
	static PyObject *tuples[ 5 ] = { PyTuple_New( 1 ), PyTuple_New( 2 ), PyTuple_New( 3 ), PyTuple_New( 4 ), PyTuple_New( 5 ) };

	printf("PythonCalMethod inMethod = %08x\n",(unsigned)inMethod);

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

	Py_DECREF( code );
	Py_DECREF( argumentCount );

	PyObject *tuple;

	if( 0 && count < 5 ) {
		tuple = tuples[ count ];
	} else {
		tuple = PyTuple_New( count + 1 );
	}

	// the instance we're calling the method for has the breveInternal module pointer

	PyObject *module = PyObject_GetAttrString( instance, "breveModule" );

	if( !module ) {
		slMessage( DEBUG_ALL, "Could not locate breveModule for %p\n", instance );
		PyErr_Print();
		return EC_ERROR;
	}

	// Set the self argument

	Py_INCREF( instance );
	int r = PyTuple_SetItem( tuple, 0, instance );

	if( r != 0 ) {
		PyErr_Print();
		return EC_ERROR;
	}

	// Set the rest of the arguments

	for( int n = 0; n < count; n++ ) {
		PyObject *argument = brPythonTypeFromEval( inArguments[ n ], module );
		PyTuple_SetItem( tuple, n + 1, argument );
	}

	Py_DECREF( module );

	PyObject *result = PyObject_Call( method, tuple, NULL );


	if( count > 5 )
		Py_DECREF( tuple );


	if( !result ) {
		PyErr_Print();
		return EC_ERROR;
	} else {
		Py_INCREF( result );
		brPythonTypeToEval( result, outResult );
	}


	return EC_OK;
}

/**
 * A brObjectType callback to determine whether one object is a subclass of another.  Used by 
 * collision detection to determine if a handler is installed for an object pair.
 */

int brPythonIsSubclass( brObjectType *inType, void *inClassA, void *inClassB ) {
	return PyObject_IsSubclass( (PyObject*)inClassA, (PyObject*)inClassB );
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
	int result = EC_OK;
	FILE *fp = fopen( inFilename, "r" );

	PyRun_SimpleString( "import breve" );

	if( fp ) {

		if( PyRun_SimpleFile( fp, inFilename ) )
			result = EC_ERROR;

		fclose( fp );

	} else {

		if( PyRun_SimpleString( inFiletext ) )
			result = EC_ERROR;
	}

	return result;
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
		{ "setController", 		brPythonSetController, 			METH_VARARGS, "" }, 
		{ "findInternalFunction", 	brPythonFindInternalFunction, 		METH_VARARGS, "" }, 
		{ "callInternalFunction", 	brPythonCallInternalFunction, 		METH_VARARGS, "" },
		{ "addInstance", 		brPythonAddInstance, 			METH_VARARGS, "" },
		{ "removeInstance", 		brPythonRemoveInstance, 		METH_VARARGS, "" },
		{ "findBridgeMethod", 		brPythonFindBridgeMethod, 		METH_VARARGS, "" },
		{ "callBridgeMethod", 		brPythonCallBridgeMethod, 		METH_VARARGS, "" },
		{ "catchOutput", 		brPythonCatchOutput, 			METH_VARARGS, "" },
		{ NULL, NULL, 0, NULL }
	};

	if( !pyInitialized ) {
		pyInitialized = 1;
		Py_Initialize();
	}

	PyObject *internal = Py_InitModule( "breveInternal", methods );

	PyObject_SetAttrString( internal, "breveEngine", PyCObject_FromVoidPtr( ( void* )breveEngine, NULL ) );
	PyObject_SetAttrString( internal, "breveObjectType", PyCObject_FromVoidPtr( ( void* )brevePythonType, NULL ) );

	// PyRun_SimpleString( "globals().clear()" );
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
		// printf( "Appending path '%s'\n", path );

		PyRun_SimpleString( path );
	}

	brevePythonType->userData = ( void* )PyImport_ImportModule( "__main__" );

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
