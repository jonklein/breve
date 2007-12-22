/*****************************************************************************
 *                                                                           *
 * The breve Simulation Environment                                          *
 * Copyright (C) 2000-2004 Jonathan Klein                                    *
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

/*! \addtogroup InternalFunctions */
/*@{*/

#include "kernel.h"
#include "gldraw.h"
#include "image.h"

/*!
    \brief Holds image _data used by the breve image class.
*/

struct brImageData {
							brImageData() {
								_data = NULL;
								_width = 0;
								_height = 0;
								_textureNumber = -1;
							}

							~brImageData() {
								if( _data )	
									slFree( _data );
							}

	unsigned char* 			_data;
	int 					_width;
	int 					_height;
	int 					_textureNumber;
};

#define BRIMAGEDATAPOINTER(p)	((brImageData*)BRPOINTER(p))

/*!
	\brief Retuns the width of an image.

	int imageGetWidth(brImageData pointer).
*/

int brIImageGetWidth( brEval args[], brEval *result, brInstance *i ) {
	brImageData *dm = BRIMAGEDATAPOINTER( &args[0] );

	result->set( dm -> _width );

	return EC_OK;
}

/*!
	\brief Retuns the height of an image.

	int imageGetHeight(brImageData pointer).
*/

int brIImageGetHeight( brEval args[], brEval *result, brInstance *i ) {
	brImageData *dm = BRIMAGEDATAPOINTER( &args[0] );

	result->set( dm -> _height );

	return EC_OK;
}

/*!
	\brief Get the pixel value at the given coordinates.

	int imageGetValueAtCoordinates(brImageData pointer, int, int).

	The value is on a scale from 0.0 to 1.0.

	Since the _data is RGB, you'll have to offset the X value accordingly
	to get the desired red, green or blue pixel.
*/

int brIImageGetValueAtCoordinates( brEval args[], brEval *result, brInstance *i ) {
	brImageData *dm = BRIMAGEDATAPOINTER( &args[0] );
	int x = BRINT( &args[ 1 ] );
	int y = BRINT( &args[ 2 ] );

	int bpp = BRINT( &args[ 3 ] );
	int bps = 2;

	if ( x < 0 || x >= ( dm -> _width * bpp ) || y < 0 || y >= dm -> _height ) {
		slMessage( DEBUG_ALL, "Image access (%d, %d) out of bounds (%d, %d)\n", x, y, dm -> _width, dm -> _height );
		result->set( 0.0 );
		return EC_OK;
	}
	
	int offset = y * ( dm -> _width * bpp ) + x;

	result->set( *( dm ->_data + offset ) / 255.0 );

	return EC_OK;
}

/*!
	\brief Get the pixel value at the given coordinates.

	void imageSetValueAtCoordinates(brImageData pointer, int, int, double).

	The value is on a scale from 0.0 to 1.0.

	Since the _data is RGB, you'll have to offset the X value accordingly
	to set the desired red, green or blue pixel.
*/

int brIImageSetValueAtCoordinates( brEval args[], brEval *result, brInstance *i ) {
	brImageData *dm = BRIMAGEDATAPOINTER( &args[0] );
	int x = BRINT( &args[1] );
	int y = BRINT( &args[2] );
	int value = ( int )( 255.0 * BRDOUBLE( &args[3] ) );

	if ( value > 255 )
		value = 255;
	else if ( value < 0 )
		value = 0;

	if ( x < 0 || x >= ( dm -> _width * 4 ) || y < 0 || y >= dm -> _height ) {
		slMessage( DEBUG_ALL, "_data matrix access (%d, %d) out of bounds (%d, %d)\n", x, y, dm -> _width, dm -> _height );
		return EC_OK;
	}

	dm ->_data[y *( dm -> _width * 4 ) + x] = value;

	return EC_OK;
}

/*!
	\brief Reads Image _data from the screen.
*/

int brIImageReadPixels( brEval args[], brEval *result, brInstance *i ) {
	brImageData *dm = BRIMAGEDATAPOINTER( &args[0] );
	int x = BRINT( &args[1] );
	int y = BRINT( &args[2] );

	// Alpha channel washed out on Windows?!  Fix it with a 1.0 bias

	if( i->engine->camera->_activateContextCallback ) {
		if( i->engine->camera->_activateContextCallback() != 0 ) {
			slMessage( DEBUG_ALL, "warning: could not read pixels, no OpenGL context available\n" );
			return EC_OK;
		}
	}

	glPixelTransferf( GL_ALPHA_BIAS, 1.0 );
	glReadPixels( x, y, dm -> _width, dm -> _height, GL_RGBA, GL_UNSIGNED_BYTE, dm ->_data );
	glPixelTransferf( GL_ALPHA_BIAS, 0.0 );

	return EC_OK;
}

