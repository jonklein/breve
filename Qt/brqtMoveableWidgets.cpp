#include <QApplication>
#include <QPushButton>
#include <QMoveEvent>
#include <QWidget>
#include <QSlider>
#include <QGridLayout>
#include <QRadioButton>

#include "brqtMoveableWidgets.h"

brqtMoveable< QPushButton > *brqtMoveablePushButton( brqtMainWindow *w ) {
	brqtMoveable< QPushButton > *button;

	button = new brqtMoveable< QPushButton >( w );

	button->setText( button->tr( "Button" ) );
	button->setMinimumSize( QSize( 50, 30 ) );
	button->resize( QSize( 100, 30 ) );

	return button;
}

brqtMoveable< QSlider > *brqtMoveableVerticalSlider( brqtMainWindow *w ) {
	brqtMoveable< QSlider > *slider;

	slider = new brqtMoveable< QSlider >( w );
	slider->setOrientation( Qt::Vertical );
	slider->setMinimumSize( QSize( 30, 100 ) );
	slider->setMaximumSize( QSize( 30, 10000 ) );
	slider->resize( QSize( 30, 200 ) );

	return slider;
}

brqtMoveable< QSlider > *brqtMoveableHorizontalSlider( brqtMainWindow *w ) {
	brqtMoveable< QSlider > *slider;

	slider = new brqtMoveable< QSlider >( w );
	slider->setOrientation( Qt::Horizontal );
	slider->setMinimumSize( QSize( 100, 30 ) );
	slider->setMaximumSize( QSize( 10000, 30 ) );
	slider->resize( QSize( 200, 30 ) );

	return slider;
}

brqtMoveable< QCheckBox > *brqtMoveableCheckBox( brqtMainWindow *w ) {
	brqtMoveable< QCheckBox > *checkbox;

	checkbox = new brqtMoveable< QCheckBox >( w );
	checkbox->setText( "Check Box" );
	checkbox->setMinimumSize( QSize( 30, 30 ) );

	return checkbox;
}

brqtMoveable< QLabel > *brqtMoveableLabel( brqtMainWindow *w ) {
	brqtMoveable< QLabel > *label;

	label = new brqtMoveable< QLabel >( w );
	label->setText( "Information Text" );
	label->resize( QSize( 200, 30 ) );
	label->setMinimumSize( QSize( 30, 30 ) );

	return label;
}

brqtMoveable< QLineEdit > *brqtMoveableLineEdit( brqtMainWindow *w ) {
	brqtMoveable< QLineEdit > *lineedit;

	lineedit = new brqtMoveable< QLineEdit >( w );
	lineedit->setText( "Input Text" );
	lineedit->resize( QSize( 200, 30 ) );
	lineedit->setMinimumSize( QSize( 30, 30 ) );

	return lineedit;
}

brqtMoveable< QGroupBox > *brqtMoveableGLWidget( brqtMainWindow *w ) {
	brqtGLWidget *glwidget;
	brqtMoveable< QGroupBox > *frame;
	QGridLayout *layout = new QGridLayout;

	frame = new brqtMoveable< QGroupBox >( w );
	frame->setTitle( "" );
	
	frame->setLayout( layout );
	frame->setAlignment( Qt::AlignHCenter );
	frame->resize( 204, 204 );
	frame->setMinimumSize( QSize( 30, 30 ) );

	glwidget = new brqtGLWidget( frame );

	glwidget -> setEnabled( false );

	layout -> setMargin( 2 );
	layout -> addWidget( glwidget );

	return frame;
}

brqtMoveable< QGroupBox > *brqtMoveableRadioButtonGroup( brqtMainWindow *w ) {
	brqtMoveable< QGroupBox > *frame;
	QVBoxLayout *layout = new QVBoxLayout;

	frame = new brqtMoveable< QGroupBox >( w );
	frame->setTitle( "Radio Button Group" );
	frame->resize( 130, 90 );
	frame->setMinimumSize( QSize( 60, 60 ) );
	
	frame->setLayout( layout );

	QRadioButton *button;

	button = new QRadioButton( "Option 1", frame );
	button->setEnabled( false );
	layout->addWidget( button );

	button = new QRadioButton( "Option 2", frame );
	button->setEnabled( false );
	layout->addWidget( button );

	return frame;
}

