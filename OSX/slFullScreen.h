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
#import <OpenGL/OpenGL.h>
#import <Carbon/Carbon.h>

@interface slFullScreen : NSObject {
    //The size of the display.
    int height, width;

	CGDirectDisplayID displayID;

	CFDictionaryRef newMode;
    
    //Our OpenGL context.
    CGLContextObj context;
    
    //We use this to save the resolution the monitor was at when we started.
    CFDictionaryRef savedMode;
    
    //We're using these to store gamma fading info.
    CGGammaValue redMin;
    CGGammaValue redMax;
    CGGammaValue redGamma;
    CGGammaValue greenMin;
    CGGammaValue greenMax;
    CGGammaValue greenGamma;
    CGGammaValue blueMin;
    CGGammaValue blueMax;
    CGGammaValue blueGamma;

	BOOL paused;
	BOOL showCursor;
}

- (id)startFullScreenAtPoint:(NSPoint)np;
- (void)stopFullScreen;

- (int)width;
- (int)height;

- (CGLContextObj)context;

- (void)fadeOut;
- (void)fadeIn;
- (void)setupGamma;

- (void)stopFullScreenFromPause;
- (void)unpauseFullScreen;
- (void)pauseFullScreen;

- (void)toggleCursor;
- (void)hideCursor;
- (void)showCursor;

@end
