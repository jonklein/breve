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
	+ slBreve
	= is the interface controller class for breve.  Up until release 1.2,
	= this file also controlled all interactions with the breve engine,
	= but the class became confusing and messy.
	= 
	= So this file now handles all strictly interface related tasks, and
	= interacts with the breve engine through the slBreveEngine object.
*/

/*
	aim:goim?screenname=rockstjaerna@mac.com
*/

#import "slBreve.h"

extern BOOL engineWillPause;

@implementation slBreve

extern char *gErrorNames[];

void slPrintLog(char *text);

/* gSimMenu and gSelf are a bit of a hack--we need to these variables */
/* from regular C callbacks, so we'll stick the values in global variables */

static NSMenu *gSimMenu;
static slBreve *gSelf;
static NSMutableString *gLogString;
static NSRecursiveLock *gLogLock;

- (void)showWelcomeMessage {
	NSString *message = NSLocalizedStringFromTable(@"Welcome Message", @"General", @"");
	NSString *title = NSLocalizedStringFromTable(@"Welcome Title", @"General", @"");

	NSBeginInformationalAlertSheet(title, @"Okay", NULL, NULL, runWindow, self, NULL, NULL, NULL, message);
}

- (BOOL)application:(NSApplication *)theApplication openFile:(NSString *)filename {
	[[[NSDocumentController sharedDocumentController] 
		openDocumentWithContentsOfFile: filename display: YES] retain];

	return YES;
}

- (void)applicationWillFinishLaunching:(NSNotification*)not {
	struct direct **docsArray;
	int demoCount, n;
	NSString *name;
	NSString *bundlePath;

	NSUserDefaults *defaults = [NSUserDefaults standardUserDefaults];

	engineWillPause = NO;

	documents = [[NSMutableArray arrayWithCapacity: 20] retain];
        
	/* we have a C function here that needs access to a class variable   */
	/* so we're going to cheat and make a local copy of it.  please dont */
	/* tell my former computer science professors, it would crush them.  */

	[[NSNotificationCenter defaultCenter] addObserver: displayView selector: @selector(updateSize:) name: nil object: displayView];
	[[NSNotificationCenter defaultCenter] addObserver: self selector: @selector(simPopupSetItems:) name: @"NSPopUpButtonWillPopUpNotification" object: simPopup];

	slSetMessageCallbackFunction(slPrintLog);

	logLock = [[NSRecursiveLock alloc] init];

	gSimMenu = simMenu;
	gLogLock = logLock;
	gSelf = self;

	gLogString = [[NSMutableString stringWithCapacity: 1024] retain];
	[gLogString setString: @""];

	editorData = [[slObjectOutline alloc] init];
	[editorData setOutlineView: variableOutlineView];

	[variableOutlineView setDataSource: editorData];
	[variableOutlineView setDelegate: editorData];

	bundlePath = [[NSBundle mainBundle] resourcePath];

	demoPath = strdup([[NSString stringWithFormat: @"%@/demos", bundlePath] cString]);
	classPath = strdup([[NSString stringWithFormat: @"%@/classes", bundlePath] cString]);
	docsPath = strdup([[NSString stringWithFormat: @"%@/docs", bundlePath] cString]);
	classDocsPath = strdup([[NSString stringWithFormat: @"%@/docs/classes", bundlePath] cString]);

	srand(time(NULL));
	srandom(time(NULL));
	
	[self buildDemoMenuForDir: demoPath forMenu: demoMenu];

	demoCount = scandir(classDocsPath, &docsArray, isHTMLfile, alphasort);

	if(demoCount > 0) {
		for(n=0;n<demoCount;n++) {
			name = [NSString stringWithCString: docsArray[n]->d_name];
			[docsMenu insertItemWithTitle: name action: @selector(docsMenu:) keyEquivalent: @"" atIndex: n];

			free(docsArray[n]);
		}

		if(demoCount) free(docsArray);
	}

	[runWindow setFrameAutosaveName: @"runWindow"];

	[simMenu setAutoenablesItems: NO];

	[saveMovieMenuItem setEnabled: NO];
	[savePictureMenuItem setEnabled: NO];

	[runWindow makeKeyAndOrderFront: nil];

	if(![[defaults stringForKey: @"ShowedWelcomeMessage"] isEqualTo: @"2.0d"]) {
		[defaults setObject: @"2.0d" forKey: @"ShowedWelcomeMessage"];
		[self showWelcomeMessage];
	}

	[preferences loadPrefs];

	docDictionary = [self parseDocsIndex];

	if([preferences shouldShowOpenOnLaunch]) 
		[[NSDocumentController sharedDocumentController] openDocument: self];

	[NSTimer scheduledTimerWithTimeInterval: .2 target: self selector: @selector(updateLog:) userInfo: NULL repeats: YES];

	[NSTimer scheduledTimerWithTimeInterval: 1.0 target: self selector: @selector(updateSelectionWindow:) userInfo: NULL repeats: YES];
}

