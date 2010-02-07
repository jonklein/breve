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

/*****************************************************************************
 *                                                                           *
 * Ask Kyle Harrington about the Scheme interface <kih03@hampshire.edu>      *
 *                                                                           *
 *****************************************************************************/

#include "kernel.h"
#include "scheme.h"

#ifdef HAVE_LIBGUILE

#include <guile/gh.h>
#include <signal.h>

//
// The brSchemeData class is a simple class we use for keeping track
// of method and object lookups.  It's subclasses will be the void 
// data ptr that is returned to the breve engine.
//

class brSchemeData {
public:
  virtual		~brSchemeData() {}
};

class brSchemeMethodData : public brSchemeData {
public:
  brSchemeMethodData( SCM inObj, int inArgCount ) { _method = inObj; _argcount = inArgCount; }

  SCM            _method;
  int		_argcount;
};

class brSchemeObjectData : public brSchemeData {
public:
  brSchemeObjectData( SCM inObj ) { _object = inObj; }

  SCM	_object;
};

class brSchemeInstanceData : public brSchemeData {
public:
  brSchemeInstanceData( SCM inInstance ) { _instance = inInstance; }

  SCM	_instance;
};
inline SCM brSchemeObjectFromEval( const brEval *inEval, SCM inBridgeObject );
inline int brSchemeObjectToEval( SCM inObject, brEval *outEval );

brEvalListHead *brSchemeToBreveListConvert( SCM inObject ) {
  brEvalListHead *list = new brEvalListHead;

  brEval eval;

  while( type_of( inObject ) == t_cons ) {
    brSchemeObjectToEval( scm_car(inObject), &eval );
		
    if( brEvalListAppend( list, &eval ) != EC_OK ) {
      delete list;
      return NULL;
    }

    inObject = scm_cdr(inObject);
  }

  return list;
}

SCM brSchemeFromBreveListConvert( brEvalListHead *inList, int inStartIndex = 0 ) {
	SCM list = scm_list_1( 0 );

	for( int n = inList -> _vector.size() - 1; n > -1; n-- ) {
		SCM l = brSchemeObjectFromEval( &inList -> _vector[ n ], NULL );
		list = scm_cons( l, list );
	}

	return list;
}


/**
 * Translation function from a Scheme object to a breve value.
 * 
 * @param inSchemeVal		The Scheme value to be translated
 * @param inEval		The Scheme value to be translated.
 * @return			EC_ERROR or EC_OK
 */


inline int brSchemeObjectToEval( SCM inObject, brEval *outEval ) {
	switch( type_of( inObject ) ) {
		case t_cons:
		  outEval -> set( brSchemeToBreveListConvert( inObject ) );
		  break;

		case t_base_string:
		case t_symbol:
		  outEval -> set( scm_to_locale_string(inObject) );
		  break;

		case t_singlefloat:
		case t_doublefloat:
		  outEval -> set( scm_to_double(inObject) );
		  break;

		  //case t_fixnum:
		  //	outEval -> set( fix( inObject ) );
		  //	break;

		// case t_character:
		// 	outEval -> set( 0 );
		// 	break;

		default:
			slMessage( DEBUG_ALL, "Error scheme type %d conversion to breve not imeplemented\n", type_of( inObject ) );
			return EC_ERROR;
	}

	return EC_OK;
}

/**
 * Translation function from a breve value to a Scheme object.
 * 
 * @param inEval		The breve value to be translated.
 * @param inBridgeObject
 * @return			A newly created Scheme object translated from the source value.
 */

inline SCM brSchemeObjectFromEval( const brEval *inEval, SCM inBridgeObject ) {
	switch( inEval -> type() ) {
		case AT_INT:
			return scm_from_signed_integer( BRINT( inEval ) );
			break;
		case AT_DOUBLE:
			return scm_from_double( BRDOUBLE( inEval ) );
			break;
		case AT_VECTOR:
			{
			  slVector *v = &BRVECTOR( inEval );
			  return scm_list_3(scm_from_double( v -> x ),
					    scm_from_double( v -> y ),
					    scm_from_double( v -> z ) );
			}
		case AT_LIST:
			return brSchemeFromBreveListConvert( BRLIST( inEval ) );
			break;
		default:
			slMessage( DEBUG_ALL, "Error: breve type %d conversion to Scheme not implemented\n", inEval -> type() );
			break;
	}

	return NULL;
}







