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

extern stSteveData *gSteveData;

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

	o = slMalloc(sizeof(stObject));

	o->version = version;

	o->engine = engine;
	o->steveData = sdata;
	o->super = super;
	o->variableList = NULL;

	o->name = slStrdup(name);

	// new namespace for the object 

	o->keywords = brNamespaceNew();

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

	sdata->allObjects = slListPrepend(sdata->allObjects, o);

	return o;
}

/*!
	\brief Instantiates a steve object and adds it to the breve engine.

	This method also fills in the "breveInstance" pointer in the steve object.
*/

brInstance *stInstanceCreateAndRegister(brEngine *e, brObject *object) {
	stInstance *newi = NULL;
	brInstance *bi;

	bi = brObjectInstantiate(e, object, NULL, 0);

	if(bi->object->type == &gSteveData->steveObjectType) {
		newi = bi->userData;
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

	i = slMalloc(sizeof(stInstance));

	i->type = o;

	i->variables = slMalloc(o->varSize);
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
}

/*!
	\brief Frees an stInstance.

	Calls destroy on the object and frees its internals.  The pointer
	to the object itself may be kept around or recycled later on.
*/

void stInstanceFreeNoInstanceLists(stInstance *i) {
	brEngine *e;

	brEval result;
	stRunInstance ri;

	ri.instance = i;
	ri.type = i->type;

	stCallMethodByName(&ri, "destroy", &result);
	stMethodTrace(&ri, "delete");

	e = i->type->engine;

	stInstanceFreeInternals(i);

	i->status = AS_RELEASED;

	if(gSteveData->retainFreedInstances) {
		gSteveData->freedInstances = slListPrepend(gSteveData->freedInstances, i);
	} else {
		slFree(i);
	}
}

/*!
	\brief Free the internals of an stInstance.

	This method should not be called directly, as it is called via stInstanceFree.
*/

void stInstanceFreeInternals(stInstance *i) {
	slList *listHead, *list;
	stObject *type;
	stVar *var;
	int n;

	if(i->status == AS_RELEASED) return;

	// garbage collect the variables.

	type = i->type;

	while(type) {
		list = listHead = brNamespaceSymbolList(type->keywords);

		while(listHead) {
			if(((brNamespaceSymbol*)listHead->data)->type == ST_VAR) {
				char *varpointer;

				var = ((brNamespaceSymbol*)listHead->data)->data;

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

			listHead = listHead->next;
		}

		type = type->super;

		slListFree(list);
	}

	slFree(i->variables);
}

/*!
	\brief Free all instances of a certain object type.

	To be used for cleanup only.
*/

void stObjectFreeAllInstances(stObject *o) {
	slList *all;

	// it's possible that the allInstances list will change while
	// we're going through it!  We'll use a copy instead.

	all = slListCopy(o->allInstances);

	while(all) {
		stInstance *i = all->data;

		// only object matching the base class should be freed now

		if(i->type == o) 
			stInstanceFreeNoInstanceLists(all->data);

		all = all->next;
	}

	slListFree(all);
}

/*!
	\brief deallocates an stObject.  

	The object no longer exists and all instances of the object will be invalid.
*/

void stObjectFree(stObject *o) {
	slList *symbolList, *start, *mList;
	brNamespaceSymbol *symbol;

	start = symbolList = brNamespaceSymbolList(o->keywords);

	// free all of the variable/method symbols associated with the object 

	while(symbolList) {
		symbol = symbolList->data;

		switch(symbol->type) {
			case ST_METHODS:
				// the methods are going to be a list of methods with the same name.

				mList = symbol->data;

				while(mList) {
					stFreeMethod(mList->data);
					mList = mList->next;
				}

				slListFree(symbol->data);

				break;
			case ST_VAR:
				stFreeStVar(symbol->data);
				break;
			default:
				slMessage(DEBUG_WARN, "warning: found unknown symbol type %d in object\n", symbol->type);
				break;
		}

		symbolList = symbolList->next;
	}

	slListFree(o->allInstances);

	brNamespaceFree(o->keywords);

	slListFree(start);
	slListFree(o->variableList);

	slFree(o->name);

	slFree(o);
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
	int n;

	if(!m->keywords) return NULL;

	for(n=0;n<m->keywords->count;n++) {
		e = m->keywords->data[n];
		if(!strcmp(keyword, e->keyword)) return e;
	}

	return NULL;
}

/*!
	\brief locate the desired keyword entry or local variable in a method.
*/

stVar *stFindLocal(char *name, stMethod *m) {
	stKeywordEntry *e;
	slList *varList;
	int n;

	varList = m->variables;

	/* is a local variable? */

	while(varList) {
		if(!strcmp(((stVar*)varList->data)->name, name)) return varList->data;
		varList = varList->next;
	}

	/* perhaps it's a keyword argument to the method... */

	if(m->keywords) {
		for(n=0;n<m->keywords->count;n++) {
			e = m->keywords->data[n];
			if(!strcmp(name, e->var->name)) return e->var;
		}
	}

	/* it's not here */

	return NULL;
}

/*!
	\brief Fills in a keyword entry struct.  

	The keyword entry will eventually be added to a methods list of keywords.
*/

stKeywordEntry *stNewKeywordEntry(char *keyword, stVar *v, brEval *defValue) {
	stKeywordEntry *e;
	brEval *copy;

	e = slMalloc(sizeof(stKeywordEntry));

	if(!e || !v || !keyword) return NULL;


	e->keyword = slStrdup(keyword);
	e->var = v;
	e->defaultKey = NULL;

	if(defValue) {
		copy = slMalloc(sizeof(brEval));
		memcpy(copy, defValue, sizeof(brEval));
		e->defaultKey = stNewKeyword(keyword, stExpNew(copy, ET_ST_EVAL, NULL, 0));
	}

	return e;
}

/*!
	\brief Frees a stKeywordEntry.
*/

void stFreeKeywordEntry(stKeywordEntry *k) {
	slFree(k->keyword);
	stFreeStVar(k->var);
	if(k->defaultKey) stFreeKeyword(k->defaultKey);
	slFree(k);
}

/*!
	\brief Frees an slArray of stKeywordEntry structs.
*/

void stFreeKeywordEntryArray(slArray *a) {
	int n;
	
	if(!a) return;

	for(n=0;n<a->count;n++) stFreeKeywordEntry(a->data[n]);
	
	slFreeArray(a);
}  

/*!
	\brief Make a new stMethod.

	Given a name and a keyword slArray stNewMethod creates and fills in an stMethod struct.

	Due to the fact that life is complicated and that we will have only seen the function 
	definition at the time this code is called, the code array and the local variables will 
	be filled in later.
*/

stMethod *stNewMethod(char *name, slArray *keywords, char *file, int line) {
	stMethod *s;
	int count;
	stKeywordEntry *e;

	s = slMalloc(sizeof(stMethod));

	s->name = slStrdup(name);

	s->stackOffset = 0;

	s->lineno = line;
	s->filename = slStrdup(file);

	/* step through the keywords, calculation their offsets and */
	/* required storage space */

	if(keywords) {
		for(count=0;count<keywords->count;count++) {
			e = keywords->data[count];

			/* we may not be aligned for this data type, so possibly */
			/* add another 4 bytes or so (it could be a double) ...  */

			s->stackOffset += (s->stackOffset % stAlign(e->var->type));

			e->var->offset = s->stackOffset;
			s->stackOffset += stSizeof(e->var->type);
		}
	} 

	/* make sure the stack offset is aligned to any possible data */

	s->stackOffset += (s->stackOffset % stAlignAtomic(AT_DOUBLE));

	s->keywords = keywords;

	s->code = NULL;
	s->variables = NULL;

	return s;
}

/*!
	\brief Frees an stMethod structure.
*/

void stFreeMethod(stMethod *meth) {
	slList *variables;

	stExpFreeArray(meth->code);
	stFreeKeywordEntryArray(meth->keywords);

	slFree(meth->filename);

	variables = meth->variables;

	while(variables) {
		stFreeStVar(variables->data);
		variables = variables->next;
	}

	slFree(meth->name);
	slListFree(meth->variables);

	slFree(meth);
}

/*!
	\brief Adds a local variable to a method.  
*/

slList *stMethodAddVar(stVar *var, stMethod *method) {
	if(stFindLocal(var->name, method)) return NULL;

	method->stackOffset += (method->stackOffset % stAlign(var->type));
		
	var->offset = method->stackOffset;

	if(var->type->type == AT_LIST || var->type->type == AT_STRING) method->gcVariables = 1;

	method->variables = slListPrepend(method->variables, var);

	method->stackOffset += stSizeof(var->type);

	return method->variables;
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

	t = slMalloc(sizeof(stVarType));

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

	v = slMalloc(sizeof(stVar));

	v->name = slStrdup(name);
	v->type = type;
	v->used = 0;

	return v;
}

/*!
	\brief Frees an stVar.
*/

void stFreeStVar(stVar *v) {
	slFree(v->name);

	if(v->type->objectName) slFree(v->type->objectName);

	slFree(v->type);
	slFree(v);
}

/*!
	\brief Associates an stVar variable with an object to make an instance variable.
*/

brNamespaceSymbol *stInstanceNewVar(stVar *v, stObject *object) {
	brNamespaceSymbol *var;

	var = brNamespaceLookup(object->keywords, v->name);

	/* symbol redefined--return NULL */

	if(var) return NULL;

	/* make sure that this object's variables are aligned to store */
	/* this data type. */

	object->varSize += (object->varSize % stAlign(v->type));

	v->offset = object->varSize;

	object->varSize += stSizeof(v->type);
	object->variableList = slListPrepend(object->variableList, v);

	return brNamespaceStore(object->keywords, v->name, ST_VAR, v);
}

/*!
	\brief Warns of unused instance variables in an object.
*/

int stUnusedInstanceVarWarning(stObject *o) {
	slList *theList, *vList, *mList, *listStart;
	brNamespaceSymbol *symbol;
	stVar *var;
	stMethod *method;
	stKeywordEntry *e;
	int n;

	listStart = theList = brNamespaceSymbolList(o->keywords);

	while(theList) {
		symbol = theList->data;

		if(symbol->type == ST_VAR) {
			var = symbol->data;

			if(!var->used)
				slMessage(DEBUG_ALL, "warning: unused class variable \"%s\" in class %s\n", var->name, o->name);
		}

		if(symbol->type == ST_METHODS) {
			int used;

			mList = symbol->data;

			while(mList) {
				method = mList->data;
				vList = method->variables;

				used = 0;

				while(vList) {
					var = vList->data;

					if(stObjectLookup(o, var->name, ST_VAR)) {
						slMessage(DEBUG_ALL, "warning: local variable \"%s\" in method %s of class %s shadows instance variable of same name\n", var->name, method->name, o->name);
					}

					/* caller is a special variable */

					if(!var->used && strcmp(var->name, "caller")) {
						slMessage(DEBUG_ALL, "warning: unused local variable \"%s\" in method %s of class %s\n", var->name, method->name, o->name);
					} else if(var->used) {
						used++;
					}

					vList = vList->next;
				}

				if(method->keywords) {
					for(n=0;n<method->keywords->count;n++) {
						e = method->keywords->data[n];

						if(!e->var->used) {
							slMessage(DEBUG_ALL, "warning: unused keyword \"%s\" in method %s of class %s\n", e->var->name, method->name, o->name);
						} else {
							used++;
						}
					}
				}

				if(used == 0) {
					method->inlined = 1;
				} 

				mList = mList->next;
			}
		}

		theList = theList->next;
	}

	slListFree(listStart);

	return 0;
}

/*!
	\brief Finds a symbol of a specified type inside an object.

	Searches the parent objects for the symbol as well.
*/

brNamespaceSymbol *stObjectLookup(stObject *ob, char *word, unsigned char type) {
	brNamespaceSymbol *var;

	do {
		var = brNamespaceLookup(ob->keywords, word);
	} while(!var && (ob = ob->super));

	if(var && var->type == type) return var;
  
	return NULL;
}

/*!
	\brief Finds an instance method without searching super-objects.
*/

stMethod *stFindInstanceMethodNoSuper(stObject *o, char *word, int nArgs) {
	brNamespaceSymbol *var;
	slList *list;
	stMethod *method;

	var = brNamespaceLookup(o->keywords, word);

	if(var) {
		if(var->type != ST_METHODS) return NULL;

		list = var->data;

		while(list) {
			method = list->data;
			if(method->keywords->count == nArgs) return method;

			list = list->next;
		}
	}

	return NULL;
}

/*!
	\brief Finds an instance method.

	Searches parent objects as well.
*/

stMethod *stFindInstanceMethod(stObject *o, char *word, int nArgs, stObject **oo) {
	return stFindInstanceMethodWithArgRange(o, word, nArgs, nArgs, oo);
}

/*!
	\brief Finds an instance method with a variable number of arguments.

	Searches parent objects as well.
*/

stMethod *stFindInstanceMethodWithArgRange(stObject *o, char *word, int minArgs, int maxArgs, stObject **oo) {
	brNamespaceSymbol *var;
	slList *list;
	stMethod *method;

	do {
		var = brNamespaceLookup(o->keywords, word);

		if(var) {
			if(var->type != ST_METHODS) return NULL;

			list = var->data;

			while(list) {
				method = list->data;
				if(method->keywords->count >= minArgs && method->keywords->count <= maxArgs) {
					if(oo) *oo = o;
					return method;
				}

				list = list->next;
			}
		}


	} while((o = o->super));

	return NULL;
}

/*!
	\brief Finds an instance method which accepts at least a given mimimum number of arguments.

    Searches parent objects as well.
*/

stMethod *stFindInstanceMethodWithMinArgs(stObject *o, char *word, int minArgs, stObject **oo) {
	brNamespaceSymbol *var;
	slList *list;
	stMethod *method, *best = NULL;

	do {
		var = brNamespaceLookup(o->keywords, word);

		if(var) {
			if(var->type != ST_METHODS) return NULL;

			list = var->data;

			while(list) {
				method = list->data;

				if(method->keywords->count >= minArgs) {
					// we want to find the method with the lowest number of arguments
					// greater than minArgs.

					if(!best || method->keywords->count < best->keywords->count) {
						best = method;
						if(oo) *oo = o;
					}
				}

				list = list->next;
			}
		}

	} while((o = o->super));

	return best;
}

/*!
	\brief Stores an instance method in an object.
*/

int stStoreInstanceMethod(stObject *o, char *word, stMethod *method) {
	brNamespaceSymbol *symbol = brNamespaceLookup(o->keywords, word);

	if(symbol && symbol->type != ST_METHODS) return -1;

	if(symbol) symbol->data = slListPrepend((slList*)symbol->data, method);
	else brNamespaceStore(o->keywords, word, ST_METHODS, slListPrepend(NULL, method));

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
		type->allInstances = slListPrepend(type->allInstances, i);
		type = type->super;
	}
}

/*!
	\brief Removes an instance from its class' instance list.
*/

void stRemoveFromInstanceLists(stInstance *i) {
	stObject *type = i->type;

	while(type) {
		type->allInstances = slListRemoveData(type->allInstances, i);
		type = type->super;
	}
}
