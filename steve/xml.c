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

enum states {
	XP_ENGINE = 1,
	XP_INSTANCE_ARCHIVE,
	XP_INSTANCE,
	XP_DATA_INSTANCE,
	XP_DATA_INSTANCE_ARCHIVE,
	XP_CLASS,
	XP_VARIABLES,
	XP_DEPENDENCIES,
	XP_OBSERVERS,
	XP_INDEX,

	XP_ARRAY,

	XP_INT,
	XP_DOUBLE,
	XP_LIST,
	XP_HASH,
	XP_STRING,
	XP_VECTOR,
	XP_POINTER,
	XP_OBJECT,
	XP_MATRIX,
	XP_DATA,

	XP_KEY,
	XP_VALUE
};

#define XMLPutSpaces(count, file)	{ int m; for(m=0;m<(count);m++) fputc(' ', (file)); }
#define XML_INDENT_SPACES	1

/*!
	\brief Assigns indices to the instances in the engine.
	
	In preparation for an archiving event, this method assigns indices to all instances
	in the engine.
*/

int stXMLAssignIndices(brEngine *e) {
	int n;
	int top = 0;

	for(n=0;n<e->instances->count;n++) {
		stInstance *i = ((brInstance*)e->instances->data[n])->pointer;
		i->index = n;
	}

	return top;
}

int stXMLWriteObjectToFile(stInstance *i, char *filename, int isDataObject) {
	FILE *file;
	int r;

	if(!(file = fopen(filename, "w"))) {
		slMessage(DEBUG_ALL, "error opening file \"%s\" for archive of instance %p\n", filename, i);
		return -1;
	}

	r = stXMLWriteObjectToStream(i, file, isDataObject);

	fclose(file);

	return r;
}

/*!
	\brief Exports an instance to a FILE stream as XML.
*/

int stXMLWriteObjectToStream(stInstance *i, FILE *file, int isDataObject) {
	stXMLArchiveRecord record;
	int spaces = 0;

	stXMLAssignIndices(i->type->engine);

	bzero(&record, sizeof(stXMLArchiveRecord));

	fprintf(file, "<?xml version=\"1.0\"?>\n");
	fprintf(file, "<!DOCTYPE steveObject SYSTEM \"steveObject.dtd\">\n");

	if(!isDataObject) fprintf(file, "<instance_archive archiveIndex=\"%d\">\n", i->index);
	else fprintf(file, "<data_instance_archive>\n");

	spaces += XML_INDENT_SPACES;
	stXMLWriteObject(&record, file, i, spaces, isDataObject);
	spaces -= XML_INDENT_SPACES;

	if(!isDataObject) fprintf(file, "</instance_archive>\n");
	else fprintf(file, "</data_instance_archive>\n");

	slListFree(record.instances);

	return 0;
}

/*!
	\brief Writes an XML archive of a simulation to a file.
*/

int stXMLWriteSimulationToFile(char *filename, brEngine *e) {
	FILE *file;

	stXMLAssignIndices(e);

	if(!(file = fopen(filename, "w"))) {
		slMessage(DEBUG_ALL, "error opening file \"%s\" for archive of simulation\n", filename);
		return -1;
	}

	stXMLWriteSimulationToStream(file, e);

	fclose(file);

	return 0;
}

/*!
	\brief Writes an XML archive of a simulation to a FILE stream.
*/

int stXMLWriteSimulationToStream(FILE *file, brEngine *e) {
	int n;
	int spaces = 0;
	stXMLArchiveRecord record;
	stInstance *controller;

	controller = e->controller->pointer;

	bzero(&record, sizeof(stXMLArchiveRecord));

	fprintf(file, "<?xml version=\"1.0\"?>\n");
	fprintf(file, "<!DOCTYPE steveEngine SYSTEM \"steveEngine.dtd\">\n");

	fprintf(file, "<engine controllerIndex=\"%d\">\n", controller->index);
	spaces += XML_INDENT_SPACES;

	for(n=0;n<e->instances->count;n++) stXMLWriteObject(&record, file, ((brObject*)e->instances->data[n])->pointer, spaces, 0);

	spaces -= XML_INDENT_SPACES;
	fprintf(file, "</engine>\n");

	return 0;
}

