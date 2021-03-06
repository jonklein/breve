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
#include "image.h"
#include "camera.h"

#if HAVE_LIBPNG
#include <png.h>
#endif


#if HAVE_LIBJPEG

#include <setjmp.h>

extern "C" {
	#include <jpeglib.h>
}

#endif

/**
 * Opens up the specified image which can be in PNG or JPG format.
 */

unsigned char *slReadImage( const char *name, int *width, int *height, int *components, int alpha ) {
	const char *last;

	last = &name[ strlen( name ) - 1 ];

	while ( last >= name && *last != '.' )
		--last;

#if HAVE_LIBJPEG
	if ( !strcasecmp( last, ".jpg" ) || !strcasecmp( last, ".jpeg" ) )
		return slReadJPEGImage( name, width, height, components, alpha );

#endif

#if HAVE_LIBPNG
	if ( !strcasecmp( last, ".png" ) )
		return slReadPNGImage( name, width, height, components, alpha );

#endif

	slMessage( DEBUG_ALL, "Unknown or unsupported image type for file \"%s\"\n", name );

	return NULL;
}

/*
	source code generally makes a pretty poor soapbox, but
	i have to express my opinion that libjpeg blows somewhere,
	right?

	lib. jpeg. blows.
*/


#if HAVE_LIBJPEG

struct slJPEGError {
	struct jpeg_error_mgr pub;	/* "public" fields */
	jmp_buf setjmp_buffer;		/* for return to caller */
};

void slJPEGErrorExit( j_common_ptr cinfo ) {

	/* cinfo->err really points to a my_error_mgr struct, so coerce pointer */

	slJPEGError *myerr = ( slJPEGError* ) cinfo->err;

	/* Always display the message. */
	/* We could postpone this until after returning, if we chose. */
	// (*cinfo->err->output_message) (cinfo);

	/* Return control to the setjmp point */
	longjmp( myerr->setjmp_buffer, 1 );
}

unsigned char *slReadJPEGImage( const char *name, int *width, int *height, int *components, int usealpha ) {

	struct jpeg_decompress_struct cinfo;
	slJPEGError jerr;
	JSAMPARRAY buffer;
	unsigned char *image;
	int rowstride;
	FILE *f = NULL;

	cinfo.err = jpeg_std_error( &jerr.pub );
	jerr.pub.error_exit = slJPEGErrorExit;

	jpeg_create_decompress( &cinfo );

	f = fopen( name, "rb" );

	if ( setjmp( jerr.setjmp_buffer ) ) {
		/* setjmp?  motherfuckinglibmotherfuckingjpeg blows, that's why */

		jpeg_destroy_decompress( &cinfo );
		fclose( f );
		return 0;
	}

	if ( !f ) {
		slMessage( DEBUG_ALL, "error opening image file \"%s\": %s\n", name, strerror( errno ) );
		return NULL;
	}

	jpeg_stdio_src( &cinfo, f );

	jpeg_read_header( &cinfo, TRUE );

	rowstride = cinfo.num_components * cinfo.image_width;

	if ( cinfo.num_components != 3 ) {
		slMessage( DEBUG_ALL, "error opening image file \"%s\": JPEG files must be RGB\n", name );
		return NULL;
	}

	image = new unsigned char[ 4 * cinfo.image_width * cinfo.image_height * sizeof( JSAMPLE ) ];

	buffer = ( *cinfo.mem->alloc_sarray )
	         (( j_common_ptr ) & cinfo, JPOOL_IMAGE, rowstride, 1 );

	// cinfo.scale_denom = 8;

	jpeg_start_decompress( &cinfo );

	while ( cinfo.output_scanline < cinfo.output_height ) {
		unsigned int n, m = 0;
		int row = cinfo.output_height - cinfo.output_scanline - 1;

		jpeg_read_scanlines( &cinfo, buffer, 1 );

		for ( n = 0;n < 4 * cinfo.output_width;n++ ) {
			if (( n + 1 ) % 4 ) image[( 4 * cinfo.output_width * row ) + n] = buffer[0][m++];
			else {
				// int total = ( int )(( buffer[0][m - 1] + buffer[0][m - 2] + buffer[0][m - 3] ) / 3.0 );
				image[( 4 * cinfo.output_width * row ) + n] = buffer[0][m - 1] = 0xff;
			}
		}
	}

	( *width ) = cinfo.output_width;
	( *height ) = cinfo.output_height;
	( *components ) = 4;

	jpeg_finish_decompress( &cinfo );

	fclose( f );

	return image;
}

#endif

#if HAVE_LIBPNG

