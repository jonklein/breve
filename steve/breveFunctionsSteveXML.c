#include "steve.h"

/*!
	\brief Archives an entire simulation to a file.
*/

int stCWriteXMLEngine(brEval args[], brEval *target, brInstance *i) {
	 char *filename = BRSTRING(&args[0]);

	 if(i->engine->outputPath) {
		  char *newfilename;
		  newfilename = slMalloc(strlen(i->engine->outputPath) + 2 + strlen(filename));
		  sprintf(newfilename, "%s/%s", i->engine->outputPath, filename);
		  BRINT(target) = stXMLWriteSimulationToFile(newfilename, i->engine);
		  slFree(newfilename);
	 } else {
		  BRINT(target) = stXMLWriteSimulationToFile(filename, i->engine);
	 }

	 return EC_OK;
}

/*!
	\brief Archives an instance to a string.
*/

int stCArchiveXMLObject(brEval args[], brEval *target, brInstance *i) {
	 char *filename = BRSTRING(&args[1]);
	 stInstance *archive;

	 archive = BRINSTANCE(&args[0])->userData;

	 if(i->engine->outputPath) {
		  char *newfilename;
		  newfilename = slMalloc(strlen(i->engine->outputPath) + 2 + strlen(filename));
		  sprintf(newfilename, "%s/%s", i->engine->outputPath, filename);
		  BRINT(target) = stXMLWriteObjectToFile(archive, newfilename, 0);
		  slFree(newfilename);
	 } else {
		  BRINT(target) = stXMLWriteObjectToFile(archive, filename, 0);
	 }

	 return EC_OK;
}

/*!
	\brief Dearchives and returns an instance from a file.
*/

int stCDearchiveXMLObject(brEval args[], brEval *target, brInstance *i) {
	char *filename = brFindFile(i->engine, BRSTRING(&args[0]), NULL);
	stInstance *si;

	if(!filename) {
		slMessage(DEBUG_ALL, "Cannot locate file \"%s\" for object dearchive\n", BRSTRING(&args[0]));
		BRINSTANCE(target) = NULL;
		return EC_OK;
	}


	si = stXMLDearchiveObjectFromFile(i->engine, filename);
	BRINSTANCE(target) = si->breveInstance;

	slFree(filename);

	return EC_OK;
}

/*!
	\brief Dearchives and returns an instance from an XML string.
*/

int stCDearchiveXMLObjectFromString(brEval args[], brEval *target, brInstance *i) {
	stInstance *si = stXMLDearchiveObjectFromString(i->engine, BRSTRING(&args[0]));

	if(!si) {
		slMessage(DEBUG_ALL, "error decoding XML message from string\n");
		STINSTANCE(target) = NULL;
		return EC_OK;
	}

	 BRINSTANCE(target) = si->breveInstance;

	 return EC_OK;
}

void breveInitXMLFuncs(brNamespace *n) {
	 brNewBreveCall(n, "writeXMLEngine", stCWriteXMLEngine, AT_INT, AT_STRING, 0);
	 brNewBreveCall(n, "archiveXMLObject", stCArchiveXMLObject, AT_NULL, AT_INSTANCE, AT_STRING, 0);
	 brNewBreveCall(n, "dearchiveXMLObject", stCDearchiveXMLObject, AT_INSTANCE, AT_STRING, 0);
	 brNewBreveCall(n, "dearchiveXMLObjectFromString", stCDearchiveXMLObjectFromString, AT_INSTANCE, AT_STRING, 0);
}
