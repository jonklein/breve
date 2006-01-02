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
#include "roam.h"
#include "vclip.h"
#include "vclipData.h"

#if HAVE_LIBTIFF
namespace tiff {
using namespace tiff;
#include "tiffio.h"
}
using namespace tiff;
#endif

/*
	fractal terrain code based on tutorial and code from Paul E. Martz.
*/

/*
	1 5 3 5 1
	5 4 5 4 5
	3 5 2 5 3
	5 4 5 4 5
	1 5 3 5 1
*/

slTerrain::slTerrain(int res, double scale, void *data) : slWorldObject() {
	slVector location;
	int n;

	userData = data;

	side = (int)pow(2, res) + 1;

	matrix = new float*[side];

	fnormals[0] = new slVector*[side - 1];
	fnormals[1] = new slVector*[side - 1];

	for(n=0;n<side - 1;n++) {
		fnormals[0][n] = new slVector[side - 1];
		fnormals[1][n] = new slVector[side - 1];
	}

	for(n=0;n<side;n++) {
		matrix[n] = new float[side];
	}

	_roam = new slRoamPatch(this);
	_roam->setDetailLevel(10);

	_drawMode = 1;

	_desiredPolygons = 8000;

	xscale = scale;

	textureScaleX = side;
	textureScaleY = side;

	_initialized = 0;

	slVectorSet(&location, 0, 0, 0);

	setLocation(&location);
}

/*!
	\brief Sets the draw mode of the terrain.

	0 = wireframe, 1 = fill.
*/

void slTerrain::setDrawMode(int m) {
	_drawMode = m;
}

/*!
	\brief Resizes the terrain to be able to hold at least side x side points.  
	The actual size will be the next power of two above side.
*/

void slTerrain::resize(int s) {
	int n;

	for(n=0;n<side;n++) {
		delete[] matrix[n];

		if(n != side - 1) {
			delete[] fnormals[0][n];
			delete[] fnormals[1][n];
		}
	}

	delete[] fnormals[0];
	delete[] fnormals[1];
	delete[] matrix;

	delete _roam;
	
	side = slNextPowerOfTwo(s) + 1;
	textureScaleX = side;
	textureScaleY = side;

	matrix = new float*[side];
	fnormals[0] = new slVector*[side - 1];
	fnormals[1] = new slVector*[side - 1];

	for(n=0;n<side;n++) {
		matrix[n] = new float[side];

		if ( n != side - 1) { 
			fnormals[0][n] = new slVector[side - 1];
			fnormals[1][n] = new slVector[side - 1];
		}
	}

	_roam = new slRoamPatch(this);
	_roam->setDetailLevel(14);

	_initialized = 0;
}

/*!
	\brief Sets the x/z scale of the terrain.
*/

void slTerrain::setScale(double scale) {
	double x;
	slVector half, p;

	x = side * xscale / 2;

	slVectorSet(&half, x, 0, x);
	slVectorAdd(&position.location, &half, &p);

	xscale = scale;

	setLocation(&p);

	_initialized = 0;
}

/*!
	\brief Sets the midpoint location of the terrain.
*/

void slTerrain::setLocation(slVector *location) {
	slVector half;
	double x;

	x = side * xscale / 2;

	slVectorSet(&half, x, 0, x);
	slVectorSub(location, &half, &position.location);

	initialize();
}

void slTerrain::initialize() {
	makeNormals();
	updateBoundingBox();

	_initialized = 1;
	_repeating = 0;
}

void slTerrain::updateBoundingBox() {
	int x, z;
	double hmax = 0.0, hmin = 0.0;

	for(x=0;x<side;x++) {
		for(z=0;z<side;z++) {
			if(matrix[x][z] > hmax) hmax = matrix[x][z];
			if(matrix[x][z] < hmin) hmin = matrix[x][z];
		}
	}

	max.x = max.z = (side * xscale);
	max.y = hmax;

	min.x = min.z = 0.0;
	min.y = hmin;

	if(_repeating) {
		min.x = min.z = -DBL_MAX;
		max.x = max.z = DBL_MAX;
	}

	slVectorAdd(&min, &position.location, &min);
	slVectorAdd(&max, &position.location, &max);
}

/*!
	\brief Initializes this terrain with a fractal generated heightmap.
*/

