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

#ifndef _ENGINE_H
#define _ENGINE_H

#include <pthread.h>

#include "simulation.h"

#ifdef __cplusplus
#include <vector>
#include <map>
#include <string>
#include <algorithm>
#endif

#ifdef WINDOWS
// #include <winsock2.h>
#endif
 
// the maximum error size 

#define BR_ERROR_TEXT_SIZE 4096

/*!
	\brief A structure to hold information about errors during parsing/execution.
*/

struct brErrorInfo {
	char message[BR_ERROR_TEXT_SIZE];
	unsigned char type;
	char *file;
	int line;
};

/*!
	\brief Parse error and evaluation codes used by \ref brEvalError.	
*/

enum parseErrorMessageCodes {
	// parse errors 

	PE_OK = 0,
	PE_PARSE,
	PE_SYNTAX,
	PE_INTERNAL,
	PE_UNKNOWN_SYMBOL,
	PE_UNKNOWN_FUNCTION,
	PE_UNKNOWN_OBJECT,
	PE_REDEFINITION,
	PE_TYPE,
	PE_PROTOTYPE,
	PE_NO_CONTROLLER,
	PE_FILE_VERSION,

	// evaluation errors 

	EE_UNKNOWN_CONTROLLER,
	EE_USER,
	EE_INTERNAL,
	EE_TYPE,
	EE_ARRAY,
	EE_CONVERT,
	EE_FREED_INSTANCE,	
	EE_NULL_INSTANCE,
	EE_MATH,
	EE_SIMULATION,
	EE_BOUNDS,
	EE_UNKNOWN_OBJECT,
	EE_UNKNOWN_METHOD,
	EE_UNKNOWN_KEYWORD,
	EE_MISSING_KEYWORD,
	EE_FILE_NOT_FOUND
};

/*!
	\brief Error codes describing where a parse error occurred.
*/

enum parseErrorCodes {
	BPE_OK = 0,
	BPE_SIM_ERROR,
	BPE_LIB_ERROR
};

/*!
	\brief Holds the menus for a certain instance, as well as a pointer to an update function.
*/

struct brMenuList {
	brMenuEntry **list;
	void (*updateMenu)(brInstance *l);
	int count;
	int maxCount;
};

/*!
	\brief A single menu item.
*/

struct brMenuEntry {
	brMenuList *submenu;
	brInstance *instance;  
	char *method;
	char *title;
	unsigned char enabled;
	unsigned char checked;
};

/*!
	\brief A scheduled event in the breve engine
*/

struct brEvent {
	char *name;
	double time;
	brInstance *instance;
};

/*!
	\brief Visualization data used if this is part of an iTunes plugin.
*/

struct brVisualData {
	unsigned char numWaveformChannels;
	unsigned char waveformData[2][512];
	unsigned char numSpectrumChannels;
	unsigned char spectrumData[2][512];
};

/*!
	\brief Signal data used if this is part of an iTunes plugin.
*/

struct briTunesData {
	struct brVisualData *data;
	int spectrumEntries;
	int waveformEntries; 
};

/*!
	\brief The main breve engine structure.
*/

#ifdef __cplusplus
typedef struct brObjectType brObjectType;

class brEngine {
	public:
		slWorld *world;
		slCamera *camera;

		std::vector<brObjectType*> objectTypes;

		char simulationWillStop;

		brSoundMixer *soundMixer;

		std::vector<brInstance*> freedInstances;

#ifdef HAVE_LIBOSMESA
		GLubyte *osBuffer;
		OSMesaContext osContext;
#endif 

		int useMouse;
		int mouseX;
		int mouseY;

		double iterationStepSize;

		FILE *logFile;

		brInstance *controller;

		std::map<std::string,brObject*> objectAliases;
		std::map<std::string,brObject*> objects;
		brNamespace *internalMethods;

		std::vector<brInstance*> postIterationInstances;
		std::vector<brInstance*> iterationInstances;
		std::vector<brInstance*> instances;
	
		std::vector<brInstance*> instancesToAdd;
		std::vector<brInstance*> instancesToRemove;

		std::vector<brEvent*> events;

		// runtime error info 

		brErrorInfo error;

		char *outputPath;
		char *path;

		// plugin, plugins, plugins!

		std::vector<brDlPlugin*> dlPlugins;

		// the drawEveryFrame flag is a hint to the display engine--if set,
		// the application attempts to draw a frame with every iteration of 
		// the breve engine. 

		unsigned char drawEveryFrame;

		struct timeval startTime;
		struct timeval realTime;

		int argc;
		char **argv;

		int nThreads;
		pthread_mutex_t lock;
		pthread_mutex_t scheduleLock;
		pthread_mutex_t conditionLock;
		pthread_cond_t condition;

