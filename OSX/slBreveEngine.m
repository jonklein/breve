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

void slMakeCurrentContext();

char *interfaceID = "aqua/1.2";

- init {
	engineLock = [[NSRecursiveLock alloc] init];
	threadLock = [[NSRecursiveLock alloc] init];

	speedFactor = 1;

	engineWillPause = NO;

	runState = BX_STOP;
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
	char *classPath; 

	if(engine) return;

	bundlePath = [[NSBundle mainBundle] resourcePath];
	classPath = (char*)[[NSString stringWithFormat: @"%@/classes", bundlePath] cString];

	frontend = brFrontendInit(0, NULL);
	frontend->data = breveFrontendInitData(frontend->engine);
	engine = frontend->engine;

	engine->speedFactor = speedFactor;
	engine->callbackData = interfaceController;
	engine->camera = slNewCamera(400, 400, GL_POLYGON);
	engine->dialogCallback = dialogCallback;
	engine->soundCallback = soundCallback;
	engine->interfaceTypeCallback = interfaceVersionCallback;
	engine->interfaceSetStringCallback = interfaceSetStringCallback;
	engine->interfaceSetNibCallback = setNibCallback;
	engine->newWindowCallback = newWindowCallback;
	engine->freeWindowCallback = freeWindowCallback;
	engine->renderWindowCallback = renderWindowCallback;
	engine->camera->activateContextCallback = slMakeCurrentContext;

	if(outputPath) brEngineSetIOPath(engine, outputPath);

	gDisplayView = displayView;

	engine->getSavename = getSaveNameCallback;
	engine->getLoadname = getLoadNameCallback;

	engine->pauseCallback = pauseCallback;

	brAddSearchPath(engine, classPath);
	brAddSearchPath(engine, (char*)[bundlePath cString]);
	brAddSearchPath(engine, (char*)[NSHomeDirectory() cString]);
}

- (void)freeEngine {
	[displayView setEngine: NULL fullscreen: NO];

	while([displayView drawing]);

	breveFrontendCleanupData(frontend->data);
	brEngineFree(frontend->engine);
	slFree(frontend);

	if(slMemoryAllocated()) {
		slMemoryReport();
		slMessage(DEBUG_ALL, "Freeing manually...\n");
		slUtilMemoryFreeAll();
	}

	engine = NULL;

	[interfaceController updateObjectSelection];
}

- (int)startSimulationWithText:(char*)buffer withFilename:(char*)name withSavedSimulationFile:(char*)saved fullscreen:(BOOL)full {
	[engineLock lock];

	if(runState != BX_STOP) return -1;

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

	if(filename) name = slStrdup(filename);
	else name = slStrdup("<untitled>");
	
	if(saved) result = breveFrontendLoadSavedSimulation(frontend, buffer, name, saved);
	else result = breveFrontendLoadSimulation(frontend, buffer, name);

	if(result == EC_OK) brSetUpdateMenuCallback(engine->controller, updateMenu);

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

	runState = BX_STOP;

	/* is this necessary? */

	[engineLock lock];
	[engineLock unlock];

	/* wait for the thread to be truely exited before continuing */

	[threadLock lock];
	[threadLock unlock];

	[displayView setEngine: NULL fullscreen: NO];
}

- (int)runCommand:(char*)command {
	if(!engine || runState == BX_STOP) return -1;

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

	while(runState != BX_STOP) {
		[engineLock lock];

		if(runState != BX_STOP) {
			if(1 || engine->useMouse) {
				NSPoint mouse = [NSEvent mouseLocation];
				NSRect bounds = [displayView frame];

				if(![displayView isFullScreen]) {
					mouse = [[displayView window] convertScreenToBase: mouse];
					mouse = [[[displayView window] contentView] convertPoint: mouse toView: displayView];
				}

				engine->mouseX = mouse.x;
				engine->mouseY = mouse.y;
			}

			if((result = brEngineIterate(engine)) != EC_OK) {
				[engineLock unlock];

				if([displayView isFullScreen]) [displayView stopFullScreen];

				if(result == EC_ERROR || result == EC_ERROR_HANDLED) [interfaceController runErrorWindow];

				[interfaceController stopSimulation: self];
			} else {
				/* if we are recording, we must catch every frame */

				if([displayView isFullScreen]) {
					[displayView drawFullScreen];
				} else if(displayMovie || engine->drawEveryFrame) {
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
	int selection;

	if(!engine) return;

	if(runState == BX_RUN) [engineLock lock];
	selection = slGlSelect(engine->world, engine->camera, p.x, engine->camera->y - p.y);
	brClickCallback(engine, selection);
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
	brInstance *i;

	if(!engine || !engine->controller) return NULL;

	method = brMethodFind(engine->controller->class, "get-selection", 0);
 
	if(method) {
		if(brMethodCall(engine->controller, method, NULL, &result) == EC_OK) {
			if(result.type = AT_INSTANCE) {
				i = BRINSTANCE(&result);

				if(i && i->status == AS_ACTIVE) return i->pointer;
			}
		} 
	}

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
	return interfaceID;
}

int interfaceSetStringCallback(char *string, int tag) {
	if(simNib) {
		return [simNib setString: [NSString stringWithCString: string] forObjectWithTag: tag];
	}

	return 0;
}

int pauseCallback(void *data) {
	engineWillPause = YES;
	return 1;
}

void setNibCallback(char *n) {
	simNib = [[slBreveNibLoader alloc] initWithNib: [NSString stringWithCString: n] andEngine: mySelf];
}

void *newWindowCallback(char *string, slGraph *graph) {
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

void slMakeCurrentContext() {
	NSLog(@"activating %@\n", gDisplayView);
	[gDisplayView activateContext];
}

@end
