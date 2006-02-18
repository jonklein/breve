#ifndef _NETSIM_H
#define _NETSIM_H

#if HAVE_LIBENET
#include <enet/enet.h>

#define NETSIM_MASTER_PORT	5529
#define NETSIM_SLAVE_PORT	5732

class slNetsimRemoteHostData {
	public:
		slNetsimRemoteHostData() {
			peer = NULL;
		}

		slVector min, max;
		double simTime;
		double syncTime;
		double speedRatio;
		ENetPeer *peer;
};

class slNetsimServerData {
	public:
		slNetsimServerData(slWorld *w);

		ENetHost *host;
		slWorld *world;
		int terminate;
};

class slNetsimData {
	public:
		slNetsimData() {
			server = NULL;
			slave = NULL;
		}

		int isMaster;
		slNetsimServerData *server;
		slNetsimServerData *slave;

		std::vector<slNetsimRemoteHostData*> remoteHosts;
};

class slNetsimClientData {
	public:
		slNetsimClientData() {
			host = NULL;
			peer = NULL;
		}

		ENetHost *host;
		ENetPeer *peer;
		int terminate;
};

struct slNetsimBoundsMessage {
	float minX, minY, minZ;
	float maxX, maxY, maxZ;
};

struct slNetsimSyncMessage {
	float simTime;
	float syncTime;
};

struct slNetworkHeader {
	float simulationTime;
	unsigned char messageType;
	int messageCount;
};

void *slNetsimThread(void *);

void slNetsimStartServer(slNetsimServerData *);
slNetsimServerData *slNetsimCreateServer(slWorld *);
slNetsimServerData *slNetsimCreateClient(slWorld *);

slNetsimClientData *slNetsimOpenConnection(ENetHost *, char *, int);
slNetsimClientData *slNetsimOpenConnectionToAddress(ENetHost *, ENetAddress *);

inline void slNetsimBoundsMessageToVectors(slNetsimBoundsMessage *, slVector *, slVector *);
inline void slNetsimVectorsToBoundsMessage(slNetsimBoundsMessage *, slVector *, slVector *);

int slNetsimBroadcastSyncMessage(slNetsimServerData *, double);
int slNetsimSendBoundsMessage(slNetsimClientData *, slVector *, slVector *);

void slDrawNetsimBounds(slWorld *);

#endif

#endif
