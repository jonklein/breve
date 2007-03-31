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

#import <Foundation/Foundation.h>
#import <Cocoa/Cocoa.h>

#import "slutil.h"

@interface slTextView : NSTextView {
    id findText;
    id findCaseSwitch;
    id findWindow;

    id thisWindow;

    id thePopupMenu;

    id lineNumberBox;

	NSDictionary *docDictionary;

    NSMutableArray *methodMenuArray;

    unsigned int tabSpaceCount;

    BOOL wholeFileNextColor;

    NSColor *typeColor;
    NSColor *numberColor;
    NSColor *stringColor;
    NSColor *commentColor;
    NSColor *backgroundColor;
    NSColor *textColor;

    BOOL shouldDoSyntaxColoring;
    BOOL shouldAutoIndent;
    BOOL shouldUpdateLineNumber;
    BOOL shouldMatchBraces;
    BOOL shouldExtraIndent;
    BOOL shouldUseSpaces;

	NSMenu *currentMenu;
	NSArray *currentDocArray;
	NSString *currentDocString;
}

/* find methods */

- (IBAction)find:sender;
- (IBAction)findNext:sender;
- (IBAction)findPrevious:sender;
- (IBAction)findSelection:sender;
- (IBAction)scrollToSelection:sender;
- (IBAction)goToMethod:sender;
- (int)goToLine:(int)line;

/* the IB methods to comment and uncomment */

- (IBAction)uncommentSelection:sender;
- (IBAction)commentSelection:sender;

/* the function that does the commenting */

- (void)modifySelectionWithComments:(BOOL)comments;

/* options */

- (void)setLineWrappingEnabled:(BOOL)l;

- (void)setTabSpaces:(int)spaces;

- (void)setShouldAutoIndent:(BOOL)setting;
- (void)setShouldDoSyntaxColoring:(BOOL)setting;
- (void)setShouldAutoIndent:(BOOL)setting;
- (void)setShouldUpdateLineNumber:(BOOL)setting;
- (void)setShouldMatchBraces:(BOOL)setting;
- (void)setShouldExtraIndent:(BOOL)setting;
- (void)setShouldUseSpaces:(BOOL)setting;

- (BOOL)control:(NSControl *)control textView:(NSTextView *)textView doCommandBySelector:(SEL)command;

- (void)setPopupMenu:(NSNotification*)n;

/* syntax coloring */

- (void)syntaxColorEntireFile:(BOOL)wholeFile;
- (NSColor*)colorForToken:(char*)string;
- (void)setStringColor:(NSColor*)c;
- (void)setTypeColor:(NSColor*)c;
- (void)setNumberColor:(NSColor*)c;
- (void)setCommentColor:(NSColor*)c;
- (void)setTextColor:(NSColor*)c;

- (void)setDocDictionary:(id)d;

- (NSMenu*)updateContextualMenu;

@end

