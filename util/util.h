/*****************************************************************************
 *                                                                           *
 * The breve Simulation Environment                                          *
 * Copyright (C) 2000, 2001, 2002, 2003 Jonathan Klein                       *
 *                                                                           *
 * This program is free software; you can redistribute it and/or modify      *
 * it under the terms of the GNU General Public License as published by      *
 * the Free Software Foundation; either version 2 of the License, or         *
 * (at your option) any later version.                                       *
 *                                                                           *
 * This program is distributed in the hope that it will be useful,           *
 * but WITHOUT ANY WARRANTY; without even the implied warranty of            *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the             *
 * GNU General Public License for more details.                              *
 *                                                                           *
 * You should have received a copy of the GNU General Public License         *
 * along with this program; if not, write to the Free Software               *
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA *
 *****************************************************************************/

/* i hate to have to do this, but we want to avoid */
/* things from getting included more than once     */

#ifndef SL_UTIL_INCLUDE
#define SL_UTIL_INCLUDE

#include "config.h"

#ifdef MINGW

#define usleep(x) _sleep((x) / 1000.0)
#define bzero(p, l) memset((p), 0, (l))
#define bcopy(x, y, z) memcpy((y), (x), z)
#define random rand
#define srandom srand

#endif /* MINGW */

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <strings.h>

#include <sys/types.h>
#include <unistd.h>
#include <sys/stat.h>

#include <zlib.h>

#include "utilTypedefs.h"

#include "list.h"
#include "stack.h"
#include "hash.h"
#include "memory.h"
#include "text.h"

#include "vector.h"
#include "matrix.h"
#include "quat.h"

#include "nr.h"
#include "error.h"

#include "stringstream.h"

#endif
