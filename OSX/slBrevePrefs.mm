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
//  slBrevePrefs.m
//  breve
//
//  Created by jk on Thu Oct 11 2001.
//  Copyright (c) 2001 __MyCompanyName__. All rights reserved.
//

#import "slBrevePrefs.h"


@implementation slBrevePrefs

- (IBAction)selectOutputPath:sender {
    NSOpenPanel *openPanel = [NSOpenPanel openPanel];
    NSString *name;

    [openPanel setCanChooseDirectories: YES];
    [openPanel setCanChooseFiles: NO];

    if([openPanel runModalForTypes: NULL] == NSOKButton) {
        name = [[openPanel filenames] objectAtIndex: 0];

        [pathField setStringValue: name];
    }

    /* reset the open panel values */

    [openPanel setCanChooseDirectories: NO];
    [openPanel setCanChooseFiles: YES];

	[self update: self];
}

- (void)updateTextView:(id)textView {
    [textView setStringColor: [stringColor color]]; 
    [textView setTypeColor: [typeColor color]]; 
    [textView setNumberColor: [numberColor color]]; 
    [textView setCommentColor: [commentColor color]];
    [textView setBackgroundColor: [backgroundColor color]];
    [textView setTextColor: [textColor color]];

    [textView setShouldAutoIndent: [autoIndent intValue]];
    [textView setShouldUpdateLineNumber: [updateLineBox intValue]];
    [textView setShouldUseSpaces: [useSpaces intValue]];
    [textView setShouldExtraIndent: [extraIndent intValue]];
    [textView setShouldMatchBraces: [matchBraces intValue]];
    [textView setShouldDoSyntaxColoring: [syntaxColor intValue]];

    [textView setLineWrappingEnabled: [lineWrapping intValue]];

    [textView setTabSpaces: [spacesPerTab intValue]];

    [textView syntaxColorEntireFile: YES];
}

- (IBAction)update:sender {
    if(defaultPath) slFree(defaultPath);

    defaultPath = strdup((char*)[[[pathField stringValue] stringByExpandingTildeInPath] cString]);

    showOpenOnLaunch = [[openOnLaunchBox selectedCell] tag];
    autoSimStart = [autoSimStartBox intValue];

	[interfaceController updateAllEditorPreferences];
}

- (void)savePrefs {
    NSUserDefaults *defaults = [NSUserDefaults standardUserDefaults];

    NSData *cc = [NSArchiver archivedDataWithRootObject: [commentColor color]];
    NSData *nc = [NSArchiver archivedDataWithRootObject: [numberColor color]];
    NSData *tc = [NSArchiver archivedDataWithRootObject: [typeColor color]];
    NSData *sc = [NSArchiver archivedDataWithRootObject: [stringColor color]];
    NSData *bc = [NSArchiver archivedDataWithRootObject: [backgroundColor color]];
    NSData *xc = [NSArchiver archivedDataWithRootObject: [textColor color]];

    [defaults setBool: YES forKey: @"prefsSaved"];

    [defaults setObject: cc forKey: @"commentColor"];
    [defaults setObject: nc forKey: @"numberColor"];
    [defaults setObject: tc forKey: @"typeColor"];
    [defaults setObject: sc forKey: @"stringColor"];
    [defaults setObject: bc forKey: @"backgroundColor"];
    [defaults setObject: xc forKey: @"textColor"];

    [defaults setBool: [autoIndent intValue] forKey: @"autoIndent"];
    [defaults setBool: [extraIndent intValue] forKey: @"extraIndent"];
    [defaults setBool: [updateLineBox intValue] forKey: @"updateLineBox"];
    [defaults setBool: [matchBraces intValue] forKey: @"matchBraces"];
    [defaults setBool: [syntaxColor intValue] forKey: @"syntaxColor"];
    [defaults setBool: [useSpaces intValue] forKey: @"useSpaces"];
    [defaults setBool: [simDirForOutput intValue] forKey: @"simDirForOutput"];

    [defaults setBool: [lineWrapping intValue] forKey: @"lineWrapping"];
    
    [defaults setInteger: [spacesPerTab intValue] forKey: @"spacesPerTab"];

	/* recentFileCount is not used in version 1.5, but set it to a */
	/* reasonable value for backwards compatability -- otherwise,  */
	/* version 1.4 will freeze up when reading these preferences */

	if(!recentFileCount) recentFileCount = 5;
    [defaults setInteger: recentFileCount forKey: @"recentFileCount"];

    [defaults setObject: [pathField stringValue] forKey: @"defaultPath"];

    [defaults setInteger: [[openOnLaunchBox selectedCell] tag] forKey: @"openOnLaunch"];
    [defaults setInteger: [autoSimStartBox intValue] forKey: @"autoSimStart"];

    [defaults synchronize];
}