/*! 
  \brief Reads Depth buffer information from the screen.

*/ 
int brIImageReadDepthBuffer( brEval args[], brEval *result, brInstance *i ) {
	brImageData *dm = BRIMAGEDATAPOINTER( &args[0] );
	int x = BRINT( &args[1] );
	int y = BRINT( &args[2] );
	int linearize = BRINT( &args[3]); 
	float maxRange = BRFLOAT (&args[4]);
	if( i->engine->camera->_activateContextCallback ) {
		if( i->engine->camera->_activateContextCallback() != 0 ) {
			slMessage( DEBUG_ALL, "warning: could not read pixels, no OpenGL context available\n" );
			return EC_OK;
		}
	}
	glReadPixels( x, y, dm -> _width, dm -> _height, GL_DEPTH_COMPONENT, GL_UNSIGNED_SHORT, dm ->_data );
	if(linearize){  
	  double objX, objY, objZ;
          double proj[16];
          int view[4];
          double model[] = {1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1};
          // We need to recover the projection matrix so that we can call gluUnProject
	  glMatrixMode( GL_PROJECTION );
	  glPushMatrix(); 
	  glLoadIdentity();
	  gluPerspective( 40.0, i->engine->camera->_fov, i->engine->camera->_frontClip, i->engine->camera->_zClip );
          glGetDoublev(GL_PROJECTION_MATRIX, proj);
	  glPopMatrix(); 

          glGetIntegerv(GL_VIEWPORT, view);
	  
	  // Loop over all the pixels, linearizing the result. 
          for (int y_counter = y; y_counter < dm -> _height; y_counter ++){
            for (int x_counter = 0; x_counter < dm -> _width; x_counter ++)
              {
                gluUnProject((double)x_counter, (double)y_counter, (double)(((unsigned short *) dm ->_data)[y_counter*(dm -> _width)+x_counter])/65535.0, model, proj, view, &objX, &objY, &objZ);

                                        // Compute the actual distance
                                        double d = sqrt(objX*objX + objY*objY + objZ*objZ);
				        d *= 65535.0/maxRange;
					// printf("Value is: %d %f\n",(((unsigned short *) dm ->_data)[y_counter*(dm -> _width)+x_counter]), d); 

					// Clip the value. 
					if(d > 65535.0) d = 65535.0; 
                                        // Store it. Note that we flip the vertical axis.
                                        ((unsigned short *) dm ->_data)[y_counter*(dm -> _width)+x_counter] = (unsigned short) d;

                                }
                        }

	}
	return EC_OK;
}


/*!
	\brief Loads an image from a file of a given name.

	brImageData pointer imageLoadFromFile(string).
*/

int brIImageLoadFromFile( brEval args[], brEval *result, brInstance *i ) {
	brImageData *dm;
	char *file;
	int c;

	file = brFindFile( i->engine, BRSTRING( &args[0] ), NULL );

	if ( !file ) {
		slMessage( DEBUG_ALL, "Error loading image file \"%s\": no such file\n", BRSTRING( &args[0] ) );

		result->set(( void* )NULL );

		return EC_OK;
	}

	dm = new brImageData;
	dm -> _data = slReadImage( file, &dm -> _width, &dm -> _height, &c, 0 );

	if ( !file ) {
		slMessage( DEBUG_ALL, "Error reading image from file \"%s\": unrecognized format or corrupt file\n", file );
		slFree( file );

		result->set(( void* )NULL );

		return EC_OK;
	}

	slFree( file );

	result->set( ( void* )dm );

	return EC_OK;
}

/**
 * \brief Updates the texture and returns the texture number number associated with an image.
 */

int brIImageUpdateTexture( brEval args[], brEval *result, brInstance *i ) {
	brImageData *image = BRIMAGEDATAPOINTER( &args[0] );

	if ( !image ) {
		result->set( -1 );
		return EC_OK;
	}

	if ( image->_textureNumber == -1 )
		image->_textureNumber = slTextureNew( i->engine->camera );

	slUpdateTexture( i->engine->camera, image->_textureNumber, image->_data, image-> _width, image-> _height, GL_RGBA );

	result->set( image->_textureNumber );

	return EC_OK;
}

