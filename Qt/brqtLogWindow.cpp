#include "brqtLogWindow.h"

brqtLogWindow::brqtLogWindow( QWidget *inParent ) : QDialog( inParent ) {
	_ui.setupUi( this );

	QFont font( "Courier", 10 );

	font.setStyleHint( QFont::TypeWriter );
	_ui.textEdit -> setCurrentFont( font );
	_ui.textEdit -> setTabStopWidth( 30 );
}
