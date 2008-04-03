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

#include "pythonFrontend.h"

struct slPythonData {
	PyObject *_vectorClass;
	PyObject *_matrixClass;

	PyObject *_breveModule;
	PyObject *_mainModule;

	PyObject *_internalModule;
};

struct slPythonMethodInfo {
	PyObject *_method;

	int _argumentCount;
};

static slPythonData sPythonData;

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

inline double PyNumber_AS_DOUBLE( PyObject *inObject ) {
	if( PyFloat_Check( inObject ) ) 
		return PyFloat_AS_DOUBLE( inObject );

	return (double)PyInt_AS_LONG( inObject );
}

/**
 * Does PyObject_GetAttrString 
 *
 * Super-suprising Python optimization note: HasAttrString actually calls GetAttrString.
 * So when I thought I was being smart by checking for the Attr before getting it, it 
 * actually was taking up more time!
 */

inline PyObject *PyObject_GetAttrStringSafe( PyObject *inObject, const char *inString  ) {
	if( !inObject || !inString ) 
		return NULL;

	PyObject *result = PyObject_GetAttrString( inObject, (char*)inString );

	if( PyErr_Occurred() )
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

	} else if( inObject->ob_type == (_typeobject*)sPythonData._vectorClass ) {
		slVector *v = NULL;

		int r = inObject->ob_type->tp_as_buffer->bf_getreadbuffer( inObject, 0, (void**)&v );

		if( r < 0 )
			PyErr_Print();

		outEval->set( *v );
		result = EC_OK;

	} else if ( ( breveInstance = PyObject_GetAttrStringSafe( inObject, "breveInstance" ) ) )  {

		if( breveInstance == Py_None ) {
			outEval->set( (brInstance*)NULL );
		} else {
			outEval->set( (brInstance*)PyCObject_AsVoidPtr( breveInstance ) );
		}

		Py_DECREF( breveInstance );

		result = EC_OK;

	} else if( inObject->ob_type == (_typeobject*)sPythonData._matrixClass ) {
		slMatrix m;

		PyObject *x1 = PyList_GET_ITEM( inObject, 0 );
		PyObject *x2 = PyList_GET_ITEM( inObject, 1 );
		PyObject *x3 = PyList_GET_ITEM( inObject, 2 );
		PyObject *y1 = PyList_GET_ITEM( inObject, 3 );
		PyObject *y2 = PyList_GET_ITEM( inObject, 4 );
		PyObject *y3 = PyList_GET_ITEM( inObject, 5 );
		PyObject *z1 = PyList_GET_ITEM( inObject, 6 );
		PyObject *z2 = PyList_GET_ITEM( inObject, 7 );
		PyObject *z3 = PyList_GET_ITEM( inObject, 8 );

		m[ 0 ][ 0 ] = PyNumber_AS_DOUBLE( x1 );
		m[ 0 ][ 1 ] = PyNumber_AS_DOUBLE( x2 );
		m[ 0 ][ 2 ] = PyNumber_AS_DOUBLE( x3 );
		m[ 1 ][ 0 ] = PyNumber_AS_DOUBLE( y1 );
		m[ 1 ][ 1 ] = PyNumber_AS_DOUBLE( y2 );
		m[ 1 ][ 2 ] = PyNumber_AS_DOUBLE( y3 );
		m[ 2 ][ 0 ] = PyNumber_AS_DOUBLE( z1 );
		m[ 2 ][ 1 ] = PyNumber_AS_DOUBLE( z2 );
		m[ 2 ][ 2 ] = PyNumber_AS_DOUBLE( z3 );

		outEval->set( m );
		result = EC_OK;

	} else if( PySequence_Check( inObject ) ) {
		
		brEvalListHead *list = new brEvalListHead;

		int size = PySequence_Size( inObject );

		for( int n = 0; n < size; n++ ) {
			brEval item;

			PyObject *pyitem = PySequence_GetItem( inObject, n );

			brPythonTypeToEval( pyitem, &item );
			brEvalListInsert( list, list->_vector.size(), &item );

			Py_DECREF( pyitem );
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
				if( breveInstance->userData )
					result = (PyObject*)breveInstance->userData;
				else
					result = Py_None;

				Py_INCREF( result );

			} else if( breveInstance ) {

				// Create a bridge object, and set the breveInstance field

				PyObject *bridgeObject = PyObject_GetAttrStringSafe( inModuleObject, "bridgeObject" );

				PyObject *args = PyTuple_New( 0 );
				result = PyObject_Call( bridgeObject, args, NULL );
				Py_DECREF( args );
				Py_DECREF( bridgeObject );

				if( !result ) {
					PyErr_Print();
				
					Py_INCREF( Py_None );
					return Py_None;
				}

				PyObject *ptr = PyCObject_FromVoidPtr( breveInstance, NULL );
				PyObject_SetAttrString( result, "breveInstance", ptr );
				Py_DECREF( ptr );

			} else {
				result = Py_None;
				Py_INCREF( result );
			}

			break;

		case AT_POINTER:
			if( BRPOINTER( inEval ) ) 
				result = PyCObject_FromVoidPtr( BRPOINTER( inEval ), NULL );
			else  {
				result = Py_None;
				Py_INCREF( result );
			}
			break;

		case AT_VECTOR:
			{
				const slVector &v = BRVECTOR( (brEval*)inEval );
				slVector *vp;

				result = PyObject_CallObject( sPythonData._vectorClass, NULL );

				result->ob_type->tp_as_buffer->bf_getwritebuffer( result, 0, (void**)&vp );

				slVectorCopy( &v, vp );
			}

			break;

		case AT_MATRIX:
			{ 
				const slMatrix &m = BRMATRIX( (brEval*)inEval );

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

				result = PyObject_Call( sPythonData._matrixClass, args, NULL );

				Py_DECREF( args );
			}

			break;

		case AT_LIST:
			{
				brEvalListHead *list = BRLIST( (brEval*)inEval );

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
 * Callback to determine whether an object is PyCObject type or not.
 */

PyObject *brPythonIsCObject( PyObject *inSelf, PyObject *inArgs ) {
	PyObject *pyobject;

	if( !PyArg_ParseTuple( inArgs, "O", &pyobject ) ) 
		return NULL;

	if( PyCObject_Check( pyobject ) ) 
		return PyInt_FromLong( 1 );
	else
		return PyInt_FromLong( 0 );
}

/**
 * Callback to handle stdout/stderr from Python and redirect it to breve output.
 */

PyObject *brPythonCatchOutput( PyObject *inSelf, PyObject *inArgs ) {
	PyObject *moduleObject;
	char *message;

	if( !PyArg_ParseTuple( inArgs, "Os", &moduleObject, &message ) ) 
		return NULL;

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

	PyObject *engineObject = PyObject_GetAttrStringSafe( moduleObject, "breveEngine" );
	PyObject *instanceObject = PyObject_GetAttrStringSafe( pythonInstance, "breveInstance" );

	brEngine *engine = (brEngine*)PyCObject_AsVoidPtr( engineObject );
	brInstance *instance = (brInstance*)PyCObject_AsVoidPtr( instanceObject );

	Py_DECREF( engineObject );
	Py_DECREF( instanceObject );

	if( brEngineSetController( engine, instance ) ) {
		PyErr_SetString( PyExc_RuntimeError, "Could not set controller object for breve simulation" );
		return NULL;
	}

	Py_INCREF( Py_None );
	return Py_None;
}

PyObject *brPythonVectorLength( PyObject *inSelf, PyObject *inArgs ) {
	PyObject *v1;
	slVector *vec1 = NULL;

	if ( !PyArg_ParseTuple( inArgs, "O", &v1 ) ) 
		return NULL;

	v1->ob_type->tp_as_buffer->bf_getreadbuffer( v1, 0, (void**)&vec1 );

	return PyFloat_FromDouble( slVectorLength( vec1 ) );
}

PyObject *brPythonSetVector( PyObject *inSelf, PyObject *inArgs ) {
	PyObject *v1;
	slVector *vec1 = NULL;
	float x, y, z;

	if ( !PyArg_ParseTuple( inArgs, "Offf", &v1, &x, &y, &z ) ) return NULL;

	v1->ob_type->tp_as_buffer->bf_getwritebuffer( v1, 0, (void**)&vec1 );

	vec1->x = x;
	vec1->y = y;
	vec1->z = z;

	Py_INCREF( v1 );
	return v1;
}


PyObject *brPythonScaleVector( PyObject *inSelf, PyObject *inArgs ) {
	slVector *vec1 = NULL, *vec3 = NULL;
	PyObject *v1;
	float f;

	if ( !PyArg_ParseTuple( inArgs, "Of", &v1, &f ) ) return NULL;

	v1->ob_type->tp_as_buffer->bf_getreadbuffer( v1, 0, (void**)&vec1 );

	PyObject *result = PyObject_CallObject( sPythonData._vectorClass, NULL );
	result->ob_type->tp_as_buffer->bf_getwritebuffer( result, 0, (void**)&vec3 );

	vec3->x = vec1->x * f;
	vec3->y = vec1->y * f;
	vec3->z = vec1->z * f;

	return result;
}

PyObject *brPythonAddVectors( PyObject *inSelf, PyObject *inArgs ) {
	slVector *vec1 = NULL, *vec2 = NULL, *vec3 = NULL;
	PyObject *v1, *v2;

	if ( !PyArg_ParseTuple( inArgs, "OO", &v1, &v2 ) ) return NULL;

	v1->ob_type->tp_as_buffer->bf_getwritebuffer( v1, 0, (void**)&vec1 );
	v2->ob_type->tp_as_buffer->bf_getwritebuffer( v2, 0, (void**)&vec2 );

	PyObject *result = PyObject_CallObject( sPythonData._vectorClass, NULL );
	result->ob_type->tp_as_buffer->bf_getwritebuffer( result, 0, (void**)&vec3 );

	vec3->x = vec1->x + vec2->x;
	vec3->y = vec1->y + vec2->y;
	vec3->z = vec1->z + vec2->z;

	return result;
}

PyObject *brPythonSubVectors( PyObject *inSelf, PyObject *inArgs ) {
	slVector *vec1 = NULL, *vec2 = NULL, *vec3 = NULL;
	PyObject *v1, *v2;

		if ( !PyArg_ParseTuple( inArgs, "OO", &v1, &v2 ) ) return NULL;

	v1->ob_type->tp_as_buffer->bf_getwritebuffer( v1, 0, (void**)&vec1 );
	v2->ob_type->tp_as_buffer->bf_getwritebuffer( v2, 0, (void**)&vec2 );

	PyObject *result = PyObject_CallObject( sPythonData._vectorClass, NULL );
	result->ob_type->tp_as_buffer->bf_getwritebuffer( result, 0, (void**)&vec3 );

	vec3->x = vec1->x - vec2->x;
	vec3->y = vec1->y - vec2->y;
	vec3->z = vec1->z - vec2->z;

	return result;
}

/**
 * Python callback to add a Python instance to the breve engine.
 */

PyObject *brPythonAddInstance( PyObject *inSelf, PyObject *inArgs ) {
	PyObject *instance, *moduleObject, *typeObject, *result;
	brObject *breveObject;

	if ( !PyArg_ParseTuple( inArgs, "OOO", &moduleObject, &typeObject, &instance ) ) 
		return NULL;

	// Extract the breveEngine from the module data

	PyObject *engineObject = PyObject_GetAttrStringSafe( moduleObject, "breveEngine" );

	if( !engineObject ) {
		PyErr_SetString( PyExc_RuntimeError, "Could not locate breve engine in first internal breve call argument" );
		return NULL;
	}

	brEngine *engine = ( brEngine* )PyCObject_AsVoidPtr( engineObject );

	Py_DECREF( engineObject );

	// Adding the instance requires a brObject type.
	// We may need to create it ourselves.

	PyObject *breveTypeObject = PyObject_GetAttrStringSafe( typeObject, "breveObject" );
	PyObject *nameObject = PyObject_GetAttrStringSafe( typeObject, "__name__" );
	char *name = PyString_AsString( nameObject );

	Py_DECREF( nameObject );

	if( breveTypeObject ) {
		// Found an existing breve object 
		breveObject = (brObject*)PyCObject_AsVoidPtr( breveTypeObject );
		Py_DECREF( breveTypeObject );
	}

	// If there was no breveObject, or if the name doesn't match 
	// (which happens when a base class has been added, but not the child),
	// We will add a new object for this class:

	if( !breveTypeObject || strcmp( name, breveObject->name ) ) {
		// This object type has not previously been added to the breve engine.

		PyObject *pythonLanguageType = PyObject_GetAttrString( moduleObject, "breveObjectType" );

		breveObject = brObjectFindWithPreferredType( engine, name, PYTHON_TYPE_SIGNATURE );

		if( !breveObject ) {
			// The engine will hold a copy of typeObject, so we INCREF
			Py_INCREF( typeObject );
			breveObject = brEngineAddObject( engine, (brObjectType*)PyCObject_AsVoidPtr( pythonLanguageType ), name, typeObject );
		}

		PyObject *ptr = PyCObject_FromVoidPtr( breveObject, NULL );
		PyObject_SetAttrString( typeObject, "breveObject", ptr );
		Py_DECREF( ptr );

		Py_DECREF( pythonLanguageType );

	} 

	if( !breveObject || breveObject -> type -> _typeSignature != PYTHON_TYPE_SIGNATURE ) {
		PyErr_SetString( PyExc_RuntimeError, "Could not add Python instance to breve engine" );
		return NULL;
	}

	// Now the engine has a copy of object -- increment the reference count
	
	Py_INCREF( instance );
	brInstance *i = brEngineAddInstance( engine, breveObject, instance );

	if( i ) {
		result = PyCObject_FromVoidPtr( i, NULL );
	} else {
		result = Py_None;
		Py_INCREF( result );
	}

	return result;
}

/**
 * Python callback to remove a Python instance from the breve engine.
 */

PyObject *brPythonRemoveInstance( PyObject *inSelf, PyObject *inArgs ) {
	PyObject *engineObject, *instance;

	if ( !PyArg_ParseTuple( inArgs, "OO", &engineObject, &instance ) ) return NULL;

	PyObject *breveObject = PyObject_GetAttrStringSafe( instance, "breveInstance" );

	if( breveObject ) {
		brInstance *instance = ( brInstance* )PyCObject_AsVoidPtr( breveObject );

		brInstanceRelease( instance );

		Py_DECREF( breveObject );
	}

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

	if ( !PyArg_ParseTuple( inArgs, "Os", &breveModule, &name ) ) 
		return NULL;

	brEngine *engine = ( brEngine* )PyCObject_AsVoidPtr( breveModule );

	brInternalFunction *function = brEngineInternalFunctionLookup( engine, name );

	if ( !function ) {
		std::string error( "could not locate internal breve method \"" );
		error += name + std::string( "\"" );

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

	PyObject *callerObject, *functionObject, *arguments;

	if ( !PyArg_ParseTuple( inArgs, "O!O!", &PyCObject_Type, &functionObject, &PyTuple_Type, &arguments ) ) 
		return NULL;

	brInternalFunction *function = ( brInternalFunction* )PyCObject_AsVoidPtr( functionObject );

	if ( !function ) {
		PyErr_SetString( PyExc_RuntimeError, "Could not execute internal breve function" );
		return NULL;
	}

	callerObject = PyTuple_GET_ITEM( arguments, 0 );

	if ( !callerObject ) {
		PyErr_SetString( PyExc_RuntimeError, "Could not execute internal breve function" );
		return NULL;
	}

	PyObject *breveObject = PyObject_GetAttrStringSafe( callerObject, "breveInstance" );

	if( !breveObject ) {
		PyErr_SetString( PyExc_RuntimeError, "Internal breve function called for Python object not in breve engine (missing call to __init__ for the parent class?)" );
		return NULL;
	}

	if( breveObject == Py_None ) {
		PyErr_SetString( PyExc_RuntimeError, "internal function called for invalid instance" );
		return NULL;
	}

	brInstance *caller = ( brInstance* )PyCObject_AsVoidPtr( breveObject );

	Py_DECREF( breveObject );

	if( caller->status != AS_ACTIVE ) {
		slMessage( DEBUG_ALL, "Warning: internal function called for freed instance %p\n", caller );
		Py_INCREF( Py_None );
		return Py_None;
	}

	if( function->_argCount != 0 ) {

		int argCount = PyTuple_GET_SIZE( arguments ) - 1;

		if( argCount != function->_argCount ) {
			char err[ 1024 ];
			snprintf( err, 1023, "internal function \"%s\" given %d arguments, expects %d", function->_name.c_str(), argCount, function->_argCount );
			PyErr_SetString( PyExc_RuntimeError, err );
			return NULL;
		}

		for( int n = 0; n < argCount; n++ ) {
			brPythonTypeToEval( PyTuple_GET_ITEM( arguments, n + 1 ), &args[ n ] );

			if( function->_argTypes[ n ] != AT_UNDEFINED && args[ n ].type() != function->_argTypes[ n ] ) {

				// int <=> double conversion 

				if( args[ n ].type() == AT_INT && function->_argTypes[ n ] == AT_DOUBLE ) {
					 args[ n ].set( (double)BRINT( &args[ n ] ) );
				} else if( args[ n ].type() == AT_DOUBLE && function->_argTypes[ n ] == AT_INT) {
					 args[ n ].set( (int)BRDOUBLE( &args[ n ] ) );
				} else {
					char err[ 1024 ];
					snprintf( err, 1023, "invalid type for argument %d of internal function \"%s\" (got \"%s\", expected \"%s\" )", n, function->_name.c_str(), brAtomicTypeStrings[ args[ n ].type() ], brAtomicTypeStrings[ function->_argTypes[ n ] ] );

					PyErr_SetString( PyExc_RuntimeError, err );
					return NULL;
				}
			}
		}
	}

	try {
		function->_call( args, &resultEval, caller );
	} catch( slException &error ) {
		PyErr_SetString( PyExc_RuntimeError, error._message.c_str() ); 
                return NULL;
	}

	PyObject *result = brPythonTypeFromEval( &resultEval, sPythonData._internalModule );


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
	std::string altname;
	char err[ 1024 ];

	if ( !PyArg_ParseTuple( inArgs, "Os", &object, &name ) ) 
		return NULL;

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
//  Python language frontend callbacks for the breveObjectType structure		   //
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

	// Much to my surprise, we're getting a buffer overrun with the GetAttrString
	// on the c_str() -- make a local copy instead !?
	char *name = strdup( brPyConvertSymbol( symbol ).c_str() );

	PyObject *method = PyObject_GetAttrStringSafe( type, name );

	free( name );

	if ( !method || !PyCallable_Check( method ) ) {
		if( method ) 
			Py_DECREF( method );

		return NULL;
	}

	PyObject *code = PyObject_GetAttrStringSafe( method, "func_code" );
	
	if( !code ) {
		Py_DECREF( method );
		return NULL;
	}

	PyObject *argumentCount = PyObject_GetAttrStringSafe( code, "co_argcount" );

	if( !argumentCount || PyInt_AS_LONG( argumentCount ) != ( inCount + 1 ) ) {
		Py_DECREF( method );
		Py_DECREF( code );

		if( argumentCount )
			Py_DECREF( argumentCount );

		return NULL;
	}

	slPythonMethodInfo *info = new slPythonMethodInfo();

	// Hang on to the method reference returned by GetAttrString

	info->_method = method;
	info->_argumentCount = PyInt_AS_LONG( argumentCount );

	Py_DECREF( argumentCount );
	Py_DECREF( code );

	return info;
}

/**
 * A breveObjectType callback to locate a class in Python.
 * 
 * @param inData 	The userdata callback pointer to a __main__ module 
 * @param inName	The name of the desired object
 */

void *brPythonFindObject( void *inData, const char *inName ) {
	PyObject *mainModule = ( PyObject* )inData;

	// Note that we are letting a reference escape here, because
	// the breve engine is going to hold on to this pointer and 
	// will call the destructor when it's done.

	PyObject *o = PyObject_GetAttrStringSafe( sPythonData._breveModule, (char*)inName );

	if( o ) 
		return o;

	return PyObject_GetAttrStringSafe( mainModule, (char*)inName );
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
	static int icount = 0;

	icount++;

	PyObject *object = ( PyObject* )inObject->userData;

	PyObject *args = PyTuple_New( 0 );
	PyObject *instance = PyObject_CallObject( object, args );

	Py_DECREF( args );

	if ( PyErr_Occurred() ) {
		PyErr_Print();
		return NULL;
	}

	PyObject *breveObject = PyObject_GetAttrStringSafe( instance, "breveInstance" );

	if( !breveObject ) {
		// This is a fundamental rule of the Python frontend: objects must add 
		// themselves to the breve engine, or else they cannot be used

		slMessage( DEBUG_ALL, "The new Python instance was not added to breve engine\n" );
		slMessage( DEBUG_ALL, "breve Python objects must inherit from class breve.object\n" );
		return NULL;
	}

	// This is tricky: the instance we just created has added itself to the engine 
	// via a callback, so we can safely release the reference we just created.

	Py_DECREF( instance );

	brInstance *result = (brInstance*)PyCObject_AsVoidPtr( breveObject );

	Py_DECREF( breveObject );

	return result;
}

/**
 * A breveObjectType callback to call a method in Python
 * 
 * @param inInstance	The object instance data (as found with \ref brPythonInstantiate)
 * @param inMethod	The method callback data (as found with \ref brPythonFindMethod )
 * @param inArguments	NOT CURRENTLY IMPLEMENTED
 * @param outResult	The brEval result of the method call
 */

int brPythonCallMethod( void *inInstance, void *inMethod, const brEval **inArguments, brEval *outResult ) {
	slPythonMethodInfo *info = (slPythonMethodInfo*)inMethod;

	PyObject *instance = ( PyObject* )inInstance;
	PyObject *method = info->_method;

	// static PyObject *tuples[ 5 ] = { PyTuple_New( 1 ), PyTuple_New( 2 ), PyTuple_New( 3 ), PyTuple_New( 4 ), PyTuple_New( 5 ) };

	int count = info->_argumentCount - 1;

	PyObject *tuple = PyTuple_New( count + 1 );

	// Set the self argument (SetItem steals a reference)

	Py_INCREF( instance );
	int r = PyTuple_SetItem( tuple, 0, instance );

	if( r != 0 ) {
		PyErr_Print();
		return EC_ERROR;
	}

	// Set the rest of the arguments

	for( int n = 0; n < count; n++ ) {
		PyObject *argument = brPythonTypeFromEval( inArguments[ n ], sPythonData._breveModule );
		PyTuple_SetItem( tuple, n + 1, argument );
	}

	PyObject *result = PyObject_Call( method, tuple, NULL );

	Py_DECREF( tuple );

	if( !result ) {
		PyErr_Print();
		return EC_ERROR;
	} else {
		brPythonTypeToEval( result, outResult );
		Py_DECREF( result );
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


	const std::vector< std::string > &paths = brEngineGetSearchPaths( inEngine );

	PyRun_SimpleString( "import breve" );

	for( unsigned int n = 0; n < paths.size(); n++ ) {
		std::string setpath = "breve.usepath( '" + paths[ n ] + "' ) ";
		PyRun_SimpleString( setpath.c_str() );
	}



	FILE *fp = fopen( inFilename, "r" );

	if( fp ) {
		if( PyRun_SimpleFile( fp, inFilename ) ) {
			brEvalError( inEngine, PE_PYTHON, "An error occurred while executing the file \"%s\"\n", inFilename );
			result = EC_ERROR;
		}

		fclose( fp );

	} else {
		std::string processed( inFiletext );

		for( unsigned int n = 0; n < processed.size(); n++ ) 
			if( processed[ n ] == '\r' )
				processed[ n ] = '\n';

		if( PyRun_SimpleString( processed.c_str() ) ) {
			brEvalError( inEngine, PE_PYTHON, "An error occurred while executing the Python code\n", inFilename );
			result = EC_ERROR;
		}
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
	if( !inObject ) 
		return;

	PyObject *object = ( PyObject* )inObject;
	Py_DECREF( object );
}

/**
 * Destroy the Python interpreter
 */

void brPythonDestroy( void *inObject ) {
	Py_DECREF( sPythonData._vectorClass );
	Py_DECREF( sPythonData._matrixClass );
	Py_DECREF( sPythonData._breveModule );
	Py_DECREF( sPythonData._mainModule );

	Py_Finalize();
}

/**
 * Setup the sys.argv variable in Python.
 */

void brPythonSetArgv( brEngine *inEngine, PyObject *inSysModule ) {
	PyObject *argv;

	argv = PyList_New( 0 );

	for( int n = 0; n < inEngine->_argc; n++ ) {
		PyObject *str = PyString_FromString( inEngine->_argv[ n ] );

		PyList_Append( argv, str );

		Py_DECREF( str );
	}

	PyObject_SetAttrString( inSysModule, "argv", argv );

 	Py_DECREF( argv );
}

char *brPythonEncodeToString( brEngine *inEngine, void *inData ) {
	PyObject *obj = (PyObject*)inData;

	PyObject *encoder = PyObject_GetAttrStringSafe( sPythonData._breveModule, "encodeToString" );

	if( !encoder )
		return NULL;

	PyObject *args = PyTuple_New( 1 );

	// SetItem steals a reference
	Py_INCREF( obj );
	PyTuple_SetItem( args, 0, obj );

	PyObject *result = PyObject_Call( encoder, args, NULL );

	Py_DECREF( args );
	Py_DECREF( encoder );

	if( !result ) {
		PyErr_Print();
		return NULL;
	}

	char *encoding = slStrdup( PyString_AsString( result ) );

	Py_DECREF( result );

	return encoding;
}

void *brPythonDecodeFromString( brEngine *inEngine, char *inString ) {
	PyObject *decoder = PyObject_GetAttrStringSafe( sPythonData._breveModule, "decodeFromString" );

	if( !decoder )
		return NULL;

	PyObject *args = PyTuple_New( 1 );

	PyTuple_SetItem( args, 0, PyString_FromString( inString ) );

	PyObject *result = PyObject_Call( decoder, args, NULL );

	Py_DECREF( args );
	Py_DECREF( decoder );

	return result;
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
		{ "setController", 		brPythonSetController, 			METH_VARARGS, "" }, 
		{ "findInternalFunction", 	brPythonFindInternalFunction, 		METH_VARARGS, "" }, 
		{ "callInternalFunction", 	brPythonCallInternalFunction, 		METH_VARARGS, "" },
		{ "addInstance", 		brPythonAddInstance, 			METH_VARARGS, "" },
		{ "removeInstance", 		brPythonRemoveInstance, 		METH_VARARGS, "" },
		{ "findBridgeMethod", 		brPythonFindBridgeMethod, 		METH_VARARGS, "" },
		{ "callBridgeMethod", 		brPythonCallBridgeMethod, 		METH_VARARGS, "" },
		{ "catchOutput", 		brPythonCatchOutput, 			METH_VARARGS, "" },
		{ "addVectors", 		brPythonAddVectors, 			METH_VARARGS, "" },
		{ "subVectors", 		brPythonSubVectors, 			METH_VARARGS, "" },
		{ "scaleVector", 		brPythonScaleVector, 			METH_VARARGS, "" },
		{ "setVector", 			brPythonSetVector, 			METH_VARARGS, "" },
		{ "vectorLength",		brPythonVectorLength, 			METH_VARARGS, "" },
		{ "isCObject",			brPythonIsCObject, 			METH_VARARGS, "" },
		{ NULL, NULL, 0, NULL }
	};

#ifdef HAVE_LIBPYTHON24
	Py_InitializeEx( 0 );
#else
	Py_Initialize();
#endif

	sPythonData._internalModule = Py_InitModule( "breveInternal", methods );

	PyObject *ptr = PyCObject_FromVoidPtr( ( void* )breveEngine, NULL );
	PyObject_SetAttrString( sPythonData._internalModule, "breveEngine", ptr );
	Py_DECREF( ptr );

	ptr = PyCObject_FromVoidPtr( ( void* )brevePythonType, NULL );
	PyObject_SetAttrString( sPythonData._internalModule, "breveObjectType", ptr );
	Py_DECREF( ptr );

	const std::vector< std::string > &paths = brEngineGetSearchPaths( breveEngine );

	PyRun_SimpleString( "import sys" );

	for( unsigned int n = 0; n < paths.size(); n++ ) {
		std::string setpath = "sys.path.append( '" + paths[ n ] + "' ) ";
		PyRun_SimpleString( setpath.c_str() );
	}

	sPythonData._mainModule  = PyImport_ImportModule( "__main__" );
	sPythonData._breveModule = PyImport_ImportModule( "breve" );

	if( !sPythonData._breveModule ) {
		PyErr_Print();
		slMessage( DEBUG_ALL, "Failed to initialize Python frontend.  breve will not be able to execute Python files.\n" );

		Py_Finalize();
		return;
	}

	PyObject *sys = PyImport_ImportModule( "sys" );
	brPythonSetArgv( breveEngine, sys );
	Py_DECREF( sys );


	sPythonData._vectorClass = PyObject_GetAttrStringSafe( sPythonData._breveModule, "vector" );
	sPythonData._matrixClass = PyObject_GetAttrStringSafe( sPythonData._breveModule, "matrix" );

	brevePythonType->userData 		= (void*)sPythonData._mainModule;

	brevePythonType->findMethod 		= brPythonFindMethod;
	brevePythonType->findObject 		= brPythonFindObject;
	brevePythonType->instantiate 		= brPythonInstantiate;
	brevePythonType->callMethod 		= brPythonCallMethod;
	brevePythonType->isSubclass 		= brPythonIsSubclass;
	brevePythonType->destroyObject 		= brPythonDestroyGenericPythonObject;
	brevePythonType->destroyMethod 		= brPythonDestroyGenericPythonObject;
	brevePythonType->destroyInstance 	= brPythonDestroyGenericPythonObject;
	brevePythonType->destroyObjectType	= brPythonDestroy;
	brevePythonType->canLoad		= brPythonCanLoad;
	brevePythonType->load			= brPythonLoad;
	brevePythonType->encodeToString		= brPythonEncodeToString;
	brevePythonType->decodeFromString	= brPythonDecodeFromString;
	brevePythonType->_typeSignature 	= PYTHON_TYPE_SIGNATURE;

	brEngineRegisterObjectType( breveEngine, brevePythonType );

 	breveInitPythonFunctions( breveEngine->internalMethods );

#ifndef WINDOWS
	// Those meddling Python kids mess with the signal handlers!
	signal( SIGINT, NULL );
#endif
}

#endif
