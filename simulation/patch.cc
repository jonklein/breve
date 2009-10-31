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
#include "glIncludes.h"
#include "camera.h"
#include "vclip.h"
#include "vclipData.h"

#ifdef WINDOWS
#include <windows.h>

PFNGLTEXIMAGE3DPROC wglTexImage3D = NULL;
PFNGLTEXSUBIMAGE3DPROC wglTexSubImage3D = NULL;

#define glTexImage3D     wglTexImage3D
#define glTexSubImage3D  wglTexSubImage3D

#endif

/**
 *  slPatch default constructor.
 */

slPatch::slPatch() :  data( NULL ) {}

/**
 *  slPatch base constructor.
 *
 *
 *  @param theGrid the slPatchGrid parent object.
 */

slPatch::slPatch( slPatchGrid* theGrid ) :  grid( theGrid ) {
	// your code here ;-)
}

slPatch::slPatch( slPatchGrid* theGrid,
                  slVector* theLocation,

                  const int theColorOffset )
		:   colorOffset( theColorOffset ),
		grid( theGrid ) {

	slVectorCopy( theLocation, &this->location );
}

/**
 * Sets the color of a patch.
 */

void slPatch::setColor( slVector *color ) {
	if ( color->x > 1.0 ) color->x = 1.0;
	else if ( color->x < 0.0 ) color->x = 0.0;

	if ( color->y > 1.0 ) color->y = 1.0;
	else if ( color->y < 0.0 ) color->y = 0.0;

	if ( color->z > 1.0 ) color->z = 1.0;
	else if ( color->z < 0.0 ) color->z = 0.0;

	grid -> colors[ this->colorOffset     ] = ( unsigned char )( 255 * color->x );
	grid -> colors[ this->colorOffset + 1 ] = ( unsigned char )( 255 * color->y );
	grid -> colors[ this->colorOffset + 2 ] = ( unsigned char )( 255 * color->z );

	grid -> _textureNeedsUpdate = true;
}

/**
 * sets the transparency of a patch.
 */

void slPatch::setTransparency( double transparency ) {
	if ( transparency > 1.0 ) transparency = 1.0;
	else if ( transparency < 0.0 ) transparency = 0.0;

	grid->colors[ this->colorOffset + 3 ] = ( unsigned char )( 255 * transparency );
	grid->_textureNeedsUpdate = true;
}

/**
 *    getColor gets the color of the patch
 *
 *    slPatch base constructor requires a parent grid
 *    location and color offset
 */

void slPatch::getColor( slVector *color ) {
	color->x = grid->colors[ this->colorOffset     ] / 255.0;
	color->y = grid->colors[ this->colorOffset + 1 ] / 255.0;
	color->z = grid->colors[ this->colorOffset + 2 ] / 255.0;
}

void slPatch::setData( void *data ) {

	this->data = data;

}

void* slPatch::getData() {
	return this->data;
}

void slPatch::getLocation( slVector *location ) {
	slVectorCopy( &this->location, location );
}

/**
 *  slPatchGrid default constructor.
 */

slPatchGrid::slPatchGrid() {
	_drawWithTexture = 1;
	_texture = NULL;
	_textureNeedsUpdate = true;
}

/**
 *  base constructor.
 *
 *  @param theLocation the location as a vector.
 *  @param theGrid the slPatchGrid parent object.
 *  @param theColorOffset the texture offset for GL display
 */

