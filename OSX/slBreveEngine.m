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

#import "slBreveEngine.h"
#import "slBreveNibLoader.h"
#import "slGraphWindowController.h"
#import "slGraphWindowView.h"

id simNib;
id mySelf;

id gDisplayView;

BOOL engineWillPause;

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
	engineLock = [[NSRecursiveLock alloc] init];
	threadLock = [[NSRecursiveLock alloc] init];

	speedFactor = 1;

	engineWillPause = NO;

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

- (void)setOutputPath:(char*)path {
	outputPath = path;
}

- (void)lock {
	[engineLock lock];
}

- (void)unlock {
	[engineLock unlock];
}

- (void)initEngine {
	NSString *bundlePath; 
	char *classPath, *pluginPath;
	slCamera *camera;

	if(engine) return;

	bundlePath = [[NSBundle mainBundle] resourcePath];
	classPath = (char*)[[NSString stringWithFormat: @"%@/classes", bundlePath] cString];
	pluginPath = (char*)[[NSString stringWithFormat: @"%@/plugins", bundlePath] cString];

	frontend = breveFrontendInit(0, NULL);
	frontend->data = breveFrontendInitData(frontend->engine);
	engine = frontend->engine;

	brEngineSetSoundCallback(engine, soundCallback);
	brEngineSetDialogCallback(engine, dialogCallback);

	// engine->newWindowCallback = newWindowCallback;
	// engine->freeWindowCallback = freeWindowCallback;
	// engine->renderWindowCallback = renderWindowCallback;

	camera = brEngineGetCamera(engine);
	slCameraSetActivateContextCallback(camera, slMakeCurrentContext);

	if(outputPath) brEngineSetIOPath(engine, outputPath);

	gDisplayView = displayView;

	brEngineSetInterfaceInterfaceTypeCallback(engine, interfaceVersionCallback);
	brEngineSetInterfaceSetStringCallback(engine, interfaceSetStringCallback);
	brEngineSetInterfaceSetNibCallback(engine, setNibCallback);

	brEngineSetGetSavenameCallback(engine, getSaveNameCallback);
	brEngineSetGetLoadnameCallback(engine, getLoadNameCallback);
	brEngineSetPauseCallback(engine, pauseCallback);

	brAddSearchPath(engine, classPath);
	brAddSearchPath(engine, pluginPath);
	brAddSearchPath(engine, (char*)[bundlePath cString]);
	brAddSearchPath(engine, (char*)[NSHomeDirectory() cString]);
}

- (void)freeEngine {
	[displayView setEngine: NULL fullscreen: NO];

	while([displayView drawing]);

	brEngineFree(frontend->engine);
	breveFrontendCleanupData(frontend->data);
	slFree(frontend);

	engine = NULL;

	[interfaceController updateObjectSelection];
}

- (int)startSimulationWithText:(char*)buffer withFilename:(char*)name withSavedSimulationFile:(char*)saved fullscreen:(BOOL)full {
	[engineLock lock];

	if(runState != BX_STOPPED) return -1;

	[self initEngine];

	if(!name) name = "<untitled>";

	[displayView setEngine: engine fullscreen: full];

	if(full) {
		if(![displayView startFullScreen]) {
			NSLog(@"error switching to fullscreen!\n");
			return -1;
		}
	} 

	if([self parseText: buffer withFilename: name withSavedSimulationFile: saved] < 0) {
		if(full) [displayView stopFullScreen];
		[engineLock unlock];
		return -1;
	}

	runState = BX_PAUSE;

	[NSThread detachNewThreadSelector: @selector(runWorld:) toTarget:self withObject:self];

	return 0;
}

- (void)go {
	runState = BX_RUN;
	[engineLock unlock];
}

/* step 1 of setting up the simulation, parse the program */

- (int)parseText:(char*)buffer withFilename:(char*)filename withSavedSimulationFile:(char*)saved {
	char *name = NULL;
	int result;
	brInstance *controller;

	if(filename) name = slStrdup(filename);
	else name = slStrdup("<untitled>");
	
	if(saved) result = breveFrontendLoadSavedSimulation(frontend, buffer, name, saved);
	else result = breveFrontendLoadSimulation(frontend, buffer, name);

	controller = brEngineGetController(engine);

	if(result == EC_OK) brSetUpdateMenuCallback(controller, updateMenu);

	if(filename) slFree(name);

	return result;
}


- (void)pauseSimulation {
	[engineLock lock];
	runState = BX_PAUSE;

	engineWillPause = NO;

	if([displayView isFullScreen]) {
		[displayView pauseFullScreen];
	}
}

- (void)unpauseSimulation {
	if([displayView isFullScreen]) {
		[displayView unpauseFullScreen];
	}

	runState = BX_RUN;
	[engineLock unlock];
}

- (void)stopSimulation {
	if([displayView isFullScreen] && runState == BX_PAUSE) {
		[displayView stopFullScreen];
	}

	if(runState == BX_PAUSE) [self unpauseSimulation];

	if(simNib) {
		[simNib release];
		simNib = NULL;
	}

	runState = BX_STOPPING;

	// is this necessary? 

	[engineLock lock];
	[engineLock unlock];

	// wait for the thread to be truely exited before continuing 

	[threadLock lock];
	[threadLock unlock];

	[displayView setEngine: NULL fullscreen: NO];

	runState = BX_STOPPED;
}

