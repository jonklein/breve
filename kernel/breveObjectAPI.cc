#include "kernel.h"

/** \defgroup breveObjectAPI The breve object API: constructing a new language frontend for breve

	The functions and structures in this group are used for creating 
	custom language frontends for breve.  If you wish to construct
	breve simulations from other languages, you'll need to use the 
	functions shown here to interface with the breve object API.
*/

/*@{*/

/*!
	\brief Registers a new object type with the engine.
*/

void brEngineRegisterObjectType(brEngine *e, brObjectType *t) {
	if(t) slStackPush(e->objectTypes, t);
}

/*!
	\brief Finds a method in an object.

	Given an object type, a name, and an argument count, this function
	finds a method and returns it as a brMethod structure.  This allows
	a pointer to the method to be cached, to avoid frequent lookups.
*/

brMethod *brMethodFind(brObject *o, char *name, unsigned char *types, int argCount) {
	brMethod *m;
	void *mp;
	int n;

	if(!types && argCount) {
		types = alloca(argCount);
		for(n=0;n<argCount;n++) types[n] = AT_UNDEFINED;
	}

	mp = o->type->findMethod(o->userData, name, types, argCount);

	if(!mp) return NULL;

	m = slMalloc(sizeof(brMethod));
	m->userData = mp;
	m->argumentCount = argCount;
	m->name = slStrdup(name);

	return m;
}

/*!
	\brief Finds a method with a range of argument counts.

	Looks for a method named "name" that accepts between min and max 
	arguments.  Will return the method with the highest number of 
	arguments possible.

	This is used for callbacks which can optionally accept arguments.
	If the user has specified that the method accepts arguments, they
	are provided.  
*/

brMethod *brMethodFindWithArgRange(brObject *o, char *name, unsigned char *types, int min, int max) {
	int n;

	for(n=max;n>=min;n--) {
		brMethod *m = brMethodFind(o, name, types, n);

		if(m) return m;
	}

	return NULL;
}

/*!
    \brief Finds an object in the given namespace

	Looks up an object in the engine's table of known objects.  If 
	the object cannot be found, \ref brUnknownObjectFind is called 
	to ask each language frontend to locate the object.
*/

brObject *brObjectFind(brEngine *e, char *name) {
	brNamespaceSymbol *nameSymbol;

	nameSymbol = brNamespaceLookup(e->objects, name);

	if(!nameSymbol) return brUnknownObjectFind(e, name);

	if(nameSymbol->type != ST_OBJECT && nameSymbol->type != ST_OBJECT_ALIAS) return NULL;

	return nameSymbol->data;
}

/*!
	\brief Looks up an unknown object and adds it to the engine.

	Uses the language frontend callbacks to locate and register
	an object that does not currently exist in the engine.
*/

brObject *brUnknownObjectFind(brEngine *e, char *name) {
	int n;

	for(n=0;n<e->objectTypes->count;n++) {
		brObjectType *type = e->objectTypes->data[n];
		void *pointer = NULL;

		if(type->findObject) {
			pointer = type->findObject(type->userData, name);

			if(pointer) return brEngineAddObject(e, type, name, pointer);
		}
	}

	return NULL;
}

/*!
	\brief Calls a method for an instance.

	Executes the callMethod callback for to trigger a method call.

	WARNING: the brEval values stored in args may be changed by the 
	method call, depending on the implementation of the language frontend.
*/

int brMethodCall(brInstance *i, brMethod *m, brEval **args, brEval *result) {
	if(i->status != AS_ACTIVE) {
		slMessage(DEBUG_ALL, "warning: method \"%s\" called for released instance %p\n", m->name, i);
		return EC_OK;
	}

	return i->object->type->callMethod(i->userData, m->userData, args, result);
}

/*!
	\brief Find and call a method by name.

	Finds and calls a method by name.  This is ineffecient because it looks
	up the method every time, instead of caching it, so this function should
	not be used if the method is going to be called frequently.
*/

int brMethodCallByName(brInstance *i, char *name, brEval *result) {
	brMethod *m = brMethodFind(i->object, name, NULL, 0);
	int r;

	if(!m) {
		slMessage(DEBUG_ALL, "warning: unknown method \"%s\" called for instance %p of class \"%s\"\n", name, i->userData, i->object->name);
		return EC_ERROR;
	}

	r = brMethodCall(i, m, NULL, result);

	brMethodFree(m);

	return r;
}

/*!
	\brief Find and call a method by name, with arguments.

	As with \ref brMethodCallByName, this method is inefficient because
	it has to look up the method being called.  This function should 
	only be used for sporatic method calls.

	WARNING: the brEval values stored in args may be changed by the 
	method call, depending on the implementation of the language frontend.
*/

int brMethodCallByNameWithArgs(brInstance *i, char *name, brEval **args, int count, brEval *result) {
	brMethod *m = brMethodFind(i->object, name, NULL, count);
	int r;

	if(!m) {
		slMessage(DEBUG_ALL, "warning: unknown method \"%s\" called for instance %p of class \"%s\"\n", name, i->userData, i->object->name);
		return EC_ERROR;
	}

	r = brMethodCall(i, m, args, result);

	brMethodFree(m);

	return r;
}

