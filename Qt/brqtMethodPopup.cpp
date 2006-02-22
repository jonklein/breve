#include "brqtMethodPopup.h"

extern char *slObjectParseText;
extern int slObjectParseLine;

int brqtQuickparserlex();
void slObjectParseSetBuffer( char *b );

void brqtMethodPopup::showPopup() {
    char *simulationText;
    clear();
    
	_lineMap.clear();
    simulationText = strdup( _textArea->toPlainText().toAscii() );
    
	slObjectParseSetBuffer( simulationText );

	int t;
    
	while((t = brqtQuickparserlex())) {
		if(t != 1) {
			QString item;

			item.sprintf("%s (line %d)", slObjectParseText, slObjectParseLine);

			insertItem( count(), item.simplified() );

			_lineMap.push_back( slObjectParseLine );
		}
	}

	if( count() == 0) insertItem( count(), "Go to method..." );
    
	free( simulationText );

    QComboBox::showPopup();
}

void brqtMethodPopup::go( unsigned int index ) {
	clear();
	insertItem( count(), "Go to method..." );

	if( index >= _lineMap.size() ) return;
	// _textArea->setSelection( _lineMap[ index ] - 1, 0, _lineMap[ index ], 0 );
}
