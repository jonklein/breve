#define ST_STACK_SIZE   0x4000

struct stSteveData {
	brObjectType steveObjectType;

	char *controllerName;

    stExp *singleStatement;
    stMethod *singleStatementMethod;

	slList *allObjects;
	slList *filesSeen;

	brNamespace *defines;

	slList *freedInstances;
	char retainFreedInstances;

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
void stObjectAllocationReport(void);
void stParseError(brEngine *, int, char *, ...);

void stSteveCleanup(stSteveData *);

int stPreprocess(stSteveData *, brEngine *, char *);

void stFreeDefine(void *);

int stSetControllerName(stSteveData *, brEngine *, char *);
