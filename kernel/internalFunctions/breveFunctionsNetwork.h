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

#ifndef MINGW
#include <sys/types.h>
#include <unistd.h>
#endif

#include <pthread.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/file.h>

void breveInitNetworkFuncs(brNamespace *n);

void *brHandleConnection(void *data);

brNetworkServer *brListenOnPort(int port, brEngine *engine);
void *brListenOnSocket(void *serverData);

int brIListenOnPort(brEval args[], brEval *target, brInstance *i);
int brICloseServer(brEval args[], brEval *target, brInstance *i);
int brISendObject(brEval args[], brEval *target, brInstance *i);
int brIGetServerURL(brEval args[], brEval *target, brInstance *i);
int brISetIndexPage(brEval args[], brEval *target, brInstance *i);

char *brFinishNetworkRead(brNetworkClientData *data, brNetworkRequest *request);

void brSendPage(brNetworkClientData *data, char *page);

int brHandleHTTPConnection(brNetworkClientData *data, char *request);

int brHTTPReadLine(int socket, char *buffer, size_t size );

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


