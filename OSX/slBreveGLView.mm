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

#import "slBreveGLView.h"
#import "slBreveApplication.h"
#import "interface.h"
#import "camera.h"
#import "gldraw.h"
#import "image.h"

@implementation slBreveGLView

/*
	+ slBreveGLView.m
	= a NSView subclass which displays breve simulations
*/ 

- (BOOL)acceptsFirstResponder {
	return YES;
}

- (void)initGL {
	if(viewEngine) slInitGL(world, camera);

	/* no padding when we get gl pixels */

	glPixelStorei(GL_PACK_ALIGNMENT, 1);
}

- (BOOL)startFullScreen {
	if(![fullScreenView startFullScreenAtPoint: [self frame].origin]) {
   	 	NSString *message = NSLocalizedStringFromTable(@"Fullscreen Error Message", @"General", @"");
		NSString *title = NSLocalizedStringFromTable(@"Fullscreen Error Title", @"General", @"");

		NSLog(@"error starting full screen mode!\n");
		NSRunAlertPanel(title, message, nil, nil, nil);

		return NO;
	}

	CGLSetCurrentContext([fullScreenView context]);

	fullScreen = YES;
	firstFullScreen = 1;
	[NSApp setFullScreenView: self];

	[self updateSize: self];

	[self initGL];

	return YES;
}

- (void)activateContext {
	if(fullScreen) CGLSetCurrentContext([fullScreenView context]);
	else [[self openGLContext] makeCurrentContext];
}

- (void)stopFullScreen {
	[theController setWindowTitleMessage: NULL];
	fullScreen = NO;
	[NSApp setFullScreenView: NULL];
	[fullScreenView stopFullScreen];

	/* the camera settings got changed to the fullscreen size, we'll change them back*/

	[self updateSize: self];
}

- (void)pauseFullScreen {
	[NSApp setFullScreenView: NULL];
	[theController setWindowTitleMessage: @"fullscreen simulation paused"];
	[fullScreenView pauseFullScreen];
}

- (void)unpauseFullScreen {
	[NSApp setFullScreenView: self];
	[theController setWindowTitleMessage: NULL];
	[fullScreenView unpauseFullScreen];
}

- (BOOL)fullScreen {
	return fullScreen;
}

- (id)initWithFrame:(NSRect)frameRect {
	// the default attribute values from IB don't seem to work on a lot  
	// of machines.  that's dumb.  we'll use our own values here and hope 
	// for the best. 

	NSOpenGLPixelFormatAttribute attribs[] = {
		NSOpenGLPFAAccelerated,
		NSOpenGLPFAWindow,
		NSOpenGLPFADepthSize, (NSOpenGLPixelFormatAttribute)24,
		NSOpenGLPFAAlphaSize, (NSOpenGLPixelFormatAttribute)8,
		NSOpenGLPFAStencilSize, (NSOpenGLPixelFormatAttribute)8,
		(NSOpenGLPixelFormatAttribute)0, (NSOpenGLPixelFormatAttribute)0
	};

	NSOpenGLPixelFormat *format = [[[NSOpenGLPixelFormat alloc] initWithAttributes: attribs] autorelease];
	
	fullScreenView = [[slFullScreen alloc] init];

	drawing = 0;
	drawCrosshair = 0;

	contextEnabled = YES;

	selectionMenu = NULL;

	[self updateSize: self];

	drawLock = [[NSLock alloc] init];

	return [super initWithFrame: frameRect pixelFormat: format];
}

/*!
	\brief update the size of this view.  

	We have to tell our camera about the new size and reallocate our pixelBuffers.
*/

