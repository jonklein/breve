#include <qsyntaxhighlighter.h>
#include <vector>

struct brqtSyntaxFeature {
	brqtSyntaxFeature( const char *inRegex, const QColor& inColor ) {
		_regex.setPattern( inRegex );	
		_color = inColor;
	}

	QColor		_color;
	QRegExp		_regex;
};

class brqtSyntaxHighlighter : public QSyntaxHighlighter {
	public:
		brqtSyntaxHighlighter( QTextEdit *textEdit) : QSyntaxHighlighter( textEdit ) {
			QColor stringColor, numberColor, typeColor, commentColor;

			typeColor.setRgb( 255, 0, 0 );
			commentColor.setRgb( 130, 130, 130 );
			numberColor.setRgb( 0, 0, 255 );
			stringColor.setRgb( 0, 255, 0 );
	
			_features.push_back( brqtSyntaxFeature( "[^\\w]int[^\\w]"   , typeColor ) );
			_features.push_back( brqtSyntaxFeature( "[^\\w]string[^\\w]", typeColor ) );
			_features.push_back( brqtSyntaxFeature( "[^\\w]matrix[^\\w]", typeColor ) );
			_features.push_back( brqtSyntaxFeature( "[^\\w]vector[^\\w]", typeColor ) );
			_features.push_back( brqtSyntaxFeature( "[^\\w]double[^\\w]", typeColor ) );
			_features.push_back( brqtSyntaxFeature( "[^\\w]float[^\\w]" , typeColor ) );
			_features.push_back( brqtSyntaxFeature( "[^\\w]hash[^\\w]"  , typeColor ) );
			_features.push_back( brqtSyntaxFeature( "[^\\w]list[^\\w]"  , typeColor ) );
			_features.push_back( brqtSyntaxFeature( "[^\\w]object[^\\w]", typeColor ) );

			_features.push_back( brqtSyntaxFeature( "\"[^\"]*\"", stringColor ) );
			_features.push_back( brqtSyntaxFeature( "\'[^\']*\'", stringColor ) );
			_features.push_back( brqtSyntaxFeature( "'''.*'''",   stringColor ) );

			_features.push_back( brqtSyntaxFeature( "[0-9]*\\.?[0-9]*", numberColor ) );

			_features.push_back( brqtSyntaxFeature( "#.*", commentColor ) );
		};
	
		void highlightBlock(const QString &text ) {
	
			for( int n = 0; n < _features.size(); n++ ) { 
				int pos = 0;
		
				while( ( pos = text.indexOf( _features[ n ]._regex, pos) ) != -1) 
					setFormat( pos++, _features[ n ]._regex.matchedLength(), _features[ n ]._color );
			}	
		}
	
	public:
		QList< brqtSyntaxFeature > _features;
};
