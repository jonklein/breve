#include "kernel.h"

#define BRSPRINGPOINTER(p)	((slSpring*)BRPOINTER(p))
#define BRLINKPOINTER(p)	((slLink*)BRPOINTER(p))

int brISpringNew(brEval args[], brEval *target, brInstance *i) {
	BRSPRINGPOINTER(target) = slSpringNew(BRLINKPOINTER(&args[0]), BRLINKPOINTER(&args[1]), &BRVECTOR(&args[2]), &BRVECTOR(&args[3]), BRDOUBLE(&args[4]), BRDOUBLE(&args[5]), BRDOUBLE(&args[6]));

	slWorldAddSpring(i->engine->world, BRSPRINGPOINTER(target));

	return EC_OK;
}

int brISpringSetStrength(brEval args[], brEval *target, brInstance *i) {
	slSpringSetStrength(BRSPRINGPOINTER(&args[0]), BRDOUBLE(&args[1]));
	return EC_OK;
}

int brISpringSetLength(brEval args[], brEval *target, brInstance *i) {
	slSpringSetLength(BRSPRINGPOINTER(&args[0]), BRDOUBLE(&args[1]));
	return EC_OK;
}

int brISpringGetCurrentLength(brEval args[], brEval *target, brInstance *i) {
	BRDOUBLE(target) = slSpringGetCurrentLength(BRSPRINGPOINTER(&args[0]));
	return EC_OK;
}

int brISpringSetDamping(brEval args[], brEval *target, brInstance *i) {
	slSpringSetDamping(BRSPRINGPOINTER(&args[0]), BRDOUBLE(&args[1]));
	return EC_OK;
}

int brISpringSetMode(brEval args[], brEval *target, brInstance *i) {
	slSpringSetMode(BRSPRINGPOINTER(&args[0]), BRINT(&args[1]));
	return EC_OK;
}

int brISpringRemove(brEval args[], brEval *target, brInstance *i) {
	slWorldRemoveSpring(i->engine->world, BRSPRINGPOINTER(&args[0]));
	return EC_OK;
}

void breveInitSpringFunctions(brNamespace *n) {
    brNewBreveCall(n, "springNew", brISpringNew, AT_POINTER, AT_POINTER, AT_POINTER, AT_VECTOR, AT_VECTOR, AT_DOUBLE, AT_DOUBLE, AT_DOUBLE, 0);
    brNewBreveCall(n, "springGetCurrentLength", brISpringGetCurrentLength, AT_DOUBLE, AT_POINTER, 0);
    brNewBreveCall(n, "springSetLength", brISpringSetLength, AT_NULL, AT_POINTER, AT_DOUBLE, 0);
    brNewBreveCall(n, "springSetDamping", brISpringSetDamping, AT_NULL, AT_POINTER, AT_DOUBLE, 0);
    brNewBreveCall(n, "springSetStrength", brISpringSetStrength, AT_NULL, AT_POINTER, AT_DOUBLE, 0);
    brNewBreveCall(n, "springSetMode", brISpringSetMode, AT_NULL, AT_POINTER, AT_INT, 0);
    brNewBreveCall(n, "springRemove", brISpringRemove, AT_NULL, AT_POINTER, 0);
}
