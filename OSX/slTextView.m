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

#import "slTextView.h"

#include <unistd.h>

extern char *slObjectParseText;
extern int slObjectParseLine;

void slObjectParseSetBuffer(char *b);

char *slStripSpaces(char *text);

@implementation slTextView

int slYylex();

/*
    + slTextView.m
    = is a subclass of NSTextView which we use to add a few custom features:
    = 1) ability to use "find"
    = 2) ability to "go to line..."
    = 3) ability to "go to method..."
    =
    = we also make a little hack to make sure this view becomes active
    = when the "go to line..." box is edited.  the default behavior for 
    = the NSTextField is to select itself as the active view when "enter"
    = is pressed, so we override this to make it look like a tab was
    = pressed.  we do this not as the object itself, but instead as the
    = object's delegate.
*/

- initWithFrame:(NSRect)frame textContainer:(NSTextContainer*)container {
    id result;

    result = [super initWithFrame: frame textContainer: container];

    if(!result) return NULL;

    tabSpaceCount = 4;

    /* set up the default editor colors */

    stringColor = [[NSColor greenColor] retain];
    commentColor = [[NSColor grayColor] retain];
    numberColor = [[NSColor blueColor] retain];
    typeColor = [[NSColor redColor] retain];
    backgroundColor = [[NSColor whiteColor] retain];
    textColor = [[NSColor blackColor] retain];

    /* turn on the fun features */

    [self setShouldDoSyntaxColoring: YES];
    [self setShouldAutoIndent: YES];
    [self setShouldUpdateLineNumber: YES];
    [self setShouldMatchBraces: YES];
    [self setShouldExtraIndent: YES];
    [self setShouldUseSpaces: YES];

    /* create a mutable array to connect a menu item in the method menu with a line number */

    methodMenuArray = [NSMutableArray arrayWithCapacity: 100];
    [methodMenuArray retain];

    return result;
}

- (void)awakeFromNib {
    /* we'll customize the menu every time it's pushed down and about to popup */

    [[NSNotificationCenter defaultCenter] addObserver: self selector: @selector(setPopupMenu:) name: @"NSPopUpButtonWillPopUpNotification" object: thePopupMenu];
}

- (BOOL)validateMenuItem:(id <NSMenuItem>)menuItem {
	if(currentMenu) {
		[self updateContextualMenu];
		currentMenu = NULL;
	}

	return [super validateMenuItem: menuItem];
}

- (NSMenu*)menuForEvent:(NSEvent*)theEvent {
    currentMenu = [[super menuForEvent: theEvent] copy];

	return currentMenu;
}

- (NSMenu*)updateContextualMenu {
    NSRange range;
	unsigned int n;

	if(currentDocArray) [currentDocArray release];
	if(currentDocString) [currentDocString release];

    range = [self selectedRange];
    currentDocString = [[[self string] substringWithRange: range] retain];
	currentDocArray = [[docDictionary objectForKey: currentDocString] retain];

    if(!currentDocArray) return NULL;

    [currentMenu addItem: [NSMenuItem separatorItem]];

	for(n=0;n<[currentDocArray count];n++) {
    	[currentMenu addItemWithTitle: [NSString stringWithFormat: @"%@ (%@)\n", currentDocString, [currentDocArray objectAtIndex: n]] action: @selector(openDocs:) keyEquivalent: @""];
		[[currentMenu itemAtIndex: n] setTag: n];
	}

    return currentMenu;
}

- (void)setLineWrappingEnabled:(BOOL)l {
	if(!l) {
		[[self textContainer] setContainerSize: NSMakeSize(10000000, 10000000)];
		[[self textContainer] setWidthTracksTextView: NO];
		[[self textContainer] setHeightTracksTextView: NO];

		[self setHorizontallyResizable: YES];
		[self setVerticallyResizable: YES];
		[self setMaxSize:NSMakeSize(10000000, 10000000)];
	} else {
		NSSize size, csize;

		[self setHorizontallyResizable: NO];
		[self setVerticallyResizable: YES];

		[[self textContainer] setWidthTracksTextView: YES];
		[[self textContainer] setHeightTracksTextView: NO];

		size = [[self enclosingScrollView] documentVisibleRect].size;
		size.height = [self bounds].size.height;
		[self setFrameSize: size];

		csize = [[self textContainer] containerSize];
		csize.width = size.width;
		[[self textContainer] setContainerSize: csize];
	}
}

