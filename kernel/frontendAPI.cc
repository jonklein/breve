#include "kernel.h"
#include "java.h"

/*!
	\brief Creates a breveFrontend structure containing a valid \ref brEngine.

	This function sets up the breveFrontend structure and creates a
	brEngine.  If this breve frontend is being run from the command line,
	the calling function should provide the input argument count (argc) and
	the input argument pointers (argv).  If this information is not 
	available, argc should be 0, and argv should be NULL.
*/

breveFrontend *breveFrontendInit(int argc, char **argv) {
	breveFrontend *frontend = new breveFrontend;

	frontend->engine = brEngineNew();
	frontend->engine->argc = argc;
	frontend->engine->argv = argv;

	// initialize frontend languages below:

#if HAVE_LIBJAVA
	// brJavaInit(frontend->engine);
#endif

	return frontend;
}

/*!
	\brief Deletes the breveFrontend.
*/

void breveFrontendDestroy(breveFrontend *frontend) {
	delete frontend;
}
