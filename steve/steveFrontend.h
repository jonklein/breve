
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
};

struct stVersionRequirement {
    float version;
    int operator;
};

int stSubclassCallback(brObject *c1, brObject *c2);

int stLoadFiles(stSteveData *s, brEngine *engine, char *code, char *file);
int stParseFile(stSteveData *s, brEngine *engine, char *filename);

int stLoadSimulation(stSteveData *sdata, brEngine *engine, char *code, char *file);
int stLoadSavedSimulation(stSteveData *sdata, brEngine *engine, char *code, char *file, char *xmlfile);

int stParseBuffer(stSteveData *s, brEngine *engine, char *buffer, char *filename);
char *stNewStParseTrack(stSteveData *e, char *name);
void stFreeParseTrack(stSteveData *e);
char *stFindParseTrack(slList *l, char *name);
stVersionRequirement *stMakeVersionRequirement(float version, int operator);
int stCheckVersionRequirement(float version, stVersionRequirement *r);
void stObjectAllocationReport(brEngine *engine);
void stParseError(brEngine *e, int type, char *proto, ...);

stSteveData *stSteveInit();

void stSteveCleanup(stSteveData *steveData, brEngine *engine);

int stPreprocess(stSteveData *steveData, brEngine *engine, char *line);

int stSetControllerName(stSteveData *steveData, brEngine *engine, char *controller);
