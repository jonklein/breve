#ifndef _NETSIM_H
#define _NETSIM_H

#ifdef __cplusplus
#include <vector>

#ifdef HAVE_LIBENET
#include <enet/enet.h>

#define NETSIM_MASTER_PORT	5529
#define NETSIM_SLAVE_PORT	5732

class slNetsimRemoteHostData {
	public:
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
		int isMaster;
		slNetsimServerData *server;
		slNetsimServerData *slave;

		std::vector<slNetsimRemoteHostData*> remoteHosts;
};

class slNetsimClientData {
	public:
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

void *slNetsimThread(void *data);

void slNetsimStartServer(slNetsimServerData *s);
slNetsimServerData *slNetsimCreateServer(slWorld *world);
slNetsimServerData *slNetsimCreateClient(slWorld *world);

slNetsimClientData *slNetsimOpenConnection(ENetHost *client, char *host, int port);
slNetsimClientData *slNetsimOpenConnectionToAddress(ENetHost *client, ENetAddress *address);

inline void slNetsimBoundsMessageToVectors(slNetsimBoundsMessage *m, slVector *min, slVector *max);
inline void slNetsimVectorsToBoundsMessage(slNetsimBoundsMessage *m, slVector *min, slVector *max);

int slNetsimBroadcastSyncMessage(slNetsimServerData *server, double time);
int slNetsimSendBoundsMessage(slNetsimClientData *client, slVector *min, slVector *max);

void slDrawNetsimBounds(slWorld *w);

#endif
#endif

#endif /* _NETSIM_H */
