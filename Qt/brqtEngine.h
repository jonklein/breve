
#ifndef BRQTENGINE
#define BRQTENGINE

#include "brqtGLWidget.h"

#include "kernel.h"

void *brqtEngineLoop(void *e);

class brqtEngine : public QObject {
public:
    brqtEngine( const char *inSimulationText, const char *inSimulationName, brqtGLWidget *inGLView );
    
    ~brqtEngine() {
		_stop = 1;

		if( _glwidget )
			_glwidget->setEngine( NULL );

		killTimer( _timerID );
		brEngineFree( _engine );
    }

	void timerEvent( QTimerEvent* ) {
		brEngineIterate( _engine );
		_glwidget->updateGL();
	}

    brEngine *_engine;
    brqtGLWidget *_glwidget;
    
    void pause() {
		if(_paused) {
			_paused = 0;
			_timerID = startTimer( 10 );
		} else {
			_paused = 1;
			killTimer( _timerID );
		}
	}
    
	int _timerID;
    bool _stop;
    bool _paused;
};

#endif
