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

#include "util.h"

#ifdef MINGW
#include <winsock2.h>
#endif

#include <stdio.h>
#include <stdlib.h>
#include <strings.h>
#include <string.h>
#include <pthread.h>
#include <sys/stat.h>
#include <sys/param.h>
#include <sys/types.h>

#ifndef WINDOWS
#include <sys/time.h>
#endif 

#include <fcntl.h>

#include "simulation.h"
#include "graph.h"

#include "kernelTypedefs.h"

#include "engine.h"

#include "breveObjectAPI.h"
#include "breveEval.h"

#include "multithread.h"
#include "namespace.h"

#include "evalData.h"
#include "evalList.h"
#include "evalHash.h"

#include "evalListCopy.h"

#include "internal.h"
#include "callback.h"

#include "plugins.h"

#include "color.h"

#include "sound.h"

#include "network.h"

#include "format.h"
#include "interface.h"

#include "frontend.h"

int yyparse(void);
char *slFormatText(char *text);
