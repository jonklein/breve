
#include "breveGLWidget.h"
#include "glIncludes.h"

/**
 * Constructor that creates a breveGLWidget widget
 */
breveGLWidget::breveGLWidget( QWidget* parent, const char* name )
  : QGLWidget( parent, name )
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
    if (_engine) brEngineRenderWorld(_engine, 0);
	else {
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
  glClearColor( 0.0, 0.0, 0.0, 0.0 ); // Let OpenGL clear to black
  glShadeModel( GL_SMOOTH ); // we want smooth shading . . . try GL_FLAT if you like
}

/*--------------------------------------------------------------------------*/
/**
 * Set up the OpenGL view port, matrix mode, etc.
 */
void breveGLWidget::resizeGL( int w, int h )
{
  glViewport( 0, 0, (GLint)w, (GLint)h );
  glMatrixMode( GL_PROJECTION );
  glLoadIdentity();
  glFrustum(-1.0,1.0,-1.0,1.0,5.0,15.0);
  glMatrixMode( GL_MODELVIEW );
}
