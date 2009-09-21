#ifndef _TEXTURE_H
#define _TEXTURE_H

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

#include "simulation.h"
#include "glIncludes.h"
#include <string>

class slTexture {

};

class slTexture2D : public slTexture {
	friend class slPatchGrid;

	public:
					slTexture2D( const std::string *inImage = NULL, bool inRepeat = false );

					~slTexture2D();

		int			loadImage( const char *inImage, bool inRepeat = true ) {
						std::string image = inImage;
						return loadImage( image, inRepeat );
					}

		int			loadImage( const std::string &inImage, bool inRepeat = true );


		void		createTextureID();

		void 		bind() { 
						if( _textureID != 0 ) {
							glEnable( GL_TEXTURE_2D ); 
							glBindTexture( GL_TEXTURE_2D, _textureID ); 
							glTexEnvf( GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE );
						}
					}

		void		unbind() {
						glBindTexture( GL_TEXTURE_2D, 0 ); 
						glDisable( GL_TEXTURE_2D );
					}

		bool		isLoaded() { return _textureID != 0; }

		void 		loadPixels( const unsigned char *pixels, int inWidth, int inHeight, int inChannels = 4, bool inRepeat = true );
		void 		resize( int inWidth, int inHeight, bool inHasAlpha = true );

		float		_unitX;
		float		_unitY;

		int			_sizeX;
		int			_sizeY;

		int			_texX;
		int			_texY;
	private:
		GLuint 		_textureID;
};

enum slVertexBufferType {
	VB_XY		= 1 << 0,
	VB_XYZ		= 1 << 1,
	VB_UV		= 1 << 2,
	VB_UVW		= 1 << 3,
	VB_NORMAL	= 1 << 4
};

enum slVertexBufferDrawType {
	VB_TRIANGLES 		= 1 << 0,
	VB_TRIANGLE_STRIP	= 1 << 1
};

class slVertexBufferGL {
	public:
							slVertexBufferGL( int inVertexCount = 0, int inPixelFormat = 0 );
							~slVertexBufferGL() {}
							
		void				resize( int inVertexCount, int inPixelFormat );

		inline float*		vertex( int inN ) const { return (float*)( &_data.data()[ _vertexSize * inN + _vertexOffset ] ); }
		inline float*		texcoord( int inN ) const { return (float*)( &_data.data()[ _vertexSize * inN + _texOffset ] ); }
		
		void 				draw( slVertexBufferDrawType inType = VB_TRIANGLES ) const { glDrawArrays( inType == VB_TRIANGLE_STRIP ? GL_TRIANGLE_STRIP : GL_TRIANGLES, 0, _vertexCount ); };
		
		void				bind();
		void				unbind();
		
	private:
		slBuffer			_data;
		unsigned int		_vertexSize;
		unsigned int		_vertexCount;
		unsigned int		_vertexFormat;
		
		unsigned int		_vertexOffset;
		unsigned int		_normalOffset;
		unsigned int		_texOffset;
};


#endif // _TEXTURE_H
