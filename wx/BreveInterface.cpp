#if defined(__GNUG__) && !defined(__APPLE__)
#pragma implementation "BreveInterface.h"
#endif

#include "wx/wxprec.h"  

#ifdef __BORLANDC__
#pragma hdrstop
#endif

#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif

#include "BreveInterface.h"
#include "BreveRender.h"
#include "BreveCanvas.h"
#include "Main.h"
#include "steve.h"
#include "camera.h"
#include "gldraw.h"
#include "java.h"
#include "SimInstance.h"
#include "BDialog.h"

BreveInterface::BreveInterface(char * simfile, wxString simdir, char * text)
{
	char buf[2048];
	wxString str;
	int i = 0;

	mSleepMS = 100;

	this->simulationfile = simfile;
	this->text = text;
	this->next = NULL;

	// frontend = breveFrontendInit(0, NULL);
	// Due to the annoying java error I'm unable to track down, it isn't safe
	// to call this function.  It calls brJavaInit, which doesn't actually
	// do anything when using the directory layout of the CVS tree.  However,
	// if you happen to have breveIDE living in a directory with lib/classes,
	// brJavaInit will attempt to create a VM - failing due to searchpaths
	// not yet being setup.  We don't want that - it'll eventually cause
	// the client to crash.  Note that even if searchpaths are configured
	// correctly before brJavaInit is called, java will still eventually
	// crash the client due to some internal error.  I have no idea how to
	// resolve this.

	frontend = new breveFrontend;
	frontend->engine = brEngineNew();
	frontend->engine->argc = 0;
	frontend->engine->argv = NULL;

	frontend->data = breveFrontendInitData(frontend->engine);

	strncpy(buf, app->GetLocalDir(), 2047);
	buf[2047] = '\0';
	brAddSearchPath(frontend->engine, (char*)&buf);

	this->simmenu = new wxMenu;
	paused = 1;
	valid = 1;
	initialized = 0;
	x = 400;
	y = 300;

	frontend->engine->getLoadname = ::getLoadname;
	frontend->engine->getSavename = ::getSavename;
	frontend->engine->soundCallback = soundCallback;
	frontend->engine->pauseCallback = pauseCallback;
	frontend->engine->dialogCallback = ::dialogCallback;
	frontend->engine->interfaceTypeCallback = interfaceVersionCallback;
	frontend->engine->newWindowCallback = newWindowCallback;
	frontend->engine->freeWindowCallback = freeWindowCallback;
	frontend->engine->renderWindowCallback = renderWindowCallback;
	brEngineSetUpdateMenuCallback(frontend->engine, ::menuCallback);

	slSetMessageCallbackFunction(::messageCallback);

	for (i = 0; i < app->GetSearchPathArray()->Count(); i++)
	{
	strncpy(buf, app->GetSearchPathArray()->Item(i), 2047);
	buf[2047] = '\0';
	brAddSearchPath(frontend->engine, (char*)&buf);
	}

	if (!simdir.IsEmpty())
	{
	strncpy(buf, simdir, 2047);
	buf[2047] = '\0';
	brAddSearchPath(frontend->engine, (char*)&buf);
	}

	strncpy(buf, app->GetBreveDir(), 2047);
	buf[2047] = '\0';
	brAddSearchPath(frontend->engine, (char*)&buf);

	strncpy(buf, app->GetBreveDir() + "plugins" + FILE_SEP_PATH, 2047);
	buf[2047] = '\0';
	brAddSearchPath(frontend->engine, (char*)&buf);

	strncpy(buf, app->GetBreveDir() + "java" + FILE_SEP_PATH, 2047);
	buf[2047] = '\0';
	brAddSearchPath(frontend->engine, (char*)&buf);
}

BreveInterface::~BreveInterface()
{
	if (frontend != NULL) {
		brEngineFree(frontend->engine);
		breveFrontendCleanupData(frontend->data);
		breveFrontendDestroy(frontend);

		frontend = NULL;
	}

	if (simmenu != NULL)
	{
	while (simmenu->GetMenuItemCount() > 0)
		simmenu->Destroy(simmenu->FindItemByPosition(0));
	}

	delete simmenu;
	free(simulationfile);

	if (text != NULL)
	free(text);
}

void BreveInterface::Pause( int pause )
{
	if ( pause == paused )
		return;

	paused = !paused;

	if ( paused )
		brPauseTimer( frontend->engine );
	else
		brUnpauseTimer( frontend->engine );
}

bool BreveInterface::Initialize()
{
	slInitGL( frontend->engine->world, frontend->engine->camera );

	if ( breveFrontendLoadSimulation( frontend, text, simulationfile ) != EC_OK ) {
		reportError();
		slFree( text );
		text = NULL;
		Abort();
		return 0;
	}

	slFree(text);
	text = NULL;

	ResizeView(x,y);

	initialized = 1;

	return 1;
}

void BreveInterface::ResizeView(int x, int y)
{
	frontend->engine->camera->setBounds( x, y );
}

