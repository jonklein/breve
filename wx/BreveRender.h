/////////////////////////////////////////////////////////////////////////////
// Name:        BreveRender.h
// Purpose:     
// Author:      
// Modified by: 
// Created:     Tue 01 Mar 2005 10:16:35 PM EST
// RCS-ID:      
// Copyright:   
// Licence:     
/////////////////////////////////////////////////////////////////////////////

#ifndef _BREVERENDER_H_
#define _BREVERENDER_H_

#if defined(__GNUG__) && !defined(__APPLE__)
#pragma interface "BreveRender.cpp"
#endif

#include <wx/frame.h>
#include <wx/checkbox.h>
#include "BreveCanvas.h"
#include "LogWindow.h"

#if !wxUSE_TOGGLEBTN
    #define wxToggleButton wxCheckBox
    #define EVT_TOGGLEBUTTON EVT_CHECKBOX
#endif

#define ID_FRAME 10000
#define SYMBOL_BREVERENDER_STYLE wxCAPTION|wxRESIZE_BORDER|wxSYSTEM_MENU|wxCLOSE_BOX
#define SYMBOL_BREVERENDER_TITLE _("Breve Render Window")
#define SYMBOL_BREVERENDER_IDNAME ID_FRAME
#define SYMBOL_BREVERENDER_SIZE wxSize(400, 300)
#define SYMBOL_BREVERENDER_POSITION wxDefaultPosition
#define ID_FOREIGN 10001
#define ID_RENDER_RUN 10002
#define ID_RENDER_STOP 10003
#define ID_SIM_SELECT 10004
#define ID_ROTATE 10005
#define ID_ZOOM 10006
#define ID_MOVE 10007
#define ID_SELECT 10008

#ifndef wxCLOSE_BOX
#define wxCLOSE_BOX 0x1000
#endif
#ifndef wxFIXED_MINSIZE
#define wxFIXED_MINSIZE 0
#endif

enum
{
    BREVE_FILEMENU_TOP = 10000, // Must always be first
    BREVE_FILEMENU_NEW,
    BREVE_FILEMENU_OPEN,
    BREVE_FILEMENU_QUIT,
    BREVE_WINDOWMENU_LOG,
    BREVE_WINDOWMENU_INSPECTOR,
    BREVE_FILEMENU_BOTTOM, // Must always be third to last
    BREVE_BREVEMENU_SIM, // Must always be second to last - uses +1500 to +2000
    BREVE_SIMMENU // Must -always- be last - uses this to +1000
};

class BreveCanvas;
class SimInstance;
class LogWindow;
class Inspector;
class wxToggleButton;

class BreveRender: public wxFrame
{    
    DECLARE_CLASS( BreveRender )
    DECLARE_EVENT_TABLE()

private:
    BreveCanvas * canvas;
    wxBitmapButton * stopbutton;
    wxBitmapButton * runbutton;

    wxToggleButton * rotatebutton;
    wxToggleButton * zoombutton;
    wxToggleButton * movebutton;
    wxToggleButton * selectbutton;

    wxChoice * simselect;

    wxBitmap pausebitmap;
    wxBitmap playbitmap;
    wxBitmap stopbitmap;

    wxMenuBar * menubar;
    wxMenu * defsimmenu;

    Inspector * inspector;
    LogWindow * logwindow;
    SimInstance * simlist;
    int cursim;
    int mousemode;

public:
    BreveRender( );
    ~BreveRender();
    BreveRender( wxWindow* parent, wxWindowID id = SYMBOL_BREVERENDER_IDNAME, const wxString& caption = SYMBOL_BREVERENDER_TITLE, const wxPoint& pos = SYMBOL_BREVERENDER_POSITION, const wxSize& size = SYMBOL_BREVERENDER_SIZE, long style = SYMBOL_BREVERENDER_STYLE );

    bool Create( wxWindow* parent, wxWindowID id = SYMBOL_BREVERENDER_IDNAME, const wxString& caption = SYMBOL_BREVERENDER_TITLE, const wxPoint& pos = SYMBOL_BREVERENDER_POSITION, const wxSize& size = SYMBOL_BREVERENDER_SIZE, long style = SYMBOL_BREVERENDER_STYLE );

    void CreateControls();

    void ResetSim(int sim = -2);
    void SetMenu(int mode);

    SimInstance * GetSimulation(int i = -1);
    int GetSimInt(SimInstance * s);
    void KillSimulation(int i = -1);
    void NewSimulation();
    void LoadSimulation(wxString, wxString, wxString);
    void LoadSimFile(wxString ffile);
    void UpdateChoice(int i);

    void OnSimMenu(wxCommandEvent &event);
    void OnSimSelect(wxCommandEvent& event);
    void OnClose(wxCloseEvent& event);
    void OnMenuOpen(wxCommandEvent& event);
    void OnMenuNew(wxCommandEvent& event);
    void OnMenuQuit(wxCommandEvent& event);
    void OnMenuLogWindow(wxCommandEvent& event);
    void OnMenuInspector(wxCommandEvent& event);
    void OnMenuSim(wxCommandEvent &event);
    void OnRenderRunClick( wxCommandEvent& event );
    void OnRenderStopClick( wxCommandEvent& event );
    void OnRotateClick(wxCommandEvent&event);
    void OnZoomClick(wxCommandEvent&event);
    void OnMoveClick(wxCommandEvent&event);
    void OnSelectClick(wxCommandEvent&event);
    void OnKeyUp(wxKeyEvent&event);
    void OnKeyDown(wxKeyEvent&event);
    void OnSize(wxSizeEvent &event);
    void OnMove(wxMoveEvent &event);

    wxBitmap GetBitmapResource( const wxString& name );
    wxIcon GetIconResource( const wxString& name );

    static bool ShowToolTips();

    Inspector * GetInspector()
    {
	return inspector;
    }

    int GetMouseMode()
    {
	return mousemode;
    }

    void queMsg(const char*m)
    {
	canvas->queMsg(m);
    }

    void queCmd(const char*m)
    {
	canvas->queCmd(m);
    }

    void KillLog()
    {
	logwindow = NULL;
    }

    void KillInspector()
    {
	inspector = NULL;
    }

    void AppendLog(const char * text)
    {
	if (logwindow != NULL)
	    logwindow->Append(text);
    }

    void RequestLogWindowReset()
    {
	canvas->RequestLogWindowReset();
    }

    void ResetLog()
    {
	if (logwindow != NULL)
	    logwindow->ResetLog();
    }

    bool MouseDown()
    {
	return canvas->MouseDown();
    }
};

#endif
    // _BREVERENDER_H_
