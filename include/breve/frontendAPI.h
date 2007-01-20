
/** \defgroup breveFrontendAPI The breve frontend API: embedding breve simulations into a program.
 * 
 * These functions are used to load breve simulations.
 */

/*@{*/

#ifdef __cplusplus
extern "C" {
#endif

/**
 * This function must be called before simulations can be loaded.
 */

void *brInitFrontendLanguages( brEngine *engine );

/**
 * Load a breve simulation with a newly created brEngine.
 */

int brLoadSimulation( brEngine *engine, const char *code, const char *file );

/**
 * Loads a single file (in any language that breve understands).
 */

int brLoadFile( brEngine *engine, const char *code, const char *file );

/**
 * Load a breve simulation with a newly created brEngine.
 */

int brLoadSavedSimulation( brEngine *engine, const char *code, const char *file, const char *xmlFile );

#ifdef __cplusplus
}
#endif

/*@}*/
