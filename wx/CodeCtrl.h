#ifndef _CODECTRL_H_
#define _CODECTRL_H_

#if defined(__GNUG__) && !defined(__APPLE__)
#pragma interface "CodeCtrl.cpp"
#endif

#include <wx/stc/stc.h>

class CodeWindow;

class CodeCtrl: public wxStyledTextCtrl
{    
    DECLARE_CLASS( CodeCtrl )
    DECLARE_EVENT_TABLE()

private:
    CodeWindow * parent;
    wxString lastsearch;
    wxString lastreplace;
    int lastflags;

public:
    CodeCtrl(CodeWindow * parent);
    ~CodeCtrl();

    bool IsModified();

    void OnMenu(wxCommandEvent &event);
    void OnMarginClick(wxStyledTextEvent &event);
    void OnChar(wxStyledTextEvent &event);
};

#endif
    // _CODECTRL_H_
