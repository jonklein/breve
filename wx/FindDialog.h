/////////////////////////////////////////////////////////////////////////////
// Name:        FindDialog.h
// Purpose:     
// Author:      
// Modified by: 
// Created:     Sun 13 Mar 2005 06:45:57 PM EST
// RCS-ID:      
// Copyright:   
// Licence:     
/////////////////////////////////////////////////////////////////////////////

#ifndef _FINDDIALOG_H_
#define _FINDDIALOG_H_

#if defined(__GNUG__) && !defined(__APPLE__)
#pragma interface "FindDialog.cpp"
#endif

#define ID_DIALOG 10000
#define SYMBOL_FINDDIALOG_STYLE wxCAPTION|wxRESIZE_BORDER|wxSYSTEM_MENU|wxCLOSE_BOX
#define SYMBOL_FINDDIALOG_TITLE _("Find Dialog")
#define SYMBOL_FINDDIALOG_IDNAME ID_DIALOG
#define SYMBOL_FINDDIALOG_SIZE wxSize(400, 300)
#define SYMBOL_FINDDIALOG_POSITION wxDefaultPosition
#define ID_FIND_FINDCTRL 10001
#define ID_FIND_REPLACECTRL 10002
#define ID_FIND_MATCHCASE 10003
#define ID_FIND_SEARCH 10004
#define ID_FIND_CANCEL 10005

#ifndef wxCLOSE_BOX
#define wxCLOSE_BOX 0x1000
#endif
#ifndef wxFIXED_MINSIZE
#define wxFIXED_MINSIZE 0
#endif

class FindDialog: public wxDialog
{    
    DECLARE_DYNAMIC_CLASS( FindDialog )
    DECLARE_EVENT_TABLE()

public:
    FindDialog( );
    FindDialog( wxWindow* parent, int mode, wxWindowID id = SYMBOL_FINDDIALOG_IDNAME, const wxString& caption = SYMBOL_FINDDIALOG_TITLE, const wxPoint& pos = SYMBOL_FINDDIALOG_POSITION, const wxSize& size = SYMBOL_FINDDIALOG_SIZE, long style = SYMBOL_FINDDIALOG_STYLE );

    bool Create( wxWindow* parent, int mode, wxWindowID id = SYMBOL_FINDDIALOG_IDNAME, const wxString& caption = SYMBOL_FINDDIALOG_TITLE, const wxPoint& pos = SYMBOL_FINDDIALOG_POSITION, const wxSize& size = SYMBOL_FINDDIALOG_SIZE, long style = SYMBOL_FINDDIALOG_STYLE );

    void CreateControls();
    void OnFindFindctrlEnter( wxCommandEvent& event );
    void OnFindReplacectrlEnter( wxCommandEvent& event );
    void OnFindSearchClick( wxCommandEvent& event );
    void OnFindCancelClick( wxCommandEvent& event );

    void SetCodeCtrl(CodeCtrl * ctrl)
    {
	codectrl = ctrl;
    }

    wxString LastSearch()
    {
	return lastsearch;
    }

    wxString LastReplace()
    {
	return lastreplace;
    }

    bool Searched()
    {
	return !lastsearch.IsEmpty();
    }

    bool Replaced()
    {
	return !lastreplace.IsEmpty();
    }

    int LastFlags()
    {
	return lastflags;
    }

    wxBitmap GetBitmapResource( const wxString& name );
    wxIcon GetIconResource( const wxString& name );
    static bool ShowToolTips();

private:
    wxTextCtrl* FindCtrl;
    wxTextCtrl* ReplaceCtrl;
    wxCheckBox* MatchCheckBox;
    wxButton* ButtonSearch;
    wxButton* ButtonCancel;

    CodeCtrl * codectrl;

    int mode;
    int lastflags;
    int setanchor;
    wxString lastsearch;
    wxString lastreplace;
};

#endif
    // _FINDDIALOG_H_
