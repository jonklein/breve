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

#import "steveTypedefs.h"
#import "expression.h"
#import "evaluation.h"
#import "steveFrontend.h"
#import "object.h"

@implementation slObjectOutlineItem;

- (id)initWithEval:(brEval*)e name:(NSString*)n withVar:(stVar*)stv withOffset:(int)off instance:(stInstance*)i {
    int c;

    theEvalList = NULL;
    theIndex = 0;

	isArray = NO;

    bcopy(e, &eval, sizeof(brEval));

    if(!n) name = [[NSString stringWithCString: "(null)"] retain];
    else name = n;
	
    if(stv && stv->type->type == AT_ARRAY) {
        isArray = YES;
        arrayType = stv->type->arrayType;
    } 

	offset = off;

    instance = i;

    if([self getExpandable] && eval.type == AT_INSTANCE) {
    	stInstance *evalInstance;

		evalInstance = (stInstance*)BRINSTANCE(&eval)->userData;
		[self setEvalObject: evalInstance->type];
    } else if([self getExpandable] && eval.type == AT_LIST) {
        childCount = BRLIST(&eval)->count;
    } else if([self getExpandable] && isArray) {
        childCount = stv->type->arrayCount;
    } else {
        childCount = 0;
        childObjects = NULL;
    }

	// childObjects will be expected to be allocated later on...

	if(childCount < 1) childCount = 1;

	childObjects = malloc(sizeof(id) * childCount);
	for(c=0;c<childCount;c++) childObjects[c] = NULL;

    return self;
}

- (void)setEvalObject:(stObject*)c {
	// This is a bit of a hack.  In the past, parent objects were actually 
	// distinct instances.  So there would be an actual stInstance for the 
	// superclass of an instance.  Now, there is just the base class.  

	object = c;

	if(object) {
		[self updateChildCount: object->variables.size() + 1];
	} else {
		[self updateChildCount: 0];
	}
}

