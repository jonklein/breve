#include "brqtEngine.h"

void *brqtEngineLoop(void *e) {
    brqtEngine *engine = (brqtEngine*)e;

    while(! engine->_stop) {
	printf("running...\n");
	brEngineIterate( engine->_engine);
    }
    return NULL;
}
