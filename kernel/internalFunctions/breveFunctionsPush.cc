/*
	the breve simulation environment

	This file was generated automatically by the script /Users/jk/dev/breve/tools/apiwrap.pl.
*/

#include "kernel.h"
#include "/usr/local/include/pushC.h"

/*@{*/
/*! \addtogroup InternalFunctions */
/*!
	\brief A breve API function wrapper for the C-function \ref pushEnvironmentNew.

	See the documentation for \ref pushEnvironmentNew for more details.
*/

int breveFunctionPushEnvironmentNew(brEval arguments[], brEval *result, brInstance *instance) {

	BRPOINTER(result) = pushEnvironmentNew();

	return EC_OK;
}

/*!
	\brief A breve API function wrapper for the C-function \ref pushEnvironmentFree.

	See the documentation for \ref pushEnvironmentFree for more details.
*/

int breveFunctionPushEnvironmentFree(brEval arguments[], brEval *result, brInstance *instance) {
	PushEnvironment *environment = BRPOINTER(&arguments[0]);

	pushEnvironmentFree(environment);

	return EC_OK;
}

/*!
	\brief A breve API function wrapper for the C-function \ref pushEnvironmentReadConfigFile.

	See the documentation for \ref pushEnvironmentReadConfigFile for more details.
*/

int breveFunctionPushEnvironmentReadConfigFile(brEval arguments[], brEval *result, brInstance *instance) {
	PushEnvironment *environment = BRPOINTER(&arguments[0]);
	char *file = BRSTRING(&arguments[1]);

	pushEnvironmentReadConfigFile(environment, file);

	return EC_OK;
}

/*!
	\brief A breve API function wrapper for the C-function \ref pushParse.

	See the documentation for \ref pushParse for more details.
*/

int breveFunctionPushParse(brEval arguments[], brEval *result, brInstance *instance) {
	char *string = BRSTRING(&arguments[0]);

	BRPOINTER(result) = pushParse(string);

	return EC_OK;
}

/*!
	\brief A breve API function wrapper for the C-function \ref pushRun.

	See the documentation for \ref pushRun for more details.
*/

int breveFunctionPushRun(brEval arguments[], brEval *result, brInstance *instance) {
	PushEnvironment *environment = BRPOINTER(&arguments[0]);
	PushCode *code = BRPOINTER(&arguments[1]);

	BRINT(result) = pushRun(environment, code);

	return EC_OK;
}

/*!
	\brief A breve API function wrapper for the C-function \ref pushGetConfigString.

	See the documentation for \ref pushGetConfigString for more details.
*/

int breveFunctionPushGetConfigString(brEval arguments[], brEval *result, brInstance *instance) {
	PushEnvironment *environment = BRPOINTER(&arguments[0]);

	BRSTRING(result) = slStrdup(pushGetConfigString(environment));

	return EC_OK;
}

/*!
	\brief A breve API function wrapper for the C-function \ref pushGetStacksString.

	See the documentation for \ref pushGetStacksString for more details.
*/

int breveFunctionPushGetStacksString(brEval arguments[], brEval *result, brInstance *instance) {
	PushEnvironment *environment = BRPOINTER(&arguments[0]);

	BRSTRING(result) = slStrdup(pushGetStacksString(environment));

	return EC_OK;
}

/*!
	\brief A breve API function wrapper for the C-function \ref pushClearStacks.

	See the documentation for \ref pushClearStacks for more details.
*/

int breveFunctionPushClearStacks(brEval arguments[], brEval *result, brInstance *instance) {
	PushEnvironment *environment = BRPOINTER(&arguments[0]);

	pushClearStacks(environment);

	return EC_OK;
}

/*!
	\brief A breve API function wrapper for the C-function \ref pushCodeGetString.

	See the documentation for \ref pushCodeGetString for more details.
*/

int breveFunctionPushCodeGetString(brEval arguments[], brEval *result, brInstance *instance) {
	PushCode *code = BRPOINTER(&arguments[0]);

	BRSTRING(result) = slStrdup(pushCodeGetString(code));

	return EC_OK;
}

