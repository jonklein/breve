#include "kernel.h"

int brISpringNew(brEval args[], brEval *target, brInstance *i) {
	BRPOINTER(target) = slSpringNew(BRPOINTER(&args[0]), BRPOINTER(&args[1]), &BRVECTOR(&args[2]), &BRVECTOR(&args[3]), BRDOUBLE(&args[4]), BRDOUBLE(&args[5]), BRDOUBLE(&args[6]));

	slWorldAddSpring(i->engine->world, BRPOINTER(target));

	return EC_OK;
}

int brISpringSetStrength(brEval args[], brEval *target, brInstance *i) {
	slSpringSetStrength(BRPOINTER(&args[0]), BRDOUBLE(&args[1]));
	return EC_OK;
}

int brISpringSetLength(brEval args[], brEval *target, brInstance *i) {
	slSpringSetLength(BRPOINTER(&args[0]), BRDOUBLE(&args[1]));
	return EC_OK;
}

int brISpringSetDamping(brEval args[], brEval *target, brInstance *i) {
	slSpringSetDamping(BRPOINTER(&args[0]), BRDOUBLE(&args[1]));
	return EC_OK;
}

int brISpringSetMode(brEval args[], brEval *target, brInstance *i) {
	slSpringSetMode(BRPOINTER(&args[0]), BRINT(&args[1]));
	return EC_OK;
}

int brISpringRemove(brEval args[], brEval *target, brInstance *i) {
	slWorldRemoveSpring(i->engine->world, BRPOINTER(&args[0]));
	return EC_OK;
}

void breveInitSpringFunctions(brNamespace *n) {
    brNewBreveCall(n, "springNew", brISpringNew, AT_POINTER, AT_POINTER, AT_POINTER, AT_VECTOR, AT_VECTOR, AT_DOUBLE, AT_DOUBLE, AT_DOUBLE, 0);
    brNewBreveCall(n, "springSetLength", brISpringSetLength, AT_NULL, AT_POINTER, AT_DOUBLE, 0);
    brNewBreveCall(n, "springSetDamping", brISpringSetDamping, AT_NULL, AT_POINTER, AT_DOUBLE, 0);
    brNewBreveCall(n, "springSetStrength", brISpringSetStrength, AT_NULL, AT_POINTER, AT_DOUBLE, 0);
    brNewBreveCall(n, "springSetMode", brISpringSetStrength, AT_NULL, AT_POINTER, AT_INT, 0);
    brNewBreveCall(n, "springRemove", brISpringRemove, AT_NULL, AT_POINTER, 0);
}
