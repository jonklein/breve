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
//  slBrevePrefs.h
//  breve
//
//  Created by jk on Thu Oct 11 2001.
//  Copyright (c) 2001 __MyCompanyName__. All rights reserved.
//

#import <Foundation/Foundation.h>
#import <Cocoa/Cocoa.h>
#import "slTextView.h"
#import "slBreveEngine.h"

@interface slBrevePrefs : NSObject {
    id breveEngine;
    id interfaceController;

    id pathField;
    
    id commentColor;
    id numberColor;
    id stringColor;
    id typeColor;
    id textColor;
    id backgroundColor;
    
    id autoIndent;
    id extraIndent;
    id updateLineBox;
    id matchBraces;
    id useSpaces;
    id syntaxColor;

    id lineWrapping;
    
    id spacesPerTab;

    id openOnLaunchBox;
    id autoSimStartBox;

    char *defaultPath;
    BOOL autoSimStart;
    BOOL showOpenOnLaunch;

	int recentFileCount;
}

- (IBAction)selectOutputPath:sender;

- (IBAction)update:sender;
- (void)loadPrefs;
- (void)savePrefs;

- (BOOL)shouldAutoSimStart;
- (BOOL)shouldShowOpenOnLaunch;
- (char*)defaultPath;

- (void)updateTextView:(id)textView;

@end
