#include <QApplication>
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
		void						closeDocument() { printf( "close document\n" ); }

	private:
		Ui::brqtEditorWindow 		_ui;
};
