enum allocStatus { 
	AS_DEARCHIVING = -2,        // allocated, but not ready for use
	AS_RELEASED,                // still allocated, but ready to be freed
	AS_FREED,                   // all memory freed--don't try to use 
	AS_ACTIVE                   // active object
};

typedef struct brObjectType brObjectType;

/*! \addtogroup breveObjectAPI */
/*@{*/

/*!
	\brief A type of object, required for each frontend language.

	brObjectType is an interface that allows breve to preform common tasks
	for a variety of fronten: languages.  It contains function pointers 
	that allow breve to create and destroy instances, call methods, etc.

	If you create a new frontend, you'll need to create a brObjectType and
	provide it with functions to do the various tasks.
*/

struct brObjectType {
	void *(*findMethod)(brObject *object, char *name, unsigned char *types, int nargs);
	void *(*findObject)(brObjectType *type, char *name);
	void *(*instantiate)(brObject *class, brEval **constructorArgs, int argCount);

	int (*callMethod)(brInstance *instance, brMethod *method, brEval **arguments, brEval *result);

	int (*isSubclass)(brObject *class1, brObject *class2);

	void (*destroyObject)(brObject *object);
	void (*destoryMethod)(brMethod *method);
	void (*destroyInstance)(brInstance *instance);
	void (*destroyObjectType)(brObjectType *objectType);

	void *data;
};

/*!
	\brief An object class.

	An object class, of any language.
*/

struct brObject {
	void *pointer;

	brObjectType *type;

	char *name;

    slStack *collisionHandlers;
};

/*!
	\brief A breve instance, of any language.
	
	brInstance is an object instance in the breve engine, though the 
	object could be part of any language, steve, java, SmallTalk, etc.

	Note: the "object" field was previously named "class".  This has been
	changed to avoid conflicts with C++ compilers.
*/

struct brInstance {
	void *pointer;

	char status;

	brMethod *iterate;
	brMethod *postIterate;

	brObject *object;
	brEngine *engine;
	brMenuList menu;

	slList *observers;
	slList *observees;

	slList *dependencies;
	slList *dependents;

	slList *events;
};

/*!
	\brief A collision handler.

	Keeps track of what methods are used to handle collisions between objects.
*/

struct brCollisionHandler {
    brObject *object;
    brMethod *method;
};

/*!
	\brief A reference to a native method.

	A reference to a method of any language.

	Note: the "object" field was previously named "class".  This has been
	changed to avoid conflicts with C++ compilers.
*/

struct brMethod {
	void *pointer;

	char *name;
	brObject *object;
	int argumentCount;
};

/*!
	\brief An observation.

	Contains information about one object watching another, waiting for 
	a certain announcement.
*/

struct brObserver {
    brInstance *instance;
    brMethod *method;
    char *notification;
};

/*@}*/

// registering a new object type

void brEngineRegisterObjectType(brEngine *e, brObjectType *t);

// locating objects and methods within objects

brMethod *brMethodFind(brObject *type, char *name, 
	unsigned char *types, int argCount);
brMethod *brMethodFindWithArgRange(brObject *o, char *name, 
	unsigned char *types, int min, int max);

brObject *brObjectFind(brEngine *n, char *name);
brObject *brUnknownObjectFind(brEngine *e, char *name);

// functions for calling methods with breve instances

int brMethodCall(brInstance *i, brMethod *m, brEval **args, brEval *result);
int brMethodCallByName(brInstance *i, char *name, brEval *result);
int brMethodCallByNameWithArgs(brInstance *i, char *name, brEval **args, 
	int count, brEval *result);

// functions related to adding and removing classes and instances to the breve engine

brObject *brEngineAddObject(brEngine *e, brObjectType *t, char *name, void *pointer);
void brEngineAddObjectAlias(brEngine *e, char *name, brObject *o);
brInstance *brEngineAddInstance(brEngine *e, brObject *o, void *pointer);
brInstance *brObjectInstantiate(brEngine *e, brObject *o, brEval **args, int argCount);
void brEngineRemoveInstance(brEngine *e, brInstance *i);

int brObjectAddCollisionHandler(brObject *handler, brObject *collider, char *name);

// adding and removing dependencies and observers 

int brInstanceAddDependency(brInstance *i, brInstance *dependency);
int brInstanceAddObserver(brInstance *i, brInstance *observer, char *notification, char *mname);
int brEngineRemoveInstanceDependency(brInstance *i, brInstance *dependency);
void brEngineRemoveInstanceObserver(brInstance *i, brInstance *observerInstance, char *notification);

// cleaning up 

void brInstanceRelease(brInstance *i);

void brObjectFree(brObject *o);
void brInstanceFree(brInstance *i);
void brMethodFree(brMethod *i);


