#include "kernel.h"

#ifdef HAVE_LIBQGAME__

// WARNING: some unknown header is defining 'Complex' on Linux
// and messing up the qgame header import -- this is a temporary
// workaround

#undef Complex
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

	try {
		program->addInstruction(BRSTRING(&args[1]));
		BRINT(target) = 0;
	} catch(qgame::Error e) {
		BRINT(target) = 1;
	}

	return EC_OK;
}

int brIQSysTestProgram(brEval args[], brEval *target, brInstance *i) {
	qgame::QSys *sys = (qgame::QSys*)BRPOINTER(&args[0]);
	qgame::QProgram *prog = (qgame::QProgram*)BRPOINTER(&args[1]);
	std::vector<qgame::TestCase> cases;
	qgame::QubitList qb;

	// get the final measurement qubits...

	brEvalListHead *list = BRLIST(&args[4]);
	brEvalList *start = list->start;

	while(start) {
		qb.append( BRINT(&start->eval) );
		start = start->next;
	}

	list = BRLIST(&args[2]);
	start = list->start;

	// generate the test cases... 

	while(start) {
		cases.push_back( qgame::TestCase( BRSTRING(&start->eval)));
		start = start->next;
	}

	qgame::Result result = sys->testProgram(BRINT(&args[3]), prog, cases, qb, BRDOUBLE(&args[5]));

	list = brEvalListNew();

	brEval eval;

	eval.type = AT_INT;
	BRINT(&eval) = result.misses;
	brEvalListInsert(list, list->count, &eval);

	eval.type = AT_DOUBLE;
	BRDOUBLE(&eval) = result.maxError;
	brEvalListInsert(list, list->count, &eval);

	eval.type = AT_DOUBLE;
	BRDOUBLE(&eval) = result.avgError;
	brEvalListInsert(list, list->count, &eval);

	eval.type = AT_DOUBLE;
	BRDOUBLE(&eval) = result.maxExpOracles;
	brEvalListInsert(list, list->count, &eval);

	eval.type = AT_DOUBLE;
	BRDOUBLE(&eval) = result.avgExpOracles;
	brEvalListInsert(list, list->count, &eval);

	BRLIST(target) = list;

	return EC_OK;
}
#endif

void breveInitQGAMEFunctions(brNamespace *n) {
#ifdef HAVE_LIBQGAME__
	brNewBreveCall(n, "qsysNew", brIQSysNew, AT_POINTER, 0);
	brNewBreveCall(n, "qsysFree", brIQSysFree, AT_NULL, AT_POINTER, 0);
	brNewBreveCall(n, "qsysRunProgram", brIQSysRunProgram, AT_NULL, AT_POINTER, AT_POINTER, 0);
	brNewBreveCall(n, "qsysTestProgram", brIQSysTestProgram, AT_LIST, AT_POINTER, AT_POINTER, AT_LIST, AT_INT, AT_LIST, AT_DOUBLE, 0);
	brNewBreveCall(n, "qprogramNew", brIQProgramNew, AT_POINTER, 0);
	brNewBreveCall(n, "qprogramFree", brIQProgramFree, AT_NULL, AT_POINTER, 0);
	brNewBreveCall(n, "qprogramAddInstruction", brIQProgramAddInstruction, AT_INT, AT_POINTER, AT_STRING, 0);
#endif
}
