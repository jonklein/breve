
#ifndef BRQTENGINE
#define BRQTENGINE

#include "breveGLWidget.h"

#include "kernel.h"

// #include "brqtThread.h"

void *brqtEngineLoop(void *e);

class brqtEngine : public QObject {
public:
    brqtEngine(brEngine *e, breveGLWidget *w) {
	_engine = e;
	_glwidget = w;
	w->setEngine(e);

		struct sched_param param;
		param.sched_priority = sched_get_priority_max(SCHED_OTHER);
		pthread_create(&_thread, NULL, brqtEngineLoop, this);
		pthread_setschedparam(_thread, SCHED_OTHER, &param);

		// min.sched_priority = sched_get_priority_min(SCHED_OTHER);
		// pthread_setschedparam(pthread_self(), SCHED_OTHER, &min);

		// startTimer(10);
    }
    
    ~brqtEngine() {
	_stop = 1;
		_glwidget->setEngine(NULL);
		pthread_join( _thread, NULL);
    }

	void timerEvent(QTimerEvent*) {
		brEngineIterate( _engine);
		_glwidget->updateGL();
	}

    brEngine *_engine;
    breveGLWidget *_glwidget;
    
    void pause();
    
    pthread_t _thread;
    bool _stop;
};


#endif
