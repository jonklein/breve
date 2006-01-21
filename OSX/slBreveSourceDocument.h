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
//  MyDocument.h
//  OSX2
//
//  Created by jon klein on Fri Nov 29 2002.
//  Copyright (c) 2002  s pi der la nd. All rights reserved.
//


#import <Cocoa/Cocoa.h>

#import "slBreve.h"
#import "slTextView.h"
#import "format.h"

@interface slBreveSourceDocument : NSDocument
{
    IBOutlet id steveText;
    IBOutlet id sourceWindow;

	BOOL inited;
	NSString *loadText;
}

- (void)setString:(NSString*)s;

- (IBAction)find:sender;
- (IBAction)findNext:sender;
- (IBAction)findPrevious:sender;
- (IBAction)findSelection:sender;
- (IBAction)scrollToSelection:sender;
- (IBAction)reformat:sender;

- (NSString*)documentText;

- (id)text;
- (id)window;

@end