void slTerrain::generateFractalTerrain(double h, double height) {
	float ratio;
	int x, y, jump;
	int oddline, newside;
	float scale;

	// matrix should be square, and each side should be 2^n + 1 

	newside = slNextPowerOfTwo(side);
	resize(newside);

	jump = side / 2;
	ratio = pow(2.0, -h);

	matrix[0][side - 1] = matrix[side - 1][0] = 0.0;
	matrix[0][0] = matrix[side - 1][side - 1] = 0.0;

	scale = 1.0;

	while(jump) {
		scale *= ratio;

		for(x=jump;x<side - 1;x+=2*jump) {
			for(y=jump;y<side - 1;y+=2*jump) {
				matrix[x][y] = (matrix[x - jump][y - jump] + matrix[x + jump][y - jump] +
					   matrix[x - jump][y + jump] + matrix[x + jump][y + jump]) / 4;

				matrix[x][y] += (slRandomDouble() -.5) * scale;
			}
		}

		oddline = 0;

		for(x=0;x<side - 1;x+=jump) {
			oddline = !oddline;
		  
			if(oddline) y = jump;
			else y = 0;

			for(;y<side - 1;y+=2*jump) {
				matrix[x][y] = averageDiamondValues(x, y, jump);
				matrix[x][y] += (slRandomDouble() - .5) * scale;

				if(!x) matrix[side - 1][y] = matrix[x][y];
				if(!y) matrix[x][side - 1] = matrix[x][y];
			}
		}

		jump /= 2;
	}

	for(x=0;x<side;x++) 
		for(y=0;y<side;y++) 
			matrix[x][y] *= height;

	_initialized = 0;
}

/*
	\brief Used internally to generate normals for the terrain faces.
*/

void slTerrain::makeNormals() {
	slVector a, b, c, d;
	slVector v1, v2;
	slVector yAxis;
	double max = 0;

	int x, z;

	heightMin = DBL_MAX;

	slVectorSet(&yAxis, 0, 1, 0);

	for(x=0;x<(side - 1);x++) {
		for(z=0;z<(side - 1);z++) {
			a.x = x * xscale;
			a.y = matrix[x][z];
			a.z = z * xscale;

			b.x = x * xscale;
			b.y = matrix[x][z + 1];
			b.z = (z + 1) * xscale;

			c.x = (x + 1) * xscale;
			c.y = matrix[x + 1][z];
			c.z = z * xscale;

			d.x = (x + 1) * xscale;
			d.y = matrix[x + 1][z + 1];
			d.z = (z + 1) * xscale;

			slVectorSub(&a, &b, &v1);
			slVectorSub(&c, &b, &v2);

			slVectorCross(&v2, &v1, &fnormals[0][x][z]);
			slVectorNormalize(&fnormals[0][x][z]);

			slVectorSub(&c, &b, &v1);
			slVectorSub(&d, &b, &v2);

			slVectorCross(&v2, &v1, &fnormals[1][x][z]);
			slVectorNormalize(&fnormals[1][x][z]);

			if(matrix[x][z] < heightMin) heightMin = matrix[x][z];
			if(matrix[x][z] > max) max = matrix[x][z];
		}
	}

	heightDelta = max - heightMin;
}

void slNormalForFace(slVector *a, slVector *b, slVector *c, slVector *n) {
	slVector v1, v2;

	slVectorSub(a, b, &v1);
	slVectorSub(c, b, &v2);

	slVectorCross(&v2, &v1, n);
	slVectorNormalize(n);
}

float slTerrain::averageDiamondValues(int x, int y, int jump) {
	// sideminus is the zero based array offset of side 

	int sideminus = side - 1;

	if(x == 0) {
		return (matrix[x][y - jump] + matrix[x + jump][y] +
				matrix[sideminus - jump][y] + matrix[x][y + jump]) / 4.0;
	} else if(y == 0) {
		return (matrix[x][sideminus - jump] + matrix[x + jump][y] +
				matrix[x - jump][y] + matrix[x][y + jump]) / 4.0;
	} else if(x == sideminus) {
		return (matrix[x][y - jump] + matrix[jump][y] +
				matrix[x - jump][y] + matrix[x][y + jump]) / 4.0;
	} else if(y == sideminus) {
		return (matrix[x][y - jump] + matrix[x + jump][y] +
				matrix[x - jump][y] + matrix[x][jump]) / 4.0;
	} else {
		return (matrix[x][y - jump] + matrix[x + jump][y] +
				matrix[x - jump][y] + matrix[x][y + jump]) / 4.0;
	}
}

