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
//  MyDocument.m
//  OSX2
//
//  Created by jon klein on Fri Nov 29 2002.
//  Copyright (c) 2002  s pi der la nd. All rights reserved.
//

#import "slBreveSourceDocument.h"

@implementation slBreveSourceDocument

- (id)init
{
    [super init];

    if (self) loadText = NULL;

    return self;
}

- (IBAction)reformat:sender {
	char *newtext;
	const char *text = [[steveText string] cString];

	newtext = slFormatText((char*)text);

	[steveText setString: [NSString stringWithCString: newtext]];
}

- (NSString *)windowNibName
{
    return @"slBreveSourceDocument";
}

- (void)windowControllerDidLoadNib:(NSWindowController *) aController
{
    [super windowControllerDidLoadNib:aController];

	if(loadText) {
		[steveText setString: loadText];
		[loadText release];
		loadText = NULL;
	}

	[steveText setAllowsUndo: YES];
	[steveText setLineWrappingEnabled: NO];
	[steveText setFont: [NSFont userFixedPitchFontOfSize: 10.0]];

	[steveText setDocDictionary: [[[NSApplication sharedApplication] delegate] docDictionary]];
	[[[NSApplication sharedApplication] delegate] registerDocument: self];
}

- (BOOL)writeToFile:(NSString *)fileName ofType:(NSString *)type {
	return [[steveText string] writeToFile: fileName atomically: YES];
}

- (BOOL)readFromFile:(NSString *)fileName ofType:(NSString *)docType {
	NSString *s = [[NSString alloc] initWithContentsOfFile: fileName];

	if(!s) return NO;

	if(!inited) {
		loadText = [[NSString stringWithString: s] retain];
	} else {
		[steveText setString: s];
	}

	[s release];

	return YES;
}

- (void)setString:(NSString*)s {
	[steveText setString: s];
}

- (void)close {
	[[[NSApplication sharedApplication] delegate] unregisterDocument: self];
	[super close];
}

- (IBAction)find:sender {
    [steveText find: sender];
}

- (IBAction)findNext:sender {
    [steveText findNext: sender];
}

- (IBAction)findPrevious:sender {
    [steveText findPrevious: sender];
}

- (IBAction)findSelection:sender {
    [steveText findSelection: sender];
}

- (IBAction)scrollToSelection:sender {
    [steveText scrollToSelection: sender];
}

- (NSString*)documentText {
	return [steveText string];
}

- (id)text {
	return steveText;
}

- (id)window {
	return sourceWindow;
}

- (void)textView:(NSTextView *)textView doubleClickedOnCell:(id <NSTextAttachmentCell>)cell inRect:(NSRect)cellFrame atIndex:(unsigned)charIndex {
    // NSLog(@"got double\n");
}


@end
