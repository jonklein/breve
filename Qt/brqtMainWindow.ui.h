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

#include <qstring.h>

#undef Unsorted
#include <qfiledialog.h>

#include "brqtEditorWindow.h"
#include "brqtErrorWindow.h"

#include <sys/types.h>
#include <dirent.h>
#include <vector>

#include "brqtEngine.h"

#include "steve.h"

breveFrontend *frontend;
brqtEditorWindow *gW;

std::vector< brqtEditorWindow* > documents;

std::map<int, QString> demoMap;

int demoMenuForDirectory(QWidget *receiver, QPopupMenu *menu, QString &directory, int offset) {
	int count, n;
	struct dirent **demos;

	count = scandir(directory.ascii(), &demos, NULL, alphasort);

	QWidget::connect(menu, SIGNAL(activated(int)), receiver, SLOT(loadDemo(int)));

	for(n=count-1;n>-1;n--) {
		QString name = demos[n]->d_name;

		if(demos[n]->d_type == DT_DIR) {
			QPopupMenu *submenu = new QPopupMenu(0, name);

			if(name != ".." && name != ".") {
				QString subname = directory + "/" + name;
				
				offset = demoMenuForDirectory(receiver, submenu, subname, offset);

				if(submenu->count()) {
					menu->insertItem( name, submenu, offset++, 0);
				} else {
					delete submenu;
				}
			}	

		} else {
			demoMap[ offset] = directory + "/" + name;
				
			if(name.endsWith(".tz")) menu->insertItem( name, offset++, 0);	
		}
	}
	
	return offset;
}

void brqtMainWindow::init() {
	QString directory;
	char *demoPath = getenv("BREVE_DEMO_PATH");

	if(demoPath) directory = demoPath;
	else directory = "demos";

	QPoint p;
	
#ifdef MACOSX 
	MenuBar->reparent(0, 0, p, true);
#endif

	currentEngine = NULL;

	demoMenuForDirectory(this, Demos, directory, 0);
}

void brqtMainWindow::fileNew() {
    brqtEditorWindow *w = newDocument();

    w->show();
    
    gW = w;
}


void brqtMainWindow::fileOpen() {
	QString s = QFileDialog::getOpenFileName(
		"/home",
		"breve files (*.tz)",
		this,
		"Open File...",
		"Choose a file" );

	// cout << s;

	if( s != "") {
		brqtEditorWindow *w = newDocument();
		w->loadFile(s);
	}
}


void brqtMainWindow::fileSave() {
}


void brqtMainWindow::fileSaveAs() {
	QString s = QFileDialog::getSaveFileName(
		"/home",
		"breve files (*.tz)",
		this,
		"Save File...",
		"Choose a filename to save under" );

}


void brqtMainWindow::filePrint()
{

}


void brqtMainWindow::fileExit()
{
    exit(0);
}


void brqtMainWindow::editUndo()
{

}


void brqtMainWindow::editRedo()
{

}


void brqtMainWindow::editCut()
{

}


brqtEditorWindow *brqtMainWindow::newDocument()
{
    int xloc;
    brqtEditorWindow *w = new brqtEditorWindow;
    w->setCaption("Untitled");
    w->setController(this);
    
    xloc = width() + 4;
    
    w->move( QPoint( xloc, 24 ) );
    documents.push_back(w);

	buildDocumentMenu();
    
    return w;
}


void brqtMainWindow::editPaste()
{

}


void brqtMainWindow::editFind()
{

}


void brqtMainWindow::helpIndex()
{

}


void brqtMainWindow::helpContents()
{

}


void brqtMainWindow::helpAbout()
{

}


void brqtMainWindow::toggleSimulation()
{
    brqtEditorWindow *w = gW;

	if(currentEngine) {
		currentEngine->pause();
		return;
	}
    
    frontend = breveFrontendInit(0, NULL);
    frontend->data = breveFrontendInitData(frontend->engine);
    frontend->engine->camera = slCameraNew(400, 400);

    if(breveFrontendLoadSimulation(frontend, w->getString(), "<untitled>") != EC_OK) {
	brqtErrorWindow *w = new brqtErrorWindow(0);
	w->displayError(&frontend->engine->error);
	w->show();
	return;
    }

    currentEngine = new brqtEngine(frontend->engine, breveGLWidget1);
}

void brqtMainWindow::loadDemo(int n)
{
    brqtEditorWindow *w = newDocument();
	
    w->loadFile(demoMap[n]);
    w->show();		
    
    gW = w;
}


void brqtMainWindow::stopSimulation()
{
    if (currentEngine) {
      delete currentEngine;
      currentEngine = NULL;

      breveFrontendDestroy(frontend);
      frontend = NULL;
    }
}


void brqtMainWindow::closeDocument( QWidget *document )
{
    std::vector< brqtEditorWindow* >::iterator wi;
    wi = std::find(documents.begin(), documents.end(), document);
    
    printf("closing document...\n");
    
    documents.erase(wi);
    buildDocumentMenu();
}

void brqtMainWindow::changeDocumentName()
{
    buildDocumentMenu();
}


void brqtMainWindow::buildDocumentMenu()
{
    documentMenu->clear();

	for(unsigned int n=0; n<documents.size(); n++) {
    	documentMenu->insertItem( documents[n]->caption() );
	}
}
