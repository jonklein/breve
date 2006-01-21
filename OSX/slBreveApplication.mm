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
//  slBreveApplication.m
//  breve
//
//  Created by jon klein on Mon Jun 24 2002.
//  Copyright (c) 2001 artificial. All rights reserved.
//

#import "slBreveApplication.h"


@implementation slBreveApplication

- (void)sendEvent:(NSEvent*)e {
    if(fullScreenView && [e type] == NSLeftMouseDown) {
        [fullScreenView mouseDown: e];
		return;
    } else if(fullScreenView && [e type] == NSLeftMouseUp) {
        [fullScreenView mouseUp: e];
		return;
	}
    
    [super sendEvent: e];
}

- (void)setFullScreenView:(id)f {
    fullScreenView = f;
}

- (NSString*)fullscreen {
	NSLog(@"getting full from app\n");
	return @"puppy";
}

- (id)startSimulationFromScript:sender {
	NSLog(@"starting simulation...\n");
	[[self delegate] startSimulation];

	return NULL;
}

- (id)stopSimulationFromScript:sender {
	NSLog(@"stopping simulation...\n");
	[[self delegate] stopSimulation: self];

	return NULL;
}

- (void)setFullscreen:(NSString*)s {
	NSLog(@"application setting fullscreen\n");
	[[self delegate] setFullScreen: [s intValue]];
}

- (id)sleepFromScript:sender {
	sleep(30);
	return NULL;
}

@end
