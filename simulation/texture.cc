/*****************************************************************************
 *                                                                           *
 * The breve Simulation Environment                                          *
 * Copyright (C) 2000-2007 Jonathan Klein                                    *
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

#include "texture.h"
#include "image.h"

#define MAX( x, y ) ( (x)>(y)?(x):(y) )

slTexture2D::slTexture2D( std::string &inFile, bool inRepeat ) {
	_textureID = 0;
	_sizeX = 0;
	_sizeY = 0;
	_texX = 0;
	_texY = 0;
	_unitX = 0;
	_unitY = 0;
	loadImage( inFile, inRepeat );
}

slTexture2D::slTexture2D() {
	_sizeX = 0;
	_sizeY = 0;
	_texX = 0;
	_texY = 0;
	_unitX = 0;
	_unitY = 0;
	_textureID = 0;
}

int slTexture2D::loadImage( std::string &inFile, bool inRepeat ) {
	unsigned char *pixels;
	int height, width, components;

	pixels = slReadImage( inFile.c_str(), &width, &height, &components, 1 );

	if ( !pixels ) 
		return -1;
		// throw slException( std::string( "Could not read image file " ) + inFile );

	loadPixels( pixels, width, height, inRepeat );

	delete[] pixels;

	if( !_textureID )
		return -1;

	return 0;
}

slTexture2D::~slTexture2D() {
	if( _textureID != 0 ) 
		glDeleteTextures( 1, &_textureID );
}

void slTexture2D::loadPixels( unsigned char *pixels, int inWidth, int inHeight, bool inRepeat ) {
	if( _textureID == 0 )
		glGenTextures( 1, &_textureID );

	bind();

	resize( inWidth, inHeight );

	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );

	GLenum edge = inRepeat ? GL_REPEAT : GL_CLAMP_TO_EDGE;

	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, edge );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, edge );
	glTexEnvf( GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE );

	if ( _sizeX != inWidth || _sizeY != inHeight ) {
		int xstart, ystart;

		unsigned char *newpixels = new unsigned char[_sizeX * _sizeY * 4];

		memset( newpixels, 0, _sizeX * _sizeY * 4 );

		xstart = 0;
		ystart = 0;

		for ( int y = 0; y < inHeight; y++ )
			memcpy( &newpixels[( y + ystart ) *( _sizeX * 4 ) + ( xstart * 4 )], &pixels[y * inWidth * 4], inWidth * 4 );

		glTexSubImage2D( GL_TEXTURE_2D, 0, 0, 0, _sizeX, _sizeY, GL_RGBA, GL_UNSIGNED_BYTE, newpixels );

		delete[] newpixels;
	} else {
		glTexSubImage2D( GL_TEXTURE_2D, 0, 0, 0, inWidth, inHeight, GL_RGBA, GL_UNSIGNED_BYTE, pixels );
	}
}

void slTexture2D::resize( int inWidth, int inHeight, bool inHasAlpha ) {
	bind();

	if( inWidth == _texX && inHeight == _texY ) 
		return;

	_texX = inWidth;
	_texY = inHeight;

	_sizeX = slNextPowerOfTwo( inWidth );
	_sizeY = slNextPowerOfTwo( inHeight );

	_sizeX = _sizeY = MAX( _sizeX, _sizeY );

	_unitX = ( inWidth - 0.0f )  / (float)_sizeX;
	_unitY = ( inHeight - 0.0f ) / (float)_sizeY;

	GLenum format = inHasAlpha ? GL_RGBA : GL_RGB;

	glTexImage2D( GL_TEXTURE_2D, 0, format, _sizeX, _sizeY, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL );
}

#undef MAX
