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
#include "glIncludes.h"

/* ********
 *  TODO: 
 *      change to row-major
 *      to c++
 */


#ifdef WINDOWS
#include <windows.h>

PFNGLTEXIMAGE3DPROC wglTexImage3D;
#endif

/**
 *  slPatch default constructor.
 */
slPatch::slPatch()
    :   data(NULL) // NULL is deprecated in c++
{
    // your code here ;-)
}

/**
 *  slPatch base constructor.
 *
 * 
 *  @param theGrid the slPatchGrid parent object.
 */
slPatch::slPatch(slPatchGrid* theGrid)
    :   grid(theGrid)
{
    // your code here ;-)
}

slPatch::slPatch(   slPatchGrid* theGrid,
                    slVector* theLocation,
                    const int theColorOffset)
    :   colorOffset(theColorOffset),
        grid(theGrid)
{
    slVectorCopy(theLocation, &this->location);
}

/**
 *    setColor sets the color of the patch to color
 *
 */
void slPatch::setColor(slVector *color) {
	if (color->x > 1.0) color->x = 1.0;
	else if(color->x < 0.0) color->x = 0.0;

	if (color->y > 1.0) color->y = 1.0;
	else if(color->x < 0.0) color->x = 0.0;

	if (color->z > 1.0) color->z = 1.0;
	else if(color->x < 0.0) color->x = 0.0;

	grid->colors[this->colorOffset    ] = (unsigned char)(255 * color->x);
	grid->colors[this->colorOffset + 1] = (unsigned char)(255 * color->y);
	grid->colors[this->colorOffset + 2] = (unsigned char)(255 * color->z);
}

/**
 *    setTransparence sets the transparency of the
 *    patch to transparency
 *
 */
void slPatch::setTransparency(double transparency) {
	if (transparency > 1.0) transparency = 1.0;
	else if(transparency < 0.0) transparency = 0.0;

	grid->colors[this->colorOffset + 3] = (unsigned char)(255 * transparency);
}

/**
 *    getColor gets the color of the patch
 *
 *    slPatch base constructor requires a parent grid
 *    location and color offset
 */
void slPatch::getColor(slVector *color) {
	color->x = grid->colors[this->colorOffset    ] / 255.0;
	color->y = grid->colors[this->colorOffset + 1] / 255.0;
	color->z = grid->colors[this->colorOffset + 2] / 255.0;
}

void slPatch::setData(void *data) {
	this->data = data;

}

void* slPatch::getData() {
	return this->data;
}

/// why isn't this a return value?
void slPatch::getLocation(slVector *location) {
	slVectorCopy(&this->location, location);
}

/**
 *  slPatchGrid default constructor.
 *
 */		
slPatchGrid::slPatchGrid()
    :   _texture(-1),
		_cubeDrawList(-1)
{

}

/**
 *  slPatch base constructor.
 *
 *  @param theLocation the location as a vector.
 *  @param theGrid the slPatchGrid parent object.
 *  @param theColorOffset the texture offset for GL display
 */