- (void)buildDemoMenuForDir:(char*)directory forMenu:(slDemoMenu*)menu {
	struct direct **demoArray;
	struct stat st;
	char *fullpath;
	int n;
	int menuCount = 0;
	int demoCount = scandir(directory, &demoArray, isTZfile, alphasort);
	NSString *name;

	[menu setPath: [NSString stringWithCString: directory]];

	for(n=0;n<demoCount;n++) {
		fullpath = slMalloc(strlen(directory) + strlen(demoArray[n]->d_name) + 2);

		sprintf(fullpath, "%s/%s", directory, demoArray[n]->d_name);

		stat(fullpath, &st);
		
		name = [NSString stringWithCString: demoArray[n]->d_name];

		if(st.st_mode & S_IFDIR && ![[name pathExtension] isEqualToString: @"nib"]) {
			id item, submenu;

			item = [menu insertItemWithTitle: name action: @selector(demoMenu:) keyEquivalent: @"" atIndex: menuCount++];

			submenu = [[[slDemoMenu alloc] init] autorelease];

			[menu setSubmenu: submenu forItem: item];

			[self buildDemoMenuForDir: fullpath forMenu: submenu];
		} else if(![[name pathExtension] isEqualToString: @"nib"]) {
			[menu insertItemWithTitle: name action: @selector(demoMenu:) keyEquivalent: @"" atIndex: menuCount++];
		}
		
		slFree(fullpath);

		free(demoArray[n]);
	}

	free(demoArray);
}

/*
	+ applicationWillTerminate:
	= actions to be called on quit
*/

- (void)applicationWillTerminate:(NSNotification*)not {
	[breveEngine stopSimulation];
	[preferences savePrefs];
}

- (IBAction)checkSyntax:sender {
	NSString *file;
	char *buffer = NULL;
	char *name = NULL;

	if(!currentDocument) return;

	buffer = slStrdup((char*)[[(slBreveSourceDocument*)currentDocument documentText] cString]);

	file = [currentDocument fileName];
	if(!file) file = [currentDocument displayName];

	(char*)[file cString];

	if([breveEngine checkSyntax: buffer withFilename: name]) {
		[self runErrorWindow];
	} else {
		NSString *title = NSLocalizedStringFromTable(@"Syntax Check Passed Title", @"General", @"");
		NSString *message = NSLocalizedStringFromTable(@"Syntax Check Passed Message", @"General", @"");

		NSRunAlertPanel(title, message, nil, nil, nil);
	}

	[breveEngine freeEngine];
}

- (void)simulateRunClick {
	[runButton performClick: self];
}

- (IBAction)toggleFullScreen:sender {
	int state = ![sender state];

	[self setFullScreen: state];
}

- (void)setFullScreen:(BOOL)s {
	fullscreen = s;

	if(s) [fullScreenMenuItem setState: NSOnState];
	else [fullScreenMenuItem setState: NSOffState];
}

