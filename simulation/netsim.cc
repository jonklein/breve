#include <pthread.h>
#include "simulation.h"
#include "netsim.h"
#include "world.h"

#if HAVE_LIBENET

enum slNetsimMessageType {
    MT_SYNC = 0,
    MT_BOUNDS = 1,
    MT_START
};

#include <enet/enet.h>

/**
 * \brief Starts the net simulation server.
 */

slNetsimServer::slNetsimServer( slWorld *inWorld ) {
	ENetAddress address;

	_world = inWorld;

	address.host = ENET_HOST_ANY;
	address.port = NETSIM_MASTER_PORT;

	_host = enet_host_create( &address, 32, 0, 0 );

	if ( !_host ) {
		slMessage( DEBUG_ALL, "netsim: error starting server on port %d\n", NETSIM_MASTER_PORT );
		_world = NULL;
	}

	slMessage( DEBUG_ALL, "netsim: server created for port %d\n", NETSIM_MASTER_PORT );
	
}

void slNetsimServer::start() {
	pthread_t thread;
	pthread_create( &thread, NULL, slNetsimThread, this );
}

/**
 * \brief The entry-point for the server listening thread.
 */

void *slNetsimThread( void *d ) {
	slNetsimBoundsMessage *bMessage;
	slNetsimSyncMessage *sMessage;

	slNetsimServer *serverData = ( slNetsimServer* )d;
	slNetsimRemoteHost *remoteHost;

	ENetHost *server = serverData->_host;
	ENetEvent event;
	int r;

	while ( ( r = enet_host_service( server, &event, 10000 ) ) >= 0 ) {
		switch ( event.type ) {

			case ENET_EVENT_TYPE_CONNECT:
				slMessage( DEBUG_ALL, "netsim: a new client connected from %x:%u.\n",
				           event.peer -> address.host,
				           event.peer -> address.port );

				remoteHost = new slNetsimRemoteHost;

				remoteHost->peer = event.peer;

				// serverData->_world->_netsimData._remoteHosts.push_back( remoteHost );

				break;

			case ENET_EVENT_TYPE_RECEIVE:
				// remoteHost = serverData->_world->_netsimData.remoteHosts[( int )event.peer->data ];

				switch ( event.channelID ) {

					case MT_SYNC:
						sMessage = ( slNetsimSyncMessage* )event.packet->data;

						printf( "the other simulation is at time %f\n", sMessage->simTime );

						break;

					case MT_BOUNDS:
						bMessage = ( slNetsimBoundsMessage* )event.packet->data;

						slNetsimBoundsMessageToVectors( bMessage, &remoteHost->min, &remoteHost->max );

						break;
				}

				enet_packet_destroy( event.packet );

				break;

			case ENET_EVENT_TYPE_DISCONNECT:
				slMessage( DEBUG_ALL, "netsim: disconnect from %d\n", ( int )event.peer->data );

			default:
				break;
		}
	}

	return NULL;
}

slNetsimClient *slNetsimServer::openConnection( ENetAddress *address ) {
	slNetsimClient *data;
	ENetEvent event;

	data = new slNetsimClient;

	data->_host = _host;

	data->_peer = enet_host_connect( _host, address, 32 );

	if ( !( enet_host_service( _host, & event, 5000 ) > 0 && event.type == ENET_EVENT_TYPE_CONNECT ) ) {
		slMessage( DEBUG_ALL, "netsim: error connecting to host %x:%d\n", address->host, address->port );
		delete data;
		return NULL;
	}

	slMessage( DEBUG_ALL, "netsim: connection to %x:%d successful.\n", address->host, address->port );

	enet_host_flush( _host );

	return data;
}

/*!
 * \brief Initiate a connection to another breve host.
 */

slNetsimClient *slNetsimServer::openConnection( const char *host, int port ) {
	ENetAddress address;

	enet_address_set_host( &address, host );
	address.port = port;

	return openConnection( &address );
}

int slNetsimBroadcastSyncMessage( slNetsimServer *server, double time ) {
	slNetsimSyncMessage message;
	ENetPacket *packet;

	message.simTime = time;

	packet = enet_packet_create( &message, sizeof( slNetsimBoundsMessage ), 0 );

	enet_host_broadcast( server->_host, MT_SYNC, packet );

	enet_host_flush( server->_host );

	return 0;
}

int slNetsimSendBoundsMessage( slNetsimClient *client, slVector *min, slVector *max ) {
	slNetsimBoundsMessage message;
	ENetPacket *packet;

	slNetsimVectorsToBoundsMessage( &message, min, max );

	packet = enet_packet_create( &message, sizeof( slNetsimBoundsMessage ), ENET_PACKET_FLAG_RELIABLE );

	enet_peer_send( client->_peer, MT_BOUNDS, packet );

	enet_host_flush( client->_host );

	return 0;
}

inline void slNetsimBoundsMessageToVectors( slNetsimBoundsMessage *m, slVector *min, slVector *max ) {
	min->x = m->minX;
	min->y = m->minY;
	min->z = m->minZ;

	max->x = m->maxX;
	max->y = m->maxY;
	max->z = m->maxZ;
}

inline void slNetsimVectorsToBoundsMessage( slNetsimBoundsMessage *m, slVector *min, slVector *max ) {
	m->minX = min->x;
	m->minY = min->y;
	m->minZ = min->z;

	m->maxX = max->x;
	m->maxY = max->y;
	m->maxZ = max->z;
}

void slDrawNetsimBounds( slWorld *w ) {
	std::vector<slNetsimRemoteHost*>::iterator hi;

	/*

	for ( hi = w->_netsimData._remoteHosts.begin(); hi != w->_netsimData._remoteHosts.end(); hi++ ) {
		slNetsimRemoteHost *data = *hi;

		glEnable( GL_BLEND );
		glColor4f( 0.4, 0.0, 0.0, .3 );

		glBegin( GL_LINE_LOOP );
		glVertex3f( data->min.x, data->min.y, data->min.z );
		glVertex3f( data->max.x, data->min.y, data->min.z );
		glVertex3f( data->max.x, data->max.y, data->min.z );
		glVertex3f( data->min.x, data->max.y, data->min.z );
		glEnd();

		glBegin( GL_LINE_LOOP );
		glVertex3f( data->min.x, data->min.y, data->max.z );
		glVertex3f( data->min.x, data->max.y, data->max.z );
		glVertex3f( data->max.x, data->max.y, data->max.z );
		glVertex3f( data->max.x, data->min.y, data->max.z );
		glEnd();

		glBegin( GL_LINES );
		glVertex3f( data->min.x, data->min.y, data->min.z );
		glVertex3f( data->min.x, data->min.y, data->max.z );

		glVertex3f( data->max.x, data->min.y, data->min.z );
		glVertex3f( data->max.x, data->min.y, data->max.z );

		glVertex3f( data->max.x, data->max.y, data->min.z );
		glVertex3f( data->max.x, data->max.y, data->max.z );

		glVertex3f( data->min.x, data->max.y, data->min.z );
		glVertex3f( data->min.x, data->max.y, data->max.z );
		glEnd();
	}

	*/
}

#endif