slPatchGrid::slPatchGrid( const slVector *center, const slVector *patchSize, const int x, const int y, const int z )
		:  _xSize( x ),
		_ySize( y ),
		_zSize( z ) {
	int a, b, c;

	_drawWithTexture = true;
	_texture = NULL;
	
	fillCubeBuffer();

#ifdef WINDOWS
	// oh windows, why do you have to be such a douchebag about everything?!

	if( !wglTexImage3D ) 
		wglTexImage3D = ( PFNGLTEXIMAGE3DPROC )wglGetProcAddress( "glTexImage3D" );

	if( !wglTexSubImage3D ) 
		wglTexSubImage3D = ( PFNGLTEXSUBIMAGE3DPROC )wglGetProcAddress( "glTexSubImage3D" );
#endif

	// I don't know what do do about this code--
	// before it would have left a dangling grid object I think
	// and it should be an exception now...

	if ( x < 1 || y < 1 || z < 1 )
		throw slException( std::string( "error instantiating PatchGrid: invalid dimensions" ) );

	this->patches = new slPatch**[z];

	this->_textureX = slNextPowerOfTwo( x );
	this->_textureY = slNextPowerOfTwo( y );
	this->_textureZ = slNextPowerOfTwo( z );

	this->colors = new unsigned char[ this->_xSize * this->_ySize * this->_zSize * 4 ];
	memset( this->colors, 0, this->_xSize * this->_ySize * this->_zSize * 4 );

	slVectorCopy( patchSize, &this->patchSize );

	this->startPosition.x = ( -( patchSize->x * x ) / 2 ) + center->x;
	this->startPosition.y = ( -( patchSize->y * y ) / 2 ) + center->y;
	this->startPosition.z = ( -( patchSize->z * z ) / 2 ) + center->z;

	for ( c = 0;c < z;c++ ) {
		this->patches[c] = new slPatch*[x];

		for ( b = 0;b < x;b++ ) {
			this->patches[c][b] = new slPatch[y]; // uses default constructor

			for ( a = 0;a < y;a++ ) {
				this->patches[c][b][a].grid = this;
				this->patches[c][b][a].location.z = this->startPosition.z + c * patchSize->z;
				this->patches[c][b][a].location.x = this->startPosition.x + b * patchSize->x;
				this->patches[c][b][a].location.y = this->startPosition.y + a * patchSize->y;
				this->patches[c][b][a].colorOffset = 4 * ( ( c * this->_xSize * this->_ySize ) + ( b * this->_ySize ) + a );
			}
		}
	}

	this->drawSmooth = 0;

}

/**
 *  slPatch destructor.
 *
 */
slPatchGrid::~slPatchGrid() {
	unsigned int b, c;

	for ( c = 0;c < _zSize;c++ ) {
		for ( b = 0;b < _xSize;b++ ) {
			delete[] patches[c][b];
		}

		delete[] patches[c];
	}

	delete[] patches;

	delete[] colors;

	if( _texture )
		delete _texture;
}

/**
 *  getPatchAtLocation returns the patch which contains the point.
 *
 */
slPatch* slPatchGrid::getPatchAtLocation( const slVector *location ) {
	double x, y, z;

	x = (( location->x - startPosition.x ) / patchSize.x );
	y = (( location->y - startPosition.y ) / patchSize.y );
	z = (( location->z - startPosition.z ) / patchSize.z );

	if ( x < 0 || x >= _xSize ) return NULL;

	if ( y < 0 || y >= _ySize ) return NULL;

	if ( z < 0 || z >= _zSize ) return NULL;

	return &patches[int( z )][int( x )][int( y )];
}

/*!
	\brief Preforms a basic cell-based collision detection -- currently
	experimental.
*/

