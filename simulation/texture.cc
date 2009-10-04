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

slTexture2D::slTexture2D( const std::string *inFile, bool inRepeat ) {
	_textureID = 0;
	_sizeX = 0;
	_sizeY = 0;
	_texX = 0;
	_texY = 0;
	_unitX = 0;
	_unitY = 0;
	
	if( inFile )
		loadImage( *inFile, inRepeat );
}

int slTexture2D::loadImage( const std::string &inFile, bool inRepeat ) {
	unsigned char *pixels;
	int height, width, components;

	pixels = slReadImage( inFile.c_str(), &width, &height, &components, 1 );

	if ( !pixels ) 
		throw slException( std::string( "Could not read image file " ) + inFile );

	loadPixels( pixels, width, height, inRepeat );

	delete pixels;

	if( !_textureID )
		return -1;

	return 0;
}

slTexture2D::~slTexture2D() {
	if( _textureID != 0 ) 
		glDeleteTextures( 1, &_textureID );
}

void slTexture2D::createTextureID() {
	if( _textureID == 0 )
		glGenTextures( 1, &_textureID );
}

void slTexture2D::loadPixels( const unsigned char *pixels, int inWidth, int inHeight, int inChannels, bool inRepeat ) {
	createTextureID();
	
	bind();

	resize( inWidth, inHeight );

	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );

	GLenum edge = inRepeat ? GL_REPEAT : GL_CLAMP_TO_EDGE;

	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, edge );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, edge );

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



slVertexBufferGL::slVertexBufferGL( int inVertexCount, int inPixelFormat ) {
	_vertexSize = 0;
	_vertexCount = 0;
	_vertexFormat = 0;
	
	if( inVertexCount && inPixelFormat )
		resize( inVertexCount, inPixelFormat );
}

void slVertexBufferGL::draw( slVertexBufferDrawType inType ) const { 
	GLenum drawType = 0;
	
	switch( inType ) {
		case VB_TRIANGLE_STRIP:
			drawType = GL_TRIANGLE_STRIP;
			break;
		case VB_TRIANGLES:
			drawType = GL_TRIANGLES;
			break;
		case VB_LINE_STRIP:
			drawType = GL_LINE_STRIP;
			break;
	}
	
	glDrawArrays( drawType, 0, _vertexCount ); 
}

void slVertexBufferGL::bind() {
	if( _vertexFormat & VB_XY ) {
		glVertexPointer( 2, GL_FLOAT, _vertexSize, _data.data() + _vertexOffset );
		glEnableClientState( GL_VERTEX_ARRAY );
	} else if( _vertexFormat & VB_XYZ ) {
		glVertexPointer( 3, GL_FLOAT, _vertexSize, _data.data() + _vertexOffset );
		glEnableClientState( GL_VERTEX_ARRAY );
	}

	if( _vertexFormat & VB_UV ) {
		glTexCoordPointer( 2, GL_FLOAT, _vertexSize, _data.data() + _texOffset );
		glEnableClientState( GL_TEXTURE_COORD_ARRAY );
	} else if( _vertexFormat & VB_UVW ) {
		glTexCoordPointer( 3, GL_FLOAT, _vertexSize, _data.data() + _texOffset );
		glEnableClientState( GL_TEXTURE_COORD_ARRAY );		
	}
	
	if( _vertexFormat & VB_NORMAL ) {
		glNormalPointer( GL_FLOAT, _vertexSize, _data.data() + _normalOffset );
		glEnableClientState( GL_NORMAL_ARRAY );
	}
}

void slVertexBufferGL::unbind() {
	glDisableClientState( GL_TEXTURE_COORD_ARRAY );
	glDisableClientState( GL_NORMAL_ARRAY );
	glDisableClientState( GL_VERTEX_ARRAY );
}

void slVertexBufferGL::resize( int inVertexCount, int inPixelFormat ) {
	_vertexSize = 0;
	_vertexCount = inVertexCount;
	_vertexFormat = inPixelFormat;
	
	if( inPixelFormat & VB_XY ) {
		_vertexOffset = _vertexSize;
		_vertexSize += sizeof( float ) * 2;
	} else if( inPixelFormat & VB_XYZ ) {
		_vertexOffset = _vertexSize;
		_vertexSize += sizeof( float ) * 3;
	}
	
	if( inPixelFormat & VB_UV ) {
		_texOffset = _vertexSize;
		_vertexSize += sizeof( float ) * 2;
	} else if( inPixelFormat & VB_UVW ) {
		_texOffset = _vertexSize;
		_vertexSize += sizeof( float ) * 3;
	}

	if( inPixelFormat & VB_NORMAL ) {
		_normalOffset = _vertexSize;
		_vertexSize += sizeof( float ) * 3;
	}
	
	_data.resize( _vertexSize * _vertexCount );
}