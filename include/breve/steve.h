
#define STINSTANCE(x)	((stInstance*)(BRPOINTER(x)))

#include "util.h"
#include "kernel.h"

#ifdef __cplusplus
extern "C" {
#endif

#include "steveTypedefs.h"

#include "object.h"

#include "gc.h"

#include "expression.h"
#include "evaluation.h"

#include "stringexp.h"
#include "dataObject.h"
#include "genetic.h"

#include "steveFrontend.h"
#include "interactive.h"

#include "evalListSort.h"
#include "evalListCopyGC.h"

#include "steveparse.h"

#ifdef __cplusplus
}
#endif
