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

#undef slMalloc
#undef slRealloc
#undef slFree
#undef slStrdup

/*!
    memory.cc defines a set of malloc wrappers
*/

#ifdef __cplusplus
extern "C" {
#endif

DLLEXPORT void *
slMalloc(size_t n) {
	return calloc(1, n);
}

DLLEXPORT void *
slRealloc(void *p, size_t n) {
	return realloc(p, n);
}

DLLEXPORT void
slFree(void *p) {
	free(p);
}

DLLEXPORT char *
slStrdup(const char *s) {
#if HAVE_STRDUP
	return strdup(s);
#else
	if (!s)
		return s;

	char *t = malloc(strlen(s));
	strcpy(t, s);
	return t;
#endif
}

#ifdef __cplusplus
}
#endif
