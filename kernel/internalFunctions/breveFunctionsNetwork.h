/*****************************************************************************
 *                                                                           *
 * The breve Simulation Environment                                          *
 * Copyright (C) 2000, 2001, 2002, 2003 Jonathan Klein                       *
 *                                                                           *
 * This program is free software; you can redistribute it and/or modify      *
 * it under the terms of the GNU General Public License as published by      *
 * the Free Software Foundation; either version 2 of the License, or         *
 * (at your option) any later version.                                       *
 *                                                                           *
 * This program is distributed in the hope that it will be useful,           *
 * but WITHOUT ANY WARRANTY; without even the implied warranty of            *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the             *
 * GNU General Public License for more details.                              *
 *                                                                           *
 * You should have received a copy of the GNU General Public License         *
 * along with this program; if not, write to the Free Software               *
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA *
 *****************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>

#if !MINGW
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#else
#include <malloc.h>
#include <wininet.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#endif

#include <sys/types.h>
#include <unistd.h>

#include <pthread.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/file.h>

/**
 * \brief Data about a network client, passed off to the function which
 * handles the connection.
 */
struct brNetworkClientData {
    brEngine *engine;
    brNetworkServer *server;
    int socket;
    struct sockaddr_in addr;
};

void breveInitNetworkFuncs(brNamespace *);

int brSendBackXMLString(int, char *);
int brSendBackXMLObject(int, brInstance *);
void *brHandleConnection(void *);

brNetworkServer *brListenOnPort(int, brEngine *);
void *brListenOnSocket(void *);

int brIListenOnPort(brEval [], brEval *, brInstance *);
int brICloseServer(brEval [], brEval *, brInstance *);
int brISendObject(brEval [], brEval *, brInstance *);
int brIGetServerURL(brEval [], brEval *, brInstance *);
int brISetIndexPage(brEval [], brEval *, brInstance *);

char *brFinishNetworkRead(brNetworkClientData *, brNetworkRequest *);

void brSendPage(brNetworkClientData *, char *);

int brHandleHTTPConnection(brNetworkClientData *, char *);

#define SL_NET_404  "<!DOCTYPE HTML PUBLIC \"-//IETF//DTD HTML 2.0//EN\">\
<HTML><HEAD> \
<TITLE>404 Not Found</TITLE> \
</HEAD><BODY> \
<H1>Not Found</H1> \
The requested URL was not found on this server.<P> \
<HR> \
</BODY></HTML>"

#define SL_NET_SUCCESS "<!DOCTYPE HTML PUBLIC \"-//IETF//DTD HTML 2.0//EN\">\
<HTML><HEAD> \
<TITLE>Command executed successfully</TITLE> \
</HEAD><BODY><H1>The breve command has been executed successfully.</H1></BODY></HTML>"

#define SL_NET_FAILURE "<!DOCTYPE HTML PUBLIC \"-//IETF//DTD HTML 2.0//EN\">\
<HTML><HEAD> \
<TITLE>Command failed</TITLE> \
</HEAD><BODY><H1>The requested breve command triggered an error or could not be found.</H1></BODY></HTML>"
