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

#include "steve.h"
#include "expression.h"
#include "evaluation.h"

/*!
	Interface for setting up and interacting with steve language objects and their associate interfaces.
*/

/*!
	\brief Creates a new object class.

	 Requires = the engine, the object name and the super object.
*/

stObject *stObjectNew(brEngine *engine, stSteveData *sdata, char *name, char *alias, stObject *super, float version) {
	stObject *o;
	brObject *bo;

	/* check to see if the object exists */

	if(brObjectFind(engine, name)) return NULL;

	o = new stObject;

	o->version = version;

	o->engine = engine;
	o->steveData = sdata;
	o->super = super;

	o->name = slStrdup(name);

	// before any variables are added here, our variable vector 
	// the size of our parent's variable vector.  our vector 
	// offset says where our parent's variables end and where
	// ours begin

	if(super) {
		int alignment = super->varSize % sizeof(double);

		o->varOffset = super->varSize + alignment; /* + super->varOffset ? */
		o->varSize = super->varSize + alignment;
	} else {
		o->varOffset = 0;
		o->varSize = 0;
	}

	// save this object in the engine's object namespace

	bo = brEngineAddObject(engine, &sdata->steveObjectType, name, o);
	
	if(alias) brEngineAddObjectAlias(engine, alias, bo);

	sdata->objects.push_back(o);

	return o;
}

/*!
	\brief Instantiates a steve object and adds it to the breve engine.

	This method also fills in the "breveInstance" pointer in the steve object.
*/

brInstance *stInstanceCreateAndRegister(stSteveData *sdata, brEngine *e, brObject *object) {
	stInstance *newi = NULL;
	brInstance *bi;

	bi = brObjectInstantiate(e, object, NULL, 0);

	if(bi->object->type == &sdata->steveObjectType) {
		newi = (stInstance*)bi->userData;
		newi->breveInstance = bi;
		if(stInstanceInit(newi) != EC_OK) return NULL;
	}

	return bi;
}

/*!
	\brief Instantiate a steve object, but do NOT call the init methods.

	This creates an sets up an stInstance, but does not call the init 
	method.  There are situations when calling functions need to have 
	a pointer to the instance before init is called, so initialization
	is a seperate step.
*/

stInstance *stInstanceNew(stObject *o) {
	stInstance *i;

	if(!o) {
		slMessage(DEBUG_ALL, "stInstanceNew called with NULL pointer\n");
		return NULL;
	}

	i = new stInstance;

	i->type = o;

	i->variables = new char[o->varSize];
	memset(i->variables, 0, o->varSize);

	i->status = AS_ACTIVE;
	i->gc = 0;
	i->retainCount = 0;

	// if(pthread_mutex_init(&i->lock, NULL)) 
	// 	slMessage(0, "warning: error creating mutex for instance %p\n", i);

	stAddToInstanceLists(i);

	return i;
}

/*!
	Complete the initialization of a steve instance by calling the "init" method.
*/

int stInstanceInit(stInstance *i) {
	stRunInstance ri;

	ri.instance = i;
	ri.type = i->type;

	// "trace" the init method (call it for this instance and all supers)

	if(stMethodTrace(&ri, "init") != EC_OK) return EC_ERROR;

	return EC_OK;
}

/*!
	\brief Adds an instance to another's dependencies list.
*/

void stInstanceAddDependency(stInstance *i, stInstance *dependency) {
	i->dependencies.insert( dependency);
	dependency->dependents.insert( i);
}

/*!
	\brief Removes an instance from another's dependencies list.
*/

void stInstanceRemoveDependency(stInstance *i, stInstance *dependency) {
	std::set< stInstance*, stInstanceCompare>::iterator ii;

	ii = i->dependencies.find( dependency);
	if(ii != i->dependencies.end()) i->dependencies.erase( ii);

	ii = dependency->dependents.find( i);
	if(ii != dependency->dependents.end()) dependency->dependents.erase( ii);
}

/*!
	\brief Increments the retain count of an object.
*/

void stInstanceRetain(stInstance *i) {
	if(!i) return;
	i->retainCount++;
}

/*!
	\brief Decrements the retain count of an object.  
*/

void stInstanceUnretain(stInstance *i) {
	if(!i) return;
	i->retainCount--;
}

/*!
	\brief Attempt to garbage collect an instance.

	If the retain count is < 1 and if GC is enabled for this object, 
	the object is freed.

	CURRENTLY EXPERIMENTAL.
*/

void stInstanceCollect(stInstance *i) {
	if(i->gc && i->retainCount < 1) brInstanceRelease(i->breveInstance);
}

