#include "kernel.h"
#include "pushC.h"

struct brPushCallbackData {
	brMethod *method;
	brInstance *instance;
};

typedef struct brPushCallbackData brPushCallbackData;

unsigned int brPushCallbackFunction(void *environment, brPushCallbackData *data);

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
	data->method = brMethodFind(callbackInstance->class, methodName, 0);

	if(!data->method) {
		slMessage(DEBUG_ALL, "Cannot locate method \"%s\" for class \"%s\" for push callback instruction\n", methodName, callbackInstance->class->name);
		return EC_ERROR;
	}

	pushAddCallbackInstruction(environment, brPushCallbackFunction, name, data);

	BRPOINTER(result) = data;

	return EC_OK;
}

int breveFunctionPushCallbackFree(brEval arguments[], brEval *result, brInstance *instance) {
	brPushCallbackData *data = BRPOINTER(&arguments[0]);

	slFree(data);
	brMethodFree(data->method);

	return EC_OK;
}

/*@}*/

void breveInitPushCallbackFunctions(brNamespace *namespace) {
	brNewBreveCall(namespace, "pushCallbackNew", breveFunctionPushCallbackNew, AT_POINTER, AT_POINTER, AT_STRING, AT_STRING, AT_INSTANCE, 0);
	brNewBreveCall(namespace, "pushCallbackFree", breveFunctionPushCallbackFree, AT_NULL, AT_POINTER, 0);
}

unsigned int brPushCallbackFunction(void *environment, brPushCallbackData *data) {
	brEval eval;

	if(brMethodCall(data->instance, data->method, NULL, &eval) != EC_OK) return -1;

	return 0;
}
