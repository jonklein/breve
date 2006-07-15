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

#include "util.h"

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
BreveRender * gBreverender;
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
	wxString str;
	int i, count;
	int found = 0;

	if (wxGetEnv("BREVE_CLASS_PATH", &str) && !str.IsEmpty())
	{
	    wxString xstr;

	    while (!str.IsEmpty())
	    {
#ifdef __WXMSW__
		xstr = str.BeforeFirst(';');
		str = str.AfterFirst(';');
#else
		xstr = str.BeforeFirst(':');
		str = str.AfterFirst(':');
#endif

		if (xstr.Last() != FILE_SEP_PATH)
		    xstr << FILE_SEP_PATH;

		SearchPathArray.Add(xstr);
	    }
	}

	if (config->Read("BreveClassPath", &str))
	{
	    if (str.Last() != FILE_SEP_PATH)
		str << FILE_SEP_PATH;

	    SearchPathArray.Add(str);
	}

#ifdef __WXCOCOA__
	SearchPathArray.Add(AppDir + FILE_SEP_PATH + ".." + FILE_SEP_PATH + "Resources" + FILE_SEP_PATH);
#endif

	SearchPathArray.Add(AppDir);
	SearchPathArray.Add(LocalDir);

	count = SearchPathArray.GetCount();

	str = "";
	str << "lib" << FILE_SEP_PATH << "classes";

	for (i = 0; i < count; i++)
	{
	    if (wxDirExists(SearchPathArray[i] + str))
	    {
		found = 1;
		SearchPathArray.Add(SearchPathArray[i] + "lib" + FILE_SEP_PATH + "classes" + FILE_SEP_PATH);
	    }
	    else if (SearchPathArray[i].Contains(str))
		found = 1;
	}

	if (found == 0)
	{
	    wxDirDialog d(NULL, "Select the directory containing Breve classes (usually breve/lib/classes)", AppDir);

	    if (d.ShowModal() == wxID_OK)
	    {
		str = d.GetPath();

		if (str.Last() != FILE_SEP_PATH)
		    str << FILE_SEP_PATH;

		config->Write("BreveClassPath", str);

		SearchPathArray.Add(str);
	    }
	    else
	    {
		wxMessageBox("breve will not function properly until a class path is defined.");
		found = -1;
	    }
	}

	BreveDir = "";

	if (config->Read("BreveAppPath", &BreveDir))
	{
	    if (BreveDir.Last() != FILE_SEP_PATH)
		BreveDir << FILE_SEP_PATH;

	    if (!wxDirExists(BreveDir + "demos") || !wxDirExists(BreveDir + "plugins"))
		BreveDir = "";
	}

	if (BreveDir.IsEmpty())
	{
	    wxString tmp, xstr;

	    for (i = 0; i < SearchPathArray.GetCount(); i++)
	    {
		xstr = SearchPathArray[i];

		while (!xstr.IsEmpty())
		{
		    tmp = xstr.BeforeLast(FILE_SEP_PATH);
		    xstr = tmp;

		    if (!tmp.IsEmpty())
		    {
			tmp << FILE_SEP_PATH << "demos";

			if (wxDirExists(tmp))
			{
			    tmp = xstr;
			    tmp << FILE_SEP_PATH << "plugins";

			    if (wxDirExists(tmp))
				break;
			}
		    }
		}

		if (!xstr.IsEmpty())
		{
		    xstr << FILE_SEP_PATH;

		    BreveDir = xstr;
		    break;
		}
	    }
	}

	if (BreveDir.IsEmpty() && found != -1)
	{
	    wxDirDialog d(NULL, "Select the directory containing breve resources (demos directory, plugins directory, etc)", AppDir);

	    while (d.ShowModal() == wxID_OK)
	    {
		str = d.GetPath();

		if (str.Last() != FILE_SEP_PATH)
		    str << FILE_SEP_PATH;

		if ( wxDirExists(str + "demos") ) {
		    BreveDir = str;
		    config->Write("BreveAppPath", BreveDir);
		    break;
		}

		wxMessageBox("The selected directory doesn't seem to contain breve resources.  At the very least,\na demos directory and a plugins directory are expected.");
	    }
	}

	if (BreveDir.IsEmpty() && found != -1)
	    wxMessageBox("Unable to locate breve resources.  breve will not work properly until a proper path is specified.");
    }

    config->Flush();

    renderwindow = new BreveRender(NULL, -1, "breve");

    gBreverender = renderwindow;

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

