#include <QWidget>
#include <QLabel>
#include <QDrag>

class brqtDragableLabel : public QLabel {
	public:
		brqtDragableLabel( QWidget *parent = 0 ) : QLabel( parent ) {
 		}

		void mousePressEvent( QMouseEvent *event ) {
			QDrag *drag = new QDrag( this );
			QMimeData *mimeData = new QMimeData;

			mimeData->setText( QWidget::objectName() );
			drag->setMimeData( mimeData );
			drag->setPixmap( *pixmap( ) );
			drag->setHotSpot( QPoint( event->x(), event->y() ) );

			drag->start();
		}

	private:

};