- (int)runCommand:(char*)command {
	if(!engine || runState == BX_STOPPED || runState == BX_STOPPING) return -1;

	if(runState == BX_RUN) [engineLock lock];
	slMessage(DEBUG_ALL, "> %s\n", command);
	stRunSingleStatement(frontend->data, engine, command);
	if(runState == BX_RUN) [engineLock unlock];

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
	return engine;
}

- (int)getRunState {
	return runState;
}

- (void)runWorld:sender {
	id pool;
	NSRect nothing;

	int frame = 0;

	int result;

	[threadLock lock];

	pool = [[NSAutoreleasePool alloc] init];

	while(runState != BX_STOPPING) {
		[engineLock lock];

		if(runState == BX_RUN) {
			if(1) {
				NSPoint mouse = [NSEvent mouseLocation];

				if(![displayView isFullScreen]) {
					mouse = [[displayView window] convertScreenToBase: mouse];
					mouse = [[[displayView window] contentView] convertPoint: mouse toView: displayView];
				}

				brEngineSetMouseLocation(engine, mouse.x, mouse.y);
			}

			if((result = brEngineIterate(engine)) != EC_OK) {
				[engineLock unlock];

				if([displayView isFullScreen]) [displayView stopFullScreen];

				if(result == EC_ERROR || result == EC_ERROR_HANDLED) [interfaceController runErrorWindow];

				[interfaceController stopSimulation: self];
			} else {
				// if we are recording a movie, we must catch every frame 

				if([displayView isFullScreen]) {
					[displayView drawFullScreen];
				} else if(displayMovie || brEngineGetDrawEveryFrame(engine)) {
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

				if(speedFactor > 1) usleep(speedFactor * 10000);
			}
		}

		[engineLock unlock];

		while(engineWillPause) {
			usleep(10000);
		}
	}

	[displayView setNeedsDisplay: YES];

	while([displayView drawing]);

	[self freeEngine];

	[engineLock unlock];
	[threadLock unlock];

	[pool release];

	[NSThread exit];
}

- (void)doSelectionAt:(NSPoint)p {
	slCamera *c;
	int x, y;

	if(!engine) return;

	c = brEngineGetCamera(engine);

	if(runState == BX_RUN) [engineLock lock];
	slCameraGetBounds(c, &x, &y);
	brClickAtLocation(engine, p.x, y - p.y);
	[interfaceController updateObjectSelection];
	if(runState == BX_RUN) [engineLock unlock];
}

- (void)doKeyEvent:(char)key isDown:(int)d {
	if(!engine) return;

	if(runState == BX_RUN) [engineLock lock];
	brKeyCallback(engine, key, d);
	if(runState == BX_RUN) [engineLock unlock];
}

- (BOOL)startMovie {
	NSString *saveName;

	int fps;
	int quality;

	if(displayMovie) return TRUE;

	if(runState == BX_RUN) [engineLock lock];

	saveName = [interfaceController saveNameForType: @"mov" withAccView: movieSaveAccView];

	if(!saveName) {
		if(runState == BX_RUN) [engineLock unlock];
		return FALSE;
	}

	fps = [movieFrameNumberText intValue];   
	quality = [movieQualityPopup indexOfSelectedItem]; 

	displayMovie = [[slMovieRecord alloc] initWithPath: saveName bounds: [displayView bounds] timeScale: fps qualityLevel: quality];

	[displayView setMovie: (id)displayMovie];

	if(runState == BX_RUN) [engineLock unlock];

	return TRUE;
}

- (void)stopMovie {
	if(runState == BX_RUN) [engineLock lock];

	[displayView setMovie: NULL];
	while([displayView drawing]);

	[displayMovie closeMovie];
	[displayMovie release];
	displayMovie = NULL;

	if(runState == BX_RUN) [engineLock unlock];
}


- (stInstance*)getSelectedInstance {
	brMethod *method;
	brEval result;
	brInstance *i, *controller;

	if(!engine) return NULL;

	controller = brEngineGetController(engine);

	method = brMethodFind(controller->object, "get-selection", NULL, 0);
 
	if(method) {
		if(brMethodCall(controller, method, NULL, &result) == EC_OK) {
			if(result.type = AT_INSTANCE) {
				i = BRINSTANCE(&result);

				if(i && i->status == AS_ACTIVE) return i->userData;
			}
		} 
	}

	brMethodFree(method);

	return NULL;
}

- (void)interfaceActionTag:(int)tag stringValue:(NSString*)s {
	if(engine) {
		[engineLock lock];
		brInterfaceCallback(engine, tag, (char*)[s cString]);
		[engineLock unlock];
	}
}

/* +++ CALLBACKS +++ */

char *getLoadNameCallback(void *data) {
	NSString *s;

	s = [(id)data loadNameForTypes: [NSArray arrayWithObjects: @"tzxml", @"tzb", NULL] withAccView: NULL];

	if(s) return slStrdup((char*)[s cString]);

	return NULL;
}

char *getSaveNameCallback(void *data) {
	NSString *s;

	s = [(id)data saveNameForType: @"tzxml" withAccView: NULL];

	if(s) return slStrdup((char*)[s cString]);

	return NULL;
}

int dialogCallback(void *data, char *title, char *message, char *b1, char *b2) {
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

int soundCallback(void *data) {
	NSBeep();

	return 0;
}

char *interfaceVersionCallback(void *data) {
	return "cocoa/2.0";
}

int interfaceSetStringCallback(char *string, int tag) {
	if(simNib)
		return [simNib setString: [NSString stringWithCString: string] forObjectWithTag: tag];

	return 0;
}

int pauseCallback(void *data) {
	engineWillPause = YES;
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
