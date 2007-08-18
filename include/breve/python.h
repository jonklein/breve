
#include <signal.h>

#include "kernel.h"

#ifdef HAVE_LIBPYTHON2_4
#include <python2.4/Python.h>
#else
#include <python2.4/Python.h>
#endif

#define PYTHON_TYPE_SIGNATURE	0x7079746f

void brPythonInit( brEngine *breveEngine );
