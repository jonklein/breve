/////////////////////////////////////////////////////////////////////////////
// Name:        BDialog.h
// Purpose:     
// Author:      
// Modified by: 
// Created:     Tue 15 Mar 2005 08:44:05 PM EST
// RCS-ID:      
// Copyright:   
// Licence:     
/////////////////////////////////////////////////////////////////////////////

#ifndef _BDIALOG_H_
#define _BDIALOG_H_

#if defined(__GNUG__) && !defined(__APPLE__)
#pragma interface "BDialog.cpp"
#endif

#define ID_BD_DIALOG 13000
#define SYMBOL_BDIALOG_STYLE wxCAPTION|wxRESIZE_BORDER|wxSYSTEM_MENU|wxCLOSE_BOX
#define SYMBOL_BDIALOG_TITLE _("BDialog Title")
#define SYMBOL_BDIALOG_IDNAME ID_BD_DIALOG
#define SYMBOL_BDIALOG_SIZE wxSize(400, 300)
#define SYMBOL_BDIALOG_POSITION wxDefaultPosition
#define ID_BD_YES 10001
#define ID_BD_NO 10002

#ifndef wxCLOSE_BOX
#define wxCLOSE_BOX 0x1000
#endif
#ifndef wxFIXED_MINSIZE
#define wxFIXED_MINSIZE 0
#endif

class BDialog: public wxDialog
{    
    DECLARE_DYNAMIC_CLASS( BDialog )
    DECLARE_EVENT_TABLE()

private:
    char * title, *message, *b1, *b2;

public:
    BDialog( );
    BDialog( wxWindow* parent, char * title, char * message, char * b1, char * b2);

    bool Create( wxWindow* parent, wxWindowID id = SYMBOL_BDIALOG_IDNAME, const wxString& caption = SYMBOL_BDIALOG_TITLE, const wxPoint& pos = SYMBOL_BDIALOG_POSITION, const wxSize& size = SYMBOL_BDIALOG_SIZE, long style = SYMBOL_BDIALOG_STYLE );

    void CreateControls();

    void OnBdYesClick( wxCommandEvent& event );
    void OnBdNoClick( wxCommandEvent& event );

    wxBitmap GetBitmapResource( const wxString& name );
    wxIcon GetIconResource( const wxString& name );
    static bool ShowToolTips();
};

#endif
    // _BDIALOG_H_