- (IBAction)startSimulationWithArchivedFile:sender {
	NSString *saved;
	char *csaved;
	NSString *file;
	char *buffer = NULL;
	char *name = NULL;

	[loadSimText setStringValue: [NSString stringWithFormat: @"Select a file containing an archived simulation of the currently selected simulation file (%@)", [currentDocument displayName]]];

	saved = [self loadNameForTypes: [NSArray arrayWithObjects: @"xml", @"brevexml", @"tzxml", NULL] withAccView: archiveOpenAccView];

	if(!saved) return;

	csaved = (char*)[saved cString];

	if(!currentDocument) return;


	buffer = slStrdup((char*)[[(slBreveSourceDocument*)currentDocument documentText] cString]);

	file = [currentDocument fileName];
	if(!file) file = [currentDocument displayName];

	name = slStrdup((char*)[file cString]);

	[self clearLog: self];

	chdir([preferences defaultPath]);
	[breveEngine setOutputPath: [preferences defaultPath]];

	if([breveEngine startSimulationWithText: buffer withFilename: name withSavedSimulationFile: csaved fullscreen: fullscreen]) {
		[self runErrorWindow];
		[self clearSimulationMenu];
		[breveEngine freeEngine];
		[runButton setState: NSOffState];

		slFree(buffer);
		slFree(name);

		return;
	}


	[saveMovieMenuItem setEnabled: YES];
	[savePictureMenuItem setEnabled: YES];
	[self updateObjectSelection];
	[stopButton setEnabled: YES];
	[syntaxMenuItem setEnabled: NO];

	[runWindow makeFirstResponder: displayView];

	slFree(buffer);
	slFree(name);
}

- (IBAction)startSimulationFromMenu:sender {
	int mode = [breveEngine getRunState];

	[runWindow makeKeyAndOrderFront: self];

	if(mode == BX_STOPPED) {
		[self startSimulation];
		[runSimMenuItem setTitle: @"Stop Simulation"];
		[runButton setState: NSOnState];
	} else if(mode == BX_RUN) {
		[self stopSimulation: sender];
		[runSimMenuItem setTitle: @"Start Simulation"];
		[runButton setState: NSOffState];
	}
}

- (IBAction)toggleSimulation:sender {
	int mode;

	mode = [breveEngine getRunState];

	if(mode == BX_STOPPED) [self startSimulation];
	if(mode == BX_RUN) [breveEngine pauseSimulation];
	if(mode == BX_PAUSE) [breveEngine unpauseSimulation];

}

- (void)startSimulation {
	NSString *file;
	char *buffer = NULL;
	char *name = NULL;
	int mode;

	[breveEngine lock];
	mode = [breveEngine getRunState];
	[breveEngine unlock];

	if(mode != BX_STOPPED) return;

	if(!currentDocument) {
		[runButton setState: NSOffState];
		return;
	}

	buffer = slStrdup((char*)[[(slBreveSourceDocument*)currentDocument documentText] cString]);

	file = [currentDocument fileName];
	if(!file) file = [currentDocument displayName];

	name = slStrdup((char*)[file cString]);

	[self clearLog: self];

	chdir([preferences defaultPath]);
	[breveEngine setOutputPath: [preferences defaultPath]];

	if([breveEngine startSimulationWithText: buffer withFilename: name withSavedSimulationFile: NULL fullscreen: fullscreen]) {
		[self runErrorWindow];
		[self clearSimulationMenu];
		[breveEngine freeEngine];
		[runButton setState: NSOffState];

		slFree(buffer);
		slFree(name);

		return;
	}

	[saveMovieMenuItem setEnabled: YES];
	[savePictureMenuItem setEnabled: YES];
	[self updateObjectSelection];
	[stopButton setEnabled: YES];
	[syntaxMenuItem setEnabled: NO];
	[runCommandButton setEnabled: YES];

	[runWindow makeFirstResponder: displayView];

	slFree(buffer);
	slFree(name);

	[breveEngine go];
}

- (IBAction)stopSimulation:sender {
	int mode = [breveEngine getRunState];

	if(mode == BX_STOPPED) return;

	if(movieRecording) [self toggleMovieRecord: self];

	[editorData setEngine: NULL instance: NULL];
	[variableOutlineView reloadData];
	[selectionText setStringValue: @"No Selection"];

	[displayView setEngine: NULL fullscreen: NO];
	[displayView setNeedsDisplay: YES];

	[self setSimulationMenuEnabled: NO];

	[saveMovieMenuItem setEnabled: NO];
	[savePictureMenuItem setEnabled: NO];
	[runCommandButton setEnabled: NO];

	[breveEngine stopSimulation];

	[self clearSimulationMenu];

	[runButton setState: NSOffState];
	[runButton setEnabled: YES];
	[stopButton setEnabled: NO];
	[syntaxMenuItem setEnabled: YES];
}

