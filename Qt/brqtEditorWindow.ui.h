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

const char * brqtEditorWindow::getString()
{
    return strdup(textEdit1->text().ascii());
}




void brqtEditorWindow::loadFile( QString &file )
{
    QString contents;
    QFile f( file);
   
    f.open( IO_ReadOnly );
    QTextStream stream( &f);
 
    contents = stream.read();
    
    setCaption( file);
    textEdit1->setText( contents);
}


void brqtEditorWindow::setupMethodPopup()
{

}
