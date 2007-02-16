#include "kernel.h"
#include "camera.h"

/** \defgroup breveObjectAPI The breve object API: constructing a new language frontend for breve

	The functions and structures in this group are used for creating
	custom language frontends for breve.  If you wish to construct
	breve simulations from other languages, you'll need to use the
	functions shown here to interface with the breve object API.
*/

/*@{*/

/**
 * \brief Registers a new object type with the engine.
 */

void brEngineRegisterObjectType( brEngine *e, brObjectType *t ) {
	e->objectTypes.push_back( t );
}

/**
 * \brief Finds a method in an object.
 * 
 * Given an object type, a name, and an argument count, this function
 * finds a method and returns it as a brMethod structure.  This allows
 * a pointer to the method to be cached, to avoid frequent lookups.
 */

brMethod *brMethodFind( brObject *o, const char *name, unsigned char *types, int argCount ) {
	brMethod *m;
	unsigned char *t = NULL;
	void *mp;
	int n;

	if ( !types && argCount ) {
		t = new unsigned char[ argCount ];
		types = t;

		for ( n = 0;n < argCount;n++ ) types[n] = AT_UNDEFINED;
	}

	mp = o->type->findMethod( o->userData, name, types, argCount );

	if ( t ) 
		delete[] t;

	if ( !mp ) 
		return NULL;

	m = new brMethod;

	m->userData = mp;
	m->object = o;
	m->argumentCount = argCount;
	m->name = slStrdup( name );

	return m;
}

/**
 * \brief Finds a method with a range of argument counts.
 * 
 * Looks for a method named "name" that accepts between min and max
 * arguments.  Will return the method with the highest number of
 * arguments possible.
 * 
 * This is used for callbacks which can optionally accept arguments.
 * If the user has specified that the method accepts arguments, they
 * are provided.
 */

brMethod *brMethodFindWithArgRange( brObject *o, const char *name, unsigned char *types, int min, int max ) {
	int n;

	for ( n = max;n >= min;n-- ) {
		brMethod *m = brMethodFind( o, name, types, n );

		if ( m ) 
			return m;
	}

	return NULL;
}

/**
 * \brief Finds an object in the given namespace
 *
 * Looks up an object in the engine's table of known objects.  If
 * the object cannot be found, \ref brUnknownObjectFind is called
 * to ask each language frontend to locate the object.
 */

brObject *brObjectFind( brEngine *e, const char *name ) {
	brObject *object;
	std::string names = name;

	if ( ( object = e->objects[ names] ) ) return object;
	if ( ( object = e->objectAliases[ names ] ) ) return object;

	return brUnknownObjectFind( e, name );
}

/**
 * \brief Looks up an unknown object and adds it to the engine.
 *
 * Uses the language frontend callbacks to locate and register
 * an object that does not currently exist in the engine.
 */

brObject *brUnknownObjectFind( brEngine *e, const char *name ) {
	std::vector<brObjectType*>::iterator oi;

	for ( oi = e->objectTypes.begin(); oi != e->objectTypes.end(); oi++ ) {
		brObjectType *type = *oi;
		void *pointer = NULL;

		if ( type->findObject ) {
			pointer = type->findObject( type->userData, name );

			if ( pointer ) 
				return brEngineAddObject( e, type, name, pointer );
		}
	}

	return NULL;
}

/**
 * \brief Returns the userData field of a brInstance.
 */

void *brInstanceGetUserData( brInstance *i ) {
	return i->userData;
}

/**
 * \brief Returns the userData field of a brObject.
 */

void *brObjectGetUserData( brObject *o ) {
	return o->userData;
}

/**
 * \brief Calls a method for an instance.
 *
 * Executes the callMethod callback for to trigger a method call.
 * WARNING: the brEval values stored in args may be changed by the
 * method call, depending on the implementation of the language frontend.
 */

