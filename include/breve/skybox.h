
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

#include <string>

#include "texture.h"

class slRenderGL;
class slCamera;

class slSkybox {
	public:
							slSkybox();
							~slSkybox();

		void 				draw( slRenderGL& inRenderer, slCamera *inCamera );

		int 				loadImage( const char *inImage, int inN ) {
								std::string s( inImage );
								return loadImage( s, inN );
							}

		int 				loadImage( std::string &inImage, int inN );
		int 				loadNumberedImages( std::string &inImage );

	private:
		slTexture2D			_textures[ 6 ];
		bool				_loaded;
};