- (void)doSelectionAt:(NSPoint)p {
	[breveEngine doSelectionAt: p];
}

- (void)doKeyEvent:(char)key isDown:(int)d {
	[breveEngine doKeyEvent: key isDown: d];
}

- (stInstance*)getSelectedInstance {
	return [breveEngine getSelectedInstance];
}

- (void)updateObjectSelection {
	stInstance *i = NULL;
	brEngine *engine = [breveEngine getEngine];
	brInstance *controller;
	
	if(!engine) return;

	controller = brEngineGetController(engine);

	i = [breveEngine getSelectedInstance];

	if(!i || i->status != AS_ACTIVE) {
		[selectionText setStringValue: 
			[NSString stringWithFormat: @"No Selection (%s [%p])", controller->object->name, controller]];

		[editorData setEngine: engine instance: controller->userData];
	} else {
		[editorData setEngine: engine instance: i];

		[selectionText setStringValue: [NSString stringWithFormat: @"%s (%p)", i->type->name, i]];
	}

	[variableOutlineView reloadData];
}

- (IBAction)updateSelectionWindow: sender {
	[variableOutlineView reloadData];
}

- (IBAction)demoMenu:sender {
	NSString *string;

	string = [NSString stringWithFormat: @"%@/%@", [(slDemoMenu*)[sender menu] path], [sender title]];

	[[[NSDocumentController sharedDocumentController] 
		openDocumentWithContentsOfFile: string display: YES] retain];
}

- (IBAction)docsMenu:sender {
	NSString *filePath;
	NSURL *path;

	filePath = [NSString stringWithFormat: @"%s/%@", classDocsPath, [sender title]];

	path = [NSURL fileURLWithPath: filePath];

	[[NSWorkspace sharedWorkspace] openURL: path];
}

- (IBAction)showHTMLHelp:sender {
	NSString *filePath;
	NSString *bundlePath = [[NSBundle mainBundle] resourcePath];

	filePath = [NSString stringWithFormat: @"%@/docs/index.html", bundlePath];
	[[NSWorkspace sharedWorkspace] openURL: [NSURL fileURLWithPath: filePath]];
}

- (IBAction)simMenu:sender {
	brEngine *e = [breveEngine getEngine];
	brInstance *controller = brEngineGetController(e);
	[self doMenuCallbackForInstance: controller->userData item: [sender tag]];
}

- (IBAction)contextualMenu:sender {
	[self doMenuCallbackForInstance: [breveEngine getSelectedInstance] item: [sender tag]];
}

- (void)doMenuCallbackForInstance:(stInstance*)i item:(int)n {
	int result;
	brEngine *e = [breveEngine getEngine];

	if([breveEngine getRunState] == BX_RUN) [breveEngine lock];
	result = brMenuCallback(e, i->breveInstance, n);

	if(result != EC_OK) {
		[self runErrorWindow];
		[breveEngine unlock];
		[self stopSimulation: self];

		return;
	}

	[displayView setNeedsDisplay: YES];

	if([breveEngine getRunState] == BX_RUN) [breveEngine unlock];
}

- (IBAction)newWithTemplate:sender {
	NSString *template, *type;
	NSDocument *d;

	template = [NSString stringWithFormat: @"%@/Template.tz", 
					[[NSBundle mainBundle] resourcePath]];

	type = [[NSDocumentController sharedDocumentController] typeFromFileExtension: @"tz"];

	d = [[[NSDocumentController sharedDocumentController] 
		openDocumentWithContentsOfFile: template display: YES] retain];

	[d setFileName: NULL];
}

- (IBAction)saveLog:sender {
	NSString *logFile = [self saveNameForType: @"txt" withAccView: NULL];

	if(logFile) [[logText string] writeToFile: logFile atomically: YES];
}

- (IBAction)snapshot:sender {
	NSString *result;
	int mode;

	if(!breveEngine) return;

	mode = [breveEngine getRunState];

	if(mode == BX_RUN) [breveEngine lock];

	result = [self saveNameForType: @"tiff" withAccView: NULL];

	if(result) [displayView snapshotToFile: result];

	if(mode == BX_RUN) [breveEngine unlock];
}


