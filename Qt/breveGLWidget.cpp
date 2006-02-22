
#include "breveGLWidget.h"
#include "glIncludes.h"
//Added by qt3to4:
#include <QMouseEvent>

/**
 * Constructor that creates a breveGLWidget widget
 */
breveGLWidget::breveGLWidget( QWidget* parent, const QGLWidget *share, Qt::WFlags f) 
 : QGLWidget( parent, share, f )
{
   _engine = NULL;
}

breveGLWidget::~breveGLWidget()
{ 
} // end destructor

/*--------------------------------------------------------------------------*/
/**
 * Paint the box. The actual openGL commands for drawing the box are
 * performed here.
 */
void breveGLWidget::paintGL()
{
    if (_engine) {
		brEngineLock(_engine);
		brEngineRenderWorld(_engine, 0);
		brEngineUnlock(_engine);
	} else {
		glClearColor(0, 0, 0, 0);
		glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);
	}
}

/*--------------------------------------------------------------------------*/
/**
 * Set up the OpenGL rendering state, and define display list
 */

void breveGLWidget::initializeGL()
{

}

/*--------------------------------------------------------------------------*/
/**
 * Set up the OpenGL view port, matrix mode, etc.
 */
void breveGLWidget::resizeGL( int w, int h )
{
  if( _engine ) {
    _engine->camera->_originx = w;
    _engine->camera->_originy = h;
  }
}

void breveGLWidget::mousePressEvent ( QMouseEvent *e) {
	if(!_engine) return;
	
	lastPosition = e->pos();
	
	brClickAtLocation(_engine, e->x(), e->y());	

	updateGL();
}

void breveGLWidget::mouseMoveEvent ( QMouseEvent *e ) {
	const QPoint &pos = e->pos();
	
	double dx = pos.x() - lastPosition.x(), dy = pos.y() - lastPosition.y();
	
	lastPosition = pos;
		
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

