/////////////////////////////////////////////////////////////////////////////
// Name:        BDialog.cpp
// Purpose:     
// Author:      
// Modified by: 
// Created:     Tue 15 Mar 2005 08:44:05 PM EST
// RCS-ID:      
// Copyright:   
// Licence:     
/////////////////////////////////////////////////////////////////////////////

#if defined(__GNUG__) && !defined(__APPLE__)
#pragma implementation "BDialog.h"
#endif

// For compilers that support precompilation, includes "wx/wx.h".
#include "wx/wxprec.h"

#ifdef __BORLANDC__
#pragma hdrstop
#endif

#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif

#include "BDialog.h"

IMPLEMENT_DYNAMIC_CLASS( BDialog, wxDialog )

BEGIN_EVENT_TABLE( BDialog, wxDialog )
    EVT_BUTTON( ID_BD_YES, BDialog::OnBdYesClick )
    EVT_BUTTON( ID_BD_NO, BDialog::OnBdNoClick )
END_EVENT_TABLE()

BDialog::BDialog( )
{
}

BDialog::BDialog( wxWindow* parent, char * title, char * message, char * b1, char * b2)
{
    this->title = title;
    this->message = message;
    this->b1 = b1;
    this->b2 = b2;
    Create(parent);
}

bool BDialog::Create( wxWindow* parent, wxWindowID id, const wxString& caption, const wxPoint& pos, const wxSize& size, long style )
{
    SetExtraStyle(GetExtraStyle()|wxWS_EX_BLOCK_EVENTS);
    wxDialog::Create( parent, id, title, pos, size, style );

    CreateControls();
    GetSizer()->Fit(this);
    GetSizer()->SetSizeHints(this);
    Centre();

    return TRUE;
}

void BDialog::CreateControls()
{    
    BDialog* itemDialog1 = this;

    wxBoxSizer* itemBoxSizer2 = new wxBoxSizer(wxVERTICAL);
    itemDialog1->SetSizer(itemBoxSizer2);

    wxStaticText* itemStaticText3 = new wxStaticText( itemDialog1, -1, message, wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer2->Add(itemStaticText3, 0, wxALIGN_CENTER_HORIZONTAL|wxALL|wxADJUST_MINSIZE, 5);

    wxBoxSizer* itemBoxSizer4 = new wxBoxSizer(wxHORIZONTAL);
    itemBoxSizer2->Add(itemBoxSizer4, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);

    wxButton* itemButton6 = new wxButton( itemDialog1, ID_BD_NO, b2, wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer4->Add(itemButton6, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxButton* itemButton5 = new wxButton( itemDialog1, ID_BD_YES, b1, wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer4->Add(itemButton5, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    SetReturnCode(0);
}

bool BDialog::ShowToolTips()
{
    return TRUE;
}

wxBitmap BDialog::GetBitmapResource( const wxString& name )
{
    return wxNullBitmap;
}

wxIcon BDialog::GetIconResource( const wxString& name )
{
    return wxNullIcon;
}
void BDialog::OnBdYesClick( wxCommandEvent& event )
{
    EndModal(0);
}

void BDialog::OnBdNoClick( wxCommandEvent& event )
{
    EndModal(1);
}


