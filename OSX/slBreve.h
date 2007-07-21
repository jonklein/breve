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

#import <Cocoa/Cocoa.h>

#import <OpenGL/gl.h>
#import <OpenGL/glu.h>

#import <sys/types.h>
#import <sys/dir.h>

#import "kernel.h"

#import "slBreveEngine.h"
#import "slBreveGLView.h"
#import "slObjectOutline.h"
#import "slTextView.h"
#import "slMovieRecord.h"
#import "slBrevePrefs.h"
#import "slDemoMenu.h"
#import "slFullScreen.h"
#import "slBreveSourceDocument.h"

enum {
	BS_STOP,
	BS_QUIT,
	BS_CLOSE,
	BS_NEW,
	BS_NEW_TEMPLATE,
	BS_RECENT,
	BS_OPEN
};

@interface slBreve : NSObject {
	BOOL movieRecording;
	
	IBOutlet id breveEngine;

	id aboutBox;

	IBOutlet id runWindow;
	IBOutlet id selectionText;

	IBOutlet id errorWindow;
	IBOutlet id errorWindowErrorMessage;

	IBOutlet id runSimMenuItem;
	IBOutlet id loadSimMenuItem;
	IBOutlet id loadSimText;

	BOOL fullscreen;

	NSDocument *currentDocument;
	NSMutableArray *documents;
	

	IBOutlet id displayView;
	
	IBOutlet id demoMenu;

	IBOutlet id _stDocsMenu;
	IBOutlet id _pyDocsMenu;

	IBOutlet id simMenu;

	IBOutlet id runButton;
	IBOutlet id stopButton;

	IBOutlet id simPopup;

	IBOutlet id fullScreenMenuItem;
	IBOutlet id saveMovieMenuItem;
	IBOutlet id savePictureMenuItem;
	IBOutlet id syntaxMenuItem;

	IBOutlet id _versionField;

	IBOutlet id logText;
	
	IBOutlet id variableOutlineView;

	IBOutlet id archiveOpenAccView;

	slObjectOutline *editorData;

	NSRecursiveLock *logLock;

	NSString *currentFilename;

	NSMutableDictionary *docDictionary;

	char *demoPath;
	char *classPath;
	char *docsPath;
	char *_pyClassDocsPath;
	char *_stClassDocsPath;
	
	IBOutlet id preferences;

	IBOutlet id runCommandButton;

	NSString *_versionString;
}

- (void)simulateRunClick;

- (IBAction)toggleMovieRecord:sender;
- (IBAction)toggleSimulation:sender;
- (IBAction)startSimulationFromMenu:sender;
- (IBAction)stopSimulation:sender;
- (IBAction)showAboutBox:sender;
- (IBAction)checkSyntax:sender;

- (IBAction)toggleFullScreen:sender;
- (void)setFullScreen:(BOOL)s;

- (brInstance*)getSelectedInstance;

- (void)showWelcomeMessage;

- (void)applicationWillFinishLaunching:(NSNotification*)n;

- (IBAction)clearLog:sender;

- (IBAction)saveLog:sender;

- (IBAction)snapshot:sender;

- (IBAction)newWithTemplate:sender;

- (void)buildDemoMenuForDir:(char*)directory forMenu:(slDemoMenu*)menu;
- (void)buildDocsMenuForDir:(char*)directory forMenu:(id)menu;

- (IBAction)showHTMLHelp:sender;

- (IBAction)demoMenu:sender;
- (IBAction)docsMenu:sender;

- (IBAction)simMenu:sender;
- (IBAction)contextualMenu:sender;

- (void)doSelectionAt:(NSPoint)p;

- (void)doMenuCallbackForInstance:(brInstance*)i item:(int)n;

- (void)clearSimulationMenu;
- (void)setSimulationMenuEnabled:(BOOL)state;

- (IBAction)openBreveHomepage:sender;
- (IBAction)openEmailMessage:sender;

- (void)updateObjectSelection;
- (IBAction)updateSelectionWindow: sender;

- (void)runErrorWindow;

- (IBAction)stopErrorWindowModal:sender;

- (void)startSimulation;

- (IBAction)startSimulationWithArchivedFile:sender;

void updateMenu(brInstance *i);

- (void)setWindowTitleMessage:(NSString*)s;

- (NSString*)saveNameForType:(NSString *)type withAccView:(NSView*)view;
- (NSString*)loadNameForTypes:(NSArray*)types withAccView:(NSView*)view;

int isBreveFile(struct dirent *d);
int isHTMLfile(struct dirent *d);

- (IBAction)find:sender;
- (IBAction)findNext:sender;
- (IBAction)findPrevious:sender;
- (IBAction)findSelection:sender;
- (IBAction)scrollToSelection:sender;
- (IBAction)commentLines:sender;
- (IBAction)uncommentLines:sender;
- (IBAction)reformat:sender;

- (IBAction)setSimulationPopup:sender;

- (NSMutableDictionary*)parseDocsIndex;
- (NSDictionary*)docDictionary;

- (void)registerDocument:(NSDocument*)d;
- (void)unregisterDocument:(NSDocument*)d;

- (void)updateAllEditorPreferences;

- (int)updateLog:sender;

@end
