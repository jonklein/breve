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

#import "slObjectOutline.h"

@implementation slObjectOutline;

/*
    + slObjectOutline.m
    = is the "data" object for the NSOutlineView variable browswer.  refer to the 
    = NSOutlineViewDataSource protocol.
    =
    = this class is basically a frontend to the individual slObjectOutlineItems
    = which make up the table.
*/

/* tell this data source who it's NSOutlineView is... */

- (void)setOutlineView:(NSOutlineView*)v {
    theView = v;
}

/* associate the data source with a certain instance to display */

- (void)setEngine:(brEngine*)e instance:(stInstance*)i {
    brEval newEval;

	// return;

    engine = e;

    if(root) {
        // if the data source was previously associated with another instance, 
        // free the data 

        [theView deselectAll: self];
        [root release];
        root = NULL;
    }

    // if this is a valid and active instance, create a data item for it

    if(i && i->status == AS_ACTIVE) {
		BRINSTANCE(&newEval) = i->breveInstance;
        newEval.type = AT_INSTANCE;
        instance = i;
        root = [[slObjectOutlineItem alloc] initWithEval: &newEval name: NULL withVar: NULL withOffset: NULL instance: i];
    } else {
        root = NULL;
        instance = NULL;
    }
}

/* make sure that this instance is still active */

- (void)checkInstance {
    if(instance && instance->status != AS_ACTIVE) [self setEngine: NULL instance: NULL];
}

/* return the child at a certain index */

- (id)outlineView:(NSOutlineView *)view child:(int)index ofItem:(id)item {
    if(!instance) return NULL;

    /* it is possible that item is NULL, indicating that they want */
    /* child of the root index */

    if(item) return [item childAtIndex: index];
    else return [root childAtIndex: index];
}

/* the number of children a certain object has... */

- (int)outlineView:(NSOutlineView *)view numberOfChildrenOfItem:(id)item {
    if(!instance) return 0;

    if(item) return [item getChildCount];
    else return [root getChildCount];
}

/* is item expandable... */

- (BOOL)outlineView:(NSOutlineView*)view isItemExpandable:(id)item {
    if(!instance) return NO;

    if(item) return [item getExpandable];
    else return NO;
}

/* format this data into a string */

- (id)outlineView:(NSOutlineView*)view objectValueForTableColumn:(NSTableColumn*)column byItem: (id)item {
    NSString *identifier;
    id theCell;
    int row;

    if(!instance || !item) return NULL;

    identifier = [column identifier];

    row = [view rowForItem: item];

    theCell = [column dataCellForRow: row];

    if([identifier isEqualToString: @"variable"] == YES) {
        [theCell setEditable: NO];
        return [item getName];
    } else {
        [theCell setAlignment: NSNaturalTextAlignment];

        return [item getValue];
    }
}

- (BOOL)outlineView:(NSOutlineView *)o shouldEditTableColumn:(NSTableColumn *)tableColumn item:(id)item {
    return NO;
}

@end
