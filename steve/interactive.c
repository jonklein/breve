#include "steve.h"
#include "expression.h"

void stSetParseObjectAndMethod(stObject *o, stMethod *m);

extern char *yyfile;
extern int lineno;

/*!
	\brief Parse a single steve statement.

	This prepares the statement for interactive evaluation.
*/

int stRunSingleStatement(stSteveData *sd, brEngine *engine, char *statement) {
	char *file = "<user input>";
	int length;
	brInstance *controller;
	char *fixedStatement;
	brEval target;
	stInstance *i;
	stRunInstance ri;
	int r;

	if(!statement) return 0;

	length = strlen(statement) - 1;

	if(length < 1) return 0;

	// put in the dot to make steve happy!


	while(statement[length] == '\n' || statement[length] == ' ' || statement[length] == '\t') length--;

	if(length > 0 && statement[length] != '.') statement[length + 1] = '.';

	fixedStatement = new char[strlen(statement) + 5];

	sprintf(fixedStatement, "> %s", statement);

	yyfile = file;

	sd->singleStatement = NULL;

	stSetParseString(fixedStatement, strlen(fixedStatement));

	controller = brEngineGetController(engine);

	stParseSetObjectAndMethod((stObject*)controller->object->userData, sd->singleStatementMethod);
	stParseSetEngine(engine);

	brClearError(engine);

	if(yyparse() || brGetError(engine)) {
		slFree(fixedStatement);
		sd->singleStatement = NULL;
		return BPE_SIM_ERROR;
	}

	i = (stInstance*)controller->userData;
	ri.instance = i;
	ri.type = i->type;

	i->gcStack = slStackNew(); 

	r = stExpEval(sd->singleStatement, &ri, &target, NULL);

	stGCCollectStack(i->gcStack);
	delete i->gcStack;

	i->gcStack = NULL;

	delete sd->singleStatement;
	delete[] fixedStatement;

	return r;
}