int brMethodCall( brInstance *i, brMethod *m, const brEval **args, brEval *result ) {
	if ( i->status != AS_ACTIVE ) {
		slMessage( DEBUG_ALL, "warning: method \"%s\" called for released instance %p\n", m->name, i );
		return EC_OK;
	}

	int r = i->object->type->callMethod( i->userData, m->userData, args, result );

	return r;
}

/**
 * Find and call a method by name.
 *
 * Finds and calls a method by name.  This is ineffecient because it looks
 * up the method every time, instead of caching it, so this function should
 * not be used if the method is going to be called frequently.
 */

int brMethodCallByName( brInstance *i, const char *name, brEval *result ) {
	brMethod *m = brMethodFind( i->object, name, NULL, 0 );
	int r;

	if ( !m ) {
		slMessage( DEBUG_ALL, "unknown method \"%s\" called for instance %p of class \"%s\"\n", name, i->userData, i->object->name );
		return EC_ERROR;
	}

	r = brMethodCall( i, m, NULL, result );

	delete m;

	return r;
}

/**
 * \brief Find and call a method by name, with arguments.
 * As with \ref brMethodCallByName, this method is inefficient because
 * it has to look up the method being called.  This function should
 * only be used for sporatic method calls.
 *
 * WARNING: the brEval values stored in args may be changed by the
 * method call, depending on the implementation of the language frontend.
 */

int brMethodCallByNameWithArgs( brInstance *i, const char *name, const brEval **args, int count, brEval *result ) {
	brMethod *m = brMethodFind( i->object, name, NULL, count );
	int r;

	if ( !m ) {
		slMessage( DEBUG_ALL, "warning: unknown method \"%s\" called for instance %p of class \"%s\"\n", name, i->userData, i->object->name );
		return EC_ERROR;
	}

	r = brMethodCall( i, m, args, result );

	delete m;

	return r;
}

/**
 * \brief Registers an instance as an observer.
 * 
 * Registers "observer" as an observer of "i", waiting for the specified
 * notification.  When the notificication is announced, the method mname
 * is executed for the observer.
 */

int brInstanceAddObserver( brInstance *i, brInstance *observer, char *notification, char *mname ) {
	brObserver *o;
	brMethod *method;
	unsigned char types[] = { AT_INSTANCE, AT_STRING };

	method = brMethodFindWithArgRange( observer->object, mname, types, 0, 2 );

	if ( !method ) {
		slMessage( DEBUG_ALL, "error adding observer: could not locate method \"%s\" for class %s\n", mname, observer->object->name );
		return -1;
	}

	o = new brObserver( observer, method, notification );

	i->observers.push_back( o );
	observer->observees.push_back( i );

	return 0;
}

/**
 * \brief Removes an instance from an object's observer list.
 *
 * Stops observerInstance from waiting for the specified notification from
 * instance i.
 */

void brEngineRemoveInstanceObserver( brInstance *i, brInstance *observerInstance, char *notification ) {
	brObserver *observer;

	if ( i->status == AS_FREED || observerInstance->status == AS_FREED ) 
		return;

	for( unsigned int n = 0; n < i->observers.size(); n++ ) {
		observer = i->observers[ n ];

		if ( observer->instance == observerInstance && ( !notification || !strcmp( notification, observer->notification ) ) ) {
			std::vector< brObserver* >::iterator observermatch;

			observermatch = std::find( i->observers.begin(), i->observers.end(), observer );

			if( observermatch != i->observers.end() ) 
				i->observers.erase( observermatch );

			delete observer;
		} 
	}

	std::vector< brInstance* >::iterator observee;

	observee = std::find( observerInstance->observees.begin(), observerInstance->observees.end(), i );

	if( observee != observerInstance->observees.end() ) 
		observerInstance->observees.erase( observee );
}

/**
 * \brief Adds an object to the engine.
 */

