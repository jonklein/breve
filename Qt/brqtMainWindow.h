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
#include <QUrl>
#include <QDesktopServices>
#include <QTextEdit>

#include "ui_brqtMainWindow.h"

#include "brqtLogWindow.h"
#include "brqtFindDialog.h"
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

		void					closeDocument( brqtEditorWindow *inWidget );

	public slots:
		void 					toggleEditing();
		void 					openDocument();
		void 					saveDocument();
		void 					newDocument();
		void 					toggleSimulation();
		void 					stopSimulation();
		void 					setButtonMode();

		void 					activateLogWindow();


		void 					copy() {
			QTextEdit *editor = focusedTextEdit();
			if( editor )
				editor -> copy();
		}

		void 					paste() {
			QTextEdit *editor = focusedTextEdit();
			if( editor )
				editor -> paste();
		}

		void 					cut() {
			QTextEdit *editor = focusedTextEdit();
			if( editor )
				editor -> cut();
		}

		void 					undo() {
			QTextEdit *editor = focusedTextEdit();
			if( editor )
				editor -> undo();
		}

		void 					redo() {
			QTextEdit *editor = focusedTextEdit();
			if( editor )
				editor -> redo();
		}

		void 					selectAll() {
			QTextEdit *editor = focusedTextEdit();
			if( editor )
				editor -> selectAll();
		}


		void					closeWindow() {
			QWidget *w = QApplication::activeWindow();

			if( w && w != this ) 
				w -> close();
		}

		void 					openDemo( QAction *inAction ) {
			QString s = inAction -> data().toString();
			openDocument( s );
		}

		void 					openHTML( QAction *inAction ) {
			QString s = QString( "file://" ) + inAction -> data().toString();
			QUrl url( s );
			QDesktopServices::openUrl( url );
		}
		
		
		
		void					find() {
			_findDialog.show();
		}

	private:
		brqtEditorWindow*			openDocument( QString &inDocument );

		void					updateSimulationPopup();

		QMenu*					buildMenuFromDirectory( const char *inDirectory, QMenu *inParent, QStringList *inFilters, const char *inSlot );

		QTextEdit*				focusedTextEdit() {
									QWidget *w = QApplication::focusWidget();

									if( w && w->inherits( "QTextEdit" ) ) 
										return (QTextEdit*)w;

									return NULL;
								}

		bool 					_editing;

		brqtWidgetPalette 			_palette;
		brqtFindDialog				_findDialog;
		
		brqtEngine*				_engine;
		brqtLogWindow*				_logWindow;
		
		QList< brqtEditorWindow* >		_documents;

		QPoint					_documentLocation;

		Ui::brqtMainWindow			_ui;
};

#endif // _BRQTMAINWINDOW_H