/////////////////////////////////////////////////////////////////////////////////////                                                
//  Scheme language frontend callbacks for the breveObjectType structure           //
/////////////////////////////////////////////////////////////////////////////////////                                                  


/**
 * A breveObjectType callback to locate a method in a Scheme object.
 * 
 * @param inObject 	A void pointer to a Scheme object.
 * @param inName	The name of the method to locate.
 * @param inTypes 	Argument types (currently unused)
 * @param inCount	The argument count
 * 
 * @return 			A void pointer to a Scheme method object. 
 */

brMethodCallbackData *brSchemeFindMethod( brObjectCallbackData* inObject, const char *inName, unsigned char *inTypes, int inCount ) {
	// No actual method lookup here -- just turn the name into a symbol and let 
	// the runtime handle any errors.

	SCM cmd = scm_from_locale_string( inName );

	return new brSchemeMethodData( cmd, inCount );
}

/**
 * A breveObjectType callback to locate a class in Scheme.
 * 
 * @param inData 	The userdata callback pointer to a __main__ module 
 * @param inName	The name of the desired object
 */

brObjectCallbackData *brSchemeFindObject( brFrontendCallbackData *inData, const char *inName ) {
	std::string command;
	command = std::string( "( find-class '" ) + inName + ")";

	SCM result = scm_eval_string( command.c_str() );

	return ( result != NULL ) ? (void*)( new brSchemeObjectData( result ) ) : NULL;
}

/**
 * A breveObjectType callback to instantiate a class in Scheme
 * 
 * @param inEngine	The breve engine creating the instancce.
 * @param inObject	The object to be instantiated.
 * @param inArgs	Constructor arguments -- not currently used.
 * @param inArgCount	Constructor argument count -- not currently used.
 */

brInstance *brSchemeInstantiate( brEngine *inEngine, brObject* inObject, const brEval **inArgs, int inArgCount ) {
	std::string command( "( make-instance '" );
	command += inObject -> name;
	command += ")";

	SCM result = scm_eval_string( command.c_str() );

	brInstance *instance = NULL;
	
	if( result ) 
		instance = brEngineAddInstance( inEngine, inObject, new brSchemeInstanceData( result ) );

	return instance;
}

/**
 * A breveObjectType callback to call a method in Scheme
 * 
 * @param inInstance	The object instance data (as found with \ref brSchemeInstantiate)
 * @param inMethod	The method callback data (as found with \ref brSchemeFindMethod )
 * @param outResult	The brEval result of the method calll
 */

int brSchemeCallMethod( brInstanceCallbackData *inInstance, brMethodCallbackData *inMethod, const brEval **inArguments, brEval *outResult ) {
	brSchemeMethodData *methodData = (brSchemeMethodData*)inMethod;
	brSchemeInstanceData *instanceData = (brSchemeInstanceData*)inInstance;

	SCM instance = instanceData -> _instance;
	SCM method = methodData -> _method;
	SCM list = scm_list_1( 0 );
	
	for( int i = methodData -> _argcount - 1; i >= 0; i-- ) {
		// go backwards, cons each argument onto the method execution list
		SCM arg = brSchemeObjectFromEval( inArguments[ i ], NULL );

		if( !arg ) {
			slMessage( DEBUG_ALL, "An error occurred converting the method arguments to Scheme\n" );
			return EC_ERROR;
		}

		list = scm_cons( brSchemeObjectFromEval( inArguments[ i ], NULL ), list );
	}

	// cons on the instance as the first argument, and then the method symbol

	list = scm_cons( instance, list );
	list = scm_cons( method,   list );

	SCM result = scm_eval( list );

	if( result != NULL ) {
		brSchemeObjectToEval( result, outResult );
		return EC_OK;
	} else {
		slMessage( DEBUG_ALL, "An error occurred executing the Scheme method \n");
	}

	return EC_ERROR;
}

/**
 * A brObjectType callback to determine whether one object is a subclass of another.  Used by 
 * collision detection to determine if a handler is installed for an object pair.
 * 
 * Clearly not implemented at the moment.
 */

int brSchemeIsSubclass( brObjectType *inObjectType,  brObjectCallbackData *inClassA, brObjectCallbackData *inClassB ) {
	return 0;
}

/**
 * The Scheme canLoad breve object callback
 */

