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

#if HAVE_WCTYPE_H
#include <wctype.h>
#endif

#if !HAVE_ISWSPACE
#define iswspace(c) ((c)==' '||(c)=='\f'||(c)=='\n'||(c)=='\r'||(c)=='\t'||(c)=='\v')
#endif

#define BRFILEPOINTER(p)  ((brFilePointer*)BRPOINTER(p))

/*!
	\brief Opens a file for writing.

	void openFileForWriting(string).

	Truncates existing files to 0.
*/

int brIOpenFileForWriting(brEval args[], brEval *target, brInstance *i) {
	brFilePointer *p;
	FILE *fp;

	char *path;

	path = brOutputPath(i->engine, BRSTRING(&args[0]));

	fp = fopen(path, "w");

	if(!fp) {
		slMessage(DEBUG_ALL, "warning: could not open file %s for writing (%s)\n", path, strerror(errno));
		BRFILEPOINTER(target) = NULL;
		slFree(path);
		return EC_OK;
	}

	slFree(path);

	p = new brFilePointer;
	p->file = fp;

	if(!p->file) {
		slMessage(DEBUG_ALL, "Could not open file \"%s\" for writing: %s\n", BRSTRING(&args[0]), strerror(errno));
		BRFILEPOINTER(target) = NULL;
	} else {
		BRFILEPOINTER(target) = p;
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

	p = new brFilePointer;
	p->file = fopen(file, "a");

	if(!p->file) {
		slMessage(DEBUG_ALL, "Could not open file \"%s\" for appending: %s\n", BRSTRING(&args[0]), strerror(errno));
		BRFILEPOINTER(target) = NULL;
	} else {
		BRFILEPOINTER(target) = p;
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
		BRFILEPOINTER(target) = NULL;
		return EC_OK;
	}

	p = new brFilePointer;
	p->file = fopen(file, "r");
	
	stat(file, &p->st);

	if(!p->file) {
		slMessage(DEBUG_ALL, "Could not open file \"%s\" for reading: %s\n", BRSTRING(&args[0]), strerror(errno));
		BRFILEPOINTER(target) = NULL;
	} else {
		BRFILEPOINTER(target) = p;
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

	p = BRFILEPOINTER(&args[0]);

	if(!p || !p->file) {
		slMessage(DEBUG_ALL, "readFileAsString called with uninitialized file\n");
		return EC_ERROR;
	}

	text = (char*)slMalloc(p->st.st_size + 1);

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

	p = BRFILEPOINTER(&args[0]);

	if (!p || !p->file) {
		slMessage(DEBUG_ALL, "readLine called with uninitialized file\n");
		return EC_ERROR;
	}

	slFgets(line, sizeof(line) - 1, p->file);

	BRSTRING(target) = slStrdup(line);

    return EC_OK;
}

/*!
	\brief Reads list data delimited with a specified string.
*/

int brIReadDelimitedList(brEval args[], brEval *target, brInstance *i) {
	brEvalListHead *head;
	brFilePointer *p;
	brEval eval;
	int n;
	char line[10240];

	p = BRFILEPOINTER(&args[0]);

	BRLIST(target) = head = brEvalListNew();

	if (!p || !p->file) {
		slMessage(DEBUG_ALL, "readDelimitedList called with uninitialized file\n");
		return EC_ERROR;
	}

	slFgets(line, sizeof(line) - 1, p->file);

	eval.type = AT_STRING;

	n = 0;

	while((BRSTRING(&eval) = slSplit(line, BRSTRING(&args[1]), n++)))
		brEvalListInsert(head, head->count, &eval);


	return EC_OK;
}

/*!
	\brief Reads space delimited data into a list.
*/

int brIReadWhitespaceDelimitedList(brEval args[], brEval *target, brInstance *i) {
	brFilePointer *p;
	brEvalListHead *head;
	brEval eval;
	int n, start;
	char line[10240];
	char field[10240];

	p = BRFILEPOINTER(&args[0]);

	BRLIST(target) = head = brEvalListNew();

	if (!p || !p->file) {
		slMessage(DEBUG_ALL, "readWhitespaceDelimitedList called with uninitialized file\n");
		return EC_ERROR;
	}

	slFgets(line, sizeof(line) - 1, p->file);

	start = 0;

	eval.type = AT_STRING;

	n = 0;

	while(line[n]) {
		if(iswspace(line[n])) {
			// if we're at a space, we've found the end of the field

			strncpy(field, &line[start], n - start);
			field[n - start] = 0;

			// continue to the next field data.

			while(line[n] && iswspace(line[n]))
				n++;

			BRSTRING(&eval) = field;

			brEvalListInsert(head, head->count, &eval);

			start = n;
		} else
			n++;
	}

	return EC_OK;
}

/*!
	\brief Reads in the contents of a file as brData data.

	data readFileAsData(brFilePointer).
*/

int brIReadFileAsData(brEval args[], brEval *target, brInstance *i) {
	char *text;
	brFilePointer *p;

	p = BRFILEPOINTER(&args[0]);

	if(!p || !p->file) {
		slMessage(DEBUG_ALL, "readFileAsString called with uninitialized file\n");
		return EC_ERROR;
	}

	text = new char[p->st.st_size + 1];

	BRDATA(target) = brDataNew(text, p->st.st_size);

	delete text;

    return EC_OK;
}

/*!
	\brief Writes a string to a file.

	writeString(brFilePointer pointer, string).
*/

int brIWriteString(brEval args[], brEval *target, brInstance *i) {
	brFilePointer *p = BRFILEPOINTER(&args[0]);

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
	brFilePointer *p = BRFILEPOINTER(&args[0]);

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
	brFilePointer *p = BRFILEPOINTER(&args[0]);

	if(!p || !p->file) {
		slMessage(DEBUG_ALL, "closeFile called with uninitialized file\n");
		return EC_ERROR;
	}

	fclose(p->file);
	delete p;

    return EC_OK;
}

/*!
	\brief Checks a file for EOF.
*/

int brIFileEOF(brEval args[], brEval *target, brInstance *i) {
	brFilePointer *p = BRFILEPOINTER(&args[0]);

	if(!p || !p->file) {
		slMessage(DEBUG_ALL, "closeFile called with uninitialized file\n");
		return EC_ERROR;
	}

	BRINT(target) = feof(p->file);

	return EC_OK;
}
/*@}*/

void breveInitFileFunctions(brNamespace *n) {
    brNewBreveCall(n, "openFileForAppending", brIOpenFileForAppending, AT_POINTER, AT_STRING, 0);
    brNewBreveCall(n, "openFileForWriting", brIOpenFileForWriting, AT_POINTER, AT_STRING, 0);
    brNewBreveCall(n, "openFileForReading", brIOpenFileForReading, AT_POINTER, AT_STRING, 0);
    brNewBreveCall(n, "readLine", brIReadLine, AT_STRING, AT_POINTER, 0);
    brNewBreveCall(n, "readDelimitedList", brIReadWhitespaceDelimitedList, AT_LIST, AT_POINTER, AT_STRING, 0);
    brNewBreveCall(n, "readWhitespaceDelimitedList", brIReadWhitespaceDelimitedList, AT_LIST, AT_POINTER, 0);
    brNewBreveCall(n, "readFileAsString", brIReadFileAsString, AT_STRING, AT_POINTER, 0);
    brNewBreveCall(n, "readFileAsData", brIReadFileAsData, AT_DATA, AT_POINTER, 0);
    brNewBreveCall(n, "writeString", brIWriteString, AT_NULL, AT_POINTER, AT_STRING, 0);
    brNewBreveCall(n, "writeData", brIWriteData, AT_NULL, AT_POINTER, AT_DATA, 0);
    brNewBreveCall(n, "closeFile", brICloseFile, AT_NULL, AT_POINTER, 0);
    brNewBreveCall(n, "fileEOF", brIFileEOF, AT_INT, AT_POINTER, 0);
}