/*!
	\brief Frees an stInstance, but leaves the instance lists in tact.

	This is used at the end of the simulation when lots of things are 
	being freed at the same time.  The time required to traverse and 
	modify the instance lists adds up, so if we know we're going to 
	end everything, then we'll take care of the instance lists later.
*/

void stInstanceFree(stInstance *i) {
	stInstanceFreeNoInstanceLists(i);

	stRemoveFromInstanceLists(i);

	if(i->type->steveData->retainFreedInstances) {
		i->type->steveData->freedInstances.push_back( i);
	} else {
		delete i;
	}
}

/*!
	\brief Frees an stInstance.

	Calls destroy on the object and frees its internals.  The pointer
	to the object itself may be kept around or recycled later on.
*/

void stInstanceFreeNoInstanceLists(stInstance *i) {
	brEval result;
	stRunInstance ri;

	ri.instance = i;
	ri.type = i->type;

	stCallMethodByName(&ri, "destroy", &result);
	stMethodTrace(&ri, "delete");

	stInstanceFreeInternals(i);

	i->status = AS_RELEASED;
}

/*!
	\brief Free the internals of an stInstance.

	This method should not be called directly, as it is called via stInstanceFree.
*/

void stInstanceFreeInternals(stInstance *i) {
	stObject *type;
	stVar *var;
	int n;

	if(i->status == AS_RELEASED) return;

	// garbage collect the variables.

	type = i->type;

	std::map< std::string, stVar* >::iterator vi;

	while(type) {
		for(vi = type->variables.begin(); vi != type->variables.end(); vi++) {
			char *varpointer;

			var = vi->second;

			if(var) {
				varpointer = &i->variables[var->offset];

				stGCUnretainAndCollectPointer(*(void**)varpointer, var->type->type);

				// arrays still suck.

				if(var->type->type == AT_ARRAY) {
					char *arraypointer = varpointer;

					for(n=0;n<var->type->arrayCount;n++) {
						varpointer = arraypointer + (n * stSizeofAtomic(var->type->arrayType));
			
						stGCUnretainAndCollectPointer(*(void**)varpointer, var->type->arrayType);
					}
				}
			}
		}

		type = type->super;
	}

	delete[] i->variables;
}

/*!
	\brief Free all instances of a certain object type.

	To be used for cleanup only.
*/

void stObjectFreeAllInstances(stObject *o) {
	std::set< stInstance*, stInstanceCompare> all;
	std::set< stInstance*, stInstanceCompare>::iterator ii;

	// it's possible that the allInstances list will change while
	// we're going through it!  We'll use a copy instead.

	all = o->allInstances;

	for(ii = all.begin(); ii != all.end(); ii++) {
		stInstance *i = *ii;

		// only object matching the base class should be freed now

		if(i->type == o) 
			stInstanceFreeNoInstanceLists(i);
	}
}

/*!
	\brief deallocates an stObject.  

	The object no longer exists and all instances of the object will be invalid.
*/

void stObjectFree(stObject *o) {
	// slList *symbolList, *start;
	// brNamespaceSymbol *symbol;

	std::map< std::string, std::vector<stMethod*> >::iterator li;

	// free all of the variable/method symbols associated with the object 

	for(li = o->methods.begin(); li != o->methods.end(); li++) {
		// the methods are going to be a list of methods with the same name.

		std::vector< stMethod* > &mlist = li->second;
		std::vector< stMethod* >::iterator mi;

		for(mi = mlist.begin(); mi != mlist.end(); mi++ ) stFreeMethod(*mi);
	}

	std::map< std::string, stVar* >::iterator vi;

	for(vi = o->variables.begin(); vi != o->variables.end(); vi++) {
		stFreeStVar(vi->second);
	}

	slFree(o->name);

	delete o;
}

/*!
	\brief Call a method for an instance and it's superclasses.

	Starting with the top object and working downwards, stMethodTrace
	calls a given method name (with no arguments) for all ancestors of
	an instance, and for the instance itself.

	This is for methods like "init" and "destroy" that should be 
	called for an entire instance hierarchy.
*/

int stMethodTrace(stRunInstance *i, char *name) {
	brEval e;
	stMethod *meth;
	stRunInstance ri;
	int result;

	if(!i->type) return EC_OK;

	ri.instance = i->instance;
	ri.type = i->type->super;

	result = stMethodTrace(&ri, name);
	if(result != EC_OK) return EC_ERROR;

	meth = stFindInstanceMethodNoSuper(i->type, name, 0);

	if(meth) {
		result = stCallMethod(i, i, meth, NULL, 0, &e);

		if(result != EC_OK) return result;
	}

	return EC_OK;
}