unsigned char *slReadPNGImage( const char *name, int *width, int *height, int *components, int usealpha ) {
	FILE *f;
	png_byte header[8];
	png_structp png_ptr;
	png_infop info;
	int passes;
	png_bytep *rows;
	unsigned char *image;
	unsigned int y, x;

	f = fopen( name, "rb" );

	if ( !f ) {
		slMessage( DEBUG_ALL, "error opening image file \"%s\": %s\n", name, strerror( errno ) );
		return NULL;
	}

	fread( header, 1, 8, f );

	if ( png_sig_cmp( header, 0, 8 ) ) {
		slMessage( DEBUG_ALL, "error reading image file \"%s\": not a PNG file\n", name );
		fclose( f );
		return NULL;
	}

	png_ptr = png_create_read_struct( PNG_LIBPNG_VER_STRING, NULL, NULL, NULL );

	if ( !png_ptr ) {
		slMessage( DEBUG_ALL, "error reading image file \"%s\": error creating PNG structure\n", name );
		fclose( f );
		return NULL;
	}

	if ( setjmp( png_jmpbuf( png_ptr ) ) ) {
		png_destroy_read_struct( &png_ptr, &info, ( png_infopp )NULL );
		slMessage( DEBUG_ALL, "error reading image file \"%s\": error reading PNG info\n" );
		fclose( f );
		return NULL;
	}

	info = png_create_info_struct( png_ptr );

	png_init_io( png_ptr, f );
	png_set_sig_bytes( png_ptr, 8 );
	png_read_info( png_ptr, info );

	*components = 4;
	*width = png_get_image_width(png_ptr, info);
	*height = png_get_image_height(png_ptr, info);

	if ( !( png_get_color_type(png_ptr, info) & PNG_COLOR_MASK_COLOR ) ) {
		slMessage( DEBUG_ALL, "error opening image file \"%s\": PNG files must be RGB or RGBA\n", name );
		fclose( f );
		return NULL;
	}

	passes = png_set_interlace_handling( png_ptr );

	png_read_update_info( png_ptr, info );

	if ( setjmp( png_jmpbuf( png_ptr ) ) ) {
		slMessage( DEBUG_ALL, "error reading image file \"%s\": error reading PNG file\n" );
		fclose( f );
		return NULL;
	}

	rows = ( png_bytep* ) malloc( sizeof( png_bytep ) * *height );

	for ( y = 0;y < *height;y++ ) rows[y] = ( png_byte* ) malloc( png_get_rowbytes(png_ptr, info) );

	png_read_image( png_ptr, rows );

	image = new unsigned char[  4 * *height * *width ];

	for ( x = 0;x < *height;x++ ) {
		int rowOffset = 0;

		for ( y = 0;y < *width;y++ ) {
			image[x *( *width * 4 ) + ( y * 4 ) + 0] = rows[( *height - 1 ) - x][rowOffset++];
			image[x *( *width * 4 ) + ( y * 4 ) + 1] = rows[( *height - 1 ) - x][rowOffset++];
			image[x *( *width * 4 ) + ( y * 4 ) + 2] = rows[( *height - 1 ) - x][rowOffset++];

			if ( png_get_color_type(png_ptr, info) & PNG_COLOR_MASK_ALPHA ) {
				image[x *( *width * 4 ) + ( y * 4 ) + 3] = rows[( *height - 1 ) - x][rowOffset++];
			} else {
				image[x *( *width * 4 ) + ( y * 4 ) + 3] = 0xff;
			}
		}
	}

	for ( y = 0;y < *height;y++ ) free( rows[y] );

	free( rows );

	png_destroy_read_struct( &png_ptr, &info, ( png_infopp )NULL );

	fclose( f );

	return image;
}

/*!
	\brief Writes a PNG image to disk.

	The number of channels may be 1 (b/w), 2 (b/w + alpha), 3 (rgb) or 4 (rgba).
	The reversed flag indicates whether the rows should be read top-to-bottom or
	bottom-to-top. The bit depth says how many bits are required to represent each 
	pixel. This would typically be either 8 or 16 bits (typically for z buffers). 

*/

