/*
	the breve simulation environment

	This file was generated automatically by the script /Users/jk/dev/breve/tools/apiwrap.pl.
*/

#include "internal.h"
#include "util.h"

void breveInitPushFunctions(brNamespace *n);

/*@{*/
/*! \addtogroup InternalFunctions */
#ifdef HAVE_LIBPUSH

#include "pushC.h"
#include "push/Code.h"
#include "push/CodeUtils.h"

/*!
	\brief A breve API function wrapper for the C-function \ref pushEnvironmentNew.

	See the documentation for \ref pushEnvironmentNew for more details.
*/

int breveFunctionPushEnvironmentNew(brEval arguments[], brEval *result, brInstance *instance) {
	int seed = BRINT(&arguments[0]);

	BRPOINTER(result) = pushEnvironmentNew(seed);

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
	\brief A breve API function wrapper for the C-function \ref pushRun.

	See the documentation for \ref pushRun for more details.
*/

int breveFunctionPushRun(brEval arguments[], brEval *result, brInstance *instance) {
	PushEnvironment *environment = BRPOINTER(&arguments[0]);
 	PushCode *code = BRPOINTER(&arguments[1]);
 	int steps = BRINT(&arguments[2]);

	BRINT(result) = pushRun(environment, code, steps);

	return EC_OK;
}

/*!
	\brief A breve API function wrapper for the C-function \ref pushStep.

	See the documentation for \ref pushStep for more details.
*/

int breveFunctionPushStep(brEval arguments[], brEval *result, brInstance *instance) {
	PushEnvironment *environment = BRPOINTER(&arguments[0]);
 	int steps = BRINT(&arguments[1]);

	BRINT(result) = pushStep(environment, steps);

	return EC_OK;
}

/*!
	\brief A breve API function wrapper for the C-function \ref pushGetConfigString.

	See the documentation for \ref pushGetConfigString for more details.
*/

int breveFunctionPushGetConfigString(brEval arguments[], brEval *result, brInstance *instance) {
	PushEnvironment *environment = BRPOINTER(&arguments[0]);

	BRSTRING(result) = slStrdupAndFree(pushGetConfigString(environment));

	return EC_OK;
}

/*!
	\brief A breve API function wrapper for the C-function \ref pushGetStacksString.

	See the documentation for \ref pushGetStacksString for more details.
*/

int breveFunctionPushGetStacksString(brEval arguments[], brEval *result, brInstance *instance) {
	PushEnvironment *environment = BRPOINTER(&arguments[0]);

	BRSTRING(result) = slStrdupAndFree(pushGetStacksString(environment));

	return EC_OK;
}

/*!
	\brief A breve API function wrapper for the C-function \ref pushGetExecStackString.

	See the documentation for \ref pushGetExecStackString for more details.
*/

int breveFunctionPushGetExecStackString(brEval arguments[], brEval *result, brInstance *instance) {
	PushEnvironment *environment = BRPOINTER(&arguments[0]);

	BRSTRING(result) = slStrdupAndFree(pushGetExecStackString(environment));

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
	\brief A breve API function wrapper for the C-function \ref pushEnvironmentSetListLimit.

	See the documentation for \ref pushEnvironmentSetListLimit for more details.
*/

int breveFunctionPushEnvironmentSetListLimit(brEval arguments[], brEval *result, brInstance *instance) {
	PushEnvironment *environment = BRPOINTER(&arguments[0]);
 	int limit = BRINT(&arguments[1]);

	pushEnvironmentSetListLimit(environment, limit);

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
	\brief A breve API function wrapper for the C-function \ref pushCodeGetString.

	See the documentation for \ref pushCodeGetString for more details.
*/

int breveFunctionPushCodeGetString(brEval arguments[], brEval *result, brInstance *instance) {
	PushCode *code = BRPOINTER(&arguments[0]);

	BRSTRING(result) = slStrdupAndFree(pushCodeGetString(code));

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
	\brief A breve API function wrapper for the C-function \ref pushCodeSize.

	See the documentation for \ref pushCodeSize for more details.
*/

int breveFunctionPushCodeSize(brEval arguments[], brEval *result, brInstance *instance) {
	PushCode *code = BRPOINTER(&arguments[0]);

	BRINT(result) = pushCodeSize(code);

	return EC_OK;
}

int breveFunctionPushCodeTopLevelSize(brEval arguments[], brEval *result, brInstance *instance) {
	push::Code *p1 = (push::Code*)BRPOINTER(&arguments[0]);

	BRINT(result) = (*p1)->get_stack().size();

	return EC_OK;
}

int breveFunctionPushCodeSwapSublists(brEval arguments[], brEval *result, brInstance *instance) {
	push::Code *p1 = (push::Code*)BRPOINTER(&arguments[0]);
	int x = BRINT(&arguments[1]);
	int y = BRINT(&arguments[2]);

	push::swap_sublist_position(*p1, x, y);

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
	pushVectorStackTop(BRPOINTER(&arguments[0]), (PushVector*)&BRVECTOR(result));
	return EC_OK;
}

/*!
	\brief A breve API function wrapper for the C-function \ref pushVectorStackPush.

	See the documentation for \ref pushVectorStackPush for more details.
*/

int breveFunctionPushVectorStackPush(brEval arguments[], brEval *result, brInstance *instance) {
	PushEnvironment *environment = BRPOINTER(&arguments[0]);
 	PushVector *value = (PushVector*)&BRVECTOR(&arguments[1]);

	pushVectorStackPush(environment, value);

	return EC_OK;
}

/*!
	\brief A breve API function wrapper for the C-function \ref pushCodeDeletionMutate.

	See the documentation for \ref pushCodeDeletionMutate for more details.
*/

int breveFunctionPushCodeDeletionMutate(brEval arguments[], brEval *result, brInstance *instance) {
 	PushCode *p1 = BRPOINTER(&arguments[0]);

	BRPOINTER(result) = pushCodeDeletionMutate(p1);

	return EC_OK;
}

/*!
	\brief A breve API function wrapper for the C-function \ref pushCodeFlattenMutate.

	See the documentation for \ref pushCodeFlattenMutate for more details.
*/

int breveFunctionPushCodeFlattenMutate(brEval arguments[], brEval *result, brInstance *instance) {
 	PushCode *p1 = BRPOINTER(&arguments[0]);

	BRPOINTER(result) = pushCodeFlattenMutate(p1);

	return EC_OK;
}

/*!
	\brief A breve API function wrapper for the C-function \ref pushCodeSubtreeMutate.

	See the documentation for \ref pushCodeSubtreeMutate for more details.
*/

int breveFunctionPushCodeSubtreeMutate(brEval arguments[], brEval *result, brInstance *instance) {
	PushEnvironment *environment = BRPOINTER(&arguments[0]);
 	PushCode *p1 = BRPOINTER(&arguments[1]);
 	int size = BRINT(&arguments[2]);

	BRPOINTER(result) = pushCodeSubtreeMutate(environment, p1, size);

	return EC_OK;
}

/*!
	\brief A breve API function wrapper for the C-function \ref pushCodeCrossover.

	See the documentation for \ref pushCodeCrossover for more details.
*/

int breveFunctionPushCodeCrossover(brEval arguments[], brEval *result, brInstance *instance) {
	PushEnvironment *environment = BRPOINTER(&arguments[0]);
	PushCode *p1 = BRPOINTER(&arguments[1]);
 	PushCode *p2 = BRPOINTER(&arguments[2]);

	BRPOINTER(result) = pushCodeCrossover(environment, p1, p2);

	return EC_OK;
}

/*!
	\brief A breve API function wrapper for the C-function \ref pushCodeRandom.

	See the documentation for \ref pushCodeRandom for more details.
*/

int breveFunctionPushCodeRandom(brEval arguments[], brEval *result, brInstance *instance) {
	PushEnvironment *environment = BRPOINTER(&arguments[0]);
 	int size = BRINT(&arguments[1]);

	BRPOINTER(result) = pushCodeRandom(environment, size);

	return EC_OK;
}

int breveFunctionPushCodeDiscrepancy(brEval arguments[], brEval *result, brInstance *instance) {
	push::Code *p1, *p2;

	p1 = (push::Code*)BRPOINTER(&arguments[0]);
	p2 = (push::Code*)BRPOINTER(&arguments[1]);

	BRINT(result) = push::discrepancy(*p1, *p2);

	return EC_OK;
}

int breveFunctionPushCodeTopLevelDiff(brEval arguments[], brEval *result, brInstance *instance) {
	push::Code *p1, *p2;
	unsigned int length, diff, n;
	unsigned int s1, s2;

	p1 = (push::Code*)BRPOINTER(&arguments[0]);
	p2 = (push::Code*)BRPOINTER(&arguments[1]);

	const push::CodeArray& st1 = (*p1)->get_stack();
	const push::CodeArray& st2 = (*p2)->get_stack();

	// get the shortest length

	s1 = st1.size();
	s2 = st2.size();

	if(s1 < s2) length = s1;
	else length = s2;

	// any extra elements are automatically different

	diff = abs(s1 - s2);

	// the lists are stored backwards.

	for(n=0; n < length; n++) {
		if( !equal_to(st1[s1 - (n+1)], st2[s2 - (n+1)])) {
			// printf("%s == %s\n", pushCodeGetString((PushCode*)&st1[n]), pushCodeGetString((PushCode*)&st2[n]));
			diff++;
		}
	}

	BRINT(result) = diff;

	return EC_OK;
}

#endif /* HAVE_LIBPUSH */
/*@}*/

void breveInitPushFunctions(brNamespace *n) {
#ifdef HAVE_LIBPUSH
	brNewBreveCall(n, "pushEnvironmentNew", breveFunctionPushEnvironmentNew, AT_POINTER, AT_INT, 0);
 	brNewBreveCall(n, "pushEnvironmentFree", breveFunctionPushEnvironmentFree, AT_NULL, AT_POINTER, 0);
 	brNewBreveCall(n, "pushEnvironmentReadConfigFile", breveFunctionPushEnvironmentReadConfigFile, AT_NULL, AT_POINTER, AT_STRING, 0);
 	brNewBreveCall(n, "pushRun", breveFunctionPushRun, AT_INT, AT_POINTER, AT_POINTER, AT_INT, 0);
 	brNewBreveCall(n, "pushStep", breveFunctionPushStep, AT_INT, AT_POINTER, AT_INT, 0);
 	brNewBreveCall(n, "pushGetConfigString", breveFunctionPushGetConfigString, AT_STRING, AT_POINTER, 0);
 	brNewBreveCall(n, "pushGetStacksString", breveFunctionPushGetStacksString, AT_STRING, AT_POINTER, 0);
 	brNewBreveCall(n, "pushGetExecStackString", breveFunctionPushGetExecStackString, AT_STRING, AT_POINTER, 0);
 	brNewBreveCall(n, "pushClearStacks", breveFunctionPushClearStacks, AT_NULL, AT_POINTER, 0);
 	brNewBreveCall(n, "pushEnvironmentSetListLimit", breveFunctionPushEnvironmentSetListLimit, AT_NULL, AT_POINTER, AT_INT, 0);
 	brNewBreveCall(n, "pushParse", breveFunctionPushParse, AT_POINTER, AT_STRING, 0);
 	brNewBreveCall(n, "pushCodeGetString", breveFunctionPushCodeGetString, AT_STRING, AT_POINTER, 0);
 	brNewBreveCall(n, "pushCodeFree", breveFunctionPushCodeFree, AT_NULL, AT_POINTER, 0);
 	brNewBreveCall(n, "pushCodeSize", breveFunctionPushCodeSize, AT_INT, AT_POINTER, 0);
 	brNewBreveCall(n, "pushCodeTopLevelSize", breveFunctionPushCodeTopLevelSize, AT_INT, AT_POINTER, 0);
 	brNewBreveCall(n, "pushCodeSwapSublists", breveFunctionPushCodeSwapSublists, AT_INT, AT_POINTER, AT_INT, AT_INT, 0);
 	brNewBreveCall(n, "pushIntStackSize", breveFunctionPushIntStackSize, AT_INT, AT_POINTER, 0);
 	brNewBreveCall(n, "pushIntStackPop", breveFunctionPushIntStackPop, AT_NULL, AT_POINTER, 0);
 	brNewBreveCall(n, "pushIntStackTop", breveFunctionPushIntStackTop, AT_INT, AT_POINTER, 0);
 	brNewBreveCall(n, "pushIntStackPush", breveFunctionPushIntStackPush, AT_NULL, AT_POINTER, AT_INT, 0);
 	brNewBreveCall(n, "pushBoolStackSize", breveFunctionPushBoolStackSize, AT_INT, AT_POINTER, 0);
 	brNewBreveCall(n, "pushBoolStackPop", breveFunctionPushBoolStackPop, AT_NULL, AT_POINTER, 0);
 	brNewBreveCall(n, "pushBoolStackTop", breveFunctionPushBoolStackTop, AT_INT, AT_POINTER, 0);
 	brNewBreveCall(n, "pushBoolStackPush", breveFunctionPushBoolStackPush, AT_NULL, AT_POINTER, AT_INT, 0);
 	brNewBreveCall(n, "pushFloatStackSize", breveFunctionPushFloatStackSize, AT_INT, AT_POINTER, 0);
 	brNewBreveCall(n, "pushFloatStackPop", breveFunctionPushFloatStackPop, AT_NULL, AT_POINTER, 0);
 	brNewBreveCall(n, "pushFloatStackTop", breveFunctionPushFloatStackTop, AT_DOUBLE, AT_POINTER, 0);
 	brNewBreveCall(n, "pushFloatStackPush", breveFunctionPushFloatStackPush, AT_NULL, AT_POINTER, AT_DOUBLE, 0);
 	brNewBreveCall(n, "pushNameStackSize", breveFunctionPushNameStackSize, AT_INT, AT_POINTER, 0);
 	brNewBreveCall(n, "pushNameStackPop", breveFunctionPushNameStackPop, AT_NULL, AT_POINTER, 0);
 	brNewBreveCall(n, "pushNameStackTop", breveFunctionPushNameStackTop, AT_INT, AT_POINTER, 0);
 	brNewBreveCall(n, "pushNameStackPush", breveFunctionPushNameStackPush, AT_NULL, AT_POINTER, AT_INT, 0);
 	brNewBreveCall(n, "pushCodeStackSize", breveFunctionPushCodeStackSize, AT_INT, AT_POINTER, 0);
 	brNewBreveCall(n, "pushCodeStackPop", breveFunctionPushCodeStackPop, AT_NULL, AT_POINTER, 0);
 	brNewBreveCall(n, "pushCodeStackTop", breveFunctionPushCodeStackTop, AT_POINTER, AT_POINTER, 0);
 	brNewBreveCall(n, "pushCodeStackPush", breveFunctionPushCodeStackPush, AT_NULL, AT_POINTER, AT_POINTER, 0);
 	brNewBreveCall(n, "pushCodeDiscrepancy", breveFunctionPushCodeDiscrepancy, AT_INT, AT_POINTER, AT_POINTER, 0);
 	brNewBreveCall(n, "pushCodeTopLevelDiff", breveFunctionPushCodeTopLevelDiff, AT_INT, AT_POINTER, AT_POINTER, 0);
 	brNewBreveCall(n, "pushVectorStackSize", breveFunctionPushVectorStackSize, AT_INT, AT_POINTER, 0);
 	brNewBreveCall(n, "pushVectorStackPop", breveFunctionPushVectorStackPop, AT_NULL, AT_POINTER, 0);
 	brNewBreveCall(n, "pushVectorStackTop", breveFunctionPushVectorStackTop, AT_VECTOR, AT_POINTER, AT_VECTOR, 0);
 	brNewBreveCall(n, "pushVectorStackPush", breveFunctionPushVectorStackPush, AT_NULL, AT_POINTER, AT_VECTOR, 0);
 	brNewBreveCall(n, "pushCodeDeletionMutate", breveFunctionPushCodeDeletionMutate, AT_POINTER, AT_POINTER, 0);
 	brNewBreveCall(n, "pushCodeFlattenMutate", breveFunctionPushCodeFlattenMutate, AT_POINTER, AT_POINTER, 0);
 	brNewBreveCall(n, "pushCodeSubtreeMutate", breveFunctionPushCodeSubtreeMutate, AT_POINTER, AT_POINTER, AT_POINTER, AT_INT, 0);
 	brNewBreveCall(n, "pushCodeCrossover", breveFunctionPushCodeCrossover, AT_POINTER, AT_POINTER, AT_POINTER, AT_POINTER, 0);
 	brNewBreveCall(n, "pushCodeRandom", breveFunctionPushCodeRandom, AT_POINTER, AT_POINTER, AT_INT, 0);
#endif /* HAVE_LIBPUSH */
}