void BreveInterface::Iterate()
{
	if (frontend == NULL || !initialized)
	return;

	if (brEngineIterate(frontend->engine) != EC_OK) {
		reportError();
		Abort();
	}

	usleep( gBreverender->GetSleepMS() * 1000 );
}

void BreveInterface::Render() {
	if (frontend == NULL || !initialized)
	return;

	brEngineLock(frontend->engine);
	brEngineRenderWorld( frontend->engine,gBreverender->MouseDown() );
	brEngineUnlock(frontend->engine);
}

void BreveInterface::Abort() {
	if ( valid == 0 ) return;

	gBreverender->ResetSim();

	valid = 0;
}

char * getLoadname()
{
	return gBreverender->GetSimulation()->GetInterface()->getLoadname();
}

char * getSavename()
{
	return gBreverender->GetSimulation()->GetInterface()->getSavename();
}

char * BreveInterface::getLoadname()
{
	wxTextEntryDialog d(gBreverender, "Loadname required.", "Please enter filename to load:");
	char * buf;

	buf = (char*)slMalloc(1024);

	if (d.ShowModal() == wxID_OK)
	{
	strncpy(buf, d.GetValue(), 1023);

	if (d.GetValue().Length() > 1023)
		buf[1023] = '\0';
	}
	else
	buf[0] = '\0';

	return buf;
}

char * BreveInterface::getSavename()
{
	wxTextEntryDialog d(gBreverender, "Savename required.", "Please enter filename to save:");
	char * buf;

	buf = (char*)slMalloc(1024);

	if (d.ShowModal() == wxID_OK)
	{
	strncpy(buf, d.GetValue(), 1023);

	if (d.GetValue().Length() > 1023)
		buf[1023] = '\0';
	}
	else
	buf[0] = '\0';

	return buf;
}

void BreveInterface::reportError()
{
	brErrorInfo *error;

	if (frontend->engine == NULL)
		return;

	error = brEngineGetErrorInfo( frontend->engine );

	char errorMessage[ 10240 ];

	snprintf( errorMessage, 10239, "Error at line %d of \"%s\":\n\n%s\n\nSee log window for details", error->line, error->file, error->message );

	gBreverender->queMsg( errorMessage );
}

void BreveInterface::messageCallback(char *text)
{
	gBreverender->AppendLog(text);
}

void messageCallback(char *text)
{
	if (gBreverender->GetSimulation() == NULL)
	return;

	gBreverender->GetSimulation()->GetInterface()->messageCallback(text);
}

int soundCallback()
{
	wxBell();
	return 0;
}

int pauseCallback()
{
	if (gBreverender->GetSimulation() == NULL)
	return 0;

	if (gBreverender->GetSimulation()->GetInterface()->Paused())
	return 0;

	gBreverender->OnRenderRunClick(*((wxCommandEvent*)NULL));

	return 0;
}

void *newWindowCallback(char *name, void *graph)
{
	printf("newWindowCallback stub\n\r");
}

void freeWindowCallback(void *w)
{
	printf("freeWindowCallback stub\n\r");
}

void renderWindowCallback(void *w)
{
	printf("renderWindowCallback\n\r");
}

void graphDisplay()
{
	printf("graphDisplay stub\n\r");
}

void menuCallback(brInstance *i)
{
	gBreverender->GetSimulation()->GetInterface()->menuCallback(i);
}

void BreveInterface::menuCallback(brInstance *binterface)
{
	brMenuEntry * e;
	int i = 0;

	if(!binterface->engine || binterface != brEngineGetController(binterface->engine))
	return;

	gBreverender->SetMenu(0);

	delete simmenu;

	simmenu = new wxMenu;

	for (i = 0; i < binterface->_menus.size(); i++)
	{
	e = (brMenuEntry*)binterface->_menus[ i ];

	if (e->title[0] == '\0')
	{
		simmenu->AppendSeparator();
		continue;
	}

	simmenu->Append(BREVE_SIMMENU + i, e->title, "", wxITEM_CHECK);

	if (!e->enabled)
		simmenu->Enable(BREVE_SIMMENU + i, FALSE);

	if (e->checked)
		simmenu->Check(BREVE_SIMMENU + i, TRUE);
	}

	gBreverender->SetMenu(1);
}

int BreveInterface::dialogCallback(char *title, char *message, char *b1, char *b2)
{
	BDialog d(gBreverender, title, message, b1, b2);

	return !d.ShowModal();
}

int dialogCallback(char *title, char *message, char *b1, char *b2)
{
	return gBreverender->GetSimulation()->GetInterface()->dialogCallback(title, message, b1, b2);
}

char *interfaceVersionCallback()
{
	return "wxwidgets/2.0";
}

void BreveInterface::RunMenu(int id, brInstance *n) {
	int i;

	if (brMenuCallback(frontend->engine, n, id) != EC_OK) {
		reportError();
		Abort();
	}
}

void BreveInterface::ExecuteCommand(wxString str) {
	wxString nstr;

	nstr << "> " << str << "\n";

	gBreverender->AppendLog(nstr);

	gBreverender->queCmd(str);
}

void BreveInterface::RunCommand(char * str) {
	stRunSingleStatement((stSteveData*)frontend->data, frontend->engine, str);
}