- (void)loadPrefs {
    NSUserDefaults *defaults = [NSUserDefaults standardUserDefaults];

    NSData *cd, *sd, *td, *nd, *bd, *xd;
    NSColor *cc, *sc, *tc, *nc, *bc, *xc;

    NSString *filePath;
	struct stat st;

    if(![defaults boolForKey: @"prefsSaved"]) {
        [self savePrefs];
    }

    cd = [defaults objectForKey: @"commentColor"];
    sd = [defaults objectForKey: @"stringColor"];
    td = [defaults objectForKey: @"typeColor"];
    nd = [defaults objectForKey: @"numberColor"];
    bd = [defaults objectForKey: @"backgroundColor"];
    xd = [defaults objectForKey: @"textColor"];

    if(cd) cc = [NSUnarchiver unarchiveObjectWithData: cd];
    else cc = [NSColor blackColor];

    if(sd) sc = [NSUnarchiver unarchiveObjectWithData: sd];
    else sc = [NSColor blackColor];

    if(td) tc = [NSUnarchiver unarchiveObjectWithData: td];
    else tc = [NSColor blackColor];

    if(nd) nc = [NSUnarchiver unarchiveObjectWithData: nd];
    else nc = [NSColor blackColor];

    if(bd) bc = [NSUnarchiver unarchiveObjectWithData: bd];
    else bc = [NSColor blackColor];

    if(xd) xc = [NSUnarchiver unarchiveObjectWithData: xd];
    else xc = [NSColor blackColor];

    [commentColor setColor: cc];
    [typeColor setColor: tc];
    [stringColor setColor: sc];
    [numberColor setColor: nc];
    [backgroundColor setColor: bc];
    [textColor setColor: xc];

    [autoIndent setState: [defaults boolForKey: @"autoIndent"]];
    [updateLineBox setState: [defaults boolForKey: @"updateLineBox"]];
    [useSpaces setState: [defaults boolForKey: @"useSpaces"]];
    [simDirForOutput setState: [defaults boolForKey: @"simDirForOutput"]];

    [lineWrapping setState: [defaults boolForKey: @"lineWrapping"]];
    [extraIndent setState: [defaults boolForKey: @"extraIndent"]];
    [matchBraces setState: [defaults boolForKey: @"matchBraces"]];
    [syntaxColor setState: [defaults boolForKey: @"syntaxColor"]];

    [spacesPerTab setIntValue: [defaults integerForKey: @"spacesPerTab"]];

    filePath = [defaults stringForKey: @"defaultPath"];

    if(!filePath || [filePath isEqualToString: @""] || stat([filePath cString], &st)) 
        filePath = [NSString stringWithFormat: @"%@/Documents", NSHomeDirectory()]; 

    [pathField setStringValue: filePath];

    recentFileCount = [defaults integerForKey: @"recentFileCount"];
	if(!recentFileCount) recentFileCount = 5;

    [openOnLaunchBox selectCellWithTag: [defaults integerForKey: @"openOnLaunch"]];

    [autoSimStartBox setState: [defaults integerForKey: @"autoSimStart"]];

    [self update: self];
}

- (char*)defaultPath {
    return defaultPath;
}

- (BOOL)shouldAutoSimStart {
    return autoSimStart;
}

- (BOOL)shouldShowOpenOnLaunch {
    return showOpenOnLaunch;
}

- (BOOL)shouldUseSimDirForOutput {
	return [simDirForOutput intValue];
}

- (void)dealloc {
    if(defaultPath) slFree(defaultPath);

	[super dealloc];
}

@end
