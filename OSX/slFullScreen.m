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

#import <ApplicationServices/ApplicationServices.h>
#import <OpenGL/OpenGL.h>
#import <OpenGL/gl.h>
#import <OpenGL/glu.h>

#import "slFullScreen.h"

//Gamma fading speed.
#define kFadeDownStep 0.04
#define kFadeUpStep 0.05

@implementation slFullScreen

- (id)startFullScreenAtPoint:(NSPoint)np {
	CGDisplayErr err;
	CGDisplayCount count;
	CGPoint p;
	int n;

	CFArrayRef modeList;

	p.x = np.x;
	p.y = np.y;

	paused = NO;

	showCursor = YES;

	CGGetDisplaysWithPoint(p, 1, &displayID, &count);

	if(count == 0) {
		displayID = kCGDirectMainDisplay;
	}
	
	modeList = CGDisplayAvailableModes(displayID);
	for(n=0;n<CFArrayGetCount(modeList);n++) {
		// CFDictionaryRef mode = CFArrayGetValueAtIndex(modeList, n);
		// DumpDisplayModeValues(mode);
	}

	//Setup the gamma fading.
	[self setupGamma];
	
	//Fade out
	[self fadeOut];
	
	//This function captures the main display, hiding everything that's
	//not in a special window layer (the shield layer). We're not going
	//to be using this layer, however, because we're going to draw OpenGL
	//directly to the screen. We still have to capture the screen to do this.
	//Also, capturing the screen makes resolution switches synchronous.
	//If we don't capture the display, we need to set a timer, which would
	//be fired after the resolution is done switching so we can be sure our
	//code is called after the resolution is switched.
	CGDisplayCapture(displayID);
	
	//Switch resolutions.
	//Save the current resolution mode.
	savedMode = CGDisplayCurrentMode(displayID);

	width = CGDisplayPixelsWide(displayID);
	height = CGDisplayPixelsHigh(displayID);
	// width = 800;
	// height = 600;
	
	//Get the best display mode with the parameters we want.
	newMode = CGDisplayBestModeForParameters(displayID, 24, width, height, 0);
	// DumpDisplayModeValues(newMode);

	if(!newMode) {
		NSLog(@"Couldn't find display mode.\n");
		return NULL;
	}
	
	//Switch to selected resolution.
	err = CGDisplaySwitchToMode(displayID, newMode);
	
	if(err != CGDisplayNoErr) {
		NSLog(@"Error switching resolution.\n");
		return NULL;
	}

	//Create an OpenGL context to draw into.
	{
		CGLError err;

		//Here we have a list of attributes that describe the OpenGL context we want.
		//These attributes can be in any order, but some attributes must be followed
		//by an integer value (like the last two here).
			CGLPixelFormatAttribute attrs[] = {
				kCGLPFAAllRenderers,
				//We want to use double buffering.
				kCGLPFADoubleBuffer,
				//We want hardware acceleration.
				kCGLPFAAccelerated,
				//We must set this, because we want to run the context full screen.
				kCGLPFAFullScreen,
				//These two attributes set which screen we want to run on.
				kCGLPFADisplayMask, CGDisplayIDToOpenGLDisplayMask(displayID),
				//We want a context with a depth buffer of any size.
				kCGLPFADepthSize, 8,
				//We want a context with a depth buffer of any size.
				kCGLPFAStencilSize, 2,
				kCGLPFASampleBuffers, 1,
				kCGLPFASamples, 4,
				//The list must always end with 0.
				0
		};

		CGLPixelFormatObj pix;
		long numPixFormats;

		//This finds the pixel format that best matches our attributes.
		err = CGLChoosePixelFormat(attrs, &pix, &numPixFormats);

		if(err) {
			NSLog([NSString stringWithFormat:@"Error creating pixel format:%s", CGLErrorString(err)]);

			ShowMenuBar();
			CGDisplaySwitchToMode(displayID, savedMode);
			[self fadeIn];
			CGDisplayRelease(displayID);

			return NULL;
		}

		//Create a new OpenGL context with our pixel format.
		err = CGLCreateContext(pix, nil, &context);

		if(err) {
			NSLog([NSString stringWithFormat:@"Error creating context:%s", CGLErrorString(err)]);

			ShowMenuBar();
			CGDisplaySwitchToMode(displayID, savedMode);
			[self fadeIn];
			CGDisplayRelease(displayID);

			return NULL;
		}

		//Dispose the pixel format.
		CGLDestroyPixelFormat(pix);
   
		//This sets the context to draw directly to the screen. This
		//allows the full potential of your graphics card's potential
		//to kick in, plus we eliminate the overhead of the window
		//server.

		err = CGLSetFullScreen(context);

		if(err) {
			NSLog([NSString stringWithFormat:@"Error going full screen: %s", CGLErrorString(err)]);

			ShowMenuBar();
			CGDisplaySwitchToMode(displayID, savedMode);
			[self fadeIn];
			CGDisplayRelease(displayID);

			return NULL;
		}

		//Set the context to the one that OpenGL commands go to.
		err = CGLSetCurrentContext(context);
	
		if(err) {
			NSLog([NSString stringWithFormat:@"Error setting current context: %s", CGLErrorString(err)]);

			ShowMenuBar();
			CGDisplaySwitchToMode(displayID, savedMode);
			[self fadeIn];
			CGDisplayRelease(displayID);

			return NULL;
		}

		if(err) return NULL;
	}
	
	HideMenuBar();
	glViewport(0, 0, width, height);
	
	//Fade back in.
	[self fadeIn];

	return self;
}

