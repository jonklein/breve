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

    wxImage::AddHandler( new wxTIFFHandler );

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
	bool hasenv = FALSE;

	if (wxGetEnv("BREVE_CLASS_PATH", &str))
	{
	    wxString tmp, xstr;

	    hasenv = TRUE;

	    if (str.Last() != FILE_SEP_PATH)
		str << FILE_SEP_PATH;

	    xstr = str;

	    while (!xstr.IsEmpty())
	    {
		tmp = xstr.BeforeLast(FILE_SEP_PATH);
		xstr = tmp;

		if (!tmp.IsEmpty())
		{
		    tmp << FILE_SEP_PATH << "lib" << FILE_SEP_PATH;

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

	{
	    wxString tmp;

	    tmp = AppDir + "play.tiff";

	    if (wxFileExists(tmp))
		ImageDir = AppDir;
	    else
		ImageDir = LocalDir;
	}

	/* Though functional, this is ugly - clean up at some point */

	if (BreveDir.IsEmpty())
	{
	    wxString tmp;

	    tmp = AppDir;

	    tmp << "lib" << FILE_SEP_PATH << "classes";

	    if (wxDirExists(tmp))
	    {
		BreveDir = AppDir;

		if (!hasenv)
		    wxSetEnv("BREVE_CLASS_PATH", tmp);
	    }
	    else
	    {
		tmp = AppDir;

		tmp << "breve_source" << FILE_SEP_PATH << "lib" << FILE_SEP_PATH << "classes";

		if (wxDirExists(tmp))
		{
		    BreveDir = AppDir;
		    BreveDir << "breve_source" << FILE_SEP_PATH;

		    if (!hasenv)
			wxSetEnv("BREVE_CLASS_PATH", tmp);
		}
		else
		{
		    wxString tmp;

		    tmp = LocalDir;

		    tmp << "lib" << FILE_SEP_PATH << "classes";

		    if (wxDirExists(tmp))
		    {
			BreveDir = LocalDir;

			if (!hasenv)
			    wxSetEnv("BREVE_CLASS_PATH", tmp);
		    }
		    else
		    {
			tmp = LocalDir;

			tmp << "breve_source" << FILE_SEP_PATH << "lib" << FILE_SEP_PATH << "classes";

			if (wxDirExists(tmp))
			{
			    BreveDir = LocalDir;
			    BreveDir << "breve_source" << FILE_SEP_PATH;

			    if (!hasenv)
				wxSetEnv("BREVE_CLASS_PATH", tmp);
			}
			else
			{
			    wxMessageBox("BREVE_CLASS_PATH environmental variable isn't specified,\nand I couldn't figure it out on my own.\nThings aren't going to work properly.");
			}
		    }
		}
	    }
	}
    }

    if (!BreveDir.IsEmpty() && BreveDir.Last() != FILE_SEP_PATH)
	BreveDir << FILE_SEP_PATH;

    renderwindow = new BreveRender(NULL, -1, "Breve Render Window");

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
    return wxApp::OnExit();
}

