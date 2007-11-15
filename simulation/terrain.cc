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
#include "terrain.h"
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

slTerrain::slTerrain( int res, double scale, void *data ) : slWorldObject() {
	slVector location;
	int n;

	_type = WO_TERRAIN;

	_userData = data;

	_side = ( int )pow( 2, res ) + 1;

	_matrix = new float*[_side];

	fnormals[0] = new slVector*[_side - 1];
	fnormals[1] = new slVector*[_side - 1];

	for ( n = 0;n < _side - 1;n++ ) {
		fnormals[0][n] = new slVector[_side - 1];
		fnormals[1][n] = new slVector[_side - 1];
	}

	for ( n = 0;n < _side;n++ ) {
		_matrix[n] = new float[_side];
		memset( _matrix[n], 0, sizeof( float ) * _side );
	}

	_drawMode = 1;

	_desiredPolygons = 8000;

	_repeating = false;

	_xscale = scale;

	_initialized = 0;

	slVectorSet( &location, 0, 0, 0 );

	setLocation( &location );

	_roam = new slRoamPatch( this );
	_roam->setDetailLevel( 10 );
}

/*!
	\brief Sets the draw mode of the terrain.

	0 = wireframe, 1 = fill.
*/

void slTerrain::setDrawMode( int m ) {
	_drawMode = m;
}

/*!
	\brief Resizes the terrain to be able to hold at least side x side points.
	The actual size will be the next power of two above side.
*/

void slTerrain::resize( int s ) {
	int n;

	for ( n = 0;n < _side;n++ ) {
		delete[] _matrix[n];

		if ( n != _side - 1 ) {
			delete[] fnormals[0][n];
			delete[] fnormals[1][n];
		}
	}

	delete[] fnormals[0];

	delete[] fnormals[1];
	delete[] _matrix;

	delete _roam;

	_side = slNextPowerOfTwo( s ) + 1;

	_matrix = new float*[_side];
	fnormals[0] = new slVector*[_side - 1];
	fnormals[1] = new slVector*[_side - 1];

	for ( n = 0;n < _side;n++ ) {
		_matrix[n] = new float[_side];

		if ( n != _side - 1 ) {
			fnormals[0][n] = new slVector[_side - 1];
			fnormals[1][n] = new slVector[_side - 1];
		}

		memset( _matrix[n], 0, sizeof( float ) * _side );
	}

	_roam = new slRoamPatch( this );

	_roam->setDetailLevel( 14 );

	_initialized = 0;
}

/*!
	\brief Sets the x/z scale of the terrain.
*/

void slTerrain::setScale( double scale ) {
	double x;
	slVector half, p;

	x = _side * _xscale / 2;

	slVectorSet( &half, x, 0, x );
	slVectorAdd( &_position.location, &half, &p );

	_xscale = scale;

	setLocation( &p );

	_initialized = 0;
}

/*!
	\brief Sets the midpoint location of the terrain.
*/

void slTerrain::setLocation( slVector *location ) {
	slVector half;
	double x;

	x = _side * _xscale / 2;

	slVectorSet( &half, x, 0, x );
	slVectorSub( location, &half, &_position.location );

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

	for ( x = 0;x < _side;x++ ) {
		for ( z = 0;z < _side;z++ ) {
			if ( _matrix[x][z] > hmax ) hmax = _matrix[x][z];

			if ( _matrix[x][z] < hmin ) hmin = _matrix[x][z];
		}
	}

	_max.x = _max.z = ( _side * _xscale );

	_max.y = hmax;

	_min.x = _min.z = 0.0;
	_min.y = hmin;

	if ( _repeating ) {
		_min.x = _min.z = -DBL_MAX;
		_max.x = _max.z = DBL_MAX;
	}

	slVectorAdd( &_min, &_position.location, &_min );

	slVectorAdd( &_max, &_position.location, &_max );
}

/*!
	\brief Initializes this terrain with a fractal generated heightmap.
*/

