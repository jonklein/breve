#ifndef _BREVEOBJECTAPI_H
#define _BREVEOBJECTAPI_H

#include <vector>

enum allocStatus { 
	AS_RELEASED = -1,	// still allocated, but ready to be freed
	AS_FREED,		// all memory freed--don't try to use 
	AS_ACTIVE		// active object
};

typedef struct brObjectType brObjectType;
typedef struct brMethod brMethod;
typedef struct brCollisionHandler brCollisionHandler;

class brEngine;
class brEval;
struct slList;
struct slStack;

/*! \addtogroup breveObjectAPI */
/*@{*/

/*!
	\brief A type of object, required for each frontend language.

	brObjectType is an interface that allows breve to preform common tasks
	for a variety of frontend languages.  It contains function pointers 
	that allow breve to create and destroy instances, call methods, etc.

	If you create a new frontend, you'll need to create a brObjectType and
	provide it with functions to do the various tasks.
*/

struct brObjectType {
	brObjectType() {
		findMethod 				= NULL;
		findObject				= NULL;
		instantiate				= NULL;
		callMethod				= NULL;
		isSubclass				= NULL;
		destroyObject			= NULL;
		destroyMethod			= NULL;
		destroyInstance 		= NULL;
		destroyObjectType		= NULL;
		userData				= NULL;
		_typeSignature 			= 0;
	}

	void 			*(*findMethod)( void *, const char *, unsigned char *, int );
	void 			*(*findObject)( void *, const char * );

	brInstance		*(*instantiate)( brEngine *, brObject *, const brEval **, int );

	int 			(*callMethod)( void *, void *, const brEval **, brEval * );

	int 			(*isSubclass)( void *, void * );

	void 			(*destroyObject)( void * );
	void 			(*destroyMethod)( void * );
	void 			(*destroyInstance)( void * );
	void 			(*destroyObjectType)( void * );

	void 			*userData;

	long			_typeSignature;
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
	brInstance( brEngine *inEngine, brObject *inObject ) {
		engine = inEngine;
		object = inObject;

		status = AS_ACTIVE; 
		
		iterate = NULL;
		postIterate = NULL;
		observers = NULL;
		observees = NULL;
	}

	void *userData;

	char status;

	brMethod *iterate;
	brMethod *postIterate;

	brObject *object;
	brEngine *engine;

	std::vector< brMenuEntry* > _menus;

	slList *observers;
	slList *observees;
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
	~brMethod() {
		if( name )
			slFree( name );

		if( object && object->type->destroyMethod && userData )
			object->type->destroyMethod( userData );
	}

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
	brObserver( brInstance *obs, brMethod *meth, char *note ) {
		instance = obs;
		method = meth;
		notification = slStrdup( note );
	}

	~brObserver() {
		delete method;
		free( notification );
	}

    brInstance *instance;
    brMethod *method;
    char *notification;
};

/*@}*/

#ifdef __cplusplus
extern "C" {
#endif

DLLEXPORT void brEngineLock( brEngine * );
DLLEXPORT void brEngineUnlock( brEngine * );

// registering a new object type

DLLEXPORT void brEngineRegisterObjectType(brEngine *, brObjectType * );

// locating objects and methods within objects

DLLEXPORT brMethod *brMethodFind(brObject *, const char *, unsigned char *, int);
DLLEXPORT brMethod *brMethodFindWithArgRange(brObject *, const char *, unsigned char *, int, int);

DLLEXPORT brObject *brObjectFind(brEngine *, const char *);
DLLEXPORT brObject *brUnknownObjectFind(brEngine *, const char *);

// functions for getting user data

DLLEXPORT void *brInstanceGetUserData(brInstance *);
DLLEXPORT void *brObjectGetUserData(brInstance *);

// functions for calling methods with breve instances

DLLEXPORT int brMethodCall(brInstance *, brMethod *, const brEval **, brEval *);
DLLEXPORT int brMethodCallByName(brInstance *, const char *, brEval *);
DLLEXPORT int brMethodCallByNameWithArgs(brInstance *, const char *, const brEval **, int, brEval *);

// functions related to adding and removing classes and instances to the breve engine

DLLEXPORT brObject *brEngineAddObject(brEngine *, brObjectType *, const char *, void *);
DLLEXPORT void brEngineAddObjectAlias(brEngine *, char *, brObject *);

DLLEXPORT brInstance *brObjectInstantiate(brEngine *, brObject *, const brEval **, int);

DLLEXPORT brInstance *brEngineAddInstance(brEngine *, brObject *, void *);
DLLEXPORT brInstance *brEngineAddBreveInstance(brEngine *, brObject *, brInstance * );

DLLEXPORT void brEngineRemoveInstance(brEngine *, brInstance *);

DLLEXPORT int brObjectAddCollisionHandler(brObject *, brObject *, char *);
DLLEXPORT int brObjectSetIgnoreCollisionsWith(brObject *, brObject *, int);

// adding and removing dependencies and observers 

DLLEXPORT int brInstanceAddDependency(brInstance *i, brInstance *dependency);
DLLEXPORT int brInstanceAddObserver(brInstance *i, brInstance *observer, char *notification, char *mname);
DLLEXPORT int brEngineRemoveInstanceDependency(brInstance *i, brInstance *dependency);
DLLEXPORT void brEngineRemoveInstanceObserver(brInstance *i, brInstance *observerInstance, char *notification);

// cleaning up 

DLLEXPORT void brInstanceRelease(brInstance *i);

DLLEXPORT void brObjectFree(brObject *o);
DLLEXPORT void brInstanceFree(brInstance *i);
DLLEXPORT void brMethodFree(brMethod *i);

#ifdef __cplusplus
}
#endif

#endif
