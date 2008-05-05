
#ifndef BRQTENGINE
#define BRQTENGINE

#include "brqtGLWidget.h"

#include "kernel.h"

class brqtEngine : public QObject {
	public:
		brqtEngine( const char *inSimulationText, const char *inSimulationName, brqtGLWidget *inGLView );
	
		~brqtEngine() {
			_stop = 1;

			if( _glwidget )
				_glwidget->setEngine( NULL );

			if( _timerID != -1 )
				killTimer( _timerID );

			brEngineFree( _engine );
		}

		void timerEvent( QTimerEvent* ) {
			brEngineIterate( _engine );
			_glwidget->updateGL();
		}

		brEngine 		*_engine;
		brqtGLWidget 		*_glwidget;

		bool error() { return _error; }
		
		bool togglePaused() {
			if(_paused) {
				_paused = 0;
				_timerID = startTimer( _timerDelay );
			} else {
				_paused = 1;
				killTimer( _timerID );
				_timerID = -1;
			}

			return _paused;
		}

		void setTimerDelay( int inTimerDelay ) {
			_timerDelay = inTimerDelay;

			if( !_paused ) {
				// pause/unpause to regenerate the timer

				togglePaused();
				togglePaused();
			}
		}

	private:
		int 			_timerID;
		bool 			_stop;
		bool 			_paused;

		int 			_timerDelay;

		bool			_error;
};

#endif