void slTerrain::generateFractalTerrain( double h, double height ) {
	float ratio;
	int x, y, jump;
	int oddline, newside;
	float scale;

	// _matrix should be square, and each side should be 2^n + 1

	newside = slNextPowerOfTwo( _side );
	resize( newside );

	jump = _side / 2;
	ratio = pow( 2.0, -h );

	_matrix[0][_side - 1] = _matrix[_side - 1][0] = 0.0;
	_matrix[0][0] = _matrix[_side - 1][_side - 1] = 0.0;

	scale = 1.0;

	while ( jump ) {
		scale *= ratio;

		for ( x = jump;x < _side - 1;x += 2 * jump ) {
			for ( y = jump;y < _side - 1;y += 2 * jump ) {
				_matrix[x][y] = ( _matrix[x - jump][y - jump] + _matrix[x + jump][y - jump] +
				                  _matrix[x - jump][y + jump] + _matrix[x + jump][y + jump] ) / 4;

				_matrix[x][y] += ( slRandomDouble() - .5 ) * scale;
			}
		}

		oddline = 0;

		for ( x = 0;x < _side - 1;x += jump ) {
			oddline = !oddline;

			if ( oddline ) y = jump;
			else y = 0;

			for ( ;y < _side - 1;y += 2 * jump ) {
				_matrix[x][y] = averageDiamondValues( x, y, jump );
				_matrix[x][y] += ( slRandomDouble() - .5 ) * scale;

				if ( !x ) _matrix[_side - 1][y] = _matrix[x][y];

				if ( !y ) _matrix[x][_side - 1] = _matrix[x][y];
			}
		}

		jump /= 2;
	}

	for ( x = 0;x < _side;x++ )
		for ( y = 0;y < _side;y++ )
			_matrix[x][y] *= height;

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

	_heightMin = DBL_MAX;

	slVectorSet( &yAxis, 0, 1, 0 );

	for ( x = 0;x < ( _side - 1 );x++ ) {
		for ( z = 0;z < ( _side - 1 );z++ ) {
			a.x = x * _xscale;
			a.y = _matrix[x][z];
			a.z = z * _xscale;

			b.x = x * _xscale;
			b.y = _matrix[x][z + 1];
			b.z = ( z + 1 ) * _xscale;

			c.x = ( x + 1 ) * _xscale;
			c.y = _matrix[x + 1][z];
			c.z = z * _xscale;

			d.x = ( x + 1 ) * _xscale;
			d.y = _matrix[x + 1][z + 1];
			d.z = ( z + 1 ) * _xscale;

			slVectorSub( &a, &b, &v1 );
			slVectorSub( &c, &b, &v2 );

			slVectorCross( &v2, &v1, &fnormals[0][x][z] );
			slVectorNormalize( &fnormals[0][x][z] );

			slVectorSub( &c, &b, &v1 );
			slVectorSub( &d, &b, &v2 );

			slVectorCross( &v2, &v1, &fnormals[1][x][z] );
			slVectorNormalize( &fnormals[1][x][z] );

			if ( _matrix[x][z] < _heightMin ) _heightMin = _matrix[x][z];

			if ( _matrix[x][z] > max ) max = _matrix[x][z];
		}
	}

	_heightDelta = max - _heightMin;
}

void slTerrain::colorForHeight( double inHeight, slVector *outColor ) {
	float t = inHeight / _heightDelta;

	outColor -> x = ( 1.0 - t ) * _bottomColor.x + t * _topColor.x;
	outColor -> y = ( 1.0 - t ) * _bottomColor.y + t * _topColor.y;
	outColor -> z = ( 1.0 - t ) * _bottomColor.z + t * _topColor.z;
}

void slNormalForFace( slVector *a, slVector *b, slVector *c, slVector *n ) {
	slVector v1, v2;

	slVectorSub( a, b, &v1 );
	slVectorSub( c, b, &v2 );

	slVectorCross( &v2, &v1, n );
	slVectorNormalize( n );
}

