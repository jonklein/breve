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

#include "steve.h"
#include "xml.h"
#include "breveFunctionsSteveObject.h"

#if !MINGW
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#else
#include <winsock2.h>
#include <wininet.h>
#endif

#define NETWORK_MAGIC   0x0b00b1e5
#define NETWORK_VERSION 1

/*  
	+ steveFuncsObject.c 
	= defines various internal functions relating to the "Object" root 
	= class in steve.
	=
	= functions in this file must be implemented to read arguments from an
	= an array of brEvals and "return" values by filling in the brEval
	= result pointer.
	=
	= functions should return (the real C return) EC_OK or EC_ERROR depending
	= on whether an error occured.  EC_ERROR will kill the simulation,
	= so on a non fatal error it is preferable to simply print an error
	= message.
	=
	= see the file internal.c for more information on the implementation
	= of internal methods.
*/ 


/*!
	\brief Sets freed instance protection on/off for steve.

	void setFreedInstanceProtection(int).

	Freed instance protection retains pointers to freed objects so that calling
	a message with a released object does not crash breve.  This should almost
	always be left on.
*/

int stSSetFreedInstanceProtection(brEval args[], brEval *target, brInstance *i) {
	stInstance *si = (stInstance*)i->userData;
    si->type->steveData->retainFreedInstances = BRINT(&args[0]);
    return EC_OK;
}

int stOCallMethodNamed(brEval args[], brEval *target, brInstance *i) {
	stInstance *newI = (stInstance*)BRINSTANCE(&args[0])->userData;
	char *method = BRSTRING(&args[1]);
	brEvalListHead *l = BRLIST(&args[2]);
	int argCount = 0, n;
	brEval **newargs = NULL;
	stRunInstance ri;

	brEvalList *start = l->start;

	while(start) {
		argCount++;
		start = start->next;
	}

	start = l->start;

	if(argCount != 0) {
		newargs = (brEval**)alloca(sizeof(brEval*) * argCount);

		for(n=0;n<argCount;n++) {
			newargs[n] = (brEval*)alloca(sizeof(brEval));
			brEvalCopy(&start->eval, newargs[n]);

			start = start->next;
		}
	}

	ri.instance = newI;
	ri.type = newI->type;

	stCallMethodByNameWithArgs(&ri, method, newargs, argCount, target);

	return EC_OK;
}

int stOIsa(brEval args[], brEval *target, brInstance *bi) {
	stObject *o;
	stObject *io;
	brObject *bo;
	stInstance *i = (stInstance*)bi->userData;

	// go down to the base instance 
	
	bo = brObjectFind(i->type->engine, BRSTRING(&args[0]));

	if(bo) o = (stObject*)bo->userData;
	else o = NULL;

	io = i->type;

	while(io) {
		if(o == io) {
			BRINT(target) = 1;
			return EC_OK;
		}

		io = io->super;
	}

	BRINT(target) = 0;
	return EC_OK;
}

/*!
	\brief Determines whether a certain instance understand a certain method.
*/

int stORespondsTo(brEval args[], brEval *target, brInstance *i) {
	stInstance *instance = (stInstance*)BRINSTANCE(&args[0])->userData;
	char *method = BRSTRING(&args[1]);

	target->type = AT_INT;

	if( instance->type->methods[ method].size() != 0) BRINT(target) = 1;
	else BRINT(target) = 0;

	return EC_OK;
}

/*!
	\brief Turns garbage collection on or off for an object.
*/

int stOSetGC(brEval args[], brEval *target, brInstance *bi) {
	stInstance *i = (stInstance*)bi->userData;

	i->gc = BRINT(&args[0]);

	return EC_OK;
}

/*!
	\brief Gets the retain count for an object.
*/

int stOGetRetainCount(brEval args[], brEval *target, brInstance *bi) {
	stInstance *i = (stInstance*)bi->userData;
	BRINT(target) = i->retainCount;
	return EC_OK;
}

int stCObjectAllocationReport(brEval args[], brEval *target, brInstance *i) {
    stObjectAllocationReport();
    return EC_OK;
}   

int stNewInstanceForClassString(brEval args[], brEval *target, brInstance *i) {
	brObject *o = brObjectFind(i->engine, BRSTRING(&args[0]));  
 
	if(!o) {
		stEvalError(i->engine, EE_SIMULATION, "Unknown class '%s'.", BRSTRING(&args[0]));
    
		return EC_ERROR;
	} 

	BRINSTANCE(target) = stInstanceCreateAndRegister(i->engine, o);
    
	return EC_OK;
}   