slTerrain::~slTerrain() {
	int n;

	for(n=0; n < side - 1;n++) {
		delete[] fnormals[0][n];
		delete[] fnormals[1][n];
	}

	for(n=0; n < side;n++) {
		delete[] matrix[n];
	}

	delete[] fnormals[0];
	delete[] fnormals[1];
	delete[] matrix;
}

void slTerrain::draw(slCamera *camera) {
	if(!_initialized) initialize();

	glPushAttrib(GL_ENABLE_BIT);

	glEnable( GL_LIGHTING );

	_roam->tessellate(camera);

	_polygonsDrawn = _roam->render( camera, _drawMode);


	glPopAttrib();
}

/*
	+ slTerrainFacesUnderRange
	= when we're looking at spheres, they may lay over the borders of terrain 
	= squares, so we will want to look at all of the faces within a certain 
	= range.  
	=
	= given the maximum and minimum x & z points for a region, this function
	= will determine the range, placing the output in the startZ, endZ, startX
	= and endZ pointers.
	=
	= if you direct your attention to the bad ascii art below, you'll see that
	= any box drawn over these faces will always cover both sections of the 
	= upper-left and lower-right vertices, but this is not necessarily the case
	= for the lower-left or upper-right vertices.  so the variables earlyStart
	= and lateEnd correspond to the inclusion of the extra lower-left face and
	= upper-right face.
	=
	= ----------
	= |\ |\ |\ |
	= | \| \| \|
	= ----------
	= |\ |\ |\ |
	= | \| \| \|
	= ----------
*/

void slTerrainFacesUnderRange(slTerrain *l, 
	double minX, double maxX, double minZ, double maxZ, 
	int *startX, int *endX, int *startZ, int *endZ, int *earlyStart, int *lateEnd) {

	int localX, localZ;

	minX -= l->position.location.x;
	maxX -= l->position.location.x;
	minZ -= l->position.location.z;
	maxZ -= l->position.location.z;

	minX /= l->xscale;
	maxX /= l->xscale;
	minZ /= l->xscale;
	maxZ /= l->xscale;

	if(maxX < 0.0 || maxZ < 0.0) {
		*startX = *startZ = *endX = *endZ = -1;
		return;
	}

	if(minX > (l->side - 1) || minZ > (l->side - 1)) {
		*startX = *startZ = *endX = *endZ = l->side - 2;
		return;
	}

	if(minX < 0.0) *startX = 0;
	else *startX = (int)minX;

	if(minZ < 0.0) *startZ = 0;
	else *startZ = (int)minZ;

	if(maxX > (l->side - 2)) *endX = l->side - 2;
	else *endX = (int)maxX;

	if(maxZ > (l->side - 2)) *endZ = l->side - 2;
	else *endZ = (int)maxZ;

	localX = (int)fmod(minX, 1.0);
	localZ = (int)fmod(minZ, 1.0);
	if((localX + localZ) < 1.0) *earlyStart = 1;
	else *earlyStart= 0;

	localX = (int)fmod(minX, 1.0);
	localZ = (int)fmod(maxZ, 1.0);
	if((localX + localZ) < 1.0) *lateEnd = 0;
	else *lateEnd = 1;
}

/*!
	\brief Takes a point, finds the X, Z and Quad location on the terrain for this point.
*/

int slTerrainAreaUnderPoint(slTerrain *l, slVector *origpoint, int *x, int *z, int *quad) {
	double localx, localz;
	double xpos, zpos;
	slVector point;
	int result = 0;

	// transform this point into the terrain's coordinates 

	slVectorSub(origpoint, &l->position.location, &point);

	// scale down by l->xscale on both x and z axes 

	xpos = point.x / l->xscale;
	zpos = point.z / l->xscale;

	localx = fmod(xpos, 1.0);
	localz = fmod(zpos, 1.0);

	*x = (int)xpos;
	*z = (int)zpos;

	if((localx + localz) < 1.0) *quad = 0;
	else *quad = 1;

	if(*x < 0) {
		*x = -1;
		*quad = 0;
		result = -1;
	} else if(*x >= l->side - 1) {
		*x = l->side - 2;
		*quad = 1;
		result = -1;
	}

	if(*z < 0) {
		*z = -1;
		*quad = 0;
		result = -1;
	} else if(*z >= l->side - 1) {
		*z = l->side - 2;
		*quad = 1;
		result = -1;
	}

	return result;
}

