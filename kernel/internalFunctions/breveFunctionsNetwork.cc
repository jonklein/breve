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

#include "kernel.h"
#include "breveFunctionsNetwork.h"
#include "steve.h"
#include "xml.h"

#if !MINGW
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#else
#include <malloc.h>
#include <wininet.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#endif

#define BRNETWORKSERVERPOINTER(p)	((brNetworkServer*)BRPOINTER(p))

/*! \addtogroup InternalFunctions */
/*@{*/

int brINetworkSetRecipient( brEval args[], brEval *target, brInstance *i ) {
	brNetworkServer *data = BRNETWORKSERVERPOINTER( &args[0] );

	if ( !data ) return EC_OK;

	data->recipient = BRINSTANCE( &args[1] );

	return EC_OK;
}

/*!
	\brief Shuts down a network server.

	void closeServer(brNetworkServer pointer).
*/

int brICloseServer( brEval args[], brEval *target, brInstance *i ) {
	brNetworkServer *data = BRNETWORKSERVERPOINTER( &args[0] );

	if ( !data ) {
		slMessage( DEBUG_ALL, "closeServer called with invalid data\n" );
		return EC_OK;
	}

	data->terminate = 1;

	shutdown( data->socket, 2 );
#if WINDOWS
	closesocket( data->socket );
#else
	close( data->socket );
#endif

	if ( data->index ) slFree( data->index );

	pthread_join( data->thread, NULL );

	return EC_OK;
}

/*!
	\brief Sets the HTML index page of a server.

	void setIndexPage(brNetworkServer pointer, string).
*/

int brISetIndexPage( brEval args[], brEval *target, brInstance *i ) {
	brNetworkServer *data = BRNETWORKSERVERPOINTER( &args[0] );

	if ( data->index ) slFree( data->index );

	data->index = slStrdup( BRSTRING( &args[1] ) );

	return EC_OK;
}

/*!
	\brief Gets the URL required to connect to the given server.

	string getServerURL(brNetworkServer pointer).
*/

int brIGetServerURL( brEval args[], brEval *target, brInstance *i ) {
	brNetworkServer *data = BRNETWORKSERVERPOINTER( &args[0] );
	char hostname[1024], url[1024];
	int l = 1024;

	gethostname( hostname, l );

	snprintf( url, sizeof( url ), "http://%s:%d", hostname, data->port );

	target->set( url );

	return EC_OK;
}

/*!
	\brief Starts a network server listening on the given port.

	brNetworkServer pointer listenOnPort(int).
*/

int brIListenOnPort( brEval args[], brEval *target, brInstance *i ) {

	target->set( brListenOnPort( BRINT( &args[0] ), i->engine ) );

	return EC_OK;
}

/*@}*/

/*!
	\brief Initialize the breve engine internal networking functions.
*/

void breveInitNetworkFunctions( brNamespace *n ) {
	brNewBreveCall( n, "listenOnPort", brIListenOnPort, AT_POINTER, AT_INT, 0 );
	brNewBreveCall( n, "networkSetRecipient", brINetworkSetRecipient, AT_NULL, AT_POINTER, AT_INSTANCE, 0 );
	brNewBreveCall( n, "getServerURL", brIGetServerURL, AT_STRING, AT_POINTER, 0 );
	brNewBreveCall( n, "closeServer", brICloseServer, AT_NULL, AT_POINTER, 0 );
	brNewBreveCall( n, "setIndexPage", brISetIndexPage, AT_NULL, AT_POINTER, AT_STRING, 0 );
}

/*!
	\brief Get a hostname from an address.
*/

char *brHostnameFromAddr( struct in_addr *addr ) {
	struct hostent *h;
	static char numeric[256];

	if ( !( h = gethostbyaddr(( const char* )addr, 4, AF_INET ) ) ) {
		unsigned char *address = ( unsigned char * )addr;

		snprintf( numeric, sizeof( numeric ), "%u.%u.%u.%u", address[0], address[1], address[2], address[3] );

		return numeric;
	}

	return h->h_name;
}

/*!
	\brief Starts a network server on a given port.
*/

brNetworkServer *brListenOnPort( int port, brEngine *engine ) {
	int ssock;
	brNetworkServer *serverData;

	struct sockaddr_in saddr;

	if ( ( ssock = socket( AF_INET, SOCK_STREAM, 0 ) ) == -1 ) {
		perror( "Can't create socket" );
		return NULL;
	}

	serverData = new brNetworkServer;

	saddr.sin_family = AF_INET;
	saddr.sin_port = htons( port );
	saddr.sin_addr.s_addr = INADDR_ANY;

	if ( bind( ssock, ( struct sockaddr* )&saddr, sizeof( saddr ) ) < 0 ) {
		perror( "Cannot bind socket" );
		return NULL;
	}

	if ( listen( ssock, 5 ) < 0 ) {
		slMessage( DEBUG_ALL, "Network server unable to listen for connections on port %d\n", port );
		return NULL;
	}

	serverData->index = NULL;

	serverData->engine = engine;
	serverData->recipient = engine-> getController();
	serverData->port = port;
	serverData->socket = ssock;
	serverData->terminate = 0;

	pthread_create( &serverData->thread, NULL, brListenOnSocket, serverData );

	return serverData;
}