/*!
	\brief Registers an instance as an observer.
		
	Registers "observer" as an observer of "i", waiting for the specified 
	notification.  When the notificication is announced, the method mname 
	is executed for the observer.
*/

int brInstanceAddObserver(brInstance *i, brInstance *observer, char *notification, char *mname) {
    brObserver *o;                                                           
    brMethod *method;
	unsigned char types[] = { AT_INSTANCE, AT_STRING };
 
	method = brMethodFindWithArgRange(observer->object, mname, types, 0, 2);
 
    if(!method) {                                                            
        slMessage(DEBUG_ALL, "error adding observer: could not locate method \"%s\" for class %s\n", mname, observer->object->name);
        return -1; 
    }
 
    o = slMalloc(sizeof(brObserver));                                        
    o->instance = observer; 
    o->method = method;
    o->notification = slStrdup(notification);
 
    i->observers = slListPrepend(i->observers, o);
    observer->observees = slListPrepend(observer->observees, i);

    return 0;
}   

/*!
	\brief Removes an instance from an object's observer list.

	Stops observerInstance from waiting for the specified notification from 
	instance i.
*/
    
void brEngineRemoveInstanceObserver(brInstance *i, brInstance *observerInstance, char *notification) {
    slList *observerList, *match, *last;
    brObserver *observer;

    observerList = i->observers;
    
    last = NULL;

    if(i->status == AS_FREED || observerInstance->status == AS_FREED) return;

    while(observerList) {
        observer = observerList->data;

        if(observer->instance == observerInstance && (!notification || !strcmp(notification, observer->notification))) {
            match = observerList;

            if(last) last->next = observerList->next;
            else i->observers = observerList->next;

            observerList = observerList->next;

            slFree(match);
        } else {
            last = observerList;
            observerList = observerList->next;
        }
    }

    observerInstance->observees = slListRemoveData(observerInstance->observees, i);
}

/*!
	\brief Adds an object dependency for i.

	Adds an object dependency for i.  This doesn't do anything to the object in 
	and of itself, but the information may be used later on for copying or 
	archiving the instance.
*/

int brInstanceAddDependency(brInstance *i, brInstance *dependency) {
    if(!i || !dependency) return 0;

    if(!slInList(i->dependencies, dependency)) i->dependencies = slListAppend(i->dependencies, dependency);
    if(!slInList(dependency->dependents, i)) dependency->dependents = slListAppend(dependency->dependents, i);

    return 1;
}

/*!
	\brief Removes a dependency from i.
*/

int brEngineRemoveInstanceDependency(brInstance *i, brInstance *dependency) {
    if(!i || !dependency) return 0;

    if(slInList(i->dependencies, dependency)) i->dependencies = slListRemoveData(i->dependencies, dependency);
    if(slInList(dependency->dependents, i)) dependency->dependents = slListRemoveData(dependency->dependents, i);

    return 1;  
}


/*!
	\brief Adds an object to the engine.
*/

brObject *brEngineAddObject(brEngine *e, brObjectType *t, char *name, void *pointer) {
	brObject *o;

	if(!name || !t || !e) return NULL;

	o = slMalloc(sizeof(brObject));

	o->name = slStrdup(name);
	o->type = t;
	o->userData = pointer;
	o->collisionHandlers = slStackNew();

	brNamespaceStore(e->objects, name, ST_OBJECT, o);

	return o;
}

/*!
	\brief Adds an alias for an object.

	An object alias is another name for an existing object.
*/

void brEngineAddObjectAlias(brEngine *e, char *name, brObject *o) {
    brNamespaceStore(e->objects, name, ST_OBJECT_ALIAS, o);
}

/*!
	\brief Adds an instance to the breve engine.

	The instance's iterate method will be called at each iteration.
*/

brInstance *brEngineAddInstance(brEngine *e, brObject *object, void *pointer) {
	brMethod *imethod, *pmethod;
	brInstance *i;
	int n;

	for(n=0;n<e->instances->count;n++) {
		i = e->instances->data[n];
		if(i->userData == pointer) return NULL;
	}

    i = slMalloc(sizeof(brInstance));
    i->engine = e;
	i->object = object;
	i->status = AS_ACTIVE;
    
    i->menu.count = 0;
    i->menu.maxCount = 0;
    i->menu.list = NULL;
    i->menu.updateMenu = NULL;

    i->userData = pointer;

	// it's a bit of a hack, but we need the camera to be informed of
	// new objects in the world.  the code which adds objects to the
	// world doesn't have a pointer to the camera, so we'll do it here

	if(e->camera) e->camera->recompile = 1;

	// find the iterate method which we will call at each iteration

	imethod = brMethodFind(i->object, "iterate", NULL, 0);
	pmethod = brMethodFind(i->object, "post-iterate", NULL, 0);

	i->iterate = imethod;
	i->postIterate = pmethod;

	slStackPush(e->instances, i);

	if(imethod) slStackPush(e->iterationInstances, i);
	if(pmethod) slStackPush(e->postIterationInstances, i);

	return i;
}

