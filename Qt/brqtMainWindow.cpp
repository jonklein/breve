#include "brqtMainWindow.h"
#include "brqtEditorWindow.h"
#include "brqtMoveableWidgets.h"

#include <QVariant>

#import <sys/types.h>
#import <sys/dir.h>

brqtMainWindow::brqtMainWindow() : QMainWindow( NULL, Qt::WindowTitleHint ) {
	_engine = NULL;

	_editing = 0;
	_ui.setupUi( this );

	setAttribute( Qt::WA_MacBrushedMetal );

	setStatusBar( NULL );

	char *argv[] = { "none" };
	int argc = 1;
	glutInit( &argc, argv );

	_palette.hide();

	// connect( _ui.editButton, SIGNAL( pressed() ), this, SLOT( toggleEditing() ) );
	connect( _ui.actionOpen, SIGNAL( triggered() ), this, SLOT( openDocument() ) );
	connect( _ui.actionSave, SIGNAL( triggered() ), this, SLOT( saveDocument() ) );
	connect( _ui.actionNew, SIGNAL( triggered() ), this, SLOT( newDocument() ) );

	connect( _ui.actionCut, SIGNAL( triggered() ), this, SLOT( cut() ) );
	connect( _ui.actionCopy, SIGNAL( triggered() ), this, SLOT( copy() ) );
	connect( _ui.actionPaste, SIGNAL( triggered() ), this, SLOT( paste() ) );
	connect( _ui.actionUndo, SIGNAL( triggered() ), this, SLOT( undo() ) );
	connect( _ui.actionRedo, SIGNAL( triggered() ), this, SLOT( redo() ) );
	connect( _ui.actionSelectAll, SIGNAL( triggered() ), this, SLOT( selectAll() ) );
	connect( _ui.actionClose, SIGNAL( triggered() ), this, SLOT( closeWindow() ) );

	connect( _ui.actionFind, SIGNAL( triggered() ), this, SLOT( find() ) );
	connect( _ui.actionLog_Window, SIGNAL( triggered() ), this, SLOT( activateLogWindow() ) );

	connect( _ui.toggleSimulationButton, SIGNAL( pressed() ), this, SLOT( toggleSimulation() ) );
	connect( _ui.stopSimulationButton, SIGNAL( pressed() ), this, SLOT( stopSimulation() ) );

	buildMenus();

	setAcceptDrops( true );

	QPoint position( 1, 1 );
	move( position );


	QRect bounds = rect();

	_documentLocation.setX( bounds.right() + 2 );
	_documentLocation.setY( bounds.top() );

	_logWindow = new brqtLogWindow( this );
	_logWindow -> move( 0, bounds.bottom() + 2 );
	_logWindow -> show();

	slSetMessageCallbackFunctions( brqtLogWindow::logout, brqtLogWindow::logerr );

	newDocument();
}

void brqtMainWindow::buildMenus() {
	QString demoPath, docsPath;

	#ifdef Q_WS_MAC

	demoPath = QApplication::applicationDirPath() + "/../Resources/demos";
	docsPath = QApplication::applicationDirPath() + "/../Resources/docs";

	#else 

	demoPath = QApplication::applicationDirPath() + "/demos";
	docsPath = QApplication::applicationDirPath() + "/docs";

	#endif


	char *docsRootEnv = getenv( "BREVE_DOCS_PATH" );
	char *demoRootEnv = getenv( "BREVE_DEMO_PATH" );

	if( docsRootEnv && strlen( docsRootEnv ) != 0 )
		docsPath = docsRootEnv;

	if( demoRootEnv && strlen( demoRootEnv ) != 0 )
		demoPath = demoRootEnv;



	QStringList demoFilters;
	demoFilters.append( "*.tz" );
	demoFilters.append( "*.py" );
	buildMenuFromDirectory( demoPath, _ui.menuDemos, &demoFilters, SLOT( openDemo(QAction*) ) );

	QStringList docFilters( "*.html" );
	buildMenuFromDirectory( docsPath + "/steveclasses",  _ui.menuSteveClasses,  &docFilters,  SLOT( openHTML(QAction*) ) );
	buildMenuFromDirectory( docsPath + "/pythonclasses", _ui.menuPythonClasses, &docFilters,  SLOT( openHTML(QAction*) ) );


	if( _ui.menuSteveClasses -> isEmpty() || _ui.menuPythonClasses -> isEmpty() ) {
		QMessageBox::warning( this, tr( "breve" ), tr( "Could not locate documentation files.  The breve docs directory should be in the same directory as the breve executable, or may be specified using the BREVE_DOCS_PATH environment variable." ), QMessageBox::Ok );
	}

	if( _ui.menuDemos -> isEmpty() ) {
		QMessageBox::warning( this, tr( "breve" ), tr( "Could not locate demo files.  The breve demos directory should be in the same directory as the breve executable, or may be specified using the BREVE_DEMO_PATH environment variable." ), QMessageBox::Ok );
	}


}


void brqtMainWindow::dragEnterEvent( QDragEnterEvent *event ) {
	if ( event->mimeData()->hasFormat("text/plain") ) 
		event->acceptProposedAction();
}


