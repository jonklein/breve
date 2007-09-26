#ifndef _NETSIM_H
#define _NETSIM_H

#if HAVE_LIBENET
#include <enet/enet.h>

#include <map>
#include <string>

#define NETSIM_MASTER_PORT	5529
#define NETSIM_SLAVE_PORT	5732

struct slNetsimMessage {
	int 						_messageType;
	int 						_messageLength;
	void*						_message;
};

class slNetsimRemoteHost {
	public:
								slNetsimRemoteHost() {
									peer = NULL;
								}

		slVector min, max;
		double simTime;
		double syncTime;
		double speedRatio;
		ENetPeer 					*peer;
};

class slNetsimClient {
	public:
		slNetsimClient() {
			_host = NULL;
			_peer = NULL;
		}

		ENetHost*					_host;
		ENetPeer* 					_peer;
		int  						_terminate;
};

class slNetsimServer {
	public:	
								slNetsimServer( slWorld *inWorld );
								
		slNetsimClient*					openConnection( ENetAddress *inAddress );
		slNetsimClient*					openConnection( char *inHost, int inPort );

		void						start();

		ENetHost*					_host;
		slWorld*					_world;


	private:
		std::map< std::string, slNetsimClient* >	_connectedHosts;
		
		void										(*_passMessage)( slNetsimMessage *inMessage );
};

class slNetsimData {
	public:
		slNetsimData() {
			_server = NULL;
		}

		int 					_isMaster;
		slNetsimServer*				_server;

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

inline void slNetsimBoundsMessageToVectors(slNetsimBoundsMessage *, slVector *, slVector *);
inline void slNetsimVectorsToBoundsMessage(slNetsimBoundsMessage *, slVector *, slVector *);

int slNetsimBroadcastSyncMessage(slNetsimServer*, double);
int slNetsimSendBoundsMessage(slNetsimClient*, slVector *, slVector *);

void slDrawNetsimBounds(slWorld *);

#endif

#endif
