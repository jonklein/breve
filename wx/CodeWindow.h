#ifndef _CODEWINDOW_H_
#define _CODEWINDOW_H_

#if defined(__GNUG__) && !defined(__APPLE__)
#pragma interface "CodeWindow.cpp"
#endif

#include "wx/frame.h"

#define ID_CODE_FRAME 10009
#define ID_CODE_TEXTCTRL 10097
#define ID_CODE_METHCHOICE 10098
#define ID_CODE_GOTOCTRL 10099
#define SYMBOL_CODEWINDOW_STYLE wxCAPTION|wxRESIZE_BORDER|wxSYSTEM_MENU|wxCLOSE_BOX
#define SYMBOL_CODEWINDOW_TITLE _("Code Window")
#define SYMBOL_CODEWINDOW_IDNAME ID_CODE_FRAME
#define SYMBOL_CODEWINDOW_SIZE wxSize(750, 500)
#define SYMBOL_CODEWINDOW_POSITION wxDefaultPosition

#ifndef wxCLOSE_BOX
#define wxCLOSE_BOX 0x1000
#endif
#ifndef wxFIXED_MINSIZE
#define wxFIXED_MINSIZE 0
#endif

class CodeWindow;

class MethodChoice : public wxChoice
{
    DECLARE_EVENT_TABLE()

    private:
	CodeWindow * parent;

    public:
	MethodChoice(CodeWindow * parent, int id);
	~MethodChoice();

	void OnMouse(wxMouseEvent&event);
};

class CodeCtrl;

class CodeWindow: public wxFrame
{    
    DECLARE_CLASS( CodeWindow )
    DECLARE_EVENT_TABLE()

private:
    CodeCtrl * codectrl;
    wxTextCtrl * gotoctrl;
    MethodChoice * methodchoice;
    wxString filename;
    wxString dir;
    SimInstance * sim;
    wxArrayString linedata;
    wxMenuBar * menubar;

public:
    CodeWindow( );
    ~CodeWindow();

    CodeWindow(SimInstance * sim, wxString file, wxString dir, wxString code);
    bool Create( wxWindow* parent, wxWindowID id = SYMBOL_CODEWINDOW_IDNAME, const wxString& caption = SYMBOL_CODEWINDOW_TITLE, const wxPoint& pos = SYMBOL_CODEWINDOW_POSITION, const wxSize& size = SYMBOL_CODEWINDOW_SIZE, long style = SYMBOL_CODEWINDOW_STYLE );

    void OnIdle(wxIdleEvent& event);
    void OnClose(wxCloseEvent& event);
    void GotoEvent(wxCommandEvent& event);
    void OnEdit(wxCommandEvent& event);
    void OnSelection(wxCommandEvent& event);
    void OnMenu(wxCommandEvent &event);

    wxString GetCode();
    void UpdateChoice();

    SimInstance * GetSim()
    {
	return sim;
    }

    void CreateControls();
    wxBitmap GetBitmapResource( const wxString& name );
    wxIcon GetIconResource( const wxString& name );
    static bool ShowToolTips();
};

#endif
    // _CODEWINDOW_H_
