#include "kernel.h"

/** \addtogroup API */
/*@{*/

/*!
	\brief Adds a collision handler to an object.

    Defines a behavior for when object type "handler" collides with "collider".
    The behavior is to call the specified method in the handler object
    with the collider instance as an argument.
*/

int brAddCollisionHandler(brObject *handler, brObject *collider, brMethod *m) {
	brCollisionHandler *ch;
	int n;

	for(n=0;n<handler->collisionHandlers->count;n++) {
		ch = handler->collisionHandlers->data[n];

		if(ch->object == collider && ch->method == m) return EC_STOP;
	}

	if(m->argumentCount > 0) {
		if(m->argumentCount > 1) {
			slMessage(DEBUG_ALL, "Collision handlers expect (at most) a single argument of type object.\n");
			return EC_ERROR;
		}
	}

	ch = slMalloc(sizeof(brCollisionHandler));

	ch->object = collider;
	ch->method = m;

	slStackPush(handler->collisionHandlers, ch);

	return EC_OK;
}

/*!
    \brief Finds an object in the given namespace

	returns an object with the given name from the given namespace.
*/

brObject *brObjectFind(brEngine *e, char *name) {
	brNamespaceSymbol *nameSymbol;

	nameSymbol = brNamespaceLookup(e->objects, name);

	if(!nameSymbol) return NULL;

	if(nameSymbol->type != ST_OBJECT && nameSymbol->type != ST_OBJECT_ALIAS) return NULL;

	return nameSymbol->data;
}

/*!
	\brief Finds a method in an object.

	Given an object type, a name, and an argument count, this function
	finds a method and returns it as a brMethod structure.  This allows
	a pointer to the method to be cached, to avoid frequent lookups.
*/

