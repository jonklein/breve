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
#include <sys/stat.h>
#include <gsl/gsl_rng.h>

#include "simulation.h"
#include "timeval.h"

#ifdef __cplusplus
#include <vector>
#include <map>
#include <string>
#include <algorithm>
#endif

// the maximum error size 

#define BR_ERROR_TEXT_SIZE 4096

/*!
	\brief A structure to hold information about errors during parsing/execution.
*/

struct brErrorInfo {
	char *file;
	int line;
	unsigned char type;
	char message[BR_ERROR_TEXT_SIZE];
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
	\brief A single menu item.
*/

struct brMenuEntry {
	brInstance *instance;
	slStack *submenus;
	char *method;
	char *title;
	unsigned char enabled;
	unsigned char checked;
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

		gsl_rng *RNG;

		std::vector<brObjectType*> objectTypes;

		bool simulationWillStop;

		brSoundMixer *soundMixer;

		std::vector<brInstance*> freedInstances;

#if HAVE_LIBOSMESA
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

		std::string outputPath;
		char *path;

		// plugin, plugins, plugins!

		std::vector<brDlPlugin*> dlPlugins;

		// the drawEveryFrame flag is a hint to the display engine
		// if set, the application attempts to draw a frame with every
		// iteration of the breve engine. 

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

		//
		// Callback functions to be set by the application frontend
		//

		// callback to update a menu for an instance

		void (*updateMenu)(brInstance *);

		// callback to run save and load dialogs 

		char *(*getSavename)(void);
		char *(*getLoadname)(void);

		// callback to show a generic dialog

		int (*dialogCallback)(char *, char *, char *, char *);
	
		// callback to play a beep sound
	
		int (*soundCallback)(void);
	
		// returns the string identifying the implementation
	
		char *(*interfaceTypeCallback)(void);
	
		// callback to setup and use the OS X interface features
	
		int (*interfaceSetStringCallback)(char *, int);
		void (*interfaceSetCallback)(char *);

		int (*pauseCallback)(void);

		void *(*newWindowCallback)(char *, void *);
		void (*freeWindowCallback)(void *);
		void (*renderWindowCallback)(void *);
};

/*!
	\brief A scheduled event in the breve engine
*/

class brEvent {
	public:
		brEvent(char *name, double time, double interval, brInstance *i);
		~brEvent();

		char *_name;
		double _time;
		double _interval;
		brInstance *_instance;
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

void brEngineLock(brEngine *);
void brEngineUnlock(brEngine *);

brEvent *brEngineAddEvent(brEngine *, brInstance *, char *, double, double);
void brEventFree(brEvent *);

int brEngineSetController(brEngine *, brInstance *);

brInstance *brEngineGetController(brEngine *);

slStack *brEngineGetAllInstances(brEngine *);

int brEngineIterate(brEngine *);

void brEngineSetIOPath(brEngine *, char *);
char *brOutputPath(brEngine *, char *);

brEngine *brEngineNew(void);
void brEngineFree(brEngine *);

void brMakeiTunesData(brEngine *);

void brPauseTimer(brEngine *);
void brUnpauseTimer(brEngine *);

void brAddToInstanceLists(brInstance *);
void brRemoveFromInstanceLists(brInstance *);

int brFileLogWrite(void *, const char *, int);

void brAddSearchPath(brEngine *, char *);
char *brFindFile(brEngine *, char *, struct stat *);
void brFreeSearchPath(brEngine *);

brMenuEntry *brAddMenuItem(brInstance *, char *, char *);
brMenuEntry *brAddContextualMenuItem(brObject *, char *, char *);

void stSetParseEngine(brEngine *);

void brPrintVersion(void);

void brFreeObjectSpace(brNamespace *);

void brEngineRenderWorld(brEngine *, int);

brInternalFunction *brEngineInternalFunctionLookup(brEngine *, char *);

void brEvalError(brEngine *, int, char *, ...);

void brClearError(brEngine *);
int brGetError(brEngine *);

int brEngineSetInterface(brEngine *, char *);

brErrorInfo *brEngineGetErrorInfo(brEngine *);

char *brEngineGetPath(brEngine *);

brNamespace *brEngineGetInternalMethods(brEngine *);

int brEngineGetDrawEveryFrame(brEngine *);

void brEngineSetSoundCallback(brEngine *, int (*)(void));
void brEngineSetDialogCallback(brEngine *, int (*)(char *, char *, char *, char *));

void brEngineSetPauseCallback(brEngine *, int (*)(void));
void brEngineSetGetLoadnameCallback(brEngine *, char *(*)(void));
void brEngineSetGetSavenameCallback(brEngine *, char *(*)(void));

void brEngineSetInterfaceInterfaceTypeCallback(brEngine *, char *(*)(void));
void brEngineSetInterfaceSetStringCallback(brEngine *, int (*)(char *, int));
void brEngineSetInterfaceSetNibCallback(brEngine *, void (*)(char *));
void brEngineSetUpdateMenuCallback(brEngine *, void (*)(brInstance *));

slCamera *brEngineGetCamera(brEngine *);
slWorld *brEngineGetWorld(brEngine *);

#ifdef __cplusplus
}
#endif

#endif