- (void)clearSimulationMenu {
	int n, items = [(NSMenu*)simMenu numberOfItems];

	/* once an item is removed, apparently the others shift down to */
	/* take it's place, so the index we want to remove is ALWAYS 0 */

	for(n=0;n<items;n++) [simMenu removeItemAtIndex: 0];
}

- (void)setSimulationMenuEnabled:(BOOL)state {
	int n, items = [(NSMenu*)simMenu numberOfItems];
	id item;

	for(n=0;n<items;n++) {
		item = [simMenu itemAtIndex: n];
		[item setEnabled: state];
	}
}

- (IBAction)clearLog:sender {
	[logText setString: @""];
}

void slPrintLog(char *text) {
	NSAutoreleasePool *pool = [[NSAutoreleasePool alloc] init];
	[gLogLock lock];
	[gLogString appendString: [NSString stringWithCString: text]];
	[gLogLock unlock];
	[pool release];
}

- (int)updateLog:sender {
	if(breveEngine && engineWillPause) [breveEngine pauseSimulation];

	if([gLogString length] == 0) return 0;

	[logLock lock];
	// [logText setSelectable: NO];

	[logText moveToEndOfDocument: self];
	[logText insertText: gLogString];
	[gLogString setString: @""];

	[logLock unlock];

	return 1;
}

- (void)runErrorWindow {
	NSRect bounds;
	NSPoint origin;
	char *simpleFilename, *currentCName = NULL;
	brEngine *engine;
	unsigned int n;
	NSString *stringKey, *text;
	brErrorInfo *error;
	
	engine = [breveEngine getEngine];

	if(!engine) return;

	error = brEngineGetErrorInfo(engine);

	if(error->file) {
		simpleFilename = (char*)[[[NSString stringWithCString: error->file] lastPathComponent] cString];
	} else {
		// this shouldn't happen, but it happened once and caused a crash, so I'm
		// fixing the symptom instead of the disease.  for shame.

		simpleFilename = "(untitled)";
	}

	for(n=0;n<[documents count];n++) {
		slBreveSourceDocument *doc = [documents objectAtIndex: n];

		if([doc fileName]) {
			currentCName = (char*)[[[doc fileName] lastPathComponent] cString];

			if(!strcmp(currentCName, simpleFilename)) {
				[[doc text] goToLine: error->line];
				[[doc window] makeKeyAndOrderFront: self];
			}
		}
	}

	stringKey = [NSString stringWithFormat: @"%s Title", gErrorNames[error->type]];
	text = NSLocalizedStringFromTable(stringKey, @"Errors", @"");

	[errorWindow setTitle: text];

	[errorWindowErrorMessage setHorizontallyResizable: NO];
	[errorWindowErrorMessage setVerticallyResizable: YES];

	[errorWindowErrorMessage setDrawsBackground: NO];
	[errorWindowErrorMessage setEditable: NO];
	[errorWindowErrorMessage setSelectable: NO];

	stringKey = [NSString stringWithFormat: @"%s Text", gErrorNames[error->type]];
	text = NSLocalizedStringFromTable(stringKey, @"Errors", @"");

	[errorWindowErrorMessage setString: 
		[NSString stringWithFormat: @"Error in file \"%s\" at line %d:\n\n%s\n\n%@", 
			simpleFilename, 
			error->line, 
			error->message, 
			text
		]
	];

	[errorWindowErrorMessage sizeToFit];
	bounds = [errorWindowErrorMessage bounds];

	// the following values are determined from IB 

	origin.x = 120;
	origin.y = 60;

	bounds.size.height += 80; // 80 is the space for the border/button from IB 
	bounds.size.width += 140; // 120 is the space for the border from IB 

	[errorWindowErrorMessage setFrameOrigin: origin];
	[errorWindow setContentSize: bounds.size];
	[errorWindow center];

	[NSApp runModalForWindow: errorWindow];

	brClearError(engine);
}

- (IBAction)stopErrorWindowModal:sender {
	[errorWindow orderOut: self];
	[NSApp stopModal];
}

