
#include <QMouseEvent>

#include "brqtGLWidget.h"
#include "glIncludes.h"

brqtGLWidget::brqtGLWidget( QWidget* parent ) : QGLWidget( parent ) {
	_engine = NULL;
	_buttonMode = 0;
}

brqtGLWidget::~brqtGLWidget() { 
} 

void brqtGLWidget::paintGL() {
	if (_engine) {
		_engine -> draw();
	} else {
		glClearColor( 0, 0, 0, 1 );
		glClear( GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT );
	}
}

void brqtGLWidget::resizeGL( int w, int h ) {
	if( _engine ) 
		_engine-> camera -> setBounds( w, h );
}

void brqtGLWidget::mousePressEvent ( QMouseEvent *e) {
	if( !_engine ) 
		return;
	
	_lastPosition = e->pos();
	
	brClickAtLocation( _engine, e->x(), e->y() );

	updateGL();
}

void brqtGLWidget::mouseMoveEvent ( QMouseEvent *e ) {
	const QPoint &pos = e->pos();
	
	double dx = pos.x() - _lastPosition.x(), dy = pos.y() - _lastPosition.y();
	
	_lastPosition = pos;
		
	if(!_engine) 
		return;

	switch( _buttonMode) {
		case 0:
			_engine -> camera -> rotateWithMouseMovement( dx, dy );
			break;
		case 1:
			_engine -> camera -> zoomWithMouseMovement( dx, dy );
			break;
		case 2:
			_engine -> camera -> moveWithMouseMovement( dx, dy );
			break;
		case 3:
			brDragCallback( _engine, pos.x(), pos.y() );
			break;
	}

	updateGL();
}