int slTerrainTestPair(slVclipData *vc, int x, int y, slCollision *ce) {
	slTerrain *terrain;
	slWorldObject *w1 = vc->objects[x];
	slWorldObject *w2 = vc->objects[y];

	if(w1->type != WO_TERRAIN) {
		slShape *s = w1->shape;
		terrain = (slTerrain*)w2;
		if(!terrain->_initialized) terrain->initialize();
		if(s->_type == ST_SPHERE) return slTerrainSphereClip(vc, terrain, x, y, ce, 0);
		else return slTerrainShapeClip(vc, terrain, x, y, ce, 0);
	} else {
		slShape *s = w2->shape;
		terrain = (slTerrain*)w1;
		if(!terrain->_initialized) terrain->initialize();
		if(s->_type == ST_SPHERE) return slTerrainSphereClip(vc, terrain, y, x, ce, 1);
		else return slTerrainShapeClip(vc, terrain, y, x, ce, 1);
	}

	return CT_DISJOINT;
}

/*!
	\brief Check for a collision of a sphere against a landscape.
*/

int slTerrainSphereClip(slVclipData *vc, slTerrain *l, int obX, int obY, slCollision *ce, int flip) {
	slPlane landPlane;
	double dist;
	slVector terrainPoint, aveNormal, toSphere;
	int startX, endX, startZ, endZ, earlyStart, lateEnd, x, z, quad;
	int collisions = 0;

	slSphere *ss = (slSphere*)vc->objects[obX]->shape;
	slPosition *sp = &vc->objects[obX]->position;

	if(ce) {
		if(!flip) {
			ce->n1 = obY;
			ce->n2 = obX;
		} else {
			ce->n1 = obX;
			ce->n2 = obY;
		}
	}

	slVectorSet(&aveNormal, 0, 0, 0);

	slTerrainFacesUnderRange(l, sp->location.x - ss->_radius, sp->location.x + ss->_radius, 
								sp->location.z - ss->_radius, sp->location.z + ss->_radius,
								&startX, &endX, &startZ, &endZ, &earlyStart, &lateEnd);

	for(x=startX;x<=endX;x++) {
		for(z=startZ;z<=endZ;z++) {
			slVectorSet(&landPlane.vertex, x * l->xscale, l->matrix[x][z], z * l->xscale);
			slVectorAdd(&landPlane.vertex, &l->position.location, &landPlane.vertex);

			for(quad=0;quad<2;quad++) {
				int skip = 0;

				// check to see if we are covering enough of the upper-right and lower-left 
				// squares to check the second face 

				if(x == startX && z == startZ && quad == 0 && !earlyStart) skip = 1;
				else if(x == endX && z == endZ && quad == 1 && !lateEnd) skip = 1;

				if(!skip) {
					int trivi;
					slVector a, b, c, sloc;

					slVectorCopy(&sp->location, &sloc);

					if(quad == 0) {
						slVectorSet(&a, x, l->matrix[x][z], z);
						slVectorSet(&b, x + 1, l->matrix[x + 1][z], z);
						slVectorSet(&c, x, l->matrix[x][z + 1], z + 1);
					} else {
						slVectorSet(&a, x + 1, l->matrix[x + 1][z + 1], z + 1);
						slVectorSet(&b, x, l->matrix[x][z + 1], z + 1);
						slVectorSet(&c, x + 1, l->matrix[x + 1][z], z);
					}

					a.x *= l->xscale;
					a.z *= l->xscale;
					b.x *= l->xscale;
					b.z *= l->xscale;
					c.x *= l->xscale;
					c.z *= l->xscale;

					slVectorAdd(&a, &l->position.location, &a);
					slVectorAdd(&b, &l->position.location, &b);
					slVectorAdd(&c, &l->position.location, &c);

					trivi = slPointIn2DTriangle(&sloc, &a, &b, &c);

					if(trivi == 0) {
						// no violations, test against the plane 

						slVectorCopy(&l->fnormals[quad][x][z], &landPlane.normal);

						dist = slPlaneDistance(&landPlane, &sp->location) - ss->_radius;

						if(dist < MC_TOLERANCE && dist > (-2 * ss->_radius)) {
							collisions++;

							if(!ce) return CT_PENETRATE;

							slVectorMul(&landPlane.normal, ss->_radius, &toSphere);
							slVectorSub(&sp->location, &toSphere, &terrainPoint);

							if(dist < VC_WARNING_TOLERANCE)
								slMessage(DEBUG_WARN, "deep collision (%f) in terrain/sphere clip\n", dist);

							ce->depths.push_back(dist);
							ce->points.push_back(terrainPoint);
							slVectorCopy(&landPlane.normal, &ce->normal);
							if(!flip) slVectorMul(&ce->normal, -1, &ce->normal);

							if(flip) ce = slNextCollision(vc, obX, obY);
							else ce = slNextCollision(vc, obY, obX);
						} 
					} else if(trivi == 0x01 || trivi == 0x02 || trivi == 0x04) {
						// a violation against one of the edges -- test that edge

						slVector *start, *end, collisionPoint;

						if(trivi == 0x01) {
							start = &a;		
							end = &b;		
						} else if(trivi == 0x02) {
							start = &b;		
							end = &c;		
						} else {
							start = &c;		
							end = &a;		
						}

						dist = slPointLineDist(start, end, &sp->location, &collisionPoint) - ss->_radius;

						if(dist < MC_TOLERANCE && dist > (-2 * ss->_radius)) {
							collisions++;

							if(!ce) return CT_PENETRATE;

							if(dist < VC_WARNING_TOLERANCE)
								slMessage(DEBUG_WARN, "deep edge collision (%f) in terrain/sphere clip, triangle violation %d\n", dist, trivi);

							ce->depths.push_back(dist);
							ce->points.push_back(collisionPoint);

							slVectorSub(&sp->location, &collisionPoint, &toSphere);
							slVectorNormalize(&toSphere);
							slVectorCopy(&toSphere, &ce->normal);
							if(!flip) slVectorMul(&ce->normal, -1, &ce->normal);

							if(flip) ce = slNextCollision(vc, obX, obY);
							else ce = slNextCollision(vc, obY, obX);
						} 
					} else {
						slVector *point;

						if(trivi == 0x03) {
							point = &b;
						} else if(trivi == 0x05) {
							point = &a;
						} else {
							point = &c;
						}

						slVectorSub(&sp->location, point, &toSphere);
						dist = slVectorLength(&toSphere) - ss->_radius;

						if(dist < MC_TOLERANCE && dist > (-2 * ss->_radius)) {
							collisions++;

							if(!ce) return CT_PENETRATE;

							if(dist < VC_WARNING_TOLERANCE)
								slMessage(DEBUG_WARN, "deep point collision (%f) in terrain/sphere clip, triangle violation %d\n", dist, trivi);

							ce->depths.push_back(dist);
							ce->points.push_back(*point);

							slVectorNormalize(&toSphere);
							slVectorCopy(&toSphere, &ce->normal);
							if(!flip) slVectorMul(&ce->normal, -1, &ce->normal);

							if(flip) ce = slNextCollision(vc, obX, obY);
							else ce = slNextCollision(vc, obY, obX);
						}
					}
				}
			}
		}
	}

	if(collisions == 0) return CT_DISJOINT;

	return CT_PENETRATE;
}

