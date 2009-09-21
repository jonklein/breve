#include <stdio.h>

#include <string>

#include "kernelTypedefs.h"

#include "breveEval.h"
#include "breveObjectAPI.h"
#include "engine.h"
#include "namespace.h"

void breveInitURLFunctions(brNamespace *);
void breveInitCameraFunctions(brNamespace *);
void breveInitJointFunctions(brNamespace *);
void breveInitControlFunctions(brNamespace *);
void breveInitFileFunctions(brNamespace *);
void breveInitGraphFunctions(brNamespace *);
void breveInitImageFunctions(brNamespace *);
void breveInitLinkFunctions(brNamespace *);
void breveInitRealFunctions(brNamespace *);
void breveInitMathFunctions(brNamespace *);
void breveInitMenuFunctions(brNamespace *);
void breveInitMovieFunctions(brNamespace *);
void breveInitMultibodyFunctions(brNamespace *);
void breveInitNetworkFunctions(brNamespace *);
void breveInitNeuralFunctions(brNamespace *);
void breveInitNeuralNetworkFunctions(brNamespace *);
void breveInitObjectFunctions(brNamespace *);
void breveInitPatchFunctions(brNamespace *);
void breveInitPhysicsFunctions(brNamespace *);
void breveInitShapeFunctions(brNamespace *);
void breveInitStationaryFunctions(brNamespace *);
void breveInitSoundFunctions(brNamespace *);
void breveInitTerrainFunctions(brNamespace *);
void breveInitWorldFunctions(brNamespace *);
void breveInitPushFunctions(brNamespace *);
void breveInitPushCallbackFunctions(brNamespace *);
void breveInitSpringFunctions(brNamespace *);
void breveInitMatrixFunctions(brNamespace *);
void breveInitVectorFunctions(brNamespace *);
void breveInitStatisticsFunctions(brNamespace *n);
void breveInitQGAMEFunctions(brNamespace *);
void breveInitDrawFunctions(brNamespace *);

#define BRBREVECALL( n, name, rvalue, ... )		brNewBreveCall( n, "" #name "", name, rvalue, __VA_ARGS__, 0 ) 
DLLEXPORT int brNewBreveCall(brNamespace *, const char *, int (*)(brEval *, brEval *, brInstance *), int, ...);
DLLEXPORT FILE *slGetLogFilePointer(brInstance *);

void breveInitPythonFunctions(brNamespace *);
void breveInitPerlFunctions(brNamespace *);

void brLoadInternalFunctions(brEngine *);
void brFreeBreveCall(void *);


/**
 * \brief An internal function provided by breve.
 */

#define ST_CMAX_ARGS 16

struct brInternalFunction {
	unsigned char _argCount;
    
	unsigned char _argTypes[ ST_CMAX_ARGS ];
	unsigned char _returnType;
    
	int (*_call)(brEval *, brEval *, brInstance *);
    
	std::string _name;
};
