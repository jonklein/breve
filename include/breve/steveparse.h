void stStartParse(void);
void stEndParse(void);
void stParseSetSteveData(stSteveData *);
void stParseSetEngine(brEngine *);
void stParseSetObject(stObject *);
void stParseSetObjectAndMethod(stObject *, stMethod *);

char *slFormatText(char *);

int yyparse(void);
