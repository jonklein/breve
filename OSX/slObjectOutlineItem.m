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

#import "slObjectOutlineItem.h"

@implementation slObjectOutlineItem;

/*
    + slObjectOutlineItem.m
    = part of the variable browser outline view, we handle a specific entry
    = in the view and can give the number of children of the object, whether
    = it's expandable, it's display string, and so forth.
    =
    = the biggest complication is that not all variables are stored the same 
    = way.  for example, regular variables are always in the instance variable
    = array, while array variables can be stored anywhere and may even change
    = as values are removed and inserted into the array.
    =
    = so with regular variables, we can just dereference a constant pointer 
    = to get the value, while with the arrays we need to look up the pointer
    = everytime.
    =
    = this will be a big issue when adding "set variable" functionality as 
    = well.
*/

- (id)initWithEval:(brEval*)e name:(NSString*)n withVar:(stVar*)stv withOffset:(int)off instance: (stInstance*)i {
    int c;
    stInstance *evalInstance;

    theEvalList = NULL;
    theIndex = 0;

    bcopy(e, &eval, sizeof(brEval));

    if(!n) name = [[NSString stringWithCString: "(null)"] retain];
    else name = n;
	
    if(stv && stv->type->type == AT_ARRAY) {
        isArray = YES;
        arrayType = stv->type->arrayType;
        arrayOffset = stv->offset;
    } else offset = off;

    instance = i;

    [self setCanExpand: YES];

    if([self getExpandable] && eval.type == AT_INSTANCE) {
		evalInstance = STINSTANCE(&eval);
		class = evalInstance->type;
       	if(class) childCount = slListCount(class->variableList) + 1;
		else childCount = 0;
    } else if([self getExpandable] && eval.type == AT_LIST) {
        childCount = BRLIST(&eval)->count;
    } else if([self getExpandable] && isArray) {
        childCount = stv->type->arrayCount;
    } else {
        childCount = 0;
        childObjects = NULL;
    }

    if(childCount != 0) {
        childObjects = malloc(sizeof(id) * childCount);
        for(c=0;c<childCount;c++) childObjects[c] = NULL;
    }

    return self;
}

- (void)setEvalClass:(stObject*)c {
	// This is a bit of a hack.  In the past, parent objects were actually 
	// distinct instances.  So there would be an actual stInstance for the 
	// superclass of an instance.  Now, there is just the base class.  So
	// if they're inspecting a parent class, t

	class = c;

	if(class) {
		[self setCanExpand: YES];
		[self updateChildCount: slListCount(class->variableList) + 1];
	} else {
		[self setCanExpand: NO];
		[self updateChildCount: 0];
	}
}

- (void)updateChildCount:(int)newChildCount {
	int n;

	for(n=0;n<childCount;n++) {
        [childObjects[n] setList: BRLIST(&eval) index: n];
	}

	if(newChildCount == childCount) return;

	childObjects = realloc(childObjects, sizeof(id) * newChildCount);

	for(n=childCount;n<newChildCount;n++) {
		childObjects[n] = NULL;
	}

	childCount = newChildCount;
}

- (void)setList:(brEvalListHead*)e index:(int)index {
    theEvalList = e;
    theIndex = index;
}

- (NSString*)getName {
    return name;
}

/* if we have a regular variable, load it and return it.  if we are a list */
/* element, then load the proper element of the list. */

- (void)getEval:(brEval*)e {
    int n;

    if(isArray) {
        e->type = NULL;
    } else if(instance && instance->status == AS_ACTIVE && offset != -1) {
		stRunInstance ri;
	
		ri.instance = instance;
		ri.type = instance->type;

        stLoadVariable(&instance->variables[offset], eval.type, e, &ri);
    } else if(theEvalList) {
        stDoEvalListIndex(theEvalList, theIndex, e);
    } else {
		bcopy(&eval, e, sizeof(brEval));
	}

	if(!isArray && eval.type == AT_LIST) {
		[self updateChildCount: BRLIST(&eval)->count];
	}

    if(!isArray && eval.type == AT_INSTANCE) {
		if(STINSTANCE(&eval) && STINSTANCE(&eval)->status != AS_ACTIVE) {
			[self setCanExpand: NO];
			[self updateChildCount: 0];
		}

		e->type = AT_INSTANCE;

        for(n=0;n<childCount;n++) if(childObjects[n]) [childObjects[n] setInstance: STINSTANCE(e)];
    }
}

- (void)setInstance:(stInstance*)i {
    instance = i;
}

- (BOOL)getExpandable {
    if(isArray) return YES;

    return(((eval.type == AT_INSTANCE) || eval.type == AT_LIST) && BRPOINTER(&eval) != NULL && canExpand);
}

- (void)setCanExpand:(BOOL)e {
    canExpand = e;
}

/*
    + getChildCount
    = return the child count we computed earlier... 
*/

- (int)getChildCount {
    return childCount;
}

/*
    + getValue
    = format this object's eval as a string.
*/

