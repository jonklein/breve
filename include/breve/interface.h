#ifdef __cplusplus
extern "C" {
#endif
void brBeginDrag(brEngine *, brInstance *);
void brEndDrag(brEngine *, brInstance *);
void brSetUpdateMenuCallback(brInstance *, void (*)(brInstance *));

int brMenuCallback(brEngine *, brInstance *, unsigned int);
int brMenuCallbackByName(brEngine *, char *);
brInstance *brClickCallback(brEngine *, int);
brInstance *brClickAtLocation(brEngine *, int, int);
int brDragCallback(brEngine *, int, int);
int brKeyCallback(brEngine *, unsigned char, int);
int brSpecialKeyCallback(brEngine *e, char *name, int isDown);

int brInterfaceCallback(brEngine *, int, char *);

char *brEngineRunSaveDialog(brEngine *);
char *brEngineRunLoadDialog(brEngine *);

void brEngineSetMouseLocation(brEngine *, int, int);

#ifdef __cplusplus
}
#endif