		int lastScheduled;

		std::vector<void*> windows;
		std::vector<char*> searchPath;

		// which keys are pressed?

		unsigned char keys[256];

		// evalList sort data... 

		void **sortVector;
		int sortVectorLength;
		int evalListSortError;
		brInstance *sortObject;

		// iTunes plugin data

		briTunesData *iTunesData;

		// *** Callback functions to be set by the application frontend ***

		void *callbackData;

		// callback to run save and load dialogs 

		char *(*getSavename)(void *data);
		char *(*getLoadname)(void *data);

		// callback to show a generic dialog

		int (*dialogCallback)(void *data, char *title, char *message, 
										char *button1, char *button2);
	
		// callback to play a beep sound
	
		int (*soundCallback)(void *data);
	
		// returns the string identifying the implementation
	
		char *(*interfaceTypeCallback)(void *data);
	
		// callback to setup and use the OS X interface features
	
		int (*interfaceSetStringCallback)(char *string, int number);
		void (*interfaceSetCallback)(char *file);

		// pause callback

		int (*pauseCallback)(void *data);

		void *(*newWindowCallback)(char *name, void *graph);
		void (*freeWindowCallback)(void *g);
		void (*renderWindowCallback)(void *g);
};
#endif

enum versionRequiermentOperators {
	VR_LT = 1,
	VR_GT,
	VR_LE,
	VR_GE,
	VR_EQ,
	VR_NE
};

#ifdef __cplusplus
extern "C" {
#endif

void brEngineLock(brEngine *e);
void brEngineUnlock(brEngine *e);

brEvent *brEngineAddEvent(brEngine *e, brInstance *i, char *name, double time);
void brEventFree(brEvent *e);

int brEngineSetController(brEngine *e, brInstance *controller);
brInstance *brEngineGetController(brEngine *e);

slStack *brEngineGetAllInstances(brEngine *e);

int brEngineIterate(brEngine *e);

void brEngineSetIOPath(brEngine *e, char *path);
char *brOutputPath(brEngine *e, char *filename);

brEngine *brEngineNew();
void brMakeiTunesData(brEngine *e);
void brEngineFree(brEngine *e);

void brPauseTimer(brEngine *e);
void brUnpauseTimer(brEngine *e);

void brAddToInstanceLists(brInstance *i);
void brRemoveFromInstanceLists(brInstance *i);

int brFileLogWrite(void *m, const char *buffer, int length);

void brAddSearchPath(brEngine *e, char *name);
char *brFindFile(brEngine *e, char *file, struct stat *st);
void brFreeSearchPath(brEngine *e);

brMenuEntry *brAddMenuItem(brInstance *i, char *name, char *title);
brMenuEntry *brAddContextualMenuItem(brObject *o, char *method, char *title);

void stSetParseEngine(brEngine *e);

void stSetParseString(char *string, int length);

void brPrintVersion();

void brFreeObjectSpace(brNamespace *ns);
void brEngineFreeObjects(brEngine *e);

void brEngineRenderWorld(brEngine *e, int crosshair);

brInternalFunction *brEngineInternalFunctionLookup(brEngine *e, char *name);

void brEvalError(brEngine *e, int type, char *proto, ...);

void brClearError(brEngine *e);
int brGetError(brEngine *e);

int brEngineSetInterface(brEngine *e, char *name);

brErrorInfo *brEngineGetErrorInfo(brEngine *e);

char *brEngineGetPath(brEngine *e);

brNamespace *brEngineGetInternalMethods(brEngine *e);

int brEngineGetDrawEveryFrame(brEngine *e);

void brEngineSetSoundCallback(brEngine *e, int (*callback)(void *));
void brEngineSetDialogCallback(brEngine *e, int (*callback)(void *, char *, char *, char *, char *));

void brEngineSetPauseCallback(brEngine *e, int (*callback)(void*));
void brEngineSetGetLoadnameCallback(brEngine *e, char *(*callback)(void*));
void brEngineSetGetSavenameCallback(brEngine *e, char *(*callback)(void*));

void brEngineSetInterfaceInterfaceTypeCallback(brEngine *e, char *(*interfaceTypeCallback)(void *data));
void brEngineSetInterfaceSetStringCallback(brEngine *e, int (*interfaceSetStringCallback)(char *string, int number));
void brEngineSetInterfaceSetNibCallback(brEngine *e, void (*interfaceSetCallback)(char *file));

slCamera *brEngineGetCamera(brEngine *e);
slWorld *brEngineGetWorld(brEngine *e);

#ifdef __cplusplus
}
#endif

#endif
