#if defined(__GNUG__) && !defined(__APPLE__)
#pragma implementation "BreveCanvas.h"
#endif

#include "wx/wxprec.h"  

#ifdef __BORLANDC__
#pragma hdrstop
#endif

#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif

#include "BreveRender.h"
#include "BreveCanvas.h"
#include "BreveInterface.h"
#include "Main.h"
#include "steve.h"
#include "SimInstance.h"

BEGIN_EVENT_TABLE(BreveCanvas, wxGLCanvas)
    EVT_SIZE(BreveCanvas::OnSize)
    EVT_PAINT(BreveCanvas::OnPaint)
    EVT_ERASE_BACKGROUND(BreveCanvas::OnEraseBackground)
    EVT_IDLE(BreveCanvas::OnIdle)
    EVT_MENU_RANGE(10100, 10200, BreveCanvas::OnRightMenu)
    EVT_LEFT_DOWN(BreveCanvas::OnMouseLDown)
    EVT_RIGHT_DOWN(BreveCanvas::OnMouseRDown)
    EVT_LEFT_UP(BreveCanvas::OnMouseUp)
    EVT_MOTION(BreveCanvas::OnMouseMotion)
END_EVENT_TABLE()

int canvas_attrib[] =
{
    4,
    1,
    11, 16,
    12, 2,
    0
};

void BreveCanvas::OnIdle(wxIdleEvent&event)
{
    SimInstance * sim = parent->GetSimulation();

    if (!msgque.IsEmpty())
    {
	wxString tmp;

	tmp = msgque;

	msgque.Empty();

	event.RequestMore(TRUE);

	wxMessageBox(tmp);
	return;
    }

    if (!cmdque.IsEmpty())
    {
	char * bstr;
	wxString tmp;

	tmp = cmdque;

	cmdque.Empty();

	if (sim != NULL && sim->GetInterface()->Initialized())
	{
	    bstr = (char*)malloc(sizeof(char) * (tmp.Length() + 2));

	    strcpy(bstr, tmp);
	    strcat(bstr,"\n");

	    sim->GetInterface()->RunCommand(bstr);

	    free(bstr);
	}
    }

    if (requestlogreset)
    {
	requestlogreset = 0;
	breverender->ResetLog();
    }

    if (sim != NULL && sim->GetInterface()->Paused() == 0)
    {
	if (sim->GetMutex()->TryLock() == wxMUTEX_BUSY)
	{
	    event.RequestMore(TRUE);
	    return;
	}

	SetCurrent();

	if (!sim->GetInterface()->Initialized())
	{
	    sim->UpdateSimCode();

	    if (!sim->GetInterface()->Initialize())
	    {
		sim->GetMutex()->Unlock();
		return;
	    }

	    sim->GetMutex()->Unlock();

	    glViewport(0, 0, sim->GetInterface()->GetX(), sim->GetInterface()->GetY());
	    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); 
	}

	event.RequestMore(TRUE);

	sim->GetInterface()->Iterate();

	sim->GetMutex()->Unlock();

	Refresh(false);
    }
}