int stSendXMLString(char *address, int port, char *object) {
	brNetworkRequest request;
	brStringHeader header;
	int sockfd;
	struct sockaddr_in saddr;
	long addr = brAddrFromHostname(address);
	header.length = strlen(object);

	if(!addr) {
		slMessage(DEBUG_ALL, "upload failed: cannot find address for host \"%s\"\n", address);
		return -1;
	}

	if((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) return -1;

	saddr.sin_family = AF_INET;
	saddr.sin_port = htons(port);

	// addr already in network order!
	saddr.sin_addr.s_addr = addr;

	request.version = NETWORK_VERSION;
	request.magic = NETWORK_MAGIC;
	request.type = NR_XML;

	if(connect(sockfd, (struct sockaddr*)&saddr, sizeof(saddr))) {
		slMessage(DEBUG_ALL, "upload failed: cannot connect to server \"%s\"\n", address);
		return -1;
	}

	header.length = strlen(object);
	write(sockfd, &request, sizeof(brNetworkRequest));
	write(sockfd, &header, sizeof(brStringHeader));
	write(sockfd, object, header.length);
	close(sockfd);
	return 0;
}

int stNSendXMLObject(brEval *args, brEval *target, brInstance *i) {
	char *addr = BRSTRING(&args[0]);
	int port = BRINT(&args[1]);
	brInstance *archive = BRINSTANCE(&args[2]);

	slStringStream *xmlBuffer = slOpenStringStream();
	FILE *file = xmlBuffer->fp;
	char *buffer;

	stXMLWriteObjectToStream((stInstance*)archive->userData, file, 0);
	buffer = slCloseStringStream(xmlBuffer);

	BRINT(target) = stSendXMLString(addr, port, buffer);

	slFree(buffer);

	return EC_OK;
}

int stCStacktrace(brEval args[], brEval *target, brInstance *i) {
	stInstance *si = (stInstance*)i->userData;

    stStackTrace(si->type->steveData);
    return EC_OK;
}

/*!
	\brief Adds a dependency to the calling object's dependency list.

	void addDependency(object dependency).
*/

int stIAddDependency(brEval args[], brEval *target, brInstance *i) {
	stInstanceAddDependency((stInstance*)i->userData, (stInstance*)BRINSTANCE(&args[0])->userData);
	return EC_OK;
}

/*!
	\brief Removes a dependency from the calling object's dependency list.

	void removeDependency(object dependency).
*/

int stIRemoveDependency(brEval args[], brEval *target, brInstance *i) {
	stInstanceRemoveDependency((stInstance*)i->userData, (stInstance*)BRINSTANCE(&args[0])->userData);
	return EC_OK;
}

void breveInitSteveObjectFuncs(brNamespace *n) {
    brNewBreveCall(n, "setFreedInstanceProtection", stSSetFreedInstanceProtection, AT_NULL, AT_INT, 0);

	brNewBreveCall(n, "callMethodNamed", stOCallMethodNamed, AT_UNDEFINED, AT_INSTANCE, AT_STRING, AT_LIST, 0);
	brNewBreveCall(n, "isa", stOIsa, AT_INT, AT_STRING, 0);
	brNewBreveCall(n, "respondsTo", stORespondsTo, AT_INT, AT_INSTANCE, AT_STRING, 0);
	brNewBreveCall(n, "setGC", stOSetGC, AT_NULL, AT_INT, 0);
	brNewBreveCall(n, "getRetainCount", stOGetRetainCount, AT_INT, 0);

    brNewBreveCall(n, "objectAllocationReport", stCObjectAllocationReport, AT_NULL, 0);
	brNewBreveCall(n, "newInstanceForClassString", stNewInstanceForClassString, AT_INSTANCE, AT_STRING, 0);

	brNewBreveCall(n, "sendXMLObject", stNSendXMLObject, AT_INT, AT_STRING, AT_INT, AT_INSTANCE, 0);
	brNewBreveCall(n, "stacktrace", stCStacktrace, AT_NULL, 0);

	brNewBreveCall(n, "addDependency", stIAddDependency, AT_NULL, AT_INSTANCE, 0);
	brNewBreveCall(n, "removeDependency", stIRemoveDependency, AT_NULL, AT_INSTANCE, 0);


}