/*!
	\brief Determines whether the vertex projection lies in the *projection* 
	of the triangle defined by t1, t2, t3 WHICH MUST define the triangle in 
	a clockwise fashion.  Ignores the Y coordinate of the point.

	Returns 0 upon success.

	Upon failure, returns an integer with the lines violated encoded as the
	following bits:
		- 0x01 = AB
		- 0x02 = BC
		- 0x04 = CA
*/

int slPointIn2DTriangle(slVector *vertex, slVector *a, slVector *b, slVector *c) {
	float AB = ((vertex->z - a->z) * (b->x - a->x)) - ((vertex->x - a->x) * (b->z - a->z));
	float BC = ((vertex->z - b->z) * (c->x - b->x)) - ((vertex->x - b->x) * (c->z - b->z));
	float CA = ((vertex->z - c->z) * (a->x - c->x)) - ((vertex->x - c->x) * (a->z - c->z));

	int violation = 0;

	if(AB < 0.0) violation |= 0x01;
	if(BC < 0.0) violation |= 0x02;
	if(CA < 0.0) violation |= 0x04;

	return violation;
}

/*!
	\brief Tests a shape against a terrain for collisions.

	I'm not sure this is the world's greatest algorithm:

	- for every point in the shape 
		- test for collision against the terrain (noting which mesh triangle it's over)

	- for every face in the shape
		- for all terrain edges covered by the face (based on the point test above)
		- test edge against the plane
*/

