#ifndef _PERL_INTERFACE_H
#define _PERL_INTERFACE_H

#include "kernel.h"
#include "perlInit.h"

extern brEngine         *breveEngine;
extern brObjectType     *brevePerlType;

brInstance *brPerlAddInstance( SV* instance );
void *brPerlSetController( SV* controller );

brInternalFunction *brPerlFindInternalFunction( SV *inSelf, char *name );
void *brPerlCallInternalFunction( SV *inSelf, void *inArgs );


#endif