- (IBAction)toggleMovieRecord:sender {
	if(!movieRecording) {
		if([breveEngine startMovie] == YES) {
			movieRecording = YES;
		} else {
			return;
		}
		
		[saveMovieMenuItem setTitle: @"Stop Recording"];
	} else {
		[saveMovieMenuItem setTitle: @"Record Movie..."];
		[breveEngine stopMovie];
		movieRecording = NO;
	}
}

- (IBAction)openBreveHomepage:sender {
	[[NSWorkspace sharedWorkspace] openURL: [NSURL URLWithString: @"http://www.spiderland.org/breve"]];
}

- (IBAction)openEmailMessage:sender {
	[[NSWorkspace sharedWorkspace] openURL: [NSURL URLWithString: @"mailto:jklein@spiderland.org"]];
}

int isTZfile(struct dirent *d) {
	char *filename = d->d_name;
	int m;

	if(d->d_type == DT_DIR && strcmp(filename, ".") && strcmp(filename, "..")) return 1;

	m = strlen(filename);
   
	if(m < 4) return 0;

	return !strcmp(&filename[m - 3], ".tz");
}

int isHTMLfile(struct dirent *d) {
	char *filename = d->d_name;
	int m;

	m = strlen(filename);
   
	if(m < 6) return 0;

	return !strcmp(&filename[m - 5], ".html");
}

void updateMenu(brInstance *i) {
	id menuItem;
	int n;

	if(!i->engine || i != brEngineGetController(i->engine)) return;

	[gSelf clearSimulationMenu];

	for(n=0;n<i->menus->count;n++) {
		brMenuEntry *entry = i->menus->data[n];

		if(!strcmp(entry->title, "")) {
			[gSimMenu addItem: [NSMenuItem separatorItem]];
		} else {
			menuItem = [gSimMenu addItemWithTitle: [NSString stringWithCString: entry->title] action: @selector(simMenu:) keyEquivalent: @""];

			[menuItem setTag: n];

			[menuItem setEnabled: entry->enabled];
			[menuItem setState: entry->checked];
		}
	}
}

- (NSString*)saveNameForType:(NSString *)type withAccView:(id)view {
	NSSavePanel *savePanel;

	savePanel = [NSSavePanel savePanel];
	[savePanel setRequiredFileType: type];

	[savePanel setAccessoryView: view];

	if([savePanel runModal] == NSFileHandlingPanelCancelButton) return NULL;

	return [savePanel filename];
}

- (NSString*)loadNameForTypes:(NSArray*)types withAccView:(NSView*)view {
	NSOpenPanel *openPanel;

	openPanel = [NSOpenPanel openPanel];

	[openPanel setAccessoryView: view];

	if([openPanel runModalForTypes: types] != NSOKButton) return NULL;

	return [[openPanel filenames] objectAtIndex: 0];
}

- (IBAction)showAboutBox:sender {
	[aboutBox center];
	[aboutBox makeKeyAndOrderFront: self];
}

- (NSSize)windowWillResize:(NSWindow*)window toSize:(NSSize)newSize {
	NSRect bounds;

	// if we're recording a movie, we can't allow the window to resize

	if(movieRecording && window == runWindow) {
		bounds = [window frame];
		return bounds.size;
	}

	return newSize;
}

- (void)setWindowTitleMessage:(NSString*)s {
	if(!s) [runWindow setTitle: [NSString stringWithFormat: @"breve"]];
	else [runWindow setTitle: [NSString stringWithFormat: @"breve (%@)", s]];
}

- (void)showFullscreenError {
	NSString *message = NSLocalizedStringFromTable(@"Fullscreen Error Message", @"General", @"");
	NSString *title = NSLocalizedStringFromTable(@"Fullscreen Error Title", @"General", @"");

	NSRunAlertPanel(title, message, nil, nil, nil);
}

- (IBAction)find:sender {
	id document = [[NSDocumentController sharedDocumentController] currentDocument];

	[document find: sender];
}

- (IBAction)findNext:sender {
	id document = [[NSDocumentController sharedDocumentController] currentDocument];

	[document findNext: sender];
}

- (IBAction)findPrevious:sender {
	id document = [[NSDocumentController sharedDocumentController] currentDocument];

	[document findPrevious: sender];
}

- (IBAction)findSelection:sender {
	id document = [[NSDocumentController sharedDocumentController] currentDocument];

	[document findSelection: sender];
}

