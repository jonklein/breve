#ifndef _BREVE_NETWORK_H
#define _BREVE_NETWORK_H

enum networkRequests {
	NR_UPLOAD,
	NR_METHOD,
	NR_XML
};

struct brNetworkServer {
	brEngine *engine;
	brInstance *recipient;
	char *index;
	int port;
	int socket;
	pthread_t thread;
	bool terminate;	// set to true when the thread should terminate 
};

// tee hee hee hee hee hpwee hah haha hahehehhe hah hehah ahahahahahh!#

#define NETWORK_MAGIC   0x0b00b1e5
#define NETWORK_VERSION 2

struct brNetworkRequest {
	brNetworkRequest() { version = 0; magic = 0; type = 0; }

	int32_t version;
	int32_t magic;
	u_char type;
};

struct brStringHeader {
	brStringHeader() { length = 0; }

	int32_t length;
};

#ifdef __cplusplus
extern "C" {
#endif
long brAddrFromHostname(char *name);
#ifdef __cplusplus
}
#endif

#endif /* _BREVE_NETWORK_H */
