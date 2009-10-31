
#ifndef BRQTENGINE
#define BRQTENGINE

#include "brqtGLWidget.h"
#include "kernel.h"

#include <QMenu>

class brqtEngine : public QObject {
	public:
		brqtEngine( const char *inSimulationText, const char *inSimulationName, brqtGLWidget *inGLView, QMenu *inSimMenu );
	
		~brqtEngine();

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
		static int		pauseCallback();
		static int		unpauseCallback();
		static void		updateSimulationMenuCallback( brInstance *inObject );

		void			updateSimulationMenu( brInstance *inObject );

		int 			_timerID;
		bool 			_stop;
		bool 			_paused;

		int 			_timerDelay;

		bool			_error;

		QMenu*			_simulationMenu;

		static brqtEngine*	_currentEngine;		
};

#endif
