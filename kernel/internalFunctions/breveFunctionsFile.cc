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

/*! \addtogroup InternalFunctions */
/*@{*/

#include <unistd.h>
#include "kernel.h"
#include "breveFunctionsFile.h"

/*!
	\brief Opens a file for writing.

	void openFileForWriting(string).

	Truncates existing files to 0.
*/

int brIOpenFileForWriting(brEval args[], brEval *target, brInstance *i) {
	brFilePointer *p;
	FILE *fp;

	char *path;

	if(*BRSTRING(&args[0]) == '/') {
		path = strdup(BRSTRING(&args[0]));
	} else {

		path = malloc(strlen(BRSTRING(&args[0])) + strlen(i->engine->outputPath) + 4);
		sprintf(path, "%s/%s", i->engine->outputPath, BRSTRING(&args[0]));
	}

	fp = fopen(path, "w");
	if(!fp) {
		free(path);
		slMessage(DEBUG_ALL, "warning: could not open file %s for writing (%s)\n", path, strerror(errno));
		BRPOINTER(target) = NULL;
		return EC_OK;
	}

	p = slMalloc(sizeof(brFilePointer));
	p->file = fp;

	free(path);

	if(!p->file) {
		slMessage(DEBUG_ALL, "Could not open file \"%s\" for writing: %s\n", BRSTRING(&args[0]), strerror(errno));
		BRPOINTER(target) = NULL;
	} else {
		BRPOINTER(target) = p;
	}

    return EC_OK;
}

/*!
	\brief Opens a file for appending.

	void openFileForAppending(string).
*/

int brIOpenFileForAppending(brEval args[], brEval *target, brInstance *i) {
	brFilePointer *p;
	char *file = brFindFile(i->engine, BRSTRING(&args[0]), NULL);

	if(!file) file = brOutputPath(i->engine, BRSTRING(&args[0]));

	p = slMalloc(sizeof(brFilePointer));
	p->file = fopen(file, "a");

	if(!p->file) {
		slMessage(DEBUG_ALL, "Could not open file \"%s\" for appending: %s\n", BRSTRING(&args[0]), strerror(errno));
		BRPOINTER(target) = NULL;
	} else {
		BRPOINTER(target) = p;
	}

	slFree(file);

	return EC_OK;
}

/*!
	\brief Opens a file for reading.

	void openFileForReading(string).
*/

int brIOpenFileForReading(brEval args[], brEval *target, brInstance *i) {
	brFilePointer *p;
	char *file = brFindFile(i->engine, BRSTRING(&args[0]), NULL);

	if(!file) {
		slMessage(DEBUG_ALL, "Could not locate file \"%s\"\n", BRSTRING(&args[0]));
		BRPOINTER(target) = 0;
	}

	p = slMalloc(sizeof(brFilePointer));
	p->file = fopen(file, "r");
	
	stat(file, &p->st);

	if(!p->file) {
		slMessage(DEBUG_ALL, "Could not open file \"%s\" for reading: %s\n", BRSTRING(&args[0]), strerror(errno));
		BRPOINTER(target) = NULL;
	} else {
		BRPOINTER(target) = p;
	}

	slFree(file);

	return EC_OK;
}

/*!
	\brief Reads the entire file into a string.

	string readFileAsString(brFilePointer pointer).
*/

int brIReadFileAsString(brEval args[], brEval *target, brInstance *i) {
	char *text;
	brFilePointer *p;

	p = BRPOINTER(&args[0]);

	if(!p || !p->file) {
		slMessage(DEBUG_ALL, "readFileAsString called with uninitialized file\n");
		return EC_ERROR;
	}

	text = slMalloc(p->st.st_size + 1);

	fread(text, 1, p->st.st_size, p->file);
	BRSTRING(target) = text;

	return EC_OK;
}

/*!
	\brief Reads a single line from a file.

	string readLine(brFilePointer pointer).
*/

int brIReadLine(brEval args[], brEval *target, brInstance *i) {
	brFilePointer *p;
	char line[10240];

	p = BRPOINTER(&args[0]);

	if(!p || !p->file) {
		slMessage(DEBUG_ALL, "readLine called with uninitialized file\n");
		return EC_ERROR;
	}

	slFgets(&line[0], 10240, p->file);

	line[strlen(line) - 1] = 0;

	BRSTRING(target) = slStrdup(line);

    return EC_OK;
}

/*!
	\brief Reads in the contents of a file as brData data.

	data readFileAsData(brFilePointer).
*/

int brIReadFileAsData(brEval args[], brEval *target, brInstance *i) {
	char *text;
	brFilePointer *p;

	p = BRPOINTER(&args[0]);

	if(!p || !p->file) {
		slMessage(DEBUG_ALL, "readFileAsString called with uninitialized file\n");
		return EC_ERROR;
	}

	text = slMalloc(p->st.st_size + 1);

	BRDATA(target) = brDataNew(text, p->st.st_size);

	slFree(text);

    return EC_OK;
}

/*!
	\brief Writes a string to a file.

	writeString(brFilePointer pointer, string).
*/

int brIWriteString(brEval args[], brEval *target, brInstance *i) {
	brFilePointer *p = BRPOINTER(&args[0]);

	if(!p || !p->file) {
		slMessage(DEBUG_ALL, "writeString called with uninitialized file\n");
		return EC_ERROR;
	}

	fprintf(p->file, "%s", BRSTRING(&args[1]));

    return EC_OK;
}

/*
	\brief Writes brData information to a file.
	
	writeData(brFilePointer pointer, data).
*/

int brIWriteData(brEval args[], brEval *target, brInstance *i) {
	brFilePointer *p = BRPOINTER(&args[0]);

	if(!p || !p->file) {
		slMessage(DEBUG_ALL, "writeData called with uninitialized file\n");
		return EC_ERROR;
	}

	fwrite(BRDATA(&args[1])->data, 1, BRDATA(&args[1])->length, p->file);

    return EC_OK;
}

/*!
	\brief Closes a file.

	void closeFile(brFilePointer pointer).
*/

int brICloseFile(brEval args[], brEval *target, brInstance *i) {
	brFilePointer *p = BRPOINTER(&args[0]);

	if(!p || !p->file) {
		slMessage(DEBUG_ALL, "closeFile called with uninitialized file\n");
		return EC_ERROR;
	}

	fclose(p->file);
	slFree(p);

    return EC_OK;
}
/*@}*/

void breveInitFileFunctions(brNamespace *n) {
    brNewBreveCall(n, "openFileForAppending", brIOpenFileForAppending, AT_POINTER, AT_STRING, 0);
    brNewBreveCall(n, "openFileForWriting", brIOpenFileForWriting, AT_POINTER, AT_STRING, 0);
    brNewBreveCall(n, "openFileForReading", brIOpenFileForReading, AT_POINTER, AT_STRING, 0);
    brNewBreveCall(n, "readLine", brIReadLine, AT_STRING, AT_POINTER, 0);
    brNewBreveCall(n, "readFileAsString", brIReadFileAsString, AT_STRING, AT_POINTER, 0);
    brNewBreveCall(n, "readFileAsData", brIReadFileAsData, AT_DATA, AT_POINTER, 0);
    brNewBreveCall(n, "writeString", brIWriteString, AT_NULL, AT_POINTER, AT_STRING, 0);
    brNewBreveCall(n, "writeData", brIWriteData, AT_NULL, AT_POINTER, AT_DATA, 0);
    brNewBreveCall(n, "closeFile", brICloseFile, AT_NULL, AT_POINTER, 0);
}