void brqtMainWindow::dropEvent( QDropEvent *event ) {
	QWidget *widget = NULL;
	QString string = event->mimeData()->text();

	// printf(" Got drop: %s\n", string.toAscii().constData() );

	if( !string.compare( "button" ) ) {
		widget = brqtMoveablePushButton( this );
	} else if( !string.compare( "glwidget" ) ) {
		widget = brqtMoveableGLWidget( this );
	} else if( !string.compare( "radiobutton" ) ) {
		widget = brqtMoveableRadioButtonGroup( this );
	} else if( !string.compare( "vslider" ) ) {
		widget = brqtMoveableVerticalSlider( this );
	} else if( !string.compare( "hslider" ) ) {
		widget = brqtMoveableHorizontalSlider( this );
	} else if( !string.compare( "checkbox" ) ) {
		widget = brqtMoveableCheckBox( this );
	} else if( !string.compare( "label" ) ) {
		widget = brqtMoveableLabel( this );
	} else if( !string.compare( "lineedit" ) ) {
		widget = brqtMoveableLineEdit( this );
	}

	if( !widget ) 
		return;

	widget->move( event->pos().x(), event->pos().y() );

	widget->show();

   	event->acceptProposedAction();
 }

void brqtMainWindow::setButtonMode() {
	_ui.glWidget -> setButtonMode( 1 );
}

void brqtMainWindow::toggleEditing() {
	_editing = !_editing;

	if( _editing ) {
		// _ui.editButton->setText( tr( "Finished Editing" ) );
		_palette.show();
	} else {
		// _ui.editButton->setText( tr( "Edit Interface" ) );
		_palette.hide();
	}

	repaint();
}

void brqtMainWindow::saveDocument() { 
	brqtEditorWindow *w = (brqtEditorWindow*)QApplication::activeWindow();

	if( _documents.contains( w ) ) 
		w -> save();
}

void brqtMainWindow::openDocument() { 
	QString s = QFileDialog::getOpenFileName( this, tr( "breve simulation files (*.tz *.py *.brevepy *.l *.brevel)" ) );

	if( s != "" )
		openDocument( s );
}

#define DOCUMENT_OFFSET 20

void brqtMainWindow::newDocument() { 
	brqtEditorWindow *editor = new brqtEditorWindow( this );

	_documents.push_back( editor );

	int x = _documentLocation.x() + ( _documents.size() - 1 ) * DOCUMENT_OFFSET;
	int y = _documentLocation.y() + ( _documents.size() - 1 ) * DOCUMENT_OFFSET;
	editor -> move( x, y );
	editor -> show();
	
	updateSimulationPopup();
}

void brqtMainWindow::activateLogWindow() {
	_logWindow -> raise();
	_logWindow -> show();
}

void brqtMainWindow::closeDocument( brqtEditorWindow *inDocument ) {
	int index = _documents.indexOf( inDocument );

	if( index >= 0 )
		_documents.removeAt( index );

	updateSimulationPopup();
}

brqtEditorWindow *brqtMainWindow::openDocument( QString &inDocument ) {
	brqtEditorWindow *editor = new brqtEditorWindow( this );

	_documents.push_back( editor );

	editor -> loadFile( inDocument );

	int x = _documentLocation.x() + ( _documents.size() - 1 ) * DOCUMENT_OFFSET;
	int y = _documentLocation.y() + ( _documents.size() - 1 ) * DOCUMENT_OFFSET;
	editor -> move( x, y );
	editor -> show();

	updateSimulationPopup();

	return editor;
}

void brqtMainWindow::stopSimulation() { 
	if( _engine ) {
		delete _engine;
		_engine = NULL;
	}
}

void brqtMainWindow::toggleSimulation() { 
	if( !_engine ) {
		int index = _ui.simulationPopup -> currentIndex();

		brqtEditorWindow *window = _documents[ index ];

		const QString simtext = window -> getText();
		char *str = slStrdup( simtext.toAscii().constData() );

		const QString paramtext = window -> getXMLParameters ();
		char *params = slStrdup( paramtext.toAscii().constData() );

		printf( "%s\n", params );

		_engine = new brqtEngine( str, window -> windowTitle().toAscii().constData(), _ui.glWidget );

		slFree( str );

		if( _engine -> error() ) 
			stopSimulation();
	} else {
		_engine -> togglePaused();
	}
}

QMenu *brqtMainWindow::buildMenuFromDirectory( const QString& inDirectory, QMenu *inParent, QStringList *inFilters, const char *inSlot ) {
	QDir dir( inDirectory );
	QFileInfoList files = dir.entryInfoList( *inFilters, QDir::Files | QDir::AllDirs, QDir::Name );

	for ( int i = 0; i < files.size(); i++ ) {
		const QFileInfo &file = files.at( i );

		if( !file.isHidden() ) {
			std::string fullpath = file.absoluteFilePath().toStdString();
	
			if( file.isDir() ) {
				QMenu *sub = new QMenu( file.fileName().toAscii(), inParent );
				inParent -> addMenu( sub );
	
				buildMenuFromDirectory( fullpath.c_str(), sub, inFilters, inSlot );
			} else {
				QAction *action = inParent -> addAction( file.fileName().toAscii() );

				action -> setData( QVariant( fullpath.c_str() ) );
			}
		}
	}

	connect( inParent, SIGNAL( triggered(QAction*) ), this, inSlot );

	return inParent;
}

void brqtMainWindow::updateSimulationPopup() {
	_ui.simulationPopup -> clear();

	for( int n = 0; n < _documents.size(); n++ ) 
		_ui.simulationPopup -> addItem( _documents[ n ] -> windowTitle(), QVariant( _documents[ n ] ) );

	// _ui.simulationPopup -> activated( _documents.size() - 1 );
}
