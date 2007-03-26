#ifdef __cplusplus
extern "C" {
#endif
#include "EXTERN.h"
#include "perl.h"
#include "XSUB.h"
#include "ppport.h"
#ifdef __cplusplus
}
#endif

#include "perlInterface.h"

MODULE = Breve		PACKAGE = Breve	

void *
brPerlAddInstance(instance)
	SV* instance

void *
brPerlSetController(controller)
	void* controller

void *
brPerlFindInternalFunction( name )
	char *name
			    
void
brPerlCallInternalFunction( inFunc, caller, argCount, inArgs )
	void *inFunc
	void *caller
	int argCount
	AV *inArgs

   PPCODE:
   brPerlCallInternalFunction(inFunc, caller, argCount, inArgs);
