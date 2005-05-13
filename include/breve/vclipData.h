#include "vclip.h"

void slAddBoundingBoxForVectors(slVclipData *, int , slVector *, slVector *);
void slVclipDataInit(slWorld *); 
void slVclipDataRealloc(slVclipData *, unsigned int);
slPairFlags slVclipDataInitPairFlags(slVclipData *, int, int);