- (IBAction)scrollToSelection:sender {
	id document = [[NSDocumentController sharedDocumentController] currentDocument];

	[document scrollToSelection: sender];
}

- (IBAction)commentLines:sender {
	id document = [[NSDocumentController sharedDocumentController] currentDocument];
	[[document text] commentSelection: self];
}

- (IBAction)uncommentLines:sender {
	id document = [[NSDocumentController sharedDocumentController] currentDocument];
	[[document text] uncommentSelection: self];
}

- (IBAction)reformat:sender {
	id document = [[NSDocumentController sharedDocumentController] currentDocument];
	[document reformat: self];
}

- (void)simPopupSetItems:(NSNotification*)note {
	unsigned int n;
	unsigned int selected;

	selected  = [simPopup indexOfSelectedItem];

	[simPopup removeAllItems];

	for(n=0;n<[documents count];n++) {
		NSString *display = [[documents objectAtIndex: n] displayName];
		NSString *file = [[documents objectAtIndex: n] fileName];
		NSString *title;

		if(display && file) title = [NSString stringWithFormat: @"%@ (%@)", display, file];
		else title = display;
	
		[simPopup addItemWithTitle: title];
	}

	if([documents count] > 0) {
		if(selected >= [documents count]) selected = 0;

		[simPopup selectItemAtIndex: selected];
		[runSimMenuItem setAction: @selector(startSimulationFromMenu:)];
		[loadSimMenuItem setAction: @selector(startSimulationWithArchivedFile:)];
	} else {
		[runSimMenuItem setAction: NULL];
		[loadSimMenuItem setAction: NULL];
		[simPopup addItemWithTitle: @"Choose Simulation..."];
	}
}

- (IBAction)setSimulationPopup:sender {
	if([documents count] < 1) return;

	currentDocument = [documents objectAtIndex: [simPopup indexOfSelectedItem]];

	if(currentDocument) [runButton setEnabled: YES];
}

- (NSMutableDictionary*)parseDocsIndex {
	NSString *index = [NSString stringWithFormat: @"%s/index.txt", docsPath];
	NSString *text = [NSString stringWithContentsOfFile: index];
	NSScanner *scanner;
	NSString *name, *title, *location;
	NSMutableCharacterSet *set;
	NSMutableArray *a;

	NSMutableDictionary *dict;

	dict = [[NSMutableDictionary dictionaryWithCapacity: 50] retain];

	if(!text) return NULL;

	scanner = [[NSScanner alloc] initWithString: text];
	set = [[NSMutableCharacterSet alloc] init];
	[set addCharactersInString: @"\n:"];

	while(![scanner isAtEnd]) {
		[scanner scanUpToCharactersFromSet: set intoString: &name];
		[scanner scanCharactersFromSet: set intoString: NULL];
		[scanner scanUpToCharactersFromSet: set intoString: &title];
		[scanner scanCharactersFromSet: set intoString: NULL];
		[scanner scanUpToCharactersFromSet: set intoString: &location];
		[scanner scanCharactersFromSet: set intoString: NULL];

		if(!(a = [dict objectForKey: name])) {
			a = [NSMutableArray arrayWithCapacity: 10];
			[dict setObject: a forKey: name];
		}

		[a addObject: [NSString stringWithString: location]];
	}

	[scanner release];

	return dict;
}

- (NSDictionary*)docDictionary {
	return docDictionary;
}

- (void)registerDocument:(NSDocument*)d {
	currentDocument = d;

	[documents addObject: d];

	[self simPopupSetItems: NULL];
	[simPopup selectItemAtIndex: [documents count] - 1];
	[runButton setEnabled: YES];

	[self updateAllEditorPreferences];
}

- (void)unregisterDocument:(NSDocument*)d {
	[documents removeObject: d];

	[self simPopupSetItems: NULL];

	if(currentDocument == d) {
		[simPopup selectItemAtIndex: 0];
	}

	if([documents count] > 0) currentDocument = [documents objectAtIndex: 0];
	else currentDocument = NULL;
}

- (void)updateAllEditorPreferences {
	unsigned int n;

	for(n=0;n<[documents count];n++) {
		[preferences updateTextView: [[documents objectAtIndex: n] text]];
	}
}

@end
