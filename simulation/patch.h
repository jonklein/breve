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

class slPatch {
	public:
		slPatch() {
			transparency = 1.0;
			data = NULL;
			slVectorSet(&color, 0, 0, 0);
		}

		void *data;
		float transparency;
		slVector color;
		slVector location;
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
		}

		void draw(slCamera *camera);

		int xSize, ySize, zSize;

		slVector startPosition;
		slVector patchSize;

		slPatch ***patches;
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

#ifdef __cplusplus
}
#endif