/*!
	\brief A breve API function wrapper for the C-function \ref pushCodeFree.

	See the documentation for \ref pushCodeFree for more details.
*/

int breveFunctionPushCodeFree(brEval arguments[], brEval *result, brInstance *instance) {
	PushCode *code = BRPOINTER(&arguments[0]);

	pushCodeFree(code);

	return EC_OK;
}

/*!
	\brief A breve API function wrapper for the C-function \ref pushIntStackSize.

	See the documentation for \ref pushIntStackSize for more details.
*/

int breveFunctionPushIntStackSize(brEval arguments[], brEval *result, brInstance *instance) {
	PushEnvironment *environment = BRPOINTER(&arguments[0]);

	BRINT(result) = pushIntStackSize(environment);

	return EC_OK;
}

/*!
	\brief A breve API function wrapper for the C-function \ref pushIntStackPop.

	See the documentation for \ref pushIntStackPop for more details.
*/

int breveFunctionPushIntStackPop(brEval arguments[], brEval *result, brInstance *instance) {
	PushEnvironment *environment = BRPOINTER(&arguments[0]);

	pushIntStackPop(environment);

	return EC_OK;
}

/*!
	\brief A breve API function wrapper for the C-function \ref pushIntStackTop.

	See the documentation for \ref pushIntStackTop for more details.
*/

int breveFunctionPushIntStackTop(brEval arguments[], brEval *result, brInstance *instance) {
	PushEnvironment *environment = BRPOINTER(&arguments[0]);

	BRINT(result) = pushIntStackTop(environment);

	return EC_OK;
}

/*!
	\brief A breve API function wrapper for the C-function \ref pushIntStackPush.

	See the documentation for \ref pushIntStackPush for more details.
*/

int breveFunctionPushIntStackPush(brEval arguments[], brEval *result, brInstance *instance) {
	PushEnvironment *environment = BRPOINTER(&arguments[0]);
	int value = BRINT(&arguments[1]);

	pushIntStackPush(environment, value);

	return EC_OK;
}

/*!
	\brief A breve API function wrapper for the C-function \ref pushBoolStackSize.

	See the documentation for \ref pushBoolStackSize for more details.
*/

int breveFunctionPushBoolStackSize(brEval arguments[], brEval *result, brInstance *instance) {
	PushEnvironment *environment = BRPOINTER(&arguments[0]);

	BRINT(result) = pushBoolStackSize(environment);

	return EC_OK;
}

/*!
	\brief A breve API function wrapper for the C-function \ref pushBoolStackPop.

	See the documentation for \ref pushBoolStackPop for more details.
*/

int breveFunctionPushBoolStackPop(brEval arguments[], brEval *result, brInstance *instance) {
	PushEnvironment *environment = BRPOINTER(&arguments[0]);

	pushBoolStackPop(environment);

	return EC_OK;
}

/*!
	\brief A breve API function wrapper for the C-function \ref pushBoolStackTop.

	See the documentation for \ref pushBoolStackTop for more details.
*/

int breveFunctionPushBoolStackTop(brEval arguments[], brEval *result, brInstance *instance) {
	PushEnvironment *environment = BRPOINTER(&arguments[0]);

	BRINT(result) = pushBoolStackTop(environment);

	return EC_OK;
}

/*!
	\brief A breve API function wrapper for the C-function \ref pushBoolStackPush.

	See the documentation for \ref pushBoolStackPush for more details.
*/

int breveFunctionPushBoolStackPush(brEval arguments[], brEval *result, brInstance *instance) {
	PushEnvironment *environment = BRPOINTER(&arguments[0]);
	int value = BRINT(&arguments[1]);

	pushBoolStackPush(environment, value);

	return EC_OK;
}

/*!
	\brief A breve API function wrapper for the C-function \ref pushFloatStackSize.

	See the documentation for \ref pushFloatStackSize for more details.
*/