int slTerrainShapeClip(slVclipData *vc, slTerrain *l, int obX, int obY, slCollision *ce, int flip) {
	std::vector<slPoint*>::iterator pi;
	std::vector<slFace*>::iterator fi;
	int collisions = 0;

	slShape *ss = vc->objects[obX]->shape;
	slPosition *sp = &vc->objects[obX]->position;

	slVectorSet(&ce->normal, 0, 0, 0);

	for(pi = ss->points.begin(); pi != ss->points.end(); pi++) {
		slPoint *p = *pi;
		slPointTerrainClip(l, sp, p, ce);
	}

	for(fi = ss->faces.begin(); fi != ss->faces.end(); fi++ ) {
		slFace *face = *fi;
		slPlane facePlane;
		slVector y;
		int np;

		slVectorSet(&y, 0, -1, 0);

		slPositionPlane(sp, &face->plane, &facePlane);

		/* is the plane well oriented towards the terrain?  since the shape is	*/
		/* convex, and the terrain cannot be more than 90 degrees from (0, 1, 0)  */
		/* then if we're more than 90 from (0, -1, 0), then there exists a better */
		/* plane containing the same edge that we should use instead. */

		if(slVectorDot(&y, &facePlane.normal) > 0.0) {
			int minX = l->side, minZ = l->side, maxX = 0, maxZ = 0;
			int earlyStart = 0, lateEnd = 0;
			int x, z;

			for(np=0;np<face->edgeCount;np++) {
				slPoint *facePoint = face->points[np];

				/* find the min/max range for the edges we'l need to test */

				if(facePoint->terrainX > maxX) maxX = facePoint->terrainX;

				if(facePoint->terrainX < minX) minX = facePoint->terrainX;

				if(facePoint->terrainZ > maxZ) {
					maxZ = facePoint->terrainZ;
					lateEnd = facePoint->terrainQuad;
				}

				if(facePoint->terrainZ < minZ) {
					minZ = facePoint->terrainZ;
					earlyStart = !facePoint->terrainQuad;
				}
			}

			for(x=minX;x<=maxX;x++) {
				for(z=minZ;z<=maxZ;z++) {
					int skip = 0;

					/*
						 v1 ---- v3
							|\ |			points v1, v2 and v3 for each of the squares 
							| \|
 							---- v2

						we'll check edges: v13, v32, v21 for collisions

						note that a terrain point piercing a face will have
						multiple edges colliding with the face as the point
						where the edges meet.  therefore, we will only count
						collisions on the "leading" point.

						note that the exception is when we're on the bottom or 
						left edge of the terrain (minX or minZ == -1).
					*/

				
					slVector v1, v2, v3;

					if(x != -1 && z != -1) {
						v2.z = z * l->xscale;
						v1.z = v3.z = (z + 1) * l->xscale;	

						v1.x = x * l->xscale;
						v2.x = v3.x = (x + 1) * l->xscale;	

						v1.y = l->matrix[x][z + 1];
						v2.y = l->matrix[x + 1][z];
						v3.y = l->matrix[x + 1][z + 1];
					} else if(x == -1) {
						v2.z = z * l->xscale;
						v1.z = v3.z = (z + 1) * l->xscale;	

						v1.x = 0;
						v2.x = v3.x = l->xscale;	

						v1.y = l->matrix[0][z + 1];
						v2.y = l->matrix[1][z];
						v3.y = l->matrix[1][z + 1];
					} else {
						v2.z = 0;
						v1.z = v3.z = l->xscale;	

						v1.x = x * l->xscale;
						v2.x = v3.x = (x + 1) * l->xscale;	

						v1.y = l->matrix[x][1];
						v2.y = l->matrix[x + 1][0];
						v3.y = l->matrix[x + 1][1];
					}

					slVectorAdd(&v1, &l->position.location, &v1);
					slVectorAdd(&v2, &l->position.location, &v2);
					slVectorAdd(&v3, &l->position.location, &v3);

					/* in the last square, we don't test the upper and right edge */

					if(z != maxZ && x != maxX) {
						/* PART 1: v1, v3 */

						slTerrainEdgePlaneClip(&v1, &v3, face, sp, &facePlane, ce);

						/* PART 2: v3, v2 */

						slTerrainEdgePlaneClip(&v3, &v2, face, sp, &facePlane, ce);
					}

					if((x == minX && z == minZ && !earlyStart) || (x == maxX && z == maxZ && !lateEnd)) skip = 1;

					/* in the frist and last squares, we don't test the diagonal, */
					/* unless earlyStart and lateEnd have been set. */

					if(!skip) {
						/* PART 3: v2, v1 */

						slTerrainEdgePlaneClip(&v2, &v1, face, sp, &facePlane, ce);
					}
				}
			}
		}
	}

	collisions = ce->points.size();

	if(collisions == 0) return CT_DISJOINT;

	if(!flip) {
		slVectorMul(&ce->normal, -1.0/collisions, &ce->normal);
		ce->n2 = obX;
		ce->n1 = obY;
	} else {
		slVectorMul(&ce->normal, 1.0/collisions, &ce->normal);
		ce->n2 = obY;
		ce->n1 = obX;
	}

	return CT_PENETRATE;
}

