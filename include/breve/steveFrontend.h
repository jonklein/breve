#define ST_STACK_SIZE   0x4000

#include <vector>
#include <map>

struct stSteveData {
	stSteveData() {
		controllerName = NULL;
		singleStatement = NULL;
		singleStatementMethod = NULL;
		stack = NULL;
		stackRecord = NULL;

		retainFreedInstances = true;
	}

	brObjectType steveObjectType;

	char *controllerName;

	stExp *singleStatement;
	stMethod *singleStatementMethod;

	std::vector< stObject* > objects;
	std::vector< std::string > filesSeen;

	std::map< std::string, brEval* > defines;

	std::vector< stInstance* > freedInstances;
	bool retainFreedInstances;

	// the stack pointer and memory for running steve code

	char stackBase[ST_STACK_SIZE];
	char *stack;

	stStackRecord *stackRecord;
};

struct stVersionRequirement {
    float version;
    int operation;
};

stSteveData *stSteveInit(brEngine *);

int stSubclassCallback(void *, void *);

int stLoadFiles(stSteveData *, brEngine *, char *, char *);
int stParseFile(stSteveData *, brEngine *, char *);

int stLoadSimulation(stSteveData *, brEngine *, char *, char *);
int stLoadSavedSimulation(stSteveData *, brEngine *, char *, char *, char *);

int stParseBuffer(stSteveData *, brEngine *, char *, char *);
char *stNewStParseTrack(stSteveData *, char *);
void stFreeParseTrack(stSteveData *);
char *stFindParseTrack(slList *, char *);
stVersionRequirement *stMakeVersionRequirement(float, int);
int stCheckVersionRequirement(float, stVersionRequirement *);
void stObjectAllocationReport(stObject *);
void stParseError(brEngine *, int, char *, ...);

void stSteveCleanup(stSteveData *);

int stPreprocess(stSteveData *, brEngine *, char *);

int stSetControllerName(stSteveData *, brEngine *, char *);

void stSetParseData(stSteveData *, char *, int);

