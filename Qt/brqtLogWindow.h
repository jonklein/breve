#include <QApplication>
#include <QList>
#include <QDialog>
#include "ui_brqtLogWindow.h"

#include <string>

class brqtLogWindow : public QDialog {
	Q_OBJECT

	public:
						brqtLogWindow( QWidget *parent = NULL );
						~brqtLogWindow();

		void				log( const char *inText, bool inErr );

		static void			logout( const char *inText );
		static void			logerr( const char *inText );

		static brqtLogWindow*		_logWindow;

		Ui::brqtLogWindow 		_ui;

		QColor				_stdoutColor;
		QColor				_stderrColor;
};