brObject *brEngineAddObject( brEngine *e, brObjectType *t, const char *name, void *pointer ) {
	brObject *o;
	std::string names = name;

	if ( !name || !t || !e ) return NULL;

	o = new brObject;

	o->name = slStrdup( name );

	o->type = t;

	o->userData = pointer;

	e->objects[ names ] = o;

	return o;
}

/**
 * \brief Adds an alias for an object.
 *
 * An object alias is another name for an existing object.
*/

void brEngineAddObjectAlias( brEngine *e, char *name, brObject *o ) {
	std::string names = name;

	e->objectAliases[name] = o;
}

/**
 * \brief Adds an instance to the breve engine.
 *
 * The instance's iterate method will be called at each iteration.
*/

brInstance *brEngineAddInstance( brEngine *e, brObject *object, void *pointer ) {
	brInstance *i;

	i = new brInstance( e, object );
	i->userData = pointer;

	return brEngineAddBreveInstance( e, object, i );
}

brInstance *brEngineAddBreveInstance( brEngine *e, brObject *object, brInstance *breveInstance ) {
	brMethod *imethod, *pmethod;

	// it's a bit of a hack, but we need the camera to be informed of
	// new objects in the world.  the code which adds objects to the
	// world doesn't have a pointer to the camera, so we'll do it here

	if ( e->camera ) e->camera->setRecompile();

	// find the iterate method which we will call at each iteration

	imethod = brMethodFind( breveInstance->object, "iterate", NULL, 0 );
	pmethod = brMethodFind( breveInstance->object, "post-iterate", NULL, 0 );

	breveInstance->iterate = imethod;
	breveInstance->postIterate = pmethod;

	e->instancesToAdd.push_back( breveInstance );

	return breveInstance;
}

/**
 * Determines whether one object is a subclass of another
 */
 
bool brObjectIsSubclass( brObject *inA, brObject *inB ) {
	if( inA->type->_typeSignature != inB->type->_typeSignature )
		return false;

	return inA->type->isSubclass( inA->type, inA->userData, inB->userData );
}

/**
 * Creates an instance of the given brObject class.
 *
 * The constructur args and argCount are currently unused.
 */

brInstance *brObjectInstantiate( brEngine *e, brObject *o, const brEval **args, int argCount ) {
	return o->type->instantiate( e, o, args, argCount );
}

/**
 * Marks a \ref brInstance as released, so that it can be removed from the engine.
 *
 * The instance will be removed from the engine and freed at the end of
 * the next simulation iteration.  This function should be used instead of
 * \ref brInstanceFree except during simulation deallocation and cleanup.
 */

void brInstanceRelease( brInstance *i ) {
	if ( !i || i->status != AS_ACTIVE ) return;

	i->engine->instancesToRemove.push_back( i );

	brInstanceFree( i );

	i->status = AS_RELEASED;
}

/**
 * Removes an object from the engine.
 * 
 * The object may still exist in the simulation (technically), but it will 
 * no longer be iterated by the engine.
 */

void brEngineRemoveInstance( brEngine *e, brInstance *i ) {
	// inform the camera of the change

	std::vector<brInstance*>::iterator bi;

	if ( e->camera ) e->camera->setRecompile();

	bi = std::find( e->iterationInstances.begin(), e->iterationInstances.end(), i );

	if ( bi != e->iterationInstances.end() ) e->iterationInstances.erase( bi );

	bi = std::find( e->postIterationInstances.begin(), e->postIterationInstances.end(), i );

	if ( bi != e->postIterationInstances.end() ) e->postIterationInstances.erase( bi );

	bi = std::find( e->instances.begin(), e->instances.end(), i );

	if ( bi != e->instances.end() ) e->instances.erase( bi );
}

/**
 * Frees a brMethod structure.
 * 
 * If you have generated a brMethod structure with \ref brMethodFind,
 * you must free it using this method when you are done with it.
 */

void brMethodFree( brMethod *m ) {
	delete m;
}

/**
 * Frees a breve object.
 */