- (void)setTabSpaces:(int)spaces {
    tabSpaceCount = spaces;
}

- (void)setShouldAutoIndent:(BOOL)setting {
    shouldAutoIndent = setting;
}

- (void)setShouldDoSyntaxColoring:(BOOL)setting {
    NSRange r;

    if(setting == NO) {
        r.location = 0;
        r.length = [[self string] length];
        [self setTextColor: textColor range: r];
    } else [self syntaxColorEntireFile: YES];

    shouldDoSyntaxColoring = setting;
}

- (void)setShouldUpdateLineNumber:(BOOL)setting {
    shouldUpdateLineNumber = setting;
}

- (void)setShouldMatchBraces:(BOOL)setting {
    shouldMatchBraces = setting;
}

- (void)setShouldExtraIndent:(BOOL)setting {
    shouldExtraIndent = setting;
}

- (void)setShouldUseSpaces:(BOOL)setting {
    shouldUseSpaces = setting;
}

/* we sure do... */

- (BOOL)allowsUndo {
    return YES;
}

- (int)goToLine:(int)line {
    int current = 1;
    NSRange r;

    /* get the data as a c string, fill out an NSRange corresponding to the desired line... */

    char *data = (char*)[[self string] cString];

    if(*data == 0) return 1;

    r.location = r.length = 0;

    /* find the start of the desired line */

    while(data[r.location] && current < line) {
        if(data[r.location] == '\n') current++;
        r.location++;
    }   

    if(!data[r.location]) {
        /* if we're at \0, then we've reached the end of the string without */
        /* finding enough lines--step backwards to select the final line.   */

        while(r.location && data[r.location--] != '\n') r.length++;
    } else {
        /* if we're here, then we've found the line we want--step forward  */
        /* until we find a newline or the end of the string */

        while(data[r.location + r.length] && data[r.location + r.length] != '\n')
            r.length++;
    }

    [self setSelectedRange: r];
    [self scrollRangeToVisible: r];
    [lineNumberBox setIntValue: line];
    [thisWindow makeFirstResponder: self];

    return current;
}

- (IBAction)findSelection:sender {
	NSString *s = [[self string] substringWithRange: [self selectedRange]];

	[findText setStringValue: s];
	[self findNext: self];
}

- (IBAction)scrollToSelection:sender {
	NSRange r = [self selectedRange];
    [self scrollRangeToVisible: r];
}

- (IBAction)find:sender {
	[findWindow makeKeyAndOrderFront: self];
}

- (IBAction)findNext:sender {
    NSRange range;
    NSString *findString;
    NSRange validRange;
    int ignoreCase, options;

    [findWindow performClose: self];
    [[self window] makeKeyAndOrderFront: self];
    [[self window] makeFirstResponder: self];

    findString = [findText stringValue];
    if([findString isEqualToString: @""]) return;

    ignoreCase = [findCaseSwitch intValue];

    validRange = [self selectedRange];

    /* if there is something already selected, just look at the rest of the string */

    if(validRange.length != 0) {
        validRange.location = validRange.length + validRange.location;
        validRange.length = [[self string] length] - validRange.location;
    } else {
        validRange.location = 0;
        validRange.length = [[self string] length];
    }

    options = 0;
    if(ignoreCase) options |= NSCaseInsensitiveSearch;

    range = [[self string] rangeOfString: findString options: options range: validRange];

    if(range.length == 0) {
		/* nothing found--try the whole string */

		validRange.location = 0;
        validRange.length = [[self string] length];

    	range = [[self string] rangeOfString: findString options: options range: validRange];

		if(range.length == 0) {
        	NSBeep();
        	return;
		}
    }

    [self setSelectedRange: range];
    [self scrollRangeToVisible: range];

    return;
}

