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

enum shapeTypes {
	ST_NORMAL,
	ST_SPHERE
};

enum slFeatureTypes {
	FT_POINT,
	FT_EDGE,
	FT_FACE
};

enum shapeDraw {
	SD_STENCIL = 0x01,
	SD_REFLECT = 0x02
};

/*!
	\brief A feature in a shape.
*/

struct slFeature {
	int type;
	void *data;
};

/*!
	\brief A struct containing rotation and location information.
*/

struct slPosition {
	double rotation[3][3];
	slVector location;
};

/*!
	\brief A plane, specified by a normal and a vertex.
*/

struct slPlane {
	slVector normal;
	slVector vertex;
};

/*!
	\brief A point on a shape.
*/

struct slPoint {
	slVector vertex;

	/* the voronoi planes defining this point and the number of planes */

	slPlane *voronoi;
	int edgeCount;

	/* temp data for terrain collisions :( */

	int terrainX;
	int terrainZ;
	int terrainQuad;
	
	/* we need to know of the edge neighbors, as well as the face neighbors */

	slFeature **neighbors;
	slFeature **faces;
};

/*!
	\brief An edge on a shape.

	The endpoints of the edge are the vertex neighbors 0 and 1.
*/

struct slEdge {
	slPlane voronoi[4];
	slFeature *neighbors[4];
};

/*!
	\brief A face on a shape.
*/

struct slFace {
	int edgeCount;

	slPlane plane;

	slPlane *voronoi;	

	slFeature **neighbors;	// neighbor edges
	slFeature **points;		// connected points 
	slFeature **faces;		// connected faces

	char drawFlags;
};

/*!
	\brief A shape in the simulated world.
*/

struct slShape {
	int referenceCount;

	int drawList;

	unsigned char recompile;

	double inertia[3][3];
	double mass;
	double density;

	/* the max reach on each axis */

	slVector max;

	/* add support for this shape to be a sphere, in which case the */
	/* normal features below are ignored */

	int type;
	double radius;

	slFeature **features;
	int featureCount;
	int maxFeatures;

	int firstPoint;
};

/*!
	\brief A header used when serializing shape data.
*/

struct slSerializedShapeHeader {
	double inertia[3][3];
	double density;
	double mass;

	slVector max;

	int type; 
	double radius;
    
	int faceCount;
};  

typedef struct slSerializedShapeHeader slSerializedShapeHeader;

/*!
	\brief A header used when serializing face data.  

	It's only contains an integer at the moment.
*/

struct slSerializedFaceHeader {
	int vertexCount;
};

typedef struct slSerializedFaceHeader slSerializedFaceHeader;

slShape *slNewShape(void);

slShape *slNewCube(slVector *size, double density);
slShape *slNewNGonDisc(int count, double radius, double height, double density);
slShape *slNewNGonCone(int count, double radius, double height, double density);
slShape *slNewSphere(double radius, double density);

slFeature *slAddFace(slShape *v, slVector **points, int vCount);
slFeature *slAddEdge(slShape *s, slFeature *theFace, slVector *start, slVector *end);

slFeature *slAddPoint(slShape *v, slVector *start);

slFeature *slNewFeature(int type, void *data);

void slFreeShape(slShape *s);
void slFreeFeature(slFeature *f);

slShape *slSetCube(slShape *s, slVector *a, double density);
slShape *slSetPyramid(slShape *s, double len, double density);
slShape *slSetNGonDisc(slShape *s, int sideCount, double radius, double height, double density);
slShape *slSetNGonCone(slShape *s, int sideCount, double radius, double height, double density);

slPlane *slSetPlane(slPlane *p, slVector *normal, slVector *vertex);

int slFeatureSort(const void *a, const void *b);

slShape *slInitNeighbors(slShape *s, double density);

void slShapeSetMass(slShape *shape, double mass);
void slShapeSetDensity(slShape *shape, double density);

void slDumpEdgeVoronoi(slShape *s);

int slCountPoints(slShape *s);

void slCubeInertiaMatrix(slVector *c, double mass, double i[3][3]);
void slSphereInertiaMatrix(double radius, double mass, double i[3][3]);

int slPointOnShape(slShape *s, slVector *dir, slVector *point);

void slScaleShape(slShape *s, slVector *scale);

slSerializedShapeHeader *slSerializeShape(slShape *s, int *length);
slShape *slDeserializeShape(slSerializedShapeHeader *header, int length);
void slShapeBounds(slShape *shape, slPosition *position, slVector *min, slVector *max);