/*!
	\brief Gets a pointer to the raw pixel _data of an image.

	char pointer imageGetPixelPointer(brImageData pointer).

	Used by plugins that need raw pixel access.
*/

int brIImageGetPixelPointer( brEval args[], brEval *result, brInstance *i ) {
	brImageData *dm = BRIMAGEDATAPOINTER( &args[0] );

	if ( !dm ) {
		slMessage( DEBUG_ALL, "pixelPointer called with uninitialized image _data\n" );
		return EC_ERROR;
	}

	result->set( dm ->_data );

	return EC_OK;
}

/*!
	\brief Writes an image buffer out to a file.

	int imageWriteToFile(brImageData pointer, string).
*/

int brIImageWriteToFile( brEval args[], brEval *result, brInstance *i ) {
#if HAVE_LIBPNG
	char *file;
	brImageData *dm = BRIMAGEDATAPOINTER( &args[0] );

	file = brOutputPath( i->engine, BRSTRING( &args[1] ) );

	int channels = BRINT ( & args[2] ); 


	int bit_depth = BRINT( &args[3] );


	result->set( slPNGWrite( file, dm -> _width, dm -> _height, dm -> _data, channels, 1, bit_depth ) );

	slFree( file );

#else
	slMessage( DEBUG_ALL, "This version of breve was built without support for image export\n" );

#endif

	return EC_OK;

}

int brISnapshot( brEval args[], brEval *result, brInstance *i ) {
#if HAVE_LIBPNG
	char *f;

	f = brOutputPath( i->engine, BRSTRING( &args[0] ) );

	result->set( slPNGSnapshot( i->engine->world, i->engine->camera, f ) );

	slFree( f );

	return EC_OK;

#else
	slMessage( DEBUG_ALL, "This version of breve was built without support for image export\n" );

	return EC_ERROR;

#endif
}

int brISnapshotDepth( brEval args[], brEval *result, brInstance *i ) {
#if HAVE_LIBPNG
	char *f;
	int lin;
	float maxDist; 
	f = brOutputPath( i->engine, BRSTRING( &args[0] ) );
	lin = BRINT(&args[1]); 
	maxDist = BRFLOAT(&args[2]); 
	
	result->set( slPNGSnapshotDepth( i->engine->world, i->engine->camera, f, lin, maxDist ) );

	slFree( f );

	return EC_OK;

#else
	slMessage( DEBUG_ALL, "This version of breve was built without support for image export\n" );

	return EC_ERROR;

#endif
}


/*!
	\brief Initializes an empty image buffer of a given size.

	brImageData pointer imageDataInit(int, int).
*/

int brIImageDataInit( brEval args[], brEval *result, brInstance *i ) {
	brImageData *dm;
	int width = BRINT( &args[0] );
	int height = BRINT( &args[1] );

	dm = new brImageData;
	dm ->_data = (unsigned char*)slMalloc( width * height * 4 );

	memset( dm ->_data, 0x0, width * height * 4 );

	// init to black, but alpha channel to white

	for( int n = 0; n < width * height * 4; n++ ) {
		if( n % 4 == 3 ) 
			dm ->_data[ n ] = 0xff;
	}

	dm -> _width = width;
	dm -> _height = height;

	result->set( dm );

	return EC_OK;
}

int brIImageGetCompressionSize( brEval args[], brEval *result, brInstance *i ) {
	brImageData *dm = BRIMAGEDATAPOINTER( &args[0] );

	static unsigned char *compressionBuffer = NULL;

	static int compressionBufferSize = 0;

	// The zlib compression buffer must be larger than the buffer to be compressed
	// for worst-case-scenario compression.

	int requiredSize = ( int )(( dm -> _width * dm -> _height * 4 + 12 ) * 1.1 );

	if ( requiredSize > compressionBufferSize ) {
		if ( compressionBuffer ) delete[] compressionBuffer;

		compressionBuffer = new unsigned char[ requiredSize ];

		compressionBufferSize = requiredSize;
	}

	unsigned long compressionBytes = compressionBufferSize;

	compress( compressionBuffer, ( uLongf* )&compressionBytes, dm ->_data, dm -> _width * dm -> _height * 4 );

	result->set(( long )compressionBytes );

	return EC_OK;
}