float slTerrain::averageDiamondValues( int x, int y, int jump ) {
	// sideminus is the zero based array offset of side

	int sideminus = _side - 1;

	if ( x == 0 ) {
		return ( _matrix[x][y - jump] + _matrix[x + jump][y] +
		         _matrix[sideminus - jump][y] + _matrix[x][y + jump] ) / 4.0;
	} else if ( y == 0 ) {
		return ( _matrix[x][sideminus - jump] + _matrix[x + jump][y] +
		         _matrix[x - jump][y] + _matrix[x][y + jump] ) / 4.0;
	} else if ( x == sideminus ) {
		return ( _matrix[x][y - jump] + _matrix[jump][y] +
		         _matrix[x - jump][y] + _matrix[x][y + jump] ) / 4.0;
	} else if ( y == sideminus ) {
		return ( _matrix[x][y - jump] + _matrix[x + jump][y] +
		         _matrix[x - jump][y] + _matrix[x][jump] ) / 4.0;
	} else {
		return ( _matrix[x][y - jump] + _matrix[x + jump][y] +
		         _matrix[x - jump][y] + _matrix[x][y + jump] ) / 4.0;
	}
}

slTerrain::~slTerrain() {
	int n;

	for ( n = 0; n < _side - 1;n++ ) {
		delete[] fnormals[0][n];
		delete[] fnormals[1][n];
	}

	for ( n = 0; n < _side;n++ ) {
		delete[] _matrix[n];
	}

	delete[] fnormals[0];

	delete[] fnormals[1];
	delete[] _matrix;
}

