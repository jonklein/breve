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

#ifdef __cplusplus

class slTerrainQuadtree;

class slTerrain: public slWorldObject {
	public:
		slTerrain() : slWorldObject() {}
		~slTerrain();

		inline void terrainPoint(int x, int z, slVector *point) {
			point->x = x * xscale + position.location.x;
			point->y = matrix[x][z] +  position.location.y;
			point->z = z * xscale + position.location.z;
		}

		void resize(int side);

		void draw(slCamera *camera);

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
	
		double heightDelta;
		double heightMin;

		// color at the valleys and peaks

		slVector bottomColor;
		slVector topColor;

		slTerrainQuadtree *quadtree;
};

class slTerrainQuadtree {
	public:
		slTerrainQuadtree(int xmin, int ymin, int xmax, int ymax, slTerrain *t) {
			_xmin = xmin;
			_xmax = xmax;
			_ymin = ymin;
			_ymax = ymax;
			
			_x = (xmax + xmin) / 2;
			_y = (ymax + ymin) / 2;

			_terrain = t;	

			if(_xmax - _xmin <= 1 || _ymax - _ymin <= 1) {
				_children[0] = NULL;
				_children[1] = NULL;
				_children[2] = NULL;
				_children[3] = NULL;
				return;
			}

			_children[0] = new slTerrainQuadtree(xmin, ymin, _x, _y, t);
			_children[1] = new slTerrainQuadtree(_x, ymin, xmax, _y, t);
			_children[2] = new slTerrainQuadtree(xmin, _y, _x, ymax, t);
			_children[3] = new slTerrainQuadtree(_x, _y, xmax, ymax, t);
		}
		
		int cull(slCamera *c) {
			int m;

			_minDist = 100000;
			_culled = true;

			_clip = slCameraFrustumPolygonTest(c, _points, 4);

			if(_clip != 1) {
				for(m=0;m<4;m++) {
					slVector toCamera;
					double distance;

					if(_children[m]) _children[m]->cull(c);

					slVectorAdd(&c->target, &c->location, &toCamera);
					slVectorSub(&toCamera, &_points[m], &toCamera);
					distance = slVectorLength(&toCamera);
				
					if(distance < _minDist) _minDist = distance;
				}

				_culled = false;
			}

			return _culled;
		}

		inline int draw(slCamera *c) {
			int m;

			if(_culled) return 0;

			if((_clip == 2 || _minDist < 100) && _children[0]) {
				int drawn = 0;
				for(m=0;m<4;m++) drawn += _children[m]->draw(c);
				return drawn;
			}

			glDisable(GL_BLEND);

			glColor4f(0, (_points[0].y - _terrain->heightMin) / _terrain->heightDelta, 0, 0.8);
			glBegin(GL_LINE_LOOP);
			glVertex3f(_points[0].x, _points[0].y, _points[0].z);
			glVertex3f(_points[1].x, _points[1].y, _points[1].z);
			glVertex3f(_points[2].x, _points[2].y, _points[2].z);
			glVertex3f(_points[3].x, _points[3].y, _points[3].z);
			glEnd();

			return 1;
		}

		void update() {
			int m;
			_terrain->terrainPoint(_xmin, _ymin, &_points[0]);
			_terrain->terrainPoint(_xmin, _ymax, &_points[1]);
			_terrain->terrainPoint(_xmax, _ymax, &_points[2]);
			_terrain->terrainPoint(_xmax, _ymin, &_points[3]);
			if(_children[0]) for(m=0;m<4;m++) _children[m]->update();
		}

		bool _culled;
		int _clip;
		double _minDist;
		int _x, _y;
		int _xmin, _xmax, _ymin, _ymax;
		slTerrainQuadtree *_children[4];
		slTerrain *_terrain;
		slVector _points[4];
};

#endif

#ifdef __cplusplus
extern "C" {
#endif
slTerrain *slTerrainNew(int res, double xscale, void *userData);
void slGenerateFractalTerrain(slTerrain *l, double h, double height);

void slTerrainSetLocation(slTerrain *l, slVector *location);
void slTerrainSetScale(slTerrain *l, double scale);

void slFractalTerrainMatrix(slTerrain *l, double h, double height);
void slTerrainBoundingBox(slTerrain *l);
float slAverageDiamondValues(slTerrain *l, int x, int y, int jump);

void slTerrainInitialize(slTerrain *l);

void slTerrainFree(slTerrain *l);
void slTerrainMakeNormals(slTerrain *l);

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

void slDrawTerrain(slTerrain *l, int texture, double textureScale, int drawMode, int flags);
void slDrawTerrainSide(slTerrain *l, int texture, double textureScale, int drawMode, int flags, int bottom);

int slPointIn2DTriangle(slVector *vertex, slVector *a, slVector *b, slVector *c);

slSerializedTerrain *slSerializeTerrain(slTerrain *t, int *size);

void slTerrainSetHeight(slTerrain *t, int x, int y, double height);
double slTerrainGetHeight(slTerrain *t, int x, int y);

void slTerrainSetTopColor(slTerrain *t, slVector *color);
void slTerrainSetBottomColor(slTerrain *t, slVector *color);

double slTerrainGetHeightAtLocation(slTerrain *t, double x, double z);

int slTerrainLoadGeoTIFF(slTerrain *t, char *file);

#ifdef __cplusplus
}
#endif
