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
	\brief Serialized \ref slTerrain data.
*/

struct slSerializedTerrain {
	int side;
	int repeating;
	double xscale;
	double yscale;

	float *values;
};

/*!
	\brief Terrain data.
*/

struct slTerrain {
	float **matrix;
	slVector **fnormals[2];
	slVector **vnormals;

	int initialized;

	int side;
	int repeating;

	int drawList;
	int drawMode;

	double xscale;
	double yscale;

	float h;

	slVector position;

	slVector max;
	slVector min;

	/* color at the valleys and peaks */

	double heightDelta;
	double heightMin;

	slVector bottomColor;
	slVector topColor;
};

slTerrain *slTerrainNew(int res, double xscale);
void slGenerateFractalTerrain(slTerrain *l, double h, double height);

void slTerrainSetLocation(slTerrain *l, slVector *location);
void slTerrainSetScale(slTerrain *l, double scale);

void slFractalTerrainMatrix(slTerrain *l, double h, double height);
void slTerrainBoundingBox(slTerrain *l);
float slAverageDiamondValues(slTerrain *l, int x, int y, int jump);

void slTerrainInitialize(slTerrain *l);

void slTerrainFree(slTerrain *l);
void slTerrainMakeNormals(slTerrain *l);

int slTerrainTestPair(slWorldObject *w1, slWorldObject *w2, 
		slShape *s1, slShape *s2, 
		slPosition *p1, slPosition *p2, 
		int x, int y, slCollisionEntry *ce);

void slTerrainFacesUnderRange(slTerrain *l, 
	double minX, double maxX, double minZ, double maxZ,
	int *startx, int *endx, int *startz, int *endz,
	int *earlyStart, int *lateEnd);

int slTerrainPlaneUnderPoint(slTerrain *l, slVector *point, slPlane *plane);
int slTerrainSphereClip(slTerrain *l, slShape *ss, slPosition *sp, int x, int y, slCollisionEntry *ce, int flip);
int slTerrainShapeClip(slTerrain *l, slShape *ss, slPosition *sp, int obX, int obY, slCollisionEntry *ce, int flip);
double slPointTerrainClip(slTerrain *t, slPosition *pp, slPoint *p, slCollisionEntry *ce);

int slTerrainEdgePlaneClip(slVector *start, slVector *end, slFace *face, slPosition *position, slPlane *facePlane, slCollisionEntry *ce);

void slDrawTerrain(slWorld *w, slCamera *c, slTerrain *l, int texture, double textureScale, int drawMode, int flags);
void slDrawTerrainSide(slWorld *w, slTerrain *l, int texture, double textureScale, int drawMode, int flags, int bottom);

int slPointIn2DTriangle(slVector *vertex, slVector *a, slVector *b, slVector *c);

slSerializedTerrain *slSerializeTerrain(slTerrain *t, int *size);
