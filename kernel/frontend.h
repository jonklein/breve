/*!
	\brief Holds data for a frontend breve interface.
*/

struct brFrontend {
    brEngine *engine;
    brObjectType objectType;
    void *data;
};

brFrontend *breveFrontendInit(int argc, char** argv);
int breveFrontendLoadSimulation(brFrontend* d, char *code, char *file);
int breveFrontendLoadSavedSimulation(brFrontend* d, char *code, char *file, char* xmlFile);
void breveFrontendCleanup(brFrontend* data);