- (void)updateSize:sender {
	NSRect bounds;
	int x, y;

	bounds = [self bounds];

	if(!pixelBuffer || !tempPixelBuffer) {
		pixelBuffer = (unsigned char*)slMalloc( (int)( bounds.size.width * bounds.size.height * 4 ) );
		tempPixelBuffer = (unsigned char*)slMalloc( (int)( bounds.size.width * bounds.size.height * 4 ) );
	} else {
		pixelBuffer = (unsigned char*)slRealloc(pixelBuffer, (int)( bounds.size.width * bounds.size.height * 4 ) );
		tempPixelBuffer = (unsigned char*)slRealloc(tempPixelBuffer, (int)( bounds.size.width * bounds.size.height * 4 ) );
	}

	if(!viewEngine) return;

	if(fullScreen) {
		x = (int)[fullScreenView width];
		y = (int)[fullScreenView height];
	} else {
		x = (int)bounds.size.width;
		y = (int)bounds.size.height;
		[[self openGLContext] makeCurrentContext];
	}

	slCameraSetBounds(camera, x, y);

	glViewport(0, 0, x, y);
}

/*!
	\brief Associate this view with a brEngine in which a simulation will be run.
*/

- (void)setEngine:(brEngine*)e fullscreen:(BOOL)f {
	viewEngine = e;

	if(!e) return;

	camera = brEngineGetCamera(e);
	world = brEngineGetWorld(e);

	if(!world || !camera) {
		e = NULL;
		world = NULL;
		camera = NULL;
		return;
	}

	if(!f) [self initGL];
	[self updateSize: self];

	[[self openGLContext] makeCurrentContext];
	glClearColor(0, 0, 0, 0);
	glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT|GL_STENCIL_BUFFER_BIT);
}

/*!
	+ setMovie:
	= not the greatest interface to tell this view that a movie is recording 
	= and that it should pass it frames.
*/

- (void)setMovie:(id)movie {
	theMovie = movie;
}

/*!
	+ menuForEvent:
	= makes a context sensitive menu for the selected object... 
*/

- menuForEvent:(NSEvent*)theEvent {
	brInstance *i;
	NSString *title;
	NSPoint p;
	id item;

	p = [self convertPoint:[theEvent locationInWindow] fromView:nil];

	[theController doSelectionAt: p];

	if(selectionMenu) [selectionMenu release];
	selectionMenu = NULL;

	i = [theController getSelectedInstance];

	if(!i) return NULL;

	title = [NSString stringWithFormat: @"%s (0x%x)", i->object->name, i];

	selectionMenu = [[NSMenu alloc] init];
	[selectionMenu setAutoenablesItems: NO];

	item = [selectionMenu addItemWithTitle: title action: NULL keyEquivalent: @""];
	[item setEnabled: NO];

	[self updateContextualMenu: selectionMenu withInstance: i];

	return selectionMenu;
}

/*!
	\brief Draws the breve view, by calling slRenderWorld.
*/

- (void)drawRect:(NSRect)r {
	brEngineLock(viewEngine);

	[[self openGLContext] makeCurrentContext];

	drawing = 1;

	if(viewEngine) {
	   	if(!fullScreen) {
			slRenderScene(world, camera, drawCrosshair);
			if(theMovie) [theMovie addFrameFromRGBAPixels: [self updateRGBAPixels]];
		}
	} else {
		glClearColor(0.0, 0.0, 0.0, 1.0);
		glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT|GL_STENCIL_BUFFER_BIT);
	}

	glFlush();

	drawing = 0;

	brEngineUnlock(viewEngine);
}

- (void)drawFullScreen {
	if(fullScreen && viewEngine) {
		brEngineLock(viewEngine);
		CGLSetCurrentContext([fullScreenView context]);

		if(firstFullScreen) {
			glClearColor(0, 0, 0, 1.0);
			glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT|GL_STENCIL_BUFFER_BIT);
			slCameraSetRecompile(camera);
		}

		slRenderScene(world, camera, drawCrosshair);
		firstFullScreen = 0;
		CGLFlushDrawable([fullScreenView context]);
		brEngineUnlock(viewEngine);
	}
}

- (BOOL)isFullScreen {
	return fullScreen;
}

- (int)drawing {
	return drawing;
}

/*
	+ mouseDown:
	= handle mouse input, checking which type of motion is currently selected
*/

