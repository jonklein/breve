#include "steve.h"

#include "errorText.h"

#include "breveFunctionsSteveDataObject.h"
#include "breveFunctionsSteveObject.h"
#include "breveFunctionsSteveXML.h"

extern char *yyfile;
extern int lineno;

stSteveData *gSteveData;

void *breveFrontendInitData(brEngine *engine) {
	return stSteveInit(engine);
}

void breveFrontendCleanupData(void *data) {
	stSteveCleanup(data);
}

int breveFrontendLoadSimulation(breveFrontend *frontend, char *code, char *file) {
	return stLoadSimulation(frontend->data, frontend->engine, code, file);
}

int breveFrontendLoadSavedSimulation(breveFrontend *frontend, char *simcode, char *simfile, char *xmlfile) {
	return stLoadSavedSimulation(frontend->data, frontend->engine, simcode, simfile, xmlfile);
}

/*!
	\brief The breve callback to determine if one object is a subclass of another.
*/

int stSubclassCallback(brObject *c1, brObject *c2) {
	return stIsSubclassOf(c1->pointer, c2->pointer);
}

/*!
	\brief The breve callback to call a method in the steve language.
*/

int stCallMethodBreveCallback(brInstance *i, brMethod *method, brEval **arguments, brEval *result) {
	int r;
	stRunInstance ri;

	ri.instance = i->pointer;
	ri.type = ri.instance->type;

	r = stCallMethod(&ri, &ri, method->pointer, arguments, method->argumentCount, result);

	return r;
}

/*!
	\brief The steve callback to create a new instance.
*/

void *stInstanceNewCallback(brObject *object, brEval **constructorArgs, int argCount) {
	return stInstanceNew(object->pointer);
}

/*!
	\brief The breve callback to find a method.
*/

void *stFindMethodBreveCallback(brObject *object, char *name, unsigned char *argTypes, int args) {
	stMethod *method;

	method = stFindInstanceMethod(object->pointer, name, args, NULL);

	return method;
}

/*!
	\brief The steve language callback to free an instance.
*/

void stInstanceFreeCallback(brInstance *i) {
	stInstanceFree(i->pointer);
}

/*!
	\brief Initializes the steve language and sets up the brObjectType structure.
*/

stSteveData *stSteveInit(brEngine *engine) {
	gSteveData = slMalloc(sizeof(stSteveData));
	gSteveData->allObjects = NULL;

	breveInitSteveDataObjectFuncs(engine->internalMethods);
	breveInitSteveObjectFuncs(engine->internalMethods);
	breveInitXMLFuncs(engine->internalMethods);

	gSteveData->steveObjectType.callMethod = stCallMethodBreveCallback;
	gSteveData->steveObjectType.findMethod = stFindMethodBreveCallback;
	gSteveData->steveObjectType.isSubclass = stSubclassCallback;
	gSteveData->steveObjectType.instantiate = stInstanceNewCallback;
	gSteveData->steveObjectType.destroyInstance = stInstanceFreeCallback;

	gSteveData->steveObjectType.findObject = NULL;

	gSteveData->retainFreedInstances = 1;
	gSteveData->freedInstances = NULL;
	gSteveData->allObjects = NULL;

	gSteveData->defines = brNamespaceNew(64);

	return gSteveData; 
}

/*!
	\brief Cleanup after steve.

	Free all instances, objects, freed instance lists, controller name, 
	defines and the engine.
*/

void stSteveCleanup(stSteveData *d) {
	slList *o;

	// free all the instances.

	o = d->allObjects;

	while(o) {
		stObjectFreeAllInstances(o->data);
		o = o->next;
	}

	// free all the objects.

	o = d->allObjects;

	while(o) {
		stObjectFree(o->data);
		o = o->next;
	}

	slListFree(d->allObjects);

	o = d->freedInstances;

	while(o) {
		slFree(o->data);
		o = o->next;
	}

	if(d->controllerName) slFree(d->controllerName);

	slListFree(d->freedInstances);

	stFreeParseTrack(d);

	if(d->defines) brNamespaceFreeWithFunction(d->defines, (void(*)(void*))stFreeDefine);

	brEvalListFreeSortVars();

	slFree(d);
}