slPatchGrid::slPatchGrid(const slVector *center, const slVector *patchSize, const int x, const int y, const int z)
    :  xSize(x),
	   ySize(y),
	   zSize(z)
{
	int a, b, c;

#ifdef WINDOWS
	// oh windows, why do you have to be such a douchebag about everything?!
	wglTexImage3D = (PFNGLTEXIMAGE3DPROC)wglGetProcAddress("glTexImage3D");
#endif
    
    // I don't know what do do about this code--
    // before it would have left a dangling grid object I think
    // and it should be an exception now...
	if(x < 1 || y < 1 || z < 1) printf("Error instantiating PatchGrid!");

	this->patches = new slPatch**[z];

	this->textureX = slNextPowerOfTwo(x);
	this->textureY = slNextPowerOfTwo(y);
	this->textureZ = slNextPowerOfTwo(z);

	this->colors = new unsigned char[this->textureX * this->textureY * this->textureZ * 4];

	memset(this->colors, 0, this->textureX * this->textureY * this->textureZ * 4);

	slVectorCopy(patchSize, &this->patchSize);

	this->startPosition.x = (-(patchSize->x * x) / 2) + center->x;
	this->startPosition.y = (-(patchSize->y * y) / 2) + center->y;
	this->startPosition.z = (-(patchSize->z * z) / 2) + center->z;

	for(c=0;c<z;c++) {
		this->patches[c] = new slPatch*[x];

		for(b=0;b<x;b++) {
			this->patches[c][b] = new slPatch[y]; // uses default constructor
            
			for(a=0;a<y;a++) {
				this->patches[c][b][a].grid = this;
				this->patches[c][b][a].location.z = this->startPosition.z + c * patchSize->z;
				this->patches[c][b][a].location.x = this->startPosition.x + b * patchSize->x;
				this->patches[c][b][a].location.y = this->startPosition.y + a * patchSize->y;
				this->patches[c][b][a].colorOffset = (c * this->textureZ * this->textureX * 4) + (b * this->textureX * 4) + (a * 4);
			}
		}
	}

	this->drawSmooth = 0;

}

/**
 *  slPatch destructor.
 *
 */
slPatchGrid::~slPatchGrid()
{
    int b, c;

    for(c=0;c<zSize;c++) {
        for(b=0;b<xSize;b++) {
            delete[] patches[c][b];
        }

        delete[] patches[c];
    }

    delete[] patches;

    delete[] colors;
}

/**
 *  getPatchAtLocation returns the patch which contains the point.
 *
 */
slPatch* slPatchGrid::slPatchGrid::getPatchAtLocation(slVector *location) {
	int x, y, z;
	
	x = (int)((location->x - startPosition.x) / patchSize.x);
	y = (int)((location->y - startPosition.y) / patchSize.y);
	z = (int)((location->z - startPosition.z) / patchSize.z);

	if(x < 0 || x >= xSize) return NULL;
	if(y < 0 || y >= ySize) return NULL;
	if(z < 0 || z >= zSize) return NULL;

	return &patches[z][y][x];
}

/**
 *	\brief Copies the contents of a 3D matrix to one z-slice of a PatchGrid.
 */
void slPatchGrid::copyColorFrom3DMatrix(slBigMatrix3DGSL *m, int channel, double scale) {
	int x, y, z;
	int xSize, ySize, zSize;
	float* mData;

	unsigned int chemTDA = m->xDim(); 
	unsigned int chemXY = m->xDim() * m->yDim();

	mData = m->getGSLVector()->data;

	xSize = m->xDim();
	ySize = m->yDim();
	zSize = m->zDim();

	for(z = 0; z < zSize; z++ ) {
		for(y = 0; y < xSize; x++ ) {
			for(x = 0; x < ySize; y++ ) {
				unsigned int crowOffset = (z * this->textureX * this->textureY * 4) + (x * this->textureX * 4);

				this->colors[crowOffset + (y << 2) + channel] = 
					(unsigned char)(255 * scale * mData[ (z * chemXY) + (x * chemTDA) + y ]);

			}
		}
	}
}

/**
 *	\brief Draws the patch grid without using 3D textures.
 */

void slPatchGrid::drawWithout3DTexture(slCamera *camera) {
	int z, y, x;
	int zVal, yVal, xVal;
	int zMid = 0, yMid = 0, xMid = 0;
	slPatch *patch;
	slVector translation, origin;

	if(_cubeDrawList == -1) this->compileCubeList();

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

	for(z=0;z<zSize;z++) {
		if(z < zMid) zVal = z;
		else zVal = (zSize - 1) - (z - zMid);

		translation.z = startPosition.z + patchSize.z * zVal;

		for(x=0;x<xSize;x++) {
			if(x < xMid) xVal = x;
			else xVal = (xSize - 1) - (x - xMid);

			translation.x = startPosition.x + patchSize.x * xVal;

			for(y=0;y<ySize;y++) {
				if(y < yMid) yVal = y;
				else yVal = (ySize - 1) - (y - yMid);

				patch = &patches[zVal][xVal][yVal];

				if(colors[patch->colorOffset + 3] != 255) {
					glPushMatrix();

					translation.y = startPosition.y + patchSize.y * yVal;

					glColor4ubv(&colors[patch->colorOffset]);

					glTranslatef(translation.x, translation.y, translation.z);

					glScalef(patchSize.x, patchSize.y, patchSize.z);

					glCallList(_cubeDrawList);

					glPopMatrix();
				}
			}
		}
	}
}

