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




/**
 * Translation function from a Lisp object to a breve value.
 * 
 * @param inLispVal		The lisp value to be translated
 * @param inEval		The lisp value to be translated.
 * @return			EC_ERROR or EC_OK
 */


inline int brLispTypeToEval( cl_object inObject, brEval *outEval ) {
	switch( type_of( inObject ) ) {
		case t_cons:
			break;

		case t_base_string:
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

		default:
			return EC_ERROR;
	}
}

/**
 * Translation function from a breve value to a Lisp object.
 * 
 * @param inEval		The breve value to be translated.
 * @param inBridgeObject
 * @return			A newly created Lisp object translated from the source value.
 */

inline cl_object brLispTypeFromEval( const brEval *inEval, cl_object inBridgeObject ) {
	switch( inEval -> type() ) {
		case AT_INT:
		case AT_DOUBLE:
		case AT_INSTANCE:
		case AT_VECTOR:
		default:
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

void *brLispFindMethod( void *inObject, const char *inName, unsigned char *inTypes, int inCount ) {
	return NULL;
}

/**
 * A breveObjectType callback to locate a class in Lisp.
 * 
 * @param inData 	The userdata callback pointer to a __main__ module 
 * @param inName	The name of the desired object
 */

void *brLispFindObject( void *inData, const char *inName ) {
	std::string command;

	command = std::string( "( find-class '" ) + inName + ")";

	printf( "cmd: %s\n", command.c_str() );
	cl_object result = cl_eval( c_string_to_object( command.c_str() ) );

	printf( "%s: %p [%d]\n", inName, result, type_of( result ) );

	return result;
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

	command += ")";

	cl_object result = cl_eval( c_string_to_object( command.c_str() ) );
	printf( "%p [%d]\n", result, type_of( result ) );

	return NULL;
}

/**
 * A breveObjectType callback to call a method in Lisp
 * 
 * @param inInstance	The object instance data (as found with \ref brLispInstantiate)
 * @param inMethod	The method callback data (as found with \ref brLispFindMethod )
 * @param inArguments	NOT CURRENTLY IMPLEMENTED
 * @param outResult	The brEval result of the method calll
 */

int brLispCallMethod( void *inInstance, void *inMethod, const brEval **inArguments, brEval *outResult ) {
	return EC_ERROR;
}

/**
 * A brObjectType callback to determine whether one object is a subclass of another.  Used by 
 * collision detection to determine if a handler is installed for an object pair.
 * 
 * Clearly not implemented at the moment.
 */

int brLispIsSubclass( brObjectType *inObjectType,  void *inClassA, void *inClassB ) {
	return 0;
}

/**
 * The Lisp canLoad breve object callback
 */

int brLispCanLoad( void *inObjectData, const char *inExtension ) {
	if( !strcasecmp( inExtension, "l" ) 
		|| !strcasecmp( inExtension, "lisp" ) 
		|| !strcasecmp( inExtension, "brevel" ) )
			return 1;

	return 0;
}

/**
 * The Lisp load breve object callback
 */

int brLispLoad( brEngine *inEngine, void *inObjectTypeUserData, const char *inFilename, const char *inFiletext ) {
	return EC_ERROR;
}

/**
 * A brObjectType callback to clean up a generic Lisp object.  This method is used as the 
 * destructor for both objects and methods
 *
 * @param inObject		A void pointer to a Lisp object.
 */

void brLispDestroyGenericLispObject( void *inObject ) {

}






int brLispInit( brEngine *breveEngine ) {
	// Boot up the lisp interpreter 

	char *argv[ 1 ] = { "breve" };
	cl_object result;

	cl_boot( 1, argv );

	// HELLO WORLD

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