- (IBAction)findPrevious:sender {
    NSRange range;
    NSString *findString;
    NSRange validRange;
    int ignoreCase, options;

    [findWindow performClose: self];
    [[self window] makeKeyAndOrderFront: self];
    [[self window] makeFirstResponder: self];

    findString = [findText stringValue];
    if([findString isEqualToString: @""]) return;

    ignoreCase = [findCaseSwitch intValue];

    validRange = [self selectedRange];

    /* if there is something already selected, just look at the rest of the string */

    if(validRange.length != 0) {
        validRange.length = validRange.location;
        validRange.location = 0;
    } else {
        validRange.location = 0;
        validRange.length = [[self string] length];
    }

    options = NSBackwardsSearch;
    if(ignoreCase) options |= NSCaseInsensitiveSearch;

    range = [[self string] rangeOfString: findString options: options range: validRange];

    if(range.length == 0) {
		/* nothing found--try the whole string */

		validRange.location = 0;
        validRange.length = [[self string] length];

    	range = [[self string] rangeOfString: findString options: options range: validRange];

		if(range.length == 0) {
        	NSBeep();
        	return;
		}
    }

    [self setSelectedRange: range];
    [self scrollRangeToVisible: range];

    return;
}

/* if we're sent a goToMethod: message by the method popup menu, we get the index */
/* of the menu item selected, we look up the line number in the menu array, and   */
/* then skip to the desired line. */

- (IBAction)goToMethod:sender {
    int line;

    line = [[methodMenuArray objectAtIndex: [sender indexOfSelectedItem]] intValue];

    [self goToLine: line];
    [thisWindow makeFirstResponder: self];
}

/* catch the TextField's "return" and turn it to a "tab".  this is not for *self*, but   */
/* instead for the "go to line..." text field. */

- (BOOL)control:(NSControl *)control textView:(NSTextView *)textView doCommandBySelector:(SEL)s {
    NSString *command;

    command = NSStringFromSelector(s);

    [lineNumberBox setIntValue: [self goToLine: [lineNumberBox intValue]]];

    if([command isEqualToString: @"insertNewline:"]) {
        [textView insertTab: self];
        return YES;
    } 

    return NO;
}   

- (void)insertNewline:id {
    char c;
    BOOL firstChar = YES;
    NSString *string = [self string];
    NSRange range = [self selectedRange];

    int n = range.location - 1;

    int spaces = 0, tabs = 0, extraTabs = 0;

    if(!shouldAutoIndent) {
        [super insertNewline: self];
        return;
    }

    /* step backwards to find how many spaces there were after  */
    /* the previous newline, so that we know how far to indent. */
    /* if we see certain characters, we'll indent even further. */

    while(n > 0 && (c = [string characterAtIndex: n]) != '\n') {
        switch(c) {
            case ' ': 
                spaces++;
                break;
            case '\t': 
                tabs++;
                break;
            case ':':
            case '{':
                if(firstChar && shouldExtraIndent) extraTabs++;
                break;
            default:
                firstChar = NO;
                spaces = 0;
                tabs = 0;
                break;
        }

        n--;
    }

    tabs += extraTabs;

    if(shouldUseSpaces) {
        spaces += (tabs * tabSpaceCount);
        tabs = 0;
    }

    [super insertNewline: self];

    for(n=0;n<tabs;n++) {
        [self insertTab: self];
    }

    for(n=0;n<spaces;n++) {
        [self insertText: @" "];
    }
}

- (void)deleteBackward:(id)sender {
    NSRange range = [self selectedRange];
    NSString *string = [self string];
	int stepsBack, n;
	unichar c;

	n = range.location - 1;

	if(n < 0) {
		/* how should this be handled? let super worry about it */
		[super deleteBackward: sender];
		return;
	}

	if(!shouldUseSpaces || [string characterAtIndex: n] == '\n') {
		[super deleteBackward: sender];
		return;
	}

	while(n > -1 && (c = [string characterAtIndex: n]) != '\n') {
		if(c != ' ') {
			[super deleteBackward: sender];
			return;
		}

		n--;
	}

	/* there's nothing but space to the left--figure out how much */
	/* to delete to get to the previous tabbing */

	stepsBack = (range.location - 1) - n;

	stepsBack %= tabSpaceCount;

	if(stepsBack == 0) stepsBack = tabSpaceCount;

	for(n=0;n<stepsBack;n++) [super deleteBackward: sender];
}