int stXMLWriteObject(stXMLArchiveRecord *record, FILE *file, stInstance *i, int spaces, int isDataObject) {
	slList *list, *d;
	stObject *o;
	brEval result;
	int r;

	if(slInList(record->instances, i)) return 0;

	if(i->status != AS_ACTIVE) {
		slMessage(DEBUG_ALL, "warning: requested archive of freed instance %p of class %s, skipping...\n", i, i->type->name);
		return -1;
	}

	o = i->type;

	record->instances = slListAppend(record->instances, i);

	// make sure the dependencies are archived first

	d = i->breveInstance->dependencies;

	while(d) {
		stXMLWriteObject(record, file, ((brObject*)d->data)->pointer, spaces, isDataObject);
		d = d->next;
	}

	// if we're not writing data only, call the archive method

	if(!isDataObject) {
		stRunInstance ri;
	
		ri.instance = i;
		ri.type = i->type;

		r = stCallMethodByName(&ri, "archive", &result);

		if(r != EC_OK || BRINT(&result) != 1) {
			slMessage(DEBUG_ALL, "archive of instance %p (%s) failed [%d]\n", i, i->type->name, BRINT(&result));
			return -1;
		}
	}

	XMLPutSpaces(spaces, file);

	if(isDataObject) {
		fprintf(file, "<data_instance class=\"%s\" index=\"%d\">\n", o->name, i->index);
	} else {
		fprintf(file, "<instance class=\"%s\" index=\"%d\">\n", o->name, i->index);
	}

	spaces += XML_INDENT_SPACES;

	// write out the observers 

	if(!isDataObject) {
		XMLPutSpaces(spaces, file);
		fprintf(file, "<observers>\n");
		spaces += XML_INDENT_SPACES;

		list = i->breveInstance->observers;

		while(list) {
			brObserver *obs = list->data;
			int index;
	
			if(obs->instance && obs->instance->status == AS_ACTIVE) {
				index = ((stInstance*)obs->instance->pointer)->index;
			} else {
				index = -1;
			}
	
			XMLPutSpaces(spaces, file);
			fprintf(file, "<object index=\"%d\"/>\n", index);
			XMLPutSpaces(spaces, file);
			fprintf(file, "<notification name=\"%s\"/>\n", obs->notification);
			XMLPutSpaces(spaces, file);
			fprintf(file, "<method name=\"%s\"/>\n", obs->method->name);
			
			list = list->next;
		}
		
		spaces -= XML_INDENT_SPACES;
		XMLPutSpaces(spaces, file);
		fprintf(file, "</observers>\n");
	}

	// write out the dependencies

	if(!isDataObject) {
		XMLPutSpaces(spaces, file);
		fprintf(file, "<dependencies>\n");
		spaces += XML_INDENT_SPACES;

		list = i->breveInstance->dependencies;

		while(list) {
			stInstance *dep = ((brInstance*)list->data)->pointer;
			int index;
	
			if(dep && dep->status == AS_ACTIVE) index = dep->index;
			else index = -1;

			XMLPutSpaces(spaces, file);
			fprintf(file, "<object index=\"%d\"/>\n", index);
			
			list = list->next;
		}
	
		spaces -= XML_INDENT_SPACES;
		XMLPutSpaces(spaces, file);
		fprintf(file, "</dependencies>\n");
	}

	/* stores the variables of this instance (& the parents) */

	XMLPutSpaces(spaces, file);

	fprintf(file, "<variables>\n");

	spaces += XML_INDENT_SPACES;

	while(o) {
		list = o->variableList;

		XMLPutSpaces(spaces, file);
		fprintf(file, "<class name=\"%s\" version=\"%f\">\n", o->name, o->version);
		spaces += XML_INDENT_SPACES;

		while(list) {
			stXMLVariablePrint(file, list->data, i, spaces);
			list = list->next;
		}

		spaces -= XML_INDENT_SPACES;
		XMLPutSpaces(spaces, file);
		fprintf(file, "</class>\n");

		o = o->super;
	}

	spaces -= XML_INDENT_SPACES;

	XMLPutSpaces(spaces, file);
	fprintf(file, "</variables>\n");

	spaces -= XML_INDENT_SPACES;

	XMLPutSpaces(spaces, file);

	if(isDataObject) fprintf(file, "</data_instance>\n");
	else fprintf(file, "</instance>\n");

	return 0;
}

int stXMLVariablePrint(FILE *file, stVar *variable, stInstance *i, int spaces) {
	brEval target;
	stRunInstance ri;

	ri.instance = i;
	ri.type = i->type;

	if(variable->type->type == AT_ARRAY) {
		int n, typeSize;

		XMLPutSpaces(spaces, file);
		fprintf(file, "<array name=\"%s\">\n", variable->name);
		spaces += XML_INDENT_SPACES;

		typeSize = stSizeofAtomic(variable->type->arrayType);

		for(n=0;n<variable->type->arrayCount;n++) {
			stLoadVariable(&i->variables[variable->offset + n * typeSize], variable->type->arrayType, &target, &ri);
			stXMLPrintEval(file, "", &target, spaces);
		}

		spaces -= XML_INDENT_SPACES;

		XMLPutSpaces(spaces, file);
		fprintf(file, "</array>\n");

		return 0;
	} 

	stLoadVariable(&i->variables[variable->offset], variable->type->type, &target, &ri);
	stXMLPrintEval(file, variable->name, &target, spaces);

	return 0;
}

