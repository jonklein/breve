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
#include "lisp.h"

#ifdef HAVE_LIBECL

#include <ecl/ecl.h>
#include <signal.h>

//
// TODO: ECL garbage collection is completely undocumented at present!
//


//
// The brLispData class is a simple class we use for keeping track
// of method and object lookups.  It's subclasses will be the void 
// data ptr that is returned to the breve engine.
//

class brLispData {
	public:
		virtual		~brLispData() {}
};

class brLispMethodData : public brLispData {
	public:
			brLispMethodData( cl_object inObj, int inArgCount ) { _method = inObj; _argcount = inArgCount; }

	cl_object	_method;
	int		_argcount;
};

class brLispObjectData : public brLispData {
	public:
			brLispObjectData( cl_object inObj ) { _object = inObj; }

	cl_object	_object;
};

class brLispInstanceData : public brLispData {
	public:
			brLispInstanceData( cl_object inInstance ) { _instance = inInstance; }

	cl_object	_instance;
};
inline cl_object brLispObjectFromEval( const brEval *inEval, cl_object inBridgeObject );
inline int brLispObjectToEval( cl_object inObject, brEval *outEval );

brEvalListHead *brLispToBreveListConvert( cl_object inObject ) {
	brEvalListHead *list = new brEvalListHead;

	brEval eval;

	while( type_of( inObject ) == t_cons ) {
		brLispObjectToEval( inObject -> cons.car, &eval );
		
		if( brEvalListAppend( list, &eval ) != EC_OK ) {
			delete list;
			return NULL;
		}

		inObject = inObject -> cons.cdr;
	}

	return list;

}

cl_object brLispFromBreveListConvert( brEvalListHead *inList, int inStartIndex = 0 ) {
	cl_object list = cl_list( 0 );

	for( int n = inList -> _vector.size() - 1; n > -1; n-- ) {
		cl_object l = brLispObjectFromEval( &inList -> _vector[ n ], NULL );
		list = cl_cons( l, list );
	}

	return list;
}


/**
 * Translation function from a Lisp object to a breve value.
 * 
 * @param inLispVal		The lisp value to be translated
 * @param inEval		The lisp value to be translated.
 * @return			EC_ERROR or EC_OK
 */


inline int brLispObjectToEval( cl_object inObject, brEval *outEval ) {
	switch( type_of( inObject ) ) {
		case t_cons:
			outEval -> set( brLispToBreveListConvert( inObject ) );
			break;

		case t_base_string:
		case t_symbol:
			outEval -> set( inObject -> base_string.self );
			break;

		case t_doublefloat:
			outEval -> set( inObject -> DF.DFVAL );
			break;

		case t_singlefloat:
			outEval -> set( inObject -> SF.SFVAL );
			break;

		case t_fixnum:
			outEval -> set( fix( inObject ) );
			break;

		// case t_character:
		// 	outEval -> set( 0 );
		// 	break;

		default:
			slMessage( DEBUG_ALL, "Error lisp type %d conversion to breve not imeplemented\n", type_of( inObject ) );
			return EC_ERROR;
	}

	return EC_OK;
}

/**
 * Translation function from a breve value to a Lisp object.
 * 
 * @param inEval		The breve value to be translated.
 * @param inBridgeObject
 * @return			A newly created Lisp object translated from the source value.
 */

inline cl_object brLispObjectFromEval( const brEval *inEval, cl_object inBridgeObject ) {
	switch( inEval -> type() ) {
		case AT_INT:
			return ecl_make_integer( BRINT( inEval ) );
			break;
		case AT_DOUBLE:
			return ecl_make_doublefloat( BRDOUBLE( inEval ) );
			break;
		case AT_VECTOR:
			{
				slVector *v = &BRVECTOR( inEval );
				return cl_list( 3, 
					ecl_make_doublefloat( v -> x ),
					ecl_make_doublefloat( v -> y ),
					ecl_make_doublefloat( v -> z ) );
			}
		case AT_LIST:
			return brLispFromBreveListConvert( BRLIST( inEval ) );
			break;
		default:
			slMessage( DEBUG_ALL, "Error: breve type %d conversion to Lisp not implemented\n", inEval -> type() );
			break;
	}

	return NULL;
}







/////////////////////////////////////////////////////////////////////////////////////                                                
//  Lisp language frontend callbacks for the breveObjectType structure           //
/////////////////////////////////////////////////////////////////////////////////////                                                  


/**
 * A breveObjectType callback to locate a method in a Lisp object.
 * 
 * @param inObject 	A void pointer to a Lisp object.
 * @param inName	The name of the method to locate.
 * @param inTypes 	Argument types (currently unused)
 * @param inCount	The argument count
 * 
 * @return 			A void pointer to a Lisp method object. 
 */

brMethodCallbackData *brLispFindMethod( brObjectCallbackData* inObject, const char *inName, unsigned char *inTypes, int inCount ) {
	// No actual method lookup here -- just turn the name into a symbol and let 
	// the runtime handle any errors.

	cl_object cmd = c_string_to_object( inName );

	return new brLispMethodData( cmd, inCount );
}

/**
 * A breveObjectType callback to locate a class in Lisp.
 * 
 * @param inData 	The userdata callback pointer to a __main__ module 
 * @param inName	The name of the desired object
 */

brObjectCallbackData *brLispFindObject( brFrontendCallbackData *inData, const char *inName ) {
	std::string command;
	command = std::string( "( find-class '" ) + inName + ")";

	cl_object result = cl_safe_eval( c_string_to_object( command.c_str() ), Cnil, OBJNULL );

	return ( result != NULL ) ? (void*)( new brLispObjectData( result ) ) : NULL;
}

