/*****************************************************************************
 *                                                                           *
 * The breve Simulation Environment                                          *
 * Copyright (C) 2000-2006 Jonathan Klein                                    *
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

#import "slBreveEngine.h"
#import "slBreveNibLoader.h"
#import "slGraphWindowController.h"
#import "slGraphWindowView.h"

#import "steve.h"
#import "interface.h"
#import "interactive.h"
#import "camera.h"

id simNib;
id mySelf;

id mainRunLoop;

id gDisplayView;

id gInterfaceController;

BOOL _engineWillPause;

@implementation slBreveEngine

NSString *movieQualityStrings[] = {
	@"Lower quality, better compression",
	@"Moderate quality, moderate compression",
	@"Better quality, lower compression",
	@"",
	@"Very high quality, very low compression",
	@"",
	@"Lossless quality, zero compression"
};

int slMakeCurrentContext();

- init {
	_engineLock = [[NSRecursiveLock alloc] init];
	_threadLock = [[NSRecursiveLock alloc] init];

	gInterfaceController = interfaceController;

	speedFactor = 1;

	_engineWillPause = NO;

	runState = BX_STOPPED;
	mySelf = self;
	return self;

	outputPath = NULL;
}

- (IBAction)setSpeed:(id)sender {
	speedFactor = [sender tag];

	[oldSpeedMenuItem setState: 0];
	[sender setState: 1];

	oldSpeedMenuItem = sender;
}

- (void)setOutputPath:(char*)path useSimDir:(BOOL)sim {
	outputPath = path;
	useSimDirForOutput = sim;
}

- (void)lock {
	[_engineLock lock];
}

- (void)unlock {
	[_engineLock unlock];
}

- (void)initEngine {
	NSString *bundlePath; 
	char *classPath, *pluginPath;
	slCamera *camera;

	if( _engine ) 
		return;

	mainRunLoop = [NSRunLoop currentRunLoop];

	bundlePath = [[NSBundle mainBundle] resourcePath];
	classPath = (char*)[[NSString stringWithFormat: @"%@/classes", bundlePath] cString];
	pluginPath = (char*)[[NSString stringWithFormat: @"%@/plugins", bundlePath] cString];

	_engine = brEngineNew();

	brEngineSetSoundCallback(_engine, soundCallback);
	brEngineSetDialogCallback(_engine, dialogCallback);

	camera = brEngineGetCamera(_engine);
	camera->setActivateContextCallback( slMakeCurrentContext );

	if(outputPath) brEngineSetIOPath(_engine, outputPath);

	gDisplayView = displayView;

	brEngineSetInterfaceInterfaceTypeCallback(_engine, interfaceVersionCallback);
	brEngineSetInterfaceSetStringCallback(_engine, interfaceSetStringCallback);
	brEngineSetInterfaceSetNibCallback(_engine, setNibCallback);

	brEngineSetGetSavenameCallback(_engine, getSaveNameCallback);
	brEngineSetGetLoadnameCallback(_engine, getLoadNameCallback);
	brEngineSetPauseCallback(_engine, pauseCallback);
	brEngineSetUnpauseCallback(_engine, unpauseCallback);
	brEngineSetUpdateMenuCallback(_engine, updateMenu);

	brAddSearchPath(_engine, classPath);
	brAddSearchPath(_engine, pluginPath);
	brAddSearchPath(_engine, (char*)[bundlePath cString]);
	brAddSearchPath(_engine, (char*)[NSHomeDirectory() cString]);

	_steveData = (stSteveData*)brInitFrontendLanguages( _engine );
}

- (void)freeEngine {
	[ displayView setEngine: NULL fullscreen: NO ];

	while( [ displayView drawing ] );

	brEngineFree( _engine );

	_engine = NULL;

	[interfaceController updateObjectSelection];
}

- (int)startSimulationWithText:(char*)buffer withFilename:(char*)name withSavedSimulationFile:(char*)saved fullscreen:(BOOL)full {
	[_engineLock lock];

	if(runState != BX_STOPPED) return -1;

	[self initEngine];

	if(!name) name = "<untitled>";
	else {
		NSString *inputDir = [[NSString stringWithCString: name] stringByDeletingLastPathComponent];

		if(useSimDirForOutput) brEngineSetIOPath(_engine, (char*)[inputDir cString]);
	}

	[displayView setEngine: _engine fullscreen: full];

	if(full) {
		if(![displayView startFullScreen]) {
			NSLog(@"error switching to fullscreen!\n");
			return -1;
		}
	} 

	if([self parseText: buffer withFilename: name withSavedSimulationFile: saved] < 0) {
		if(full) [displayView stopFullScreen];
		[_engineLock unlock];
		return -1;
	}

	runState = BX_PAUSE;

	[NSThread detachNewThreadSelector: @selector(runWorld:) toTarget:self withObject:self];

	return 0;
}

- (void)go {
	runState = BX_RUN;
	[_engineLock unlock];
}

/* step 1 of setting up the simulation, parse the program */

