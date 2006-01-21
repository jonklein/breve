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

//
//  slGLWindowView.m
//  breve
//
//  Created by jon klein on Fri May 02 2003.
//  Copyright (c) 2003 artificial. All rights reserved.
//

#import "slGraphWindowView.h"
#import "glIncludes.h"
// #import "graph.h"

@implementation slGraphWindowView

- (id)initWithFrame:(NSRect)frameRect {
    /* the default attribute values from IB don't seem to work on a lot   */
    /* of machines.  that's dumb.  we'll use our own values here and hope */
    /* for the best. */

    NSOpenGLPixelFormatAttribute attribs[] = {
        NSOpenGLPFAAccelerated,
        NSOpenGLPFADepthSize, (NSOpenGLPixelFormatAttribute)16,
        NSOpenGLPFAMinimumPolicy,
        (NSOpenGLPixelFormatAttribute)0
    };

    NSOpenGLPixelFormat *format = [[[NSOpenGLPixelFormat alloc] initWithAttributes: attribs] autorelease];

	graph = NULL;

    return [super initWithFrame: frameRect pixelFormat: format];
}

- (void)setGraph:(void*)g {
    graph = g;
}

- (void)drawRect:(NSRect)r {
	NSRect bounds = [self bounds];

	[[self openGLContext] makeCurrentContext];

	glViewport( 0, 0, (int)bounds.size.width, (int)bounds.size.height );

	if(!graph) {
    	glClearColor(0.0, 0.0, 0.0, 0.0);
    	glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
	} else {
		// graph->width = bounds.size.width;
		// graph->height = bounds.size.height;
		// slDrawGraph(graph);
	}

	glFlush();
}

@end
