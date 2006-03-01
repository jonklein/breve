
#include <QMouseEvent>

#include "brqtGLWidget.h"
#include "glIncludes.h"

brqtGLWidget::brqtGLWidget( QWidget* parent ) : QGLWidget( parent ) {
   _engine = NULL;
}

brqtGLWidget::~brqtGLWidget() { 
} 

void brqtGLWidget::paintGL()
{
    if (_engine) {
		brEngineLock(_engine);
		brEngineRenderWorld(_engine, 0);
		brEngineUnlock(_engine);
	} else {
		glClearColor( .3, .15, .8, 1.0 );
		glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);
	}
}

void brqtGLWidget::initializeGL() {

}

void brqtGLWidget::resizeGL( int w, int h ) {
  if( _engine ) _engine->camera->setBounds( w, h );
}

void brqtGLWidget::mousePressEvent ( QMouseEvent *e) {
	if(!_engine) return;
	
	_lastPosition = e->pos();
	
	brClickAtLocation(_engine, e->x(), e->y());	

	updateGL();
}

void brqtGLWidget::mouseMoveEvent ( QMouseEvent *e ) {
	const QPoint &pos = e->pos();
	
	double dx = pos.x() - _lastPosition.x(), dy = pos.y() - _lastPosition.y();
	
	_lastPosition = pos;
		
	if(!_engine) return;

	switch( _buttonMode) {
		case 0:
			_engine->camera->rotateWithMouseMovement( dx, dy );
			break;
		case 1:
			_engine->camera->zoomWithMouseMovement( dx, dy );
			break;
		case 2:
			_engine->camera->moveWithMouseMovement( dx, dy );
			break;
		case 3:
			brDragCallback( _engine, pos.x(), pos.y() );
			break;
	}

	updateGL();
}