- (void)insertTab:id {
    unsigned int n;

    if(!shouldUseSpaces) {
        [super insertTab: self];
        return;
    }

    for(n=0;n<tabSpaceCount;n++) {
        [self insertText: @" "];
    }
}

/* balance parens and so forth... */

- (void)insertText:(NSString*)s {
    char right = 0, left = 0;
    int depth = 1;
    NSRange start, end;
    BOOL done = NO;

    if(!shouldMatchBraces) {
        [super insertText: s];
        return;
    }

    right = [s characterAtIndex: 0];

    /* do parent balencing */

    if(right == ')' || right == '}' || right == ']') {
        if(right == ')') left = '(';
        if(right == '}') left = '{';
        if(right == ']') left = '[';

        end = [self selectedRange];
        start = [self selectedRange];
        start.location--;

        while(!done) {
            if([[self string] characterAtIndex: start.location] == left) depth--;
            if([[self string] characterAtIndex: start.location] == right) depth++;

            /* start.location is unsigned, so we can't just test for it being less */
            /* than 0... we have to find out when we've just done the 0 test and   */
            /* set done = YES. */

            if(start.location == 0 || depth == 0) done = YES;
            else start.location--;
        }

        start.length = 1;

        [super insertText: s];

        if(depth != 0) {
            NSBeep();
        } else {
            end.location += [s length];

            [self setSelectedRange: start];
            [self scrollRangeToVisible: start];
            [self display];
            usleep(300000);
            [self setSelectedRange: end];
            [self scrollRangeToVisible: end];
        }
    } else {
        [super insertText: s];
    }
}

- (IBAction)commentSelection:sender {
    [self modifySelectionWithComments: YES];
    [self syntaxColorEntireFile: YES];
}

- (IBAction)uncommentSelection:sender {
    [self modifySelectionWithComments: NO];
    [self syntaxColorEntireFile: YES];
}

- (void)modifySelectionWithComments:(BOOL)comments {
    NSString *string = [self string];
    NSRange selection = [self selectedRange], newRange;

    unsigned int n = selection.location;

    if([string length] == 0) return;
    if(selection.length == 0) return;

    /* step back to the beginning of this line */

    while(n >= 0 && !strchr("\r\n", [string characterAtIndex: n])) n--;
    n++;

    while(n < selection.location + selection.length) {
        while(strchr("\t ", [string characterAtIndex: n])) n++;

        /* if it's not a newline, and it's not already commented, comment it */

        if(comments && !strchr("\n\r", [string characterAtIndex: n]) && n < [string length]) {
            newRange.location = n;
            newRange.length = 0;
 
            [self replaceCharactersInRange: newRange withString: @"# "];

            selection.length += 2;
        }

        if(!comments && [string characterAtIndex: n] == '#') {
            newRange.location = n;
            newRange.length = 1;

            if([string characterAtIndex: n + 1] == ' ') newRange.length = 2;
            
            [self replaceCharactersInRange: newRange withString: @""];

            selection.length -= newRange.length;
        }

        /* skip forward to the next newline */

        while(n < [string length] && !strchr("\r\n", [string characterAtIndex: n])) n++;

        /* skip over multiple newlines */

        while(n < [string length] && strchr("\r\n", [string characterAtIndex: n])) n++;
    }

    [self setSelectedRange: selection];
}

/* the method menu has been clicked on--customize the method list before it pops up! */ 

