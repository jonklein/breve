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

#include "slutil.h"
#include "glIncludes.h"

class slTexture2D {
	public:
					slTexture2D( std::string &inImage );
					slTexture2D();

					~slTexture2D();

		int			loadImage( char *inImage ) {
						std::string image = inImage;
						return loadImage( image );
					}

		int			loadImage( std::string &inImage );

		void 			bind() { 
						if( _textureID != 0 ) {
							glEnable( GL_TEXTURE_2D ); 
							glBindTexture( GL_TEXTURE_2D, _textureID ); 
						}
					}

		void			unbind() {
							glDisable( GL_TEXTURE_2D );
					}

		bool			isLoaded() { return _textureID != 0; }

		void 			loadPixels( unsigned char *pixels, int width, int height );

		float			_unitX;
		float			_unitY;


	private:
		GLuint 			_textureID;
};

#endif // _TEXTURE_H
