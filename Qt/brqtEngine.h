
#ifndef BRQTENGINE
#define BRQTENGINE

#include "kernel.h"
#include "breveGLWidget.h"

void *brqtEngineLoop(void *e);

class brqtEngine {
public:
    brqtEngine(brEngine *e, breveGLWidget *w) {
	_engine = e;
	_glwidget = w;
	w->setEngine(e);
	pthread_create(&_thread, NULL, brqtEngineLoop, this);
    }
    
    ~brqtEngine() {
	_stop = 1;
	pthread_join( _thread, NULL);
    }
    
    brEngine *_engine;
    breveGLWidget *_glwidget;
    
    void pause();
    
    pthread_t _thread;
    bool _stop;
};
#endif
