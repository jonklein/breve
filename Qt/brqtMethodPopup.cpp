#include "brqtMethodPopup.h"

extern char *slObjectParseText;
extern int slObjectParseLine;

int brqtQuickparserlex();
void slObjectParseSetBuffer( char *b );

void brqtMethodPopup::showPopup() {
	char *simulationText;
	clear();

	_updating = true;
	
	_lineMap.clear();
	simulationText = strdup( _textArea->toPlainText().toAscii() );
	
	slObjectParseSetBuffer( simulationText );

	int t;
	
	while( ( t = brqtQuickparserlex() ) ) {
		if( t != 1 ) {
			QString item;

			item.sprintf( "%s (line %d)", slObjectParseText, slObjectParseLine );

			insertItem( count(), item.simplified() );

			_lineMap.push_back( slObjectParseLine );
		}
	}

	if( count() == 0 ) 
		insertItem( count(), "Go to method..." );
	
	free( simulationText );


	QComboBox::showPopup();

	_updating = false;
}

void brqtMethodPopup::go( int index ) {
	if( _updating )
		return;

	// clear();
	// insertItem( count(), "Go to method..." );

	if ( index >= _lineMap.size() ) 
		return;


	QTextCursor cursor = _textArea -> textCursor();

	QString str = _textArea -> toPlainText();

	// Note: the line numbers are 1 based, but our counting is 0 based

	int pos = 0;
	int line = _lineMap[ index ] - 1;

	while( line-- )
		pos = str.indexOf( "\n", pos ) + 1;

	cursor.setPosition( pos, QTextCursor::MoveAnchor );
	_textArea -> setTextCursor( cursor );

	pos = str.indexOf( "\n", pos ) + 1;
	cursor.setPosition( pos, QTextCursor::KeepAnchor );
	_textArea -> setTextCursor( cursor );
}