int brSchemeCanLoad( brFrontendCallbackData *inData, const char *inExtension ) {
	// check the file extension

	if( !strcasecmp( inExtension, "l" ) 
		|| !strcasecmp( inExtension, "Scheme" ) 
		|| !strcasecmp( inExtension, "brevel" ) )
			return 1;

	return 0;
}

/**
 * The Scheme load breve object callback
 */

int brSchemeLoad( brEngine *inEngine, brFrontendCallbackData *inData, const char *inFilename, const char *inFiletext ) {
	std::string command = "( PROGN ";
	command += inFiletext;
	command += " ) ";

	SCM result = scm_eval_string( command.c_str() );
	return result == NULL ? EC_ERROR : EC_OK;
}

/**
 * A brObjectType callback to clean up a generic Scheme object.  This method is used as the 
 * destructor for both objects and methods
 *
 * @param inObject		A void pointer to a Scheme object.
 */

void brSchemeDestroyGenericSchemeObject( void *inObject ) {
	if( inObject ) 
		delete (brSchemeData*)inObject;
}

/**
 * brSchemeFunctionCallback takes a list of the form:
 *  (functionName arg1 arg2 ...)
 * and the respective scheme call appears as:
 *  (breve (functionName arg1 arg2 ...))
 */
SCM brSchemeFunctionCallback(SCM lst) {
  if( scm_length( lst ) < 1 ) {
    slMessage( DEBUG_ALL, "Internal error: Missing Scheme function name\n");
    return -1;
  }

  char *instanceName = scm_to_locale_string( scm_car( lst ) );
  char *functionName = scm_to_locale_string( scm_car( scm_cdr( lst ) ) );

  //Tsk, tsk... global variable... for shame...
  brEngine *e = globalengine;

  brInternalFunction *brFunction = brEngineInternalFunctionLookup( e, functionName );

  int provided_args = scm_length( lst ) - 1;

  if( provided_args < brFunction->_argCount ) {
    slMessage( DEBUG_ALL, "Internal error: Insufficient arguments for function: %s, %d required, %d given.\n", functionName, brFunction->_argCount, provided_args );
    return -1;
  }

  brEval args[ provided_args ];
  brEval target;
  brInstance *bi = brSchemeFindObject( NULL, objectName );

  for( unsigned i = 0; i < provided_args; i++) {
    brSchemeObjectToEval( scm_list_ref( lst, i + 1 ), &args[i] );
  }
  
  if( brFunction->_call( args, &target, bi ) ){
    slMessage( DEBUG_ALL, "Internal error: Error evaluating function: %s.\n", functionName);
    return -1;
  }  
}

void brSchemeRegister( ) {
  scm_c_define_gsubr( "breve", 1, 0, 0, brSchemeFunctionCallback );
}

int brSchemeInit( brEngine *breveEngine ) {
  // Link our global brEngine variable to this engine
  globalengine = breveEngine;

	// Boot up the Scheme interpreter 

	char *argv[ 1 ] = { "breve" };
	gh_enter( 1, argv, brSchemeRegister );

	/*	// Scheme messes with our signal handlers!  We cannot allow that.

	signal( SIGFPE,  NULL );
	signal( SIGSEGV, NULL );
	signal( SIGINT,  NULL );*/

	// Register the Scheme object type

	brObjectType *breveSchemeType = new brObjectType();

	breveSchemeType -> findMethod 		= brSchemeFindMethod;
	breveSchemeType -> findObject 		= brSchemeFindObject;
	breveSchemeType -> instantiate 		= brSchemeInstantiate;
	breveSchemeType -> callMethod 		= brSchemeCallMethod;
	breveSchemeType -> isSubclass 		= brSchemeIsSubclass;
	breveSchemeType -> destroyObject 		= brSchemeDestroyGenericSchemeObject;
	breveSchemeType -> destroyMethod 		= brSchemeDestroyGenericSchemeObject;
	breveSchemeType -> destroyInstance 	= brSchemeDestroyGenericSchemeObject;
	breveSchemeType -> canLoad		= brSchemeCanLoad;
	breveSchemeType -> load			= brSchemeLoad;
	breveSchemeType -> _typeSignature 	= SCHEME_TYPE_SIGNATURE;

	brEngineRegisterObjectType( breveEngine, breveSchemeType );

	return EC_OK;
}

#endif
