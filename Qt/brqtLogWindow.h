#include <QApplication>
#include <QList>
#include <QDialog>
#include "ui_brqtLogWindow.h"

#include <string>

class brqtLogWindow : public QDialog {
	Q_OBJECT

	public:
						brqtLogWindow( QWidget *parent = NULL );

	private:
		Ui::brqtLogWindow 		_ui;
};
