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

#ifndef _KERNEL_H
#define _KERNEL_H


/*!
	\mainpage The breve Simulation Environment: source documentation

	This is the documentation for the source code of the breve 
	simulation environment.

	<HR>

	For those interested in implementing alternative language frontends
	for breve, so that simulations can be written in languages beside 
	steve, reading \ref breveObjectAPI is the first step.

	Once a language frontend has been written, the user may either link
	their new language frontend against an existing application frontend, 
	which is described in \ref breveFrontendAPI, or may construct
	a totally novel application frontend, which uses the API documented 
	in \ref breveEngineAPI.

	Users wishing to use breve simulations as part of other applications
	should also refer to \ref breveEngineAPI.

	When a new breve object frontend has been implemented, the standard
	breve class files will need to be ported to the new language.  Working
	from the standard steve-language class files, this is a rather 
	straightforward process, but the simulation API is documented in
	further detail in \ref InternalFunctions.
*/

#include "util.h"

#ifdef MINGW
#include <winsock2.h>
#endif

#include <stdio.h>
#include <stdlib.h>
#include <strings.h>
#include <string.h>
#include <pthread.h>
#include <inttypes.h>
#include <sys/stat.h>
#include <sys/param.h>
#include <sys/types.h>

#include "timeval.h"

#include <fcntl.h>

#include "simulation.h"

#include "kernelTypedefs.h"

#include "internal.h"

#include "engine.h"

#include "breveObjectAPI.h"
#include "frontendAPI.h"

#include "breveEval.h"

#include "multithread.h"
#include "namespace.h"

#include "evalData.h"
#include "evalList.h"
#include "evalHash.h"

#include "callback.h"

#include "plugins.h"

#include "color.h"

#include "sound.h"

#include "network.h"

#include "interface.h"

#endif /* _KERNEL_H */
