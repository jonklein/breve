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

	glGenTextures(1, &grid->texture);

	grid->patches = new slPatch**[z];
	grid->xSize = x;
	grid->ySize = y;
	grid->zSize = z;

	grid->textureX = slNextPowerOfTwo(x);
	grid->textureY = slNextPowerOfTwo(y);
	grid->textureZ = slNextPowerOfTwo(z);

	grid->colors = new unsigned char[grid->textureX * grid->textureY * grid->textureZ * 4];

	memset(grid->colors, 0, grid->textureX * grid->textureY * grid->textureZ * 4);

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
				grid->patches[c][b][a].grid = grid;
				grid->patches[c][b][a].colorOffset = (c * grid->textureX * grid->textureY * 4) + (b * grid->textureX * 4) + (a * 4);
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
				grid->colors[(z * grid->textureX * grid->textureY * 4) + (y * grid->textureX * 4) + (x * 4) + channel] = 
					(unsigned char)(255 * scale * mData[ (z * chemXY) + (x * chemTDA) + y ]);

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
				grid->colors[(z * grid->textureX * grid->textureY * 4) + (y * grid->textureX * 4) + (x * 4) + channel] = 
					(unsigned char)(255 * scale * mData[ (x * chemTDA) + y ]);
		}
	}
}

void slPatchGrid::drawWith3DTexture(slCamera *camera) {
	slVector origin, diff, size;

	slVectorAdd(&camera->location, &camera->target, &origin);

	diff.x = abs(origin.x - (startPosition.x + (xSize / 2) * patchSize.x));
	diff.y = abs(origin.y - (startPosition.y + (ySize / 2) * patchSize.y));
	diff.z = abs(origin.z - (startPosition.z + (zSize / 2) * patchSize.z));

	glEnable(GL_TEXTURE_3D);

	glDisable(GL_CULL_FACE);

	glColor4f(1,1,1,1);

	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_CLAMP);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_CLAMP);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, GL_CLAMP);

	glBindTexture(GL_TEXTURE_3D, texture);
	glTexImage3D(GL_TEXTURE_3D, 0, GL_RGBA, textureX, textureY, textureZ, 0, GL_RGBA, GL_UNSIGNED_BYTE, colors);

	glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);

	glBegin(GL_QUADS);

	size.x = patchSize.x * xSize;
	size.y = patchSize.y * ySize;
	size.z = patchSize.y * zSize;

	// can this crap possible be right?

	if(diff.x > diff.z) {
		if(diff.y > diff.x) {
			textureDrawYPass(size, 0);
			textureDrawXPass(size, 0);
			textureDrawZPass(size, 0);
		} else if(diff.y < diff.z) {
			textureDrawXPass(size, 0);
			textureDrawZPass(size, 0);
			textureDrawYPass(size, 0);
		} else {
			textureDrawXPass(size, 0);
			textureDrawZPass(size, 0);
			textureDrawYPass(size, 0);
		}
	} else if(diff.y > diff.z) {
		textureDrawYPass(size, 0);
		textureDrawZPass(size, 0);
		textureDrawXPass(size, 0);
	} else if(diff.x > diff.y) {
		textureDrawZPass(size, 0);
		textureDrawXPass(size, 0);
		textureDrawYPass(size, 0);
	} else {
		textureDrawZPass(size, 0);
		textureDrawYPass(size, 0);
		textureDrawXPass(size, 0);
	}
	

	glEnd();

	glDisable(GL_TEXTURE_3D);

	return;
}

void slPatchGrid::textureDrawYPass(slVector &size, int dir) {
	int y;

	for(y=0;y<ySize + 1;y++) {
		double yp;

		yp = startPosition.y + (y*patchSize.y);
		// yp = startPosition.y + ((ySize - y)*patchSize.y);

		glTexCoord3f(0, ((double)y / textureY), 0);
		glVertex3f(startPosition.x, yp, startPosition.z);

		glTexCoord3f(xSize / textureX, ((double)y / textureY), 0);
		glVertex3f(startPosition.x + size.x, yp, startPosition.z);

		glTexCoord3f(xSize / textureX, ((double)y / textureY), zSize / textureZ);
		glVertex3f(startPosition.x + size.x, yp, startPosition.z + size.z);

		glTexCoord3f(0, ((double)y / textureY), zSize / textureZ);
		glVertex3f(startPosition.x, yp, startPosition.z + size.z);
	}
}

void slPatchGrid::textureDrawXPass(slVector &size, int dir) {
	int x;

	for(x=0;x<xSize + 1;x++) {
		double xp;

		xp = startPosition.x + (x*patchSize.x);
		// xp = startPosition.x + ((xSize - x)*patchSize.x);

		glTexCoord3f(((double)x / textureX), 0, 0);
		glVertex3f(xp, startPosition.y, startPosition.z);

		glTexCoord3f(((double)x / textureX), ySize / textureY, 0);
		glVertex3f(xp, startPosition.y + size.y, startPosition.z);

		glTexCoord3f(((double)x / textureX), ySize / textureY, zSize / textureZ);
		glVertex3f(xp, startPosition.y + size.y, startPosition.z + size.z);

		glTexCoord3f(((double)x / textureX), 0, zSize / textureZ);
		glVertex3f(xp, startPosition.y, startPosition.z + size.z);
	}
}

void slPatchGrid::textureDrawZPass(slVector &size, int dir) {
	int z;

	for(z=0;z<zSize + 1;z++) {
		double zp;

		zp = startPosition.z + (z*patchSize.z);
		// if(diff.z > 0) zp = startPosition.z + ((zSize - z)*patchSize.z);

		glTexCoord3f(0, 0, ((double)z / textureZ));
		glVertex3f(startPosition.x, startPosition.y, zp);

		glTexCoord3f(xSize / textureX, 0, ((double)z / textureZ));
		glVertex3f(startPosition.x + size.x, startPosition.y, zp);

		glTexCoord3f(xSize / textureX, ySize / textureY, ((double)z / textureZ));
		glVertex3f(startPosition.x + size.x, startPosition.y + size.y, zp);

		glTexCoord3f(0, ySize / textureY, ((double)z / textureZ));
		glVertex3f(startPosition.x, startPosition.y + size.y, zp);
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

	// return drawWith3DTexture(camera);

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

				if(colors[patch->colorOffset + 3] != 0) {
					translation.x = startPosition.x + patchSize.x * xVal;
					translation.y = startPosition.y + patchSize.y * yVal;
					translation.z = startPosition.z + patchSize.z * zVal;

					if(camera->pointInFrustum(&translation)) {
						glPushMatrix();

						glColor4ubv(&colors[patch->colorOffset]);

						glTranslatef(translation.x, translation.y, translation.z);

						glScalef(patchSize.x, patchSize.y, patchSize.z);

						glCallList(camera->cubeDrawList);

						glPopMatrix();
					}
				}
			}
		}
	}
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
	p->grid->colors[p->colorOffset    ] = (unsigned char)(256 * color->x);
	p->grid->colors[p->colorOffset + 1] = (unsigned char)(256 * color->y);
	p->grid->colors[p->colorOffset + 2] = (unsigned char)(256 * color->z);
}

void slPatchSetTransparency(slPatch *p, double transparency) {
	p->grid->colors[p->colorOffset + 3] = (unsigned char)(256 * transparency);
}

void slPatchGetColor(slPatch *p, slVector *color) {
	color->x = p->grid->colors[p->colorOffset    ] / 256.0;
	color->y = p->grid->colors[p->colorOffset + 1] / 256.0;
	color->z = p->grid->colors[p->colorOffset + 2] / 256.0;
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
