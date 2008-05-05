#include "brqtLogWindow.h"

#include <QScrollBar>

brqtLogWindow *brqtLogWindow::_logWindow = NULL;

brqtLogWindow::brqtLogWindow( QWidget *inParent ) : QDialog( inParent ) {
	_ui.setupUi( this );

	// multiple log windows not allowed

	if( _logWindow )
		throw std::exception();

	_logWindow = this;

 	QFont font( "Courier", 10 );

	font.setStyleHint( QFont::TypeWriter );
	_ui.textEdit -> setCurrentFont( font );
	_ui.textEdit -> setTabStopWidth( 30 );

	_stdoutColor.setRgb( 0xff, 0xff, 0xff, 0xff );
	_stderrColor.setRgb( 0xff, 0x44, 0x44, 0xff );
}

brqtLogWindow::~brqtLogWindow() {
	_logWindow = NULL;
}


void brqtLogWindow::log( const char *inText, bool inErr ) {
	QTextCursor cursor = _logWindow -> _ui.textEdit -> textCursor();
	QScrollBar *scrollbar = _logWindow -> _ui.textEdit -> verticalScrollBar();

	bool atEnd = ( scrollbar -> value() == scrollbar -> maximum() );

	cursor.movePosition( QTextCursor::End );

	_logWindow -> _ui.textEdit -> setTextColor( inErr ? _stderrColor : _stdoutColor );
	cursor.insertText( inText );

	if( atEnd )
		scrollbar -> setValue( scrollbar -> maximum() );
}

void brqtLogWindow::logerr( const char *inText ) {
	if( _logWindow ) 
		_logWindow -> log( inText, true );
}

void brqtLogWindow::logout( const char *inText ) {
	if( _logWindow ) 
		_logWindow -> log( inText, false );
}