int stXMLPrintList(FILE *file, char *name, brEvalListHead *theHead, int spaces) {
	brEvalList *list;

	list = theHead->start;

	XMLPutSpaces(spaces, file);
   	fprintf(file, "<list name=\"%s\">\n", name);
	spaces += XML_INDENT_SPACES;

	while(list) {
		stXMLPrintEval(file, "", &list->eval, spaces);
		list = list->next;
	}

	spaces -= XML_INDENT_SPACES;
	XMLPutSpaces(spaces, file);
	fprintf(file, "</list>\n");

	return 0;
}

int stXMLPrintHash(FILE *file, char *name, brEvalHash *hash, int spaces) {
	brEvalListHead *keys;
	brEvalList *list;

	keys = brEvalHashKeys(hash);
	list = keys->start;

	XMLPutSpaces(spaces, file);
   	fprintf(file, "<hash name=\"%s\">\n", name);
	spaces += XML_INDENT_SPACES;

	while(list) {
		brEval newEval, value;

		brEvalHashLookup(hash, &list->eval, &value);

		XMLPutSpaces(spaces, file);
   		fprintf(file, "<key>\n");
		spaces += XML_INDENT_SPACES;

		brEvalCopy(&list->eval, &newEval);
		stXMLPrintEval(file, "", &newEval, spaces);

		spaces -= XML_INDENT_SPACES;
		XMLPutSpaces(spaces, file);
   		fprintf(file, "</key>\n");

		XMLPutSpaces(spaces, file);
   		fprintf(file, "<value>\n");
		spaces += XML_INDENT_SPACES;

		brEvalCopy(&value, &newEval);
		stXMLPrintEval(file, "", &newEval, spaces);

		spaces -= XML_INDENT_SPACES;
		XMLPutSpaces(spaces, file);
   		fprintf(file, "</value>\n");

		list = list->next;
	}

	spaces -= XML_INDENT_SPACES;
	XMLPutSpaces(spaces, file);
	fprintf(file, "</hash>\n");

	return 0;
}

int stXMLPrintEval(FILE *file, char *name, brEval *target, int spaces) {
	stInstance *i;
	int index;
	char *data, *encoded;

	switch(target->type) {
		case AT_LIST:
			return stXMLPrintList(file, name, BRLIST(target), spaces);
			break;
		case AT_HASH:
			return stXMLPrintHash(file, name, BRHASH(target), spaces);
			break;
		case AT_DATA:
			data = brDataHexEncode((brData*)BRDATA(target));
			XMLPutSpaces(spaces, file);
			fprintf(file, "<data name=\"%s\">%s</data>\n", name, data);
			slFree(data);
			break;
		case AT_INT:
			XMLPutSpaces(spaces, file);
			fprintf(file, "<int name=\"%s\">%d</int>\n", name, BRINT(target));
			break;
		case AT_DOUBLE:
			XMLPutSpaces(spaces, file);
			fprintf(file, "<float name=\"%s\">%.16g</float>\n", name, BRDOUBLE(target));
			break;
		case AT_STRING:
			XMLPutSpaces(spaces, file);
			encoded = stXMLEncodeString(BRSTRING(target));
			fprintf(file, "<string name=\"%s\">%s</string>\n", name, encoded);
			slFree(encoded);
			break;
		case AT_VECTOR:
			XMLPutSpaces(spaces, file);
			fprintf(file, "<vector name=\"%s\">(%.16g, %.16g, %.16g)</vector>\n", name, BRVECTOR(target).x, BRVECTOR(target).y, BRVECTOR(target).z);
			break;
		case AT_MATRIX:
			XMLPutSpaces(spaces, file);
			fprintf(file, "<matrix name=\"%s\">[ (%.16g, %.16g, %.16g), (%.16g, %.16g, %.16g), (%.16g, %.16g, %.16g) ]</matrix>\n", name, BRMATRIX(target)[0][0], BRMATRIX(target)[0][1], BRMATRIX(target)[0][2], BRMATRIX(target)[1][0], BRMATRIX(target)[1][1], BRMATRIX(target)[1][2], BRMATRIX(target)[2][0], BRMATRIX(target)[2][1], BRMATRIX(target)[2][2]);
			break;
		case AT_POINTER:
			XMLPutSpaces(spaces, file);
			fprintf(file, "<pointer name=\"%s\"/>\n", name);
			break;
		case AT_INSTANCE:
			i = STINSTANCE(target);

			if(i && i->status == AS_ACTIVE) index = i->index;
			else index = -1;

			XMLPutSpaces(spaces, file);
			fprintf(file, "<object name=\"%s\" index=\"%d\"/>\n", name, index);
			break;
		default:
			slMessage(DEBUG_ALL, "warning: unknown atomic type (%d) while writing XML object\n", target->type);
			break;
	}

	return 0;
}

#undef XMLPutSpaces

int stXMLReadObjectFromFile(stInstance *i, char *filename) {
	char *buffer;
	int result = 0;

	buffer = slUtilReadFile(filename);

	if(!buffer) {
		slMessage(DEBUG_ALL, "error opening file \"%s\" for dearchive of instance\n", filename, i);
		return -1;
	}
  
	result = stXMLReadObjectFromString(i, buffer);

	if(result == -1) {
		slMessage(DEBUG_ALL, "error reading file \"%s\" for dearchive of instance\n", filename, i);
		return -1;
	}

	slFree(buffer);
	return result;
}

