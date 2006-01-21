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
//  slCommandTextField.m
//  breve
//
//  Created by jon klein on Fri Oct 25 2002.
//  Copyright (c) 2002 __MyCompanyName__. All rights reserved.
//

#import "slCommandTextField.h"

@implementation slCommandTextField

- (void)awakeFromNib {
	array = [[NSMutableArray arrayWithCapacity: 100] retain];
	[array addObject: @""];
	last = 0;
	[self setDelegate: self];
}

- (void)textDidChange:(NSNotification *)aNotification {
	[array replaceObjectAtIndex: last withObject: [self stringValue]];
}

- (IBAction)run:sender {
	int n;

	n = [engine runCommand: (char*)[[self stringValue] cString]];

	if(![[array objectAtIndex: last] isEqualTo: @""]) {
		[array addObject: @""];
		last++;
	}

	position = last;

	[self setStringValue: @""];
}

- (void)moveUp:sender {
	printf("moving up\n");
}

- (void)dealloc {
	[array release];
	[super dealloc];
}

- (BOOL)control:(NSControl *)control textView:(NSTextView *)textView doCommandBySelector:(SEL)s {
	NSString *command = NSStringFromSelector(s);

	if([command isEqualTo: @"moveUp:"]) {
		position--;

		if(position < 0) position = 0;

		[self setStringValue: [array objectAtIndex: position]];

		return YES;
	} else if([command isEqualTo: @"moveDown:"]) {
		position++;

		if(position > last) position = last;

		[self setStringValue: [array objectAtIndex: position]];

		return YES;
	} 

	return NO;
}

@end