/**
 *	\brief Draws a set of patches using a volumetric 3D texture.
 *
 *	This may not be supported on older OpenGL implementations, so the alternative 
 *	\ref drawWithout3DTexture may be used instead.  Drawing without the 3D texture
 *	is slower than drawing with a 3D texture.
 */

void slPatchGrid::draw(slCamera *camera) {
	slVector origin, diff, adiff, size;

	if(_texture == -1) _texture = slTextureNew(camera);

#ifdef WINDOWS
	return drawWithout3DTexture(camera);
#endif

	slVectorAdd(&camera->location, &camera->target, &origin);

	diff.x = (origin.x - (startPosition.x + (xSize / 2) * patchSize.x));
	diff.y = (origin.y - (startPosition.y + (ySize / 2) * patchSize.y));
	diff.z = (origin.z - (startPosition.z + (zSize / 2) * patchSize.z));

	adiff.x = fabs(diff.x);
	adiff.y = fabs(diff.y);
	adiff.z = fabs(diff.z);

	glDisable(GL_CULL_FACE);

	glColor4f(1,1,1,1);

	glEnable(GL_TEXTURE_3D);
	glBindTexture(GL_TEXTURE_3D, _texture);

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

	glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);

#ifdef WINDOWS
	wglTexImage3D(GL_TEXTURE_3D, 0, GL_RGBA, textureX, textureY, textureZ, 0, GL_RGBA, GL_UNSIGNED_BYTE, colors);
#else 
	glTexImage3D(GL_TEXTURE_3D, 0, GL_RGBA, textureX, textureY, textureZ, 0, GL_RGBA, GL_UNSIGNED_BYTE, colors);
#endif

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

// ????
void slPatchGridFree(slPatchGrid *g) {
	delete g;
}


slPatch* slPatchGrid::getPatchAtIndex(int x, int y, int z)
{
	if(x < 0 || x >= xSize) return NULL;
	if(y < 0 || y >= ySize) return NULL;
	if(z < 0 || z >= zSize) return NULL;
	return &patches[z][x][y];
}

void slPatchGrid::setDataAtIndex(int x, int y, int z, void *data)
{
	if(x < 0 || x >= this->xSize) return;
	if(y < 0 || y >= this->ySize) return;
	if(z < 0 || z >= this->zSize) return;
	this->patches[z][x][y].data = data;
}

void slPatchGrid::compileCubeList() {
	_cubeDrawList = glGenLists(1);

	glNewList(_cubeDrawList, GL_COMPILE);
	
	glBegin(GL_TRIANGLE_STRIP);
		glVertex3f(0.0, 0.0, 0.0);
		glVertex3f(1.0, 0.0, 0.0);
		glVertex3f(0.0, 0.0, 1.0);
		glVertex3f(1.0, 0.0, 1.0);
		glVertex3f(1.0, 1.0, 1.0);
		glVertex3f(1.0, 0.0, 0.0);
		glVertex3f(1.0, 1.0, 0.0);
		glVertex3f(0.0, 0.0, 0.0);
		glVertex3f(0.0, 1.0, 0.0);
		glVertex3f(0.0, 0.0, 1.0);
		glVertex3f(0.0, 1.0, 1.0);
		glVertex3f(1.0, 1.0, 1.0);
		glVertex3f(0.0, 1.0, 0.0);
		glVertex3f(1.0, 1.0, 0.0);
	
	glEnd();
    
	glEndList();
}
