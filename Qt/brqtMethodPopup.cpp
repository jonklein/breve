#include "brqtMethodPopup.h"

extern char *slObjectParseText;
extern int slObjectParseLine;

int brqtQuickparserlex();
void slObjectParseSetBuffer(char *b);

void brqtMethodPopup::popup() {
    char *text;
    clear();
    
	_lineMap.clear();
    text = strdup(_textArea->text().ascii());
    
	slObjectParseSetBuffer(text);

	int t;
    
	while((t = brqtQuickparserlex())) {
		if(t != 1) {
			QString item;

			item.sprintf("%s (line %d)", slObjectParseText, slObjectParseLine);
			item.simplifyWhiteSpace();

			insertItem(item);

			_lineMap.push_back(slObjectParseLine);
		}
	}

	if( count() == 0) insertItem("Go to method...");
    
	free(text);

    QComboBox::popup();
}

void brqtMethodPopup::go(int index) {
	clear();
	insertItem("Go to method...");

	if(index >= _lineMap.size()) return;
	_textArea->setSelection( _lineMap[index] - 1, 0, _lineMap[index], 0);
}
