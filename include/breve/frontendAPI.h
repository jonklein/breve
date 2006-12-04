
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

#ifdef __cplusplus
extern "C" {
#endif

/*!
	This function should return a pointer to your language specific 
	frontend-data.  This pointer is stored in the \ref breveFrontend 
	field data.  You can use this field later when the \ref 
	breveFrontend is passed to \ref breveFrontendLoadSimulation, 
	\ref breveFrontendLoadSavedSimulation or \ref breveFrontendCleanupData.
*/

void *brInitFrontendLanguages( brEngine *engine );

/*!
	This function uses the language frontend data (found in the \ref 
	breveFrontend field data) to load a simulation from the specified file.
	The code variable contains the text of the file.
*/

int brLoadSimulation( brEngine *engine, const char *code, const char *file );

/*!
	\brief [TO BE IMPLEMENTED BY THE LANGUAGE FRONTEND] Load an archived simulation from a given file.

	This function uses the language frontend data (found in the \ref 
	breveFrontend field data) to load an archived simulation.  The 
	simulation archive is given as the xmlFile.  The file containing 
	the original simulation code is provided as the file argument,
	with the text of that file in the code argument.
*/

int brLoadSavedSimulation( brEngine *engine, const char *code, const char *file, const char *xmlFile );

#ifdef __cplusplus
}
#endif

/*@}*/
