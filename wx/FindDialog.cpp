/////////////////////////////////////////////////////////////////////////////
// Name:        FindDialog.cpp
// Purpose:     
// Author:      
// Modified by: 
// Created:     Sun 13 Mar 2005 06:45:57 PM EST
// RCS-ID:      
// Copyright:   
// Licence:     
/////////////////////////////////////////////////////////////////////////////

#if defined(__GNUG__) && !defined(__APPLE__)
#pragma implementation "FindDialog.h"
#endif

// For compilers that support precompilation, includes "wx/wx.h".
#include "wx/wxprec.h"

#ifdef __BORLANDC__
#pragma hdrstop
#endif

#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif

#include "CodeCtrl.h"
#include "FindDialog.h"

IMPLEMENT_DYNAMIC_CLASS( FindDialog, wxDialog )

BEGIN_EVENT_TABLE( FindDialog, wxDialog )
    EVT_TEXT_ENTER( ID_FIND_FINDCTRL, FindDialog::OnFindFindctrlEnter )
    EVT_TEXT_ENTER( ID_FIND_REPLACECTRL, FindDialog::OnFindReplacectrlEnter )
    EVT_BUTTON( ID_FIND_SEARCH, FindDialog::OnFindSearchClick )
    EVT_BUTTON( ID_FIND_CANCEL, FindDialog::OnFindCancelClick )
END_EVENT_TABLE()

FindDialog::FindDialog( )
{
}

FindDialog::FindDialog( wxWindow* parent, int mode, wxWindowID id, const wxString& caption, const wxPoint& pos, const wxSize& size, long style )
{
    Create(parent, mode, id, caption, pos, size, style);
}

bool FindDialog::Create( wxWindow* parent, int mode, wxWindowID id, const wxString& caption, const wxPoint& pos, const wxSize& size, long style )
{
    FindCtrl = NULL;
    ReplaceCtrl = NULL;
    MatchCheckBox = NULL;
    ButtonSearch = NULL;
    ButtonCancel = NULL;

    this->mode = mode;

    SetExtraStyle(GetExtraStyle()|wxWS_EX_BLOCK_EVENTS);
    wxDialog::Create( parent, id, caption, pos, size, style );

    CreateControls();
    GetSizer()->Fit(this);
    GetSizer()->SetSizeHints(this);
    Centre();

    lastflags = 0;
    setanchor = 0;

    return TRUE;
}

void FindDialog::CreateControls()
{    
    FindDialog* itemDialog1 = this;

    wxBoxSizer* itemBoxSizer2 = new wxBoxSizer(wxHORIZONTAL);
    itemDialog1->SetSizer(itemBoxSizer2);

    wxBoxSizer* itemBoxSizer3 = new wxBoxSizer(wxVERTICAL);
    itemBoxSizer2->Add(itemBoxSizer3, 0, 0, 0);

    wxFlexGridSizer* itemFlexGridSizer4 = new wxFlexGridSizer(0, 2, 0, 0);
    itemBoxSizer3->Add(itemFlexGridSizer4, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);

    wxStaticText* itemStaticText5 = new wxStaticText( itemDialog1, wxID_STATIC, _("Find:"), wxDefaultPosition, wxDefaultSize, 0 );
    itemFlexGridSizer4->Add(itemStaticText5, 0, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL|wxALL|wxADJUST_MINSIZE, 5);

    FindCtrl = new wxTextCtrl( itemDialog1, ID_FIND_FINDCTRL, _T(""), wxDefaultPosition, wxSize(175, -1), wxTE_PROCESS_ENTER );
    itemFlexGridSizer4->Add(FindCtrl, 1, wxGROW|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    if (mode == 1)
    {
	wxStaticText* itemStaticText7 = new wxStaticText( itemDialog1, wxID_STATIC, _("Replace:"), wxDefaultPosition, wxDefaultSize, 0 );
	itemFlexGridSizer4->Add(itemStaticText7, 0, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL|wxALL|wxADJUST_MINSIZE, 5);

	ReplaceCtrl = new wxTextCtrl( itemDialog1, ID_FIND_REPLACECTRL, _T(""), wxDefaultPosition, wxSize(150, -1), wxTE_PROCESS_ENTER );
	itemFlexGridSizer4->Add(ReplaceCtrl, 1, wxGROW|wxALIGN_CENTER_VERTICAL|wxALL, 5);
    }

    wxFlexGridSizer* itemFlexGridSizer9 = new wxFlexGridSizer(2, 2, 0, 0);
    itemBoxSizer3->Add(itemFlexGridSizer9, 0, wxALIGN_LEFT|wxALL, 5);

    MatchCheckBox = new wxCheckBox( itemDialog1, ID_FIND_MATCHCASE, _("Match Case"), wxDefaultPosition, wxDefaultSize, 0 );
    MatchCheckBox->SetValue(FALSE);
    itemFlexGridSizer9->Add(MatchCheckBox, 0, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxBoxSizer* itemBoxSizer11 = new wxBoxSizer(wxVERTICAL);
    itemBoxSizer2->Add(itemBoxSizer11, 0, wxALIGN_TOP|wxALL, 5);

    ButtonSearch = new wxButton( itemDialog1, ID_FIND_SEARCH, mode == 0 ? _("Search") : _("Replace"), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer11->Add(ButtonSearch, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);

    ButtonCancel = new wxButton( itemDialog1, ID_FIND_CANCEL, _("Cancel"), wxDefaultPosition, wxDefaultSize, 0 );
    ButtonCancel->SetDefault();
    itemBoxSizer11->Add(ButtonCancel, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);
}

void FindDialog::OnFindFindctrlEnter( wxCommandEvent& event )
{
    OnFindSearchClick(event);
}

void FindDialog::OnFindReplacectrlEnter( wxCommandEvent& event )
{
    OnFindSearchClick(event);
}

void FindDialog::OnFindSearchClick( wxCommandEvent& event )
{
    int i = 0, n = 0;

    if (FindCtrl->GetValue().Length() < 1)
    {
	wxMessageBox("No text has been entered.");
	return;
    }

    if (setanchor == 1 && FindCtrl->GetValue() != lastsearch)
	setanchor = 0;

    if (setanchor == 0)
    {
	setanchor = 1;

	codectrl->GetSelection(&i, &n);

	//printf("%d %d\n\r", i, n);

	//if (i == n)
	codectrl->SetSelection(0, codectrl->GetTextLength());

	codectrl->SearchAnchor();

	codectrl->SetSelection(i, n);
    }

    if (mode == 1)
	lastreplace = ReplaceCtrl->GetValue();

    lastflags = MatchCheckBox->GetValue();

    lastsearch = FindCtrl->GetValue();

    i = codectrl->SearchNext(lastflags, lastsearch);

    if (i == -1)
	wxBell();
    else
    {
	codectrl->SetSelection(i + lastsearch.Length(), codectrl->GetTextLength());
	codectrl->SearchAnchor();

	codectrl->SetSelection(i, i + lastsearch.Length());

	if (mode == 1)
	{
	    codectrl->ReplaceSelection(lastreplace);

	    codectrl->SetSelection(i, i + lastreplace.Length());
	}

	codectrl->EnsureCaretVisible();
    }
}

void FindDialog::OnFindCancelClick( wxCommandEvent& event )
{
    Close();
}

bool FindDialog::ShowToolTips()
{
    return TRUE;
}

wxBitmap FindDialog::GetBitmapResource( const wxString& name )
{
    return wxNullBitmap;
}

wxIcon FindDialog::GetIconResource( const wxString& name )
{
    return wxNullIcon;
}