int stXMLReadObjectFromStream(stInstance *i, FILE *file) {
  char *buffer;
	int result = 0;
	buffer = slUtilReadStream(file);

	if(!buffer) {
		slMessage(DEBUG_ALL, "error opening stream for dearchive of instance\n", i);
		return -1;
	}
  
	result = stXMLReadObjectFromString(i, buffer);

	if(result == -1) {
		slMessage(DEBUG_ALL, "error reading stream  for dearchive of instance\n", i);
		return -1;
	}

	slFree(buffer);
	return result;
}

int stXMLReadObjectFromString(stInstance *i, char *buffer) {
  	XML_Parser parser;
	stXMLParserState parserState;
	int result = 0;

	bzero(&parserState, sizeof(stXMLParserState));

	parser = XML_ParserCreate(NULL);

	parserState.currentInstance = i;
	parserState.mode = PARSE_INSTANCE;
	parserState.error = 0;

	/* preparse */

	XML_SetStartElementHandler(parser, (void*)stXMLPreparseStartElementHandler);
	XML_SetEndElementHandler(parser, NULL);
	XML_SetCharacterDataHandler(parser, NULL);
	XML_SetUserData(parser, &parserState);

	if(parserState.error) {
		slMessage(DEBUG_ALL, "Error loading simulation from buffer\n");
		result = -1;
	}

	if(!XML_Parse(parser, buffer, strlen(buffer), 1)) {
		slMessage(DEBUG_ALL, "Error loading simulation from buffer:\n");
		stPrintXMLError(parser);
		result = -1;
	}

	XML_ParserFree(parser);

	/* do the real parse stage */

	parserState.currentInstance = i;
	parser = XML_ParserCreate(NULL);
	XML_SetStartElementHandler(parser, (void*)stXMLObjectStartElementHandler);
	XML_SetEndElementHandler(parser, (void*)stXMLObjectEndElementHandler);
	XML_SetCharacterDataHandler(parser, (void*)stXMLObjectCharacterDataHandler);
	XML_SetUserData(parser, &parserState);

	if(!XML_Parse(parser, buffer, strlen(buffer), 1)) {
		stPrintXMLError(parser);
		result = -1;
	}

	if(parserState.error) {
		result = -1;
	}

	XML_ParserFree(parser);
	return result;
}

stInstance *stXMLDearchiveObjectFromFile(brEngine *e, char *filename) {
	char *buffer;
	stInstance *i;

	buffer = slUtilReadFile(filename);

	if(!buffer) {
		slMessage(DEBUG_ALL, "error opening file \"%s\" for dearchive of instance\n", filename);
		return NULL;
	}

	i = stXMLDearchiveObjectFromString(e, buffer);

	slFree(buffer);

	return i;
}

stInstance *stXMLDearchiveObjectFromStream(brEngine *e, FILE *stream) {
	char *buffer;
	stInstance *i;

	buffer = slUtilReadStream(stream);

	if(!buffer) {
		slMessage(DEBUG_ALL, "error opening stream for dearchive of instance\n");
		return NULL;
	}

	i = stXMLDearchiveObjectFromString(e, buffer);

	slFree(buffer);

	return i;
}

stInstance *stXMLDearchiveObjectFromString(brEngine *e, char *buffer) {
	XML_Parser parser;
	stXMLParserState parserState;
	stInstance *dearchivedInstance;
	int result = 0;

	bzero(&parserState, sizeof(stXMLParserState));
	parser = XML_ParserCreate(NULL);

	parserState.engine = e;
	parserState.mode = PARSE_INSTANCE;
	parserState.error = 0;

	XML_SetParamEntityParsing(parser, XML_PARAM_ENTITY_PARSING_ALWAYS);
	XML_SetExternalEntityRefHandler(parser, (void*)stExternalEntityParserCreate);

	/* do the preparse stage */

	XML_SetStartElementHandler(parser, (void*)stXMLPreparseStartElementHandler);
	XML_SetEndElementHandler(parser, NULL);
	XML_SetCharacterDataHandler(parser, NULL);
	XML_SetUserData(parser, &parserState);

	if(parserState.error) {
		slMessage(DEBUG_ALL, "Error loading simulation from string\n");
		result = -1;
	}

	if(!XML_Parse(parser, buffer, strlen(buffer), 1)) {
		stPrintXMLError(parser);
		result = -1;
	}

	XML_ParserFree(parser);

	/* do the real parse stage */

	parser = XML_ParserCreate(NULL);
	XML_SetStartElementHandler(parser, (void*)stXMLObjectStartElementHandler);
	XML_SetEndElementHandler(parser, (void*)stXMLObjectEndElementHandler);
	XML_SetCharacterDataHandler(parser, (void*)stXMLObjectCharacterDataHandler);
	XML_SetUserData(parser, &parserState);

	if(!XML_Parse(parser, buffer, strlen(buffer), 1)) {
		stPrintXMLError(parser);
		result = -1;
	}

	dearchivedInstance = stXMLFindDearchivedInstance(parserState.instances, parserState.archiveIndex);

	stXMLRunDearchiveMethods(parserState.instances);

	if(parserState.error) result = -1;

	XML_ParserFree(parser);

	if(result == -1) return NULL;
	else return dearchivedInstance;
}