/*!
	\brief Free a define brEval.
*/

void stFreeDefine(void *d) {
	brEval *e = d;
    
	if(e->type == AT_STRING) slFree(BRSTRING(e));
	slFree(e);
}       

/*!
	\brief Loads steve simulation code to prepare to run a simulation.

	This is the top-level file reading function--this is the one that 
	is called externally to load in files for a simulation.  
*/

int stLoadFiles(stSteveData *sdata, brEngine *engine, char *code, char *file) {
	int r;
	brObject *controller;
	char *path = slStrdup(file);
	int n = strlen(path) - 1;

	while(n && path[n] != '/' && path[n] != '\\') n--;
	path[n] = 0;
	
	if(n != 0 && path[0] == '/') {
		/* absolute path */
		
		brAddSearchPath(engine, path);
	} else if(engine->path && n != 0) {
		/* relative path */
		
		char *fullpath = slMalloc(strlen(engine->path) + strlen(path) + 2);
		sprintf(fullpath, "%s/%s", engine->path, path);
		
		brAddSearchPath(engine, fullpath);
		
		slFree(fullpath);
	} else if(engine->path) { 
		// no path--just a file in the current directory
		
		brAddSearchPath(engine, engine->path);
	}
	
	slFree(path);
	
	r = stParseBuffer(sdata, engine, code, file);
	
	if(r != BPE_OK) return EC_ERROR;
	
	lineno = 1;
	yyfile = file;
	
	if(!sdata->controllerName) {
		stParseError(engine, PE_NO_CONTROLLER, "No \"Controller\" object has been defined");
		return EC_ERROR;
	}
	
	controller = brObjectFind(engine, sdata->controllerName);

	if(!controller) {
		stParseError(engine, EE_UNKNOWN_CONTROLLER, "Unknown \"Controller\" object");
		return EC_ERROR;
	}

	sdata->singleStatementMethod = stNewMethod("internal-user-input-method", NULL, "<user-input>", 0);
	stStoreInstanceMethod(controller->pointer, "internal-user-input-method", sdata->singleStatementMethod);

	return EC_OK;
}

/*!
	\brief Prepares to run a simulation.
	
	Parses and loads the specified code.
*/

int stLoadSimulation(stSteveData *d, brEngine *engine, char *code, char *file) {
	brObject *controllerClass;
	stInstance *controller;
	int r;

	if(stLoadFiles(d, engine, code, file) != EC_OK) return EC_ERROR;

	controllerClass = brObjectFind(engine, d->controllerName);

	if(!controllerClass) {
		stParseError(engine, EE_UNKNOWN_CONTROLLER, "Unknown \"Controller\" object");
		return EC_ERROR;
	}

	controller = stInstanceNew(controllerClass->pointer);

	controller->breveInstance = brEngineAddInstance(engine, controllerClass, controller);

	brEngineSetController(engine, controller->breveInstance);

	r = stInstanceInit(controller);

	if(r != EC_OK) return EC_ERROR; 

	return r;
}

/*!
	\brief Prepares to run a simulation from an XML archive.

	Parses and loads the specified code, then restores the state of the
	simulation from the data stored in the provided XML file.
*/

int stLoadSavedSimulation(stSteveData *sdata, brEngine *engine, char *code, char *file, char *xmlfile) {
	char *xmlpath = brFindFile(engine, xmlfile, NULL);

	if(!xmlpath) {
		slMessage(DEBUG_ALL, "Cannot locate archived XML simulation file \"%s\"\n", xmlfile);
		return EC_ERROR;
	}

	if(stLoadFiles(sdata, engine, code, file) != EC_OK) {
		slFree(xmlpath);
		return EC_ERROR;
	}

	if(stXMLInitSimulationFromFile(engine, xmlpath)) {
		slFree(xmlpath);
		return EC_ERROR;
	}

	slFree(xmlpath);

	return EC_OK;
}

/*!
	\brief Parses a single steve file.

	Given a filename, this function loads and parses the file.  Used by 
	stParseBuffer, not typically called manually.
*/

