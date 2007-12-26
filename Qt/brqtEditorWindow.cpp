#include "brqtEditorWindow.h"
#include "slutil.h"

#include "brqtSyntaxHighlighter.h"

brqtEditorWindow::brqtEditorWindow( QWidget *parent ) : QDialog( parent ) {
	QFont font( "Courier", 10 );

	font.setStyleHint( QFont::TypeWriter );

	_ui.setupUi( this );
	
	_ui.methodPopup->setTextArea( _ui.textEdit );

	_ui.textEdit->setCurrentFont( font );
	_ui.textEdit->setTabStopWidth( 30 );

	new brqtSyntaxHighlighter( _ui.textEdit );
}

int brqtEditorWindow::loadFile( std::string &inFile ) {
	char *text = slUtilReadFile( inFile.c_str() );

	if( !text )
		return -1;

	_ui.textEdit -> setText( text );
	slFree( text );
	return 0;
}

bool brqtEditorWindow::close() {
	printf( "Editor window closing [%p]\n", parentWidget() );

	return QDialog::close();
}

const QString brqtEditorWindow::getText() {
	return _ui.textEdit -> toPlainText();
}