int stXMLInitSimulationFromFile(brEngine *e, char *filename) {
	char *buffer;
	int result = 0;

	buffer = slUtilReadFile(filename);

	if(!buffer) {
		slMessage(DEBUG_ALL, "Error loading archived simulation file: could not open \"%s\" (%s)\n", filename, strerror(errno));
		return -1;
	}
  
	result = stXMLInitSimulationFromString(e, buffer);

	slFree(buffer);
	return result;
}

int stXMLInitSimulationFromStream(brEngine *e, FILE *stream) {
	char *buffer;
	int result = 0;

	buffer = slUtilReadStream(stream);

	if(!buffer) {
		slMessage(DEBUG_ALL, "Error loading archived simulation stream\n");
		return -1;
	}
  
	result = stXMLInitSimulationFromString(e, buffer);
	
	slFree(buffer);
	return result;
}

int stXMLInitSimulationFromString(brEngine *e, char *buffer) {
	XML_Parser parser;
	stXMLParserState parserState;
	int result = 0;

	bzero(&parserState, sizeof(stXMLParserState));

	if(!buffer) {
		slMessage(DEBUG_ALL, "Error loading archived simulation file: could not open buffer\n");
		return -1;
	}

	parser = XML_ParserCreate(NULL);

	parserState.engine = e;
	parserState.mode = PARSE_ENGINE;
	parserState.error = 0;

	/* do the preparse stage */

	XML_SetStartElementHandler(parser, (void*)stXMLPreparseStartElementHandler);
	XML_SetEndElementHandler(parser, NULL);
	XML_SetCharacterDataHandler(parser, NULL);
	XML_SetUserData(parser, &parserState);

	if(parserState.error) {
		slMessage(DEBUG_ALL, "Error loading archived simulation\n");
		result = -1;
	}

	if(!XML_Parse(parser, buffer, strlen(buffer), 1)) {
		slMessage(DEBUG_ALL, "Error loading archived simulation\n");
		stPrintXMLError(parser);
		result = -1;
	}

	XML_ParserFree(parser);

	/* do the real parse stage */

	parser = XML_ParserCreate(NULL);
	XML_SetStartElementHandler(parser, (void*)stXMLObjectStartElementHandler);
	XML_SetEndElementHandler(parser, (void*)stXMLObjectEndElementHandler);
	XML_SetCharacterDataHandler(parser, (void*)stXMLObjectCharacterDataHandler);
	XML_SetUserData(parser, &parserState);

	if(!XML_Parse(parser, buffer, strlen(buffer), 1)) {
		slMessage(DEBUG_ALL, "Error loading archived simulation\n");
		stPrintXMLError(parser);
		result = -1;
	}

	stXMLRunDearchiveMethods(parserState.instances);

	if(parserState.error) {
		slMessage(DEBUG_ALL, "Error loading archived simulation\n");
		result = -1;
	}

	XML_ParserFree(parser);

	return result;
}

/*!
	\brief Runs dearchive for the list of instances.
*/

int stXMLRunDearchiveMethods(slList *l) {
	int r;
	brEval result;

	while(l) {
		stRunInstance ri;

		ri.instance = l->data;
		ri.type = ri.instance->type;

		r = stCallMethodByName(&ri, "dearchive", &result);

		if(r != EC_OK || BRINT(&result) != 1) {
			slMessage(DEBUG_ALL, "dearchive of instance %p (%s) failed\n", ri.instance, ri.instance->type->name);
			return -1;
		}

		l = l->next;
	}

	return 0;
}

/*!
	\brief Preparse stage for XML dearchiving.

	In the preparse stage, we go through the archive and create the empty
	instances.  We do this first so that we can correctly assign instance
	pointers the second time around.
*/

void stXMLPreparseStartElementHandler(stXMLParserState *userData, const XML_Char *attname, const XML_Char **atts) {
	int state;
	int n = 0;
	int index = -1;
	const char *class = NULL;

	state = stXMLStateForElement((char*)attname);

	while(atts[n]) {
		if(!strcasecmp(atts[n], "class")) class = atts[n + 1];
		if(!strcasecmp(atts[n], "index")) index = atoi(atts[n + 1]);
		n += 2;
	}

	if(state == XP_INSTANCE || state == XP_DATA_INSTANCE) {
		stInstance *i;

		if(userData->mode != PARSE_DATA_INSTANCE) {
			brObject *object = brObjectFind(userData->engine, (char*)class);

			if(!object) {
				slMessage(DEBUG_ALL, "archive contains an instance of unknown class \"%s\"\n", class);
				slMessage(DEBUG_ALL, "mismatch between simulation file and XML archive\n", class);
				userData->error++;
				return;
			}

			i = stInstanceNew(object->pointer);
		} else i = userData->currentInstance;

		i->index = index;

		userData->instances = slListAppend(userData->instances, i);
	} 
}

