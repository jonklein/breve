#ifndef _BRQTMAINWINDOW_H
#define _BRQTMAINWINDOW_H

#include <QPushButton>
#include <QMoveEvent>
#include <QWidget>
#include <QSlider>
#include <QCheckBox>
#include <QPainter>
#include <QGLWidget>
#include <QFileDialog>
#include <QVariant>

#include "ui_brqtMainWindow.h"
#include "brqtWidgetPalette.h"
#include "brqtEngine.h"
#include "brqtGLWidget.h"

class brqtEditorWindow;

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

		void closeDocument();

	public slots:
		void 					toggleEditing();
		void 					openDocument();
		void 					newDocument();
		void 					toggleSimulation();

		void 					openDemo( QAction *inAction ) {
			QString path = inAction -> data().toString();
			printf( "%s\n", path.toAscii().constData() );
		}

		void 					openHTML( QAction *inAction ) {
			QString path = inAction -> data().toString();
			printf( "%s\n", path.toAscii().constData() );
		}

	private:
		QMenu*									buildMenuFromDirectory( const char *inDirectory, QMenu *inParent, QStringList *inFilters, const char *inSlot );

		bool 									_editing;

		brqtWidgetPalette 						_palette;

		brqtEngine*								_engine;
		brqtGLWidget*							_glview;

		std::vector< brqtEditorWindow* >		_documents;

		Ui::brqtMainWindow						_ui;
};

#endif // _BRQTMAINWINDOW_H
