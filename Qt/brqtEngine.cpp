#include "brqtEngine.h"

void *brqtEngineLoop(void *e) {
    brqtEngine *engine = e;

    while(1) {
	brEngineIterate( engine->_engine);
	engine->_glwidget->repaint();
    }
    return NULL;
}