brMethod *brMethodFind(brObject *o, char *name, int argCount) {
	brMethod *m;
	void *mp;

	mp = o->type->findMethod(o, name, argCount);

	if(!mp) return NULL;

	m = slMalloc(sizeof(brMethod));
	m->pointer = mp;
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

brMethod *brMethodFindWithArgRange(brObject *o, char *name, int min, int max) {
	int n;

	for(n=max;n>=min;n--) {
		brMethod *m = brMethodFind(o, name, n);

		if(m) return m;
	}

	return NULL;
}

/*!
	\brief Calls a method for an instance.

	Executes the callMethod callback for to trigger a method call.
*/

int brMethodCall(brInstance *i, brMethod *m, brEval **args, brEval *result) {
	if(i->status != AS_ACTIVE) {
		slMessage(DEBUG_ALL, "warning: method \"%s\" called for released instance %p\n", m->name, i);
		return EC_OK;
	}

	return i->class->type->callMethod(i, m, args, result);
}

/*!
	\brief Find and call a method by name.

	Finds and calls a method by name.  This is ineffecient because it looks
	up the method every time, instead of caching it, so this function should
	not be used if the method is going to be called frequently.
*/

int brMethodCallByName(brInstance *i, char *name, brEval *result) {
	brMethod *m = brMethodFind(i->class, name, 0);
	int r;

	if(!m) {
		slMessage(DEBUG_ALL, "warning: unknown method \"%s\" called for instance %p of class \"%s\"\n", name, i->pointer, i->class->name);
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
*/

int brMethodCallByNameWithArgs(brInstance *i, char *name, brEval **args, int count, brEval *result) {
	brMethod *m = brMethodFind(i->class, name, count);
	int r;

	if(!m) {
		slMessage(DEBUG_ALL, "warning: unknown method \"%s\" called for instance %p of class \"%s\"\n", name, i->pointer, i->class->name);
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

int brAddObserver(brInstance *i, brInstance *observer, char *notification, char *mname) {
    brObserver *o;                                                           
    brMethod *method;
 
	method = brMethodFindWithArgRange(observer->class, mname, 0, 2);
 
    if(!method) {                                                            
        slMessage(DEBUG_ALL, "error adding observer: could not locate method \"%s\" for class %s\n", mname, observer->class->name);
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
    
void brRemoveObserver(brInstance *i, brInstance *observerInstance, char *notification) {
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

int brAddDependency(brInstance *i, brInstance *dependency) {
    if(!i || !dependency) return 0;

    if(!slInList(i->dependencies, dependency)) i->dependencies = slListAppend(i->dependencies, dependency);
    if(!slInList(dependency->dependents, i)) dependency->dependents = slListAppend(dependency->dependents, i);

    return 1;
}

/*!
	\brief Removes a dependency from i.
*/

int brRemoveDependency(brInstance *i, brInstance *dependency) {
    if(!i || !dependency) return 0;

    if(slInList(i->dependencies, dependency)) i->dependencies = slListRemoveData(i->dependencies, dependency);
    if(slInList(dependency->dependents, i)) dependency->dependents = slListRemoveData(dependency->dependents, i);

    return 1;  
}

/*!
	\brief Destroys a brInstance structure.

	Frees all of the data associated with a brInstance structure.
*/

void brInstanceFree(brInstance *i) {
	slList *olist;
	brObserver *observer;
	int n;

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
        brRemoveObserver(olist->data, i, NULL);
		olist = olist->next;
    }

    slListFree(olist);

    slListFree(i->observees);

    for(n=0;n<i->menu.count;n++) {
        slFree(i->menu.list[n]->title);
        slFree(i->menu.list[n]->method);
        slFree(i->menu.list[n]);
    }

	if(i->iterate) brMethodFree(i->iterate);
	if(i->postIterate) brMethodFree(i->postIterate);

    if(i->menu.list) slFree(i->menu.list);

	slFree(i);
}

void brInstanceRelease(brInstance *i) {
	if(!i) return;

	i->status = AS_RELEASED;
}

/*!
	\brief Adds an object to the engine.
*/

brObject *brAddObjectToEngine(brEngine *e, brObjectType *t, char *name, void *pointer) {
	brObject *o;

	if(!name || !t || !e) return NULL;

	o = slMalloc(sizeof(brObject));

	o->name = slStrdup(name);
	o->type = t;
	o->pointer = pointer;
	o->collisionHandlers = slStackNew();

    brNamespaceStore(e->objects, name, ST_OBJECT, o);

	return o;
}

/*!
	\brief Adds an alias for an object.

	An object alias is another name for an existing object.
*/

void brAddObjectAlias(brEngine *e, char *name, brObject *o) {
    brNamespaceStore(e->objects, name, ST_OBJECT_ALIAS, o);
}

/*!
	\brief Frees a breve object.
*/

void brObjectFree(brObject *o) {
	int n;

	for(n=0;n<o->collisionHandlers->count;n++) {
		brCollisionHandler *h = o->collisionHandlers->data[n];

		brMethodFree(h->method);
		slFree(h);
	}

	slStackFree(o->collisionHandlers);
	slFree(o->name);
	slFree(o);
}

/*!
	\brief Adds an instance to the breve engine.

	The instance's iterate method will be called at each iteration.
*/

brInstance *brAddInstanceToEngine(brEngine *e, brObject *class, void *pointer) {
	brMethod *imethod, *pmethod;
	brInstance *i;

    i = slMalloc(sizeof(brInstance));
    i->engine = e;
	i->class = class;
	i->status = AS_ACTIVE;
    
    i->menu.count = 0;
    i->menu.maxCount = 0;
    i->menu.list = NULL;
    i->menu.updateMenu = NULL;

    i->pointer = pointer;

	// it's a bit of a hack, but we need the camera to be informed of
	// new objects in the world.  the code which adds objects to the
	// world doesn't have a pointer to the camera, so we'll do it here

	if(e->camera) e->camera->recompile = 1;

	// find the iterate method which we will call at each iteration

	imethod = brMethodFind(i->class, "iterate", 0);
	pmethod = brMethodFind(i->class, "post-iterate", 0);

	i->iterate = imethod;
	i->postIterate = pmethod;

	slStackPush(e->instances, i);

	if(imethod) slStackPush(e->iterationInstances, i);
	if(pmethod) slStackPush(e->postIterationInstances, i);

	return i;
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
	
/*@}*/
