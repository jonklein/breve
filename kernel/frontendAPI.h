/*!
	\brief Holds data for a frontend breve interface.
*/

struct brFrontend {
    brEngine *engine;
    // brObjectType objectType;
    void *data;
};

brFrontend *brFrontendInit(int argc, char ** argv);

//
// TO BE IMPLEMENTED BY THE LANGUAGE FRONTEND:
//

/*!
	\brief Create the language-frontend data.

	This function should return a pointer to your language specific 
	frontend-data.  This pointer is stored in the \ref brFrontend 
	field data.  You can use this field later when the \ref 
	brFrontend is passed to \ref breveFrontendLoadSimulation, 
	\ref breveFrontendLoadSavedSimulation or \ref breveFrontendCleanup.
*/

void *breveFrontendInitData(brEngine *engine);

/*!
	\brief Start a new simulation from a given file.

	This function uses the language frontend data (found in the \ref 
	brFrontend field data) to load a simulation from the specified file.
	The code variable contains the text of the file.
*/

int breveFrontendLoadSimulation(brFrontend *d, char *code, char *file);

/*!
	\brief Load an archived simulation from a given file.

	This function uses the language frontend data (found in the \ref 
	brFrontend field data) to load an archived simulation.  The 
	simulation archive is given as the xmlFile.  The file containing 
	the original simulation code is provided as the file argument,
	with the text of that file in the code argument.
*/

int breveFrontendLoadSavedSimulation(brFrontend *d, char *code, char *file, char *xmlFile);

/*!
	\brief Called to destroy the language-frontend data.

	This function should destroy the language-frontend data and all 
	memory that was allocated with \ref breveFrontendInitData.
*/

void breveFrontendCleanupData(void *data);
