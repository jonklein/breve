
/** \defgroup breveFrontendAPI The breve frontend API: using a new language frontend with an existing breve application frontend
 
	Functions required for interfacing custom language frontends
	with the standard breve application frontends (the same 
	application frontends that are provided in the binary 
	breve distribution).

	Most of the functions listed below are to be implemented
	by the programmer who is creating a new language frontend.
	The source code documented here is that of the default
	steve-frontend, which may be used as a guide for other 
	implementations.
*/

/*@{*/

/*!
	\brief Holds data for a frontend breve interface.

	This structure simply holds a the breve simulation engine 
	and a custom data pointer that is used for simulation
	frontends.  It is the data that gets passed to the various 
	breveFrontend functions which comprise the breveFrontendAPI.
*/

typedef struct breveFrontend breveFrontend;

struct breveFrontend {
    brEngine *engine;
    void *data;
};

#ifdef __cplusplus
extern "C" {
#endif

breveFrontend *breveFrontendInit(int argc, char ** argv);
void breveFrontendDestroy(breveFrontend *frontend);

/*!
	\brief [TO BE IMPLEMENTED BY THE LANGUAGE FRONTEND] Create the language-frontend data.

	This function should return a pointer to your language specific 
	frontend-data.  This pointer is stored in the \ref breveFrontend 
	field data.  You can use this field later when the \ref 
	breveFrontend is passed to \ref breveFrontendLoadSimulation, 
	\ref breveFrontendLoadSavedSimulation or \ref breveFrontendCleanupData.
*/

void *breveFrontendInitData(brEngine *engine);

/*!
	\brief [TO BE IMPLEMENTED BY THE LANGUAGE FRONTEND] Start a new simulation from a given file.

	This function uses the language frontend data (found in the \ref 
	breveFrontend field data) to load a simulation from the specified file.
	The code variable contains the text of the file.
*/

int breveFrontendLoadSimulation(breveFrontend *d, char *code, char *file);

/*!
	\brief [TO BE IMPLEMENTED BY THE LANGUAGE FRONTEND] Load an archived simulation from a given file.

	This function uses the language frontend data (found in the \ref 
	breveFrontend field data) to load an archived simulation.  The 
	simulation archive is given as the xmlFile.  The file containing 
	the original simulation code is provided as the file argument,
	with the text of that file in the code argument.
*/

int breveFrontendLoadSavedSimulation(breveFrontend *d, char *code, char *file, char *xmlFile);

/*!
	\brief [TO BE IMPLEMENTED BY THE LANGUAGE FRONTEND] Called to destroy the language-frontend data.

	This function should destroy the language-frontend data and all 
	memory that was allocated with \ref breveFrontendInitData.
*/

void breveFrontendCleanupData(void *data);

#ifdef __cplusplus
}
#endif

/*@}*/