void slPatchGrid::assignObjectsToPatches( slWorld *w ) {
	unsigned int x, y, z;
	// unsigned int ob;
	std::vector< slWorldObject* >::iterator wo;

	slVclipData *vc = w->_clipData;

	slCollision *ce = slNextCollision( vc );

	for ( z = 0; z < _zSize; z++ ) {
		for ( y = 0; y < _ySize; y++ ) {
			for ( x = 0; x < _xSize; x++ ) {
				patches[z][x][y]._objectsInPatch.clear();
			}
		}
	}

	for ( x = 0; x < w->_objects.size(); x++ ) {
		slWorldObject *w1 = w->_objects[ x ];
		slPatch *p = getPatchAtLocation( &w1->getPosition().location );

		if ( p ) {
			std::vector< slPatch* >::iterator pi;

			for ( pi = p->_neighbors.begin(); pi != p->_neighbors.end(); pi++ ) {
				slPatch *neighbor = *pi;

				std::vector< int >::iterator ii;

				for ( ii = neighbor->_objectsInPatch.begin(); ii != neighbor->_objectsInPatch.end(); ii++ ) {
					y = *ii;

					slPairFlags flags = slVclipPairFlagValue( vc, x, y );

					if ( flags & BT_UNKNOWN ) {
						// the UNKNOWN flag indicates that we have not yet preformed a callback to
						// determine whether further collision detection is necessary.

						flags = vc->initPairFlags( x, y );
					}

					if ( flags & BT_CHECK ) {
						slCollisionCandidate c( vc, x, y );

						if ( vc->testPair( &c, ce, 0 ) == CT_PENETRATE ) 
							ce = slNextCollision( vc );
					}
				}
			}

			p->_objectsInPatch.push_back( x );
		}
	}

	vc->collisionCount--;
}

/**
 * Copies the contents of a 3D matrix to one z-slice of a PatchGrid.
 */

void slPatchGrid::copyColorFrom3DMatrix( slBigMatrix3DGSL *m, int channel, double scale ) {
	int x, y, z;
	int _xSize, _ySize, _zSize;
	float* mData;

	unsigned int chemTDA = m->yDim();
	unsigned int chemXY = m->xDim() * m->yDim();
	unsigned int yStride = this->_xSize * 4;
	unsigned int zStride = this->_xSize * this->_ySize * 4;

	mData = m->getGSLVector()->data;

	_xSize = m->xDim();
	_ySize = m->yDim();
	_zSize = m->zDim();

	_textureNeedsUpdate = true;

	for ( z = 0; z < _zSize; z++ ) {

		int zOff = ( z * zStride );

		for ( x = 0; x < _xSize; x++ ) {
			unsigned int mrowOffset = ( z * chemXY ) + ( x * chemTDA );
			unsigned int crowOffset = zOff + ( x << 2 ) + channel;

			for ( y = 0; y < _ySize; y++ ) {
				float value = scale * mData[ mrowOffset + y ];

				if ( value > 1.0 ) value = 1.0;

				this->colors[ ( y * yStride ) + crowOffset ] = ( unsigned char )( 255 * value );
			}
		}
	}
}

/**
 *	\brief Draws the patch grid without using 3D textures.
 */

void slPatchGrid::drawWithout3DTexture( slRenderGL &inRenderer, slCamera *camera ) {
	int z, y, x;
	unsigned int zVal, yVal, xVal;
	int zMid = 0, yMid = 0, xMid = 0;
	slPatch *patch;
	slVector translation, origin;
		
	// we want to always draw from back to front for the
	// alpha blending to work.  figure out the points
	// closest to the camera.

	slVectorAdd( &camera->_location, &camera->_target, &origin );

	xMid = ( int )(( origin.x - startPosition.x ) / patchSize.x );
	if ( xMid < 0 ) xMid = 0;
	if ( xMid >= (int)_xSize ) xMid = _xSize - 1;

	yMid = ( int )(( origin.y - startPosition.y ) / patchSize.y );
	if ( yMid < 0 ) yMid = 0;
	if ( yMid >= (int)_ySize ) yMid = _ySize - 1;

	zMid = ( int )(( origin.z - startPosition.z ) / patchSize.z );
	if ( zMid < 0 ) zMid = 0;
	if ( zMid >= (int)_zSize ) zMid = _zSize - 1;

	inRenderer.SetBlendMode( slBlendAlpha );
	
	for ( z = 0; z < ( int )_zSize; z++ ) {
		if ( z < ( int )zMid ) zVal = z;
		else zVal = ( _zSize - 1 ) - ( z - zMid );

		translation.z = startPosition.z + patchSize.z * zVal;

		for ( x = 0;x < ( int )_xSize ;x++ ) {
			if ( x < ( int )xMid ) xVal = x;
			else xVal = ( _xSize - 1 ) - ( x - xMid );

			translation.x = startPosition.x + patchSize.x * xVal;

			for ( y = 0;y < ( int )_ySize;y++ ) {
				if ( y < ( int )yMid ) yVal = y;
				else yVal = ( _ySize - 1 ) - ( y - yMid );

				patch = &patches[ zVal ][ xVal ][ yVal ];

				if ( colors[ patch->colorOffset + 3 ] != 255 ) {
					translation.y = startPosition.y + patchSize.y * yVal;

					inRenderer.PushMatrix( slMatrixGeometry );

					inRenderer.SetBlendColor( &colors[ patch->colorOffset ] );
					inRenderer.Translate( slMatrixGeometry, translation.x, translation.y, translation.z );
					inRenderer.Scale( slMatrixGeometry, patchSize.x, patchSize.y, patchSize.z );

					_cubeBuffer.bind();
					_cubeBuffer.draw( VB_TRIANGLE_STRIP );		
					_cubeBuffer.unbind();

					inRenderer.PopMatrix( slMatrixGeometry );
				}
			}
		}
	}
}


