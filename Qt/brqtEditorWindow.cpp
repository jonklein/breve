#include "brqtEditorWindow.h"

brqtEditorWindow::brqtEditorWindow( QWidget *parent ) : QDialog( parent ) {
	QFont font( "Courier", 10 );

	font.setStyleHint( QFont::TypeWriter );

	ui.setupUi( this );
	
	ui.methodPopup->setTextArea( ui.textEdit );

	ui.textEdit->setCurrentFont( font );
	ui.textEdit->setTabStopWidth( 30 );
}
