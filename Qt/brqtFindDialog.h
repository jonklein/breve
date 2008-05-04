#include <QWidget>
#include <QDialog>
#include <QApplication>
#include <QTextDocument>
#include <QTextEdit>

#include "ui_brqtFindDialog.h"

class brqtFindDialog: public QDialog {
	Q_OBJECT

	public:
					brqtFindDialog( QWidget *parent = NULL ) : QDialog( parent ) {
						_ui.setupUi( this );

						connect( _ui.findNext, SIGNAL( pressed() ), this, SLOT( findNext() ) );
						connect( _ui.findPrev, SIGNAL( pressed() ), this, SLOT( findPrev() ) );
					}
				
		virtual			~brqtFindDialog() {} 

	public slots:
		void			findNext() { find( 0 ); }
		void			findPrev() { find( QTextDocument::FindBackward ); }

	private:
		void			find( QTextDocument::FindFlags inOptions ) {
						QTextEdit *editor = focusedTextEdit();

						if( editor ) {
							if( _ui.ignoreCase -> checkState() ) 
								inOptions |= QTextDocument::FindCaseSensitively;

							QString f = _ui.findText -> text();
							editor -> find( f, inOptions );
						} 
					}

		QTextEdit* 		focusedTextEdit() {
						QWidget *w = QApplication::focusWidget();

						if( w && w->inherits( "QTextEdit" ) )
							return (QTextEdit*)w;

						return NULL;
					}


		Ui::brqtFindDialog	_ui;
};

