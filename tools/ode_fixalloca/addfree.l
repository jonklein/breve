%{
	typedef struct nameList nameList;
	typedef struct nameStack nameStack;

	struct nameList {
		char *name;
		nameList *next;
	};

	struct nameStack {
		nameList *list;
		nameStack *next;
	};

	nameStack *stack = NULL;

	char *markNamed(char *text);
	nameList *nameListAppend(nameList *list, char *name);
	nameStack *nextNameStack(nameStack *stack);
%}

%x COMMENT

%%

<COMMENT>.			ECHO;
<COMMENT>\*\/		{ ECHO; BEGIN(0); };

\/\*		{ ECHO; BEGIN(COMMENT); }

\/\/\ ALLOC_MARK:[a-zA-Z0-9_]*\n	{
		char *name;

		/* strip the \n */

		yytext[strlen(yytext) - 1] = 0;

		name = markNamed(yytext);

		stack->list = nameListAppend(stack->list, name);
	}

\/\/.*				printf("%s", yytext);

\{				{	
					printf("{"); 

					stack = nextNameStack(stack);
				}
\}				{	
					if(stack->list) {
						nameList *list = stack->list;
						printf("#ifdef NO_STACK_ALLOC\n");
						while(list) {
							printf("  free(%s);\n", list->name);
							list = list->next;
						}
						printf("#endif /* NO_STACK_ALLOC */\n");

						list = NULL;
					}

					stack = stack->next;

					printf("}"); 
				}

.					putchar(yytext[0]);

return[^a-zA-Z0-9_].*;			{
						if(stack->list) {
							nameList *list = stack->list;

							printf("{\n");
							printf("#ifdef NO_STACK_ALLOC\n");

							while(list) {
								printf("  free(%s);\n", list->name);
								list = list->next;
							}

							printf("#endif /* NO_STACK_ALLOC */\n");

							ECHO;

							printf("}\n");
						} else ECHO;
					}

%%

char *markNamed(char *text) {
	int n = 0;
	char *name;

	while(text[n] && text[n] != ':') n++;
	if(text[n] == ':') n++;

	name = strdup(&text[n]);

	return name;
}

nameList *nameListAppend(nameList *list, char *name) {
	nameList *new = malloc(sizeof(nameList));
	nameList *start = list;

	new->name = name;
	new->next = NULL;

	if(list) {
		while(list->next) list = list->next;
		list->next = new;
	} else {
		return new;
	}

	return start;
}

nameStack *nextNameStack(nameStack *n) {
	nameStack *s;

	s = malloc(sizeof(nameStack));
	s->next = n;
	s->list = NULL;

	return s;
}
