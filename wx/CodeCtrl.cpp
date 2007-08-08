#if defined(__GNUG__) && !defined(__APPLE__)
#pragma implementation "CodeCtrl.h"
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
#include "Main.h"
#include "CodeWindow.h"
#include "CodeCtrl.h"
#include "FindDialog.h"
#include "steve.h"
#include "SimInstance.h"

IMPLEMENT_CLASS( CodeCtrl, wxStyledTextCtrl )

BEGIN_EVENT_TABLE( CodeCtrl, wxStyledTextCtrl )
    EVT_STC_MARGINCLICK (wxID_ANY,     CodeCtrl::OnMarginClick)
    EVT_MENU_RANGE(12000, 12017, CodeCtrl::OnMenu)
    EVT_STC_CHARADDED (wxID_ANY, CodeCtrl::OnChar)
END_EVENT_TABLE()

CodeCtrl::CodeCtrl(CodeWindow * parent) :
    wxStyledTextCtrl(parent, -1, wxDefaultPosition, wxDefaultSize, wxSUNKEN_BORDER|wxVSCROLL)
{
    this->parent = parent;

    StyleClearAll();

    SetLexer(wxSTC_LEX_CPP);

    StyleSetForeground (wxSTC_STYLE_LINENUMBER, wxColour (_T("DARK GREY"))); 
    StyleSetBackground (wxSTC_STYLE_LINENUMBER, wxColour (_T("WHITE")));

    int Nr;
    for (Nr = 0; Nr < wxSTC_STYLE_LASTPREDEFINED; Nr++) {
        wxFont font (10, wxMODERN, wxNORMAL, wxNORMAL);
        StyleSetFont (Nr, font);
    }

    StyleSetForeground (wxSTC_STYLE_DEFAULT, wxColour (_T("DARK GREY")));
    StyleSetForeground (wxSTC_STYLE_INDENTGUIDE, wxColour (_T("DARK GREY")));

    StyleSetForeground(wxSTC_C_PREPROCESSOR, wxColour("GREY")); // Preprocessor

    StyleSetForeground(wxSTC_C_STRING, wxColour("FOREST GREEN")); //Comments
    StyleSetForeground(wxSTC_C_STRINGEOL, wxColour("FOREST GREEN")); //Comments

    StyleSetForeground(wxSTC_C_NUMBER, *wxBLUE); // Numbers

    StyleSetForeground(wxSTC_C_WORD, *wxRED); // Special words

    StyleSetForeground(wxSTC_C_OPERATOR, wxColour("GREY"));

    SetKeyWords(0, "int ints object objects double doubles float floats vector vectors string strings hash hashes list lists matrix matrices");

    SetMarginType(2, wxSTC_MARGIN_SYMBOL);
    SetMarginMask(2, wxSTC_MASK_FOLDERS);
    StyleSetBackground (2, wxColour (_T("WHITE")));
    SetMarginWidth (2, 16);
    SetMarginSensitive (2, 1);

    SetProperty("fold", "1");
    SetProperty("fold.compact", "1");
    SetProperty("fold.preprocessor", "1");

    SetFoldFlags (wxSTC_FOLDFLAG_LINEBEFORE_CONTRACTED |
                  wxSTC_FOLDFLAG_LINEAFTER_CONTRACTED);

    SetTabWidth(4);
    SetUseTabs(true);
    SetTabIndents(true);
    SetBackSpaceUnIndents(true);
    SetIndent(4);

    //SetEdgeMode(wxSTC_EDGE_NONE);

    //SetMarginType(1, wxSTC_MARGIN_SYMBOL);

    SetMargins(0,0);
    SetMarginWidth(0, 0);
    SetMarginWidth(1, 0);
    //SetMarginWidth(2, 0);

    MarkerDefine (wxSTC_MARKNUM_FOLDER,        wxSTC_MARK_DOTDOTDOT, _T("BLACK"), _T("BLACK"));
    MarkerDefine (wxSTC_MARKNUM_FOLDEROPEN,    wxSTC_MARK_ARROWDOWN, _T("BLACK"), _T("BLACK"));
    MarkerDefine (wxSTC_MARKNUM_FOLDERSUB,     wxSTC_MARK_EMPTY,     _T("BLACK"), _T("BLACK"));
    MarkerDefine (wxSTC_MARKNUM_FOLDEREND,     wxSTC_MARK_DOTDOTDOT, _T("BLACK"), _T("WHITE"));
    MarkerDefine (wxSTC_MARKNUM_FOLDEROPENMID, wxSTC_MARK_ARROWDOWN, _T("BLACK"), _T("WHITE"));
    MarkerDefine (wxSTC_MARKNUM_FOLDERMIDTAIL, wxSTC_MARK_EMPTY,     _T("BLACK"), _T("BLACK"));
    MarkerDefine (wxSTC_MARKNUM_FOLDERTAIL,    wxSTC_MARK_EMPTY,     _T("BLACK"), _T("BLACK"));
}

CodeCtrl::~CodeCtrl()
{
}

// From wxStyledTextCtrl sample
void CodeCtrl::OnChar(wxStyledTextEvent &event)
{
    int currentLine = GetCurrentLine();

    if (event.GetKey() == '\n') {
        int lineInd = 0;

        if (currentLine > 0)
            lineInd = GetLineIndentation(currentLine - 1);

        if (lineInd == 0)
	    return;

        SetLineIndentation (currentLine, lineInd);
        GotoPos(PositionFromLine (currentLine) + lineInd);
    }
}

