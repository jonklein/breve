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
//  slBreveNibLoader.h
//  breve
//
//  Created by jon klein on Sun Jun 16 2002.
//  Copyright (c) 2002 artificial. All rights reserved.
//

#import <Foundation/Foundation.h>
#import <AppKit/AppKit.h>

@interface slBreveNibLoader : NSObject {
    id window;
	id bundle;
	id engine;

	id tagDict;

	id test;
}

- initWithNib:(NSString*)nib andEngine:(id)e;
- (void)setRecursiveTarget:(id)target andSelector:(SEL)s forView:(NSView*)v;
- (BOOL)setString:(NSString*)string forObjectWithTag:(int)n;
- (IBAction)action:sender;

@end
