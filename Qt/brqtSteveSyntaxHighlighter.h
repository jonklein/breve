#include <qsyntaxhighlighter.h>
#include <vector>

class brqtSteveSyntaxHighlighter : public QSyntaxHighlighter {
public:
	brqtSteveSyntaxHighlighter( QTextEdit *textEdit) : QSyntaxHighlighter( textEdit) {
	    _typeColor.setRgb(255, 0, 0);
	    _commentColor.setRgb(100, 100, 100);
	    _numberColor.setRgb(0, 0, 255);
	    _stringColor.setRgb(0, 255, 0);

		_types.push_back("int");
		_types.push_back("string");
		_types.push_back("matrix");
		_types.push_back("vector");
		_types.push_back("double");
		_types.push_back("float");
		_types.push_back("hash");
		_types.push_back("list");
		_types.push_back("object");
	};

    int highlightParagraph(const QString &text, int state) {
		state = 0;
	
		for(unsigned int n=0;n<_types.size();n++) { 
			int pos = 0;
	
			while((pos = text.find(_types[n], pos)) != -1) setFormat(pos++, strlen(_types[n]), _typeColor);	
		}	
	
	for(unsigned int n = 0; n < text.length(); n++) {
	    if(text[n] == "#") {
		setFormat(n, text.length() - n, _commentColor);
		return 0;
	    }
	    
	    if((text[n] == '\"' && text[n - 1] != '\\')) {
		n++;
		
		int start = n;
	
		while(n < text.length() && (text[n] != '\"' || text[n - 1] == '\\')) n++;
		
		setFormat(start, n - start, _stringColor);
	    }
	
	    if(text[n].isDigit() || text[n] == '.' && text[n+1].isDigit()) {
		int start = n;
		while(text[n].isDigit() || text[n] == '.') n++;
		
		setFormat(start, n - start, _numberColor);
	    }
	    
	    if(text[n].isLetter()) {
		
	    }
	}
	
	
	return 0;
    }
    
public:
    QColor _stringColor, _numberColor, _typeColor, _commentColor;
	std::vector< char* > _types;
};
