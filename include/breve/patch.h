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

#ifndef _PATCH_H
#define _PATCH_H

#include "bigMatrix.hh"
#include "render.h"
#include "texture.h"

class slPatchGrid;

/**
 *  slPatch represent a region of space.  This class is used to represent
 *  the location, breve object and color for display rendering.
 *
 *  slPatch classes only instantiated as a part of a slPatchGrid object.
 */
class slPatch {
	public:
        
        /** default constructor */
		slPatch();
		
		// //// the costructors are outlined below for possible
		// //// future use and/or to convert from using array
		// //// default constructor initialization
		/**
		 *    slPatch grid constructor.
		 *
		 *    slPatch base constructor requires a parent grid
		 */
		 slPatch( slPatchGrid* theGrid);

		/**
		 *    slPatch full constructor.
		 *
		 *    slPatch base constructor requires a parent grid
		 *    location and color offset
		 */
		 slPatch( slPatchGrid* theGrid,
		          slVector* theLocation,
		          const int theColorOffset);

        void setData(void *data);
        void* getData();
        void getLocation(slVector *location);

        // //// further refactoring in the future should reduce
        // //// these names to 'setColor()', etc.
		/**
		 *    setColor sets the color of the patch to color
		 *
		 */        
        void setColor(slVector *color);

		/**
		 *    getColor sets the color of the patch to color
		 *
		 */
		 // ///// shouldn't this be a return value
        void getColor(slVector *color);

		/**
		 *    SetTransparency sets the transparency level
		 *
		 */
        void setTransparency(double transparency);

    // these should become private/protected
		void *data;

		slVector location;
		int colorOffset;
		slPatchGrid *grid;

		// this vector can be used for an optional patch-based collision
		// pruning phase

		std::vector< int > _objectsInPatch;
		std::vector< slPatch* > _neighbors;
};

/**
 * \brief A grid of \ref slPatch objects, data associated with a certain region of 3D space.
 */


class slPatchGrid {
	friend class slPatch;
	
	public:
								slPatchGrid();

								slPatchGrid( const slVector *center, const slVector *patchSize, int x, int y, int z);

								~slPatchGrid();

		void 					draw( slRenderGL &inRenderer, slCamera *camera );

		slPatch* 				getPatchAtIndex( int x, int y, int z );
		slPatch* 				getPatchAtLocation( const slVector *location );
		void 					setDataAtIndex(int x, int y, int z, void *data);
		void 					copyColorFrom3DMatrix(slBigMatrix3DGSL *m, int channel, double scale);

		void 					assignObjectsToPatches( slWorld *w );
        
		/**
		 * Sets the PatchGrid to be drawn with or without volumetric texturing.
		 */
		void 					setDrawWithTexture( bool inUseTexture ) { _drawWithTexture = inUseTexture; }
		void 					setSmoothDrawing( bool inSmooth ) { drawSmooth = inSmooth; }


	protected:
		void 					drawWithout3DTexture( slRenderGL &inRenderer, slCamera *camera );

		void 					textureDrawXPass( slVector &size, int dir );
		void 					textureDrawYPass( slVector &size, int dir );
		void 					textureDrawZPass( slVector &size, int dir );

		// below should be private/protected

		unsigned int 			_xSize;
		unsigned int 			_ySize;
		unsigned int 			_zSize;

		slVector 				startPosition;
		slVector 				patchSize;

		slPatch 				***patches;

		// colors holds all of the color information for the patches.  it is a raw 
		// array of char values so that we can use it as texture data if desired.

		unsigned char 			*colors;

		unsigned int 			_textureX;
		unsigned int 			_textureY;
		unsigned int 			_textureZ;

		bool 					drawSmooth;
		void 					fillCubeBuffer();

		slTexture2D*			_texture;
		slVertexBufferGL		_cubeBuffer;
		slVertexBufferGL		_quadBuffer;
		
		bool 					_drawWithTexture;
		bool 					_textureNeedsUpdate;

};

#endif /* _PATCH_H */
