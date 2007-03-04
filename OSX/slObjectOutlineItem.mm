/*****************************************************************************
 *																		   *
 * The breve Simulation Environment										  *
 * Copyright (C) 2000, 2001, 2002, 2003 Jonathan Klein					   *
 *																		   *
 * This program is free software; you can redistribute it and/or modify	  *
 * it under the terms of the GNU General Public License as published by	  *
 * the Free Software Foundation; either version 2 of the License, or		 *
 * (at your option) any later version.									   *
 *																		   *
 * This program is distributed in the hope that it will be useful,		   *
 * but WITHOUT ANY WARRANTY; without even the implied warranty of			*
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the			 *
 * GNU General Public License for more details.							  *
 *																		   *
 * You should have received a copy of the GNU General Public License		 *
 * along with this program; if not, write to the Free Software			   *
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

	mEval = new brEval;

	theEvalList = NULL;
	theIndex = 0;

	isArray = NO;

	brEvalCopy( e, mEval );

	if(!n) name = [[NSString stringWithCString: "(null)"] retain];
	else name = n;
	
	if(stv && stv->type->_type == AT_ARRAY) {
		isArray = YES;
		arrayType = stv->type->_arrayType;
	} 

	offset = off;

	instance = i;

	if( [self getExpandable] && mEval->type() == AT_INSTANCE) {
		stInstance *evalInstance;

		if( BRINSTANCE( mEval )->object->type->_typeSignature == STEVE_TYPE_SIGNATURE ) {
			evalInstance = (stInstance*)BRINSTANCE( mEval )->userData;
			[ self setEvalObject: evalInstance->type ];
		} else {
			evalInstance = NULL;
			[ self setEvalObject: NULL ];
		}
	} else if([self getExpandable] && mEval->type() == AT_LIST) {
		childCount = BRLIST( mEval )->_vector.size();
	} else if([self getExpandable] && isArray) {
		childCount = stv->type->_arrayCount;
	} else {
		childCount = 0;
		childObjects = NULL;
	}

	// childObjects will be expected to be allocated later on...

	if(childCount < 1) childCount = 1;

	childObjects = (id*)malloc(sizeof(id) * childCount);

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

	if( mEval->type() == AT_LIST) {
		for(n=0;n<childCount;n++) 
			[childObjects[n] setList: BRLIST( mEval ) index: n];
	}

	childObjects = (id*)realloc(childObjects, sizeof(id) * newChildCount);

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
		return;
	} 

	if(instance && instance->status == AS_ACTIVE && offset != -1) {
		stRunInstance ri;
		stInstance *evalInstance;
	
		ri.instance = instance;
		ri.type = instance->type;

		stLoadVariable( &instance->variables[offset], mEval->type(), mEval, &ri);

		if( mEval->type() == AT_INSTANCE && BRINSTANCE( mEval ) && BRINSTANCE( mEval )->status == AS_ACTIVE) {
			evalInstance = (stInstance*)BRINSTANCE( mEval )->userData;
			[self setEvalObject: evalInstance->type];
		}
	} else if(theEvalList) {
		stDoEvalListIndex(theEvalList, theIndex, mEval);
	} 

	if( mEval->type() == AT_LIST ) {
		[self updateChildCount: BRLIST( mEval )->getVector().size() ];
	}

	if(mEval->type() == AT_INSTANCE) {
		if(BRINSTANCE( mEval ) && BRINSTANCE( mEval )->status != AS_ACTIVE) {
			[self updateChildCount: 0];
		} 

		for(n=0;n<childCount;n++) if(childObjects[n]) [childObjects[n] setInstance: (stInstance*)BRINSTANCE( mEval )->userData];
	}
}

- (void)setInstance:(stInstance*)i {
	instance = i;
}

- (BOOL)getExpandable {
	if(isArray) return YES;

	[self getEval];

	if(mEval->type() == AT_INSTANCE && BRPOINTER( mEval ) && BRINSTANCE( mEval )->status == AS_ACTIVE) return YES;
	if(mEval->type() == AT_LIST && BRPOINTER( mEval )) return YES;
	
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

	cstr = brFormatEvaluation( mEval, NULL );

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

	if( childObjects[index] ) {
		if( mEval->type() == AT_INSTANCE ) [childObjects[index] setInstance: (stInstance*)BRINSTANCE( mEval )->userData];
		return childObjects[index];
	}

	i = index;

	evalInstance = (stInstance*)BRINSTANCE( mEval )->userData;

	if( mEval->type() == AT_INSTANCE ) {
		stRunInstance ri;
		ri.instance = instance;
		ri.type = instance->type;

		if(BRINSTANCE( mEval )->status != AS_ACTIVE) {
			slMessage(DEBUG_ALL, "warning: freed instance in object inspector\n");
			return NULL;
		}

		if( index == (int)object->variables.size() ) {
			newEval.set( mEval->getInstance() );

			childObjects[index] = [[slObjectOutlineItem alloc] initWithEval: &newEval name: @"super" withVar: NULL withOffset: -1 instance: instance];

			[childObjects[index] setEvalObject: object->super];

			return childObjects[index];
		}

		std::map< std::string, stVar* >::iterator vi = object->variables.begin();

		while(i--) vi++;

		var = vi->second;

		newTitle = [ [ NSString stringWithCString: var->name.c_str() ] retain ];

		if(var->type->_type != AT_ARRAY)
			stLoadVariable(&evalInstance->variables[var->offset], var->type->_type, &newEval, &ri);
//		else 
//			newEval.type = AT_ARRAY;

		childObjects[index] = [[slObjectOutlineItem alloc] initWithEval: &newEval name: newTitle withVar: var withOffset: var->offset instance: evalInstance];

	} else if( mEval->type() == AT_LIST ) {
		stDoEvalListIndex( BRLIST( mEval ), index, &newEval );

		newTitle = [[NSString stringWithFormat: @"list index %d", index] retain];

		childObjects[index] = [[slObjectOutlineItem alloc] initWithEval: &newEval name: newTitle withVar: NULL withOffset: -1 instance: instance];

		[childObjects[index] setList: BRLIST( mEval ) index: index];
	} else if( mEval->type() == AT_ARRAY ) {
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

	delete mEval;
	
	[name release];
	[super dealloc];
}

@end

/*

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
	if( index == (int)object->variables.size() ) {
		return [ [slObjectOutlineObject alloc] initWithInstance: instance object: object->super ];
	} else {
		return [ [slObjectOutlineVariable alloc] initWithInstance: instance type: AT_INT offset: 0];	
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
}

- (BOOL)getExpandable {
	return NO;
}

- (int)getChildCount {
	return 0;
}

@end

*/