void CodeCtrl::OnMarginClick(wxStyledTextEvent &event)
{
    if (event.GetMargin() == 2)
    {
	int lineClick = LineFromPosition (event.GetPosition());
	int levelClick = GetFoldLevel (lineClick);

	if ((levelClick & wxSTC_FOLDLEVELHEADERFLAG) > 0)
	{
	    ToggleFold (lineClick);
	}
    }
}

bool CodeCtrl::IsModified()
{
    if (gBreverender->GetSimulation() != NULL &&
	gBreverender->GetSimulation()->GetOriginCode().IsSameAs(GetText()))
	return FALSE;

    return TRUE;
}

void CodeCtrl::OnMenu(wxCommandEvent &event)
{
    switch (event.m_id)
    {
	case 12000: // Save
	    if (!parent->GetSim()->GetDir().IsEmpty())
	    {
		wxString t;
		wxString text;
		wxFile file;

		t << parent->GetSim()->GetDir() << FILE_SEP_PATH << parent->GetSim()->GetFilename();

		file.Open(t, wxFile::write);

		if (!file.IsOpened())
		{
		    text << "Failed to open " << t << " for writing.";
		    wxMessageBox(text);
		    return;
		}

		file.Write(GetText(), strlen(GetText()));

		parent->GetSim()->UpdateFile(parent->GetSim()->GetFilename(), parent->GetSim()->GetDir(), GetText());

		break;
	    }

	case 12001: // Save as
	   {
	    wxFileDialog d(parent, "Please enter a filename", "", "", "steve files (*.tz)|*.tz|Python files (*.py)|*.py", wxSAVE | wxOVERWRITE_PROMPT);

	    if (d.ShowModal() == wxID_OK)
	    {
		wxString t;
		wxString text;
		wxFile file;

		t << d.GetDirectory() << FILE_SEP_PATH << d.GetFilename();

		file.Open(t, wxFile::write);

		if (!file.IsOpened())
		{
		    text << "Failed to open " << t << " for writing.";
		    wxMessageBox(text);
		    return;
		}

		file.Write(GetText(), strlen(GetText()));

		parent->GetSim()->UpdateFile(d.GetFilename(), d.GetDirectory(), GetText());

		parent->SetTitle(d.GetFilename());
	    }

	    break;
	   }

	case 12002: // Close
	    parent->Close();
	    break;

	case 12003: // Undo
	    if (CanUndo())
		Undo();
	    break;

	case 12004: // Redo
	    if (CanRedo())
		Redo();
	    break;

	case 12005: // Cut
	    if (GetSelectionEnd()-GetSelectionStart() > 0)
		Cut();
	    break;

	case 12006: // Copy
	    if (GetSelectionEnd()-GetSelectionStart() > 0)
		Copy();
	    break;

	case 12007: // Paste
	    if (CanPaste())
		Paste();
	    break;

	case 12008: // Delete
	    Clear();
	    break;

	case 12009: // Find
	    {
		FindDialog d(parent, 0);

		d.SetCodeCtrl(this);

		d.ShowModal();

		if (d.Searched())
		    lastsearch = d.LastSearch();

		lastreplace = "";

		lastflags = d.LastFlags();
	    }
	    break;

	case 12010: // Find next
	    if (lastsearch.IsEmpty())
		return;

	    {
		int i = 0;

		i = SearchNext(lastflags, lastsearch);

		if (i == -1)
		    wxBell();
		else
		{
		    SetSelection(i + lastsearch.Length(), GetTextLength());
		    SearchAnchor();

		    SetAnchor(i);
		    SetCurrentPos(i + lastsearch.Length());
		    SetFocus();
		}

		EnsureCaretVisible();
	    }

	    break;

	case 12011: // Replace
	    {
		FindDialog d(parent, 1);

		d.SetCodeCtrl(this);

		d.ShowModal();

		if (d.Searched())
		    lastsearch = d.LastSearch();

		if (d.Replaced())
		    lastreplace = d.LastReplace();

		lastflags = d.LastFlags();
	    }
	    break;

	case 12012: // Replace again
	    if (lastsearch.IsEmpty() || lastreplace.IsEmpty())
		return;

	    {
		int i = 0;

		i = SearchNext(lastflags, lastsearch);

		if (i == -1)
		    wxBell();
		else
		{
		    SetSelection(i + lastsearch.Length(), GetTextLength());
		    SearchAnchor();

		    SetAnchor(i);
		    SetCurrentPos(i + lastsearch.Length());

		    ReplaceSelection(lastreplace);

		    SetAnchor(i);
		    SetCurrentPos(i + lastreplace.Length());

		    EnsureCaretVisible();
		    SetFocus();
		}
	    }
	    break;

	case 12013: // Goto
	    {
		wxString t;
		long l;
		t = ::wxGetTextFromUser("Enter a line number:", "Goto line...");

		if (!t.IsEmpty() && t.ToLong(&l))
		{
		    l--;
		    ScrollToLine(l);

		    int lineStart = PositionFromLine (l);

		    SetAnchor(lineStart);
		    SetCurrentPos(lineStart);
		    SetFocus();

		    EnsureCaretVisible();
		}
	    }
	    break;

	case 12014: // Indent increase
	    CmdKeyExecute (wxSTC_CMD_TAB);
	    break;

	case 12015: // Indent reduce
	    CmdKeyExecute (wxSTC_CMD_DELETEBACK);
	    break;

	case 12016: // Select all
	    SetSelection (0, GetTextLength ());
	    break;

	case 12017: // Select line
	{
	    int lineStart = PositionFromLine (GetCurrentLine());
	    int lineEnd = PositionFromLine (GetCurrentLine() + 1);
	    SetSelection (lineStart, lineEnd);
	    break;
	}

	default:
	    printf("Unknown menu %d\n\r", event.m_id);
	    break;
    }
}


