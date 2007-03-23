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
brPerlSetController(controller)
	SV* controller

void *
brPerlFindInternalFunction( inSelf, name )
	SV *inSelf
	char *name
			    
void *
brPerlCallInternalFunction( inSelf, inArgs )
	SV *inSelf
	void *inArgs