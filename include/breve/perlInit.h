#ifndef _PERLINIT_H
#define _PERLINIT_H
#ifdef HAVE_LIBPERL


#include "EXTERN.h"
#include "perl.h"

#define PERL_TYPE_SIGNATURE	0xaabbccdd
extern PerlInterpreter *my_perl;
void brPerlInit( brEngine *breveEngine );


#endif /* HAVE_LIBPERL */
#endif /* _PERLINIT_H */
