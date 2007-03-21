#ifndef _PERL_INTERFACE_H
#define _PERL_INTERFACE_H

// can't use BREngine* and BRInstance* right now because they
// aren't in the typemap file
// int brEngineSetController(BREngine* inEngine, BRInstance* inInstance);
void *brPerlSetController( void *inSelf, void *inArgs );

void blahblah();

#endif
