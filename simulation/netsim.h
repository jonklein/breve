
#ifdef __cplusplus
#include <vector>

#ifdef HAVE_LIBENET
#include <enet/enet.h>

#define NETSIM_MASTER_PORT	5529
#define NETSIM_SLAVE_PORT	5732

struct slNetsimRemoteHostData {
	slVector min, max;
	double simTime;
	double syncTime;
	double speedRatio;
	ENetPeer *peer;
};

struct slNetsimData {
	int isMaster;
	slNetsimServerData *server;
	slNetsimServerData *slave;

	std::vector<slNetsimRemoteHostData*> remoteHosts;
};

struct slNetsimServerData {
	ENetHost *host;
	slWorld *world;
	int terminate;
};

struct slNetsimClientData {
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

__inline__ void slNetsimBoundsMessageToVectors(slNetsimBoundsMessage *m, slVector *min, slVector *max);
__inline__ void slNetsimVectorsToBoundsMessage(slNetsimBoundsMessage *m, slVector *min, slVector *max);

int slNetsimBroadcastSyncMessage(slNetsimServerData *server, double time);
int slNetsimSendBoundsMessage(slNetsimClientData *client, slVector *min, slVector *max);
#endif
#endif
