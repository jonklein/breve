#ifndef _BRQTMAINWINDOW_H
#define _BRQTMAINWINDOW_H

#include <QPushButton>
#include <QMoveEvent>
#include <QWidget>
#include <QSlider>
#include <QCheckBox>
#include <QPainter>
#include <QGLWidget>

#include "ui_brqtMainWindow.h"
#include "brqtWidgetPalette.h"

class brqtMainWindow : public QMainWindow {
	Q_OBJECT

	public:
		brqtMainWindow();

		void dragEnterEvent( QDragEnterEvent *event );

		void dropEvent( QDropEvent *event );

		bool isEditing() { return _editing; };

		void setEditing( bool e ) {
			_editing = e; 
		}

	public slots:
		void toggleEditing() {
			_editing = !_editing;

			if( _editing ) {
				_ui.editButton->setText( tr( "Finished Editing" ) );
				_palette.show();
			} else {
				_ui.editButton->setText( tr( "Edit Interface" ) );
				_palette.hide();
			}

			repaint();
		}

	private:
		bool _editing;

		QPushButton *_editButton;

		brqtWidgetPalette _palette;

		Ui::brqtMainWindow	_ui;
};

#endif // _BRQTMAINWINDOW_H