/*!
	\brief Find the desired keyword in the given method.
*/

stKeywordEntry *stFindKeyword(char *keyword, stMethod *m) {
	stKeywordEntry *e;
	unsigned int n;

	for(n=0;n<m->keywords.size();n++) {
		e = m->keywords[n];
		if(!strcmp(keyword, e->keyword)) return e;
	}

	return NULL;
}

/*!
	\brief Locate a keyword or local variable in a method.

	Return the variable, or NULL if the method is not found.
*/

stVar *stFindLocal(char *name, stMethod *m) {
	stKeywordEntry *e;
	unsigned int n;

	std::vector< stVar* >::iterator vi;

	// is a local variable? 

	for(vi = m->variables.begin(); vi != m->variables.end(); vi++ ) 
		if(!strcmp((*vi)->name, name)) return *vi;

	// perhaps it's a keyword argument to the method... 

	for(n=0;n<m->keywords.size();n++) {
		e = m->keywords[n];
		if(!strcmp(name, e->var->name)) return e->var;
	}

	// it's not here 

	return NULL;
}

/*!
	\brief Fills in a keyword entry struct.  

	The keyword entry will eventually be added to a methods list of keywords.
*/

stKeywordEntry *stNewKeywordEntry(char *keyword, stVar *v, brEval *defValue) {
	stKeywordEntry *e;
	brEval *copy;

	e = new stKeywordEntry;

	if(!e || !v || !keyword) return NULL;


	e->keyword = slStrdup(keyword);
	e->var = v;
	e->defaultKey = NULL;

	if(defValue) {
		copy = new brEval;
		memcpy(copy, defValue, sizeof(brEval));
		e->defaultKey = new stKeyword(keyword, new stEvalExp(copy, NULL, 0));
	}

	return e;
}

/*!
	\brief Frees a stKeywordEntry.
*/

void stFreeKeywordEntry(stKeywordEntry *k) {
	slFree(k->keyword);
	stFreeStVar(k->var);

	if(k->defaultKey) delete k->defaultKey;

	delete k;
}

/*!
	\brief Make a new stMethod.

	Given a name and a keyword vector, stNewMethod creates and fills in an stMethod struct.

	Due to the fact that life is complicated and that we will have only seen the function 
	definition at the time this code is called, the code array and the local variables will 
	be filled in later.
*/

stMethod *stNewMethod(char *name, std::vector< stKeywordEntry* > *keywords, char *file, int line) {
	stMethod *method;
	unsigned int count;
	stKeywordEntry *e;

	method = new stMethod;

	method->name = slStrdup(name);
	method->stackOffset = 0;
	method->lineno = line;
	method->filename = slStrdup(file);

	if(keywords) method->keywords = *keywords;

	// step through the keywords, calculation their offsets and required storage space 

	for(count=0;count<method->keywords.size();count++) {
		e = method->keywords[count];

		// we may not be aligned for this data type, so possibly
		// add another 4 bytes or so (it could be a double)...

		method->stackOffset += (method->stackOffset % stAlign(e->var->type));

		e->var->offset = method->stackOffset;
		method->stackOffset += stSizeof(e->var->type);
	} 

	// make sure the stack offset is aligned to any possible data 

	method->stackOffset += (method->stackOffset % stAlignAtomic(AT_DOUBLE));

	return method;
}

/*!
	\brief Frees an stMethod structure.
*/

void stFreeMethod(stMethod *meth) {
	std::vector< stVar* >::iterator vi;
	unsigned int n;

	for(n=0;n<meth->code.size();n++) delete meth->code[n];
	for(n=0;n<meth->keywords.size();n++) stFreeKeywordEntry(meth->keywords[n]);

	slFree(meth->filename);

	for(vi = meth->variables.begin(); vi != meth->variables.end(); vi++) stFreeStVar( *vi );

	slFree(meth->name);

	delete meth;
}

/*!
	\brief Adds a local variable to a method.  

	Returns the variable added to the list of methods, or NULL if the variable
	already exists.
*/

stVar *stMethodAddVar(stVar *var, stMethod *method) {
	if(stFindLocal(var->name, method)) return NULL;

	method->stackOffset += (method->stackOffset % stAlign(var->type));
		
	var->offset = method->stackOffset;

	method->variables.push_back( var);

	method->stackOffset += stSizeof(var->type);

	return var;
}

/*!
	\brief Aligns the method's stack offset.

	Makes sure the stack is aligned to the size of an AT_DOUBLE which should be a 64-bit 
	double.
*/

void stMethodAlignStack(stMethod *method) {
	method->stackOffset += (method->stackOffset % stAlignAtomic(AT_DOUBLE));
}

