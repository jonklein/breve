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

void* brPerlSetController(alpha, beta)
    void* alpha
    void* beta

void testingfoo()
	CODE:
		printf("testingfoo called.\n");

void blahblah()

BOOT:
	printf("Hello from the bootstrap!\n");


