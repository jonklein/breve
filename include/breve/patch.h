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

#ifdef __cplusplus
/*!
	\brief Data associated with a certain region of 3D space.
*/

#include "bigMatrix.hh"
#include "glIncludes.h"

class slPatchGrid;

class slPatch {
	public:
		slPatch() {
			data = NULL;
		}

		void *data;

		slVector location;
		int colorOffset;
		slPatchGrid *grid;
};

/*!
	\brief A grid of \ref slPatch objects.
*/

class slPatchGrid {
	public:
		~slPatchGrid() {
			int b, c;

			for(c=0;c<zSize;c++) {
				for(b=0;b<ySize;b++) {
					delete[] patches[c][b];
				}

				delete[] patches[c];
			}

			delete[] patches;

			delete[] colors;
		}

		slPatchGrid() {
			_texture = -1;
			_cubeDrawList = -1;
		}

		void compileCubeList();

		void draw(slCamera *camera);
		void drawWithout3DTexture(slCamera *camera);

		void setSmoothDrawing(int d) { drawSmooth = d; }

		void textureDrawXPass(slVector &size, int dir);
		void textureDrawYPass(slVector &size, int dir);
		void textureDrawZPass(slVector &size, int dir);

		int xSize, ySize, zSize;

		slVector startPosition;
		slVector patchSize;

		slPatch ***patches;

		// colors holds all of the color information for the patches.  it is a raw 
		// array of char values so that we can use it as texture data if desired.

		unsigned char *colors;

		int textureX, textureY, textureZ;

		int drawSmooth;

		int _texture;
		int _cubeDrawList;
};
#endif

typedef struct slPatchGrid slPatchGrid;
typedef struct slPatch slPatch;

#ifdef __cplusplus
extern "C" {
#endif

slPatchGrid *slPatchGridNew(slVector *center, slVector *size, int x, int y, int z);

void slPatchGridFree(slPatchGrid *g);
void slInitPatch(slPatch *p);

void slPatchSetData(slPatch *p, void *data);
void *slPatchGetData(slPatch *p);

void slPatchGetLocation(slPatch *p, slVector *location);
void slPatchGetColor(slPatch *p, slVector *color);
void slPatchSetColor(slPatch *p, slVector *color);

void slPatchSetTransparency(slPatch *p, double transparency);

slPatch *slPatchForLocation(slPatchGrid *g, slVector *location);

void slDrawPatches(slPatchGrid *patches, slVector *cam, slVector *target);
slPatch *slPatchAtIndex(slPatchGrid *patches, int x, int y, int z);

void *slPatchGetDataAtIndex(slPatchGrid *grid, int x, int y, int z);

void slPatchSetDataAtIndex(slPatchGrid *grid, int x, int y, int z, void *data);

void slPatchGridCopyColorFrom3DMatrix(slPatchGrid *grid, slBigMatrix3DGSL *m, int channel, double scale);
void slPatchGridCopyColorFrom2DMatrix(slPatchGrid *grid, slBigMatrix2DGSL *m, int slice, int channel, double scale);

#ifdef __cplusplus
}
#endif

