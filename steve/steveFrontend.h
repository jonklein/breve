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

stSteveData *stSteveInit(brEngine *engine);

int stSubclassCallback(void *c1, void *c2);

int stLoadFiles(stSteveData *s, brEngine *engine, char *code, char *file);
int stParseFile(stSteveData *s, brEngine *engine, char *filename);

int stLoadSimulation(stSteveData *sdata, brEngine *engine, char *code, char *file);
int stLoadSavedSimulation(stSteveData *sdata, brEngine *engine, char *code, char *file, char *xmlfile);

int stParseBuffer(stSteveData *s, brEngine *engine, char *buffer, char *filename);
char *stNewStParseTrack(stSteveData *e, char *name);
void stFreeParseTrack(stSteveData *e);
char *stFindParseTrack(slList *l, char *name);
stVersionRequirement *stMakeVersionRequirement(float version, int operation);
int stCheckVersionRequirement(float version, stVersionRequirement *r);
void stObjectAllocationReport();
void stParseError(brEngine *e, int type, char *proto, ...);

void stSteveCleanup(stSteveData *steveData);

int stPreprocess(stSteveData *steveData, brEngine *engine, char *line);

void stFreeDefine(void *d);

int stSetControllerName(stSteveData *steveData, brEngine *engine, char *controller);