/**
 * A breveObjectType callback to instantiate a class in Lisp
 * 
 * @param inEngine	The breve engine creating the instancce.
 * @param inObject	The object to be instantiated.
 * @param inArgs	Constructor arguments -- not currently used.
 * @param inArgCount	Constructor argument count -- not currently used.
 */

brInstance *brLispInstantiate( brEngine *inEngine, brObject* inObject, const brEval **inArgs, int inArgCount ) {
	std::string command( "( make-instance '" );
	command += inObject -> name;
	command += ")";

	cl_object result = cl_safe_eval( c_string_to_object( command.c_str() ), Cnil, OBJNULL );

	brInstance *instance = NULL;
	
	if( result ) 
		instance = brEngineAddInstance( inEngine, inObject, new brLispInstanceData( result ) );

	return instance;
}

/**
 * A breveObjectType callback to call a method in Lisp
 * 
 * @param inInstance	The object instance data (as found with \ref brLispInstantiate)
 * @param inMethod	The method callback data (as found with \ref brLispFindMethod )
 * @param outResult	The brEval result of the method calll
 */

int brLispCallMethod( brInstanceCallbackData *inInstance, brMethodCallbackData *inMethod, const brEval **inArguments, brEval *outResult ) {
	brLispMethodData *methodData = (brLispMethodData*)inMethod;
	brLispInstanceData *instanceData = (brLispInstanceData*)inInstance;

	cl_object instance = instanceData -> _instance;
	cl_object method = methodData -> _method;
	cl_object list = cl_list( 0 );

	for( int i = methodData -> _argcount - 1; i >= 0; i-- ) {
		// go backwards, cons each argument onto the method execution list
		cl_object arg = brLispObjectFromEval( inArguments[ i ], NULL );

		if( !arg ) {
			slMessage( DEBUG_ALL, "An error occurred converting the method arguments to Lisp\n" );
			return EC_ERROR;
		}

		list = cl_cons( brLispObjectFromEval( inArguments[ i ], NULL ), list );
	}

	// cons on the instance as the first argument, and then the method symbol

	list = cl_cons( instance, list );
	list = cl_cons( method,   list );

	cl_object result = cl_safe_eval( list, Cnil, OBJNULL );

	if( result != NULL ) {
		brLispObjectToEval( result, outResult );
		return EC_OK;
	} else {
		slMessage( DEBUG_ALL, "An error occurred executing the Lisp method \"%s\"\n", method -> symbol.name -> base_string.self );
	}

	return EC_ERROR;
}

/**
 * A brObjectType callback to determine whether one object is a subclass of another.  Used by 
 * collision detection to determine if a handler is installed for an object pair.
 * 
 * Clearly not implemented at the moment.
 */

int brLispIsSubclass( brObjectType *inObjectType,  brObjectCallbackData *inClassA, brObjectCallbackData *inClassB ) {
	return 0;
}

/**
 * The Lisp canLoad breve object callback
 */

int brLispCanLoad( brFrontendCallbackData *inData, const char *inExtension ) {
	// check the file extension

	if( !strcasecmp( inExtension, "l" ) 
		|| !strcasecmp( inExtension, "lisp" ) 
		|| !strcasecmp( inExtension, "brevel" ) )
			return 1;

	return 0;
}

/**
 * The Lisp load breve object callback
 */

int brLispLoad( brEngine *inEngine, brFrontendCallbackData *inData, const char *inFilename, const char *inFiletext ) {
	std::string command = "( PROGN ";
	command += inFiletext;
	command += " ) ";

	cl_object result = cl_safe_eval( c_string_to_object( command.c_str() ), Cnil, OBJNULL );
	return result == NULL ? EC_ERROR : EC_OK;
}

/**
 * A brObjectType callback to clean up a generic Lisp object.  This method is used as the 
 * destructor for both objects and methods
 *
 * @param inObject		A void pointer to a Lisp object.
 */

void brLispDestroyGenericLispObject( void *inObject ) {
	if( inObject ) 
		delete (brLispData*)inObject;
}






extern int cl_boot( int, char** ) __attribute__((weak_import));

int brLispInit( brEngine *breveEngine ) {
	// On OS X we weaklink libecl

	if( cl_boot == NULL )
		return EC_ERROR;

	// Boot up the lisp interpreter 

	char *argv[ 1 ] = { "breve" };
	cl_boot( 1, argv );

	// Lisp messes with our signal handlers!  We cannot allow that.

	signal( SIGFPE,  NULL );
	signal( SIGSEGV, NULL );
	signal( SIGINT,  NULL );

	// Register the lisp object type

	brObjectType *breveLispType = new brObjectType();

	breveLispType -> findMethod 		= brLispFindMethod;
	breveLispType -> findObject 		= brLispFindObject;
	breveLispType -> instantiate 		= brLispInstantiate;
	breveLispType -> callMethod 		= brLispCallMethod;
	breveLispType -> isSubclass 		= brLispIsSubclass;
	breveLispType -> destroyObject 		= brLispDestroyGenericLispObject;
	breveLispType -> destroyMethod 		= brLispDestroyGenericLispObject;
	breveLispType -> destroyInstance 	= brLispDestroyGenericLispObject;
	breveLispType -> canLoad		= brLispCanLoad;
	breveLispType -> load			= brLispLoad;
	breveLispType -> _typeSignature 	= LISP_TYPE_SIGNATURE;

	brEngineRegisterObjectType( breveEngine, breveLispType );

	return EC_OK;
}



#endif