void stXMLObjectStartElementHandler(stXMLParserState *userData, const XML_Char *name, const XML_Char **atts) {
	int n = 0;
	stXMLStackEntry *state;
	const char *class = NULL;
	int controllerIndex = 0, index = 0, archiveIndex = 0;
	brNamespaceSymbol *symbol;

	if(userData->error != 0) return;

	state = slMalloc(sizeof(stXMLStackEntry));
	state->state = stXMLStateForElement((char*)name);

	state->string = NULL;

	userData->stateStack = slListPrepend(userData->stateStack, state);

	while(atts[n]) {
		if(!strcasecmp(atts[n], "name")) state->name = slStrdup((char*)atts[n + 1]);
		if(!strcasecmp(atts[n], "class")) class = atts[n + 1];
		if(!strcasecmp(atts[n], "controllerIndex")) controllerIndex = atoi(atts[n + 1]);
		if(!strcasecmp(atts[n], "archiveIndex")) archiveIndex = atoi(atts[n + 1]);
		if(!strcasecmp(atts[n], "index")) index = atoi(atts[n + 1]);
		n += 2;
	}

	switch(state->state) {
		case XP_ENGINE:
			userData->controllerIndex = controllerIndex;
			break;
		case XP_INSTANCE_ARCHIVE:
			userData->archiveIndex = archiveIndex;
			break;
		case XP_INSTANCE:
			userData->currentInstance = stXMLFindDearchivedInstance(userData->instances, index);
			break;
		case XP_ARRAY:
			symbol = stObjectLookup(userData->currentObject, state->name, ST_VAR);

			if(symbol) {
				state->variable = symbol->data;
				state->arrayIndex = 0;
			} else {
				slMessage(DEBUG_ALL, "archive contains unknown variable \"%s\" for class \"%s\"\n", state->name, userData->currentInstance->type->name);
				slMessage(DEBUG_ALL, "mismatch between simulation file and XML archive\n", class);
				userData->error++;
			}

			break;
		case XP_LIST:
			BRLIST(&state->eval) = brEvalListNew();
			state->eval.type = AT_LIST;
			break;
		case XP_HASH:
			BRHASH(&state->eval) = brEvalHashNew();
			state->eval.type = AT_HASH;
			break;
		case XP_OBJECT:
			// we don't see the character data for this one 
			state->eval.type = AT_INSTANCE;
			STINSTANCE(&state->eval) = stXMLFindDearchivedInstance(userData->instances, index);
			break;
		case XP_POINTER:
			BRINSTANCE(&state->eval) = 0;
			state->eval.type = AT_POINTER;
			break;
		case XP_CLASS:
			userData->currentObject = stObjectFind(userData->engine->objects, state->name);
			break;
	}
}

/*!
	\brief Read the character data for a given XML tag.

	Many states do not use this character data.  Since this function
	may be called multiple times, we have to continually reallocate
	and append data.
*/

void stXMLObjectCharacterDataHandler(stXMLParserState *userData, const XML_Char *data, int len) {
	stXMLStackEntry *e;
	int oldLen;

	if(userData->error != 0) return;

	e = userData->stateStack->data;

	// these data do not expect character data

	if(e->state == XP_ENGINE) return;
	if(e->state == XP_INSTANCE) return;
	if(e->state == XP_INSTANCE_ARCHIVE) return;
	if(e->state == XP_DATA_INSTANCE) return;
	if(e->state == XP_DATA_INSTANCE_ARCHIVE) return;
	if(e->state == XP_OBSERVERS) return;
	if(e->state == XP_CLASS) return;
	if(e->state == XP_VARIABLES) return;
	if(e->state == XP_LIST) return;
	if(e->state == XP_HASH) return;
	if(e->state == XP_ARRAY) return;
	if(e->state == XP_KEY) return;
	if(e->state == XP_VALUE) return;
	if(e->state == XP_DEPENDENCIES) return;

	// allocate or reallocate the string data.

	oldLen = 0;

	if(!e->string) {
		e->string = slMalloc(len + 1);
	} else {
		oldLen = strlen(e->string);
		e->string = slRealloc(e->string, oldLen + len + 1);
	}

	// copy in the new data

	strncpy(&e->string[oldLen], data, len);
	e->string[oldLen + len] = 0;
}

