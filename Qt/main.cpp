#include <qapplication.h>
#include "brqtMainWindow.h"

int main( int argc, char ** argv )
{
    QApplication a( argc, argv );
    brqtMainWindow w;
    w.show();
    a.connect( &a, SIGNAL( lastWindowClosed() ), &a, SLOT( quit() ) );
    return a.exec();
}
