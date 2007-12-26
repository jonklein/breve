#include <qsyntaxhighlighter.h>
#include <vector>

class brqtSyntaxHighlighter : public QSyntaxHighlighter {
	public:
		brqtSyntaxHighlighter( QTextEdit *textEdit) : QSyntaxHighlighter( textEdit ) {
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
	
		void highlightBlock(const QString &text ) {
	
			for(unsigned int n=0;n<_types.size();n++) { 
				int pos = 0;
		
				while( ( pos = text.indexOf( _types[n], pos) ) != -1) 
					setFormat(pos++, strlen( _types[n] ), _typeColor );	
			}	
		}
	
	public:
		QColor _stringColor, _numberColor, _typeColor, _commentColor;
		std::vector< char* > _types;
};
