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

	grid->drawSmooth = 0;

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
	\brief Draws a set of patches using a volumetric 3D texture.
*/

void slPatchGrid::draw(slCamera *camera) {
	slVector origin, diff, adiff, size;

	slVectorAdd(&camera->location, &camera->target, &origin);

	diff.x = (origin.x - (startPosition.x + (xSize / 2) * patchSize.x));
	diff.y = (origin.y - (startPosition.y + (ySize / 2) * patchSize.y));
	diff.z = (origin.z - (startPosition.z + (zSize / 2) * patchSize.z));

	adiff.x = fabs(diff.x);
	adiff.y = fabs(diff.y);
	adiff.z = fabs(diff.z);

	glEnable(GL_TEXTURE_3D);

	glDisable(GL_CULL_FACE);

	glColor4f(1,1,1,1);

	if(drawSmooth) {
		glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	} else {
		glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	}

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

	if(adiff.x > adiff.z && adiff.x > adiff.y) 
		textureDrawXPass(size, diff.x > 0);
	else if(adiff.y > adiff.z) 
		textureDrawYPass(size, diff.y > 0);
	else 
		textureDrawZPass(size, diff.z > 0);

	glEnd();

	glDisable(GL_TEXTURE_3D);

	return;
}

void slPatchGrid::textureDrawYPass(slVector &size, int dir) {
	float y, start, end, inc;

	if(dir) {
		start = 0;
		end = ySize;
		inc = .1;
	} else {
		start = ySize;
		inc = -.1;
		end = -1;
	}

	for(y=start;floor(y)!=end;y+=inc) {
		double yp;

		yp = startPosition.y + (y*patchSize.y);

		glTexCoord3f(0, (y / textureY), 0);
		glVertex3f(startPosition.x, yp, startPosition.z);

		glTexCoord3f((float)xSize / textureX, (y / textureY), 0);
		glVertex3f(startPosition.x + size.x, yp, startPosition.z);

		glTexCoord3f((float)xSize / textureX, (y / textureY), (float)zSize / textureZ);
		glVertex3f(startPosition.x + size.x, yp, startPosition.z + size.z);

		glTexCoord3f(0, (y / textureY), (float)zSize / textureZ);
		glVertex3f(startPosition.x, yp, startPosition.z + size.z);
	}
}

void slPatchGrid::textureDrawXPass(slVector &size, int dir) {
	float x, start, end, inc;

	if(dir) {
		start = 0;
		end = xSize;
		inc = .1;
	} else {
		start = xSize;
		inc = -.1;
		end = -1;
	}

	for(x=start;floor(x)!=end;x+=inc) {
		double xp;

		xp = startPosition.x + (x*patchSize.x);

		glTexCoord3f((x / textureX), 0, 0);
		glVertex3f(xp, startPosition.y, startPosition.z);

		glTexCoord3f((x / textureX), (float)ySize / textureY, 0);
		glVertex3f(xp, startPosition.y + size.y, startPosition.z);

		glTexCoord3f((x / textureX), (float)ySize / textureY, (float)zSize / textureZ);
		glVertex3f(xp, startPosition.y + size.y, startPosition.z + size.z);

		glTexCoord3f((x / textureX), 0, (float)zSize / textureZ);
		glVertex3f(xp, startPosition.y, startPosition.z + size.z);
	}
}

void slPatchGrid::textureDrawZPass(slVector &size, int dir) {
	float z, start, end, inc;

	if(dir) {
		start = 0;
		inc = .1;
		end = zSize;
	} else {
		inc = -.1;
		start = zSize - inc;
		end = -1;
	}

	for(z=start;floor(z)!=end;z+=inc) {
		double zp;

		zp = startPosition.z + (z*patchSize.z);

		glTexCoord3f(0, 0, (z / textureZ));
		glVertex3f(startPosition.x, startPosition.y, zp);

		glTexCoord3f((float)xSize / textureX, 0, (z / textureZ));
		glVertex3f(startPosition.x + size.x, startPosition.y, zp);

		glTexCoord3f((float)xSize / textureX, (float)ySize / textureY, (z / textureZ));
		glVertex3f(startPosition.x + size.x, startPosition.y + size.y, zp);

		glTexCoord3f(0, (float)ySize / textureY, (z / textureZ));
		glVertex3f(startPosition.x, startPosition.y + size.y, zp);
	}
}

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
	if (color->x > 1.0) color->x = 1.0;
	else if(color->x < 0.0) color->x = 0.0;

	if (color->y > 1.0) color->y = 1.0;
	else if(color->x < 0.0) color->x = 0.0;

	if (color->z > 1.0) color->z = 1.0;
	else if(color->x < 0.0) color->x = 0.0;

	p->grid->colors[p->colorOffset    ] = (unsigned char)(255 * color->x);
	p->grid->colors[p->colorOffset + 1] = (unsigned char)(255 * color->y);
	p->grid->colors[p->colorOffset + 2] = (unsigned char)(255 * color->z);
}

void slPatchSetTransparency(slPatch *p, double transparency) {
	if (transparency > 1.0) transparency = 1.0;
	else if(transparency < 0.0) transparency = 0.0;

	p->grid->colors[p->colorOffset + 3] = (unsigned char)(255 * transparency);
}

void slPatchGetColor(slPatch *p, slVector *color) {
	color->x = p->grid->colors[p->colorOffset    ] / 255.0;
	color->y = p->grid->colors[p->colorOffset + 1] / 255.0;
	color->z = p->grid->colors[p->colorOffset + 2] / 255.0;
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
