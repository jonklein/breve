#if defined(__GNUG__) && !defined(__APPLE__)
#pragma implementation "CodeWindow.h"
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
#include "BreveInterface.h"
#include "CodeWindow.h"
#include "CodeCtrl.h"
#include "Main.h"
#include "steve.h"
#include "SimInstance.h"

IMPLEMENT_CLASS( CodeWindow, wxFrame )

BEGIN_EVENT_TABLE( CodeWindow, wxFrame )
    EVT_CLOSE (CodeWindow::OnClose)
    EVT_TEXT_ENTER(ID_CODE_GOTOCTRL, CodeWindow::GotoEvent)
    EVT_IDLE(CodeWindow::OnIdle)
    EVT_CHOICE(ID_CODE_METHCHOICE, CodeWindow::OnSelection)
    EVT_MENU_RANGE(12000, 12017, CodeWindow::OnMenu)
END_EVENT_TABLE()

BEGIN_EVENT_TABLE( MethodChoice, wxChoice )
    EVT_LEFT_DOWN(MethodChoice::OnMouse)
END_EVENT_TABLE()

MethodChoice::MethodChoice(CodeWindow * parent, int id) :
    wxChoice(parent, id)
{
    this->parent = parent;
}

MethodChoice::~MethodChoice()
{
}

void MethodChoice::OnMouse(wxMouseEvent&event)
{
    parent->UpdateChoice();
    SetSelection(0);
    event.Skip();
}

CodeWindow::CodeWindow( )
{
}

CodeWindow::CodeWindow(SimInstance * sim, wxString file, wxString dir, wxString code)
{
    this->sim = sim;
    this->filename = file;
    this->dir = dir;

    Create(NULL);

#ifdef __WXMSW__
    SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_MENU));
#endif

    codectrl->SetText(code);

    codectrl->EmptyUndoBuffer();

    gotoctrl->SetValue("1");

    SetTitle(file);

    Show(TRUE);
}

CodeWindow::~CodeWindow()
{
}

bool CodeWindow::Create( wxWindow* parent, wxWindowID id, const wxString& caption, const wxPoint& pos, const wxSize& size, long style )
{
    wxFrame::Create( parent, id, caption, pos, size, style );

    CreateControls();
    //Centre();

    return TRUE;
}

void CodeWindow::CreateControls()
{    
    wxBoxSizer * sizer = new wxBoxSizer(wxVERTICAL);

    SetSizer(sizer);

    wxBoxSizer * csizer = new wxBoxSizer(wxHORIZONTAL);

#ifdef __WXMSW__
    sizer->Add(csizer, 0, wxEXPAND|wxGROW|wxALIGN_CENTER_HORIZONTAL|wxALL, 0);
#else
    sizer->Add(csizer, 0, wxEXPAND|wxGROW|wxALIGN_CENTER_HORIZONTAL|wxALL, 5);
#endif
    wxStaticText * gtext = new wxStaticText(this, -1, "Go to line:");

#ifdef __WXMSW__
    gtext->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_MENU));
#endif

    csizer->Add(gtext, 0, wxRIGHT|wxALIGN_CENTER_VERTICAL|wxADJUST_MINSIZE, 5);

    gotoctrl = new wxTextCtrl(this, ID_CODE_GOTOCTRL, "", wxDefaultPosition, wxDefaultSize, wxTE_PROCESS_ENTER);

    csizer->Add(gotoctrl, 0, wxLEFT|wxRIGHT|wxALIGN_CENTER_VERTICAL|wxADJUST_MINSIZE, 5);

    methodchoice = new MethodChoice(this, ID_CODE_METHCHOICE);

//    csizer->Add(methodchoice, 1, wxEXPAND|wxALIGN_CENTER_VERTICAL|wxGROW|wxLEFT|wxRIGHT|wxTOP|wxADJUST_MINSIZE, 5);
    csizer->Add(methodchoice, 1, wxALIGN_CENTER_VERTICAL|wxLEFT, 5);

    codectrl = new CodeCtrl(this);

#ifdef __WXMSW__
    sizer->Add(codectrl, 1, wxALIGN_CENTER_VERTICAL|wxEXPAND|wxGROW|wxALL|wxADJUST_MINSIZE, 0);
#else
    sizer->Add(codectrl, 1, wxALIGN_CENTER_VERTICAL|wxEXPAND|wxGROW|wxLEFT|wxRIGHT|wxBOTTOM|wxADJUST_MINSIZE, 5);
#endif

    {
	menubar = new wxMenuBar();

	wxMenu * filemenu = new wxMenu;

	filemenu->Append(12000, "&Save\tCTRL-S");
	filemenu->Append(12001, "Save &as...\tSHIFT-CTRL-S");
	filemenu->Append(12002, "&Close\tCTRL-W");

	menubar->Append(filemenu, "&File");

	wxMenu * editmenu = new wxMenu;

	editmenu->Append(12003, "&Undo\tCTRL-Z");
	editmenu->Append(12004, "&Redo\tShift-CTRL-Z");
	editmenu->AppendSeparator();
	editmenu->Append(12005, "Cu&t\tCTRL-X");
	editmenu->Append(12006, "&Copy\tCTRL-C");
	editmenu->Append(12007, "&Paste\tCTRL-V");
	editmenu->Append(12008, "&Delete\tDelete");
	editmenu->AppendSeparator();
	editmenu->Append(12009, "&Find\tCTRL-F");
	editmenu->Append(12010, "Find &next\tF3");
	editmenu->Append(12011, "&Replace\tCTRL-H");
	editmenu->Append(12012, "Replace &again\tShift-F4");
	editmenu->AppendSeparator();
	editmenu->Append(12013, "&Goto\tCTRL-G");
	editmenu->AppendSeparator();
	/*editmenu->Append(12014, "&Indent increase");
	editmenu->Append(12015, "I&ndent reduce");
	editmenu->AppendSeparator();*/
	editmenu->Append(12016, "Select all\tCTRL-A");
	editmenu->Append(12017, "Select line\tCTRL-L");

	menubar->Append(editmenu, "&Edit");

	SetMenuBar(menubar);
    }

    methodchoice->Append("Go to method...");
    methodchoice->SetSelection(0);
}