/*!
	\brief Listens for connection on a socket.
*/

void *brListenOnSocket( void *data ) {
	brNetworkServer *serverData = ( brNetworkServer* )data;

	socklen_t caddr_size = sizeof( struct sockaddr_in );

	while ( !serverData->terminate ) {
		brNetworkClientData clientData;

		clientData.engine = serverData->engine;
		clientData.server = serverData;

		clientData.socket = accept( serverData->socket, ( struct sockaddr* ) & clientData.addr, &caddr_size );

		if ( clientData.socket != -1 ) {
			// fcntl(clientData.socket, F_SETFL, O_NONBLOCK);
			printf( "about to lock\n" );

			clientData.engine -> lock();

			brHandleConnection( &clientData );

			clientData.engine -> unlock();

#if WINDOWS
			closesocket( clientData.socket );

#else
			close( clientData.socket );

#endif
		}
	}

	return NULL;
}

/*!
	\brief Sending back an XML encoded instance to the client.
*/

int brSendBackXMLString( int sockfd, char *object ) {
	brNetworkRequest request;
	brStringHeader header;

	if ( object )
		header.length = strlen( object );
	else
		header.length = 0;

	request.version = NETWORK_VERSION;

	request.magic = NETWORK_MAGIC;

	request.type = NR_XML;

	write( sockfd, &request, sizeof( brNetworkRequest ) );

	write( sockfd, &header, sizeof( brStringHeader ) );

	if ( header.length ) write( sockfd, object, header.length );

	return 0;
}

/*!
	\brief XML encoding instance and sending back to client.
*/

int brSendBackXMLObject( int sockfd, brInstance *i ) {
	char *buffer = NULL;
	int returnedValue = 0;

	if ( i ) {  
		// If we have an instance, let XML encode it...

		slStringStream *xmlBuffer = slOpenStringStream();
		FILE *file = xmlBuffer->fp;
		brXMLWriteObjectToStream( i, file, 0 );
		buffer = slCloseStringStream( xmlBuffer );
	}

	returnedValue = brSendBackXMLString( sockfd, buffer );

	if ( buffer ) 
		slFree( buffer );

	return returnedValue;
}

/*!
	\brief Handles a network connection.
*/

void *brHandleConnection( void *p ) {
	brNetworkClientData *data = ( brNetworkClientData* )p;
	brNetworkRequest request;
	char *hostname, *buffer;
	brStringHeader header;
	int length, count;
	brMethod *method;
	brEval eval[2], result;
	const brEval *args[2];

	hostname = brHostnameFromAddr( &data->addr.sin_addr );

	if ( hostname ) {
		slMessage( DEBUG_ALL, "network connection from %s\n", hostname );
	}

	count = recv( data->socket, ( char* ) & request, sizeof( brNetworkRequest ), 0 );

	if ( !strncasecmp(( char* )&request, "GET ", 4 ) ) {
		char *http;

		// if the data we read is as large as the brNetworkRequest, then
		// there is more data left to go.  if it's shorter, then we've
		// already hit the end of the stream.

		if ( count == sizeof( brNetworkRequest ) ) {
			http = brFinishNetworkRead( data, &request );
		} else {
			http = slStrdup(( char* ) & request );
		}

		brHandleHTTPConnection( data, http );

		slFree( http );

		return NULL;
	}

	if ( request.magic != NETWORK_MAGIC ) {
		slMessage( DEBUG_ALL, "network connection from invalid client\n" );
		return NULL;
	}

	switch ( request.type ) {

		case NR_XML:
			recv( data->socket, ( char* )&header, sizeof( brStringHeader ), 0 );

			length = header.length;

			buffer = new char[length+1];

			count = slUtilRead( data->socket, buffer, length );

			if ( count != length ) {
				slMessage( DEBUG_ALL, "Received less data then expected: (received = %d, expected = %d)\n", count, length );
				delete[] buffer;
				return NULL;
			}

			buffer[length] = 0;

			args[0] = &eval[0];
			args[1] = &eval[1];

			eval[0].set( buffer );

			brMethodCallByNameWithArgs( data->engine-> getController(), "parse-xml-network-request", args, 1, &result );

			args[0] = &result;

			eval[1].set( hostname );

			method = brMethodFindWithArgRange( data->server->recipient->object, "accept-upload", NULL, 0, 2 );

			if ( method ) {
				brMethodCall( data->server->recipient, method, args, &result );
				brMethodFree( method );

				if ( request.version >= 2 ) { // for backward compatibility, send nothing to earlier NETWORK_VERSION...
					// send back an object based on accept-upload returned value...

					if ( result.type() == AT_INSTANCE ) {
						brSendBackXMLObject( data->socket, result.getInstance() );
					} else { // Nothing to send... Send an empty request...

						if ( result.type() != AT_NULL && result.type() != AT_INT )
							slMessage( DEBUG_ALL, "accept-upload method should return an instance;  Will return an empty object to client: returned type = %d\n", result.type() );

						brSendBackXMLObject( data->socket, ( brInstance * )NULL );
					}
				}
			}

			delete[] buffer;

			break;
	}

	return NULL;
}