- (int)parseText:(char*)buffer withFilename:(char*)filename withSavedSimulationFile:(char*)saved {
	char *name = NULL;
	int result;
	brInstance *controller;

	if(filename) name = slStrdup(filename);
	else name = slStrdup("<untitled>");
	
	if(saved) result = brLoadSavedSimulation( _engine, buffer, name, saved );
	else result = brLoadSimulation( _engine, buffer, name );

	controller = brEngineGetController( _engine);

	if(filename) slFree( name );

	return result;
}


- (void)pauseSimulation:sender {
	[_engineLock lock];
	runState = BX_PAUSE;

	_engineWillPause = NO;

	if([displayView isFullScreen]) {
		[displayView pauseFullScreen];
	}
}

- (void)unpauseSimulation:sender {
	if([displayView isFullScreen]) {
		[displayView unpauseFullScreen];
	}

	runState = BX_RUN;
	[_engineLock unlock];
}

- (void)stopSimulation {
	if([displayView isFullScreen]) [displayView stopFullScreen];

	if( runState == BX_PAUSE ) [self unpauseSimulation: self];

	if(simNib) {
		[simNib release];
		simNib = NULL;
	}

	runState = BX_STOPPED;

	// is this necessary? 

	[_engineLock lock];
	[_engineLock unlock];

	// wait for the thread to be truely exited before continuing 

	[_threadLock lock];
	[_threadLock unlock];

	[displayView setEngine: NULL fullscreen: NO];

	runState = BX_STOPPED;
}

- (int)runCommand:(char*)command {
	if( !_engine || runState == BX_STOPPED ) return -1;

	if(runState == BX_RUN) [_engineLock lock];
	slMessage(DEBUG_ALL, "> %s\n", command);
	stRunSingleStatement( _steveData, _engine, command);
	if(runState == BX_RUN) [_engineLock unlock];

	return 0;
}

- (int)checkSyntax:(char*)buffer withFilename:(char*)name {
	int value;

	[self initEngine];

	value = [self parseText: buffer withFilename: name withSavedSimulationFile: NULL];

	if(value == EC_OK) return 0;

	return -1;
}

- (brEngine*)getEngine {
	if( !_engine ) return NULL;
	return _engine;
}

- (int)getRunState {
	return runState;
}

- (void)runWorld:sender {
	NSRect nothing = NSMakeRect( 0, 0, 1, 1 );
	id pool;

	int frame = 0;

	int result;

	[_threadLock lock];

	pool = [[NSAutoreleasePool alloc] init];

	while(runState != BX_STOPPED) {
		[_engineLock lock];

		if(runState == BX_RUN) {
			if(1) {
				NSPoint mouse = [NSEvent mouseLocation];

				if(![displayView isFullScreen]) {
					mouse = [[displayView window] convertScreenToBase: mouse];
					mouse = [[[displayView window] contentView] convertPoint: mouse toView: displayView];
				}

				brEngineSetMouseLocation(_engine, (int)mouse.x, (int)mouse.y);
			}

			if((result = brEngineIterate(_engine)) != EC_OK) {
				[_engineLock unlock];

				if([displayView isFullScreen]) [displayView stopFullScreen];

				if(result == EC_ERROR || result == EC_ERROR_HANDLED) [interfaceController runErrorWindow];

				[interfaceController stopSimulation: self];
			} else {
				// if we are recording a movie, we must catch every frame 

				if([displayView isFullScreen]) {
					[displayView drawFullScreen];
				} else if(displayMovie || brEngineGetDrawEveryFrame(_engine)) {
					[displayView lockFocus];
					[displayView drawRect: nothing];
					[displayView unlockFocus];
				} else {
					frame++;

					// force a frame draw periodically to keep everything moving along

					if((frame % 45) == 0) {
						[displayView lockFocus];
						[displayView drawRect: nothing];
						[displayView unlockFocus];
					} else [displayView setNeedsDisplay: YES];

				}
			}
		}

		[_engineLock unlock];

		// A tiny little sleep here will help ensure that other threads get a shot
		// at taking the lock on a multiprocessor system. 

		if(speedFactor > 1) usleep( (int)( speedFactor * 10000 ) );
		else usleep( 50 );

		// while(_engineWillPause) usleep(10000);
	}

	[displayView setNeedsDisplay: YES];

	while( [displayView drawing] );

	[self freeEngine];

	[_threadLock unlock];

	[pool release];

	[NSThread exit];
}

- (void)doSelectionAt:(NSPoint)p {
	slCamera *c;
	unsigned int x, y;

	if( !_engine ) return;

	c = brEngineGetCamera(_engine);

	if(runState == BX_RUN) [_engineLock lock];
	c->getBounds( &x, &y );
	brClickAtLocation(_engine, (int)p.x, (int)(y - p.y) );
	[interfaceController updateObjectSelection];
	if(runState == BX_RUN) [_engineLock unlock];
}

