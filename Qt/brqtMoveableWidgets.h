#ifndef _BRQTMOVEABLEWIDGETS_H
#define _BRQTMOVEABLEWIDGETS_H

#include <QPushButton>
#include <QMoveEvent>
#include <QWidget>
#include <QSlider>
#include <QCheckBox>
#include <QPainter>
#include <QGLWidget>
#include <QLineEdit>

#include "brqtMainWindow.h"
#include "brqtGLWidget.h"

template < class T > class brqtMoveable : public T {
	public:
		brqtMoveable( brqtMainWindow *parent = 0 ) : T( parent ) {
			// I unexpectedly <heart> templates! 

			_parent = parent;

			_isSelected = false;
			_resizing = false;
			_canDelete = false;
 		}

		void paintEvent( QPaintEvent *e ) {
			if( !_parent->isEditing() ) {
				T::paintEvent( e );
				return;
			}

			QPainter paint(this);
			QPen pen( Qt::DashLine );

			if( _isSelected ) pen.setColor( Qt::black );
			else pen.setColor( Qt::gray );

			paint.setPen( pen );

			int sx, sy, ex, ey;

			sx = sy = 2;
			ex = T::width() - 4;
			ey = T::height() - 4;

			paint.drawLine( sx, sy, sx, ey );
			paint.drawLine( sx, ey, ex, ey );
			paint.drawLine( ex, ey, ex, sy );
			paint.drawLine( ex, sy, sx, sy );

			pen.setStyle( Qt::SolidLine );
			paint.setBrush( QBrush( Qt::gray ) );

			paint.setPen( pen );

			sx = sy = 0;
			ex = T::width() - 7;
			ey = T::height() - 7;

			paint.drawEllipse( sx, sy, 5, 5 );
			paint.drawEllipse( sx, ey, 5, 5 );
			paint.drawEllipse( ex, sy, 5, 5 );
			paint.drawEllipse( ex, ey, 5, 5 );

			paint.drawEllipse( ex / 2, sy, 5, 5 );
			paint.drawEllipse( ex / 2, ey, 5, 5 );
			paint.drawEllipse( sx, ey / 2, 5, 5 );
			paint.drawEllipse( ex, ey / 2, 5, 5 );

			T::paintEvent( e );
		}

		void mousePressEvent( QMouseEvent *event ) {
			if( !_parent->isEditing() || !event->buttons() ) {
				T::mousePressEvent( event );
				return;
			}

			_pressOffset = event->pos();

			bool xborder = ( _pressOffset.x() < 10 || ( T::width()  - _pressOffset.x() ) < 10 );
			bool yborder = ( _pressOffset.y() < 10 || ( T::height() - _pressOffset.y() ) < 10 );

			_originalSize = T::size();

			if( xborder && yborder ) _resizing = true;
			else _resizing = false;
		}

		void mouseMoveEvent( QMouseEvent *event ) {
			if( !_parent->isEditing() ) {
				T::mouseMoveEvent( event );
				return;
			}

			if( !_resizing ) {
				T::move( T::x() + event->pos().x() - _pressOffset.x(), 
						 T::y() + event->pos().y() - _pressOffset.y() );
			} else {
				int newWidth  = _originalSize.width()  + event->pos().x() - _pressOffset.x();
				int newHeight = _originalSize.height() + event->pos().y() - _pressOffset.y();

				T::resize( newWidth, newHeight );
			}

			_parent->repaint();
		}

	private:
		brqtMainWindow *_parent;
		QPoint _pressOffset;
		QSize _originalSize;

		bool _resizing;
		bool _canDelete;
		bool _isSelected;
};

brqtMoveable< QPushButton  > *brqtMoveablePushButton( brqtMainWindow *w );
brqtMoveable< QGroupBox    > *brqtMoveableGLWidget( brqtMainWindow *w );
brqtMoveable< QGroupBox    > *brqtMoveableRadioButtonGroup( brqtMainWindow *w );
brqtMoveable< QSlider      > *brqtMoveableVerticalSlider( brqtMainWindow *w );
brqtMoveable< QSlider      > *brqtMoveableHorizontalSlider( brqtMainWindow *w );
brqtMoveable< QCheckBox    > *brqtMoveableCheckBox( brqtMainWindow *w );
brqtMoveable< QLabel       > *brqtMoveableLabel( brqtMainWindow *w );
brqtMoveable< QLineEdit    > *brqtMoveableLineEdit( brqtMainWindow *w );


#endif // _BRQTMOVEABLEWIDGETS_H 