/*!
	\brief Creates a new stVarType struct.

	objectType is the optional object type.
*/

stVarType *stVarTypeNew(unsigned char type, unsigned char arrayType, int arrayCount, char *objectType) {
	stVarType *t;

	t = new stVarType;

	t->type = type;
	t->arrayType = arrayType;
	t->arrayCount = arrayCount;

	if(objectType) t->objectName = slStrdup(objectType);

	return t;
}

/*!
	\brief Copies an existing stVarType.
*/

stVarType *stVarTypeCopy(stVarType *t) {
	return stVarTypeNew(t->type, t->arrayType, t->arrayCount, t->objectName);	
}

/*!
	\brief Allocates and fills in an stVar.
*/

stVar *stVarNew(char *name, stVarType *type) {
	stVar *v;

	v = new stVar;

	v->name = slStrdup(name);
	v->type = type;
	v->used = 0;

	return v;
}

/*!
	\brief Frees an stVar.
*/

void stFreeStVar(stVar *v) {
	if(!v) return;

	slFree(v->name);

	if(v->type->objectName) slFree(v->type->objectName);

	delete v->type;

	delete v;
}

/*!
	\brief Associates an stVar variable with an object to make an instance variable.
*/

stVar *stInstanceNewVar(stVar *v, stObject *object) {
	if(object->variables[ v->name]) return NULL;

	// make sure that this object's variables are aligned to store the data

	object->varSize += (object->varSize % stAlign(v->type));

	v->offset = object->varSize;

	object->varSize += stSizeof(v->type);

	object->variables[ v->name] = v;

	return v;
}

/*!
	\brief Warns of unused instance variables in an object.
*/

int stUnusedInstanceVarWarning(stObject *o) {
	stVar *var;
	stMethod *method;
	stKeywordEntry *e;
	unsigned int n;

	std::map< std::string, stVar* >::iterator vi;

	for(vi = o->variables.begin(); vi != o->variables.end(); vi++ ) {
		stVar *var = vi->second;

		if(var && !var->used)
			slMessage(DEBUG_ALL, "warning: unused class variable \"%s\" in class %s\n", var->name, o->name);
	}

	std::map< std::string, std::vector< stMethod* > >::iterator mli;

	for(mli = o->methods.begin(); mli != o->methods.end(); mli++ ) {
		std::vector< stMethod*> methodList = mli->second;
		std::vector< stMethod*>::iterator mi;

		for(mi = methodList.begin(); mi != methodList.end(); mi++) {	
			int used = 0;

			method = *mi;

			std::vector< stVar* >::iterator vi;

			for(vi = method->variables.begin(); vi != method->variables.end(); vi++ ) {
				var = *vi;

				if(stObjectLookupVariable(o, var->name)) {
					slMessage(DEBUG_ALL, "warning: local variable \"%s\" in method %s of class %s shadows instance variable of same name\n", var->name, method->name, o->name);
				}

				/* caller is a special variable */

				if(!var->used && strcmp(var->name, "caller")) {
					slMessage(DEBUG_ALL, "warning: unused local variable \"%s\" in method %s of class %s\n", var->name, method->name, o->name);
				} else if(var->used) {
					used++;
				}
			}

			for(n=0;n<method->keywords.size();n++) {
				e = method->keywords[n];

				if(!e->var->used) {
					// slMessage(DEBUG_ALL, "warning: unused keyword \"%s\" in method %s of class %s\n", e->var->name, method->name, o->name);
				} else {
					used++;
				}
			}

			if(used == 0) {
				method->inlined = 1;
			} 
		}
	}

	return 0;
}

/*!
	\brief Finds a variable in an object.

	Searches the parent objects for the variable as well.
*/

stVar *stObjectLookupVariable(stObject *ob, char *word) {
	do {
		std::map< std::string, stVar* >::iterator vi;

		vi = ob->variables.find( word);

		if( vi != ob->variables.end()) return vi->second;
	} while((ob = ob->super));

	return NULL;
}

/*!
	\brief Finds an instance method without searching super-objects.
*/

stMethod *stFindInstanceMethodNoSuper(stObject *object, char *word, unsigned int nArgs) {
	std::vector< stMethod* > methodList;
	std::vector< stMethod* >::iterator mi;
	stMethod *method;

	methodList = object->methods[ word];

	for(mi = methodList.begin(); mi != methodList.end(); mi++ ) {
		method = *mi;
		if(method->keywords.size() == nArgs) return method;
	}

	return NULL;
}