int slTerrainEdgePlaneClip(slVector *start, slVector *end, slFace *face, slPosition *position, slPlane *facePlane, slCollision *ce) {
	double headDelta, tailDelta, hdist, tdist;
	int update1, update2;
	slVector edgeVector, headPoint, tailPoint;
	int result;

	/* if the edge is totally excluded from the face's plane, we're done. */

	result = slClipEdgePoints(start, end, face->voronoi, position, face->edgeCount, &update1, &update2, &headDelta, &tailDelta);
	if(!result) return 0;

	/* compute and test the first vertex -- the point on the edge that is */
	/* within the bounds of the plane's voronoi region */

	slVectorSub(end, start, &edgeVector);
	slVectorMul(&edgeVector, headDelta, &headPoint);
	slVectorAdd(&headPoint, start, &headPoint);

	hdist = slPlaneDistance(facePlane, &headPoint);

	if(hdist < MC_TOLERANCE && hdist > -0.1) {
		if(hdist < VC_WARNING_TOLERANCE)
			slMessage(DEBUG_WARN, "deep collision (%f) in terrain/edge clip\n", hdist);

		if(!ce) return CT_PENETRATE;

		ce->depths.push_back(hdist);
		ce->points.push_back(headPoint);

		slVectorSub(&ce->normal, &facePlane->normal, &ce->normal);
	}

	/* compute and test the second vertex -- the point on the edge that is */
	/* within the bounds of the plane's voronoi region.  however, if the   */
	/* edge is completely contained within the voronoi region, then we	 */
	/* know that the same point will come up as the "leading point" in	 */
	/* another edge check, so we'll handle it then. */

	if(update2 == -1) return 0;

	slVectorMul(&edgeVector, tailDelta, &tailPoint);
	slVectorAdd(&tailPoint, start, &tailPoint);
	tdist = slPlaneDistance(facePlane, &tailPoint);

	if(tdist < MC_TOLERANCE && tdist > -0.1) {
		if(tdist < VC_WARNING_TOLERANCE)
			slMessage(DEBUG_WARN, "deep collision (%f) in terrain/edge clip\n", tdist);

		if(!ce) return CT_PENETRATE;

		ce->depths.push_back(tdist);
		ce->points.push_back(tailPoint);

		slVectorSub(&ce->normal, &facePlane->normal, &ce->normal);
	}

	return CT_DISJOINT;
}

/*!
	\brief Returns the distance from the a point to the terrain. 
*/

