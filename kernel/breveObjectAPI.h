#ifndef _BREVEOBJECTAPI_H
#define _BREVEOBJECTAPI_H

enum allocStatus { 
	AS_DEARCHIVING = -2,        // allocated, but not ready for use
	AS_RELEASED,                // still allocated, but ready to be freed
	AS_FREED,                   // all memory freed--don't try to use 
	AS_ACTIVE                   // active object
};

typedef struct brObjectType brObjectType;
typedef struct brMethod brMethod;
typedef struct brCollisionHandler brCollisionHandler;

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
	void *(*findMethod)(void *objectData, char *name, unsigned char *types, int nargs);
	void *(*findObject)(void *typeData, char *name);
	void *(*instantiate)(void *objectData, brEval **constructorArgs, int argCount);

	int (*callMethod)(void *instanceData, void *methodData, brEval **arguments, brEval *result);

	int (*isSubclass)(void *classData1, void *classData2);

	void (*destroyObject)(void *objectData);
	void (*destoryMethod)(void *methodData);
	void (*destroyInstance)(void *instanceData);
	void (*destroyObjectType)(void *objectTypeData);

	void *userData;
};

/*!
	\brief An object class.

	An object class, of any language.
*/

struct brObject {
	void *userData;

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
	void *userData;

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
	unsigned char ignore;
};

/*!
	\brief A reference to a native method.

	A reference to a method of any language.

	Note: the "object" field was previously named "class".  This has been
	changed to avoid conflicts with C++ compilers.
*/

struct brMethod {
	void *userData;

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

#ifdef __cplusplus
extern "C" {
#endif

// registering a new object type

void brEngineRegisterObjectType(brEngine *e, brObjectType *t);

// locating objects and methods within objects

brMethod *brMethodFind(brObject *type, char *name, 
	unsigned char *types, int argCount);
brMethod *brMethodFindWithArgRange(brObject *o, char *name, 
	unsigned char *types, int min, int max);

brObject *brObjectFind(brEngine *n, char *name);
brObject *brUnknownObjectFind(brEngine *e, char *name);

// functions for getting user data

void *brInstanceGetUserData(brInstance *i);
void *brObjectGetUserData(brInstance *i);

// functions for calling methods with breve instances

int brMethodCall(brInstance *i, brMethod *m, brEval **args, brEval *result);
int brMethodCallByName(brInstance *i, char *name, brEval *result);
int brMethodCallByNameWithArgs(brInstance *i, char *name, brEval **args, 
	int count, brEval *result);

// functions related to adding and removing classes and instances to the breve engine

brObject *brEngineAddObject(brEngine *e, brObjectType *t, char *name, void *userData);
void brEngineAddObjectAlias(brEngine *e, char *name, brObject *o);
brInstance *brEngineAddInstance(brEngine *e, brObject *o, void *userData);
brInstance *brObjectInstantiate(brEngine *e, brObject *o, brEval **args, int argCount);
void brEngineRemoveInstance(brEngine *e, brInstance *i);

int brObjectAddCollisionHandler(brObject *handler, brObject *collider, char *name);
int brObjectSetIgnoreCollisionsWith(brObject *handler, brObject *collider, int ignore);

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

#ifdef __cplusplus
}
#endif

#endif
