#include "kernelTypedefs.h"
#include "breveEval.h"
#include "engine.h"
#include "breveObjectAPI.h"
#include "namespace.h"
#include <stdio.h>
#include <stdarg.h>

#ifdef __cplusplus
extern "C" {
#endif
void breveInitCameraFunctions(brNamespace *n);
void breveInitJointFunctions(brNamespace *n);
void breveInitControlFunctions(brNamespace *n);
void breveInitFileFunctions(brNamespace *n);
void breveInitGraphFunctions(brNamespace *n);
void breveInitImageFunctions(brNamespace *n);
void breveInitLinkFunctions(brNamespace *n);
void breveInitMathFunctions(brNamespace *n);
void breveInitMenuFunctions(brNamespace *n);
void breveInitMovieFunctions(brNamespace *n);
void breveInitMultibodyFunctions(brNamespace *n);
void breveInitNetworkFunctions(brNamespace *n);
void breveInitNeuralFunctions(brNamespace *n);
void breveInitObjectFunctions(brNamespace *n);
void breveInitPatchFunctions(brNamespace *n);
void breveInitPhysicsFunctions(brNamespace *n);
void breveInitShapeFunctions(brNamespace *n);
void breveInitStationaryFunctions(brNamespace *n);
void breveInitSoundFunctions(brNamespace *n);
void breveInitTerrainFunctions(brNamespace *n);
void breveInitWorldFunctions(brNamespace *n);
void breveInitPushFunctions(brNamespace *n);
void breveInitPushCallbackFunctions(brNamespace *n);
void breveInitSpringFunctions(brNamespace *n);
void breveInitMatrixFunctions(brNamespace *n);
void breveInitQGAMEFunctions(brNamespace *n);

int brNewBreveCall(brNamespace *n, char *name, int (*call)(brEval *a, brEval *r, brInstance *i), int rtype, ...);

FILE *slGetLogFilePointer(brInstance *i);
#ifdef __cplusplus
}
#endif

void brLoadInternalFunctions(brEngine *e);
void brFreeBreveCall(void *d);


/*!
	\brief An internal function provided by breve.
*/

#define ST_CMAX_ARGS 16

struct brInternalFunction {
	unsigned char nargs;
    
	unsigned char argtypes[ST_CMAX_ARGS];
	unsigned char rtype;
    
	int (*call)(brEval *args, brEval *result, brInstance *i);
    
	char *name;
};