void stXMLObjectEndElementHandler(stXMLParserState *userData, const XML_Char *name) {
	slList *top = userData->stateStack;
	stXMLStackEntry *state = NULL, *lastState = NULL;
	stRunInstance ri;
	double x, y, z;
	double *m;
	stVar *v;

	if(userData->error != 0) return;

	state = top->data;

	// finish up the current state by parsing the character string

	if(state->string) {
		/* variable states, read into the eval */

		switch(state->state) {
			case XP_DATA:
				state->eval.type = AT_DATA;
				BRDATA(&state->eval) = brDataHexDecode(state->string);
				break;
			case XP_POINTER:
				state->eval.type = AT_POINTER;
				BRPOINTER(&state->eval) = NULL;
				break;
			case XP_INT:
				state->eval.type = AT_INT;
				BRINT(&state->eval) = atoi(state->string);
				break;
			case XP_DOUBLE:
				state->eval.type = AT_DOUBLE;
				BRDOUBLE(&state->eval) = atof(state->string);
				break;
			case XP_STRING:
				state->eval.type = AT_STRING;
				BRSTRING(&state->eval) = stXMLDecodeString(state->string);
				break;
			case XP_VECTOR:
				state->eval.type = AT_VECTOR;
				sscanf(state->string, "(%lf, %lf, %lf)", &x, &y, &z);
				BRVECTOR(&state->eval).x = x;
				BRVECTOR(&state->eval).y = y;
				BRVECTOR(&state->eval).z = z;
				break;
			case XP_MATRIX:
				state->eval.type = AT_MATRIX;
				m = &BRMATRIX(&state->eval)[0][0];
				sscanf(state->string, "[ (%lf, %lf, %lf), (%lf, %lf, %lf), (%lf, %lf, %lf) ]", &m[0], &m[1], &m[2], &m[3], &m[4], &m[5], &m[6], &m[7], &m[8]);
				break;
			default:
				state->eval.type = AT_NULL;
				slMessage(DEBUG_ALL, "warning: unknown tag (%d) while parsing XML\n", state->state);
				break;
		}

		slFree(state->string);
	}

	lastState = top->data;
	userData->stateStack = top->next;

	if(userData->stateStack) state = userData->stateStack->data;

	if(state) {
		switch(state->state) {
			case XP_INSTANCE:
				if(userData->mode != PARSE_DATA_INSTANCE) {
					brObject *o;

					o = brObjectFind(userData->engine, userData->currentInstance->type->name);

					userData->currentInstance->breveInstance = brEngineAddInstance(userData->engine, o, userData->currentInstance);

					if(userData->controllerIndex == userData->currentInstance->index) 
						userData->engine->controller = userData->currentInstance->breveInstance;
				}
				break;
			case XP_CLASS:
				if(lastState->state != XP_ARRAY) {
					brNamespaceSymbol *symbol;
					stInstance *i = userData->currentInstance;

					symbol = stObjectLookup(i->type, lastState->name, ST_VAR);

					if(symbol && lastState->eval.type) {
						stRunInstance ri;
						stVar *var = symbol->data;

						ri.instance = userData->currentInstance;
						ri.type = ri.instance->type;

						stSetVariable(&userData->currentInstance->variables[var->offset], var->type->type, NULL, &lastState->eval, &ri);	

						stGCCollect(&lastState->eval);
					}
				}
				break;
			case XP_KEY:
			case XP_VALUE:
				brEvalCopy(&lastState->eval, &state->eval);
				break;
			case XP_HASH:
				if(lastState->state == XP_KEY) {
					brEvalCopy(&lastState->eval, &state->key);
				} else if(lastState->state == XP_VALUE) {
					brEvalHashStore(BRHASH(&state->eval), &state->key, &lastState->eval, NULL);
				}

				stGCCollect(&lastState->eval);

				break;
			case XP_LIST:
				brEvalListInsert(BRLIST(&state->eval), BRLIST(&state->eval)->count, &lastState->eval);
				stGCCollect(&lastState->eval);

				break;
			case XP_ARRAY:
				v = state->variable;

				ri.instance = userData->currentInstance;
				ri.type = ri.instance->type; 

				stSetVariable(&userData->currentInstance->variables[v->offset + state->arrayIndex * stSizeofAtomic(v->type->arrayType)], v->type->arrayType, NULL, &lastState->eval, &ri);	

				state->arrayIndex++;
				break;
			case XP_DEPENDENCIES:
				brInstanceAddDependency(userData->currentInstance->breveInstance, STINSTANCE(&lastState->eval)->breveInstance);
				break;
		} 
	}

	if(lastState) {
		if(lastState->name) slFree(lastState->name);
		slFree(lastState);
		slListFreeHead(top);
	}
}

