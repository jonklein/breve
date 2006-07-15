/////////////////////////////////////////////////////////////////////////////
// Name:        Main.h
// Purpose:     
// Author:      
// Modified by: 
// Created:     Tue 01 Mar 2005 10:13:19 PM EST
// RCS-ID:      
// Copyright:   
// Licence:     
/////////////////////////////////////////////////////////////////////////////

#ifndef _MAIN_H_
#define _MAIN_H_

#if defined(__GNUG__) && !defined(__APPLE__)
#pragma interface "Main.cpp"
#endif

#include "wx/image.h"
#include "BreveRender.h"

class BCTestApp;
class wxFileConfig;

extern BreveRender * gBreverender;
extern BCTestApp * app;
extern char FILE_SEP_PATH;

class BCTestApp: public wxApp
{    
    DECLARE_CLASS( BCTestApp )
    DECLARE_EVENT_TABLE()

public:
    BCTestApp();

    virtual bool OnInit();
    virtual int OnExit();

    wxString GetAppDir()
    {
	return AppDir;
    }

    wxString GetLocalDir()
    {
	return LocalDir;
    }

    wxString GetBreveDir()
    {
	return BreveDir;
    }

    wxArrayString * GetSearchPathArray()
    {
	return &SearchPathArray;
    }

    private:
	wxArrayString SearchPathArray;
	BreveRender * renderwindow;
	wxString AppDir;
	wxString LocalDir;
	wxString BreveDir;
	wxFileConfig * config;
};

DECLARE_APP(BCTestApp)

#endif
    // _BCTEST_H_
