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

	_shape = new slTerrainShape();

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

void slTerrainShape::updateGeom( slTerrain *inTerrain ) {

	if( _indices )
		delete[] _indices;

	if( _vertices )
		delete[] _vertices;

	_vertexCount = inTerrain -> _side * inTerrain -> _side;

	// In each row, we do ( side - 1 ) triangles, with 3 indices per triangle

	int indicesPerRow = ( 2 * 3 * ( inTerrain -> _side - 1 ) );

	_indexCount = indicesPerRow * ( inTerrain -> _side - 1 );

	_vertices = new float[ _vertexCount * 3 ];
	_indices  = new int[ _indexCount ];

	for( int n = 0; n < inTerrain -> _side; n++ ) {
		for( int m = 0; m < inTerrain -> _side; m++ ) {
			slVector v;
			int vIndex = ( inTerrain -> _side * n + m ); 

			inTerrain -> terrainPoint( n, m, &v );

			_vertices[ vIndex * 3     ] = v.x;
			_vertices[ vIndex * 3 + 1 ] = v.y;
			_vertices[ vIndex * 3 + 2 ] = v.z;

			if( n < inTerrain -> _side - 1 && m < inTerrain -> _side - 1 ) {
				int iIndex = indicesPerRow * n + ( m * 6 );

				_indices[ iIndex     ] = vIndex;
				_indices[ iIndex + 1 ] = vIndex + 1; 
				_indices[ iIndex + 2 ] = vIndex + inTerrain -> _side;

				_indices[ iIndex + 3 ] = vIndex + 1;
				_indices[ iIndex + 4 ] = vIndex + inTerrain -> _side + 1;
				_indices[ iIndex + 5 ] = vIndex + inTerrain -> _side;
			}
		}
	}

	createODEGeom();
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

	( ( slTerrainShape *)_shape ) -> updateGeom( this );
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

/**
 * \brief Initializes this terrain with a fractal generated heightmap.
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

	initialize();
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
	if ( !_initialized ) 
		initialize();

	glPushAttrib( GL_ENABLE_BIT );

	glEnable( GL_LIGHTING );

	_roam->tessellate( camera );

	_polygonsDrawn = _roam->render( camera, _drawMode );

	glPopAttrib();
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

	if ( xoff < 0 || xoff >= _side || zoff < 0 || zoff >= _side ) 
		return 0.0;

	return _matrix[xoff][zoff] + _position.location.y;
}
