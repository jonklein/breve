#ifndef _STEVE_FRONTEND_H
#define _STEVE_FRONTEND_H

#define ST_STACK_SIZE   0x4000
#define STEVE_TYPE_SIGNATURE		0x73747665

#include <vector>
#include <map>

class stStackRecord;

struct stSteveData {
	stSteveData() {
		controllerName 		= NULL;
		singleStatement 	= NULL;
		singleStatementMethod 	= NULL;
		stack 			= NULL;
		stackRecord 		= NULL;
		retainFreedInstances 	= true;
	}

	brObjectType *steveObjectType;

	char *controllerName;

	stExp *singleStatement;
	stMethod *singleStatementMethod;

	std::vector< stObject* > objects;
	std::vector< std::string > filesSeen;
	std::map< std::string, std::vector< std::string > > 	_includes;
	std::map< std::string, std::vector< std::string > > 	_paths;

	std::map< std::string, brEval* > defines;

	std::vector< stInstance* > freedInstances;
	bool retainFreedInstances;

	// the stack pointer and memory for running steve code

	char stackBase[ ST_STACK_SIZE ];
	char *stack;

	stStackRecord *stackRecord;

};

struct stVersionRequirement {
    float version;
    int operation;
};

stSteveData *stSteveInit(brEngine *);

int stSubclassCallback(void *, void *);

int stLoadFiles( stSteveData *, brEngine *, const char *, const char * );
int stParseFile( stSteveData *, brEngine *, const char * );

int stLoadSimulation( stSteveData *, brEngine *, const char *, const char * );
int stLoadSavedSimulation(stSteveData *, brEngine *, const char *, const char *, const char *);

int stParseBuffer( stSteveData *, brEngine *, const char *, const char * );
char *stNewStParseTrack( stSteveData *, const char * );
void stFreeParseTrack( stSteveData * );
char *stFindParseTrack( slList *, const char * );
stVersionRequirement *stMakeVersionRequirement( float, int );
int stCheckVersionRequirement( float, stVersionRequirement * );
void stObjectAllocationReport( stObject * );
void stParseError( brEngine *, int, const char *, ... );

void stSteveCleanup( void* );

int stPreprocess( stSteveData *, brEngine *, const char *, const char * );

int stSetControllerName(stSteveData *, brEngine *, const char *);

void stSetParseData( stSteveData *, const char *, int );

#endif
