#include "brqtEngine.h"

void *brqtEngineLoop(void *e) {
    brqtEngine *engine = e;

    while(! engine->_stop) {
	brEngineIterate( engine->_engine);
    }
    return NULL;
}
