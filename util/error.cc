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

#include "util.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>

#ifdef MINGW
#include <malloc.h>
#endif

static int gDebugLevel;

void slStderrMessageCallback(char *string);

static void (*gMessageOutputFunction)(char *output) = slStderrMessageCallback;

/*!
	\brief Set the level of detail of error output.

    This should be 0, except for developers who may choose to use 
    another value to get more detail about certain processes.

    the error reporting functions report every message under the 
    current gDebugLevel, so an important error should be reported
    (using slMessage) with value DEBUG_ALL (0) while a developer 
    debugging message might use DEBUG_INFO (50).
*/

void slSetDebugLevel(int level) {
    gDebugLevel = level;
}

/*!
	\brief Set the message-output callback function.

	When output is produced, it will be passed to the message-output
	callback function.  By default, the data will be printed to 
	stderr, but the callback function could ignore it, print it 
	to a file, etc.
*/

void slSetMessageCallbackFunction(void (f)(char *text)) {
	gMessageOutputFunction = f;
}

/*!
	\brief The default message-output callback function.
*/

void slStderrMessageCallback(char *string) {
	fprintf(stderr, "%s", string);
	fflush(stderr);
}

/*!
	\brief Prints a fatal error to stderr and quits.
    
	slFatal is an exception to the normal error handling rules.
    Regardless of the message-output callback function, we'll 
	print the error to stderr before quitting.
*/

void slFatal(char *format, ...) {
    va_list vp;

    fprintf(stderr, "Fatal error: ");
    va_start(vp, format);
    vfprintf(stderr, format, vp);
    va_end(vp);

    fprintf(stderr, "[hit enter to exit]\n");
    getchar();

    exit(0);
}

/*!
    \brief Pauses execution of the program -- for debugging only.
*/

void slUtilPause() {
    fprintf(stderr, "[hit enter to continue]\n");
    getchar();
}

/*!
    \brief A stub to break on during debugging.
*/

void slDebugFunction(char *file, int line, char *text, ...) {
    va_list vp;   

    va_start(vp, text);

	slMessage(0 , text, vp);
	slMessage(0, "error at line %d of source file \"%s\"\n", line, file);

    va_end(vp);
}

/*!
	\brief Prints printf style output to the output log.

	Uses the global message output function.  Uses the same argument
	formatting as printf to interpret arguments and build an output
	string.
*/

void slMessage(int level, char *format, ...) {
    va_list vp;   
    char *queueMessage;
	int length = 1024 + strlen(format) * 10; 

    if(level > gDebugLevel) return;

    va_start(vp, format);

	queueMessage = (char*)alloca(length);
	vsnprintf(queueMessage, length - 1, format, vp);

    va_end(vp);

	if(gMessageOutputFunction) gMessageOutputFunction(queueMessage);
}

/*!
	\brief Prints a string to the output log.

	Uses the global message output function to output the specified 
	string.  Unlike slMessage, this method does not use sprintf,
	meaning that '%' characters can safely be printed.
*/

void slFormattedMessage(int level, char *string) {
	if(gMessageOutputFunction) gMessageOutputFunction(string);
}

/*!
	\brief Does a stack-trace of the current running program on Mac OS X 
	only.

	This is a hack.  It uses the malloc_history program which does a 
	stack trace.  One of the following environment variables must be 
	set: MallocStackLogging or MallocStackLoggingNoCompact.
*/

void slStackTrace() {
	char command[1024];

	/* this is a hack */

	if(getenv("MallocStackLogging") || getenv("MallocStackLoggingNoCompact")) {
		void *test = malloc(1);

		sprintf(command, "malloc_history %d %p | tail -1 2> /dev/null", getpid(), test);
		system(command);

		free(test);
	}
}
