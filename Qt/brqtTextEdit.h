#include <QTextEdit>

class brqtTextEdit : public QTextEdit {
	public:
		brqtTextEdit( QWidget *inParent ) : QTextEdit( inParent ) {}

	public slots:
		void on_textEdit_cursorPositionChanged() {	
			printf( "Changed\n" );
		}
};
