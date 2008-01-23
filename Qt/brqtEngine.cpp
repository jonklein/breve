#include "brqtEngine.h"

brqtEngine::brqtEngine( const char *inSimulationText, const char *inSimulationName, brqtGLWidget *inGLView ) {
	_glwidget = inGLView;

	_engine = brEngineNew();
	_glwidget -> setEngine( _engine );

	brAddSearchPath( _engine, "../lib/classes" );

	brInitFrontendLanguages( _engine );

	if( brLoadSimulation( _engine, inSimulationText, inSimulationName ) == EC_OK ) {
		_timerID = startTimer( 10 );
	}
}