int breveFunctionPushFloatStackSize(brEval arguments[], brEval *result, brInstance *instance) {
	PushEnvironment *environment = BRPOINTER(&arguments[0]);

	BRINT(result) = pushFloatStackSize(environment);

	return EC_OK;
}

/*!
	\brief A breve API function wrapper for the C-function \ref pushFloatStackPop.

	See the documentation for \ref pushFloatStackPop for more details.
*/

int breveFunctionPushFloatStackPop(brEval arguments[], brEval *result, brInstance *instance) {
	PushEnvironment *environment = BRPOINTER(&arguments[0]);

	pushFloatStackPop(environment);

	return EC_OK;
}

/*!
	\brief A breve API function wrapper for the C-function \ref pushFloatStackTop.

	See the documentation for \ref pushFloatStackTop for more details.
*/

int breveFunctionPushFloatStackTop(brEval arguments[], brEval *result, brInstance *instance) {
	PushEnvironment *environment = BRPOINTER(&arguments[0]);

	BRDOUBLE(result) = pushFloatStackTop(environment);

	return EC_OK;
}

/*!
	\brief A breve API function wrapper for the C-function \ref pushFloatStackPush.

	See the documentation for \ref pushFloatStackPush for more details.
*/

int breveFunctionPushFloatStackPush(brEval arguments[], brEval *result, brInstance *instance) {
	PushEnvironment *environment = BRPOINTER(&arguments[0]);
	double value = BRDOUBLE(&arguments[1]);

	pushFloatStackPush(environment, value);

	return EC_OK;
}

/*!
	\brief A breve API function wrapper for the C-function \ref pushNameStackSize.

	See the documentation for \ref pushNameStackSize for more details.
*/

int breveFunctionPushNameStackSize(brEval arguments[], brEval *result, brInstance *instance) {
	PushEnvironment *environment = BRPOINTER(&arguments[0]);

	BRINT(result) = pushNameStackSize(environment);

	return EC_OK;
}

/*!
	\brief A breve API function wrapper for the C-function \ref pushNameStackPop.

	See the documentation for \ref pushNameStackPop for more details.
*/

int breveFunctionPushNameStackPop(brEval arguments[], brEval *result, brInstance *instance) {
	PushEnvironment *environment = BRPOINTER(&arguments[0]);

	pushNameStackPop(environment);

	return EC_OK;
}

/*!
	\brief A breve API function wrapper for the C-function \ref pushNameStackTop.

	See the documentation for \ref pushNameStackTop for more details.
*/

int breveFunctionPushNameStackTop(brEval arguments[], brEval *result, brInstance *instance) {
	PushEnvironment *environment = BRPOINTER(&arguments[0]);

	BRINT(result) = pushNameStackTop(environment);

	return EC_OK;
}

/*!
	\brief A breve API function wrapper for the C-function \ref pushNameStackPush.

	See the documentation for \ref pushNameStackPush for more details.
*/

int breveFunctionPushNameStackPush(brEval arguments[], brEval *result, brInstance *instance) {
	PushEnvironment *environment = BRPOINTER(&arguments[0]);
	int value = BRINT(&arguments[1]);

	pushNameStackPush(environment, value);

	return EC_OK;
}

/*!
	\brief A breve API function wrapper for the C-function \ref pushCodeStackSize.

	See the documentation for \ref pushCodeStackSize for more details.
*/

int breveFunctionPushCodeStackSize(brEval arguments[], brEval *result, brInstance *instance) {
	PushEnvironment *environment = BRPOINTER(&arguments[0]);

	BRINT(result) = pushCodeStackSize(environment);

	return EC_OK;
}

/*!
	\brief A breve API function wrapper for the C-function \ref pushCodeStackPop.

	See the documentation for \ref pushCodeStackPop for more details.
*/

int breveFunctionPushCodeStackPop(brEval arguments[], brEval *result, brInstance *instance) {
	PushEnvironment *environment = BRPOINTER(&arguments[0]);

	pushCodeStackPop(environment);

	return EC_OK;
}

