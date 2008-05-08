#include "brqtEditorWindow.h"
#include "brqtMainWindow.h"
#include "slutil.h"
#include <QWaitCondition>

#include "brqtSyntaxHighlighter.h"

brqtEditorWindow::brqtEditorWindow( QWidget *parent ) : QDialog( parent ) {
	_ui.setupUi( this );
	
	resize( 700, 500 );

	_ui.methodPopup -> setTextArea( _ui.textEdit );
	setWindowTitle( "Untitled[*]" );

	QFont font( "Courier", 10 );

	font.setStyleHint( QFont::TypeWriter );
	_ui.textEdit -> setCurrentFont( font );
	_ui.textEdit -> setTabStopWidth( 30 );

	QList< int > ratio;
	ratio.push_back( 800 );
	ratio.push_back( 200 );

	_ui.splitter->setSizes( ratio );

	connect( _ui.addChildParameter, SIGNAL( pressed() ), this, SLOT( addChildParameter() ) );
	connect( _ui.removeParameter,   SIGNAL( pressed() ), this, SLOT( removeParameter() ) );
	connect( _ui.loadParameters,    SIGNAL( pressed() ), this, SLOT( loadParameters() ) );

	connect( &_fileWatcher,    SIGNAL( fileChanged( const QString& ) ), this, SLOT( fileChanged() ) );

	connect( _ui.textEdit -> document(), SIGNAL( contentsChanged() ), this, SLOT( setModified() ) );

	_ui.parameters -> expandItem( _ui.parameters -> topLevelItem( 0 ) );
	_ui.parameters -> resizeColumnToContents( 0 );

	new brqtSyntaxHighlighter( _ui.textEdit );

	_hasPath = false;

	_lostFileWatch = false;

	startTimer( 1000 );
}

void brqtEditorWindow::fileChanged() {
	if( _fileInfo.exists() ) {
		if( isWindowModified() ) {
			QMessageBox box( QMessageBox::Question, "breve", tr( "This document has changed externally!  Do you want to reload the document from disk and discard your changes?" ), 0, this, Qt::Sheet );

			QPushButton* keep   = box.addButton( tr( "Keep Changes" ), QMessageBox::RejectRole );
			QPushButton* reload = box.addButton( tr( "Reload From Disk" ), QMessageBox::AcceptRole );

			box.setEscapeButton( keep );
			box.setDefaultButton( reload );

			box.exec();

			if( box.clickedButton() == keep ) 
				return;
		}

		if( !loadFile( _fileInfo.filePath() ) ) 
			_lostFileWatch = true;
	} else {
		// Some editors (vi, for example) preform a write by deleting the old
		// file and replacing it with a new one.  Unfortunately, QFileSystemWatcher
		// gets the delete and then cannot keep watching for the new file.  We
		// will keep a flag set and readd the file in a timer event.

		_lostFileWatch = true;
	}
}

void brqtEditorWindow::timerEvent( QTimerEvent* ) {
	if( _lostFileWatch && _fileInfo.exists() ) {
		_fileWatcher.addPath( _fileInfo.filePath() );
		_lostFileWatch = false;
		fileChanged();
	}
}

void brqtEditorWindow::setDocumentPath( const QString &inPath ) {
	if( _hasPath ) {
		QString path = _fileInfo.filePath();
		_fileWatcher.removePath( path );
	}

	_fileInfo.setFile( inPath );

	QString name = _fileInfo.fileName();
	QString path = _fileInfo.filePath();

	QString title = QString( "%1[*] - %2" ).arg( name, path );

	setWindowTitle( title );

	_fileWatcher.addPath( inPath );
	_hasPath = true;
}

bool brqtEditorWindow::loadFile( const QString &inFile ) {
	char *text = slUtilReadFile( inFile.toAscii() );

	if( !text )
		return false;

	setDocumentPath( inFile );

	_ui.textEdit -> setText( text );
	slFree( text );

	setWindowModified( false );

	return true;
}

bool brqtEditorWindow::save() {
	if( !_hasPath ) {
                QString s = QFileDialog::getSaveFileName( this );

		if( s == "" ) 
			return false;

		setDocumentPath( s );
	}

	_fileWatcher.removePath( _fileInfo.filePath() );

	QFile file( _fileInfo.filePath() );
	bool success = false;

	if( file.open( QIODevice::Truncate | QIODevice::WriteOnly | QIODevice::Text ) ) {

		const char *str = _ui.textEdit -> toPlainText().toAscii();

		int written = file.write( str, strlen( str ) );

		if( written > 0 ) {
			_ui.textEdit -> document() -> setModified( false );
			setWindowModified( false );

			success = true;
		} 

	}

	if (!success ) {
		QMessageBox box( QMessageBox::Warning, "breve", QString( tr( "Error Writing File: $1" ) ).arg( file.errorString() ), 0, this, Qt::Sheet );
		box.setDefaultButton( box.addButton( tr( "OK" ), QMessageBox::AcceptRole ) );
		box.exec();

		return false;
	}

	file.close();
	_fileWatcher.addPath( _fileInfo.filePath() );

	return true;
}

void brqtEditorWindow::closeEvent( QCloseEvent* inEvent ) {
	if( isWindowModified() ) {

		QMessageBox box( QMessageBox::Question, "breve", tr( "Do you want to save the changes you made in this document?" ), 0, this, Qt::Sheet );

		QPushButton *bsave   = box.addButton( tr( "Save" ), QMessageBox::AcceptRole );
		QPushButton *bcancel = box.addButton( tr( "Cancel" ), QMessageBox::RejectRole );
		box.addButton( tr( "&Don't Save" ), QMessageBox::NoRole );

		box.setEscapeButton( bcancel );
		box.setDefaultButton( bsave );

		int result = box.exec();

		if( box.clickedButton() == bcancel ) {
			inEvent -> ignore();
			return;
		}

		if( box.clickedButton() == bsave && !save() ) {
			inEvent -> ignore();
			return;
		}
	}

	( (brqtMainWindow*)parentWidget() ) -> closeDocument( this );

	inEvent -> accept();
}

QString brqtEditorWindow::getText() const {
	return _ui.textEdit -> toPlainText();
}

void brqtEditorWindow::addChildParameter() {
	QTreeWidgetItem *selection = _ui.parameters -> currentItem();

	if( selection ) {
		QTreeWidgetItem *child = new QTreeWidgetItem( selection );
		selection -> addChild( child );

		child -> setText( 0, "Untitled" );
		child -> setText( 1, "0" );
		child -> setFlags( Qt::ItemIsEnabled | Qt::ItemIsEditable | Qt::ItemIsSelectable );
		_ui.parameters -> expandItem( child );
	}
}

void brqtEditorWindow::removeParameter() {
	QTreeWidgetItem *selection = _ui.parameters -> currentItem();

	delete selection;
}

void brqtEditorWindow::loadParameters() {
	QString s = QFileDialog::getOpenFileName( this, tr( "Open Simulation Parameters" ), tr( "breve parameter files (*.xml *.brxml)" ) );

}