double slPointTerrainClip(slTerrain *t, slPosition *pp, slPoint *p, slCollision *ce) {
	slVector tp;
	double dist;
	slPlane landPlane;

	slPositionVertex(pp, &p->vertex, &tp);

	// if slTerrainAreaUnderPoint doesn't return 0, then the result is outside the terrain
	
	if(slTerrainAreaUnderPoint(t, &tp, &p->terrainX, &p->terrainZ, &p->terrainQuad)) {
		return 1.0;
	}

	slVectorSet(&landPlane.vertex, (p->terrainX + 1) * t->xscale, t->matrix[p->terrainX + 1][p->terrainZ], p->terrainZ * t->xscale);
	slVectorAdd(&landPlane.vertex, &t->position.location, &landPlane.vertex);
	slVectorCopy(&t->fnormals[p->terrainQuad][p->terrainX][p->terrainZ], &landPlane.normal);

	dist = slPlaneDistance(&landPlane, &tp);

	if(dist < MC_TOLERANCE && dist > -1.0) {
		if(!ce) return dist;

		if(dist < VC_WARNING_TOLERANCE) 
			slMessage(DEBUG_WARN, "deep collision (%f) in point/terrain clip\n", dist);

		ce->depths.push_back(dist);
		ce->points.push_back(tp);

		slVectorAdd(&ce->normal, &landPlane.normal, &ce->normal);
	} 

	return dist;
}

slSerializedTerrain *slSerializeTerrain(slTerrain *t, int *size) {
	slSerializedTerrain *st;
	int x, y;

	*size = sizeof(slSerializedTerrain) + t->side * t->side * sizeof(double);

	st = (slSerializedTerrain*)slMalloc(*size);

	for(x=0;x<t->side;x++) {
		for(y=0;y<t->side;y++) {
			st->values[t->side * y + x] = t->matrix[x][y];
		}
	}

	return st;
}

double slTerrainGetHeight(slTerrain *t, int x, int y) {
	if(x < 0 || y < 0 || x >= t->side || y >= t->side) return 0.0;
	return t->matrix[x][y];
}

void slTerrainSetHeight(slTerrain *t, int x, int y, double height) {
	if(x < 0 || y < 0 || x >= t->side || y >= t->side) return;
	t->matrix[x][y] = height;
	t->_initialized = 0;
}

void slTerrainSetTopColor(slTerrain *t, slVector *color) {
	slVectorCopy(color, &t->topColor);
}

void slTerrainSetBottomColor(slTerrain *t, slVector *color) {
	slVectorCopy(color, &t->bottomColor);
}

int slTerrain::loadGeoTIFF(char *file) {
#if HAVE_LIBTIFF
	TIFF* tif;
	float *row;
	int height, width, x, y;
	unsigned short depth, samples;

	if (!(tif = TIFFOpen(file, "r")))
		return -1;

	TIFFGetField(tif, TIFFTAG_IMAGEWIDTH, &width);
	TIFFGetField(tif, TIFFTAG_IMAGELENGTH, &height);
	TIFFGetField(tif, TIFFTAG_BITSPERSAMPLE, &depth);
	TIFFGetField(tif, TIFFTAG_SAMPLESPERPIXEL, &samples);

	printf("Loading GeoTIFF data (%d x %d) x %d x %d\n", height, width, depth, samples);

	if(height < width) resize(height);
	else resize(width);

	row = new float[width];

	if (width > side) width = side;
	if (height > side) height = side;

	for (y = 0; y < height; y++) {
		TIFFReadScanline(tif, row, y, 0);

		for (x = 0; x < width; x++) {
			matrix[x][y] = 3 * row[x * samples];

			if(matrix[x][y] < 90) matrix[x][y] = 10;
		}
	}

	delete row;
	
	initialize();

	TIFFClose(tif);

	_roam->computeVariance();
#endif
	return 0;
}

double slTerrain::getHeightAtLocation(double x, double z) {
	slVector point;
	int xoff, zoff, quad;

	point.x = x;
	point.z = z;

	slTerrainAreaUnderPoint(this, &point, &xoff, &zoff, &quad);

	if(xoff < 0 || xoff >= side || zoff < 0 || zoff >= side) return 0.0;

	return matrix[xoff][zoff] + position.location.y;
} 