- (NSString*)getValue {
    stInstance *o;

    if(isArray) return [NSString stringWithFormat: @"%p [array]", &instance->variables[offset]];

    [self getEval: &eval];

    switch(eval.type) {
        case AT_INT:
            return [NSString stringWithFormat: @"%d", BRINT(&eval)];
            break;
        case AT_DOUBLE:
            return [NSString stringWithFormat: @"%f", BRDOUBLE(&eval)];
            break;
        case AT_INSTANCE:
            o = STINSTANCE(&eval);
            if(class && o && o->status == AS_ACTIVE) return [NSString stringWithFormat: @"%p [instance of \"%s\"]", o, class->name];
            else if(class) return [NSString stringWithFormat: @"%p [freed instance of \"%s\"]", o, class->name];
            else return [NSString stringWithFormat: @"%p [object]", o];
            break;
        case AT_POINTER:
            return [NSString stringWithFormat: @"%p [pointer]", BRPOINTER(&eval)];
            break;
        case AT_STRING:
            return [NSString stringWithCString: BRSTRING(&eval)];
            break;
        case AT_VECTOR:
            return [NSString stringWithFormat: @"(%.2f, %.2f, %.2f)", BRVECTOR(&eval).x, BRVECTOR(&eval).y, BRVECTOR(&eval).z];
            break;
        case AT_LIST:
            return [NSString stringWithFormat: @"%p [list]", BRPOINTER(&eval)];
            break;
        case AT_HASH:
            return [NSString stringWithFormat: @"%p [hash]", BRPOINTER(&eval)];
            break;
		case AT_DATA:
            return [NSString stringWithFormat: @"%p [data]", BRPOINTER(&eval)];
            break;
    }

    return NULL;
}

/*
    + childAtIndex:
    = because of the issues described at the top of this file, getting the children of 
    = an object is non-trivial...
*/

- (id)childAtIndex:(int)index {
    NSString *newTitle;
    stInstance *evalInstance;
	stRunInstance ri;
    brEval newEval;
    slList *vars;
    stVar *var;
    int i, off;

    /* make sure the child request is within the bounds of the number of children */

    if(index > childCount) return NULL;

    /* if we've found this child previously, we can just return the object.     */
    /* of course, we should also update the child's parent object in case       */
    /* this instance has been changed.  a change to this object would obviously */
    /* effect the children of this object */

    if(childObjects[index]) {
        if(eval.type == AT_INSTANCE) [childObjects[index] setInstance: STINSTANCE(&eval)];
        return childObjects[index];
    }

    i = index;

    evalInstance = STINSTANCE(&eval);

	ri.instance = instance;
	ri.type = instance->type;

    /* depending on what kind of object WE are, we have different */
    /* ways to reference our children */
    if(eval.type == AT_INSTANCE) {
        vars = class->variableList;

		if(index == slListCount(vars)) {
			// is this the parent?

			newEval.type = AT_INSTANCE;
			newEval.values.instanceValue = STINSTANCE(&eval);
        	childObjects[index] = [[slObjectOutlineItem alloc] initWithEval: &newEval name: @"super" withVar: NULL withOffset: -1 instance: NULL];
			[childObjects[index] setEvalClass: class->super];
		} else {
        	while(vars && i--) vars = vars->next;

	        if(!vars) return NULL;

	   	     var = vars->data;

			newTitle = [[NSString stringWithCString: var->name] retain];

	        if(var->type->type != AT_ARRAY)
				stLoadVariable(&evalInstance->variables[var->offset], var->type->type, &newEval, &ri);
			else 
				newEval.type = AT_ARRAY;

	        childObjects[index] = [[slObjectOutlineItem alloc] initWithEval: &newEval name: newTitle withVar: var withOffset: var->offset instance: evalInstance];
		}
    } else if(eval.type == AT_LIST) {
        stDoEvalListIndex(BRLIST(&eval), index, &newEval);

        newTitle = [[NSString stringWithFormat: @"#%d", index] retain];

        childObjects[index] = [[slObjectOutlineItem alloc] initWithEval: &newEval name: newTitle withVar: NULL withOffset: -1 instance: evalInstance];

        [childObjects[index] setList: BRLIST(&eval) index: index];
    } else if(eval.type == AT_ARRAY) {
        off = arrayOffset + index * stSizeofAtomic(arrayType);
    
        stLoadVariable(&instance->variables[off], arrayType, &newEval, &ri); 
        
        newTitle = [[NSString stringWithFormat: @"#%d", index] retain];
    
        childObjects[index] = [[slObjectOutlineItem alloc] initWithEval: &newEval name: newTitle withVar: NULL withOffset: off instance: instance];
    
    }

    // this is to avoid obvious loops--don't let us expand our own parents or ourselves. 
    // there are ways to get around this of course, but...

    if((newEval.type == AT_INSTANCE && STINSTANCE(&newEval)) && // it's an instance 
			(STINSTANCE(&newEval)->status != AS_ACTIVE)) 		// it's not active
				[childObjects[index] setCanExpand: NO];

    return childObjects[index];
}

- (void)dealloc {
    int n;

    if(childObjects) {
        for(n=0;n<childCount;n++) if(childObjects[n]) [childObjects[n] release];
        free(childObjects);
    }

    [name release];
    [super dealloc];
}

@end
