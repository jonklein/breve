/****************************************************************************
** ui.h extension file, included from the uic-generated form implementation.
**
** If you want to add, delete, or rename functions or slots, use
** Qt Designer to update this file, preserving your code.
**
** You should not define a constructor or destructor in this file.
** Instead, write your code in functions called init() and destroy().
** These will automatically be called by the form's constructor and
** destructor.
*****************************************************************************/

#include <iostream>
#include <qfile.h>
#include <brqtSteveSyntaxHighlighter.h>

void brqtEditorWindow::init() {
    popupMenu->setTextArea(sourceTextArea);

	setWFlags( Qt::WDestructiveClose);
    
    new brqtSteveSyntaxHighlighter(sourceTextArea);
}

destroy() {
	printf("destroying function...\n");
}

void brqtEditorWindow::destroy() {
	printf("destroying...\n");
    closed(this);
}

char * brqtEditorWindow::getString()
{
    return strdup(sourceTextArea->text().ascii());
}

void brqtEditorWindow::loadFile( QString &file )
{
    QString contents;
    QFile f( file);
   
    f.open( IO_ReadOnly );
    QTextStream stream( &f);
 
    contents = stream.read();
    
    setCaption( file);
    sourceTextArea->setText( contents);
	nameChanged();
}


void brqtEditorWindow::setupMethodPopup()
{

}


void brqtEditorWindow::lineChanged( int para, int pos )
{
    QString s;

    pos = 0;
    
    s.sprintf("%d", para + 1);
    lineBox->setText( s);
}


void brqtEditorWindow::selectLine( )
{
    int line = lineBox->text().toInt();
    
    if(line > sourceTextArea->paragraphs()) {
	QString newLine;
	
	line = sourceTextArea->paragraphs();	
	newLine.sprintf("%d", line);
	lineBox->setText(newLine);
    }
	
    sourceTextArea->setSelection(line - 1, 0, line, 0);
}

void brqtEditorWindow::setController( QWidget *w)
{
    connect(this, SIGNAL( closed( QWidget*) ), w, SLOT( closeDocument( QWidget*) ) );
    connect(this, SIGNAL( nameChanged() ), w, SLOT( changeDocumentName( ) ) );
}
