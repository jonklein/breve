#include "kernel.h"

brFrontend *brFrontendInit(int argc, char ** argv) {
	brFrontend *frontend = slMalloc(sizeof(brFrontend));

	frontend->engine = brEngineNew();
	frontend->engine->argc = argc;
	frontend->engine->argv = argv;

	return frontend;
}
