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
//  slBreveNibLoader.m
//  breve
//
//  Created by jon klein on Sun Jun 16 2002.
//  Copyright (c) 2001 artificial. All rights reserved.
//

#import "slBreveNibLoader.h"
#import "slBreveEngine.h"

@implementation slBreveNibLoader

- init {
	tagDict = [[NSMutableDictionary dictionaryWithCapacity: 20] retain];
    return self;
}

- initWithNib:(NSString*)nib andEngine:(id)e {
	NSString *path, *file;
	NSMutableDictionary *dict;

	[self init];

	engine = e;

	dict = [NSMutableDictionary dictionaryWithCapacity: 5];
	[dict setObject: self forKey: @"NSOwner"];

	path = [nib stringByDeletingLastPathComponent];
	file = [nib lastPathComponent];

	bundle = [NSBundle bundleWithPath: path];

	[bundle loadNibFile: file externalNameTable: dict withZone: [self zone]];

	[self setRecursiveTarget: self andSelector: @selector(action:) forView: [window contentView]];

	return self;
}

- (void)setRecursiveTarget:(id)target andSelector:(SEL)s forView:(NSView*)v {
    NSArray *subviews;
    unsigned int n;
    
	if([v isKindOfClass: [NSControl class]] || [v isKindOfClass: [NSCell class]]) {
		NSString *key = [NSString stringWithFormat: @"%d", [v tag]];

		if(![(NSControl*)v action]) {
			[(NSControl*)v setAction: s];
			[(NSControl*)v setTarget: target];
		}

		[tagDict setObject: v forKey: key];
	}

	if([v isKindOfClass: [NSCell class]]) return;
    
	if([v isKindOfClass: [NSMatrix class]]) subviews = [(NSMatrix*)v cells];
    else subviews = [v subviews];

    for(n=0;n<[subviews count];n++) {
        [self setRecursiveTarget: target andSelector: s forView: [subviews objectAtIndex: n]];
    }
}

- (IBAction)action:sender {
	if([sender isMemberOfClass: [NSMatrix class]]) {
		sender = [sender selectedCell];
	}

	[engine interfaceActionTag: [sender tag] stringValue: [sender stringValue]];
}

- (BOOL)setString:(NSString*)string forObjectWithTag:(int)n {
	id o;
	NSString *key = [NSString stringWithFormat: @"%d", n];

	o = [tagDict objectForKey: key];

	if(o) [o setStringValue: string];

	return NO;
}

- (id)window {
	return window;
}

- (void)dealloc {
	[window orderOut: self];
	[window release];
	[tagDict release];
	[super dealloc];
}

@end
