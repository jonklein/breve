#include "steve.h"

void stSetParseObjectAndMethod(stObject *o, stMethod *m);

extern char *yyfile;
extern int lineno;

/*!
	\brief Parse a single steve statement.

	This prepares the statement for interactive evaluation.
*/

int stRunSingleStatement(stSteveData *sd, brEngine *engine, char *statement) {
    char *file = "<user input>";
    int length = strlen(statement) - 1;
    char *fixedStatement;
	brEval target;
	stInstance *i;
	stRunInstance ri;
	int r;

	if(length < 1) return 0;

	// put in the dot to make steve happy!

    while(statement[length] == '\n' || statement[length] == ' ' || statement[length] == '\t') length--;

    if(length > 0 && statement[length] != '.') statement[length + 1] = '.';

    fixedStatement = slMalloc(strlen(statement) + 5);

    sprintf(fixedStatement, "> %s", statement);

    yyfile = file;

    sd->singleStatement = NULL;

    stSetParseString(fixedStatement, strlen(fixedStatement));
    stParseSetObjectAndMethod((stObject*)engine->controller->class->pointer, sd->singleStatementMethod);
    stParseSetEngine(engine);

    engine->error.type = 0;

    if(yyparse() || engine->error.type) {
        slFree(fixedStatement);
        sd->singleStatement = NULL;
        return BPE_SIM_ERROR;
    }

	i = engine->controller->pointer;
	ri.instance = i;
	ri.type = i->type;

	i->gcStack = slStackNew(); 

    r = stExpEval(sd->singleStatement, &ri, &target, NULL);

	stGCCollectStack(i->gcStack);
	slStackFree(i->gcStack);
	i->gcStack = NULL;

    stExpFree(sd->singleStatement);
    slFree(fixedStatement);

    return r;
}