- (void)mouseDown:(NSEvent*)theEvent {
	NSPoint p, lastp;
	static NSPoint d;
	int mode;
	BOOL firstTime = YES;

	if(!viewEngine) return;

	if(!fullScreen) lastp = [self convertPoint:[theEvent locationInWindow] fromView:nil];
	else {
		lastp = [theEvent locationInWindow];
	}

	mode = [motionSelector selectedColumn];

	do {
		theEvent = [[self window] nextEventMatchingMask:(NSLeftMouseDraggedMask | NSLeftMouseUpMask)];

		if(!fullScreen) p = [self convertPoint:[theEvent locationInWindow] fromView:nil];
		else {
			p = [theEvent locationInWindow];
		}

		d.x = (p.x - lastp.x);
		d.y = -(p.y - lastp.y);

		lastp = p;

		switch(mode) {
			case 0: /* rotation */
				drawCrosshair = 1;
				slRotateCameraWithMouseMovement(camera, d.x, d.y);
				break;

			case 1: /* zoom */
				drawCrosshair = 1;
				slZoomCameraWithMouseMovement(camera, d.x, d.y);
				break;

			case 2: /* motion */
				drawCrosshair = 1;
				slMoveCameraWithMouseMovement(camera, d.x, d.y);
				break;
			case 3: /* select */
				if(firstTime) {
					[theController doSelectionAt: p];
					firstTime = NO;
					drawCrosshair = 0;
				} else {
					unsigned int x, y;

					[drawLock lock];
					slCameraGetBounds(camera, &x, &y);
					brDragCallback(viewEngine, (int)p.x, (int)(y - p.y));
					[drawLock unlock];
				}

				[self setNeedsDisplay: YES];
				break;
			default:
				break;
		}
		
		slCameraUpdate(camera);
		[self setNeedsDisplay: YES];

	} while([theEvent type] != NSLeftMouseUp);

	drawCrosshair = 0;
}

- (void)keyDown:(NSEvent *)theEvent {
	unichar key;
	NSString *str;

	if([theEvent isARepeat]) return;

	str = [theEvent characters];
	if([str length] != 1) return;
	key = [str characterAtIndex: 0];

	if(!viewEngine) return;

	if(fullScreen && key == '\t') {
		[fullScreenView toggleCursor];
		return;
	}

	if(fullScreen && key == 0x1b) {
		/* are they hitting the escape key?  are they angry at us? */

		[theController simulateRunClick];
		return;
	}

	brEngineLock(viewEngine);
	switch(key) {
		case NSUpArrowFunctionKey:
			brSpecialKeyCallback(viewEngine, "up", 1);
			break;
		case NSLeftArrowFunctionKey:
			brSpecialKeyCallback(viewEngine, "left", 1);
			break;
		case NSDownArrowFunctionKey:
			brSpecialKeyCallback(viewEngine, "down", 1);
			break;
		case NSRightArrowFunctionKey:
			brSpecialKeyCallback(viewEngine, "right", 1);
			break;
		default:
			brKeyCallback(viewEngine, key, 1);
			break;
	}
	brEngineUnlock(viewEngine);
}

- (void)keyUp:(NSEvent *)theEvent {
	unichar key;
	NSString *str;

	if(!viewEngine) return;

	str = [theEvent characters];
	if([str length] != 1) return;
	key = [str characterAtIndex: 0];

	brEngineLock(viewEngine);
	switch(key) {
		case NSUpArrowFunctionKey:
			brSpecialKeyCallback(viewEngine, "up", 0);
			break;
		case NSLeftArrowFunctionKey:
			brSpecialKeyCallback(viewEngine, "left", 0);
			break;
		case NSDownArrowFunctionKey:
			brSpecialKeyCallback(viewEngine, "down", 0);
			break;
		case NSRightArrowFunctionKey:
			brSpecialKeyCallback(viewEngine, "right", 0);
			break;
		default:
			brKeyCallback(viewEngine, key, 0);
			break;
	}
	brEngineUnlock(viewEngine);
}

