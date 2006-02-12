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

#include "kernel.h"
#include "movie.h"

/*! \addtogroup InternalFunctions */
/*@{*/

#if HAVE_LIBAVFORMAT
#define BRMOVIEPOINTER(p)  ((slMovie*)BRPOINTER(p))
#endif

int breveMovieCreate(brEval args[], brEval *result, brInstance *i) {
#if HAVE_LIBAVFORMAT
	char *path;
	slMovie *movie;
	slCamera *camera = i->engine->camera;

	path = brOutputPath(i->engine, BRSTRING(&args[0]));

	movie = slMovieCreate( path, camera->_width, camera->_height );
	slFree(path);

	result->set( movie );

	return EC_OK;
#else
	slMessage(DEBUG_ALL, "This version of breve was built without support for movie export\n");

	return EC_ERROR;
#endif
}

int breveMovieAddWorldFrame(brEval args[], brEval *result, brInstance *i) {
#if HAVE_LIBAVFORMAT
	slMovie *movie = BRMOVIEPOINTER(&args[0]);

	if (!movie)
		slMessage(DEBUG_ALL, "warning: attempt to add frame to null movie pointer\n");
	else
		result->set( slMovieAddWorldFrame(movie, i->engine->world, i->engine->camera) );
#endif

	return EC_OK;
}

int breveMovieClose(brEval args[], brEval *result, brInstance *i) {
#if HAVE_LIBAVFORMAT
	slMovie *movie = BRMOVIEPOINTER(&args[0]);

	if (!movie)
		slMessage(DEBUG_ALL, "warning: attempt to close null movie pointer\n");
	else
		slMovieFinish(movie);
#endif

	return EC_OK;

}

void breveInitMovieFunctions(brNamespace *n) {
	brNewBreveCall(n, "movieCreate", breveMovieCreate, AT_POINTER, AT_STRING, 0);
	brNewBreveCall(n, "movieAddWorldFrame", breveMovieAddWorldFrame, AT_INT, AT_POINTER, 0);
	brNewBreveCall(n, "movieClose", breveMovieClose, AT_INT, AT_POINTER, 0);
}
