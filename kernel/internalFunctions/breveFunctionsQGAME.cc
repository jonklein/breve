#include "kernel.h"

#if HAVE_LIBQGAME__

// WARNING: some unknown header is defining 'Complex' on Linux
// and messing up the qgame header import -- this is a temporary
// workaround

#undef Complex
#include <qgame++.h>

int brIQSysNew( brEval args[], brEval *target, brInstance *i ) {

	target->set( new qgame::QSys );

	return EC_OK;
}

int brIQSysFree( brEval args[], brEval *target, brInstance *i ) {
	qgame::QSys *sys = ( qgame::QSys * )BRPOINTER( &args[0] );
	delete sys;

	return EC_OK;
}

int brIQSysRunProgram( brEval args[], brEval *target, brInstance *i ) {
	return EC_OK;
}

int brIQProgramNew( brEval args[], brEval *target, brInstance *i ) {

	target->set( new qgame::QProgram );

	return EC_OK;
}

int brIQProgramFree( brEval args[], brEval *target, brInstance *i ) {
	qgame::QProgram *program = ( qgame::QProgram * )BRPOINTER( &args[0] );
	delete program;

	return EC_OK;
}

int brIQProgramAddInstruction( brEval args[], brEval *target, brInstance *i ) {
	qgame::QProgram *program = ( qgame::QProgram * )BRPOINTER( &args[0] );
	std::string s = BRSTRING( &args[1] );

	try {
		program->addInstruction( s );

		target->set( 0 );
	} catch ( qgame::Error e ) {
		slMessage( DEBUG_ALL, "error adding QGAME instruction: %s\n", e.s.c_str() );

		target->set( 1 );
	}

	return EC_OK;
}

int brIQProgramGetString( brEval args[], brEval *target, brInstance *i ) {
	qgame::QProgram *program = ( qgame::QProgram * )BRPOINTER( &args[0] );
	std::ostringstream os;

	os << *program;

	target->set( slStrdup(( char * )os.str().c_str() ) );

	return EC_OK;
}

int brIQProgramClear( brEval args[], brEval *target, brInstance *i ) {
	qgame::QProgram *program = ( qgame::QProgram * )BRPOINTER( &args[0] );

	program->clear();

	return EC_OK;
}

int brIQSysTestProgram( brEval args[], brEval *target, brInstance *i ) {
	qgame::QSys *sys = ( qgame::QSys * )BRPOINTER( &args[0] );
	qgame::QProgram *prog = ( qgame::QProgram * )BRPOINTER( &args[1] );
	std::vector< qgame::TestCase > cases;
	std::vector< brEval* >::iterator li;
	qgame::QubitList qb;

	// get the final measurement qubits...

	brEvalListHead *list = BRLIST( &args[4] );

	for ( li = list->_vector.begin(); li != list->_vector.end(); li++ ) {
		qb.append( BRINT( *li ) );
	}

	list = BRLIST( &args[2] );

	// generate the test cases...

	for ( li = list->_vector.begin(); li != list->_vector.end(); li++ ) {
		cases.push_back( qgame::TestCase( BRSTRING( *li ) ) );
	}

	qgame::Result result;

	try {
		result = sys->testProgram( BRINT( &args[3] ), prog, cases, qb,
		                           BRDOUBLE( &args[5] ) );
	} catch ( qgame::Error e ) {
		slMessage( DEBUG_ALL, "error executing QGAME program: %s\n",
		           e.s.c_str() );
		result.misses = -1;
		result.maxError = -1;
		result.avgError = -1;
		result.maxExpOracles = -1;
		result.avgExpOracles = -1;
	}

	list = new brEvalListHead();

	brEval eval;

	eval.set( result.misses );
	brEvalListInsert( list, list->_vector.size(), &eval );

	eval.set( result.maxError );
	brEvalListInsert( list, list->_vector.size(), &eval );

	eval.set( result.avgError );
	brEvalListInsert( list, list->_vector.size(), &eval );

	eval.set( result.maxExpOracles );
	brEvalListInsert( list, list->_vector.size(), &eval );

	eval.set( result.avgExpOracles );
	brEvalListInsert( list, list->_vector.size(), &eval );

	target->set( list );

	return EC_OK;
}

#endif

void breveInitQGAMEFunctions( brNamespace *n ) {
#if HAVE_LIBQGAME__
	brNewBreveCall( n, "qsysNew", brIQSysNew, AT_POINTER, 0 );
	brNewBreveCall( n, "qsysFree", brIQSysFree, AT_NULL, AT_POINTER, 0 );
	brNewBreveCall( n, "qsysRunProgram", brIQSysRunProgram, AT_NULL, AT_POINTER, AT_POINTER, 0 );
	brNewBreveCall( n, "qsysTestProgram", brIQSysTestProgram, AT_LIST, AT_POINTER, AT_POINTER, AT_LIST, AT_INT, AT_LIST, AT_DOUBLE, 0 );
	brNewBreveCall( n, "qprogramNew", brIQProgramNew, AT_POINTER, 0 );
	brNewBreveCall( n, "qprogramGetString", brIQProgramGetString, AT_STRING, AT_POINTER, 0 );
	brNewBreveCall( n, "qprogramClear", brIQProgramClear, AT_NULL, AT_POINTER, 0 );
	brNewBreveCall( n, "qprogramFree", brIQProgramFree, AT_NULL, AT_POINTER, 0 );
	brNewBreveCall( n, "qprogramAddInstruction", brIQProgramAddInstruction, AT_INT, AT_POINTER, AT_STRING, 0 );
#endif
}
