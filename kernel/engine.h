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

#ifdef WINDOWS
// #include <winsock2.h>
#endif
 
/* the maximum error size */

#define BR_ERROR_TEXT_SIZE 4096

/* the size of the steve runtime evaluation stack */

#define ST_STACK_SIZE   0x4000

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
	brInstance *instance;
	brMethod *method;
	char *name;
	double time;
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

struct brEngine {
	slWorld *world;
	slCamera *camera;

	char simulationWillStop;

	brSoundMixer *soundMixer;

#ifdef HAVE_LIBAVCODEC
	slMovie *movie;
#endif

#ifdef HAVE_LIBOSMESA
	GLubyte *osBuffer;
	OSMesaContext osContext;
	unsigned char useOSMesa;
#endif 

	int useMouse;
	int mouseX;
	int mouseY;

	double iterationStepSize;

	int evalStats[100];
	double speedFactor;

	FILE *logFile;

	slStack *drawContexts;

	char *controllerName;
	brInstance *controller;

	brNamespace *objects;
	brNamespace *internalMethods;

	slStack *iterationInstances;
	slStack *postIterationInstances;
	slStack *instances;

	brMenuList menu;

	// runtime error info 

	brErrorInfo error;

	char *outputPath;
	char *path;

	// plugin, plugins, plugins!

	slList *dlPlugins;

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

	slList *windows;

	slList *searchPath;

	// a list of events and the date of the next event

	slList *events;
	double nextEvent;

	// the stack pointer and memory for running steve code 

	char stackBase[ST_STACK_SIZE];
	char *stack;
	stStackRecord *stackRecord;

	// evalList sort data... 

	void **sortVector;
	int sortVectorLength;
	// stMethod *sortMethod;
	int evalListSortError;
	brInstance *sortObject;

	// iTunes plugin data

	briTunesData *iTunesData;

	//
	// CALLBACK FUNCTIONS AND RELATED DATA FOR THE APPLICATION FRONTEND
	//

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
	void (*interfaceSetNibCallback)(char *file);

	// keypress callback 

	int (*keyCallback)(void *data, char key);

	// pause callback

	int (*pauseCallback)(void *data);

	void *(*newWindowCallback)(char *name, slGraph *graph);
	void (*freeWindowCallback)(void *g);
	void (*renderWindowCallback)(void *g);
};

enum versionRequiermentOperators {
	VR_LT = 1,
	VR_GT,
	VR_LE,
	VR_GE,
	VR_EQ,
	VR_NE
};

brEvent *brEngineAddEvent(brEngine *e, brInstance *i, char *name, double time);
void brEventFree(brEvent *e);

int brEngineSetController(brEngine *e, brInstance *controller);

int brEngineIterate(brEngine *e);

void brEngineSetIOPath(brEngine *e, char *path);
char *brOutputPath(brEngine *e, char *filename);

brEngine *brEngineNew();
void brMakeiTunesData(brEngine *e);
void brEngineFree(brEngine *e);

void stFreeDefine(void *d);

void brPauseTimer(brEngine *e);
void brUnpauseTimer(brEngine *e);

void brInstanceRemove(brEngine *engine, brInstance *o);

void brAddToInstanceLists(brInstance *i);
void brRemoveFromInstanceLists(brInstance *i);

int brFileLogWrite(void *m, const char *buffer, int length);

void brAddSearchPath(brEngine *e, char *name);
char *brFindFile(brEngine *e, char *file, struct stat *st);
void brFreeSearchPath(brEngine *e);

brMenuEntry *brAddMenuItem(brInstance *i, char *name, char *title);
brMenuEntry *brAddContextualMenuItem(brObject *o, char *method, char *title);

int brMenuCallback(brEngine *e, brInstance *i, int n);
int brMenuCallbackByName(brEngine *e, char *name);
brInstance *brClickCallback(brEngine *e, int n);
int brDragCallback(brEngine *e, int x, int y);
int brKeyCallback(brEngine *e, char keyCode, int isDown);
int brInterfaceCallback(brEngine *e, int interfaceID, char *string);

void stSetParseEngine(brEngine *e);

void stParseError(brEngine *e, int type, char *proto, ...);

void stSetParseString(char *string, int length);

void brPrintVersion();

void brFreeObjectSpace(brNamespace *ns);

void brEngineRenderWorld(brEngine *e, int crosshair);

void brFreeInternalFunction(void *d);

void brEvalError(brEngine *e, int type, char *proto, ...);
