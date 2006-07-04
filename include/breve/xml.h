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
	std::map< stInstance*, int > instanceToIndexMap;
	std::set< stInstance*, stInstanceCompare > written;
};

/*!
	\brief Stack data used when parsing an XML archive.

	As nested tags are parsed, a stack is used to store information
	about the various levels.
*/

struct stXMLStackEntry {
	stXMLStackEntry() {
		state = 0;
		name = NULL;
		variable = NULL;
		string = NULL;

		arrayIndex = 0;
		objectIndex = 0;
	}

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

/*!
	\brief Data used when parsing an XML archive.
*/

struct stXMLParserState {
	stXMLParserState() {
		currentInstance = NULL;
		currentObject = NULL;
		engine = NULL;
		mode = 0;
		error = 0;
		controllerIndex = 0;
		archiveIndex = 0;
	}

	int mode;
	int error;
	brEngine *engine;
	stInstance *currentInstance;
	stObject *currentObject;
	std::vector< stXMLStackEntry* > stateStack;
	int controllerIndex;
	int archiveIndex;
	std::map< int, stInstance* > indexToInstanceMap;
	std::vector< stInstance* > dearchiveOrder;
};

// typedef struct stXMLArchiveRecord stXMLArchiveRecord;
// typedef struct stXMLDearchiveRecord stXMLDearchiveRecord;

// typedef struct stXMLParserState stXMLParserState;
// typedef struct stXMLStackEntry stXMLStackEntry;

int stXMLAssignIndices(brEngine *, std::map< stInstance*, int>&);

int stXMLWriteObjectToFile(stInstance *, char *, int);
int stXMLWriteSimulationToFile(char *, brEngine *);
int stXMLWriteSimulationToStream(FILE *, brEngine *);
int stXMLWriteObject(stXMLArchiveRecord *, FILE *, stInstance *, int, int);
int stXMLSimulationWrite(FILE *, brEngine *, int);

int stXMLVariablePrint(stXMLArchiveRecord *, FILE *, stVar *, stInstance *, int);
int stXMLPrintEval(stXMLArchiveRecord *, FILE *, char *, brEval *, int);
int stXMLPrintList(stXMLArchiveRecord *, FILE *, char *, brEvalListHead *, int);
int stXMLPrintHash(stXMLArchiveRecord *, FILE *, char *, brEvalHash *, int);

int stXMLReadObjectFromFile(stInstance *i, char *);
int stXMLReadObjectFromStream(stInstance *i, FILE *);
int stXMLReadObjectFromString(stInstance *i, char *);
stInstance *stXMLDearchiveObjectFromFile(brEngine *, char *);
stInstance *stXMLDearchiveObjectFromStream(brEngine *, FILE *);
stInstance *stXMLDearchiveObjectFromString(brEngine *, char *);
int stXMLInitSimulationFromFile(brEngine *, char *);
int stXMLInitSimulationFromStream(brEngine *, FILE *);
int stXMLInitSimulationFromString(brEngine *, char *);

int stXMLStateForElement(char *);

void stXMLObjectStartElementHandler(void *, const XML_Char *, const XML_Char **);
void stXMLObjectCharacterDataHandler(void *, const XML_Char *, int);
void stXMLObjectEndElementHandler(void *, const XML_Char *);

void stXMLPreparseStartElementHandler(void *, const XML_Char *, const XML_Char **);
void stXMLPreparseCharacterDataHandler(void *, const XML_Char *, int);
void stXMLPreparseEndElementHandler(void *, const XML_Char *);

XML_Parser stExternalEntityParserCreate(XML_Parser, const XML_Char *, const XML_Char *);

void stPrintXMLError(XML_Parser);

char *stXMLEncodeString(char *);
char *stXMLDecodeString(char *);

int stXMLWriteObjectToStream(stInstance *, FILE *, int);

int stXMLRunDearchiveMethods(stXMLParserState *);