int stParseFile(stSteveData *sdata, brEngine *engine, char *filename) {
	struct stat fs;
	char *path = NULL;
	int result;

	char *fileString;

	/* try to open the file in the current directory. */

	path = brFindFile(engine, filename, &fs);

	if(!path) {
		slMessage(DEBUG_ALL, "could not locate file %s\n", filename);
		return EC_ERROR;
	}

	fileString = slUtilReadFile(path);

	if(!fileString) {
		slMessage(DEBUG_ALL, "error reading file %s\n", path);
		return EC_ERROR;
	}

	result = stParseBuffer(sdata, engine, fileString, filename);

	slFree(fileString);
	slFree(path);

	return result;
}

/*!
	\brief Parses the text of a steve file.

	stParseBuffer will first pick out included files and recursively parse
	them.  the engine keeps track of what files it's seen, however, so there
	should be no real danger of including the same file twice.

	Typically called by stParseFile, which reads in the file first.
*/

int stParseBuffer(stSteveData *s, brEngine *engine, char *buffer, char *filename) {
	char *thisFile;

	if(stFindParseTrack(s->filesSeen, filename)) {
		slMessage(DEBUG_INFO, "skipping \"%s\", already included\n", filename);
		return 0;
	}

	/* if this file hasn't been seen, add it to the parse track and go */

	if(filename) {
		thisFile = stNewStParseTrack(s, filename);
	} else thisFile = "<untitled>";

	/* set the global variables for the parser */
	/* preprocess the buffer--look for other included files */

	yyfile = thisFile;
	lineno = 1;

	if(stPreprocess(s, engine, buffer)) return BPE_LIB_ERROR;

	/* preprocess changes the yyfile and lineno globals -- reset them */

	yyfile = thisFile;
	lineno = 1;

	stSetParseString(buffer, strlen(buffer));

	/* the REAL parse--set the parse engine so the parser knows */
	/* what to do with the info it parses */

	stParseSetEngine(engine);
	stParseSetSteveData(s);

	engine->error.type = 0;
	if(yyparse()) return BPE_SIM_ERROR;

	return BPE_OK;
}

/*!
	\brief Preprocess a steve document.

	Called automatically by stParseBuffer -- not to be called manually.

	Parse out all of the "@include", or "@path" lines.  by convention
	all '@' lines are preprocessor directives, but currently there
	aren't so many.
*/

int stPreprocess(stSteveData *s, brEngine *engine, char *line) {
	char *start, *end;
	char *filename;
	int n;
	int include = 0, path = 0, use = 0;
	char useWord[1024];
	char *oldYyfile = yyfile;
	int oldLineno = lineno;

	/* i don't wanna comment this */

	if(!line) return -1;

	/* damn this is ugly */

	do {
		while(*line == '\n') {
			line++;
			lineno++;
			oldLineno++;
		}

		include = 0;
		path = 0;
		use = 0;

		if(!strncmp(line, "@include", strlen("@include"))) include = 1;
		else if(!strncmp(line, "@path", strlen("@path"))) path = 1;
		else if(!strncmp(line, "@use", strlen("@use"))) use = 1;

		if(include || path || use) {
			start = end = NULL;
			n = 0;

			if(use) {
				n = 4;

				sscanf(line, "@use %s", useWord);
				if(useWord[strlen(useWord) - 1] == '.')
					useWord[strlen(useWord) - 1] = 0;

				filename = slMalloc(strlen(useWord) + 4);
				sprintf(filename, "%s.tz", useWord);
			} else {
				while(line[n] != 0 && line[n] != '\n') {
					n++;
					if(line[n] == '\"' && !start) start = &line[n + 1];
					else if(line[n] == '\"') end = &line[n];
				}

				filename = slMalloc((end - start) + 1);
				strncpy(filename, start, (end - start));
				filename[end - start] = 0;
			}

			if(include || use) {
				if(stParseFile(s, engine, filename)) {
					yyfile = oldYyfile;
					lineno = oldLineno;
					stParseError(engine, EE_FILE_NOT_FOUND, "Error including file \"%s\"", filename);
					slFree(filename);
					return -1;
				}

				yyfile = oldYyfile;
				lineno = oldLineno;
			} else {
				brAddSearchPath(engine, filename);
			}

			slFree(filename);
		}
	} while((line = strchr(line, '\n')));

	return 0;
}


