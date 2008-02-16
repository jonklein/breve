/*****************************************************************************
 *                                                                           *
 * The breve Simulation Environment                                          *
 * Copyright (C) 2000-2006 Jonathan Klein									 *
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

#import "slBreveSaver.h"

@implementation slBreveSaver

- (void)initGL {
	char *simFile, *simName, *text;
	int result;
		
	glInited = YES;

	if( viewEngine ) 
		brEngineGetCamera( viewEngine ) -> initGL();
	
	simName = [self getSimName];

	if(*simName != '/') {
			simFile = strdup((char*)[[NSString stringWithFormat: @"%@/%s", inputDirectory, simName] cString]);
	} else {
			simFile = strdup(simName);
	}
	
	text = slUtilReadFile(simFile);

	result = brLoadSimulation( viewEngine, text, simFile);

	free(simFile);
	slFree(text);

	if(result != EC_OK) {
		NSLog(@"breveSaver error initializing engine"); 
		return;
	}

	if(displayID != CGMainDisplayID()) {
		[self callControllerMethod: @"disable-save"];
	}
}

- (void)loadSettings {

}

- initWithFrame:(NSRect)frame isPreview:(BOOL)p {
	NSString *myNib;
	NSString *defaultsName;
	NSString *path;
	NSArray *paths;
	CGDisplayCount count;
	CGPoint point;

	point.x = frame.origin.x;
	point.y = frame.origin.x;

	CGGetDisplaysWithPoint(point, 1, &displayID, &count);

	defaultsName = [NSString stringWithFormat: @"org.spiderland.%s", [self getDefaultsName]];

	defaults = [ScreenSaverDefaults defaultsForModuleWithName: defaultsName];

	disableFX = [defaults integerForKey: @"disableEffects"];

	srand(time(NULL));
	srandom(time(NULL));

	self = [super initWithFrame:frame isPreview: p];

	nofx = NO;

	if(self) {
		NSOpenGLPixelFormat *format;

		NSOpenGLPixelFormatAttribute attribs[] = {	   
			NSOpenGLPFADepthSize, (NSOpenGLPixelFormatAttribute)8,
			NSOpenGLPFAStencilSize, (NSOpenGLPixelFormatAttribute)8,
			NSOpenGLPFAWindow,
			NSOpenGLPFAAccelerated,
			NSOpenGLPFAAllRenderers,
			(NSOpenGLPixelFormatAttribute)0
		};

		if(!disableFX) {
			format = [ [ [ NSOpenGLPixelFormat alloc ] initWithAttributes: attribs ] autorelease ];
			theView = [ [ [ NSOpenGLView alloc ] initWithFrame: NSZeroRect pixelFormat: format ] autorelease ]; 
		}

		if(disableFX || !format || !theView) {
			NSOpenGLPixelFormatAttribute attribs[] = {	   
				NSOpenGLPFADepthSize, (NSOpenGLPixelFormatAttribute)4,
				NSOpenGLPFAAccelerated,
				NSOpenGLPFAClosestPolicy,
				(NSOpenGLPixelFormatAttribute)0
			};

			NSOpenGLPixelFormat *format = [ [ [ NSOpenGLPixelFormat alloc ] initWithAttributes: attribs ] autorelease ];

			nofx = YES;

			NSLog(@"breveSaver view initialization failed first attempt");
			theView = [[[NSOpenGLView alloc] initWithFrame:NSZeroRect pixelFormat:format] autorelease]; 

			if(!theView) {
				NSLog(@"breveSaver unable to create OpenGL view");
				return NULL;
			}
		}

		[self addSubview: theView];
	} else {
		NSLog(@"breveSaver error during superview initialization");
	}

	inputDirectory = [[[NSBundle bundleForClass: [self class]] resourcePath] retain];

	glInited = NO;

	if([self initEngineWithFrame: frame]) {
		NSLog(@"breveSaver error initializing engine"); 
		return NULL;
	}

	path = [NSString stringWithFormat: @"%@/classes", inputDirectory];

	brAddSearchPath( viewEngine, slStrdup((char*)[path cString]) ); 

	/* set the output path to the resource dir, and add it to the search path */

	brAddSearchPath( viewEngine, slStrdup((char*)[inputDirectory cString]) ); 
	
	paths = NSSearchPathForDirectoriesInDomains(NSLibraryDirectory, NSUserDomainMask, YES);

	if([paths count] > 0) { 
		outputDirectory = [[[paths objectAtIndex: 0] stringByAppendingString: @"/Preferences/"] retain];
		brEngineSetIOPath( viewEngine, (char*)[outputDirectory cString] );
	}
		
	if( [defaults boolForKey: @"initialized"] != YES) 
		[self saveDefaults];

	myNib = [self getNibName];
	if(myNib) [NSBundle loadNibNamed: myNib owner:self];

	// no output.

	slSetMessageCallbackFunction( NULL );

	return self;
}