- (void)setPopupMenu:(NSNotification*)note {
    NSNumber *number;
    NSString *title;
    char *text;
    int type;

    [thePopupMenu removeAllItems];
    [thePopupMenu addItemWithTitle: @"Go to method..."];

    [methodMenuArray removeAllObjects];
    [methodMenuArray addObject: [NSNumber numberWithInt: -1]];

    text = (void*)[[self string] cString];

    slObjectParseSetBuffer(text);

    while(type = slYylex()) {
        if(type != 1) {
			char *objectText;

			objectText = slStripSpaces(slObjectParseText);

            title = [NSString stringWithFormat: @"%s (line %d)", objectText, slObjectParseLine];

			slFree(objectText);

            [thePopupMenu addItemWithTitle: title]; 

            number = [NSNumber numberWithInt: slObjectParseLine];
            [methodMenuArray addObject: number];
        }
    }

    [thePopupMenu synchronizeTitleAndSelectedItem];
}

char *slStripSpaces(char *text) {
	char *out;
	char last = 0;
	int index = 0, doubleSpace = 0;

	out = slMalloc(strlen(text) + 1);

	while(*text) {
		if(*text == ' ' && last == ' ') doubleSpace = 1;
		else doubleSpace = 0;

		if(*text != '\n' && !doubleSpace) out[index++] = *text;

		last = *text;

		text++;
	}

	out[index] = 0;

	return out;
}

- (void)setSelectedRange:(NSRange)charRange affinity:(NSSelectionAffinity)affinity stillSelecting:(BOOL)flag {
    int line = 1;
    unsigned int n;

    if(flag == NO || flag == YES) {
        for(n=0;n<charRange.location;n++) {
            if([[self string] characterAtIndex: n] == '\n') line++;
        }

        [lineNumberBox setIntValue: line];
    }

    [super setSelectedRange: charRange affinity: affinity stillSelecting: flag];

	[self updateContextualMenu];
}

/*
    + 
    = we need to override the following two methods in order to 
    = do syntax coloring when changes are made to the file.  after
    = we catch the calls we call the methods in the superclass.
*/

- (void)setString:(NSString *)s {
	NSMutableString *newString;
	NSMutableString *tabs;
	NSRange range;
	unsigned int n;

	newString = [NSMutableString stringWithString: s]; 

	if(shouldUseSpaces) {
		tabs = [NSMutableString stringWithString: @""];
		for(n=0;n<tabSpaceCount;n++) [tabs appendString: @" "];

		for(n=0;n<[newString length];n++) {
			if([newString characterAtIndex: n] == '\t') {
				range.location = n;
				range.length = 1;

				[newString replaceCharactersInRange: range withString: tabs];
			}
		}
	}

	[super setString: newString];
	[self syntaxColorEntireFile: YES];
}

- (BOOL)shouldChangeTextInRange:(NSRange)r replacementString:(NSString*)s {
    if([s length] > 1) wholeFileNextColor = YES;

    return [super shouldChangeTextInRange: r replacementString: s];
}

- (void)didChangeText {
    [self syntaxColorEntireFile: wholeFileNextColor];
    wholeFileNextColor = NO;

    [super didChangeText];
}

/* syntax coloring functions */

/*!
    Do the syntax coloring.  if wholeFile is true, the entire file is done, otherwise,
    just the line where the cursor is found is done.
*/

- (void)syntaxColorEntireFile:(BOOL)wholeFile {
    NSString *text = [self string];
    NSRange change;
    NSColor *color = NULL;
    BOOL lineStart;

    char token[1024];
     
    unsigned int x, n, m;
    unsigned int start, end;

    if(!shouldDoSyntaxColoring) return;

    if(wholeFile == YES) {
        start = 0;
        end = [[self string] length];
    } else {
		if([self selectedRange].location == 0) 
        	start = 0;
		else 
        	start = [self selectedRange].location - 1;

        end = [self selectedRange].location + 1;

        if(start < 0) start = 0;
        if(end > [text length]) end = [text length];

        while(start > 0 && [text characterAtIndex: start] != '\n') start--;
        while(end < [text length] && [text characterAtIndex: end] != '\n') end++;
    }

    change.location = start;
    change.length = (end - start);

    [self setTextColor: textColor range: change];

	lineStart = YES;

    for(n=start;n<end;n++) {
        char c = [text characterAtIndex: n];

        if(c == '#' || (lineStart && c == '%')) {
            m = n+1;
            while(m < end && [text characterAtIndex: m] != '\n') m++;
            color = commentColor;
        } else if(c == '.' || isdigit(c)) {
            m = n+1;
            while(m < end && (isdigit([text characterAtIndex: m]) || [text characterAtIndex: m] == '.')) m++;
            color = numberColor;
        } else if(c == '\"') {
            n++;
            m = n;

            while(m < end && ([text characterAtIndex: m] != '\"' || [text characterAtIndex: m - 1] == '\\')) m++;
            color = stringColor;
        } else if(isalpha(c)) {
            x = 0;
            m = n;

            while((isalpha(c) || c == '-' || c == '_') && m < end - 1) {
                token[x++] = c;
                m++;
                c = [text characterAtIndex: m];
            }

            token[x] = 0;

            color = [self colorForToken: token];
        } else {
			m = n;
		}

		if(c != '\t' && c != ' ') lineStart = NO;

        if(n != m && n < end && color) {
            change.location = n;
            change.length = m - n;
            [self setTextColor: color range: change];
        }

        n = m;
    }
}