void slPatchGrid::setDrawWithTexture( bool t ) {

	_drawWithTexture = t;
}

/**
 *	\brief Draws a set of patches using a volumetric 3D texture.
 *
 *	This may not be supported on older OpenGL implementations, so the alternative
 *	\ref drawWithout3DTexture may be used instead.  Drawing without the 3D texture
 *	is slower than drawing with a 3D texture.
 */

void slPatchGrid::draw( slRenderGL &inRenderer, slCamera *camera ) {
	slVector origin, diff, adiff, size;

#ifdef WINDOWS
	if( !glTexImage3D || !glTexSubImage3D ) 
		_drawWithTexture = false;
#endif


#ifdef OPENGLES
	// No GL_TEXTURE_3D support with OpenGL ES
	_drawWithTexture = false;
#endif

	if ( !_drawWithTexture ) 
		return drawWithout3DTexture( inRenderer, camera );

	if ( !_texture ) {
		_texture = new slTexture2D();
		_texture -> createTextureID();
		glEnable( GL_TEXTURE_3D );
		glBindTexture( GL_TEXTURE_3D, _texture -> _textureID );
		glTexImage3D( GL_TEXTURE_3D, 0, GL_RGBA, _textureX, _textureY, _textureZ, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL );
	}

	slVectorAdd( &camera->_location, &camera->_target, &origin );

	diff.x = ( origin.x - ( startPosition.x + ( _xSize / 2 ) * patchSize.x ) );
	diff.y = ( origin.y - ( startPosition.y + ( _ySize / 2 ) * patchSize.y ) );
	diff.z = ( origin.z - ( startPosition.z + ( _zSize / 2 ) * patchSize.z ) );

	adiff.x = fabs( diff.x );
	adiff.y = fabs( diff.y );
	adiff.z = fabs( diff.z );

	glDisable( GL_CULL_FACE );

	glColor4f( 1, 1, 1, 1 );

	glEnable( GL_TEXTURE_3D );
	glBindTexture( GL_TEXTURE_3D, _texture -> _textureID );

	if ( drawSmooth ) {
		glTexParameteri( GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
		glTexParameteri( GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
	} else {
		glTexParameteri( GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_NEAREST );
		glTexParameteri( GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_NEAREST );
	}

	glTexParameteri( GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_CLAMP );
	glTexParameteri( GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_CLAMP );
	glTexParameteri( GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, GL_CLAMP );

	glTexEnvf( GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE );

	if( _textureNeedsUpdate ) { 
		glPixelTransferf( GL_ALPHA_SCALE, 3.0f );
		glTexSubImage3D( GL_TEXTURE_3D, 0, 0, 0, 0, _ySize, _xSize, _zSize, GL_RGBA, GL_UNSIGNED_BYTE, colors );
		glPixelTransferf( GL_ALPHA_SCALE, 1.0f );

		_textureNeedsUpdate = false;
	}

	_quadBuffer.bind();
	glBegin( GL_QUADS );

	size.x = patchSize.x * _xSize;
	size.y = patchSize.y * _ySize;
	size.z = patchSize.y * _zSize;

	if ( adiff.x > adiff.z && adiff.x > adiff.y )
		textureDrawXPass( size, diff.x > 0 );
	else if ( adiff.y > adiff.z )
		textureDrawYPass( size, diff.y > 0 );
	else
		textureDrawZPass( size, diff.z > 0 );

	glEnd();

	_quadBuffer.unbind();

	glDisable( GL_TEXTURE_3D );

	return;
}

void slPatchGrid::textureDrawYPass( slVector &size, int dir ) {
	float y, start, end, inc;

	if ( dir ) {
		start = 0;
		end = _ySize;
		inc = .3;
	} else {
		start = _ySize;
		inc = -.3;
		end = -1;
	}

#ifndef OPENGLES
	for ( y = start; floor( y ) != end; y += inc ) {
		double yp;

		yp = startPosition.y + ( y * patchSize.y );

		glTexCoord3f( 0, ( y / _textureY ), 0 );
		glVertex3f( startPosition.x, yp, startPosition.z );

		glTexCoord3f(( float )_xSize / _textureX, ( y / _textureY ), 0 );
		glVertex3f( startPosition.x + size.x, yp, startPosition.z );

		glTexCoord3f(( float )_xSize / _textureX, ( y / _textureY ), ( float )_zSize / _textureZ );
		glVertex3f( startPosition.x + size.x, yp, startPosition.z + size.z );

		glTexCoord3f( 0, ( y / _textureY ), ( float )_zSize / _textureZ );
		glVertex3f( startPosition.x, yp, startPosition.z + size.z );
	}
#endif
}

void slPatchGrid::textureDrawXPass( slVector &size, int dir ) {
	float x, start, end, inc;

	if ( dir ) {
		start = 0;
		end = _xSize;
		inc = .3;
	} else {
		start = _xSize;
		inc = -.3;
		end = -1;
	}
#ifndef OPENGLES
	for ( x = start;floor( x ) != end;x += inc ) {
		double xp;

		xp = startPosition.x + ( x * patchSize.x );

		glTexCoord3f( ( x / _textureX ), 0, 0 );
		glVertex3f( xp, startPosition.y, startPosition.z );

		glTexCoord3f( ( x / _textureX ), ( float )_ySize / _textureY, 0 );
		glVertex3f( xp, startPosition.y + size.y, startPosition.z );

		glTexCoord3f(( x / _textureX ), ( float )_ySize / _textureY, ( float )_zSize / _textureZ );
		glVertex3f( xp, startPosition.y + size.y, startPosition.z + size.z );

		glTexCoord3f(( x / _textureX ), 0, ( float )_zSize / _textureZ );
		glVertex3f( xp, startPosition.y, startPosition.z + size.z );
	}
#endif
}

void slPatchGrid::textureDrawZPass( slVector &size, int dir ) {
	float z, start, end, inc;

	if ( dir ) {
		start = 0;
		inc = .3;
		end = _zSize;
	} else {
		inc = -.3;
		start = _zSize - inc;
		end = -1;
	}

#ifndef OPENGLES
	for ( z = start;floor( z ) != end;z += inc ) {
		double zp;

		zp = startPosition.z + ( z * patchSize.z );

		glTexCoord3f( 0, 0, ( z / _textureZ ) );
		glVertex3f( startPosition.x, startPosition.y, zp );

		glTexCoord3f(( float )_xSize / _textureX, 0, ( z / _textureZ ) );
		glVertex3f( startPosition.x + size.x, startPosition.y, zp );

		glTexCoord3f(( float )_xSize / _textureX, ( float )_ySize / _textureY, ( z / _textureZ ) );
		glVertex3f( startPosition.x + size.x, startPosition.y + size.y, zp );

		glTexCoord3f( 0, ( float )_ySize / _textureY, ( z / _textureZ ) );
		glVertex3f( startPosition.x, startPosition.y + size.y, zp );
	}
#endif
}

slPatch* slPatchGrid::getPatchAtIndex( int x, int y, int z ) {
	if ( x < 0 || x >= ( int )_xSize ) return NULL;
	if ( y < 0 || y >= ( int )_ySize ) return NULL;
	if ( z < 0 || z >= ( int )_zSize ) return NULL;

	return &patches[ z ][ x ][ y ];
}

void slPatchGrid::setDataAtIndex( int x, int y, int z, void *data ) {
	if ( x < 0 || x >= ( int )_xSize ) return;
	if ( y < 0 || y >= ( int )_ySize ) return;
	if ( z < 0 || z >= ( int )_zSize ) return;

	this->patches[ z ][ x ][ y ].data = data;
}

void slPatchGrid::fillCubeBuffer() {
	_cubeBuffer.resize( 14, VB_XYZ );
	
	float *v;
	
	v = _cubeBuffer.vertex( 0 );
	v[ 0 ] = -.5; v[ 1 ] = -.5; v[ 2 ] = -.5;

	v = _cubeBuffer.vertex( 1 );
	v[ 0 ] =  .5; v[ 1 ] = -.5; v[ 2 ] = -.5;
	
	v = _cubeBuffer.vertex( 2 );
	v[ 0 ] = -.5; v[ 1 ] = -.5; v[ 2 ] =  .5;
	
	v = _cubeBuffer.vertex( 3 );
	v[ 0 ] =  .5; v[ 1 ] = -.5; v[ 2 ] =  .5;
	
	v = _cubeBuffer.vertex( 4 );
	v[ 0 ] =  .5; v[ 1 ] =  .5; v[ 2 ] =  .5;
	
	v = _cubeBuffer.vertex( 5 );
	v[ 0 ] =  .5; v[ 1 ] = -.5; v[ 2 ] = -.5;
	
	v = _cubeBuffer.vertex( 6 );
	v[ 0 ] =  .5; v[ 1 ] =  .5; v[ 2 ] = -.5;
	
	v = _cubeBuffer.vertex( 7 );
	v[ 0 ] = -.5; v[ 1 ] = -.5; v[ 2 ] = -.5;
	
	v = _cubeBuffer.vertex( 8 );
	v[ 0 ] = -.5; v[ 1 ] =  .5; v[ 2 ] = -.5;
	
	v = _cubeBuffer.vertex( 9 );
	v[ 0 ] = -.5; v[ 1 ] = -.5; v[ 2 ] =  .5;
	
	v = _cubeBuffer.vertex( 10 );
	v[ 0 ] = -.5; v[ 1 ] =  .5; v[ 2 ] =  .5;
	
	v = _cubeBuffer.vertex( 11 );
	v[ 0 ] =  .5; v[ 1 ] =  .5; v[ 2 ] =  .5;
	
	v = _cubeBuffer.vertex( 12 );
	v[ 0 ] = -.5; v[ 1 ] =  .5; v[ 2 ] = -.5;
	
	v = _cubeBuffer.vertex( 13 );
	v[ 0 ] =  .5; v[ 1 ] =  .5; v[ 2 ] = -.5;


	_quadBuffer.resize( 6, VB_XY );
	v = _quadBuffer.vertex( 0 );
	v[ 0 ] = -.5; v[ 1 ] = -.5;

	v = _quadBuffer.vertex( 1 );
	v[ 0 ] = -.5; v[ 1 ] =  .5;

	v = _quadBuffer.vertex( 2 );
	v[ 0 ] =  .5; v[ 1 ] =  .5;

	v = _quadBuffer.vertex( 3 );
	v[ 0 ] =  .5; v[ 1 ] =  .5;

	v = _quadBuffer.vertex( 4 );
	v[ 0 ] =  .5; v[ 1 ] = -.5;

	v = _quadBuffer.vertex( 5 );
	v[ 0 ] = -.5; v[ 1 ] = -.5;

}
