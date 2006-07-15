#if defined(__GNUG__) && !defined(__APPLE__)
#pragma implementation "LogWindow.h"
#endif

// For compilers that support precompilation, includes "wx/wx.h".
#include "wx/wxprec.h"

#ifdef __BORLANDC__
#pragma hdrstop
#endif

#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif

#include <wx/file.h>

#include "BreveRender.h"
#include "BreveCanvas.h"
#include "LogWindow.h"
#include "BreveInterface.h"
#include "Main.h"
#include "steve.h"
#include "SimInstance.h"

#include <wx/confbase.h>

IMPLEMENT_CLASS( LogWindow, wxFrame )

BEGIN_EVENT_TABLE( LogWindow, wxFrame )
    EVT_CLOSE (LogWindow::OnClose)
    EVT_TEXT_ENTER(ID_LOG_CMDINPUT, LogWindow::RunEvent)
    EVT_BUTTON(ID_LOG_CMDBUTTON, LogWindow::RunEvent)
    EVT_SIZE(LogWindow::OnSize)
    EVT_MOVE(LogWindow::OnMove)
END_EVENT_TABLE()

LogWindow::LogWindow( )
{
}

LogWindow::LogWindow( wxWindow* parent, wxWindowID id, const wxString& caption, const wxPoint& pos, const wxSize& size, long style )
{
    Create( parent, id, caption, pos, size, style );
}

LogWindow::~LogWindow()
{
}

bool LogWindow::Create( wxWindow* parent, wxWindowID id, const wxString& caption, const wxPoint& pos, const wxSize& size, long style )
{
    wxPoint tpoint;
    wxSize tsize;
    bool loaded = FALSE;

    tpoint = pos;
    tsize = size;

    wxConfigBase * config = wxConfigBase::Get();

    if (config != NULL)
    {
	int x, y, w, h;

	if (config->Read("LogWindowX", &x) && config->Read("LogWindowY", &y) &&
	    config->Read("LogWindowWidth", &w) && config->Read("LogWindowHeight", &h))
	{
	    tpoint.x = x;
	    tpoint.y = y;
	    tsize.SetHeight(h);
	    tsize.SetWidth(w);
	    loaded = TRUE;
	}
    }

    wxFrame::Create( parent, id, caption, tpoint, tsize, style );

#ifdef __WXMSW__
    SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_MENU));
#endif

    CreateControls();
    //Centre();

    if (loaded)
    {
	Move(tpoint);
	SetSize(tsize);
    }

    Show(TRUE);
    return TRUE;
}

void LogWindow::CreateControls()
{    
    wxBoxSizer * sizer = new wxBoxSizer(wxVERTICAL);

    SetSizer(sizer);

    textctrl = new wxTextCtrl(this, -1, "", wxDefaultPosition, wxDefaultSize, wxTE_RICH2 | wxTE_MULTILINE | wxTE_READONLY | wxALWAYS_SHOW_SB | wxVSCROLL);

    sizer->Add(textctrl, 1, wxEXPAND|wxGROW|wxALL|wxADJUST_MINSIZE, 5);

    wxBoxSizer * csizer = new wxBoxSizer(wxHORIZONTAL);

    sizer->Add(csizer, 0, wxEXPAND|wxGROW|wxALIGN_CENTER_HORIZONTAL|wxALL, 0);

    cmdinput = new wxTextCtrl(this, ID_LOG_CMDINPUT, "", wxDefaultPosition, wxDefaultSize, wxTE_PROCESS_ENTER);

    csizer->Add(cmdinput, 1, wxEXPAND|wxGROW|wxALL|wxADJUST_MINSIZE, 5);

    cmdbutton = new wxButton(this, ID_LOG_CMDBUTTON, "Run");

    csizer->Add(cmdbutton, 0, wxALL|wxADJUST_MINSIZE, 5);
}

bool LogWindow::ShowToolTips()
{
    return TRUE;
}

wxBitmap LogWindow::GetBitmapResource( const wxString& name )
{
    return wxNullBitmap;
}

wxIcon LogWindow::GetIconResource( const wxString& name )
{
    return wxNullIcon;
}

void LogWindow::OnClose(wxCloseEvent& event)
{
    if (event.CanVeto())
    {
	Hide();
	return;
    }

    gBreverender->KillLog();
    event.Skip();
}

void LogWindow::Append(const char * text)
{
    textctrl->AppendText(text);
    textctrl->ShowPosition(textctrl->GetLastPosition() - 1);
    gBreverender->RequestLogWindowReset();
}

void LogWindow::ResetLog()
{
    textctrl->ShowPosition(textctrl->GetLastPosition() - 1);
}

void LogWindow::RunEvent(wxCommandEvent &event)
{
    wxString str;

    str = cmdinput->GetValue();

    cmdinput->SetSelection(-1, -1);

    if (str.Length() < 1 || gBreverender->GetSimulation() == NULL || !gBreverender->GetSimulation()->GetInterface()->Initialized())
	return;

    gBreverender->GetSimulation()->GetInterface()->ExecuteCommand(str);
}


void LogWindow::OnSize(wxSizeEvent &event)
{
    if (!IsMaximized())
    {
	wxConfigBase * config = wxConfigBase::Get();

	if (config != NULL)
	{
	    config->Write("LogWindowWidth", event.GetSize().GetWidth());
	    config->Write("LogWindowHeight", event.GetSize().GetHeight());
	}
    }
    
    event.Skip();
}

void LogWindow::OnMove(wxMoveEvent &event)
{
    {
	wxConfigBase * config = wxConfigBase::Get();

	if (config != NULL)
	{
	    config->Write("LogWindowX", event.GetPosition().x);
	    config->Write("LogWindowY", event.GetPosition().y);
	}
    }
    
    event.Skip();
}