- (void)updateChildCount:(int)newChildCount {
	int n;

	if(newChildCount == childCount) return;

	if(eval.type == AT_LIST) {
		for(n=0;n<childCount;n++) 
	        [childObjects[n] setList: BRLIST(&eval) index: n];
	}

	childObjects = realloc(childObjects, sizeof(id) * newChildCount);

	for(n=childCount;n<newChildCount;n++) childObjects[n] = NULL;

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

- (void)getEval {
    int n;

    if(isArray) {
        eval.type = NULL;
		return;
    } 

	if(instance && instance->status == AS_ACTIVE && offset != -1) {
		stRunInstance ri;
		stInstance *evalInstance;
	
		ri.instance = instance;
		ri.type = instance->type;

        stLoadVariable(&instance->variables[offset], eval.type, &eval, &ri);

		if(eval.type == AT_INSTANCE && BRINSTANCE(&eval) && BRINSTANCE(&eval)->status == AS_ACTIVE) {
			evalInstance = BRINSTANCE(&eval)->userData;
			[self setEvalObject: evalInstance->type];
		}
    } else if(theEvalList) {
        stDoEvalListIndex(theEvalList, theIndex, &eval);
    } 

	if(eval.type == AT_LIST) {
		[self updateChildCount: BRLIST(&eval)->count];
	}

    if(eval.type == AT_INSTANCE) {
		if(BRINSTANCE(&eval) && BRINSTANCE(&eval)->status != AS_ACTIVE) {
			[self updateChildCount: 0];
		} 

		eval.type = AT_INSTANCE;

        for(n=0;n<childCount;n++) if(childObjects[n]) [childObjects[n] setInstance: BRINSTANCE(&eval)->userData];
    }
}

- (void)setInstance:(stInstance*)i {
    instance = i;
}

- (BOOL)getExpandable {
    if(isArray) return YES;

	[self getEval];

	if(eval.type == AT_INSTANCE && BRPOINTER(&eval) && BRINSTANCE(&eval)->status == AS_ACTIVE) return YES;
	if(eval.type == AT_LIST && BRPOINTER(&eval)) return YES;
	
	return NO;
}

/*!
    \brief Return the child count computed earlier. 
*/

- (int)getChildCount {
    return childCount;
}

/*!
    \brief Format this object's eval as a string.
*/


- (NSString*)getValue {
    NSString *result;
    char *cstr;

	[self getEval];

    cstr = brFormatEvaluation(&eval, NULL);

    result = [NSString stringWithCString: cstr];

    slFree(cstr);

    return result;
}

/*
    + childAtIndex:
    = because of the issues described at the top of this file, getting the children of 
    = an object is non-trivial...
*/

- (id)childAtIndex:(int)index {
    NSString *newTitle;
    stInstance *evalInstance;
    brEval newEval;
    stVar *var;
    int i, off;

	[self getEval];

    // make sure the child request is within the bounds of the number of children 

    if(index > childCount) return NULL;

    // if we've found this child previously, we can just return the object.  
    // of course, we should also update the child's parent object in case       
    // this instance has been changed.  a change to this object would obviously 
    // effect the children of this object 

    if(childObjects[index]) {
        if(eval.type == AT_INSTANCE) [childObjects[index] setInstance: (stInstance*)BRINSTANCE(&eval)->userData];
        return childObjects[index];
    }

    i = index;

    evalInstance = (stInstance*)BRINSTANCE(&eval)->userData;

    if(eval.type == AT_INSTANCE) {
		stRunInstance ri;
		ri.instance = instance;
		ri.type = instance->type;

		if(BRINSTANCE(&eval)->status != AS_ACTIVE) {
			slMessage(DEBUG_ALL, "warning: freed instance in object inspector\n");
			return NULL;
		}

		if(index == object->variables.size() ) {
			newEval.type = AT_INSTANCE;
			BRINSTANCE(&newEval) = BRINSTANCE(&eval);

            childObjects[index] = [[slObjectOutlineItem alloc] initWithEval: &newEval name: @"super" withVar: NULL withOffset: -1 instance: instance];

            [childObjects[index] setEvalObject: object->super];

			return childObjects[index];
		}

		std::map< std::string, stVar* >::iterator vi = object->variables.begin();

		while(i--) vi++;

		var = vi->second;

		newTitle = [[NSString stringWithCString: var->name] retain];

		if(var->type->type != AT_ARRAY)
			stLoadVariable(&evalInstance->variables[var->offset], var->type->type, &newEval, &ri);
		else
			newEval.type = AT_ARRAY;

		childObjects[index] = [[slObjectOutlineItem alloc] initWithEval: &newEval name: newTitle withVar: var withOffset: var->offset instance: evalInstance];

    } else if(eval.type == AT_LIST) {
        stDoEvalListIndex(BRLIST(&eval), index, &newEval);

        newTitle = [[NSString stringWithFormat: @"list index %d", index] retain];

        childObjects[index] = [[slObjectOutlineItem alloc] initWithEval: &newEval name: newTitle withVar: NULL withOffset: -1 instance: instance];

        [childObjects[index] setList: BRLIST(&eval) index: index];
    } else if(eval.type == AT_ARRAY) {
		stRunInstance ri;
		ri.instance = instance;
		ri.type = instance->type;

        off = offset + index * stSizeofAtomic(arrayType);
    
        stLoadVariable(&instance->variables[off], arrayType, &newEval, &ri); 
        
        newTitle = [[NSString stringWithFormat: @"array index %d", index] retain];
    
        childObjects[index] = [[slObjectOutlineItem alloc] initWithEval: &newEval name: newTitle withVar: NULL withOffset: off instance: instance];
    }

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

@implementation slObjectOutlineObject;

- initWithInstance:(stInstance*)i object:(stObject*)o {
	instance = i;
	object = o;

	return self;
}

- (NSString*)getName {
	return @"";
}

- (NSString*)getValue {
	return @"object";
}

- (int)getChildCount {
	return object->variables.size() + 1;
}

- (BOOL)getExpandable {
	if (object) return NO;
	return YES;
}

- (id)childAtIndex:(int)index {
	if(index == object->variables.size() ) {
		return [[slObjectOutlineObject alloc] initWithInstance: instance object: object->super];
	} else {
		return [[slObjectOutlineVariable alloc] initWithInstance: instance type: AT_INT offset: 0];	
	}
}

@end

@implementation slObjectOutlineEvaluation

@end

@implementation slObjectOutlineVariable;

- initWithInstance:(stInstance*)i type:(int)t offset:(int)o name:(NSString*)name {
	return self;
}

- (NSString*)getName {
	return @"variable name";
	// return [NSString stringWithFormat: @"%s", var->name ];
}

- (BOOL)getExpandable {
	return NO;
}

- (int)getChildCount {
	return 0;
}

@end

@implementation slObjectOutlineListIndex;

- initWithList:(brEvalListHead*)l index:(int)i {

}

- (void*)setList:(brEvalListHead*)l {

}

- (NSString*)getName {
	return @"list index 4";
}

@end 

@implementation slObjectOutlineHashIndex;

- initWithHash:(brEvalHash*)l index:(int)i {

}

- (void*)setHash:(brEvalHash*)l {

}

- (NSString*)getName {
	return @"hash index 3";
}

@end 
