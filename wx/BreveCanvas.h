#ifndef _BREVECANVAS_H_
#define _BREVECANVAS_H_

#if defined(__GNUG__) && !defined(__APPLE__)
#pragma interface "BreveCanvas.cpp"
#endif

#include <wx/glcanvas.h>

class wxMenu;
class BreveRender;
struct brInstance;

class BreveCanvas : public wxGLCanvas
{
    public:
	BreveCanvas(BreveRender *parent);
	~BreveCanvas();

	void OnIdle(wxIdleEvent& event);
	void OnPaint(wxPaintEvent& event);
	void OnSize(wxSizeEvent& event);
	void OnEraseBackground(wxEraseEvent& event);
	void OnMouseLDown(wxMouseEvent &event);
	void OnMouseRDown(wxMouseEvent &event);
	void OnRightMenu(wxCommandEvent &event);
	void OnMouseUp(wxMouseEvent &event);
	void OnMouseMotion(wxMouseEvent &event);
	void Render();
	void ResetSelection();

	void queMsg(const char *m)
	{
	    msgque.Append(m);
	}

	void queCmd(const char *m)
	{
	    cmdque.Append(m);
	}

	void RequestLogWindowReset()
	{
	    requestlogreset = 1;
	}

	bool MouseDown()
	{
	    return mousedown;
	}

    private:
	BreveRender * parent;
	brInstance * selected;
	wxMenu * rightmenu;
	wxString msgque, cmdque;
	int requestlogreset, rightmenuinit;
	int wasdrag, mLastX, mLastY, mousedown;
	double StartCamX;

DECLARE_EVENT_TABLE()
};

#endif
