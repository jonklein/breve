/*****************************************************************************
 *                                                                           *
 * The breve Simulation Environment                                          *
 * Copyright (C) 2000, 2001, 2002, 2003 Jonathan Klein                       *
 *                                                                           *
 * This program is free software; you can redistribute it and/or modify      *
 * it under the terms of the GNU General Public License as published by      *
 * the Free Software Foundation; either version 2 of the License, or         *
 * (at your option) any later version.                                       *
 *                                                                           *
 * This program is distributed in the hope that it will be useful,           *
 * but WITHOUT ANY WARRANTY; without even the implied warranty of            *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the             *
 * GNU General Public License for more details.                              *
 *                                                                           *
 * You should have received a copy of the GNU General Public License         *
 * along with this program; if not, write to the Free Software               *
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA *
 *****************************************************************************/

/*
 * Based on code by Rachel Grey, lemming@alum.mit.edu.
 */



#include <windows.h>
#include <winbase.h>
#include <scrnsave.h>
#include <GL/gl.h>
#include <GL/glu.h>

#include "steve.h"
#include "kernel.h"
#include "simulation.h"
#include "gldraw.h"

//Define a Windows timer

#define TIMER 1

/*

DLG_CONFIG DIALOG DISCARDABLE  0, 0, 186, 95
STYLE DS_MODALFRAME | WS_POPUP | WS_CAPTION | WS_SYSMENU
CAPTION "Config dialog"
FONT 8, "MS Sans Serif"
BEGIN
    DEFPUSHBUTTON   "OK",IDOK,129,7,50,14
    PUSHBUTTON      "Cancel",IDCANCEL,129,24,50,14
    CONTROL         "Flash screen",IDC_FLASH,"Button",BS_AUTOCHECKBOX | 
                    WS_TABSTOP,23,13,56,10
END

*/

//These forward declarations are just for readability,
//so the big three functions can come first 

void InitGL(HWND hWnd, HDC & hDC, HGLRC & hRC);
void CloseGL(HWND hWnd, HDC hDC, HGLRC hRC);
void GetConfig();		
void WriteConfig(HWND hDlg);
void SetupAnimation(int Width, int Height);
void CleanupAnimation();
void OnTimer(HDC hDC);


int Width, Height; //globals for size of screen


//////////////////////////////////////////////////
////   INFRASTRUCTURE -- THE THREE FUNCTIONS   ///
//////////////////////////////////////////////////


// Screen Saver Procedure
LRESULT WINAPI ScreenSaverProc(HWND hWnd, UINT message, 
                               WPARAM wParam, LPARAM lParam) {
  static HDC hDC;
  static HGLRC hRC;
  static RECT rect;

  switch ( message ) {

  case WM_CREATE: 
    // get window dimensions
    GetClientRect( hWnd, &rect );
    Width = rect.right;		
    Height = rect.bottom;
    
	//get configuration from registry
	GetConfig();

	// setup OpenGL, then animation
    InitGL( hWnd, hDC, hRC );
    SetupAnimation(Width, Height);

	//set timer to tick every 10 ms
    SetTimer( hWnd, TIMER, 10, NULL );
    return 0;
 
  case WM_DESTROY:
    KillTimer( hWnd, TIMER );
	CleanupAnimation();
    CloseGL( hWnd, hDC, hRC );
    return 0;

  case WM_TIMER:
    OnTimer( hDC );	//animate!	
    return 0;				

  }

  return DefScreenSaverProc( hWnd, message, wParam, lParam );

}

bool bTumble = true;


BOOL WINAPI
ScreenSaverConfigureDialog(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam) {
	return FALSE;
}

BOOL WINAPI RegisterDialogClasses(HANDLE hInst) {
  return TRUE;
}

/////////////////////////////////////////////////
////   INFRASTRUCTURE ENDS, SPECIFICS BEGIN   ///
////                                          ///
////    In a more complex scr, I'd put all    ///
////     the following into other files.      ///
/////////////////////////////////////////////////


// Initialize OpenGL
void InitGL(HWND hWnd, HDC & hDC, HGLRC & hRC) {
  PIXELFORMATDESCRIPTOR pfd;
  ZeroMemory( &pfd, sizeof( pfd ) );
  pfd.nSize = sizeof pfd;
  pfd.nVersion = 1;
  pfd.dwFlags = PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
  pfd.iPixelType = PFD_TYPE_RGBA;
  pfd.cColorBits = 24;
  
  hDC = GetDC( hWnd );
  
  int i = ChoosePixelFormat( hDC, &pfd );  
  SetPixelFormat( hDC, i, &pfd );

  hRC = wglCreateContext( hDC );
  wglMakeCurrent( hDC, hRC );
}

void CloseGL(HWND hWnd, HDC hDC, HGLRC hRC) {
  wglMakeCurrent( NULL, NULL );
  wglDeleteContext( hRC );
  ReleaseDC( hWnd, hDC );
}

breveFrontend *gFrontend;

void SetupAnimation( int inWidth, int inHeight ) {
	glViewport( 0, 0, 300, 300 );

	slSetMessageCallbackFunction( NULL );

	gFrontend = breveFrontendInit( 0, NULL );

	gFrontend->data = breveFrontendInitData( gFrontend->engine );

	// Do all the path stuff
	
	char winDir[ 1024 ];
	GetWindowsDirectory( winDir, 1024 );
	
	std::string classPath = std::string( winDir ) + "\\breve\\lib\\classes\\";

    brEngineSetIOPath( gFrontend->engine, classPath.c_str() );
	brAddSearchPath( gFrontend->engine, classPath.c_str() );

	std::string simpath = std::string( winDir ) + "\\breve\\Creatures.tz";

	char *simulationText = slUtilReadFile( simpath.c_str() );
	breveFrontendLoadSimulation( gFrontend, simulationText, simpath.c_str() );

	// Hooray!

	gFrontend->engine->camera->setBounds( inWidth, inHeight );
    slInitGL( gFrontend->engine->world, gFrontend->engine->camera );
}

void OnTimer(HDC hDC) {
	brEngineIterate( gFrontend->engine );
	brEngineRenderWorld( gFrontend->engine, 0 );

	SwapBuffers(hDC);
}

void CleanupAnimation() {
	delete gFrontend;
}

void GetConfig() {
}

void WriteConfig(HWND hDlg) {
}
