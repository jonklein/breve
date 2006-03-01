#include <QApplication>
#include <QMainWindow>
#include <QSlider>
#include <QComboBox>

#include "brqtMainWindow.h"
#include "brqtEditorWindow.h"
#include "brqtMoveableWidgets.h"

int main( int argc, char ** argv ) {
    QApplication a( argc, argv );

	brqtMainWindow *window = new brqtMainWindow;

	window->show();

    a.connect( &a, SIGNAL( lastWindowClosed() ), &a, SLOT( quit() ) );

    return a.exec();
}
