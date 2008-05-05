#include <QApplication>
#include <QList>
#include <QDialog>
#include <QFileSystemWatcher>
#include <QFileInfo>
#include <QMessageBox>


#include "ui_brqtEditorWindow.h"

#include <string>

class brqtEditorWindow : public QDialog {
	Q_OBJECT

	public:
						brqtEditorWindow( QWidget *parent );

		bool				loadFile( const QString &inPath );

		QString				getText() const;

		bool				save();
		void				closeEvent( QCloseEvent* inEvent );

		void				timerEvent( QTimerEvent* );

	public slots:

		void 				fileChanged();

		void 				setModified() {
						     setWindowModified( _ui.textEdit -> document() -> isModified() );
						}

		void 				on_textEdit_cursorPositionChanged() {
							int line = _ui.textEdit -> textCursor().blockNumber() + 1;
							QString str;

							str.setNum( line );

							_ui.lineBox -> setText( str );
						}

		void 				on_lineBox_lostFocus() {
							goToLine( _ui.lineBox -> text().toInt() );
						}

		void				addChildParameter();
		void				removeParameter();
		void				loadParameters();

	private:
		void 				setDocumentPath( const QString &inPath );

		void 				goToLine( int inLine ) {

			QTextCursor cursor = _ui.textEdit -> textCursor();
			QString str = _ui.textEdit -> toPlainText();

			// Note: the line numbers are 1 based, but our counting is 0 based

			int pos = 0;
			inLine--;

		        while( inLine-- && pos >= 0 )
				pos = str.indexOf( "\n", pos ) + 1;

			cursor.setPosition( pos, QTextCursor::MoveAnchor );
			_ui.textEdit -> setTextCursor( cursor );

			pos = str.indexOf( "\n", pos ) + 1;
			cursor.setPosition( pos, QTextCursor::KeepAnchor );
			_ui.textEdit -> setTextCursor( cursor );
		}

		Ui::brqtEditorWindow 		_ui;

		bool				_hasPath;
		bool				_lostFileWatch;

		QFileSystemWatcher		_fileWatcher;

		QFileInfo			_fileInfo;
};
