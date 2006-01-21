#include "kernel.h"
#include "gldraw.h"
#include "drawcommand.h"

int brIDrawListNew(brEval args[], brEval *target, brInstance *i) {
	target->set( (void*)( new slDrawCommandList(i->engine->world) ) );
	return EC_OK;
}

int brIDrawListFree(brEval args[], brEval *target, brInstance *i) {
	delete (slDrawCommandList*)BRPOINTER(&args[0]);
	return EC_OK;
}

int brIDrawListClear(brEval args[], brEval *target, brInstance *i) {
	slDrawCommandList *list = (slDrawCommandList*)BRPOINTER(&args[0]);
	list->clear();
	return EC_OK;
}

int brIDrawListSetCommandLimit(brEval args[], brEval *target, brInstance *i) {
	slDrawCommandList *list = (slDrawCommandList*)BRPOINTER(&args[0]);
	list->setCommandLimit(BRINT(&args[1]));
	return EC_OK;
}

int brIDrawListGetCommandCount(brEval args[], brEval *target, brInstance *i) {
	slDrawCommandList *list = (slDrawCommandList*)BRPOINTER(&args[0]);
	list->getCommandCount();
	return EC_OK;
}

int brIDrawListDrawVertex(brEval args[], brEval *target, brInstance *i) {
	slDrawCommandList *list = (slDrawCommandList*)BRPOINTER(&args[0]);

	list->addCommand( (slDrawCommand*)new slDrawCommandPoint( &BRVECTOR(&args[1]) ) );

	return EC_OK;
}

int brIDrawListSetLineStyle(brEval args[], brEval *target, brInstance *i) {
	slDrawCommandList *list = (slDrawCommandList*)BRPOINTER(&args[0]);
	char *patternString = BRSTRING(&args[1]);
	int n = strlen(patternString);
	unsigned int pattern = 0;

	if(n > 16) n = 16;

	while(n--) {
		pattern <<= 1;
		pattern |= (*(patternString++) == '-');
	}

	list->addCommand( (slDrawCommand*)new slDrawSetLineStyle(pattern) );

	return EC_OK;
}

int brIDrawListSetLineWidth(brEval args[], brEval *target, brInstance *i) {
	slDrawCommandList *list = (slDrawCommandList*)BRPOINTER(&args[0]);

	list->addCommand( (slDrawCommand*)new slDrawSetLineWidth(BRDOUBLE(&args[1]) ) );

	return EC_OK;
}

int brIDrawListEndPolygon(brEval args[], brEval *target, brInstance *i) {
	slDrawCommandList *list = (slDrawCommandList*)BRPOINTER(&args[0]);

	list->addCommand( (slDrawCommand*)new slDrawEndPolygon );

	return EC_OK;
}

int brIDrawListDrawLine(brEval args[], brEval *target, brInstance *i) {
	slDrawCommandList *list = (slDrawCommandList*)BRPOINTER(&args[0]);

	list->addCommand( (slDrawCommand*)new slDrawCommandLine( &BRVECTOR(&args[1]), &BRVECTOR(&args[2]) ) );

	return EC_OK;
}

int brIDrawListSetColor(brEval args[], brEval *target, brInstance *i) {
	slDrawCommandList *list = (slDrawCommandList*)BRPOINTER(&args[0]);

	list->addCommand( (slDrawCommand*)new slDrawCommandColor( &BRVECTOR(&args[1]), BRDOUBLE(&args[2]) ) );

	return EC_OK;
}

void breveInitDrawFunctions(brNamespace *n) {
	brNewBreveCall(n, "drawListNew", brIDrawListNew, AT_POINTER, 0);
	brNewBreveCall(n, "drawListFree", brIDrawListFree, AT_NULL, AT_POINTER, 0);
	brNewBreveCall(n, "drawListClear", brIDrawListClear, AT_NULL, AT_POINTER, 0);
	brNewBreveCall(n, "drawListSetCommandLimit", brIDrawListSetCommandLimit, AT_NULL, AT_POINTER, AT_INT, 0);
	brNewBreveCall(n, "drawListGetCommandCount", brIDrawListGetCommandCount, AT_INT, AT_POINTER, 0);
	brNewBreveCall(n, "drawListEndPolygon", brIDrawListEndPolygon, AT_NULL, AT_POINTER, 0);
	brNewBreveCall(n, "drawListDrawVertex", brIDrawListDrawVertex, AT_NULL, AT_POINTER, AT_VECTOR, 0);
	brNewBreveCall(n, "drawListDrawLine", brIDrawListDrawLine, AT_NULL, AT_POINTER, AT_VECTOR, AT_VECTOR, 0);
	brNewBreveCall(n, "drawListSetColor", brIDrawListSetColor, AT_NULL, AT_POINTER, AT_VECTOR, AT_DOUBLE, 0);
	brNewBreveCall(n, "drawListSetLineStyle", brIDrawListSetLineStyle, AT_NULL, AT_POINTER, AT_STRING, 0);
	brNewBreveCall(n, "drawListSetLineWidth", brIDrawListSetLineWidth, AT_NULL, AT_POINTER, AT_DOUBLE, 0);
}
