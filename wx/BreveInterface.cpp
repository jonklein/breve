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
#include "java.h"
#include "SimInstance.h"
#include "BDialog.h"

BreveInterface::BreveInterface(char * simfile, char * text)
{
    char wd[10240];
    static char * buf[1];

    buf[0] = '\0';

    this->simulationfile = simfile;
    this->text = text;

    this->next = NULL;

    frontend = breveFrontendInit(1, (char**)&buf);
    frontend->data = breveFrontendInitData(frontend->engine);

    brEngineSetIOPath(frontend->engine, getcwd(wd, 10239));

    frontend->engine->argc = 1;
    frontend->engine->argv = buf;
    // Gotta love hacky stuff

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

    {
	char buf[2048];
	wxString str;

	str = app->GetBreveDir();

	str << "plugins";

	strncpy(buf, str, 2048);

	brAddSearchPath(frontend->engine, (char*)&buf);

	str = app->GetBreveDir();

	str << "lib" << FILE_SEP_PATH << "classes";

	strncpy(buf, str, 2048);

	brAddSearchPath(frontend->engine, (char*)&buf);

	str = app->GetBreveDir();

	str << "java";

	strncpy(buf, str, 2048);

	brAddSearchPath(frontend->engine, (char*)&buf);
    }
}

BreveInterface::~BreveInterface()
{
    if (frontend != NULL)
    {
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
}

void BreveInterface::Pause(int pause)
{
    if (pause == paused)
	return;

    paused = !paused;

    if (paused)
	brPauseTimer(frontend->engine);
    else
	brUnpauseTimer(frontend->engine);
}

bool BreveInterface::Initialize()
{
    slInitGL(frontend->engine->world, frontend->engine->camera);

    if (breveFrontendLoadSimulation(frontend, text, simulationfile) != EC_OK)
    {
	reportError();
	slFree(text);
	text = NULL;
	Abort();
	return 0;
    }

    slFree(text);

    ResizeView(x,y);

    initialized = 1;

    return 1;
}

void BreveInterface::ResizeView(int x, int y)
{
    frontend->engine->camera->x = x; frontend->engine->camera->y = y;
    frontend->engine->camera->fov = (double)x/(double)y;
}

void BreveInterface::Iterate()
{
    if (frontend == NULL || !initialized)
	return;

    if (brEngineIterate(frontend->engine) != EC_OK)
    {
	reportError();
	Abort();
    }
}

void BreveInterface::Render()
{
    if (frontend == NULL || !initialized)
	return;

    brEngineLock(frontend->engine);
    brEngineRenderWorld(frontend->engine,breverender->MouseDown());
    brEngineUnlock(frontend->engine);
}

void BreveInterface::Abort()
{
    if (valid == 0)
	return;

    valid = 0;
}

char * getLoadname()
{
    return breverender->GetSimulation()->GetInterface()->getLoadname();
}

char * getSavename()
{
    return breverender->GetSimulation()->GetInterface()->getSavename();
}

char * BreveInterface::getLoadname()
{
    wxTextEntryDialog d(breverender, "Loadname required.", "Please enter filename to load:");
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
    wxTextEntryDialog d(breverender, "Savename required.", "Please enter filename to save:");
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
    brErrorInfo * error;

    if (frontend->engine == NULL)
	return;

    error = brEngineGetErrorInfo(frontend->engine);

    breverender->queMsg(error->message);
}

void BreveInterface::messageCallback(char *text)
{
    breverender->AppendLog(text);
}

void messageCallback(char *text)
{
    if (breverender->GetSimulation() == NULL)
	return;

    breverender->GetSimulation()->GetInterface()->messageCallback(text);
}

int soundCallback()
{
    wxBell();
    return 0;
}

int pauseCallback()
{
    if (breverender->GetSimulation() == NULL)
	return 0;

    if (breverender->GetSimulation()->GetInterface()->Paused())
	return 0;

    breverender->OnRenderRunClick(*((wxCommandEvent*)NULL));

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
    breverender->GetSimulation()->GetInterface()->menuCallback(i);
}

void BreveInterface::menuCallback(brInstance *binterface)
{
    brMenuEntry * e;
    int i = 0;

    if(!binterface->engine || binterface != brEngineGetController(binterface->engine))
	return;

    breverender->SetMenu(0);

    delete simmenu;

    simmenu = new wxMenu;

    for (i = 0; i < binterface->menus->count; i++)
    {
	e = (brMenuEntry*)binterface->menus->data[i];

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

    breverender->SetMenu(1);
}

int BreveInterface::dialogCallback(char *title, char *message, char *b1, char *b2)
{
    BDialog * d = new BDialog(breverender, title, message, b1, b2);

    return !d->ShowModal();
}

int dialogCallback(char *title, char *message, char *b1, char *b2)
{
    return breverender->GetSimulation()->GetInterface()->dialogCallback(title, message, b1, b2);
}

char *interfaceVersionCallback()
{
    return "wxwidgets/2.0";
}

void BreveInterface::RunMenu(int id, brInstance *n)
{
    int i;

    if (brMenuCallback(frontend->engine, n, id) != EC_OK)
    {
	reportError();
	Abort();
    }
}

void BreveInterface::ExecuteCommand(wxString str)
{
    wxString nstr;

    nstr << "> " << str << "\n";

    breverender->AppendLog(nstr);

    breverender->queCmd(str);

    //stRunSingleStatement((stSteveData*)frontend->data, frontend->engine, bstr);
}

void BreveInterface::RunCommand(char * str)
{
    stRunSingleStatement((stSteveData*)frontend->data, frontend->engine, str);
}