/**
 * \brief Frees image _data.
 * imageDataFree(brImageData).
 */

int brIImageDataFree( brEval args[], brEval *result, brInstance *i ) {
	brImageData *dm = BRIMAGEDATAPOINTER( &args[0] );

	if ( dm ->_textureNumber != -1 )
		slTextureFree( i->engine->camera, dm ->_textureNumber );

	delete dm;

	return EC_OK;
}

/** 
 * Serializes an image to brData.
 */

int brIImageArchive( brEval args[], brEval *result, brInstance *i ) {
	brImageData *dm = BRIMAGEDATAPOINTER( &args[0] );
	int bytes = sizeof( int ) * 2 + dm -> _height * dm -> _width * 4;

	unsigned char *dataptr = new unsigned char[ bytes ];

	( (int*)dataptr )[ 0 ] = dm -> _width;
	( (int*)dataptr )[ 1 ] = dm -> _height;

	memcpy( (char*)( ( (int*)dataptr ) + 2 ), dm -> _data, dm -> _height * dm -> _width * 4 );

	result -> set( new brData( dataptr, bytes ) );

	delete[] dataptr;

	return EC_OK;
}

/** 
 * Deserializes an image to brData.
 */

int brIImageDearchive( brEval args[], brEval *result, brInstance *i ) {
	brData *data = BRDATA( &args[ 0 ] );
	brImageData *dm = new brImageData;

	dm -> _width  = ( (int*)data -> data )[ 0 ];
	dm -> _height = ( (int*)data -> data )[ 1 ];
    dm -> _data = (unsigned char*)slMalloc( dm -> _width * dm -> _height * 4 );

	memcpy( (char*)dm -> _data, (int*)data -> data + 2 , dm -> _height * dm -> _width * 4 );

	result -> set( (void*)dm );

	return EC_OK;
}

void breveInitImageFunctions( brNamespace *n ) {
	BRBREVECALL( n, brIImageArchive, AT_DATA, AT_POINTER, 0 );
	BRBREVECALL( n, brIImageDearchive, AT_POINTER, AT_DATA, 0 );

	brNewBreveCall( n, "imageGetWidth", brIImageGetWidth, AT_INT, AT_POINTER, 0 );
	brNewBreveCall( n, "imageGetHeight", brIImageGetHeight, AT_INT, AT_POINTER, 0 );
	brNewBreveCall( n, "imageGetPixelPointer", brIImageGetPixelPointer, AT_POINTER, AT_POINTER, 0 );
	brNewBreveCall( n, "imageGetCompressionSize", brIImageGetCompressionSize, AT_INT, AT_POINTER, 0 );
	brNewBreveCall( n, "imageGetValueAtCoordinates", brIImageGetValueAtCoordinates, AT_DOUBLE, AT_POINTER, AT_INT, AT_INT, 0 );
	brNewBreveCall( n, "imageSetValueAtCoordinates", brIImageSetValueAtCoordinates, AT_NULL, AT_POINTER, AT_INT, AT_INT, AT_DOUBLE, 0 );
	brNewBreveCall( n, "imageLoadFromFile", brIImageLoadFromFile, AT_POINTER, AT_STRING, 0 );
	brNewBreveCall( n, "imageWriteToFile", brIImageWriteToFile, AT_INT, AT_POINTER, AT_STRING, AT_INT, AT_INT,  0 );
	brNewBreveCall( n, "imageDataFree", brIImageDataFree, AT_NULL, AT_POINTER, 0 );
	brNewBreveCall( n, "imageDataInit", brIImageDataInit, AT_POINTER, AT_INT, AT_INT, 0 );
	brNewBreveCall( n, "imageUpdateTexture", brIImageUpdateTexture, AT_INT, AT_POINTER, 0 );
	brNewBreveCall( n, "imageReadPixels", brIImageReadPixels, AT_NULL, AT_POINTER, AT_INT, AT_INT, 0 );
	brNewBreveCall( n, "imageReadDepthBuffer", brIImageReadDepthBuffer, AT_NULL, AT_POINTER, AT_INT, AT_INT, AT_INT, AT_DOUBLE, 0 );
	brNewBreveCall( n, "snapshot", brISnapshot, AT_INT, AT_STRING, 0 );
	brNewBreveCall( n, "snapshotDepth", brISnapshotDepth, AT_INT, AT_STRING, AT_INT, AT_DOUBLE,  0 );
}
