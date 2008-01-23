#include <QApplication>
#include <QList>
#include <QDialog>
#include "ui_brqtEditorWindow.h"

#include <string>

class brqtEditorWindow : public QDialog {
	Q_OBJECT

	public:
									brqtEditorWindow( QWidget *parent = NULL ) ;

		int							loadFile( std::string &inPath );

		bool						close();

		const QString				getText();

	public slots:
   public slots:
        void on_textEdit_cursorPositionChanged() {
            int line = _ui.textEdit -> textCursor().blockNumber();
			QString str;

			str.setNum( line );

			_ui.lineBox -> setText( str );
        }

		void on_lineBox_lostFocus() {
			int line = _ui.lineBox -> text().toInt();

			_ui.textEdit -> textCursor();
		}

		void						closeDocument() { printf( "close document\n" ); }
		void						addChildParameter();
		void						removeParameter();
		void						loadParameters();

	private:
		Ui::brqtEditorWindow 		_ui;
};
