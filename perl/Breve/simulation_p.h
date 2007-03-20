
#ifndef _SIMULATION_H
#define _SIMULATION_H

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <math.h>
#include <stdlib.h>
#include <float.h>
#include <limits.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <errno.h>

// M_PI not defined on some platforms?  bastards.

#ifndef M_PI
#define M_PI 3.14159265
#endif

// do not change the order of the includes--they depend on eachother

#include "util.h"
#include "simulationTypedefs.h"

#endif /* _SIMULATION_H */
