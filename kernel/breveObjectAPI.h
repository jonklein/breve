enum allocStatus { 
	AS_DEARCHIVING = -2,        // allocated, but not ready for use
	AS_RELEASED,                // still allocated, but ready to be freed
	AS_FREED,                   // all memory freed--don't try to use 
	AS_ACTIVE                   // active object
};

typedef struct brObjectType brObjectType;

/*! \addtogroup API */
/*@{*/

/*!
	\brief A type of object, required for each frontend language.

	brObjectType is an interface that allows breve to preform common tasks
	for a variety of frontent languages.  It contains function pointers 
	that allow breve to create and destroy instances, call methods, etc.

	If you create a new frontend, you'll need to create a brObjectType and
	provide it with functions to do the various tasks.
*/

struct brObjectType {
	int (*callMethod)(brInstance *instancePointer, brMethod *method, brEval **arguments, brEval *result);
	void *(*findMethod)(brObject *objectPointer, char *name, int nargs);
	int (*isSubclass)(brObject *class1, brObject *class2);

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
*/

struct brInstance {
	void *pointer;

	char status;

	brMethod *iterate;
	brMethod *postIterate;

	brObject *class;
	brEngine *engine;
	brMenuList menu;

	slList *observers;
	slList *observees;

	slList *dependencies;
	slList *dependents;

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
*/

struct brMethod {
	void *pointer;

	char *name;
	brObject *class;
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

// locating objects and methods within objects

brMethod *brMethodFind(brObject *type, char *name, int argCount);
brMethod *brMethodFindWithArgRange(brObject *o, char *name, int min, int max);
brObject *brObjectFind(brEngine *n, char *name);

// functions for calling methods with breve instances

int brMethodCall(brInstance *i, brMethod *m, brEval **args, brEval *result);
int brMethodCallByName(brInstance *i, char *name, brEval *result);
int brMethodCallByNameWithArgs(brInstance *i, char *name, brEval **args, int count, brEval *result);

// functions related to adding classes and instances to the breve engine

brObject *brAddObjectToEngine(brEngine *e, brObjectType *t, char *name, void *pointer);
void brAddObjectAlias(brEngine *e, char *name, brObject *o);
brInstance *brAddInstanceToEngine(brEngine *e, brObject *o, void *pointer);

int brAddCollisionHandler(brObject *handler, brObject *collider, brMethod *m);

// adding and removing dependencies and observers 

int brAddDependency(brInstance *i, brInstance *dependency);
int brAddObserver(brInstance *i, brInstance *observer, char *notification, char *mname);
int brRemoveDependency(brInstance *i, brInstance *dependency);
void brRemoveObserver(brInstance *i, brInstance *observerInstance, char *notification);

// cleaning up 

void brInstanceRelease(brInstance *i);

void brObjectFree(brObject *o);
void brInstanceFree(brInstance *i);
void brMethodFree(brMethod *i);


