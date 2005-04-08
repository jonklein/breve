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

extern int gCubeDrawList;

slPatchGrid *slPatchGridNew(slVector *center, slVector *patchSize, int x, int y, int z) {
	struct slPatchGrid *grid;
	int a, b, c;

	if(x < 1 || y < 1 || z < 1) return NULL;

	grid = new slPatchGrid;

	grid->patches = new slPatch**[z];
	grid->xSize = x;
	grid->ySize = y;
	grid->zSize = z;

	grid->colors[0] = new unsigned char[x * y * z];
	grid->colors[1] = new unsigned char[x * y * z];
	grid->colors[2] = new unsigned char[x * y * z];
	grid->colors[3] = new unsigned char[x * y * z];

	slVectorCopy(patchSize, &grid->patchSize);

	grid->startPosition.x = (-(patchSize->x * x) / 2) + center->x;
	grid->startPosition.y = (-(patchSize->y * y) / 2) + center->y;
	grid->startPosition.z = (-(patchSize->z * z) / 2) + center->z;

	for(c=0;c<z;c++) {
		grid->patches[c] = new slPatch*[y];

		for(b=0;b<y;b++) {
			grid->patches[c][b] = new slPatch[x];
		
			for(a=0;a<x;a++) {
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

/*!
	\brief Copies the contents of a 3D matrix to one z-slice of a PatchGrid.
*/

void slPatchGridCopyColorFrom3DMatrix(slPatchGrid *grid, slBigMatrix3DGSL *m, int channel, double scale) {
	int x, y, z;
	int xSize, ySize, zSize;
	float* mData;

	unsigned int chemTDA = m->xDim(); 
	unsigned int chemXY = m->xDim() * m->yDim();

	mData = m->getGSLVector()->data;

	xSize = m->xDim();
	ySize = m->yDim();
	zSize = m->zDim();

	for(x = 0; x < xSize; x++ ) {
		for(y = 0; y < ySize; y++ ) {
			for(z = 0; z < zSize; z++ ) {
				grid->patches[x][y][z].color[channel] = scale * mData[ (z * chemXY) + (x * chemTDA) + y ];
			}
		}
	}
}

/*!
	\brief Copies the contents of a 2D matrix to one z-slice of a PatchGrid.
*/

void slPatchGridCopyColorFrom2DMatrix(slPatchGrid *grid, slBigMatrix2DGSL *m, int z, int channel, double scale) {
	int x, y;
	int xSize, ySize;
	float* mData;

	unsigned int chemTDA = m->xDim(); 

	mData = m->getGSLVector()->data;

	xSize = m->xDim();
	ySize = m->yDim();

	for(x = 0; x < xSize; x++ ) {
		for(y = 0; y < ySize; y++ ) {
			grid->patches[x][y][z].color[channel] = scale * mData[ (x * chemTDA) + y ];
		}
	}
}

/*!
	\brief Draws a set of patches.
*/

void slPatchGrid::draw(slCamera *camera) {
	int z, y, x;
	int zVal, yVal, xVal;
	int zMid = 0, yMid = 0, xMid = 0;
	slPatch *patch;
	slVector translation, origin;

	// we want to always draw from back to front for the 
	// alpha blending to work.  figure out the points
	// closest to the camera.

	slVectorAdd(&camera->location, &camera->target, &origin);

	xMid = (int)((origin.x - startPosition.x) / patchSize.x);
	if(xMid < 0) xMid = 0;
	if(xMid > xSize) xMid = xSize - 1;

	yMid = (int)((origin.y - startPosition.y) / patchSize.y);
	if(yMid < 0) yMid = 0;
	if(yMid > ySize) yMid = ySize - 1;

	zMid = (int)((origin.z - startPosition.z) / patchSize.z);
	if(zMid < 0) zMid = 0;
	if(zMid > zSize) zMid = zSize - 1;

	glEnable(GL_BLEND);

	glEnable(GL_CULL_FACE);

	glPushMatrix();

	for(z=0;z<zSize;z++) {
		if(z < zMid) zVal = z;
		else zVal = (zSize - 1) - (z - zMid);

		for(y=0;y<ySize;y++) {
			if(y < yMid) yVal = y;
			else yVal = (ySize - 1) - (y - yMid);

			for(x=0;x<xSize;x++) {
				if(x < xMid) xVal = x;
				else xVal = (xSize - 1) - (x - xMid);

				patch = &patches[zVal][yVal][xVal];

				if(patch->color[3] != 0.0) {
					translation.x = startPosition.x + patchSize.x * xVal;
					translation.y = startPosition.y + patchSize.y * yVal;
					translation.z = startPosition.z + patchSize.z * zVal;

					if(camera->pointInFrustum(&translation)) {
						glPushMatrix();

						glColor4fv(patch->color);

						glTranslatef(translation.x, translation.y, translation.z);

						glScalef(patchSize.x, patchSize.y, patchSize.z);

						glCallList(camera->cubeDrawList);

						glPopMatrix();
					}
				}
			}
		}
	}

	glPopMatrix();
}

/*!
	\brief Picks an object for selection based on a click at the given coordinates.
*/

void slPatchGridFree(slPatchGrid *g) {
	delete g;
}

void slPatchSetData(slPatch *p, void *data) {
	p->data = data;
}

void *slPatchGetData(slPatch *p) {
	if(!p) return NULL;
	return p->data;
}

void slPatchGetLocation(slPatch *p, slVector *location) {
	slVectorCopy(&p->location, location);
}

void slPatchSetColor(slPatch *p, slVector *color) {
	p->color[0] = color->x;
	p->color[1] = color->y;
	p->color[2] = color->z;
}

void slPatchSetTransparency(slPatch *p, double transparency) {
	p->color[3] = transparency;
}

void slPatchGetColor(slPatch *p, slVector *color) {
	color->x = p->color[0];
	color->y = p->color[1];
	color->z = p->color[2];
}

slPatch *slPatchAtIndex(slPatchGrid *grid, int x, int y, int z) {
	if(x < 0 || x >= grid->xSize) return NULL;
	if(y < 0 || y >= grid->ySize) return NULL;
	if(z < 0 || z >= grid->zSize) return NULL;
	return &grid->patches[z][y][x];
}

void *slPatchGetDataAtIndex(slPatchGrid *grid, int x, int y, int z) {
	if(x < 0 || x >= grid->xSize) return NULL;
	if(y < 0 || y >= grid->ySize) return NULL;
	if(z < 0 || z >= grid->zSize) return NULL;
	return grid->patches[z][y][x].data;
}

void slPatchSetDataAtIndex(slPatchGrid *grid, int x, int y, int z, void *data) {
	if(x < 0 || x >= grid->xSize) return;
	if(y < 0 || y >= grid->ySize) return;
	if(z < 0 || z >= grid->zSize) return;
	grid->patches[z][y][x].data = data;
}