/*!
	\brief A breve API function wrapper for the C-function \ref pushCodeStackTop.

	See the documentation for \ref pushCodeStackTop for more details.
*/

int breveFunctionPushCodeStackTop(brEval arguments[], brEval *result, brInstance *instance) {
	PushEnvironment *environment = BRPOINTER(&arguments[0]);

	BRPOINTER(result) = pushCodeStackTop(environment);

	return EC_OK;
}

/*!
	\brief A breve API function wrapper for the C-function \ref pushCodeStackPush.

	See the documentation for \ref pushCodeStackPush for more details.
*/

int breveFunctionPushCodeStackPush(brEval arguments[], brEval *result, brInstance *instance) {
	PushEnvironment *environment = BRPOINTER(&arguments[0]);
	PushCode *value = BRPOINTER(&arguments[1]);

	pushCodeStackPush(environment, value);

	return EC_OK;
}

/*!
	\brief A breve API function wrapper for the C-function \ref pushVectorStackSize.

	See the documentation for \ref pushVectorStackSize for more details.
*/

int breveFunctionPushVectorStackSize(brEval arguments[], brEval *result, brInstance *instance) {
	PushEnvironment *environment = BRPOINTER(&arguments[0]);

	BRINT(result) = pushVectorStackSize(environment);

	return EC_OK;
}

/*!
	\brief A breve API function wrapper for the C-function \ref pushVectorStackPop.

	See the documentation for \ref pushVectorStackPop for more details.
*/

int breveFunctionPushVectorStackPop(brEval arguments[], brEval *result, brInstance *instance) {
	PushEnvironment *environment = BRPOINTER(&arguments[0]);

	pushVectorStackPop(environment);

	return EC_OK;
}

/*!
	\brief A breve API function wrapper for the C-function \ref pushVectorStackTop.

	See the documentation for \ref pushVectorStackTop for more details.
*/

int breveFunctionPushVectorStackTop(brEval arguments[], brEval *result, brInstance *instance) {
	PushEnvironment *environment = BRPOINTER(&arguments[0]);

	BRPOINTER(result) = pushVectorStackTop(environment);

	return EC_OK;
}

/*!
	\brief A breve API function wrapper for the C-function \ref pushVectorStackPush.

	See the documentation for \ref pushVectorStackPush for more details.
*/

int breveFunctionPushVectorStackPush(brEval arguments[], brEval *result, brInstance *instance) {
	PushEnvironment *environment = BRPOINTER(&arguments[0]);
	PushVector *value = BRPOINTER(&arguments[1]);

	pushVectorStackPush(environment, value);

	return EC_OK;
}

/*@}*/