- (int)initEngineWithFrame:(NSRect)frame {
	viewEngine = brEngineNew();
	brInitFrontendLanguages( viewEngine );
	
	if( !viewEngine ) return -1;
		
	return 0;
}

- (BOOL)hasConfigureSheet {
	if(configWindow) return YES;
	else return NO;
}
 
- (NSWindow*)configureSheet {
	NSColor *c;

	c = [NSColor colorWithCalibratedRed: [defaults floatForKey: @"cloudRed"]
							  green: [defaults floatForKey: @"cloudGreen"]
							   blue: [defaults floatForKey: @"cloudBlue"]
							  alpha: 1.0];

	[cloudColor setColor: c];

	c = [NSColor colorWithCalibratedRed: [defaults floatForKey: @"skyRed"]
							  green: [defaults floatForKey: @"skyGreen"]
							   blue: [defaults floatForKey: @"skyBlue"]
							  alpha: 1.0];

	[skyColor setColor: c];

	[effectsCheckbox setState: [defaults integerForKey: @"disableEffects"]];

	return configWindow;
}

- (IBAction)openBreveHomepage:sender {
	[[NSWorkspace sharedWorkspace] openURL: [NSURL URLWithString: @"http://www.spiderland.org/breve"]];
}

- (IBAction)closeSheet:(id)sender {
	[self saveSettings];

	[[NSColorPanel sharedColorPanel] close];

	[NSApp endSheet: configWindow];

	[defaults synchronize];
	[self loadSettings];
}

- (void)saveSettings {
	NSColor *c;

	c = [[cloudColor color] colorUsingColorSpaceName: @"NSCalibratedRGBColorSpace"];

	[defaults setFloat: [c redComponent] forKey: @"cloudRed"];
	[defaults setFloat: [c greenComponent] forKey: @"cloudGreen"];
	[defaults setFloat: [c blueComponent] forKey: @"cloudBlue"];

	c = [[skyColor color] colorUsingColorSpaceName: @"NSCalibratedRGBColorSpace"];

	[defaults setFloat: [c redComponent] forKey: @"skyRed"];
	[defaults setFloat: [c greenComponent] forKey: @"skyGreen"];
	[defaults setFloat: [c blueComponent] forKey: @"skyBlue"];

	[defaults setInteger: [effectsCheckbox intValue] forKey: @"disableEffects"];
}

- (void)saveDefaults {
	NSLog(@"saving the defaults\n");

	[defaults setBool: YES forKey: @"initialized"];

	[defaults setFloat: 0.2 forKey: @"skyRed"];
	[defaults setFloat: 0.5 forKey: @"skyGreen"];
	[defaults setFloat: 1.0 forKey: @"skyBlue"];

	[defaults setFloat: 1.0 forKey: @"cloudRed"];
	[defaults setFloat: 1.0 forKey: @"cloudGreen"];
	[defaults setFloat: 1.0 forKey: @"cloudBlue"];

	[defaults setInteger: 0 forKey: @"disableEffects"];

	[defaults synchronize];
}

- (void)setFrameSize:(NSSize)newSize {
	slCamera *camera;
		
	[super setFrameSize:newSize];
	[theView setFrameSize:newSize];

	if( !viewEngine ) return;
	
	camera = brEngineGetCamera( viewEngine );

	camera->setBounds( newSize.width, newSize.height );
}

- (void)startAnimation {
	[self setAnimationTimeInterval: 0.01];

	[super startAnimation];

	glClearColor(0.0, 0.0, 0.0, 1.0);
	glClear(GL_COLOR_BUFFER_BIT);
	glFlush();
}

- (void)stopAnimation {	   
	[super stopAnimation];
	
	return;
} 

- (void)animateOneFrame {
	 brEngineIterate( viewEngine );

	[[theView openGLContext] makeCurrentContext];

	if(!glInited) {
		[self initGL];
		[self setFrameSize: [self frame].size];
		[self startBreveSimulation];

		glClearColor(0.0, 0.0, 0.0, 1.0);
		glClear(GL_COLOR_BUFFER_BIT);
		glFlush();

	}
	
	brEngineRenderWorld( viewEngine, 0 );
	
	glFlush();
}

- (void)startBreveSimulation {
	[self loadSettings];
}

- (void)callControllerMethod:(NSString*)method {
	brEval r;
	
	if( !viewEngine ) return;
	
	brInstance *controller = brEngineGetController( viewEngine );

	if(controller) brMethodCallByName(controller, (char*)[method cString], &r);
}

- (void)dealloc {
	brEngineFree( viewEngine );

	[outputDirectory release];
	[inputDirectory release];
	[theView removeFromSuperview];
	[super dealloc];
}

/* this should be overriden by subclasses */

- (char*)getSimName {
	return "Simulation.tz";
}

- (char*)getDefaultsName {
	return "Simulation";
}

- (NSString*)getNibName {
	return NULL;
}

@end