void brObjectFree( brObject *o ) {
	unsigned int n;

	for ( n = 0; n < o->collisionHandlers.size(); n++ ) {
		brCollisionHandler *h = o->collisionHandlers[ n ];

		if ( h->method ) 
			delete h->method;

		delete h;
	}

	slFree( o->name );
	delete o;
}

/**
 * Destroys a \ref brInstance structure.
 *
 * Immediately frees all of the data associated with a \ref brInstance
 * structure.  This function should not be used while a breve engine
 * is being actively iterated.  It may be used for cleanup when the
 * engine isn't running, but otherwise use \ref brInstanceRelease instead.
 */

void brInstanceFree( brInstance *i ) {
	if ( i && i->userData ) 
		i->object->type->destroyInstance( i->userData );

	std::vector< brObserver* > observerList = i->observers;

	for( unsigned int n = 0; n < observerList.size(); n++ ) {
		delete observerList[ n ];
	}

	i->observers.clear();

	// removing observers will modify the observee list,
	// so copy the list first

	std::vector< brInstance* > observeeList = i->observees;

	for( unsigned int n = 0; n < observeeList.size(); n++ ) {
		brEngineRemoveInstanceObserver( observeeList[ n ], i, NULL );
	}

	for ( unsigned int n = 0;n < i->_menus.size(); n++ ) {
		brMenuEntry *menu = i->_menus[ n ];

		slFree( menu->title );
		slFree( menu->method );

		delete menu;
	}

	if ( i->iterate ) 
		delete i->iterate;

	if ( i->postIterate ) 
		delete i->postIterate;

	i->userData = NULL;

	i->engine->freedInstances.push_back( i );
}

/**
 * Adds a collision handler to an object.
 *
 * Defines a behavior for when object type "handler" collides with "collider".
 * The behavior is to call the specified method in the handler object
 * with the collider instance as an argument.
 */

int brObjectAddCollisionHandler( brObject *handler, brObject *collider, char *name ) {
	brCollisionHandler *ch;
	bool nomethods = true;

	const int maxargs = 5;

	brMethod* method[maxargs+1];

	unsigned int n;

	unsigned char types[] = { AT_INSTANCE, AT_DOUBLE };

	for ( int i = 0; i <= maxargs; i++ ) method[i] = NULL;

	// Dont add a second collisionHandler for the same collider?
	for ( n = 0;n < handler->collisionHandlers.size();n++ ) {
		ch = handler->collisionHandlers[ n ];

		if ( ch->object == collider ) return EC_STOP;
	}

	// searching for methods
	for ( int i = 0; i <= maxargs; i++ ) {
		method[i] = brMethodFindWithArgRange( handler, name, types, i, i );

		if ( method[i] ) nomethods = false;
	}


	if ( nomethods ) {
		slMessage( DEBUG_ALL, "Error adding collision handler: cannot locate method \"%s\" for class \"%s\"\n", name, handler->name );
		return EC_ERROR;
	}

	//Adding all methods
	for ( int i = 0; i <= maxargs; i++ ) {
		if ( method[i] ) {
			ch = new brCollisionHandler;
			ch->object = collider;
			ch->method = method[i];
			ch->ignore = 0;
			handler->collisionHandlers.push_back( ch );
		}
	}

	return EC_OK;
}

int brObjectSetIgnoreCollisionsWith( brObject *handler, brObject *collider, int ignore ) {
	brCollisionHandler *ch;
	unsigned int n;

	for ( n = 0;n < handler->collisionHandlers.size() ;n++ ) {
		ch = handler->collisionHandlers[ n ];

		if ( ch->object == collider ) {
			ch->ignore = ignore;
			return EC_OK;
		}
	}

	ch = new brCollisionHandler;

	ch->object = collider;
	ch->method = NULL;
	ch->ignore = ignore;

	handler->collisionHandlers.push_back( ch );

	return EC_OK;
}

/*@}*/
