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


#include "steve.h"
#include "xml.h"
#include "breveFunctionsSteveObject.h"
#include "evaluation.h"

#if !MINGW
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#else
#include <winsock2.h>
#include <wininet.h>
#include <malloc.h>
#endif


/*
	+ steveFuncsObject.c
	= defines various internal functions relating to the "Object" root
	= class in steve.
	=
	= functions in this file must be implemented to read arguments from an
	= an array of brEvals and "return" values by filling in the brEval
	= result pointer.
	=
	= functions should return (the real C return) EC_OK or EC_ERROR depending
	= on whether an error occured.  EC_ERROR will kill the simulation,
	= so on a non fatal error it is preferable to simply print an error
	= message.
	=
	= see the file internal.c for more information on the implementation
	= of internal methods.
*/


/*!
	\brief Sets freed instance protection on/off for steve.

	void setFreedInstanceProtection(int).

	Freed instance protection retains pointers to freed objects so that calling
	a message with a released object does not crash breve.  This should almost
	always be left on.
*/

int stSSetFreedInstanceProtection( brEval args[], brEval *target, brInstance *i ) {
	stInstance *si = ( stInstance* )i->userData;

	if( i->object->type->_typeSignature != STEVE_TYPE_SIGNATURE ) {
		slMessage( DEBUG_ALL, "warning: stSSetFreedInstanceProtection called for non-steve instance\n" );
		return EC_OK;
	}

	si->type->steveData->retainFreedInstances = BRINT( &args[0] );
	return EC_OK;
}

int stOCallMethodNamed( brEval args[], brEval *target, brInstance *i ) {
	stInstance *newI = ( stInstance* )BRINSTANCE( &args[0] )->userData;
	char *method = BRSTRING( &args[1] );
	brEvalListHead *l = BRLIST( &args[2] );
	int argCount = 0;
	const brEval **newargs = NULL;
	stRunInstance ri;

	argCount = l->_vector.size();

	if ( argCount != 0 ) {
		newargs = ( const brEval** )alloca( sizeof( brEval* ) * argCount );

		for ( int n = 0; n < argCount; n++ ) {
			brEval *newEval = new brEval;
			brEvalCopy( &l->_vector[ n ], newEval );
			newargs[ n ] = newEval;
		}
	}

	ri.instance = newI;

	ri.type = newI->type;

	stCallMethodByNameWithArgs( &ri, method, newargs, argCount, target );

	for ( int n = 0; n < argCount; n++ )
		delete newargs[ n ];

	return EC_OK;
}

/*!
	\brief Turns garbage collection on or off for an object.
*/

int stOSetGC( brEval args[], brEval *target, brInstance *bi ) {
	stInstance *i = ( stInstance* )bi->userData;

	if( bi->object->type->_typeSignature != STEVE_TYPE_SIGNATURE ) {
		slMessage( DEBUG_ALL, "warning: stOSetGC called for non-steve instance\n" );
		return EC_OK;
	}

	i->gc = BRINT( &args[0] );

	return EC_OK;
}

/*!
	\brief Gets the retain count for an object.
*/

int stOGetRetainCount( brEval args[], brEval *target, brInstance *bi ) {
	stInstance *i = ( stInstance* )bi->userData;

	target->set( i->retainCount );

	return EC_OK;
}

int stCObjectAllocationReport( brEval args[], brEval *target, brInstance *bi ) {
	stInstance *i = ( stInstance* )bi->userData;
	stObjectAllocationReport( i->type );
	return EC_OK;
}

int stNewInstanceForClassString( brEval args[], brEval *target, brInstance *bi ) {
	brObject *o = brObjectFind( bi->engine, BRSTRING( &args[0] ) );

	if ( !o ) {
		stEvalError( (stInstance*)bi->userData, EE_SIMULATION, "Unknown class '%s'.", BRSTRING( &args[0] ) );

		return EC_ERROR;
	}

	target->set( brObjectInstantiate( bi->engine, o, NULL, 0, false ) );

	return EC_OK;
}

int stWaitForServerReply( int sockfd, brEval *target, brInstance *i ) {
	ssize_t count;
	brNetworkRequest request;
	brStringHeader header;

	count = recv( sockfd, ( char* ) & request, sizeof( brNetworkRequest ), 0 );

	if ( count < 1 ) {
		slMessage( DEBUG_ALL, "error while waiting for Server reply (brNetworkRequest)...\n" );
		return -1;
	}

	if ( request.magic != NETWORK_MAGIC ) {
		slMessage( DEBUG_ALL, "network connection from invalid Server\n" );
		return -1;
	}

	switch ( request.type ) {

		case NR_XML:
			count = recv( sockfd, ( char* ) & header, sizeof( brStringHeader ), 0 );

			if ( count < 1 ) {
				slMessage( DEBUG_ALL, "error while waiting for Server reply (brStringHeader)...\n" );
				return -1;
			}

			if ( header.length ) {
				char *buffer;
				buffer = new char[header.length+1];
				count = slUtilRead( sockfd, buffer, header.length );

				if ( count != header.length ) {
					slMessage( DEBUG_ALL, "error while waiting for Server reply (incomplete data)...\n" );
					delete[] buffer;
					return -1;
				}

				buffer[header.length] = 0;

				const brEval *args[2];
				brEval eval[2];

				eval[0].set( buffer );
				args[0] = &eval[0];
				args[1] = &eval[1];

				brMethodCallByNameWithArgs( i->engine-> getController(), "parse-xml-network-request", args, 1, target );
				delete[] buffer;
			} else {  // Empty request, Server did not send object...
				// Do nothing, brEval is set to AT_NULL by default...
			}

			break;

		default:
			slMessage( DEBUG_ALL, "received unknown request type from Server: type = %d\n", request.type );

			return -1;

			break;
	}

	return 0;
}