- (unsigned char*)updateRGBAPixels {
	NSRect bounds = [self bounds];

	if(!pixelBuffer || !tempPixelBuffer) [self updateSize: self];

	glReadPixels(0, 0, (int)bounds.size.width, (int)bounds.size.height, GL_RGBA, GL_UNSIGNED_BYTE, tempPixelBuffer);

	slReversePixelBuffer(tempPixelBuffer, pixelBuffer, (int)bounds.size.width * 4, (int)bounds.size.height);

	return pixelBuffer;
}

- (int)snapshotToFile:(NSString*)filename {
	NSBitmapImageRep *i = [self makeImageRep];
	NSData *image;

	image = [i TIFFRepresentationUsingCompression: NSTIFFCompressionNone factor: 0.0];

	if(!image) {
		[i release];
		return -1;
	}

	[image writeToFile: filename atomically: NO];
	[i release];

	return 0;
}

- (NSBitmapImageRep*)makeImageRep {
	NSBitmapImageRep *i;
	NSRect bounds;
	int x, y;

	bounds = [self bounds];

	x = (int)bounds.size.width;
	y = (int)bounds.size.height;

	i = [NSBitmapImageRep alloc];

	[self updateRGBAPixels];

	[i initWithBitmapDataPlanes: &pixelBuffer
	   pixelsWide: x
	   pixelsHigh: y
	   bitsPerSample: 8
	   samplesPerPixel: 4
	   hasAlpha: YES
	   isPlanar: NO 
	   colorSpaceName: @"NSDeviceRGBColorSpace"
	   bytesPerRow: 0
		bitsPerPixel: 0];

	return i;
}

- (void)setContextMenuEnabled:(BOOL)c {
	contextEnabled = c;
}


- (void)updateContextualMenu:(id)menu withInstance:(brInstance*)i {
	id menuItem;
	unsigned int n;

	if(slStackSize(i->menus) == 0) return;

	[selectionMenu addItem: [NSMenuItem separatorItem]];

	for(n=0;n<slStackSize(i->menus);n++) {
		brMenuEntry *menuEntry = (brMenuEntry*)slStackGet(i->menus, n);

		if(!strcmp(menuEntry->title, "")) {
			[menu addItem: [NSMenuItem separatorItem]];
		} else {
			menuItem = [menu addItemWithTitle: [NSString stringWithCString: menuEntry->title] action: @selector(contextualMenu:) keyEquivalent: @""];
	
			[menuItem setTag: n];
	
			if(contextEnabled == YES) [menuItem setEnabled: menuEntry->enabled];
			else [menuItem setEnabled: NO];

			[menuItem setState: menuEntry->checked]; 
		}
	}   
}

- (void)dealloc {
	[fullScreenView release];
	[super dealloc];
}

- (void)print:(id)sender {
    NSBitmapImageRep *r = [self makeImageRep];
    NSData *imageData;
    NSImage *image;
    NSRect b;

    if(!r) return;

    imageData = [r TIFFRepresentationUsingCompression: NSTIFFCompressionNone factor: 0.0];

    image = [[NSImage alloc] initWithData: imageData];

    b = [self bounds];
    b.origin.x = -b.size.width - 100;
    b.origin.y = -b.size.height - 100;

	// for some reason, the printView size isn't getting set 
	// correctly the first time.

    [printView setBounds: b];
    [printView setFrame: b];
    [printView setImage: image];
    [printView setImageScaling: NSScaleNone];
    [printView setNeedsDisplay: YES];
    [printView setBounds: b];
    [printView setFrame: b];

	b = [printView bounds];

    [printView lockFocus];
    [image lockFocus];
    [image drawAtPoint: NSMakePoint(0, 0) fromRect: [self bounds] operation: NSCompositeCopy fraction: 1.0];
    [image unlockFocus];
    [printView unlockFocus];

    [[NSPrintInfo sharedPrintInfo] setHorizontalPagination: NSAutoPagination];
    [[NSPrintInfo sharedPrintInfo] setVerticalPagination: NSAutoPagination];

    [printView print: sender];
    [r release];
    [image release];
}

@end
