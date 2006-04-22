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
#include "camera.h"
#include "gldraw.h"
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
    WX_GL_RGBA,
    WX_GL_DOUBLEBUFFER,
    WX_GL_DEPTH_SIZE, 16,
    WX_GL_STENCIL_SIZE, 2,
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

    if (requestlogreset) {
		requestlogreset = 0;
		breverender->ResetLog();
    }

    if (sim != NULL && sim->GetInterface()->Paused() == 0) {
		// The wxMutex documentation states that the default mutex behavior is non
		// recursive, meaning that the same thread cannot lock the mutex twice.  But
		// sure enough, the same thread is able to lock this mutex twice!  When a 
		// breve dialog callback triggers a modal dialog, control is returned to the
		// thread and the engine gets iterated a second time, resulting in a deadlock.
		// So we'll use a mLocked boolean ghetto lock to prevent this.  Note that in
		// a true multithread situation there would be a deadlock here, but since we 
		// are specifically trying to avoid the *same* thread from getting control
		// twice, we're in the clear (this function cannot be run twice from the same
		// thread simultaneously).

		if ( mLocked || sim->GetMutex()->TryLock() != wxMUTEX_NO_ERROR ) {
			event.RequestMore(TRUE);
		    return;
		}

		mLocked = true;

		SetCurrent();

		if (!sim->GetInterface()->Initialized()) {
		    glViewport(0, 0, sim->GetInterface()->GetX(), sim->GetInterface()->GetY());
		    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); 

			return;
		}

		event.RequestMore(TRUE);

		sim->GetInterface()->Iterate();

		sim->GetMutex()->Unlock();

		Refresh(false);

		mLocked = false;
    }
}


//
// the current WXCOCOA implementation seems to be missing the 
// regular constructor?
//

#ifdef __WXCOCOA__
BreveCanvas::BreveCanvas(BreveRender *parent)
    : wxGLCanvas(parent, (wxGLCanvas*) NULL, -1, wxDefaultPosition, 
		wxDefaultSize, 0, "breve canvas" )
#else 
BreveCanvas::BreveCanvas(BreveRender*parent)
    : wxGLCanvas(parent, -1, wxDefaultPosition, wxDefaultSize, 0,
		"breve canvas", (int*)&canvas_attrib, wxNullPalette)
#endif
{
    wxSize size(400,300);

    this->parent = parent;

    rightmenu = new wxMenu("Rightclick menu");
    rightmenuinit = 0;
    requestlogreset = 0;
    mousedown = 0;
    selected = NULL;

	mLocked = false;

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

	i = breverender->GetSimulation()->GetInterface()->GetFrontend()->engine->camera->select( breverender->GetSimulation()->GetInterface()->GetFrontend()->engine->world, x, y );

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

	i = breverender->GetSimulation()->GetInterface()->GetFrontend()->engine->camera->select( breverender->GetSimulation()->GetInterface()->GetFrontend()->engine->world, x, y );

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
	    engine->camera->rotateWithMouseMovement( event.GetX() - mLastX, event.GetY() - mLastY );
	    break;

	case 1:
	    engine->camera->zoomWithMouseMovement( event.GetX() - mLastX, event.GetY() - mLastY );
	    break;

	case 2:
	    engine->camera->moveWithMouseMovement( event.GetX() - mLastX, event.GetY() - mLastY );
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
}