int stXMLStateForElement(char *name) {
	if(!strcasecmp(name, "engine")) return XP_ENGINE;
	if(!strcasecmp(name, "instance")) return XP_INSTANCE;
	if(!strcasecmp(name, "instance_archive")) return XP_INSTANCE_ARCHIVE;
	if(!strcasecmp(name, "data_instance")) return XP_DATA_INSTANCE;
	if(!strcasecmp(name, "data_instance_archive")) return XP_DATA_INSTANCE_ARCHIVE;
	if(!strcasecmp(name, "observers")) return XP_OBSERVERS;
	if(!strcasecmp(name, "class")) return XP_CLASS;
	if(!strcasecmp(name, "variables")) return XP_VARIABLES;
	if(!strcasecmp(name, "dependencies")) return XP_DEPENDENCIES;
	if(!strcasecmp(name, "index")) return XP_INDEX;

	if(!strcasecmp(name, "array")) return XP_ARRAY;
	if(!strcasecmp(name, "int")) return XP_INT;
	if(!strcasecmp(name, "float")) return XP_DOUBLE;
	if(!strcasecmp(name, "list")) return XP_LIST;
	if(!strcasecmp(name, "hash")) return XP_HASH;
	if(!strcasecmp(name, "string")) return XP_STRING;
	if(!strcasecmp(name, "vector")) return XP_VECTOR;
	if(!strcasecmp(name, "pointer")) return XP_POINTER;
	if(!strcasecmp(name, "object")) return XP_OBJECT;
	if(!strcasecmp(name, "data")) return XP_DATA;
	if(!strcasecmp(name, "matrix")) return XP_MATRIX;

	if(!strcasecmp(name, "key")) return XP_KEY;
	if(!strcasecmp(name, "value")) return XP_VALUE;

	slMessage(DEBUG_ALL, "warning: unknown tag \"%s\" in XML parse\n", name);

	return -1;
}

XML_Parser stExternalEntityParserCreate(XML_Parser p, const XML_Char *context, const XML_Char *encoding) {
	return XML_ParserCreate(NULL);
}

void stPrintXMLError(XML_Parser p) {
	int line = XML_GetCurrentLineNumber(p);
	int col = XML_GetCurrentColumnNumber(p);

	slMessage(DEBUG_ALL, "Error at line %d (character %d) of saved simulation file: %s\n", line, col, XML_ErrorString(XML_GetErrorCode(p)));
}

stInstance *stXMLFindDearchivedInstance(slList *l, int n) {
	stInstance *i;

	while(l) {
		i = l->data;

		if(i->index == n) return i;

		l = l->next;
	}

	return NULL;
}

/*!
	\brief Encode a string for XML export.	

	Both the encode and decode XML string functions use hardcoded values
	to parse out < > & ' and ".  A more general solution might be nice, but 
	whatever.

	Returns an slMalloc'd string.
*/

char *stXMLEncodeString(char *string) {
	char *result;
	int n, m;
	int size = 0;

	/* first calculate the size of the new string */

	n = 0;

	while(string[n] != 0) {
		switch(string[n]) {
			case '<':
			case '>':
				size += 4; /* &lt; and &gt; */
				break;
			case '&':
				size += 5; /* &amp; */
				break;
			case '\'':
			case '\"':
				size += 6; /* &apos; and &quot; */
				break;
			default:
				size++;
		}

		n++;
	}

	result = slMalloc(size + 1);

	n = 0;
	m = 0;

	while(string[n] != 0) {
		switch(string[n]) {
			case '<':
				strcpy(&result[m], "&lt;");
				m += 4;
				break;
			case '>':
				strcpy(&result[m], "&gt;");
				m += 4;
				break;
			case '&':
				strcpy(&result[m], "&amps;");
				m += 5;
				break;
			case '\'':
				strcpy(&result[m], "&apos;");
				m += 6; 
				break;
			case '\"':
				strcpy(&result[m], "&quot;");
				m += 6; 
				break;
			default:
				result[m] = string[n];
				m++;
		}

		n++;
	}

	result[m] = 0;

	return result;
}

/*!
	\brief Decodes a string for XML import.	

	Both the encode and decode XML string functions use hardcoded values
	to parse out < > & ' and ".  A more general solution might be nice, but 
	whatever.

	Returns an slMalloc'd string.
*/

char *stXMLDecodeString(char *string) {
	char *result;
	int n, m;

	result = slMalloc(strlen(string) + 1);

	n = 0;
	m = 0;

	while(string[n] != 0) {
		if(string[n] == '&') {
			if(!strcmp(&string[n], "&gt;")) {
				result[m] = '>';
				n += 4;
			} else if(!strcmp(&string[n], "&lt;")) {
				result[m] = '<';
				n += 4;
			} else if(!strcmp(&string[n], "&amp;")) {
				result[m] = '&';
				n += 5;
			} else if(!strcmp(&string[n], "&apos;")) {
				result[m] = '\'';
				n += 6;
			} else if(!strcmp(&string[n], "&quot;")) {
				result[m] = '\"';
				n += 6;
			} else {
				slMessage(DEBUG_ALL, "warning: stray '&' in XML file\n");
				result[m] = ' ';
				n++;
			}

			m++;
		} else {
			result[m] = string[n];

			m++;
			n++;
		}
	}

	result[m] = 0;

	return result;
}
