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

/*!
	\brief Data associated with a certain region of 3D space.
*/

struct slPatch {
	void *data;
	float transparency;
	slVector color;
	slVector location;
};

typedef struct slPatch slPatch;

/*!
	\brief A grid of \ref slPatch objects.
*/

struct slPatchGrid {
	int xSize, ySize, zSize;

	slVector startPosition;
	slVector patchSize;

	slPatch ***patches;
};

typedef struct slPatchGrid slPatchGrid;

slPatchGrid *slPatchGridNew(slVector *center, slVector *size, int x, int y, int z);
void slPatchGridFree(slPatchGrid *g);
void slInitPatch(slPatch *p);
void slPatchSetData(slPatch *p, void *data);

slPatch *slPatchForLocation(slPatchGrid *g, slVector *location);
