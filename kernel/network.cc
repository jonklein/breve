#include "kernel.h"

#if !MINGW
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#else
#include <winsock2.h>
#include <wininet.h>
#endif

/*!
	\brief Get an address from a hostname.
*/

long brAddrFromHostname(char *name) {
	struct hostent *e;

	e = gethostbyname(name);

	if(!e) return 0;

	return *(long*)e->h_addr_list[0];
}