int slPNGWrite( const char *name, int width, int height, unsigned char *buffer, int channels, int reversed , int bit_depth) {
	FILE *fp;
	png_structp png_ptr;
	png_infop info_ptr;
	png_bytepp rowPtrs;
	int n;

	if ( !( fp = fopen( name, "wb" ) ) ) {
		slMessage( DEBUG_ALL, "Could not open image file \"%s\" for writing: %s\n", name, strerror( errno ) );
		return -1;
	}

	if ( !( png_ptr = png_create_write_struct( PNG_LIBPNG_VER_STRING, NULL, NULL, NULL ) ) ) {
		slMessage( DEBUG_ALL, "Error writing PNG file \"%s\"\n", name );
		fclose( fp );
		return -1;
	}

	if ( !( info_ptr = png_create_info_struct( png_ptr ) ) ) {
		png_destroy_write_struct( &png_ptr, NULL );
		slMessage( DEBUG_ALL, "Error writing PNG file \"%s\"\n", name );
		fclose( fp );
		return -1;
	}

	if ( setjmp( png_jmpbuf( png_ptr ) ) ) {
		slMessage( DEBUG_ALL, "Error writing PNG file \"%s\"\n", name );
		png_destroy_write_struct( &png_ptr, &info_ptr );
		fclose( fp );
		return -1;
	}

	rowPtrs = new png_bytep[height];

	if ( !reversed )
		for ( n = 0; n < height; n++ )
			rowPtrs[n] = &buffer[n * ( width * channels * bit_depth/8 )];
	else
		for ( n = 0; n < height; n++ )
			rowPtrs[height - ( n + 1 )] = &buffer[n * ( width * channels * bit_depth/8)];

	png_init_io( png_ptr, fp );
	
  int color_type = 0;

	switch ( channels ) {
		case 1:
			color_type = PNG_COLOR_TYPE_GRAY;
			break;

		case 2:
			color_type = PNG_COLOR_TYPE_GRAY_ALPHA;
			break;

		case 3:
			color_type = PNG_COLOR_TYPE_RGB;
			break;

		case 4:
			color_type = PNG_COLOR_TYPE_RGB_ALPHA;
			break;
	}

  png_set_IHDR(png_ptr, info_ptr, width, height, bit_depth, color_type, PNG_INTERLACE_NONE, 3, PNG_NO_FILTERS);
	png_set_rows( png_ptr, info_ptr, rowPtrs );

	if(bit_depth > 8 ){
	  png_write_png( png_ptr, info_ptr, PNG_TRANSFORM_SWAP_ENDIAN, NULL );
	} else {
	  png_write_png( png_ptr, info_ptr, 0, NULL );
	}
	
	png_destroy_write_struct( &png_ptr, &info_ptr );

	delete[] rowPtrs;

	fclose( fp );

	return 0;
}

/*!
	\brief Takes a PNG snapshot of the current screen output and writes it to a file.
*/

int slPNGSnapshot( slWorld *w, slCamera *c, const char *file ) {
	unsigned char *buf;
	int r;

	if ( c->_activateContextCallback && c->_activateContextCallback() ) {
		slMessage( DEBUG_ALL, "Cannot generate PNG snapshot: no OpenGL context available\n" );
		return -1;
	}

	if ( c->_renderContextCallback ) c->_renderContextCallback( w, c );

	buf = new unsigned char[c->_width * c->_height * 3];

	glReadPixels( c->_originx, c->_originy, c->_width, c->_height, GL_RGB, GL_UNSIGNED_BYTE, buf );

	r = slPNGWrite( file, c->_width, c->_height, buf, 3, 1 , 8 );

	delete[] buf;

	return r;
}

#if 0

int slPNGSnapshotDepth( slWorld *w, slCamera *c, const char *file, int lin, double maxRange ) {
	unsigned char *buf;
	int r;
	if ( c->_activateContextCallback && c->_activateContextCallback() ) {
		slMessage( DEBUG_ALL, "Cannot generate PNG snapshot: no OpenGL context available\n" );
		return -1;
	}

	if ( c->_renderContextCallback ) c->_renderContextCallback( w, c );

	buf = new unsigned char[c->_width * c->_height * 2];

	glReadPixels( c->_originx, c->_originy, c->_width, c->_height, GL_DEPTH_COMPONENT, GL_UNSIGNED_SHORT, buf );
	if(lin){
	  double objX, objY, objZ;
	  double model[16] = {1,0,0,0,
			    0,1,0,0,
			    0,0,1,0,
			    0,0,0,1};
          double proj[16];
          GLint view[4];
	  
	  // We need to recover the projection matrix so that we can call gluUnProject

	  glMatrixMode( GL_PROJECTION );
	  glPushMatrix(); 
	  glLoadIdentity();
	  gluPerspective( 40.0, c->_fov, c->_frontClip, c->_zClip );
          glGetDoublev(GL_PROJECTION_MATRIX, proj);
	  glPopMatrix(); 


          glGetIntegerv(GL_VIEWPORT, view);

          for (int y_counter = 0; y_counter < c->_height; y_counter ++){
            for (int x_counter = 0; x_counter < c->_width; x_counter ++)
              {
                gluUnProject((double)x_counter, (double)y_counter, (double)(((unsigned short *) buf)[y_counter*(c->_width)+x_counter])/65535.0, model, proj, view, &objX, &objY, &objZ);

                                        // Compute the actual distance
                                        double d = sqrt(objX*objX + objY*objY + objZ*objZ);

				        d *= 65535.0/maxRange;
					// Clip the value. 
					if(d > 65535.0) d = 65535.0; 
                                        ((unsigned short *) buf)[y_counter*(c->_width)+x_counter] = (unsigned short) d;

                                }
                        }

	}
	r = slPNGWrite( file, c->_width, c->_height, buf, 1, 1 , 16 );

	delete[] buf;

	return r;
}

#endif

#endif

/*!
	\brief Vertically reverses a pixel buffer.

	Some (external) libraries and routines expect top-to-bottom pixel data,
	others bottom-to-top.  This function swaps between the two.  Width must
	be the number of bytes in an entire row (taking into account the number
	of channels), not just the number of pixels.
*/

void slReversePixelBuffer( const unsigned char *source, unsigned char *dest, int width, int height ) {
	for ( int n = 0; n < height; n++ )
		memcpy( &dest[( height - ( n + 1 ) ) * width], &source[n * width], width );
}
