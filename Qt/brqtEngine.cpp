#include "brqtEngine.h"

#include <QApplication>

brqtEngine *brqtEngine::_currentEngine = NULL;

brqtEngine::brqtEngine( const char *inSimulationText, const char *inSimulationName, brqtGLWidget *inGLView, QMenu *inSimulationMenu ) {
	_glwidget = inGLView;

	_currentEngine = this;

	_simulationMenu = inSimulationMenu;

	_timerDelay = 0;

	_engine = brEngineNew();
	_glwidget -> setEngine( _engine );

	brEngineSetUpdateMenuCallback( _engine, updateSimulationMenuCallback );
	brEngineSetPauseCallback( _engine, pauseCallback );
	brEngineSetUnpauseCallback( _engine, unpauseCallback );

	QString classPath;

	#ifdef Q_WS_MAC
        
	classPath = QApplication::applicationDirPath() + "/../Resources/classes";
        
        #else
        
	classPath = QApplication::applicationDirPath() + "../lib/classes";
        
	#endif


	brAddSearchPath( _engine, classPath.toAscii().constData() );

	brInitFrontendLanguages( _engine );

	if( brLoadSimulation( _engine, inSimulationText, inSimulationName ) == EC_OK ) {
		_timerID = startTimer( _timerDelay );
		_error = false;
	} else {
		_timerID = -1;
		_error = true;
	}
}

brqtEngine::~brqtEngine() {
	_stop = 1;

	if( _glwidget )
		_glwidget->setEngine( NULL );

	if( _timerID != -1 )
		killTimer( _timerID );

	brEngineFree( _engine );

	_currentEngine = NULL;
}

void brqtEngine::updateSimulationMenu( brInstance *inInstance ) {
	_simulationMenu -> clear();

	for( int n = 0; n < inInstance -> _menus.size(); n++ ) {
		brMenuEntry *entry = inInstance -> _menus[ n ];
		_simulationMenu -> addAction( entry -> title );
	}
}





/**
 * Static engine callbacks
 */

int brqtEngine::pauseCallback() {
	if( _currentEngine && _currentEngine -> _paused == false ) 
		_currentEngine -> togglePaused();
}

int brqtEngine::unpauseCallback() {
	if( _currentEngine && _currentEngine -> _paused == true ) 
		_currentEngine -> togglePaused();
}

void brqtEngine::updateSimulationMenuCallback( brInstance *inInstance ) {
	if( _currentEngine ) 
		_currentEngine -> updateSimulationMenu( inInstance );
}