/*!
	\brief Finishes a network read.

	The stream has already been partially read in order to determine
	what kind of request we're dealing with, so here we ready the rest.
*/

char *brFinishNetworkRead( brNetworkClientData *data, brNetworkRequest *request ) {
	char	buffer[1024];
	ssize_t count;
	size_t	size = sizeof( brNetworkRequest );
	char	*d = ( char * )slMalloc( size + 1 );

	memcpy( d, request, size );

	while ( !strchr( "\r\n", d[size - 1] ) || ( !strchr( "\r\n", d[size - 2] ) && !strchr( "\r\n", d[size - 3] ) ) ) {

		count = recv( data->socket, buffer, sizeof( buffer ), 0 );

		if ( count < 1 )
			break;

		d = ( char * )slRealloc( d, size + count + 1 );

		memcpy( &d[size], buffer, count );

		d[size + count] = 0;

		size += count;
	}

	d[size] = 0;

	return d;
}

/*!
	\brief A baby little HTTP server, takes care of callbacks.
*/

int brHandleHTTPConnection( brNetworkClientData *data, char *request ) {
	char *end, *method, *string;
	brEval target;
	int count, n;
	int result;
	const brEval **evalPtrs;

	// remove the "GET ", skip forward to the request

	request += 4;

	while ( *request && isspace( *request ) ) request++;

	end = request;

	while ( *end && !isspace( *end ) ) end++;

	*end = 0;

	if ( *request == '/' ) request++;

	// if the request was ONLY "/", then they want the index

	if ( *request == 0 && data->server->index ) request = data->server->index;

	if ( strstr( request, ".html" ) || request == data->server->index ) {
		brSendPage( data, request );
		return 0;
	}

	method = slSplit( request, "_", 0 );

	n = 0;
	count = 0;

	while ( request[n] ) {
		if ( request[n] == '_' ) count++;

		n++;
	}

	evalPtrs = ( const brEval** )alloca( sizeof( brEval* ) * count );

	for ( n = 0;n < count;n++ ) {
		char *str = slSplit( request, "_", n + 1 );
	
		brEval *newEval = new brEval;

		newEval->set( str );

		evalPtrs[ n ] = newEval;

		slFree( str );
	}

	result = brMethodCallByNameWithArgs( data->engine-> getController(), method, evalPtrs, count, &target );

	for ( n = 0;n < count;n++ ) {
		delete evalPtrs[ n ];
	}

	slFree( method );

	if ( result != EC_OK ) {
		send( data->socket, SL_NET_FAILURE, strlen( SL_NET_FAILURE ), 0 );
		return 0;
	}

	if ( target.type() == AT_NULL ) {
		if ( data->server->index ) brSendPage( data, data->server->index );
		else send( data->socket, SL_NET_SUCCESS, strlen( SL_NET_SUCCESS ), 0 );

		return 0;
	}

	string = brFormatEvaluation( &target, data->engine -> getController() );

	if ( strstr( string, ".html" ) ) {
		brSendPage( data, string );
		return 0;
	}

	send( data->socket, string, strlen( string ), 0 );

	return 0;
}

/*!
	\brief Sends an html page over a socket.
*/

void brSendPage( brNetworkClientData *data, char *page ) {
	char *file, *text;

	file = brFindFile( data->engine, page, NULL );

	if ( !file ) {
		send( data->socket, SL_NET_404, strlen( SL_NET_404 ), 0 );
		slMessage( DEBUG_ALL, "network request for unknown file: %s\n", page );
		return;
	}

	text = slUtilReadFile( file );

	// slMessage(DEBUG_ALL, "network request for file: %s\n", page);
	send( data->socket, text, strlen( text ), 0 );

	slFree( file );
	slFree( text );

	return;
}
