#ifndef _LOGWINDOW_H_
#define _LOGWINDOW_H_

#if defined(__GNUG__) && !defined(__APPLE__)
#pragma interface "LogWindow.cpp"
#endif

#include "wx/frame.h"
#include "LogWindow.h"

#define ID_LOG_FRAME 10005
#define ID_LOG_CMDINPUT 10098
#define ID_LOG_CMDBUTTON 10099
#define SYMBOL_LOGWINDOW_STYLE wxCAPTION|wxRESIZE_BORDER|wxSYSTEM_MENU|wxCLOSE_BOX
#define SYMBOL_LOGWINDOW_TITLE _("Log Window")
#define SYMBOL_LOGWINDOW_IDNAME ID_LOG_FRAME
#define SYMBOL_LOGWINDOW_SIZE wxSize(500, 150)
#define SYMBOL_LOGWINDOW_POSITION wxDefaultPosition

#ifndef wxCLOSE_BOX
#define wxCLOSE_BOX 0x1000
#endif
#ifndef wxFIXED_MINSIZE
#define wxFIXED_MINSIZE 0
#endif

class LogWindow: public wxFrame
{    
    DECLARE_CLASS( LogWindow )
    DECLARE_EVENT_TABLE()

private:
    wxTextCtrl * textctrl;
    wxButton * cmdbutton;
    wxTextCtrl * cmdinput;

public:
    LogWindow( );
    ~LogWindow();
    LogWindow( wxWindow* parent, wxWindowID id = SYMBOL_LOGWINDOW_IDNAME, const wxString& caption = SYMBOL_LOGWINDOW_TITLE, const wxPoint& pos = SYMBOL_LOGWINDOW_POSITION, const wxSize& size = SYMBOL_LOGWINDOW_SIZE, long style = SYMBOL_LOGWINDOW_STYLE );
    bool Create( wxWindow* parent, wxWindowID id = SYMBOL_LOGWINDOW_IDNAME, const wxString& caption = SYMBOL_LOGWINDOW_TITLE, const wxPoint& pos = SYMBOL_LOGWINDOW_POSITION, const wxSize& size = SYMBOL_LOGWINDOW_SIZE, long style = SYMBOL_LOGWINDOW_STYLE );

    void Append(const char * text);
    void ResetLog();

    void OnClose(wxCloseEvent& event);
    void RunEvent(wxCommandEvent& event);

    void CreateControls();
    wxBitmap GetBitmapResource( const wxString& name );
    wxIcon GetIconResource( const wxString& name );
    static bool ShowToolTips();
};

#endif
    // _LOGWINDOW_H_
