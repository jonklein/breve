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

#import "slBreve.h"
#import "slFullScreen.h"

#import "steve.h"
#import "engine.h"

@interface slBreveGLView : NSOpenGLView
{
    brEngine *viewEngine;

    id motionSelector;
    id theMovie;

    char *pixelBuffer;
    char *tempPixelBuffer;

    id theController;
    slFullScreen *fullScreenView;

    int drawCrosshair;
    int drawing;
    BOOL allowClicks;
    BOOL contextEnabled;

    id selectionMenu;

    char keyDown[256];

    BOOL fullScreen;
    BOOL firstFullScreen;

    NSLock *drawLock;

    BOOL noDraw;
}

- (void)initGL;

- (void)setMovie:(id)movie;

- (void)updateSize:sender;

- (void)setEngine:(brEngine*)e fullscreen:(BOOL)f;

- (int)drawing;

- (void)drawRect:(NSRect)aRect;

- (void)mouseDown:(NSEvent*)theEvent;

- (void)setAllowsClicks:(BOOL)allows;

- (char*)RGBAPixels;
- (int)snapshotToFile:(NSString*)filename;

- (void)setContextMenuEnabled:(BOOL)c;
- (void)updateContextualMenu:(id)menu withInstance:(stInstance*)i;

- (BOOL)startFullScreen;
- (void)stopFullScreen;

- (void)pauseFullScreen;
- (void)unpauseFullScreen;

- (BOOL)isFullScreen;
- (void)drawFullScreen;

- (void)activateContext;

@end