wxString CodeWindow::GetCode()
{
    return codectrl->GetText();
}

bool CodeWindow::ShowToolTips()
{
    return TRUE;
}

wxBitmap CodeWindow::GetBitmapResource( const wxString& name )
{
    return wxNullBitmap;
}

wxIcon CodeWindow::GetIconResource( const wxString& name )
{
    return wxNullIcon;
}

void CodeWindow::OnClose(wxCloseEvent& event)
{
    if (event.CanVeto())
    {
	if (codectrl->IsModified())
	{
	    int i;

	    i = wxMessageBox("Are you sure you want to close this file?  You haven't saved your changes.", "Confirmation", wxYES_NO|wxICON_EXCLAMATION);

	    if (i == wxNO)
		return;
	}

	breverender->KillSimulation(breverender->GetSimInt(sim));
	return;
    }

    event.Skip();
}

void CodeWindow::GotoEvent(wxCommandEvent &event)
{
    long val;

    if (gotoctrl->GetValue().ToLong(&val))
    {
	if (val > 0)
	    val--;

	if (val > codectrl->GetLineCount())
	    val = codectrl->GetLineCount();

	val = codectrl->PositionFromLine(val);

	codectrl->SetCurrentPos(val);
	codectrl->SetAnchor(val);
	codectrl->EnsureCaretVisible();
	codectrl->SetFocus();
    }
    else
	gotoctrl->SetSelection(-1,-1);
}

void CodeWindow::OnIdle(wxIdleEvent& event)
{
    if (wxWindow::FindFocus() == codectrl)
    {
	long line;
	wxString str;

	line = codectrl->GetCurrentLine();

	line++;

	str << line;

	gotoctrl->SetValue(str);
    }
}

void CodeWindow::OnSelection(wxCommandEvent& event)
{
    int i = methodchoice->GetSelection();
    int line;

    if (i < 1)
	return;

    line = (int)methodchoice->GetClientData(i);

    line = codectrl->PositionFromLine(line);

    codectrl->SetCurrentPos(line);
    codectrl->SetAnchor(line);
    codectrl->EnsureCaretVisible();
    codectrl->SetFocus();

    methodchoice->SetSelection(0);
}

void CodeWindow::OnMenu(wxCommandEvent &event)
{
    codectrl->OnMenu(event);
}

void CodeWindow::UpdateChoice()
{
    wxChar word[2048];
    wxChar * buf;
    wxChar * str;
    int len, blen;
    int i = 0;
    int inquote = 0;
    wxString inclass, inmethod;
    int line = -1;
    int wloc = 0;
    int inword, atword, mode;

    methodchoice->Clear();

    methodchoice->Append("Go to method...");

    i = codectrl->GetTextLength();

    buf = (wxChar*)malloc(sizeof(wxChar) * (i + 1));
    blen = i + 1;

    strcpy(buf, codectrl->GetText());

    len = -1;

    str = buf;

    while (1)
    {
	str += len + 1;
	line++;

	if (str > buf + blen)
	    break;

	for (i = 0; str[i] != '\0'; i++)
	    if (str[i] == '\n')
		break;

	len = i;

	str[len] = '\0';

	inword = atword = wloc = 0;

	if (inquote)
	    mode = 3;
	else
	    mode = 0;

	i = 0;

	while (i < len)
	{
	    switch (mode)
	    {
		case 0:
		    if (str[i] == ' ' || str[i] == '\t')
			i++;
		    else
			mode = 1;

		    break;

		case 1:
		    if (str[i] == '#')
		    {
			i = len;
			break;
		    }

		    if (str[i] == ':' && atword == 0 && inword == 1)
		    {
			i++;

			while (str[i] == ' ' || str[i] == '\t')
			    i++;

			inclass = "";
			inclass << str + i << " (" << line + 1 << ")";

			methodchoice->Append(inclass, (void*)line);
			i = len;
			break;
		    }

		    if (str[i] == '+' && atword == 0 && inword == 0 && !inclass.IsEmpty())
		    {
			i++;

			while (str[i] == ' ' || str[i] == '\t')
			    i++;

			inmethod = "";
			inmethod << "... " << (str + i) << " (" << line + 1 << ")";

			methodchoice->Append(inmethod, (void*)line);
			i = len;
			break;
		    }

		    if (wloc != 0)
			atword++;

		    mode = 2;
		    inword = 1;
		    wloc = 0;

		case 2:
		    if (str[i] == '#')
		    {
			i = len;
			break;
		    }

		    if (str[i] == ' ' || str[i] == '\t' || (str[i] == ':' && atword == 0))
		    {
			word[wloc] = '\0';
			mode = 0;
			break;
		    }

		    if (str[i] == '"' && (i == 0 || str[i - 1] != '\\'))
		    {
			i++;
			atword++;
			mode = 3;
			inquote = 1;
			break;
		    }

		    word[wloc++] = str[i++];
		    break;

		case 3:
		    if (str[i] == '"' && (i == 0 || str[i - 1] != '\\'))
		    {
			inquote = 0;
			i++;
			mode = 0;
			break;
		    }

		    i++;
		    break;
	    }
	}
    }

    free(buf);
}
