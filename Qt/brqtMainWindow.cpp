#include "brqtMainWindow.h"
#include "brqtMoveableWidgets.h"
#include "brqtEditorWindow.h"

brqtMainWindow::brqtMainWindow() {
	brqtEditorWindow *editor;

	_editing = 0;
	_ui.setupUi( this );

	_palette.hide();

    connect( _ui.editButton, SIGNAL( pressed() ), this, SLOT( toggleEditing() ) );

	editor = new brqtEditorWindow( this );
	editor->show();

	setAcceptDrops( true );
}

void brqtMainWindow::dragEnterEvent( QDragEnterEvent *event ) {
	if (event->mimeData()->hasFormat("text/plain")) event->acceptProposedAction();
}


void brqtMainWindow::dropEvent( QDropEvent *event ) {
	QWidget *widget = NULL;
	QString string = event->mimeData()->text();

	printf(" Got drop: %s\n", string.toAscii().constData() );

	if( !string.compare( "button" ) ) {
		widget = brqtMoveablePushButton( this );
	} else if( !string.compare( "glwidget" ) ) {
		widget = brqtMoveableGLWidget( this );
	} else if( !string.compare( "radiobutton" ) ) {
		widget = brqtMoveableRadioButtonGroup( this );
	} else if( !string.compare( "vslider" ) ) {
		widget = brqtMoveableVerticalSlider( this );
	} else if( !string.compare( "hslider" ) ) {
		widget = brqtMoveableHorizontalSlider( this );
	} else if( !string.compare( "checkbox" ) ) {
		widget = brqtMoveableCheckBox( this );
	} else if( !string.compare( "label" ) ) {
		widget = brqtMoveableLabel( this );
	} else if( !string.compare( "lineedit" ) ) {
		widget = brqtMoveableLineEdit( this );
	}

	if( !widget ) return;

	// widget ...
	
	widget->move( event->pos().x(), event->pos().y() );

	widget->show();

   	event->acceptProposedAction();
 }
