#include "kernel.h"
#include "pushC.h"

struct brPushCallbackData {
	brMethod *method;
	brInstance *instance;
};

typedef struct brPushCallbackData brPushCallbackData;

unsigned int brPushCallbackFunction(void *environment, brPushCallbackData *data);
void brPushFreeData(void *d);

/*@{*/
/*! \addtogroup InternalFunctions */

#ifdef HAVE_LIBPUSH
int breveFunctionPushCallbackNew(brEval arguments[], brEval *result, brInstance *instance) {
	void *environment = BRPOINTER(&arguments[0]);
	char *name = BRSTRING(&arguments[1]);
	char *methodName = BRSTRING(&arguments[2]);
	brInstance *callbackInstance = BRINSTANCE(&arguments[3]);

	brPushCallbackData *data;

	data = slMalloc(sizeof(brPushCallbackData));
	data->instance = callbackInstance;
	data->method = brMethodFind(callbackInstance->class, methodName, 0);

	if(!data->method) {
		slMessage(DEBUG_ALL, "Cannot locate method \"%s\" for class \"%s\" for push callback instruction\n", methodName, callbackInstance->class->name);
		return EC_ERROR;
	}

	BRPOINTER(result) = pushAddCallbackInstruction(environment, brPushCallbackFunction, brPushFreeData, name, data);

	return EC_OK;
}
#endif */ HAVE_LIBPUSH */

/*@}*/

void breveInitPushCallbackFunctions(brNamespace *namespace) {
#ifdef HAVE_LIBPUSH
	brNewBreveCall(namespace, "pushCallbackNew", breveFunctionPushCallbackNew, AT_POINTER, AT_POINTER, AT_STRING, AT_STRING, AT_INSTANCE, 0);
#endif
}

unsigned int brPushCallbackFunction(void *environment, brPushCallbackData *data) {
	brEval eval;

	if(brMethodCall(data->instance, data->method, NULL, &eval) != EC_OK) return -1;

	return 0;
}

void brPushFreeData(void *d) {
	brPushCallbackData *data = d;

	brMethodFree(data->method);
	slFree(data);
}
