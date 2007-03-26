#ifndef _PERL_INTERFACE_H
#define _PERL_INTERFACE_H

#include "kernel.h"
#include "perlInit.h"

extern brEngine         *breveEngine;
extern brObjectType     *brevePerlType;

extern brPerlTypeFromEval( const brEval *inEval, SV ***prevStackPtr );

/////////////

void *brPerlSetController( brInstance* controller );

brInternalFunction *brPerlFindInternalFunction( char *name );

void brPerlCallInternalFunction( brInternalFunction *inFunc,
								brInstance *caller, int argCount, AV* inArgs);

brInstance *brPerlAddInstance( SV* instance );

#endif