- (BOOL)startMovie {
	NSString *saveName;

	int fps;
	int quality;

	if(displayMovie) return TRUE;

	if(runState == BX_RUN) [_engineLock lock];

	saveName = [interfaceController saveNameForType: @"mov" withAccView: movieSaveAccView];

	if(!saveName) {
		if(runState == BX_RUN) [_engineLock unlock];
		return FALSE;
	}

	fps = [movieFrameNumberText intValue];   
	quality = [movieQualityPopup indexOfSelectedItem]; 

	displayMovie = [[slMovieRecord alloc] initWithPath: saveName bounds: [displayView bounds] timeScale: fps qualityLevel: quality];

	[displayView setMovie: (id)displayMovie];

	if(runState == BX_RUN) [_engineLock unlock];

	return TRUE;
}

- (void)stopMovie {
	if(runState == BX_RUN) [_engineLock lock];

	[displayView setMovie: NULL];
	while([displayView drawing]);

	[displayMovie closeMovie];
	[displayMovie release];
	displayMovie = NULL;

	if(runState == BX_RUN) [_engineLock unlock];
}


- (brInstance*)getSelectedInstance {
	brMethod *method;
	brEval result;
	brInstance *i, *controller;

	if( !_engine ) return NULL;

	controller = brEngineGetController(_engine);

	method = brMethodFind(controller->object, "get-selection", NULL, 0);
 
	if(method) {
		if(brMethodCall(controller, method, NULL, &result) == EC_OK) {
			if( result.type() == AT_INSTANCE ) {
				i = BRINSTANCE(&result);

				if(i && i->status == AS_ACTIVE) return i;
			}
		} 
	}

	brMethodFree(method);

	return NULL;
}

- (void)interfaceActionTag:(int)tag stringValue:(NSString*)s {
	if( _engine ) {
		[_engineLock lock];
		brInterfaceCallback(_engine, tag, (char*)[s cString]);
		[_engineLock unlock];
	}
}

/* +++ CALLBACKS +++ */

char *getLoadNameCallback() {
	NSString *s;

	s = [(id)gInterfaceController loadNameForTypes: [NSArray arrayWithObjects: @"xml", @"tzxml", @"tzb", NULL] withAccView: NULL];

	if(s) return slStrdup((char*)[s cString]);

	return NULL;
}

char *getSaveNameCallback() {
	NSString *s;

	s = [(id)gInterfaceController saveNameForType: @"tzxml" withAccView: NULL];

	if(s) return slStrdup((char*)[s cString]);

	return NULL;
}

int dialogCallback(char *title, char *message, char *b1, char *b2) {
	  NSString *ts, *ms, *b1s, *b2s;
	  int result;

	  ts = [NSString stringWithCString: title];
	  ms = [NSString stringWithCString: message];
	  b1s = [NSString stringWithCString: b1];
	  b2s = [NSString stringWithCString: b2];

	  result = NSRunAlertPanel(ts, ms, b1s, b2s, NULL);

	  if(result == NSAlertDefaultReturn) return 1;
	  return 0; 
}

int soundCallback() {
	NSBeep();

	return 0;
}

char *interfaceVersionCallback() {
	return "cocoa/2.4";
}

int interfaceSetStringCallback(char *string, int tag) {
	if(simNib)
		return [simNib setString: [NSString stringWithCString: string] forObjectWithTag: tag];

	return 0;
}

int pauseCallback() {
	[mainRunLoop performSelector: @selector(pauseSimulation:) target: mySelf argument: NULL order: 0 modes: [ NSArray arrayWithObject: NSDefaultRunLoopMode ] ];
	return 1;
}

int unpauseCallback() {
	[mainRunLoop performSelector: @selector(unpauseSimulation:) target: mySelf argument: NULL order: 0 modes: [ NSArray arrayWithObject: NSDefaultRunLoopMode ] ];
	return 1;
}

void setNibCallback(char *n) {
	simNib = [[slBreveNibLoader alloc] initWithNib: [NSString stringWithCString: n] andEngine: mySelf];
}

void *newWindowCallback(char *string, void *graph) {
    id windowController;
    
    windowController = [[slGraphWindowController alloc] init];

    [NSBundle loadNibNamed: @"slGraphWindow.nib" owner: windowController];

    [[windowController window] setTitle: [NSString stringWithCString: string]];
    [[windowController graphView] setGraph: graph];

    return windowController;
}

void freeWindowCallback(void *data) {
    [(id)data release];
}

void renderWindowCallback(void *data) {
	[[(id)data graphView] setNeedsDisplay: YES];
}

- (IBAction)changeMovieQuality:sender {
    int selection = [sender indexOfSelectedItem];

    [movieQualityMessageText setStringValue: movieQualityStrings[selection]];
}

int slMakeCurrentContext() {
	[gDisplayView activateContext];
	return 0;
}

@end
