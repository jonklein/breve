enum networkRequests {
    NR_UPLOAD,
    NR_METHOD,
    NR_XML
};

struct brNetworkServerData {
    brEngine *engine;
    char *index;
    int port;
    int socket;
    pthread_t thread;
    unsigned char terminate;    /* set to true when the thread should terminate */
};

// tee hee hee hee hee hpwee hah haha hahehehhe hah hehah ahahahahahh!#

#define NETWORK_MAGIC   0x0b00b1e5
#define NETWORK_VERSION 1

struct brNetworkRequest {
    int32_t version;
    int32_t magic;
    u_char type;
};

struct brStringHeader {
    int32_t length;
};

long brAddrFromHostname(char *name);
