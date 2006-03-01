#include <QWidget>

#include "ui_brqtWidgetPalette.h"

class brqtWidgetPalette : public QDialog {
	public:
		brqtWidgetPalette() {
			ui.setupUi( this );
		}

	private:
		Ui::brqtWidgetPalette ui;
};
