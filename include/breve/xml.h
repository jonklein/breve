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

#ifdef __cplusplus
extern "C" {
#endif
#include <expat.h>
#ifdef __cplusplus
}
#endif

enum parserModes {
	PARSE_INSTANCE,
	PARSE_DATA_INSTANCE,
	PARSE_ENGINE
};

/*!
	\brief Data used when preforming an archive.
*/

struct stXMLArchiveRecord {
	slList *instances;
};

/*!
	\brief Data used when preforming a dearchive.
*/

struct stXMLDearchiveRecord {
	slList *instances;
};

/*!
	\brief Data used when parsing an XML archive.
*/

struct stXMLParserState {
	int mode;
	int error;
	brEngine *engine;
	stInstance *currentInstance;
	stObject *currentObject;
	char *currentKey;
	slList *stateStack;
	int controllerIndex;
	int archiveIndex;
	slList *instances;
};

/*!
	\brief Stack data used when parsing an XML archive.

	As nested tags are parsed, a stack is used to store information
	about the various levels.
*/

struct stXMLStackEntry {
	int state;
	brEval eval;

	brEval key;
	brEval value;

	char *name;
	stVar *variable;
	int arrayIndex;
	int objectIndex;

	char *string;
};

typedef struct stXMLArchiveRecord stXMLArchiveRecord;
typedef struct stXMLDearchiveRecord stXMLDearchiveRecord;

typedef struct stXMLParserState stXMLParserState;
typedef struct stXMLStackEntry stXMLStackEntry;

int stXMLAssignIndices(brEngine *e);

int stXMLWriteObjectToFile(stInstance *i, char *filename, int isData);
int stXMLWriteSimulationToFile(char *filename, brEngine *i);
int stXMLWriteSimulationToStream(FILE *f, brEngine *e);
int stXMLWriteObject(stXMLArchiveRecord *record, FILE *file, stInstance *i, int spaces, int isDataObject);
int stXMLSimulationWrite(FILE *file, brEngine *i, int spaces);
int stXMLVariablePrint(FILE *file, stVar *variable, stInstance *i, int spaces);
int stXMLPrintEval(FILE *file, char *name, brEval *target, int spaces);

int stXMLPrintList(FILE *file, char *name, brEvalListHead *head, int spaces);
int stXMLPrintHash(FILE *file, char *name, brEvalHash *hash, int spaces);

int stXMLReadObjectFromFile(stInstance *i, char *filename);
int stXMLReadObjectFromStream(stInstance *i, FILE *stream);
int stXMLReadObjectFromString(stInstance *i, char *buffer);
stInstance *stXMLDearchiveObjectFromFile(brEngine *i, char *filename);
stInstance *stXMLDearchiveObjectFromStream(brEngine *i, FILE *stream);
stInstance *stXMLDearchiveObjectFromString(brEngine *i, char *buffer);
int stXMLInitSimulationFromFile(brEngine *i, char *filename);
int stXMLInitSimulationFromStream(brEngine *i, FILE *stream);
int stXMLInitSimulationFromString(brEngine *i, char *buffer);

int stXMLStateForElement(char *name);

void stXMLObjectStartElementHandler(stXMLParserState *userData, const XML_Char *name, const XML_Char **atts);
void stXMLObjectCharacterDataHandler(stXMLParserState *userData, const XML_Char *name, int len);
void stXMLObjectEndElementHandler(stXMLParserState *userData, const XML_Char *name);

void stXMLPreparseStartElementHandler(stXMLParserState *userData, const XML_Char *name, const XML_Char **atts);
void stXMLPreparseCharacterDataHandler(stXMLParserState *userData, const XML_Char *name, int len);
void stXMLPreparseEndElementHandler(stXMLParserState *userData, const XML_Char *name);

XML_Parser stExternalEntityParserCreate(XML_Parser p, const XML_Char *context, const XML_Char *encoding);

void stPrintXMLError(XML_Parser p);

stInstance *stXMLFindDearchivedInstance(slList *l, int n);

char *stXMLEncodeString(char *string);
char *stXMLDecodeString(char *string);

int stXMLWriteObjectToStream(stInstance *i, FILE * file, int isDataObject);

int stXMLRunDearchiveMethods(slList *l);