//int stSendXMLString(char *address, int port, char *object) {
int stSendXMLString( char *address, int port, char *object, brEval *target, brInstance *i ) {
	brNetworkRequest request;
	brStringHeader header;
	int sockfd;

	struct sockaddr_in saddr;
	long addr = brAddrFromHostname( address );
	header.length = strlen( object );
	int returnedValue = 0;

	if ( !addr ) {
		slMessage( DEBUG_ALL, "upload failed: cannot find address for host \"%s\"\n", address );
		return -1;
	}

	if (( sockfd = socket( AF_INET, SOCK_STREAM, 0 ) ) < 0 ) return -1;

	saddr.sin_family = AF_INET;

	saddr.sin_port = htons( port );

	// addr already in network order!
	saddr.sin_addr.s_addr = addr;

	request.version = NETWORK_VERSION;

	request.magic = NETWORK_MAGIC;

	request.type = NR_XML;

	if ( connect( sockfd, ( struct sockaddr* )&saddr, sizeof( saddr ) ) ) {
		slMessage( DEBUG_ALL, "upload failed: cannot connect to server \"%s\"\n", address );
		return -1;
	}

	header.length = strlen( object );

	write( sockfd, &request, sizeof( brNetworkRequest ) );
	write( sockfd, &header, sizeof( brStringHeader ) );
	write( sockfd, object, header.length );

	// And now lets wait for server reply...
	returnedValue = stWaitForServerReply( sockfd, target, i );

	close( sockfd );
	//return 0;
	return returnedValue;
}

int stNSendXMLObject( brEval *args, brEval *target, brInstance *i ) {
	char *addr = BRSTRING( &args[0] );
	int port = BRINT( &args[1] );
	brInstance *archive = BRINSTANCE( &args[2] );

	// Set target to null instance to avoid problem when it fails...

	target->set(( brInstance * )NULL );

	slStringStream *xmlBuffer = slOpenStringStream();

	FILE *file = xmlBuffer->fp;

	char *buffer;

	brXMLWriteObjectToStream( archive, file, 0 );

	buffer = slCloseStringStream( xmlBuffer );

	brEngineUnlock( i->engine );

	if ( stSendXMLString( addr, port, buffer, target, i ) ) {
		// Something went wrong, we are not happy, but returning EC_ERROR
		// is no good, so lets just print one more message...
		slMessage( DEBUG_ALL, "warning: network send of object %p failed\n", i );
	}

	brEngineLock( i->engine );

	slFree( buffer );

	return EC_OK;
}

int stCStacktrace( brEval args[], brEval *target, brInstance *i ) {
	stInstance *si = ( stInstance* )i->userData;

	if( i->object->type->_typeSignature != STEVE_TYPE_SIGNATURE ) {
		// slMessage( DEBUG_ALL, "warning: stCStacktrace called for non-steve instance\n" );
		return EC_OK;
	}

	stStackTrace( si->type->steveData );
	return EC_OK;
}

/*!
	\brief Adds a dependency to the calling object's dependency list.

	void addDependency(object dependency).
*/

int stIAddDependency( brEval args[], brEval *target, brInstance *i ) {
	if ( !BRINSTANCE( &args[0] ) ) 
		return EC_OK;

	brInstanceAddDependency( i, BRINSTANCE( &args[0] ) );

	return EC_OK;
}

/*!
	\brief Removes a dependency from the calling object's dependency list.

	void removeDependency(object dependency).
*/

int stIRemoveDependency( brEval args[], brEval *target, brInstance *i ) {
	if ( !BRINSTANCE( &args[0] ) ) 
		return EC_OK;

	brInstanceRemoveDependency( i, BRINSTANCE( &args[0] ) );

	return EC_OK;
}

void breveInitSteveObjectFuncs( brNamespace *n ) {
	brNewBreveCall( n, "setFreedInstanceProtection", stSSetFreedInstanceProtection, AT_NULL, AT_INT, 0 );

	brNewBreveCall( n, "callMethodNamed", stOCallMethodNamed, AT_UNDEFINED, AT_INSTANCE, AT_STRING, AT_LIST, 0 );
	brNewBreveCall( n, "setGC", stOSetGC, AT_NULL, AT_INT, 0 );
	brNewBreveCall( n, "getRetainCount", stOGetRetainCount, AT_INT, 0 );

	brNewBreveCall( n, "objectAllocationReport", stCObjectAllocationReport, AT_NULL, 0 );
	brNewBreveCall( n, "newInstanceForClassString", stNewInstanceForClassString, AT_INSTANCE, AT_STRING, 0 );

	//brNewBreveCall(n, "sendXMLObject", stNSendXMLObject, AT_INT, AT_STRING, AT_INT, AT_INSTANCE, 0);
	brNewBreveCall( n, "sendXMLObject", stNSendXMLObject, AT_INSTANCE, AT_STRING, AT_INT, AT_INSTANCE, 0 );
	brNewBreveCall( n, "stacktrace", stCStacktrace, AT_NULL, 0 );

	brNewBreveCall( n, "addDependency", stIAddDependency, AT_NULL, AT_INSTANCE, 0 );
	brNewBreveCall( n, "removeDependency", stIRemoveDependency, AT_NULL, AT_INSTANCE, 0 );


}
