#include "kernel.h"

#ifdef HAVE_LIBPUSH
#include "pushC.h"

struct brPushCallbackData {
	brMethod *method;
	brInstance *instance;
};

typedef struct brPushCallbackData brPushCallbackData;

unsigned int brPushCallbackFunction(PushEnvironment *environment, void *data);
void brPushFreeData(void *d);

/*@{*/
/*! \addtogroup InternalFunctions */

int breveFunctionPushCallbackNew(brEval arguments[], brEval *result, brInstance *instance) {
	void *environment = BRPOINTER(&arguments[0]);
	char *name = BRSTRING(&arguments[1]);
	char *methodName = BRSTRING(&arguments[2]);
	brInstance *callbackInstance = BRINSTANCE(&arguments[3]);

	brPushCallbackData *data;

	data = slMalloc(sizeof(brPushCallbackData));
	data->instance = callbackInstance;
	data->method = brMethodFind(callbackInstance->object, methodName, NULL, 0);

	if(!data->method) {
		slMessage(DEBUG_ALL, "Cannot locate method \"%s\" for class \"%s\" for push callback instruction\n", methodName, callbackInstance->object->name);
		return EC_ERROR;
	}

	BRPOINTER(result) = pushAddCallbackInstruction(environment, brPushCallbackFunction, brPushFreeData, name, data);

	return EC_OK;
}

unsigned int brPushCallbackFunction(PushEnvironment *environment, void *d) {
	brEval eval;
	brPushCallbackData *data = (brPushCallbackData*)d;

	if(brMethodCall(data->instance, data->method, NULL, &eval) != EC_OK) return 0;

	return 0;
}

void brPushFreeData(void *d) {
	brPushCallbackData *data = d;

	brMethodFree(data->method);
	slFree(data);
}
#endif /* HAVE_LIBPUSH */

/*@}*/

void breveInitPushCallbackFunctions(brNamespace *names) {
#ifdef HAVE_LIBPUSH
	brNewBreveCall(names, "pushCallbackNew", breveFunctionPushCallbackNew, AT_POINTER, AT_POINTER, AT_STRING, AT_STRING, AT_INSTANCE, 0);
#endif
}
