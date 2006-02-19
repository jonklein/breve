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

/*!
	\brief Starts the net simulation server.
*/

slNetsimServerData::slNetsimServerData(slWorld *w) {
	world = w;
	host = enet_host_create(NULL, 32, 0, 0);
}

slNetsimServerData *slNetsimCreateServer(slWorld *world) {
	slNetsimServerData *data;
	ENetAddress address;

	data = new slNetsimServerData( world);

	address.host = ENET_HOST_ANY;
	address.port = NETSIM_MASTER_PORT;

	data->host = enet_host_create(&address, 32, 0, 0);

	if(!data->host) {
		slMessage(DEBUG_ALL, "netsim: error starting server on port %d\n", NETSIM_MASTER_PORT);
		delete data;
		return NULL;
	}

	slMessage(DEBUG_ALL, "netsim: server created for port %d\n", NETSIM_MASTER_PORT);

	return data;
}

void slNetsimStartServer(slNetsimServerData *data) {
	pthread_t thread;
	pthread_create(&thread, NULL, slNetsimThread, data);
}

/*!
	\brief The entry-point for the server listening thread.
*/

void *slNetsimThread(void *d) {
	slNetsimBoundsMessage *bMessage;
	slNetsimSyncMessage *sMessage;

	slNetsimServerData *serverData = (slNetsimServerData*)d;
	slNetsimRemoteHostData *remoteHost;

	ENetHost *server = serverData->host;
	ENetEvent event;
	int r;

	while ((r = enet_host_service (server, &event, 10000)) >= 0) {
		switch (event.type) {
			case ENET_EVENT_TYPE_CONNECT:
				slMessage(DEBUG_ALL, "netsim: a new client connected from %x:%u.\n",
					event.peer -> address.host,
					event.peer -> address.port);

				remoteHost = new slNetsimRemoteHostData;
				remoteHost->peer = event.peer;

				serverData->world->_netsimData.remoteHosts.push_back(remoteHost);
				
				break;

			case ENET_EVENT_TYPE_RECEIVE:
				remoteHost = serverData->world->_netsimData.remoteHosts[ (int)event.peer->data ];

				switch(event.channelID) {
					case MT_SYNC:
						sMessage = (slNetsimSyncMessage*)event.packet->data;
						printf("the other simulation is at time %f\n", sMessage->simTime);
						break;

					case MT_BOUNDS:
						bMessage = (slNetsimBoundsMessage*)event.packet->data;
						slNetsimBoundsMessageToVectors(bMessage, &remoteHost->min, &remoteHost->max);
						break;
				}

				enet_packet_destroy(event.packet);

				break;

			case ENET_EVENT_TYPE_DISCONNECT:
				slMessage(DEBUG_ALL, "netsim: disconnect from %d\n", (int)event.peer->data);

			default:
				break;
		}
	}

	return NULL;
}

slNetsimClientData *slNetsimClientOpenConnectionToAddress(ENetHost *client, ENetAddress *address) {
	slNetsimClientData *data;
	ENetEvent event;

	data = new slNetsimClientData;

	data->host = client;

	data->peer = enet_host_connect(client, address, 32);

	if (!(enet_host_service (client, & event, 5000) > 0 && event.type == ENET_EVENT_TYPE_CONNECT)) {
		slMessage(DEBUG_ALL, "netsim: error connecting to host %x:%d\n", address->host, address->port);
		delete data;
		return NULL;
	}

	slMessage(DEBUG_ALL, "netsim: connection to %x:%d successful.\n", address->host, address->port);

	enet_host_flush(client);

	return data;
}

/*!
	\brief Initiate a connection to another breve host.
*/

slNetsimClientData *slNetsimOpenConnection(ENetHost *client, char *host, int port) {
	ENetAddress address;

	enet_address_set_host(&address, host);
	address.port = port;

	return slNetsimOpenConnectionToAddress(client, &address);
}