void slTerrain::draw( slCamera *camera ) {
	if ( !_initialized ) initialize();

	glPushAttrib( GL_ENABLE_BIT );

	glEnable( GL_LIGHTING );

	_roam->tessellate( camera );

	_polygonsDrawn = _roam->render( camera, _drawMode );


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

void slTerrain::facesUnderRange( double minX, double maxX, double minZ, double maxZ,
                                 int *startX, int *endX, int *startZ, int *endZ, int *earlyStart, int *lateEnd ) {

	int localX, localZ;

	minX -= _position.location.x;
	maxX -= _position.location.x;
	minZ -= _position.location.z;
	maxZ -= _position.location.z;

	minX /= _xscale;
	maxX /= _xscale;
	minZ /= _xscale;
	maxZ /= _xscale;

	if ( maxX < 0.0 || maxZ < 0.0 ) {
		*startX = *startZ = *endX = *endZ = -1;
		return;
	}

	if ( minX > ( _side - 1 ) || minZ > ( _side - 1 ) ) {
		*startX = *startZ = *endX = *endZ = _side - 2;
		return;
	}

	if ( minX < 0.0 ) *startX = 0;
	else *startX = ( int )minX;

	if ( minZ < 0.0 ) *startZ = 0;
	else *startZ = ( int )minZ;

	if ( maxX > ( _side - 2 ) ) *endX = _side - 2;
	else *endX = ( int )maxX;

	if ( maxZ > ( _side - 2 ) ) *endZ = _side - 2;
	else *endZ = ( int )maxZ;

	localX = ( int )fmod( minX, 1.0 );

	localZ = ( int )fmod( minZ, 1.0 );

	if (( localX + localZ ) < 1.0 ) *earlyStart = 1;
	else *earlyStart = 0;

	localX = ( int )fmod( minX, 1.0 );

	localZ = ( int )fmod( maxZ, 1.0 );

	if (( localX + localZ ) < 1.0 ) *lateEnd = 0;
	else *lateEnd = 1;
}

/*!
	\brief Takes a point, finds the X, Z and Quad location on the terrain for this point.
*/

int slTerrain::areaUnderPoint( slVector *origpoint, int *x, int *z, int *quad ) {
	double localx, localz;
	double xpos, zpos;
	slVector point;
	int result = 0;

	// transform this point into the terrain's coordinates

	slVectorSub( origpoint, &_position.location, &point );

	// scale down by _xscale on both x and z axes

	xpos = point.x / _xscale;
	zpos = point.z / _xscale;

	localx = fmod( xpos, 1.0 );
	localz = fmod( zpos, 1.0 );

	*x = ( int )xpos;
	*z = ( int )zpos;

	if (( localx + localz ) < 1.0 ) *quad = 0;
	else *quad = 1;

	if ( *x < 0 ) {
		*x = -1;
		*quad = 0;
		result = -1;
	} else if ( *x >= _side - 1 ) {
		*x = _side - 2;
		*quad = 1;
		result = -1;
	}

	if ( *z < 0 ) {
		*z = -1;
		*quad = 0;
		result = -1;
	} else if ( *z >= _side - 1 ) {
		*z = _side - 2;
		*quad = 1;
		result = -1;
	}

	return result;
}

int slTerrainTestPair( slVclipData *vc, int x, int y, slCollision *ce ) {
	slTerrain *terrain;
	slWorldObject *w1 = vc->world->_objects[x];
	slWorldObject *w2 = vc->world->_objects[y];

	if ( w1->getType() != WO_TERRAIN ) {
		const slShape *s = w1->getShape();

		terrain = ( slTerrain* )w2;

		if ( !terrain->_initialized ) terrain->initialize();

		if ( s->_type == ST_SPHERE ) return terrain->sphereClip( vc, x, y, ce, 0 );
		else return terrain->shapeClip( vc, x, y, ce, 0 );
	} else {
		const slShape *s = w2->getShape();

		terrain = ( slTerrain* )w1;

		if ( !terrain->_initialized ) terrain->initialize();

		if ( s->_type == ST_SPHERE ) return terrain->sphereClip( vc, y, x, ce, 1 );
		else return terrain->shapeClip( vc, y, x, ce, 1 );
	}

	return CT_DISJOINT;
}

/*!
	\brief Check for a collision of a sphere against a landscape.
*/

int slTerrain::sphereClip( slVclipData *vc, int obX, int obY, slCollision *ce, int flip ) {
	slPlane landPlane;
	double dist;
	slVector terrainPoint, aveNormal, toSphere;
	int startX, endX, startZ, endZ, earlyStart, lateEnd, x, z, quad;
	int collisions = 0;

	slSphere *ss = ( slSphere* )vc->world->_objects[ obX ]->_shape;
	slPosition *sp = &vc->world->_objects[ obX ]->_position;

	if ( ce ) {
		if ( !flip ) {
			ce->n1 = obY;
			ce->n2 = obX;
		} else {
			ce->n1 = obX;
			ce->n2 = obY;
		}
	}

	slVectorSet( &aveNormal, 0, 0, 0 );

	facesUnderRange( sp->location.x - ss->_radius, sp->location.x + ss->_radius,
	                 sp->location.z - ss->_radius, sp->location.z + ss->_radius,
	                 &startX, &endX, &startZ, &endZ, &earlyStart, &lateEnd );

	for ( x = startX;x <= endX;x++ ) {
		for ( z = startZ;z <= endZ;z++ ) {
			slVectorSet( &landPlane.vertex, x * _xscale, _matrix[x][z], z * _xscale );
			slVectorAdd( &landPlane.vertex, &_position.location, &landPlane.vertex );

			for ( quad = 0;quad < 2;quad++ ) {
				int skip = 0;

				// check to see if we are covering enough of the upper-right and lower-left
				// squares to check the second face

				if ( x == startX && z == startZ && quad == 0 && !earlyStart ) skip = 1;
				else if ( x == endX && z == endZ && quad == 1 && !lateEnd ) skip = 1;

				if ( !skip ) {
					int trivi;
					slVector a, b, c, sloc;

					slVectorCopy( &sp->location, &sloc );

					if ( quad == 0 ) {
						slVectorSet( &a, x, _matrix[x][z], z );
						slVectorSet( &b, x + 1, _matrix[x + 1][z], z );
						slVectorSet( &c, x, _matrix[x][z + 1], z + 1 );
					} else {
						slVectorSet( &a, x + 1, _matrix[x + 1][z + 1], z + 1 );
						slVectorSet( &b, x, _matrix[x][z + 1], z + 1 );
						slVectorSet( &c, x + 1, _matrix[x + 1][z], z );
					}

					a.x *= _xscale;

					a.z *= _xscale;
					b.x *= _xscale;
					b.z *= _xscale;
					c.x *= _xscale;
					c.z *= _xscale;

					slVectorAdd( &a, &_position.location, &a );
					slVectorAdd( &b, &_position.location, &b );
					slVectorAdd( &c, &_position.location, &c );

					trivi = slPointIn2DTriangle( &sloc, &a, &b, &c );

					if ( trivi == 0 ) {
						// no violations, test against the plane

						slVectorCopy( &fnormals[quad][x][z], &landPlane.normal );

						dist = slPlaneDistance( &landPlane, &sp->location ) - ss->_radius;

						if ( dist < MC_TOLERANCE && dist > ( -2 * ss->_radius ) ) {
							collisions++;

							if ( !ce ) return CT_PENETRATE;

							slVectorMul( &landPlane.normal, ss->_radius, &toSphere );

							slVectorSub( &sp->location, &toSphere, &terrainPoint );

							if ( dist < VC_WARNING_TOLERANCE )
								slMessage( DEBUG_WARN, "deep collision (%f) in terrain/sphere clip\n", dist );

							ce->depths.push_back( dist );

							ce->points.push_back( terrainPoint );

							slVectorCopy( &landPlane.normal, &ce->normal );

							if ( !flip ) slVectorMul( &ce->normal, -1, &ce->normal );

							if ( flip ) ce = slNextCollision( vc, obX, obY );
							else ce = slNextCollision( vc, obY, obX );
						}
					} else if ( trivi == 0x01 || trivi == 0x02 || trivi == 0x04 ) {
						// a violation against one of the edges -- test that edge

						slVector *start, *end, collisionPoint;

						if ( trivi == 0x01 ) {
							start = &a;
							end = &b;
						} else if ( trivi == 0x02 ) {
							start = &b;
							end = &c;
						} else {
							start = &c;
							end = &a;
						}

						dist = slPointLineDist( start, end, &sp->location, &collisionPoint ) - ss->_radius;

						if ( dist < MC_TOLERANCE && dist > ( -2 * ss->_radius ) ) {
							collisions++;

							if ( !ce ) return CT_PENETRATE;

							if ( dist < VC_WARNING_TOLERANCE )
								slMessage( DEBUG_WARN, "deep edge collision (%f) in terrain/sphere clip, triangle violation %d\n", dist, trivi );

							ce->depths.push_back( dist );

							ce->points.push_back( collisionPoint );

							slVectorSub( &sp->location, &collisionPoint, &toSphere );

							slVectorNormalize( &toSphere );

							slVectorCopy( &toSphere, &ce->normal );

							if ( !flip ) slVectorMul( &ce->normal, -1, &ce->normal );

							if ( flip ) ce = slNextCollision( vc, obX, obY );
							else ce = slNextCollision( vc, obY, obX );
						}
					} else {
						slVector *point;

						if ( trivi == 0x03 ) {
							point = &b;
						} else if ( trivi == 0x05 ) {
							point = &a;
						} else {
							point = &c;
						}

						slVectorSub( &sp->location, point, &toSphere );

						dist = slVectorLength( &toSphere ) - ss->_radius;

						if ( dist < MC_TOLERANCE && dist > ( -2 * ss->_radius ) ) {
							collisions++;

							if ( !ce ) return CT_PENETRATE;

							if ( dist < VC_WARNING_TOLERANCE )
								slMessage( DEBUG_WARN, "deep point collision (%f) in terrain/sphere clip, triangle violation %d\n", dist, trivi );

							ce->depths.push_back( dist );

							ce->points.push_back( *point );

							slVectorNormalize( &toSphere );

							slVectorCopy( &toSphere, &ce->normal );

							if ( !flip ) slVectorMul( &ce->normal, -1, &ce->normal );

							if ( flip ) ce = slNextCollision( vc, obX, obY );
							else ce = slNextCollision( vc, obY, obX );
						}
					}
				}
			}
		}
	}

	if ( collisions == 0 ) return CT_DISJOINT;

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

int slPointIn2DTriangle( slVector *vertex, slVector *a, slVector *b, slVector *c ) {
	float AB = (( vertex->z - a->z ) * ( b->x - a->x ) ) - (( vertex->x - a->x ) * ( b->z - a->z ) );
	float BC = (( vertex->z - b->z ) * ( c->x - b->x ) ) - (( vertex->x - b->x ) * ( c->z - b->z ) );
	float CA = (( vertex->z - c->z ) * ( a->x - c->x ) ) - (( vertex->x - c->x ) * ( a->z - c->z ) );

	int violation = 0;

	if ( AB < 0.0 ) violation |= 0x01;

	if ( BC < 0.0 ) violation |= 0x02;

	if ( CA < 0.0 ) violation |= 0x04;

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

int slTerrain::shapeClip( slVclipData *vc, int obX, int obY, slCollision *ce, int flip ) {
	std::vector<slPoint*>::iterator pi;
	std::vector<slFace*>::iterator fi;
	int collisions = 0;

	slShape *ss = vc->world->_objects[obX]->_shape;
	slPosition *sp = &vc->world->_objects[obX]->_position;

	slVectorSet( &ce->normal, 0, 0, 0 );

	for ( pi = ss->points.begin(); pi != ss->points.end(); pi++ ) {
		pointClip( sp, *pi, ce );
	}

	for ( fi = ss->faces.begin(); fi != ss->faces.end(); fi++ ) {
		slFace *face = *fi;
		slPlane facePlane;
		slVector y;
		int np;

		slVectorSet( &y, 0, -1, 0 );

		slPositionPlane( sp, &face->plane, &facePlane );

		/* is the plane well oriented towards the terrain?  since the shape is	*/
		/* convex, and the terrain cannot be more than 90 degrees from (0, 1, 0)  */
		/* then if we're more than 90 from (0, -1, 0), then there exists a better */
		/* plane containing the same edge that we should use instead. */

		if ( slVectorDot( &y, &facePlane.normal ) > 0.0 ) {
			int minX = _side, minZ = _side, maxX = 0, maxZ = 0;
			int earlyStart = 0, lateEnd = 0;
			int x, z;

			for ( np = 0;np < face->edgeCount;np++ ) {
				slPoint *facePoint = face->points[np];

				/* find the min/max range for the edges we'l need to test */

				if ( facePoint->terrainX > maxX ) maxX = facePoint->terrainX;

				if ( facePoint->terrainX < minX ) minX = facePoint->terrainX;

				if ( facePoint->terrainZ > maxZ ) {
					maxZ = facePoint->terrainZ;
					lateEnd = facePoint->terrainQuad;
				}

				if ( facePoint->terrainZ < minZ ) {
					minZ = facePoint->terrainZ;
					earlyStart = !facePoint->terrainQuad;
				}
			}

			for ( x = minX;x <= maxX;x++ ) {
				for ( z = minZ;z <= maxZ;z++ ) {
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

					if ( x != -1 && z != -1 ) {
						v2.z = z * _xscale;
						v1.z = v3.z = ( z + 1 ) * _xscale;

						v1.x = x * _xscale;
						v2.x = v3.x = ( x + 1 ) * _xscale;

						v1.y = _matrix[x][z + 1];
						v2.y = _matrix[x + 1][z];
						v3.y = _matrix[x + 1][z + 1];
					} else if ( x == -1 ) {
						v2.z = z * _xscale;
						v1.z = v3.z = ( z + 1 ) * _xscale;

						v1.x = 0;
						v2.x = v3.x = _xscale;

						v1.y = _matrix[0][z + 1];
						v2.y = _matrix[1][z];
						v3.y = _matrix[1][z + 1];
					} else {
						v2.z = 0;
						v1.z = v3.z = _xscale;

						v1.x = x * _xscale;
						v2.x = v3.x = ( x + 1 ) * _xscale;

						v1.y = _matrix[x][1];
						v2.y = _matrix[x + 1][0];
						v3.y = _matrix[x + 1][1];
					}

					slVectorAdd( &v1, &_position.location, &v1 );

					slVectorAdd( &v2, &_position.location, &v2 );
					slVectorAdd( &v3, &_position.location, &v3 );

					/* in the last square, we don't test the upper and right edge */

					if ( z != maxZ && x != maxX ) {
						/* PART 1: v1, v3 */

						slTerrainEdgePlaneClip( &v1, &v3, face, sp, &facePlane, ce );

						/* PART 2: v3, v2 */

						slTerrainEdgePlaneClip( &v3, &v2, face, sp, &facePlane, ce );
					}

					if (( x == minX && z == minZ && !earlyStart ) || ( x == maxX && z == maxZ && !lateEnd ) ) skip = 1;

					/* in the frist and last squares, we don't test the diagonal, */
					/* unless earlyStart and lateEnd have been set. */

					if ( !skip ) {
						/* PART 3: v2, v1 */

						slTerrainEdgePlaneClip( &v2, &v1, face, sp, &facePlane, ce );
					}
				}
			}
		}
	}

	collisions = ce->points.size();

	if ( collisions == 0 ) return CT_DISJOINT;

	if ( !flip ) {
		slVectorMul( &ce->normal, -1.0 / collisions, &ce->normal );
		ce->n2 = obX;
		ce->n1 = obY;
	} else {
		slVectorMul( &ce->normal, 1.0 / collisions, &ce->normal );
		ce->n2 = obY;
		ce->n1 = obX;
	}

	return CT_PENETRATE;
}

int slTerrainEdgePlaneClip( slVector *start, slVector *end, slFace *face, slPosition *position, slPlane *facePlane, slCollision *ce ) {
	double headDelta, tailDelta, hdist, tdist;
	int update1, update2;
	slVector edgeVector, headPoint, tailPoint;
	int result;

	/* if the edge is totally excluded from the face's plane, we're done. */

	result = slClipEdgePoints( start, end, face->voronoi, position, face->edgeCount, &update1, &update2, &headDelta, &tailDelta );

	if ( !result ) return 0;

	/* compute and test the first vertex -- the point on the edge that is */
	/* within the bounds of the plane's voronoi region */

	slVectorSub( end, start, &edgeVector );

	slVectorMul( &edgeVector, headDelta, &headPoint );

	slVectorAdd( &headPoint, start, &headPoint );

	hdist = slPlaneDistance( facePlane, &headPoint );

	if ( hdist < MC_TOLERANCE && hdist > -0.1 ) {
		if ( hdist < VC_WARNING_TOLERANCE )
			slMessage( DEBUG_WARN, "deep collision (%f) in terrain/edge clip\n", hdist );

		if ( !ce ) return CT_PENETRATE;

		ce->depths.push_back( hdist );

		ce->points.push_back( headPoint );

		slVectorSub( &ce->normal, &facePlane->normal, &ce->normal );
	}

	/* compute and test the second vertex -- the point on the edge that is */
	/* within the bounds of the plane's voronoi region.  however, if the   */
	/* edge is completely contained within the voronoi region, then we	 */
	/* know that the same point will come up as the "leading point" in	 */
	/* another edge check, so we'll handle it then. */

	if ( update2 == -1 ) return 0;

	slVectorMul( &edgeVector, tailDelta, &tailPoint );

	slVectorAdd( &tailPoint, start, &tailPoint );

	tdist = slPlaneDistance( facePlane, &tailPoint );

	if ( tdist < MC_TOLERANCE && tdist > -0.1 ) {
		if ( tdist < VC_WARNING_TOLERANCE )
			slMessage( DEBUG_WARN, "deep collision (%f) in terrain/edge clip\n", tdist );

		if ( !ce ) return CT_PENETRATE;

		ce->depths.push_back( tdist );

		ce->points.push_back( tailPoint );

		slVectorSub( &ce->normal, &facePlane->normal, &ce->normal );
	}

	return CT_DISJOINT;
}

/*!
	\brief Returns the distance from the a point to the terrain.
*/

double slTerrain::pointClip( slPosition *pp, slPoint *p, slCollision *ce ) {
	slVector tp;
	double dist;
	slPlane landPlane;

	slPositionVertex( pp, &p->vertex, &tp );

	// if areaUnderPoint doesn't return 0, then the result is outside the terrain

	if ( areaUnderPoint( &tp, &p->terrainX, &p->terrainZ, &p->terrainQuad ) ) {
		return 1.0;
	}

	slVectorSet( &landPlane.vertex, ( p->terrainX + 1 ) * _xscale, _matrix[p->terrainX + 1][p->terrainZ], p->terrainZ * _xscale );

	slVectorAdd( &landPlane.vertex, &_position.location, &landPlane.vertex );
	slVectorCopy( &fnormals[p->terrainQuad][p->terrainX][p->terrainZ], &landPlane.normal );

	dist = slPlaneDistance( &landPlane, &tp );

	if ( dist < MC_TOLERANCE && dist > -1.0 ) {
		if ( !ce ) return dist;

		if ( dist < VC_WARNING_TOLERANCE )
			slMessage( DEBUG_WARN, "deep collision (%f) in point/terrain clip\n", dist );

		ce->depths.push_back( dist );

		ce->points.push_back( tp );

		slVectorAdd( &ce->normal, &landPlane.normal, &ce->normal );
	}

	return dist;
}

slSerializedTerrain *slSerializeTerrain( slTerrain *t, int *size ) {
	slSerializedTerrain *st;
	int x, y;

	*size = sizeof( slSerializedTerrain ) + t->_side * t->_side * sizeof( double );

	st = ( slSerializedTerrain* )slMalloc( *size );

	for ( x = 0;x < t->_side;x++ ) {
		for ( y = 0;y < t->_side;y++ ) {
			st->values[t->_side * y + x] = t->_matrix[x][y];
		}
	}

	return st;
}

double slTerrain::getHeight( int x, int y ) {
	if ( x < 0 || y < 0 || x >= _side || y >= _side ) return 0.0;

	return _matrix[x][y];
}

void slTerrain::setHeight( int x, int y, double height ) {
	if ( x < 0 || y < 0 || x >= _side || y >= _side ) return;

	_matrix[x][y] = height;

	_initialized = 0;
}

void slTerrain::setTopColor( slVector *color ) {
	slVectorCopy( color, &_topColor );
}

void slTerrain::setBottomColor( slVector *color ) {
	slVectorCopy( color, &_bottomColor );
}

int slTerrain::loadGeoTIFF( char *file ) {
#if HAVE_LIBTIFF
	TIFF* tif;
	float *row;
	int height, width, x, y;
	unsigned short depth, samples;

	if ( !( tif = TIFFOpen( file, "r" ) ) )
		return -1;

	TIFFGetField( tif, TIFFTAG_IMAGEWIDTH, &width );

	TIFFGetField( tif, TIFFTAG_IMAGELENGTH, &height );

	TIFFGetField( tif, TIFFTAG_BITSPERSAMPLE, &depth );

	TIFFGetField( tif, TIFFTAG_SAMPLESPERPIXEL, &samples );

	printf( "Loading GeoTIFF data (%d x %d) x %d x %d\n", height, width, depth, samples );

	if ( height < width ) resize( height );
	else resize( width );

	row = new float[width];

	if ( width > _side ) width = _side;

	if ( height > _side ) height = _side;

	for ( y = 0; y < height; y++ ) {
		TIFFReadScanline( tif, row, y, 0 );

		for ( x = 0; x < width; x++ ) {
			_matrix[x][y] = 3 * row[x * samples];

			if ( _matrix[x][y] < 90 ) _matrix[x][y] = 10;
		}
	}

	delete row;

	initialize();

	TIFFClose( tif );

	_roam->computeVariance();
#endif
	return 0;
}

double slTerrain::getHeightAtLocation( double x, double z ) {
	slVector point;
	int xoff, zoff, quad;

	point.x = x;
	point.z = z;

	areaUnderPoint( &point, &xoff, &zoff, &quad );

	if ( xoff < 0 || xoff >= _side || zoff < 0 || zoff >= _side ) return 0.0;

	return _matrix[xoff][zoff] + _position.location.y;
}