void breveInitPushFunctions(brNamespace *namespace) {
	brNewBreveCall(namespace, "pushEnvironmentNew", breveFunctionPushEnvironmentNew, AT_POINTER,  0);
 	brNewBreveCall(namespace, "pushEnvironmentFree", breveFunctionPushEnvironmentFree, AT_NULL, AT_POINTER, 0);
 	brNewBreveCall(namespace, "pushEnvironmentReadConfigFile", breveFunctionPushEnvironmentReadConfigFile, AT_NULL, AT_POINTER, AT_STRING, 0);
 	brNewBreveCall(namespace, "pushParse", breveFunctionPushParse, AT_POINTER, AT_STRING, 0);
 	brNewBreveCall(namespace, "pushRun", breveFunctionPushRun, AT_INT, AT_POINTER, AT_POINTER, 0);
 	brNewBreveCall(namespace, "pushGetConfigString", breveFunctionPushGetConfigString, AT_STRING, AT_POINTER, 0);
 	brNewBreveCall(namespace, "pushGetStacksString", breveFunctionPushGetStacksString, AT_STRING, AT_POINTER, 0);
 	brNewBreveCall(namespace, "pushClearStacks", breveFunctionPushClearStacks, AT_NULL, AT_POINTER, 0);
 	brNewBreveCall(namespace, "pushCodeGetString", breveFunctionPushCodeGetString, AT_STRING, AT_POINTER, 0);
 	brNewBreveCall(namespace, "pushCodeFree", breveFunctionPushCodeFree, AT_NULL, AT_POINTER, 0);
 	brNewBreveCall(namespace, "pushIntStackSize", breveFunctionPushIntStackSize, AT_INT, AT_POINTER, 0);
 	brNewBreveCall(namespace, "pushIntStackPop", breveFunctionPushIntStackPop, AT_NULL, AT_POINTER, 0);
 	brNewBreveCall(namespace, "pushIntStackTop", breveFunctionPushIntStackTop, AT_INT, AT_POINTER, 0);
 	brNewBreveCall(namespace, "pushIntStackPush", breveFunctionPushIntStackPush, AT_NULL, AT_POINTER, AT_INT, 0);
 	brNewBreveCall(namespace, "pushBoolStackSize", breveFunctionPushBoolStackSize, AT_INT, AT_POINTER, 0);
 	brNewBreveCall(namespace, "pushBoolStackPop", breveFunctionPushBoolStackPop, AT_NULL, AT_POINTER, 0);
 	brNewBreveCall(namespace, "pushBoolStackTop", breveFunctionPushBoolStackTop, AT_INT, AT_POINTER, 0);
 	brNewBreveCall(namespace, "pushBoolStackPush", breveFunctionPushBoolStackPush, AT_NULL, AT_POINTER, AT_INT, 0);
 	brNewBreveCall(namespace, "pushFloatStackSize", breveFunctionPushFloatStackSize, AT_INT, AT_POINTER, 0);
 	brNewBreveCall(namespace, "pushFloatStackPop", breveFunctionPushFloatStackPop, AT_NULL, AT_POINTER, 0);
 	brNewBreveCall(namespace, "pushFloatStackTop", breveFunctionPushFloatStackTop, AT_DOUBLE, AT_POINTER, 0);
 	brNewBreveCall(namespace, "pushFloatStackPush", breveFunctionPushFloatStackPush, AT_NULL, AT_POINTER, AT_DOUBLE, 0);
 	brNewBreveCall(namespace, "pushNameStackSize", breveFunctionPushNameStackSize, AT_INT, AT_POINTER, 0);
 	brNewBreveCall(namespace, "pushNameStackPop", breveFunctionPushNameStackPop, AT_NULL, AT_POINTER, 0);
 	brNewBreveCall(namespace, "pushNameStackTop", breveFunctionPushNameStackTop, AT_INT, AT_POINTER, 0);
 	brNewBreveCall(namespace, "pushNameStackPush", breveFunctionPushNameStackPush, AT_NULL, AT_POINTER, AT_INT, 0);
 	brNewBreveCall(namespace, "pushCodeStackSize", breveFunctionPushCodeStackSize, AT_INT, AT_POINTER, 0);
 	brNewBreveCall(namespace, "pushCodeStackPop", breveFunctionPushCodeStackPop, AT_NULL, AT_POINTER, 0);
 	brNewBreveCall(namespace, "pushCodeStackTop", breveFunctionPushCodeStackTop, AT_POINTER, AT_POINTER, 0);
 	brNewBreveCall(namespace, "pushCodeStackPush", breveFunctionPushCodeStackPush, AT_NULL, AT_POINTER, AT_POINTER, 0);
 	brNewBreveCall(namespace, "pushVectorStackSize", breveFunctionPushVectorStackSize, AT_INT, AT_POINTER, 0);
 	brNewBreveCall(namespace, "pushVectorStackPop", breveFunctionPushVectorStackPop, AT_NULL, AT_POINTER, 0);
 	brNewBreveCall(namespace, "pushVectorStackTop", breveFunctionPushVectorStackTop, AT_POINTER, AT_POINTER, 0);
 	brNewBreveCall(namespace, "pushVectorStackPush", breveFunctionPushVectorStackPush, AT_NULL, AT_POINTER, AT_POINTER, 0);
}
