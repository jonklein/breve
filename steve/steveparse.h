void stStartParse();
void stEndParse();
void stParseSetSteveData(stSteveData *d);
void stParseSetEngine(brEngine *e);
void stParseSetObject(stObject *o);
void stParseSetObjectAndMethod(stObject *o, stMethod *m);

char *slFormatText(char *text);

int yyparse();
