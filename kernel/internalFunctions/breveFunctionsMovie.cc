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

/*! \addtogroup InternalFunctions */
/*@{*/

#ifdef HAVE_LIBAVCODEC
int breveMovieCreate(brEval args[], brEval *result, brInstance *i) {
	slCamera *camera = i->engine->camera;
	slMovie *movie;
	char *path;

	path = brOutputPath(i->engine, BRSTRING(&args[0]));

	movie = slMovieCreate(path, camera->x, camera->y, 30, 1.0);
	slFree(path);

	BRPOINTER(result) = movie;

	return EC_OK;
}

int breveMovieAddWorldFrame(brEval args[], brEval *result, brInstance *i) {
	slMovie *movie = BRPOINTER(&args[0]);

	BRINT(result) = slMovieAddWorldFrame(movie, i->engine->world, i->engine->camera);

	return EC_OK;
}

int breveMovieClose(brEval args[], brEval *result, brInstance *i) {
	slMovie *movie = BRPOINTER(&args[0]);

	if(!movie) {
		slMessage(DEBUG_ALL, "warning: attempt to close uninitialized movie pointer\n");
		return EC_OK;
	}

	slMovieFinish(movie);

	return EC_OK;

}
#endif /* HAVE_LIBAVCODEC */

#ifdef HAVE_LIBPNG
int breveSnapshot(brEval args[], brEval *result, brInstance *i) {
	// this doesn't really belong here
	char *path;
	slCamera *c = i->engine->camera;

	path = brOutputPath(i->engine, BRSTRING(&args[0]));

	BRINT(result) = slPNGSnapshot(i->engine->world, c, path);
	slFree(path);

	return EC_OK;
}
#endif /* HAVE_LIBPNG */

/*!
	\brief Called if this version of breve is built without movie export support.

	Prints an error message.
*/

int breveMovieUnsupported(brEval args[], brEval *result, brInstance *i) {
    slMessage(DEBUG_ALL, "This version of breve was built without support for movie export\n");
	return EC_ERROR;
}

/*!
	\brief Called if this version of breve is built without image export support.

	Prints an error message.
*/

int breveSnapshotUnsupported(brEval args[], brEval *result, brInstance *i) {
    slMessage(DEBUG_ALL, "This version of breve was built without support for image export\n");
	return EC_ERROR;
}
/*@}*/

void breveInitMovieFunctions(brNamespace *n) {
#ifdef HAVE_LIBAVCODEC
	brNewBreveCall(n, "movieCreate", breveMovieCreate, AT_POINTER, AT_STRING, 0);
	brNewBreveCall(n, "movieAddWorldFrame", breveMovieAddWorldFrame, AT_INT, AT_POINTER, 0);
	brNewBreveCall(n, "movieClose", breveMovieClose, AT_INT, AT_POINTER, 0);
#else 
	brNewBreveCall(n, "movieCreate", breveMovieUnsupported, AT_POINTER, AT_STRING, 0);
	brNewBreveCall(n, "movieAddWorldFrame", breveMovieUnsupported, AT_INT, AT_POINTER, 0);
	brNewBreveCall(n, "movieClose", breveMovieUnsupported, AT_INT, AT_POINTER, 0);
#endif

#ifdef HAVE_LIBPNG
	brNewBreveCall(n, "snapshot", breveSnapshot, AT_INT, AT_STRING, 0);
#else
	brNewBreveCall(n, "snapshot", breveSnapshotUnsupported, AT_INT, AT_STRING, 0);
#endif
}
