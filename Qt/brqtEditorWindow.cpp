#include "brqtEditorWindow.h"
#include "brqtMainWindow.h"
#include "slutil.h"

#include "brqtSyntaxHighlighter.h"

brqtEditorWindow::brqtEditorWindow( QWidget *parent ) : QDialog( parent ) {
	_ui.setupUi( this );
	
	resize( 700, 500 );

	_ui.methodPopup -> setTextArea( _ui.textEdit );
	setWindowTitle( "Untitled" );

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

	_ui.parameters -> expandItem( _ui.parameters -> topLevelItem( 0 ) );
	_ui.parameters -> resizeColumnToContents( 0 );

	new brqtSyntaxHighlighter( _ui.textEdit );
}

void brqtEditorWindow::setDocumentTitle( const QString &inName ) {

}

int brqtEditorWindow::loadFile( std::string &inFile ) {
	char *text = slUtilReadFile( inFile.c_str() );

	if( !text )
		return -1;

	setWindowTitle( inFile.c_str() );

	_ui.textEdit -> setText( text );
	slFree( text );
	return 0;
}

bool brqtEditorWindow::close() {
	( (brqtMainWindow*)parentWidget() ) -> closeDocument( this );

	return QDialog::close();
}

const QString brqtEditorWindow::getText() {
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

}
