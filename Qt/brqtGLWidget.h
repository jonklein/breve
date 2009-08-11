#ifndef BRQTGLWIDGET_H
#define BRQTGLWIDGET_H 

#include <QtOpenGL/QGLWidget>
#include <QMouseEvent>

#include "kernel.h"
#include "camera.h"
#include "gldraw.h"

class brqtGLWidget : public QGLWidget {
	Q_OBJECT

	public:
		brqtGLWidget( QWidget* parent );

		~brqtGLWidget();
  
		void setEngine(brEngine *e) { 
			_engine = e; 

			if(_engine) {
				_engine -> camera -> initGL();
				_engine -> camera -> setBounds( width(), height() );
			} 

			updateGL();
		}

	public slots: 
		virtual void 	setButtonMode(int mode) {
			_buttonMode = mode;
		}
  
	protected:
		void 			paintGL();
		void 			resizeGL( int w, int h );

		void 			mousePressEvent( QMouseEvent *e);
		void 			mouseMoveEvent( QMouseEvent *e );

	private:
		brEngine*		_engine;    
		int 			_buttonMode;
		QPoint 			_lastPosition;
};

#endif // BRQTGLWIDGET_H