/*
	\brief Keep track of files that have been seen.

	Adds the specified file to the list of files that have been seen.
	This avoids parsing the same file multiple times.
*/

char *stNewStParseTrack(stSteveData *s, char *name) {
	char *dup = slStrdup(name);

	s->filesSeen = slListPrepend(s->filesSeen, dup);

	return dup;
}

/*
	\brief Frees the parse track data.
*/

void stFreeParseTrack(stSteveData *s) {
	slList *start = s->filesSeen;

	while(start) {
		slFree(start->data);
		start = start->next;
	}

	slListFree(s->filesSeen);
}

/*!
	\brief Determine if a file has already been seen.
*/

char *stFindParseTrack(slList *l, char *name) {
	while(l) {
		if(!strcmp(l->data, name)) return l->data;

		l = l->next;
	}

	return NULL;
}

/*!
	\brief Makes a version requirement.
*/

stVersionRequirement *stMakeVersionRequirement(float version, int operation) {
	stVersionRequirement *b;

	b = slMalloc(sizeof(stVersionRequirement));

	b->version = version;
	b->operation = operation;

	return b;
}

/*!
	\brief Checks to see whether a version requirement has been fulfilled.
*/

int stCheckVersionRequirement(float version, stVersionRequirement *r) {
	if(!r) return 1;

	switch(r->operation) {
		case VR_GT:
			return version > r->version;
			break;
		case VR_GE:
			return version >= r->version;
			break;
		case VR_LT:
			return version < r->version;
			break;
		case VR_LE:
			return version <= r->version;
			break;
		case VR_EQ:
			return version == r->version;
			break;
		case VR_NE:
			return version != r->version;
			break;
		default:
			slMessage(DEBUG_ALL, "unknown operator %d in stCheckVersionRequirement\n", r->operation);
			return 0;
			break;
	}
	
	return 0;
}

/*!
	\brief Reports on current object usage.
*/
	
void stObjectAllocationReport(brEngine *engine) {
	brObject *bo;
	stObject *o;
	slList *objects = brNamespaceSymbolList(engine->objects);
	
	while(objects) {
		bo = objects->data;
	
		o = bo->pointer;

		slMessage(DEBUG_ALL, "class %s: %d objects allocated\n", o->name, slListCount(o->allInstances));
	}
	
	slListFree(objects);
}   

/*! 
	\brief Trigger a steve-language parse error.

	Called when a parse (or other "parse-time") error occurs.
	Prints out error messages and stops the simulation.
*/

void stParseError(brEngine *e, int type, char *proto, ...) {
	va_list vp;
	char localMessage[BR_ERROR_TEXT_SIZE];

	/* if there is no "primary" error defined, then we do the whole process */

	if(e->error.type == 0) {
		e->error.type = type;
		e->error.file = slStrdup(yyfile);
		e->error.line = lineno;

		va_start(vp, proto);
		vsnprintf(e->error.message, BR_ERROR_TEXT_SIZE, proto, vp);
		va_end(vp);

		slMessage(DEBUG_ALL, "%s: %s", gErrorNames[type], e->error.message);
		slMessage(DEBUG_ALL, " at line %d of file \"%s\"", lineno, yyfile);
		//slMessage(DEBUG_ALL, "\n%s\n", gErrorMessages[type]);
	} else {
		va_start(vp, proto);
		vsnprintf(localMessage, BR_ERROR_TEXT_SIZE, proto, vp);
		va_end(vp);

		slMessage(DEBUG_ALL, localMessage);
		slMessage(DEBUG_ALL, " at line %d of file \"%s\"", lineno, yyfile);
	}

	slMessage(DEBUG_ALL, "\n");
}

/*!
	\brief Set the steve controller object for the simulation.
*/

int stSetControllerName(stSteveData *data, brEngine *engine, char *controller) {
    if(data->controllerName) {
        stParseError(engine, PE_REDEFINITION, "Redefinition of \"Controller\" object");
        return -1;
    }

    data->controllerName = slStrdup(controller);

    return 0;
}
