/////////////////////////////////////////////////////////////////////////////
// Name:        Main.cpp
// Purpose:     
// Author:      
// Modified by: 
// Created:     Tue 01 Mar 2005 10:13:19 PM EST
// RCS-ID:      
// Copyright:   
// Licence:     
/////////////////////////////////////////////////////////////////////////////

#if defined(__GNUG__) && !defined(__APPLE__)
#pragma implementation "Main.h"
#endif

// For compilers that support precompilation, includes "wx/wx.h".
#include "wx/wxprec.h"

#ifdef __BORLANDC__
#pragma hdrstop
#endif

#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif

#include "Main.h"
#include "BreveRender.h"
#include "BreveInterface.h"

#include <wx/confbase.h>
#include <wx/fileconf.h>
#include <wx/filename.h>

IMPLEMENT_APP( BCTestApp )
IMPLEMENT_CLASS( BCTestApp, wxApp )

BEGIN_EVENT_TABLE( BCTestApp, wxApp )
END_EVENT_TABLE()

BCTestApp::BCTestApp()
{
}

char FILE_SEP_PATH;
BreveRender * breverender;
BCTestApp * app;

bool BCTestApp::OnInit()
{    
    srandom(time(NULL));

    app = this;

#if wxUSE_XPM
    wxImage::AddHandler( new wxXPMHandler );
#endif
#if wxUSE_LIBPNG
    wxImage::AddHandler( new wxPNGHandler );
#endif
#if wxUSE_LIBJPEG
    wxImage::AddHandler( new wxJPEGHandler );
#endif
#if wxUSE_GIF
    wxImage::AddHandler( new wxGIFHandler );
#endif

    config = new wxFileConfig("breveIDE");

    wxConfigBase::Set(config);

    // Absolute path code from http://www.wxwidgets.org/technote/install.htm
    {
#ifdef __WXMSW__
        char buf[512];
        *buf = '\0';
        GetModuleFileName(NULL, buf, 511);
        AppDir = ::wxPathOnly(buf);

	FILE_SEP_PATH = '\\';
#else
        wxString argv0 = argv[0];

	FILE_SEP_PATH = '/';

        if (wxIsAbsolutePath(argv0))
            AppDir = argv0;
        else
        {
            wxPathList pathlist;
            pathlist.AddEnvList(wxT("PATH"));
            AppDir = pathlist.FindAbsoluteValidPath(argv0);
        }

        wxFileName filename(AppDir);
        filename.Normalize();
        AppDir = filename.GetPath(wxPATH_GET_VOLUME);
#endif
	LocalDir = wxGetCwd();

        if (AppDir.Last() != FILE_SEP_PATH)
                AppDir << FILE_SEP_PATH;

        if (LocalDir.Last() != FILE_SEP_PATH)
                LocalDir << FILE_SEP_PATH;
    }

    {
	wxString str, sep;
	bool hasenv = FALSE, manual = FALSE;

#ifdef __WXCOCOA__
	wxSetEnv("BREVE_CLASS_PATH", AppDir << FILE_SEP_PATH << ".." << FILE_SEP_PATH << "Resources");
#endif

	if (wxGetEnv("BREVE_CLASS_PATH", &str) && !str.IsEmpty())
	{
	    hasenv = TRUE;

	    if (str.Last() != FILE_SEP_PATH)
		str << FILE_SEP_PATH;
	}
	else if (config->Read("BreveClassPath", &str))
	{
	    manual = TRUE;

	    if (str.Last() != FILE_SEP_PATH)
		str << FILE_SEP_PATH;
	}
	else if (wxDirExists(AppDir + "lib" + FILE_SEP_PATH + "classes"))
	{
	    manual = TRUE;
	    str = AppDir + "lib" + FILE_SEP_PATH + "classes";
	}
	else if (wxDirExists(LocalDir + "lib" + FILE_SEP_PATH + "classes"))
	{
	    manual = TRUE;
	    str = LocalDir + "lib" + FILE_SEP_PATH + "classes";
	}
	else
	{
	    wxDirDialog d(NULL, "Select the directory containing Breve classes (usually breve/lib/classes)", AppDir);

	    if (d.ShowModal() == wxID_OK)
	    {
		wxSetEnv("BREVE_CLASS_PATH", d.GetPath());

		str = d.GetPath();

		if (str.Last() != FILE_SEP_PATH)
		    str << FILE_SEP_PATH;

		config->Write("BreveClassPath", str);

		manual = TRUE;
	    }
	    else
		wxMessageBox("The Breve engine will not function properly until a class path is defined.");
	}

	BreveDir = "";

	if (config->Read("BreveAppPath", &BreveDir))
	{
	    if (BreveDir.Last() != FILE_SEP_PATH)
		BreveDir << FILE_SEP_PATH;

	    if (!wxDirExists(BreveDir + "demos") || !wxDirExists(BreveDir + "plugins"))
		BreveDir = "";
	}

	if (BreveDir.IsEmpty() && (manual || hasenv))
	{
	    wxString tmp, xstr;

	    xstr = str;

	    while (!xstr.IsEmpty())
	    {
		tmp = xstr.BeforeLast(FILE_SEP_PATH);
		xstr = tmp;

		if (!tmp.IsEmpty())
		{
		    tmp << FILE_SEP_PATH << "lib";

		    if (wxDirExists(tmp))
		    {
			tmp = xstr;
			tmp << FILE_SEP_PATH << "demos";

			if (wxDirExists(tmp))
			    break;
		    }
		}
	    }

	    if (!xstr.IsEmpty())
	    {
		xstr << FILE_SEP_PATH;

		BreveDir = xstr;
	    }
	}

	if (BreveDir.IsEmpty() && (manual || hasenv))
	{
	    wxDirDialog d(NULL, "Select the directory containing Breve resources (demos directory, plugins directory, etc)", AppDir);

	    while (d.ShowModal() == wxID_OK)
	    {
		str = d.GetPath();

		if (str.Last() != FILE_SEP_PATH)
		    str << FILE_SEP_PATH;

		if (wxDirExists(str + "demos") && wxDirExists(str + "plugins"))
		{
		    BreveDir = str;
		    config->Write("BreveAppPath", BreveDir);
		    break;
		}

		wxMessageBox("The selected directory doesn't seem to contain Breve resources.  At the very least,\na demos directory and a plugins directory are expected.");
	    }
	}

	if (BreveDir.IsEmpty() && (manual || hasenv))
	    wxMessageBox("Unable to locate Breve resources.  The Breve engine will not work properly until a proper path is specified.");
    }

    config->Flush();

    renderwindow = new BreveRender(NULL, -1, "breve");

    breverender = renderwindow;

    SetTopWindow(renderwindow);

    if (argc > 1)
    {
	int i = 0;

	for (i = 1; i < argc; i++)
	    renderwindow->LoadSimFile(argv[i]);
    }

    return true;
}

int BCTestApp::OnExit()
{    
    wxConfigBase::Set(NULL);

    delete config;

    return wxApp::OnExit();
}

