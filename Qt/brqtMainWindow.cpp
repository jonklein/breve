#include "brqtMainWindow.h"
#include "brqtEditorWindow.h"
#include "brqtMoveableWidgets.h"

#import <sys/types.h>
#import <sys/dir.h>

brqtMainWindow::brqtMainWindow() {
	_engine = NULL;

	_editing = 0;
	_ui.setupUi( this );

	_palette.hide();

	// connect( _ui.editButton, SIGNAL( pressed() ), this, SLOT( toggleEditing() ) );
	connect( _ui.actionOpen, SIGNAL( triggered() ), this, SLOT( openDocument() ) );
	connect( _ui.actionNew, SIGNAL( triggered() ), this, SLOT( newDocument() ) );

	connect( _ui.actionCut, SIGNAL( triggered() ), this, SLOT( cut() ) );
	connect( _ui.actionCopy, SIGNAL( triggered() ), this, SLOT( copy() ) );
	connect( _ui.actionPaste, SIGNAL( triggered() ), this, SLOT( paste() ) );
	connect( _ui.actionUndo, SIGNAL( triggered() ), this, SLOT( undo() ) );
	connect( _ui.actionRedo, SIGNAL( triggered() ), this, SLOT( redo() ) );
	connect( _ui.actionClose, SIGNAL( triggered() ), this, SLOT( close() ) );

	connect( _ui.actionFind, SIGNAL( triggered() ), this, SLOT( find() ) );

	connect( _ui.toggleSimulationButton, SIGNAL( pressed() ), this, SLOT( toggleSimulation() ) );

	setAcceptDrops( true );

	QPoint position( 0, 0 );

	move( position );

	QStringList demoFilters;
	demoFilters.append( "*.tz" );
	demoFilters.append( "*.py" );
	buildMenuFromDirectory( "../demos",             _ui.menuDemos, &demoFilters, SLOT( openDemo(QAction*) ) );

	QStringList docFilters( "*.html" );
	buildMenuFromDirectory( "../docs/steveclasses",  _ui.menuSteveClasses,  &docFilters,  SLOT( openHTML(QAction*) ) );
	buildMenuFromDirectory( "../docs/pythonclasses", _ui.menuPythonClasses, &docFilters,  SLOT( openHTML(QAction*) ) );


	newDocument();
}

void brqtMainWindow::dragEnterEvent( QDragEnterEvent *event ) {
	if ( event->mimeData()->hasFormat("text/plain") ) 
		event->acceptProposedAction();
}


void brqtMainWindow::dropEvent( QDropEvent *event ) {
	QWidget *widget = NULL;
	QString string = event->mimeData()->text();

	printf(" Got drop: %s\n", string.toAscii().constData() );

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

	// widget ...
	
	widget->move( event->pos().x(), event->pos().y() );

	widget->show();

   	event->acceptProposedAction();
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

void brqtMainWindow::openDocument() { 
	QString s = QFileDialog::getOpenFileName( this );

	openDocument( s );
}

void brqtMainWindow::newDocument() { 
	brqtEditorWindow *editor = new brqtEditorWindow( this );
	editor -> show();

	_documents.push_back( editor );
}

void brqtMainWindow::closeDocument() {

}



int brqtMainWindow::openDocument( QString &inDocument ) {
	std::string file = inDocument.toStdString();

	brqtEditorWindow *editor = new brqtEditorWindow( this );

	editor -> show();
	editor -> loadFile( file );

	_documents.push_back( editor );

	return 0;
}


void brqtMainWindow::toggleSimulation() { 
	if( !_engine ) {
		brqtEditorWindow *window = _documents.back();

		const QString qstr = window -> getText();
		char *str = slStrdup( qstr.toAscii().constData() );

		_engine = new brqtEngine( str, "untitled.tz", _ui.glWidget );

		slFree( str );
	} else {
		delete _engine;
	}
}

QMenu *brqtMainWindow::buildMenuFromDirectory( const char *inDirectory, QMenu *inParent, QStringList *inFilters, const char *inSlot ) {
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
