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

#include "simulation.h"

slPatchGrid *slPatchGridNew(slVector *center, slVector *patchSize, int x, int y, int z) {
	struct slPatchGrid *grid;
	int a, b, c;

	if(x < 1 || y < 1 || z < 1) return NULL;

	grid = new slPatchGrid;

	grid->patches = new slPatch**[z];
	grid->xSize = x;
	grid->ySize = y;
	grid->zSize = z;

	slVectorCopy(patchSize, &grid->patchSize);

	grid->startPosition.x = (-(patchSize->x * x) / 2) + center->x;
	grid->startPosition.y = (-(patchSize->y * y) / 2) + center->y;
	grid->startPosition.z = (-(patchSize->z * z) / 2) + center->z;

	for(c=0;c<z;c++) {
		grid->patches[c] = new slPatch*[y];

		for(b=0;b<y;b++) {
			grid->patches[c][b] = new slPatch[x];
		
			for(a=0;a<x;a++) {
				slInitPatch(&grid->patches[c][b][a]);
				grid->patches[c][b][a].location.x = grid->startPosition.x + a * patchSize->x;
				grid->patches[c][b][a].location.y = grid->startPosition.y + b * patchSize->y;
				grid->patches[c][b][a].location.z = grid->startPosition.z + c * patchSize->z;
			}
		}
	}

	return grid;
}

slPatch *slPatchForLocation(slPatchGrid *g, slVector *location) {
	int x, y, z;
	x = (int)((location->x - g->startPosition.x) / g->patchSize.x);
	y = (int)((location->y - g->startPosition.y) / g->patchSize.y);
	z = (int)((location->z - g->startPosition.z) / g->patchSize.z);

	if(x < 0 || x >= g->xSize) return NULL;
	if(y < 0 || y >= g->ySize) return NULL;
	if(z < 0 || z >= g->zSize) return NULL;

	return &g->patches[z][y][x];
}

void slPatchGridFree(slPatchGrid *g) {
	int a, b, c;

	for(c=0;c<g->zSize;c++) {
		for(b=0;b<g->ySize;b++) {
			for(a=0;a<g->xSize;a++) {

			}

			delete[] g->patches[c][b];
		}

		delete[] g->patches[c];
	}

	delete[] g->patches;

	delete g;
}

void slInitPatch(slPatch *p) {
	p->transparency = 1.0;
	p->data = NULL;
}

void slPatchSetData(slPatch *p, void *data) {
	p->data = data;
}
