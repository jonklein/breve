#include "kernel.h"

#ifdef HAVE_LIBQGAME__

#include <qgame++.h>

int brIQSysNew(brEval args[], brEval *target, brInstance *i) {
	BRPOINTER(target) = (void*)new qgame::QSys;
	return EC_OK;
}

int brIQSysFree(brEval args[], brEval *target, brInstance *i) {
	qgame::QSys *sys = (qgame::QSys*)BRPOINTER(&args[0]);
	delete sys;
	return EC_OK;
}

int brIQSysRunProgram(brEval args[], brEval *target, brInstance *i) {
	return EC_OK;
}

int brIQProgramNew(brEval args[], brEval *target, brInstance *i) {
	BRPOINTER(target) = (void*)new qgame::QProgram;
	return EC_OK;
}

int brIQProgramFree(brEval args[], brEval *target, brInstance *i) {
	qgame::QProgram *program = (qgame::QProgram*)BRPOINTER(&args[0]);
	delete program;
	return EC_OK;
}

int brIQProgramAddInstruction(brEval args[], brEval *target, brInstance *i) {
	qgame::QProgram *program = (qgame::QProgram*)BRPOINTER(&args[0]);
	BRINT(target) = program->addInstruction(BRSTRING(&args[0]));
	return EC_OK;
}

int brIQSysTestProgram(brEval args[], brEval *target, brInstance *i) {
	qgame::QSys *sys = (qgame::QSys*)BRPOINTER(&args[0]);
	qgame::QProgram *prog = (qgame::QProgram*)BRPOINTER(&args[1]);
	std::vector<qgame::TestCase> cases;
	qgame::QubitList qb;

	brEvalListHead *list = BRLIST(&args[2]);
	brEvalList *start = list->start;

	while(start) {
		start = start->next;

		cases.push_back( qgame::TestCase( BRSTRING(&start->eval)));
	}

	sys->testProgram(BRINT(&args[3]), prog, cases, qb, BRDOUBLE(&args[4]));

	return EC_OK;
}
#endif

void breveInitQGAMEFunctions(brNamespace *n) {
#ifdef HAVE_LIBQGAME__
	brNewBreveCall(n, "qsysNew", brIQSysNew, AT_POINTER, 0);
	brNewBreveCall(n, "qsysFree", brIQSysFree, AT_NULL, AT_POINTER, 0);
	brNewBreveCall(n, "qsysRunProgram", brIQSysRunProgram, AT_NULL, AT_POINTER, AT_POINTER, 0);
	brNewBreveCall(n, "qsysTestProgram", brIQSysTestProgram, AT_NULL, AT_POINTER, AT_POINTER, AT_LIST, AT_INT, AT_DOUBLE, 0);
	brNewBreveCall(n, "qprogramNew", brIQProgramNew, AT_POINTER, 0);
	brNewBreveCall(n, "qprogramFree", brIQProgramFree, AT_NULL, AT_POINTER, 0);
	brNewBreveCall(n, "qprogramAddInstruction", brIQProgramAddInstruction, AT_INT, AT_POINTER, AT_STRING, 0);
#endif
}
