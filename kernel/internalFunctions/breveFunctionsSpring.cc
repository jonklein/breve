#include "kernel.h"

#define BRSPRINGPOINTER(p)	((slSpring*)BRPOINTER(p))
#define BRLINKPOINTER(p)	((slLink*)BRPOINTER(p))

int brISpringNew(brEval args[], brEval *target, brInstance *i) {
	target->set( new slSpring(i->engine->world, BRLINKPOINTER(&args[0]), BRLINKPOINTER(&args[1]), &BRVECTOR(&args[2]), &BRVECTOR(&args[3]), BRDOUBLE(&args[4]), BRDOUBLE(&args[5]), BRDOUBLE(&args[6])) );

	return EC_OK;
}

int brISpringSetStrength(brEval args[], brEval *target, brInstance *i) {
	slSpring *s = (slSpring*)BRPOINTER(&args[0]);

	s->setStrength(BRDOUBLE(&args[1]));

	return EC_OK;
}

int brISpringSetLength(brEval args[], brEval *target, brInstance *i) {
	slSpring *s = (slSpring*)BRPOINTER(&args[0]);

	s->setLength(BRDOUBLE(&args[1]));

	return EC_OK;
}

int brISpringGetCurrentLength(brEval args[], brEval *target, brInstance *i) {
	slSpring *s = (slSpring*)BRPOINTER(&args[0]);

	target->set( s->getCurrentLength() );

	return EC_OK;
}

int brISpringGetForce(brEval args[], brEval *target, brInstance *i) {
	slSpring *s = (slSpring*)BRPOINTER(&args[0]);

	target->set( s->getForce() );

	return EC_OK;
}

int brISpringSetMaxForce(brEval args[], brEval *target, brInstance *i) {
	slSpring *s = (slSpring*)BRPOINTER(&args[0]);

	s->setMaxForce(BRDOUBLE(&args[1]));

	return EC_OK;
}

int brISpringSetDamping(brEval args[], brEval *target, brInstance *i) {
	slSpring *s = (slSpring*)BRPOINTER(&args[0]);

	s->setDamping(BRDOUBLE(&args[1]));

	return EC_OK;
}

int brISpringSetMode(brEval args[], brEval *target, brInstance *i) {
	slSpring *s = (slSpring*)BRPOINTER(&args[0]);

	s->setMode(BRINT(&args[1]));

	return EC_OK;
}

int brISpringRemove(brEval args[], brEval *target, brInstance *i) {
	slWorldRemoveConnection(i->engine->world, BRSPRINGPOINTER(&args[0]));

	return EC_OK;
}

void breveInitSpringFunctions(brNamespace *n) {
    brNewBreveCall(n, "springNew", brISpringNew, AT_POINTER, AT_POINTER, AT_POINTER, AT_VECTOR, AT_VECTOR, AT_DOUBLE, AT_DOUBLE, AT_DOUBLE, 0);
    brNewBreveCall(n, "springGetCurrentLength", brISpringGetCurrentLength, AT_DOUBLE, AT_POINTER, 0);
    brNewBreveCall(n, "springGetForce", brISpringGetForce, AT_DOUBLE, AT_POINTER, 0);
    brNewBreveCall(n, "springSetLength", brISpringSetLength, AT_NULL, AT_POINTER, AT_DOUBLE, 0);
    brNewBreveCall(n, "springSetMaxForce", brISpringSetMaxForce, AT_NULL, AT_POINTER, AT_DOUBLE, 0);
    brNewBreveCall(n, "springSetDamping", brISpringSetDamping, AT_NULL, AT_POINTER, AT_DOUBLE, 0);
    brNewBreveCall(n, "springSetStrength", brISpringSetStrength, AT_NULL, AT_POINTER, AT_DOUBLE, 0);
    brNewBreveCall(n, "springSetMode", brISpringSetMode, AT_NULL, AT_POINTER, AT_INT, 0);
    brNewBreveCall(n, "springRemove", brISpringRemove, AT_NULL, AT_POINTER, 0);
}
