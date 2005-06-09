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

#ifndef _TERRAIN_H
#define _TERRAIN_H

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

#ifdef __cplusplus

void slNormalForFace(slVector *a, slVector *b, slVector *c, slVector *n);

class slTerrainQuadtree;
class slRoamPatch;

class slTerrain: public slWorldObject {
	public:
		slTerrain(int res, double scale, void *data);
		~slTerrain();

		inline void terrainPoint(int x, int z, slVector *point) {
			point->x = x * xscale + position.location.x;
			point->y = matrix[x][z] +  position.location.y;
			point->z = z * xscale + position.location.z;
		}

		void setLocation(slVector *location);
		void setScale(double scale);

		double getHeightAtLocation(double x, double z);

		void updateBoundingBox();
		void setDrawMode(int mode);

		void resize(int side);

		void draw(slCamera *camera);

		void generateFractalTerrain(double h, double height);
		float averageDiamondValues(int x, int y, int jump);

		int loadGeoTIFF(char *file);

		float **matrix;
		slVector **fnormals[2];

		int _initialized;

		int side;
		int _repeating;

		int _drawMode;

		double xscale;

		// float h;
	
		double heightDelta;
		double heightMin;

		// color at the valleys and peaks

		slVector bottomColor;
		slVector topColor;

		slRoamPatch *_roam;

		int _polygonsDrawn;
		int _desiredPolygons;


		void makeNormals();
		void initialize();
};

#endif

#ifdef __cplusplus
extern "C" {
#endif

int slTerrainTestPair(slVclipData *vc, int x, int y, slCollision *ce);

void slTerrainFacesUnderRange(slTerrain *l, 
	double minX, double maxX, double minZ, double maxZ,
	int *startx, int *endx, int *startz, int *endz,
	int *earlyStart, int *lateEnd);

int slTerrainPlaneUnderPoint(slTerrain *l, slVector *point, slPlane *plane);
int slTerrainSphereClip(slVclipData *vc, slTerrain *l, int x, int y, slCollision *ce, int flip);
int slTerrainShapeClip(slVclipData *vc, slTerrain *l, int obX, int obY, slCollision *ce, int flip);
double slPointTerrainClip(slTerrain *t, slPosition *pp, slPoint *p, slCollision *ce);

int slTerrainEdgePlaneClip(slVector *start, slVector *end, slFace *face, slPosition *position, slPlane *facePlane, slCollision *ce);

int slPointIn2DTriangle(slVector *vertex, slVector *a, slVector *b, slVector *c);

slSerializedTerrain *slSerializeTerrain(slTerrain *t, int *size);

void slTerrainSetHeight(slTerrain *t, int x, int y, double height);
double slTerrainGetHeight(slTerrain *t, int x, int y);

void slTerrainSetTopColor(slTerrain *t, slVector *color);
void slTerrainSetBottomColor(slTerrain *t, slVector *color);

void slTerrainGetSlope(slTerrain *t, double x1, double z1, double x2, double z2, slVector *result);

#ifdef __cplusplus
}
#endif

#endif /* _TERRAIN_H */
