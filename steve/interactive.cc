#include "steve.h"
#include "expression.h"
#include "evaluation.h"

void stSetParseObjectAndMethod( stObject *o, stMethod *m );

extern const char *yyfile;
extern int lineno;

/**
 * \brief Parse a single steve statement.
 * This prepares the statement for interactive evaluation.
 */

int stRunSingleStatement( stSteveData *inSteveData, brEngine *inEngine, const char *inStatement ) {
	const char *file = "<user input>";
	int length, r;
	brInstance *controller;
	char *fixedStatement;
	brEval target;
	stInstance *i;
	stRunInstance ri;

	if ( !inStatement ) 
		return 0;

	char *statement = slStrdup( inStatement );


	length = strlen( statement ) - 1;

	if ( length < 1 ) return 0;

	// put in the dot to make steve happy!

	while ( statement[length] == '\n' || statement[length] == ' ' || statement[length] == '\t' ) 
		length--;

	if ( length > 0 && statement[length	] != '.' ) 
		statement[length + 1] = '.';

	fixedStatement = new char[strlen( statement ) + 5];

	// std::string fixedStatement = "> ";
	// fixedStatement += statement;

	sprintf( fixedStatement, "> %s", statement );

	yyfile = file;

	inSteveData -> singleStatement = NULL;

	stSetParseData( inSteveData, fixedStatement, strlen( fixedStatement ) );

	controller = inEngine -> getController();

	stParseSetEngine( inEngine );
	stParseSetObjectAndMethod( ( stObject* )controller->object->userData, inSteveData -> singleStatementMethod );

	brClearError( inEngine );

	if ( yyparse() || brGetError( inEngine ) ) {
		slFree( fixedStatement );
		inSteveData -> singleStatement = NULL;
		return BPE_SIM_ERROR;
	}

	i = ( stInstance* )controller->userData;

	ri.instance = i;
	ri.type = i->type;

	r = stExpEval( inSteveData -> singleStatement, &ri, &target, NULL );

	delete inSteveData -> singleStatement;
	delete[] fixedStatement;

	slFree( statement );

	return r;
}