BreveCanvas::BreveCanvas(BreveRender*parent)
    : wxGLCanvas(parent, -1, wxDefaultPosition, wxDefaultSize, 0,
		       "Breve Canvas", (int*)&canvas_attrib, wxNullPalette)
{
    wxSize size(400,300);

    this->parent = parent;

    rightmenu = new wxMenu("Rightclick menu");
    rightmenuinit = 0;
    requestlogreset = 0;
    mousedown = 0;
    selected = NULL;

    SetSizeHints(400, 300);
//    SetMinSize(size);

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

BreveCanvas::~BreveCanvas()
{
    delete rightmenu;
}

void BreveCanvas::Render()
{
    static int i = 0;
    static int init = 0;

    wxPaintDC dc(this);
    
#ifndef __WXMOTIF__  
    if (!GetContext()) return;
#endif

    SetCurrent();

    SimInstance * sim = parent->GetSimulation();

    if (sim != NULL && sim->GetInterface()->Initialized())
    {
	sim->GetMutex()->Lock();

	sim->GetInterface()->Render();

	sim->GetMutex()->Unlock();

	glFlush();
	SwapBuffers();
    }
    else
    {
	dc.SetBackground(*wxBLACK_BRUSH);
	dc.Clear();
    }
}

void BreveCanvas::OnPaint( wxPaintEvent& WXUNUSED(event) )
{
    Render();
}

void BreveCanvas::OnEraseBackground(wxEraseEvent& WXUNUSED(event))
{
  // Do nothing, to avoid flashing.
}

void BreveCanvas::OnSize(wxSizeEvent& event)
{   
    int w, h;

    wxGLCanvas::OnSize(event);

    GetClientSize(&w, &h);

    SimInstance * sim = parent->GetSimulation();

    if (sim == NULL || !sim->GetInterface()->Initialized())
    {
	wxClientDC dc(this);

	dc.SetBackground(*wxBLACK_BRUSH);
	dc.Clear();
    }

// Possible check needed - see cube
    SetCurrent();

    glViewport(0, 0, (GLint) w, (GLint) h);

    if (sim != NULL)
    {
	sim->GetInterface()->SetX(w);
	sim->GetInterface()->SetY(h);
	sim->GetInterface()->ResizeView(w,h);
    }

    Refresh(false);
}
       
void BreveCanvas::OnMouseRDown(wxMouseEvent &event)
{
    int x, y;

    event.Skip();

    if (breverender->GetSimulation() == NULL || !breverender->GetSimulation()->GetInterface()->Initialized())
	return;

    x = event.GetX();
    y = event.GetY();

    {
	int i = 0;

	breverender->GetSimulation()->GetMutex()->Lock();

	i = slGlSelect(breverender->GetSimulation()->GetInterface()->GetFrontend()->engine->world, breverender->GetSimulation()->GetInterface()->GetFrontend()->engine->camera, x, y);

	selected = brClickCallback(breverender->GetSimulation()->GetInterface()->GetFrontend()->engine, i);

	breverender->GetSimulation()->GetMutex()->Unlock();

	breverender->GetSimulation()->SetSelected(selected);

	delete rightmenu;
	rightmenu = new wxMenu("Rightclick Menu");

	rightmenuinit = 0;

	if (selected)
	{
	    brMenuEntry * e;

	    for (i = 0; i < selected->menus->count; i++)
	    {
		rightmenuinit = 1;

		e = (brMenuEntry*)selected->menus->data[i];

		if (e->title[0] == '\0')
		{
		    rightmenu->AppendSeparator();
		    continue;
		}

		rightmenu->Append(10100 + i, e->title, "", wxITEM_NORMAL);

		if (!e->enabled)
		    rightmenu->Enable(10100 + i, FALSE);

		//if (e->checked)
		//    rightmenu->Check(10100 + i, TRUE);
	    }
	}

	//brClickAtLocation(
	//    breverender->GetSimulation()->GetInterface()->GetFrontend()->engine,
	//    x, y);

	breverender->GetSimulation()->GetInterface()->menuCallback(
	    breverender->GetSimulation()->GetInterface()->GetFrontend()->engine->controller);

	Refresh(TRUE, NULL);

	if (rightmenuinit)
	    PopupMenu(rightmenu, event.GetX(), event.GetY());
    }
}

void BreveCanvas::OnMouseLDown(wxMouseEvent &event)
{
    event.Skip();

    CaptureMouse();

    mousedown = 1;

    if (breverender->GetSimulation() == NULL || !breverender->GetSimulation()->GetInterface()->Initialized())
	return;

    wasdrag = 0;

    mLastX = event.GetX();
    mLastY = event.GetY();
}

void BreveCanvas::OnMouseUp(wxMouseEvent &event)
{
    int x, y;

    event.Skip();

    ReleaseMouse();

    mousedown = 0;

    Refresh(TRUE, NULL);

    if (breverender->GetSimulation() == NULL || !breverender->GetSimulation()->GetInterface()->Initialized())
	return;

    x = event.GetX();
    y = event.GetY();

    if (wasdrag == 0 && parent->GetMouseMode() == 3)
    {
	int i = 0;

	breverender->GetSimulation()->GetMutex()->Lock();

	i = slGlSelect(breverender->GetSimulation()->GetInterface()->GetFrontend()->engine->world, breverender->GetSimulation()->GetInterface()->GetFrontend()->engine->camera, x, y);

	selected = brClickCallback(breverender->GetSimulation()->GetInterface()->GetFrontend()->engine, i);

	breverender->GetSimulation()->GetMutex()->Unlock();

	breverender->GetSimulation()->SetSelected(selected);

	delete rightmenu;
	rightmenu = new wxMenu("Rightclick Menu");

	rightmenuinit = 0;

	if (selected)
	{
	    brMenuEntry * e;

	    for (i = 0; i < selected->menus->count; i++)
	    {
		rightmenuinit = 1;

		e = (brMenuEntry*)selected->menus->data[i];

		if (e->title[0] == '\0')
		{
		    rightmenu->AppendSeparator();
		    continue;
		}

		rightmenu->Append(10100 + i, e->title, "", wxITEM_NORMAL);

		if (!e->enabled)
		    rightmenu->Enable(10100 + i, FALSE);

		//if (e->checked)
		//    rightmenu->Check(10100 + i, TRUE);
	    }
	}

	//brClickAtLocation(
	//    breverender->GetSimulation()->GetInterface()->GetFrontend()->engine,
	//    x, y);

	breverender->GetSimulation()->GetInterface()->menuCallback(
	    breverender->GetSimulation()->GetInterface()->GetFrontend()->engine->controller);

	Refresh(TRUE, NULL);
	return;
    }
}

void BreveCanvas::OnMouseMotion(wxMouseEvent &event)
{
    brEngine * engine;

    event.Skip();

    if (!event.Dragging() || breverender->GetSimulation() == NULL || !breverender->GetSimulation()->GetInterface()->Initialized())
	return;

    wasdrag = 1;

    engine = breverender->GetSimulation()->GetInterface()->GetFrontend()->engine;

    breverender->GetSimulation()->GetMutex()->Lock();

    switch (parent->GetMouseMode())
    {
	case 0:
	    slRotateCameraWithMouseMovement(engine->camera, event.GetX() - mLastX, event.GetY() - mLastY);
	    break;

	case 1:
	    slZoomCameraWithMouseMovement(engine->camera, event.GetX() - mLastX, event.GetY() - mLastY);
	    break;

	case 2:
	    slMoveCameraWithMouseMovement(engine->camera, event.GetX() - mLastX, event.GetY() - mLastY);
	    break;

	case 3:
	    brDragCallback(engine, event.GetX(), event.GetY());
	    break;
    }

    breverender->GetSimulation()->GetMutex()->Unlock();

    Refresh(TRUE, NULL);

    mLastX = event.GetX();
    mLastY = event.GetY();
}

void BreveCanvas::OnRightMenu(wxCommandEvent &event)
{
    if (breverender->GetSimulation() == NULL || !breverender->GetSimulation()->GetInterface()->Initialized() || !selected)
	return;

    breverender->GetSimulation()->GetMutex()->Lock();
    breverender->GetSimulation()->GetInterface()->RunMenu(event.m_id - 10100, selected);
    breverender->GetSimulation()->GetMutex()->Unlock();
}

        
void BreveCanvas::ResetSelection()
{
    selected = NULL;

    if (breverender->GetSimulation() != NULL)
	breverender->GetSimulation()->SetSelected(NULL);
}

