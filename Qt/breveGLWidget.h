/**
 * glfractal.h
 *
 * Brief Definition of breveGLWidget
 *
 * This is a simple QGLWidget displaying an openGL Sierpinski Gasket
 */

#ifndef GLFRACTAL_H
#define GLFRACTAL_H

#include <qgl.h>
#include "kernel.h"

/**
 * breveGLWidget inherits from QGLWidget (http://doc.trolltech.com/2.3/qglwidget.html) <BR>
 * QGLWidget basically allows you to control OpenGL through method calls
 */
class breveGLWidget : public QGLWidget
{
  Q_OBJECT
public:
  breveGLWidget( QWidget* parent, const char* name );
  ~breveGLWidget();
  
  void setEngine(brEngine *e) { 
		_engine = e; 
		slInitGL(_engine->world, _engine->camera);
  }

protected:
    void initializeGL();
    void paintGL();
    void resizeGL( int w, int h );

private:
    brEngine *_engine;    
};

#endif // GLFRACTAL_H
