
#ifndef BRQTENGINE
#define BRQTENGINE

#include "brqtGLWidget.h"

#include "kernel.h"

void *brqtEngineLoop(void *e);

class brqtEngine : public QObject {
public:
    brqtEngine( brEngine *e, brqtGLWidget *w ) {
		_engine = e;
		_glwidget = w;
		w->setEngine(e);

		_timerID = startTimer(10);
		_paused = 0;
    }
    
    ~brqtEngine() {
		_stop = 1;
		_glwidget->setEngine(NULL);
		killTimer( _timerID);
		brEngineFree( _engine);
    }

	void timerEvent(QTimerEvent*) {
		brEngineIterate( _engine);
		_glwidget->updateGL();
	}

    brEngine *_engine;
    brqtGLWidget *_glwidget;
    
    void pause() {
		if(_paused) {
			_paused = 0;
			_timerID = startTimer( 10);
		} else {
			_paused = 1;
			killTimer( _timerID);
		}
	}
    
	int _timerID;
    bool _stop;
    bool _paused;
};

#endif
