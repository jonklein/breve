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

#include "worldObject.h"

/**
 *	\brief Serialized \ref slTerrain data.
 */

struct slSerializedTerrain {
	int side;
	int repeating;
	double xscale;
	double yscale;

	float *values;
};

/**
 * \brief Terrain data.
 */

void slNormalForFace(slVector *a, slVector *b, slVector *c, slVector *n);

class slTerrainQuadtree;
class slRoamPatch;

class slTerrain: public slWorldObject {
	public:
		friend class slRoamPatch;

		slTerrain(int res, double scale, void *data);
		~slTerrain();

		inline void terrainPoint(int x, int z, slVector *point) {
			point->x =   x * _xscale + _position.location.x;
			point->y = _matrix[x][z] + _position.location.y;
			point->z =   z * _xscale + _position.location.z;
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

		void colorForHeight( double inHeight, slVector *outColor );
	
		int loadGeoTIFF(char *file);

		float **_matrix;
		slVector **fnormals[2];

		int _initialized;

		int _side;
		int _repeating;

		int _drawMode;

		double _xscale;

		// float h;
	
		double _heightDelta;
		double _heightMin;

		// color at the valleys and peaks

		void setTopColor( slVector *color );
		void setBottomColor( slVector *color );

		slVector _bottomColor;
		slVector _topColor;

		slRoamPatch *_roam;

		int _polygonsDrawn;
		int _desiredPolygons;

		void makeNormals();
		void initialize();

		int  shapeClip( slVclipData *vc, int x, int y, slCollision *ce, int flip );
		int sphereClip( slVclipData *vc, int x, int y, slCollision *ce, int flip );

		double pointClip( slPosition *pp, slPoint *p, slCollision *ce );

		void facesUnderRange( double minX, double maxX, double minZ, double maxZ,
    		int *startX, int *endX, int *startZ, int *endZ, int *earlyStart, int *lateEnd);

		void setHeight( int x, int y, double height );
		double getHeight( int x, int y );

		int areaUnderPoint( slVector *origpoint, int *x, int *z, int *quad );
};

int slTerrainTestPair(slVclipData *vc, int x, int y, slCollision *ce);

int slTerrainPlaneUnderPoint(slTerrain *l, slVector *point, slPlane *plane);
double slPointTerrainClip(slTerrain *t, slPosition *pp, slPoint *p, slCollision *ce);

int slTerrainEdgePlaneClip(slVector *start, slVector *end, slFace *face, slPosition *position, slPlane *facePlane, slCollision *ce);

int slPointIn2DTriangle(slVector *vertex, slVector *a, slVector *b, slVector *c);

slSerializedTerrain *slSerializeTerrain(slTerrain *t, int *size);

void slTerrainGetSlope(slTerrain *t, double x1, double z1, double x2, double z2, slVector *result);

#endif // _TERRAIN_H 
