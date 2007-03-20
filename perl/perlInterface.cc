#include "kernel.h"

#ifdef HAVE_LIBPERL

//#include "python.h"
#include "perlInterface.h"

SV *brPerlTypeFromEval( const brEval *inEval, SV ***prevStackPtr ) {
// actually returns an SV*, AV*, or HV* (all sizeof(int) types though)

    brEvalListHead *list;
    brInstance *breveInstance;
    SV *result = NULL;
    register SV **sp = *prevStackPtr;
    
    switch ( inEval->type() ) {
	case AT_NULL:
		result = 0;
		break;

	case AT_INT:
		printf("Coercing into perlType AT_INT.\n");
		result = sv_2mortal(newSViv(BRINT(inEval)));
		break;

	case AT_DOUBLE:
		printf("Coercing into perlType AT_DOUBLE.\n");
		result = sv_2mortal(newSVnv(BRDOUBLE( inEval ) ));
		break;

	case AT_STRING:   // perl will calculate string length using strlen
		printf("Coercing into perlType AT_STRING.\n");
		result = sv_2mortal(newSVpv(BRSTRING(inEval), 0 ) ); 
		break;

	case AT_LIST:
		list = BRLIST( inEval );
		unsigned int n;
		SV * ret;
		for(n = 1; n < list->_vector.size(); n++ ) {
			ret = brPerlTypeFromEval(&list->_vector[n], &sp);
			XPUSHs(ret);
		}
		ret = brPerlTypeFromEval(&list->_vector[2],&sp);
		XPUSHs(ret);
		ret = brPerlTypeFromEval(&list->_vector[2],&sp);
		XPUSHs(ret);   // callMethod pushes the first value
		result = brPerlTypeFromEval(&list->_vector[0], &sp);
		*prevStackPtr = sp;
		break;
			
	case AT_INSTANCE:
		breveInstance = BRINSTANCE( inEval );// Is this a native type, or should we make a bridge of it?
            
		if( breveInstance && breveInstance->object->type->_typeSignature == PERL_TYPE_SIGNATURE ) {
			result = breveInstance->userData; // this is the sv, i don't think it has to be mortalized
			SvREFCNT_inc((sv*)breveInstance->userData); // TODO not sure if this is necessary.
		} else if( breveInstance ) {

		} else {
			result = NULL;
		}

		break;
			
	case AT_POINTER:
	case AT_VECTOR:
	case AT_MATRIX:
	case AT_ARRAY:
	case AT_DATA:
	case AT_HASH:
	default:
		slMessage( DEBUG_ALL, "Could not convert breve internal type \"%d\" to a Perl type\n", inEval->type() );


		break;
	}

	return result;
}

/* copying from python below */
/*
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

double PyNumber_AS_DOUBLE( PyObject *inObject ) {

	if( PyInt_Check( inObject ) ) 
       		return (double)PyInt_AS_LONG( inObject );

	return PyFloat_AS_DOUBLE( inObject );
}

inline PyObject *PyObject_GetAttrStringSafe( PyObject *inObject, char *inString  ) {
	PyObject *result = PyObject_GetAttrString( inObject, inString );

	if( !result )
		PyErr_Clear();

	return result;
}
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
PyObject *brPythonCatchOutput( PyObject *inSelf, PyObject *inArgs ) {
	PyObject *moduleObject;
	char *message;

	if( !PyArg_ParseTuple( inArgs, "Os", &moduleObject, &message ) ) return NULL;

	slFormattedMessage( DEBUG_ALL, message );

	Py_INCREF( Py_None );
	return Py_None;
}
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
	// Figure out the brObject type for this instance.  We may need to create it ourselves.

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
*/

#endif
