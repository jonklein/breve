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

@implementation slBreveGLView

/*
	+ slBreveGLView.m
	= a NSView subclass which displays breve simulations
*/ 

- (BOOL)acceptsFirstResponder {
	return YES;
}

- (void)initGL {
	if(viewEngine) slInitGL(viewEngine->world);

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
	int n;

	/* the default attribute values from IB don't seem to work on a lot   */
	/* of machines.  that's dumb.  we'll use our own values here and hope */
	/* for the best. */

	NSOpenGLPixelFormatAttribute attribs[] = {
		NSOpenGLPFAAccelerated,
		NSOpenGLPFAWindow,
		NSOpenGLPFADepthSize, 24,
		NSOpenGLPFAAlphaSize, 8,
		NSOpenGLPFAStencilSize, 8,
		0,0
	};

	NSOpenGLPixelFormat *format = [[[NSOpenGLPixelFormat alloc] initWithAttributes: attribs] autorelease];
	
	fullScreenView = [[slFullScreen alloc] init];

	[self setAllowsClicks: YES];
	drawing = 0;
	drawCrosshair = 0;

	contextEnabled = YES;

	selectionMenu = NULL;

	[self updateSize: self];

	for(n=0;n<256;n++) keyDown[n] = 0;

	drawLock = [[NSLock alloc] init];

	return [super initWithFrame: frameRect pixelFormat: format];
}

/*!
	\brief update the size of this view.  

	We have to tell our camera about the new size and reallocate our pixelBuffers.
*/

- (void)updateSize:sender {
	NSRect bounds;

	if(!viewEngine) return;

	bounds = [self bounds];

	if(fullScreen) {
		viewEngine->camera->x = [fullScreenView width];
		viewEngine->camera->y = [fullScreenView height];
	} else {
		[[self openGLContext] makeCurrentContext];
		viewEngine->camera->x = bounds.size.width;
		viewEngine->camera->y = bounds.size.height;
	}

	viewEngine->camera->fov = (double)viewEngine->camera->x/(double)viewEngine->camera->y;

	if(!pixelBuffer || !tempPixelBuffer) {
		pixelBuffer = malloc(viewEngine->camera->x * viewEngine->camera->y * 4);
		tempPixelBuffer = malloc(viewEngine->camera->x * viewEngine->camera->y * 4);
	} else {
		pixelBuffer = realloc(pixelBuffer, viewEngine->camera->x * viewEngine->camera->y * 4);
		tempPixelBuffer = realloc(tempPixelBuffer, viewEngine->camera->x * viewEngine->camera->y * 4);
	}

	glViewport(0, 0, viewEngine->camera->x, viewEngine->camera->y);
}

/*!
	\brief Associate this view with a brEngine in which a simulation will be run.
*/

- (void)setEngine:(brEngine*)e fullscreen:(BOOL)f {
	slVector *c;

	viewEngine = e;

	if(!e || !e->world || !e->camera) return;

	if(!f) [self initGL];
	[self updateSize: self];

	[[self openGLContext] makeCurrentContext];
	c = &viewEngine->world->backgroundColor;
	glClearColor(c->x, c->y, c->z, 1.0);
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
	stInstance *i;
	NSString *title;
	NSPoint p;
	id item;

	p = [self convertPoint:[theEvent locationInWindow] fromView:nil];

	[theController doSelectionAt: p];

	if(selectionMenu) [selectionMenu release];
	selectionMenu = NULL;

	i = [theController getSelectedInstance];

	if(!i) return NULL;

	title = [NSString stringWithFormat: @"%s (0x%x)", i->type->name, i];

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
	[drawLock lock];

	[[self openGLContext] makeCurrentContext];

	drawing = 1;

	if(viewEngine) {
	   	if(!fullScreen) {
			slRenderWorld(viewEngine->world, viewEngine->camera, 0, GL_RENDER, drawCrosshair, 0);
			if(theMovie) [theMovie addFrameFromRGBAPixels: [self RGBAPixels]];
		}
	} else {
		glClearColor(0.0, 0.0, 0.0, 1.0);
		glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT|GL_STENCIL_BUFFER_BIT);
	}

	glFlush();

	drawing = 0;

	[drawLock unlock];
}

- (void)drawFullScreen {
	if(fullScreen && viewEngine) {
		pthread_mutex_lock(&viewEngine->lock);
		CGLSetCurrentContext([fullScreenView context]);

		if(firstFullScreen) {
			slVector *c;

			c = &viewEngine->world->backgroundColor;
			glClearColor(c->x, c->y, c->z, 1.0);
			glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT|GL_STENCIL_BUFFER_BIT);
		}

		slRenderWorld(viewEngine->world, viewEngine->camera, firstFullScreen, GL_RENDER, drawCrosshair, 0);
		firstFullScreen = 0;
		CGLFlushDrawable([fullScreenView context]);
		pthread_mutex_unlock(&viewEngine->lock);
	}
}

- (BOOL)isFullScreen {
	return fullScreen;
}

- (int)drawing {
	return drawing;
}

/* 
	+ setAllowsClicks:
	= specify whether this view should respond to mouse input.  the default
	= is yes.
*/

