enum { 
	MT_BOUNDS = 1,
	MT_SYNC
} messageTypes;

struct slNetworkBoundsMessage {
	float minX, minY, minZ;
	float maxX, maxY, maxZ;
};

struct slNetworkHeader {
	float simulationTime;
	unsigned char messageType;
	int messageCount;
};
