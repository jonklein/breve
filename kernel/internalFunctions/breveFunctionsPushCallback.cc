#include "kernel.h"

#if HAVE_LIBPUSH
#include "pushC.h"

struct brPushCallbackData {
	brMethod *method;
	brInstance *instance;
};

typedef struct brPushCallbackData brPushCallbackData;

static int breveFunctionPushMacroNew( brEval *, brEval *, brInstance * );

static unsigned int brPushCallbackFunction( PushEnvironment *, void * );

static void brPushFreeData( void * );

/*@{*/
/*! \addtogroup InternalFunctions */

int breveFunctionPushCallbackNew( brEval arguments[], brEval *result, brInstance *instance ) {
	void *environment = BRPOINTER( &arguments[0] );
	char *name = BRSTRING( &arguments[1] );
	char *methodName = BRSTRING( &arguments[2] );
	brInstance *callbackInstance = BRINSTANCE( &arguments[3] );

	brPushCallbackData *data;

	data = new brPushCallbackData;
	data->instance = callbackInstance;
	data->method = brMethodFind( callbackInstance->object, methodName, NULL, 0 );

	if ( !data->method ) {
		slMessage( DEBUG_ALL, "Cannot locate method \"%s\" for class \"%s\" for push callback instruction\n", methodName, callbackInstance->object->name );
		return EC_ERROR;
	}

	result->set( pushAddCallbackInstruction( environment, brPushCallbackFunction, brPushFreeData, name, data ) );

	return EC_OK;
}

int breveFunctionPushMacroNew( brEval arguments[], brEval *result, brInstance *instance ) {

	void *environment = BRPOINTER( &arguments[0] );
	char *name = BRSTRING( &arguments[1] );
	PushCode *code = BRPOINTER( &arguments[2] );

	pushAddMacroInstruction( environment, name, code );

	return EC_OK;
}

unsigned int brPushCallbackFunction( PushEnvironment *environment, void *d ) {
	brEval eval;
	brPushCallbackData *data = ( brPushCallbackData * )d;

	( void )brMethodCall( data->instance, data->method, NULL, &eval );

	return 0;
}

void brPushFreeData( void *d ) {
	brPushCallbackData *data = ( brPushCallbackData * )d;

	brMethodFree( data->method );
	delete data;
}

#endif

/*@}*/

void breveInitPushCallbackFunctions( brNamespace *names ) {
#if HAVE_LIBPUSH
	brNewBreveCall( names, "pushCallbackNew", breveFunctionPushCallbackNew, AT_POINTER, AT_POINTER, AT_STRING, AT_STRING, AT_INSTANCE, 0 );
	brNewBreveCall( names, "pushMacroNew", breveFunctionPushMacroNew, AT_NULL, AT_POINTER, AT_STRING, AT_POINTER, 0 );
#endif
}