- (void)setAllowsClicks:(BOOL)allows {
	allowClicks = allows;
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
	double startCamX;

	if(allowClicks == NO) return;
	
	if(!viewEngine || !viewEngine->world || !viewEngine->camera) return;

	if(!fullScreen) lastp = [self convertPoint:[theEvent locationInWindow] fromView:nil];
	else {
		lastp = [theEvent locationInWindow];
	}

	startCamX = viewEngine->camera->rx;
	
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
				slRotateCameraWithMouseMovement(viewEngine->camera, d.x, d.y, startCamX);
				break;

			case 1: /* zoom */
				drawCrosshair = 1;
				slZoomCameraWithMouseMovement(viewEngine->camera, d.x, d.y);
				break;

			case 2: /* motion */
				drawCrosshair = 1;
				slMoveCameraWithMouseMovement(viewEngine->camera, d.x, d.y);
				break;
			case 3: /* select */
				if(firstTime) {
					[theController doSelectionAt: p];
					firstTime = NO;
					drawCrosshair = 0;
				} else {
					[drawLock lock];
					brDragCallback(viewEngine, (int)p.x, (int)(viewEngine->camera->y - p.y));
					[drawLock unlock];
				}

				[self setNeedsDisplay: YES];
				break;
			default:
				break;
		}
		
		slUpdateCamera(viewEngine->camera);
		[self setNeedsDisplay: YES];

	} while([theEvent type] != NSLeftMouseUp);

	drawCrosshair = 0;
}

- (void)keyDown:(NSEvent *)theEvent {
	char key;
	NSString *str;

	str = [theEvent characters];
	if([str length] != 1) return;
	key = [str characterAtIndex: 0];

	if(fullScreen && key == '\t') {
		[fullScreenView toggleCursor];
		return;
	}

	if(fullScreen && key == 0x1b) {
		/* are they hitting the escape key?  are they angry at us? */

		[theController simulateRunClick];
		return;
	}

	if(keyDown[(int)key]) return;
	keyDown[(int)key] = 1;

	[theController doKeyEvent: key isDown: 1];
}

- (void)keyUp:(NSEvent *)theEvent {
	char key;
	NSString *str;

	str = [theEvent characters];
	if([str length] != 1) return;
	key = [str characterAtIndex: 0];

	[theController doKeyEvent: key isDown: 0];

	keyDown[(int)key] = 0;
}

- (char*)RGBAPixels {
	NSRect bounds = [self bounds];

	glReadPixels(0, 0, bounds.size.width, bounds.size.height, GL_RGBA, GL_UNSIGNED_BYTE, tempPixelBuffer);

	slReversePixelBuffer(tempPixelBuffer, pixelBuffer, bounds.size.width * 4, bounds.size.height);

	return pixelBuffer;
}

- (int)snapshotToFile:(NSString*)filename {
	NSBitmapImageRep *i;
	NSData *image;
	NSRect bounds;
	unsigned char *planes[3];
	unsigned char *temp;
	int x, y;
	int returnCode = 0;

	bounds = [self bounds];

	x = bounds.size.width;
	y = bounds.size.height;

	planes[0] = slMalloc(x * y);
	planes[1] = slMalloc(x * y);
	planes[2] = slMalloc(x * y);
	temp = slMalloc(x * y);

	/* OpenGL returns the pixels from the bottom row up, while */
	/* NSBitmapImageRep expects them from the top--we have to  */
	/* reverse the directions of each of the buffers */

	glReadPixels(0, 0, x, y, GL_RED, GL_UNSIGNED_BYTE, temp);
	slReversePixelBuffer(temp, planes[0], x, y);

	glReadPixels(0, 0, x, y, GL_GREEN, GL_UNSIGNED_BYTE, temp);
	slReversePixelBuffer(temp, planes[1], x, y);

	glReadPixels(0, 0, x, y, GL_BLUE, GL_UNSIGNED_BYTE, temp);
	slReversePixelBuffer(temp, planes[2], x, y);

	i = [NSBitmapImageRep alloc];

	[i initWithBitmapDataPlanes: planes
	   pixelsWide: x
	   pixelsHigh: y
	   bitsPerSample: 8
	   samplesPerPixel: 3
	   hasAlpha: NO
	   isPlanar: YES
	   colorSpaceName: @"NSDeviceRGBColorSpace"
	   bytesPerRow: 0
	   bitsPerPixel: 0];

	image = [i TIFFRepresentationUsingCompression: NSTIFFCompressionNone factor: 0.0];

	if(image) {
		[image writeToFile: filename atomically: NO];
		returnCode = 0;
	} else {
		returnCode = -1;		
	}

	[i release];

	slFree(planes[0]);
	slFree(planes[1]);
	slFree(planes[2]);
	slFree(temp);

	return returnCode;
}

- (void)setContextMenuEnabled:(BOOL)c {
	contextEnabled = c;
}

- (void)updateContextualMenu:(id)menu withInstance:(stInstance*)i {
	id menuItem;
	brMenuList *menuList;
	int n;

	menuList = &i->breveInstance->menu;

	if(!menuList->count) return;

	[selectionMenu addItem: [NSMenuItem separatorItem]];

	for(n=0;n<menuList->count;n++) {
		if(!strcmp(menuList->list[n]->title, "")) {
			[menu addItem: [NSMenuItem separatorItem]];
		} else {
			menuItem = [menu addItemWithTitle: [NSString stringWithCString: menuList->list[n]->title] action: @selector(contextualMenu:) keyEquivalent: @""];
	
			[menuItem setTag: n];
	
			if(contextEnabled == YES) [menuItem setEnabled: menuList->list[n]->enabled];
			else [menuItem setEnabled: NO];

			[menuItem setState: menuList->list[n]->checked]; 
		}
	}   
}

- (void)dealloc {
	[fullScreenView release];
}


@end
