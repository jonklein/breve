#include <QApplication>
#include <QList>
#include <QDialog>
#include "ui_brqtEditorWindow.h"

#include <string>

class brqtEditorWindow : public QDialog {
	Q_OBJECT

	public:
						brqtEditorWindow( QWidget *parent ) ;

		int				loadFile( std::string &inPath );

		bool				close();

		const QString			getText();

		void 				setDocumentTitle( const QString &inName );

	public slots:
		void on_textEdit_cursorPositionChanged() {
			int line = _ui.textEdit -> textCursor().blockNumber() + 1;
			QString str;

			str.setNum( line );

			_ui.lineBox -> setText( str );
		}

		void on_lineBox_lostFocus() {
			int line = _ui.lineBox -> text().toInt();

			goToLine( line );
		}

		void				addChildParameter();
		void				removeParameter();
		void				loadParameters();

	private:

		void 				goToLine( int inLine ) {

			QTextCursor cursor = _ui.textEdit -> textCursor();
			QString str = _ui.textEdit -> toPlainText();

			// Note: the line numbers are 1 based, but our counting is 0 based

			int pos = 0;
			inLine--;

		        while( inLine-- )
				pos = str.indexOf( "\n", pos ) + 1;

			cursor.setPosition( pos, QTextCursor::MoveAnchor );
			_ui.textEdit -> setTextCursor( cursor );

			pos = str.indexOf( "\n", pos ) + 1;
			cursor.setPosition( pos, QTextCursor::KeepAnchor );
			_ui.textEdit -> setTextCursor( cursor );
		}

		void				closeDocument() { printf( "close document\n" ); }
		Ui::brqtEditorWindow 		_ui;
};
