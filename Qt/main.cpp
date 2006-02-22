#include <qapplication.h>
#include "ui_brqtMainWindow.h"

int main( int argc, char ** argv ) {
    QApplication a( argc, argv );
    brqtMainWindow w( NULL );

	w.move( QPoint(2, 24) );
    w.show();
    a.connect( &a, SIGNAL( lastWindowClosed() ), &a, SLOT( quit() ) );
    return a.exec();
}