slNetsimClientData *slNetsimOpenConnectionToAddress(ENetHost *client, ENetAddress *address) {
	slNetsimClientData *data;
	ENetEvent event;

	data = new slNetsimClientData;

	data->host = client;

	if(!client) {
		slMessage(DEBUG_ALL, "netsim: error creating client!\n");
		delete data;
		return NULL;
	}

	data->peer = enet_host_connect(client, address, 32);

	if (!(enet_host_service (client, & event, 5000) > 0 && event.type == ENET_EVENT_TYPE_CONNECT)) {
		slMessage(DEBUG_ALL, "netsim: error connecting to host %x:%d\n", address->host, address->port);
		delete data;
		return NULL;
	}

	slMessage(DEBUG_ALL, "netsim: connection to %x:%d successful.\n", address->host, address->port);

	enet_host_flush(client);

	return data;
}

int slNetsimBroadcastSyncMessage(slNetsimServerData *server, double time) {
	slNetsimSyncMessage message;
	ENetPacket *packet;

	message.simTime = time;

	packet = enet_packet_create(&message, sizeof(slNetsimBoundsMessage), 0);

	enet_host_broadcast(server->host, MT_SYNC, packet);

	enet_host_flush(server->host);

	return 0;
}

int slNetsimSendBoundsMessage(slNetsimClientData *client, slVector *min, slVector *max) {
	slNetsimBoundsMessage message;
	ENetPacket *packet;

	slNetsimVectorsToBoundsMessage(&message, min, max);

	packet = enet_packet_create(&message, sizeof(slNetsimBoundsMessage), ENET_PACKET_FLAG_RELIABLE);

	enet_peer_send(client->peer, MT_BOUNDS, packet);

	enet_host_flush(client->host);

	return 0;
}

inline void slNetsimBoundsMessageToVectors(slNetsimBoundsMessage *m, slVector *min, slVector *max) {
	min->x = m->minX;
	min->y = m->minY;
	min->z = m->minZ;

	max->x = m->maxX;
	max->y = m->maxY;
	max->z = m->maxZ;
}

inline void slNetsimVectorsToBoundsMessage(slNetsimBoundsMessage *m, slVector *min, slVector *max) {
	m->minX = min->x;
	m->minY = min->y;
	m->minZ = min->z;

	m->maxX = max->x;
	m->maxY = max->y;
	m->maxZ = max->z;
}

void slDrawNetsimBounds(slWorld *w) {
	std::vector<slNetsimRemoteHostData*>::iterator hi;

	for( hi = w->_netsimData.remoteHosts.begin(); hi != w->_netsimData.remoteHosts.end(); hi++ ) {
		slNetsimRemoteHostData *data = *hi;

		glEnable(GL_BLEND);
		glColor4f(0.4, 0.0, 0.0, .3);

		glBegin(GL_LINE_LOOP);
			glVertex3f(data->min.x, data->min.y, data->min.z);
			glVertex3f(data->max.x, data->min.y, data->min.z);
			glVertex3f(data->max.x, data->max.y, data->min.z);
			glVertex3f(data->min.x, data->max.y, data->min.z);
		glEnd();

		glBegin(GL_LINE_LOOP);
			glVertex3f(data->min.x, data->min.y, data->max.z);
			glVertex3f(data->min.x, data->max.y, data->max.z);
			glVertex3f(data->max.x, data->max.y, data->max.z);
			glVertex3f(data->max.x, data->min.y, data->max.z);
		glEnd();

		glBegin(GL_LINES);
			glVertex3f(data->min.x, data->min.y, data->min.z);
			glVertex3f(data->min.x, data->min.y, data->max.z);

			glVertex3f(data->max.x, data->min.y, data->min.z);
			glVertex3f(data->max.x, data->min.y, data->max.z);

			glVertex3f(data->max.x, data->max.y, data->min.z);
			glVertex3f(data->max.x, data->max.y, data->max.z);

			glVertex3f(data->min.x, data->max.y, data->min.z);
			glVertex3f(data->min.x, data->max.y, data->max.z);
		glEnd();
	}
}
#endif
