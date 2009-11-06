
void brBeginDrag(brEngine *, brInstance *);
void brEndDrag(brEngine *, brInstance *);
void brSetUpdateMenuCallback(brInstance *, void (*)(brInstance *));

int brMenuCallback(brEngine *, brInstance *, unsigned int);
brInstance *brClickCallback(brEngine *inEngine, brInstance *inClickedObject );
brInstance *brClickAtLocation(brEngine *, int, int);
int brDragCallback(brEngine *, int, int);
int brKeyCallback(brEngine *, unsigned char, int);
int brSpecialKeyCallback(brEngine *e, const char *name, int isDown);

int brInterfaceCallback(brEngine *, int, const char *);

char *brEngineRunSaveDialog(brEngine *);
char *brEngineRunLoadDialog(brEngine *);

void brEngineSetMouseLocation(brEngine *, int, int);
	