/*
    + colorForToken:
    = gives the color for a certain word token.  this could be modified to 
    = look up methods and stuff, but for the time being it just looks at 
    = types.
*/

- (NSColor*)colorForToken:(char*)string {
    if(!strcmp(string, "int")) return typeColor;
    else if(!strcmp(string, "ints")) return typeColor;
    else if(!strcmp(string, "object")) return typeColor;
    else if(!strcmp(string, "objects")) return typeColor;
    else if(!strcmp(string, "double")) return typeColor;
    else if(!strcmp(string, "doubles")) return typeColor;
    else if(!strcmp(string, "float")) return typeColor;
    else if(!strcmp(string, "floats")) return typeColor;
    else if(!strcmp(string, "vector")) return typeColor;
    else if(!strcmp(string, "vectors")) return typeColor;
    else if(!strcmp(string, "string")) return typeColor;
    else if(!strcmp(string, "strings")) return typeColor;
    else if(!strcmp(string, "hash")) return typeColor;
    else if(!strcmp(string, "hashes")) return typeColor;
    else if(!strcmp(string, "list")) return typeColor;
    else if(!strcmp(string, "lists")) return typeColor;
    else if(!strcmp(string, "matrix")) return typeColor;
    else if(!strcmp(string, "matrices")) return typeColor;
    else return NULL;
}

- (void)setTypeColor:(NSColor*)c {
    NSColor *newColor = [c copy];
   
    [typeColor release];
    typeColor = newColor;
}

- (void)setCommentColor:(NSColor*)c {
    NSColor *newColor = [c copy];

    [commentColor release];
    commentColor = newColor;
}

- (void)setNumberColor:(NSColor*)c {
    NSColor *newColor = [c copy];

    [numberColor release];
    numberColor = newColor;
}

- (void)setStringColor:(NSColor*)c {
    NSColor *newColor = [c copy];

    [stringColor release];
    stringColor = newColor;
}

- (void)setTextColor:(NSColor*)c {
    NSColor *newColor = [c copy];

    [textColor release];
    textColor = newColor;

    [self setInsertionPointColor: textColor];
}

- (void)dealloc {
	[[NSNotificationCenter defaultCenter] removeObserver: self];

    [stringColor release];
    [typeColor release];
    [commentColor release];
    [numberColor release];

	[super dealloc];
}

- (void)setDocDictionary:(id)d {
	docDictionary = d;
}

- (void)openDocs:sender {
	NSString *path;

	path = [NSString stringWithFormat: @"file://%@/docs/classes/%@#%@", [[NSBundle mainBundle] resourcePath], [currentDocArray objectAtIndex: [sender tag]], currentDocString];

	[[NSWorkspace sharedWorkspace] openURL: [NSURL URLWithString: path]];
}

- (void)textView:(NSTextView *)textView clickedOnCell:(id <NSTextAttachmentCell>)cell inRect:(NSRect)cellFrame atIndex:(unsigned)charIndex {
	[super textView: textView clickedOnCell: cell inRect: cellFrame atIndex: charIndex];
}

@end
