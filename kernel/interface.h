#ifdef __cplusplus
extern "C" {
#endif
void brBeginDrag(brEngine*, brInstance*);
void brEndDrag(brEngine*, brInstance*);
void brSetUpdateMenuCallback(brInstance *i, void (*updateMenu)(brInstance *l));

int brMenuCallback(brEngine *e, brInstance *i, int n);
int brMenuCallbackByName(brEngine *e, char *name);
brInstance *brClickCallback(brEngine *e, int n);
brInstance *brClickAtLocation(brEngine *e, int x, int y);
int brDragCallback(brEngine *e, int x, int y);
int brKeyCallback(brEngine *e, unsigned char keyCode, int isDown);
int brInterfaceCallback(brEngine *e, int interfaceID, char *string);

char *brEngineRunSaveDialog(brEngine *e);
char *brEngineRunLoadDialog(brEngine *e);

void brEngineSetMouseLocation(brEngine *e, int x, int y);

#ifdef __cplusplus
}
#endif
