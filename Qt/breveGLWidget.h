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

class breveGLWidget : public QGLWidget
{
  Q_OBJECT
public:
  breveGLWidget( QWidget* parent, const char* name );
  ~breveGLWidget();
  
  void setEngine(brEngine *e) { 
		_engine = e; 

		if(_engine) {
			slInitGL(_engine->world, _engine->camera);

			_drawTimer = startTimer(20);
			_engine->camera->x = width();
			_engine->camera->y = height();
		} else {
			killTimer( _drawTimer);
		}
  }

public slots: 
    virtual void setButtonMode(int mode) {
    _buttonMode = mode;
    }
  
protected:
    void initializeGL();
    void paintGL();
    void resizeGL( int w, int h );

	void mousePressEvent ( QMouseEvent *e);
	void mouseMoveEvent ( QMouseEvent *e );

private:
    brEngine *_engine;    
	int _buttonMode;
	QPoint lastPosition;

	int _drawTimer;

	void timerEvent( QTimerEvent *);
};

#endif // GLFRACTAL_H
