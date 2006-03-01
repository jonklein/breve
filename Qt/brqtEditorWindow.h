#include <QApplication>
#include <QDialog>
#include "ui_brqtEditorWindow.h"

class brqtEditorWindow : public QDialog {
	Q_OBJECT

	public:
		brqtEditorWindow( QWidget *parent = NULL ) ;

	private:
		Ui::brqtEditorWindow ui;
};