/*!
	\brief Finds an instance method.

	Searches parent objects as well.  Sets foundObject to the object where 
	the method was found, which may be a superclass of object.
*/

stMethod *stFindInstanceMethod(stObject *object, char *word, int nArgs, stObject **foundObject) {
	return stFindInstanceMethodWithArgRange(object, word, nArgs, nArgs, foundObject);
}

/*!
	\brief Finds an instance method with a variable number of arguments.

	Searches parent objects as well.  Sets foundObject to the object where 
	the method was found, which may be a superclass of object.
*/

stMethod *stFindInstanceMethodWithArgRange(stObject *object, char *word, unsigned int minArgs, unsigned int maxArgs, stObject **foundObject) {
	stMethod *method;
	std::vector< stMethod* > methodList;
	std::vector< stMethod* >::iterator mi;

	do {
		methodList = object->methods[ word];

		for(mi = methodList.begin(); mi != methodList.end(); mi++ ) {
			method = *mi;

			if(method->keywords.size() >= minArgs && method->keywords.size() <= maxArgs) {
				if(foundObject) *foundObject = object;
				return method;
			}
		}
	} while((object = object->super));

	return NULL;
}

/*!
	\brief Finds an instance method which accepts at least a given mimimum number of arguments.

	Searches parent objects as well.  Sets foundObject to the object where 
	the method was found, which may be a superclass of object.
*/

stMethod *stFindInstanceMethodWithMinArgs(stObject *object, char *word, unsigned int minArgs, stObject **foundObject) {
	stMethod *method, *best = NULL;

	std::vector< stMethod* > mlist;
	std::vector< stMethod* >::iterator mi;

	do {
		mlist = object->methods[ word];

		for(mi = mlist.begin(); mi != mlist.end(); mi++ ) {
			method = *mi;

			if(method->keywords.size() >= minArgs) {
				// we want to find the method with the lowest number of arguments
				// greater than minArgs.

				if(!best || method->keywords.size() < best->keywords.size() ) {
					best = method;
					if(foundObject) *foundObject = object;
				}
			}
		}
	} while((object = object->super));

	return best;
}

/*!
	\brief Stores an instance method in an object.
*/

int stStoreInstanceMethod(stObject *o, char *word, stMethod *method) {
	unsigned int n;
	std::vector< stMethod* > &mlist = o->methods[ word];

	for(n = 0; n < mlist.size(); n++ )
		if( mlist[ n]->keywords.size() == method->keywords.size() ) return -1;

	o->methods[ word].push_back( method);

	return 0;
}

/*!
	\brief Determines whether a is a subclass of o.
*/

int stIsSubclassOf(stObject *a, stObject *o) {
	while(a) {
		if(o == a) return 1;
		a = a->super;
	}

	return 0; 
}

/*!
	\brief Adds an instance to its class' instance lists.
*/

void stAddToInstanceLists(stInstance *i) {
	stObject *type = i->type;
    
	while(type) { 
		type->allInstances.insert(i);
		type = type->super;
	}
}

/*!
	\brief Removes an instance from its class' instance list.
*/

void stRemoveFromInstanceLists(stInstance *i) {
	stObject *type = i->type;

	while(type) {
		std::set< stInstance*, stInstanceCompare>::iterator ii;

		ii = type->allInstances.find( i);

		if(ii != type->allInstances.end()) type->allInstances.erase( ii);

		type = type->super;
	}
}

/*!
	\brief Returns the required alignment for a given stVarType.

	Intended for aligning 8-byte variables (doubles or 64-bit pointers).  
*/

int stAlign(stVarType *var) {
	if(var->type == AT_ARRAY) return stAlignAtomic(var->arrayType);
	return stAlignAtomic(var->type);
}

/*!
	\brief Returns the required alignment for a given atomic type.

	Called by \ref stAlign.
*/

int stAlignAtomic(int type) {
	switch(type) {
		case AT_INT:
			return sizeof(int);
			break;
		case AT_DOUBLE:
			return sizeof(double);
			break;
		case AT_VECTOR:
			return sizeof(double);
			break;
		case AT_INSTANCE:
			return sizeof(stObject*);
			break;
		case AT_HASH:
			return sizeof(brEvalHash*);
			break;
		case AT_DATA:
		case AT_POINTER:
			return sizeof(void*);
			break;
		case AT_STRING:
			return sizeof(char*);
			break;
		case AT_LIST:
			return sizeof(brEvalList*);
			break;
		case AT_MATRIX:
			return sizeof(double);
			break;
		default:
			slMessage(DEBUG_ALL, "INTERNAL ERROR: stAlign: unknown type %d\n", type);
			return 0;
			break;
	}
}