- (int)width {
	return width;
}

- (int)height {
	return height;
}

- (CGLContextObj)context {
	return context;
}

- (void)setupGamma {
	//Get the current gamma setting, so we know what the settings should
	//be when we're not faded out at all.
	CGGetDisplayTransferByFormula(displayID,
									&redMin,
									&redMax,
									&redGamma,
									&greenMin,
									&greenMax,
									&greenGamma,
									&blueMin,
									&blueMax,
									&blueGamma);
}

//This method fades the screen out by setting the minimum luminocity
//to 0, and ramping the maximum luminocity towards 0.
- (void)fadeOut {
	float gammaMult;
	
	for (gammaMult = 1.0; gammaMult > 0.0; gammaMult -= kFadeDownStep)
	{
	CGSetDisplayTransferByFormula(displayID,
									0.0,
									redMax * gammaMult,
									redGamma,
									0.0,
									greenMax * gammaMult,
									greenGamma,
									0.0,
									blueMax * gammaMult,
									blueGamma);
	}
}

//Basically reverses -[AppDelegate fadeOut].
- (void)fadeIn {
	float gammaMult;
	
	for (gammaMult = kFadeUpStep; gammaMult <= 1.0; gammaMult += kFadeUpStep)
	{
	CGSetDisplayTransferByFormula(displayID,
									0.0,
									redMax * gammaMult,
									redGamma,
									0.0,
									greenMax * gammaMult,
									greenGamma,
									0.0,
									blueMax * gammaMult,
									blueGamma);
	}

	CGSetDisplayTransferByFormula(displayID,
								0.0,
								redMax,
								redGamma,
								0.0,
								greenMax,
								greenGamma,
								0.0,
								blueMax,
								blueGamma);
}

//Called when the application quits.
- (void)stopFullScreen {				  
	if(paused) {
		[self stopFullScreenFromPause];
		return;
	}

	[self fadeOut];

	if(showCursor == NO) [self showCursor];

	ShowMenuBar();
	CGDisplaySwitchToMode(displayID, savedMode);
	CGLClearDrawable(context);
	CGLDestroyContext(context);
	CGDisplayRelease(displayID);
	
	[self fadeIn];
}

- (void)stopFullScreenFromPause {
	CGLClearDrawable(context);
	CGLDestroyContext(context);
}

- (void)pauseFullScreen {
	paused = YES;
	[self fadeOut];
	CGDisplaySwitchToMode(displayID, savedMode);
	CGLClearDrawable(context);
	CGDisplayRelease(displayID);
	ShowMenuBar();

	if(showCursor == NO) [self showCursor];

	[self fadeIn];
}

- (void)unpauseFullScreen {
	paused = NO;
	[self fadeOut];
	CGDisplayCapture(displayID);
	CGDisplaySwitchToMode(displayID, newMode);
	CGLSetCurrentContext(context);
	CGLSetFullScreen(context);
	HideMenuBar();

	if(showCursor == NO) [self hideCursor];

	[self fadeIn];
}

- (void)toggleCursor {
	if(showCursor == YES) {
		showCursor = NO;
		[self hideCursor];
	} else {
		showCursor = YES;
		[self showCursor];
	}
}

- (void)hideCursor {
	CGDisplayHideCursor(displayID);
}

- (void)showCursor {
	CGDisplayShowCursor(displayID);
}

 
long get_value(CFDictionaryRef values, CFStringRef key) {
	long int_value;
	CFNumberRef number_value = (CFNumberRef) CFDictionaryGetValue(values, key);

	if (!number_value) return -1;
  
	if (!CFNumberGetValue(number_value, kCFNumberLongType, &int_value))
	return -1;

	return int_value;
}

void DumpDisplayModeValues(CFDictionaryRef values) {
	// printf("   ----- Display Mode Info for %d -----\n", get_value(values, kCGDisplayMode));
	// printf("   Bounds = %d x %d\n", get_value(values, kCGDisplayWidth), get_value(values, kCGDisplayHeight));
	// printf("   bpp = %d, hz = %d\n", get_value(values, kCGDisplayBitsPerPixel), get_value(values,  kCGDisplayRefreshRate));
 }

@end