brInstance *brObjectInstantiate(brEngine *e, brObject *o, brEval **args, int argCount) {
	return brEngineAddInstance(e, o, o->type->instantiate(o->userData, args, argCount));
}

/*!
	\brief Marks a \ref brInstance as released, so that it can be removed
	from the engine.

	The instance will be removed from the engine and freed at the end of 
	the next simulation iteration.  This function should be used instead of
	\ref brInstanceFree except during simulation deallocation and cleanup.
*/

void brInstanceRelease(brInstance *i) {
	if(!i) return;

	i->status = AS_RELEASED;
}

/*!
	\brief Removes an object from the engine.

	The object may still exist in the simulation (technically), but it
	will no longer be iterated by the engine.
*/

void brEngineRemoveInstance(brEngine *e, brInstance *i) {
	// inform the camera of the change

	if(e->camera) e->camera->recompile = 1;

	slStackRemove(e->instances, i);
	slStackRemove(e->iterationInstances, i);
	slStackRemove(e->postIterationInstances, i);
}

/*!
	\brief Frees a brMethod structure.

	If you have generated a brMethod structure with \ref brMethodFind, 
	you must free it using this method when you are done with it.
*/

void brMethodFree(brMethod *m) {
	if(m->name) slFree(m->name);
	slFree(m);
}

/*!
	\brief Frees a breve object.
*/

void brObjectFree(brObject *o) {
	int n;

	for(n=0;n<o->collisionHandlers->count;n++) {
		brCollisionHandler *h = o->collisionHandlers->data[n];

		if(h->method) brMethodFree(h->method);
		slFree(h);
	}

	slStackFree(o->collisionHandlers);
	slFree(o->name);
	slFree(o);
}

/*!
	\brief Destroys a \ref brInstance structure.

	Immediately frees all of the data associated with a \ref brInstance 
	structure.  This function should not be used while a breve engine 
	is being actively iterated.  It may be used for cleanup when the
	engine isn't running, but otherwise use \ref brInstanceRelease instead.
*/

void brInstanceFree(brInstance *i) {
	slList *olist, *events;
	brObserver *observer;
	int n;

	if(i && i->userData) i->object->type->destroyInstance(i->userData);

    slListFree(i->dependencies);
    slListFree(i->dependents);

    olist = slListCopy(i->observers);

    while(olist) {
        observer = olist->data;
        slFree(observer->notification);
        slFree(observer);
        olist = olist->next;
    }

    slListFree(olist);
    slListFree(i->observers);

    i->observers = NULL;

	// removing observers will modify the observee list,
	// so copy the list first

	olist = slListCopy(i->observees);

    while(olist) {
        brEngineRemoveInstanceObserver(olist->data, i, NULL);
		olist = olist->next;
    }

    slListFree(olist);

    slListFree(i->observees);

	// free the instance's events

	events = i->events;

	while(events) {
		brEventFree(events->data);
		events = events->next;
	}

	slListFree(i->events);

    for(n=0;n<i->menu.count;n++) {
        slFree(i->menu.list[n]->title);
        slFree(i->menu.list[n]->method);
        slFree(i->menu.list[n]);
    }

	if(i->iterate) brMethodFree(i->iterate);
	if(i->postIterate) brMethodFree(i->postIterate);

    if(i->menu.list) slFree(i->menu.list);

	i->userData = NULL;

	i->engine->freedInstances = slListPrepend(i->engine->freedInstances, i);
}

/*!
	\brief Adds a collision handler to an object.

    Defines a behavior for when object type "handler" collides with "collider".
    The behavior is to call the specified method in the handler object
    with the collider instance as an argument.
*/

int brObjectAddCollisionHandler(brObject *handler, brObject *collider, char *name) {
	brCollisionHandler *ch;
	brMethod *method;
	int n;
	unsigned char types[] = { AT_INSTANCE, AT_DOUBLE };

	for(n=0;n<handler->collisionHandlers->count;n++) {
		ch = handler->collisionHandlers->data[n];

		if(ch->object == collider) return EC_STOP;
	}

	method = brMethodFindWithArgRange(handler, name, types, 0, 1);

	if(!method) {
		slMessage(DEBUG_ALL, "Error adding collision handler: cannot locate method \"%s\" for class \"%s\"\n", handler->name, name);
		return EC_ERROR;
    }

	ch = slMalloc(sizeof(brCollisionHandler));

	ch->object = collider;
	ch->method = method;
	ch->ignore = 0;

	slStackPush(handler->collisionHandlers, ch);

	return EC_OK;
}


int brObjectSetIgnoreCollisionsWith(brObject *handler, brObject *collider, int ignore) {
	brCollisionHandler *ch;
	int n;

	for(n=0;n<handler->collisionHandlers->count;n++) {
		ch = handler->collisionHandlers->data[n];

		if(ch->object == collider) {
			ch->ignore = ignore;
			return EC_OK;
		}
	}

	ch = slMalloc(sizeof(brCollisionHandler));

	ch->object = collider;
	ch->method = NULL;
	ch->ignore = ignore;

	slStackPush(handler->collisionHandlers, ch);

	return EC_OK;
}

/*@}*/
