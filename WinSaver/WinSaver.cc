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

//
// Adapted from code by Rachel Grey, lemming@alum.mit.edu.
//

#include <windows.h>
#include <winbase.h>
#include <scrnsave.h>
#include <GL/gl.h>
#include <GL/glu.h>

#include "WinSaverResource.h"

#include "steve.h"
#include "kernel.h"
#include "simulation.h"
#include "gldraw.h"

#define ABOUT_URL	"http://www.spiderland.org"

void InitGL( HWND hWnd, HDC & hDC, HGLRC & hRC );
void CloseGL( HWND hWnd, HDC hDC, HGLRC hRC );
void GetConfig();
void WriteConfig(HWND hDlg);
void SetupAnimation( int Width, int Height );
void CleanupAnimation();
void OnTimer( HDC hDC );

brEngine *gEngine;

#define TIMER 1

LRESULT WINAPI ScreenSaverProc( HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam ) {
	static HDC hDC;
	static HGLRC hRC;
	static RECT rect;

	switch ( message ) {
		case WM_CREATE: 
			// get window dimensions
			GetClientRect( hWnd, &rect );
		
			//get configuration from registry
			GetConfig();

			// setup OpenGL, then animation
			InitGL( hWnd, hDC, hRC );
			SetupAnimation( rect.right, rect.bottom );

			//set timer to tick every 10 ms
			SetTimer( hWnd, 1, 10, NULL );
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

BOOL WINAPI ScreenSaverConfigureDialog(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam) {
	if( message == WM_INITDIALOG  )
		return TRUE;

	if( message == WM_COMMAND ) {
		long command = LOWORD( wParam );
		char winDir[ 1024 ];
		std::string dataFile;

		switch( command ) {
			case IDABOUT:
				::ShellExecute( NULL, "open", ABOUT_URL, NULL, NULL, SW_SHOWNORMAL );
				return TRUE;
				break;
	
			case IDRESET:
				GetWindowsDirectory( winDir, 1024 );
				dataFile = std::string( winDir ) + "\\breve\\lib\\classes\\breveCreatures.xml";

				unlink( dataFile.c_str() );

				return TRUE;
				break;

			default:
				EndDialog( hDlg, command );
				return TRUE;
		}
	}

	return FALSE;
}

BOOL WINAPI RegisterDialogClasses(HANDLE hInst) {
	return TRUE;
}


void InitGL(HWND hWnd, HDC & hDC, HGLRC & hRC) {
	PIXELFORMATDESCRIPTOR pfd;
	ZeroMemory( &pfd, sizeof( pfd ) );

	pfd.nSize = sizeof pfd;
	pfd.nVersion = 1;
	pfd.dwFlags = PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
	pfd.iPixelType = PFD_TYPE_RGBA;
	pfd.cColorBits = 32;
	pfd.cDepthBits = 16;
	pfd.cStencilBits= 2;
	
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

void SetupAnimation( int inWidth, int inHeight ) {
	glViewport( 0, 0, inWidth, inHeight );

	slSetMessageCallbackFunction( NULL );

	gEngine = brEngineNew();
	brInitFrontendLanguages( gEngine );

	// Do all the path stuff
	
	char winDir[ 1024 ];
	GetWindowsDirectory( winDir, 1024 );
	std::string classPath = std::string( winDir ) + "\\breve\\lib\\classes\\";

	brEngineSetIOPath( gEngine, classPath.c_str() );
	brAddSearchPath( gEngine, classPath.c_str() );

	std::string simpath = std::string( winDir ) + "\\breve\\Creatures.tz";
	gEngine -> camera -> setBounds( inWidth, inHeight );
	gEngine -> camera -> initGL();

	char *simulationText = slUtilReadFile( simpath.c_str() );
	brLoadSimulation( gEngine, simulationText, simpath.c_str() );

	// Hooray!

}

void OnTimer( HDC hDC ) {
	brEngineIterate( gEngine );
	brEngineRenderWorld( gEngine, 0 );
	SwapBuffers(hDC);
}

void CleanupAnimation() {
	brEngineFree( gEngine );
}

void GetConfig() {
}

void WriteConfig(HWND hDlg) {

}
