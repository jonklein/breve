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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#include "simulation.h"

#include "glIncludes.h"

#include "image.h"
#include "camera.h"

#if HAVE_LIBPNG
#include <png.h>
#endif

#include <setjmp.h>

#if HAVE_LIBJPEG
#ifdef __cplusplus      // make jpeglib C++ safe

extern "C" {
#endif
#include <jpeglib.h>
#ifdef __cplusplus
}

#endif

struct slJPEGError {

	struct jpeg_error_mgr pub;	/* "public" fields */
	jmp_buf setjmp_buffer;		/* for return to caller */
};

void slJPEGErrorExit( j_common_ptr );

#endif

/*!
	\brief An error structure for the JPEG library.
*/

/*!
	\brief Opens up the specified image which can be a SGI, PNG or JPG
	image.
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

	if ( !strcasecmp( last, ".sgi" ) )
		return slReadSGIImage( name, width, height, components, alpha );

	slMessage( DEBUG_ALL, "Unknown or unsupported image type for file \"%s\"\n", name );

	return NULL;
}

/*
	source code generally makes a pretty poor soapbox, but
	i have to express my opinion that libjpeg blows somewhere,
	right?

	lib. jpeg. blows.
*/

void bwtorgba( unsigned char *b, unsigned char *l, int n ) {
	while ( n-- ) {
		l[0] = *b;
		l[1] = *b;
		l[2] = *b;
		l[3] = *b;
		l += 4;
		b++;
	}
}

void latorgba( unsigned char *b, unsigned char *a, unsigned char *l, int n ) {

	while ( n-- ) {
		l[0] = *b;
		l[1] = *b;
		l[2] = *b;
		l[3] = *a;
		l += 4;
		b++;
		a++;
	}
}

void rgbtorgba( unsigned char *r, unsigned char *g, unsigned char *b, unsigned char *l, int n, int usealpha ) {
	while ( n-- ) {
		l[0] = r[0];
		l[1] = g[0];
		l[2] = b[0];

		if ( usealpha ) l[3] = 0xff - (( r[0] + g[0] + b[0] ) / 3 );
		else l[3] = 0xff;

		l += 4;

		r++;

		g++;

		b++;
	}
}

void rgbatorgba( unsigned char *r, unsigned char *g, unsigned char *b, unsigned char *a, unsigned char *l, int n ) {
	while ( n-- ) {
		l[0] = r[0];
		l[1] = g[0];
		l[2] = b[0];
		l[3] = a[0];
		l += 4;
		r++;
		g++;
		b++;
		a++;
	}
}

/*!
	\brief Data used for reading a .sgi file.
*/

typedef struct slSGIslSGIImageRec {
	unsigned short imagic;
	unsigned short type;
	unsigned short dim;
	unsigned short xsize, ysize, zsize;
	unsigned int min, max;
	unsigned int wasteBytes;
	char name[80];
	unsigned long colorMap;
	FILE *file;
	unsigned char *tmp, *tmpR, *tmpG, *tmpB;
	unsigned long rleEnd;
	unsigned int *rowStart;
	int *rowSize;
}

slSGIImageRec;

static void ConvertShort( unsigned short *array, long length ) {
	unsigned b1, b2;
	unsigned char *ptr;

	ptr = ( unsigned char * )array;

	while ( length-- ) {
		b1 = *ptr++;
		b2 = *ptr++;
		*array++ = ( b1 << 8 ) | ( b2 );
	}
}

static void ConvertLong( unsigned *array, long length ) {
	unsigned b1, b2, b3, b4;
	unsigned char *ptr;

	ptr = ( unsigned char * )array;

	while ( length-- ) {
		b1 = *ptr++;
		b2 = *ptr++;
		b3 = *ptr++;
		b4 = *ptr++;
		*array++ = ( b1 << 24 ) | ( b2 << 16 ) | ( b3 << 8 ) | ( b4 );
	}
}

static slSGIImageRec *ImageOpen( const char *fileName ) {
	union {
		int testWord;
		char testByte[4];
	} endianTest;

	slSGIImageRec *image;
	int swapFlag;
	int x;

	endianTest.testWord = 1;

	if ( endianTest.testByte[0] == 1 ) {
		swapFlag = 1;
	} else {
		swapFlag = 0;
	}

	image = ( slSGIImageRec * )malloc( sizeof( slSGIImageRec ) );

	if (( image->file = fopen( fileName, "rb" ) ) == NULL ) {
		perror( fileName );
		return NULL;
	}

	fread( image, 1, 12, image->file );

	if ( swapFlag ) {
		ConvertShort( &image->imagic, 6 );
	}

	image->tmp = ( unsigned char * )malloc( image->xsize * 256 );

	image->tmpR = ( unsigned char * )malloc( image->xsize * 256 );
	image->tmpG = ( unsigned char * )malloc( image->xsize * 256 );
	image->tmpB = ( unsigned char * )malloc( image->xsize * 256 );

	if ( image->tmp == NULL || image->tmpR == NULL || image->tmpG == NULL || image->tmpB == NULL ) {
		return NULL;
	}

	if (( image->type & 0xFF00 ) == 0x0100 ) {
		x = image->ysize * image->zsize * sizeof( unsigned );
		image->rowStart = ( unsigned * )malloc( x );
		image->rowSize = ( int * )malloc( x );

		if ( image->rowStart == NULL || image->rowSize == NULL ) return NULL;

		image->rleEnd = 512 + ( 2 * x );

		fseek( image->file, 512, SEEK_SET );

		fread( image->rowStart, 1, x, image->file );

		fread( image->rowSize, 1, x, image->file );

		if ( swapFlag ) {
			ConvertLong( image->rowStart, x / ( int )sizeof( unsigned ) );
			ConvertLong(( unsigned * )image->rowSize, x / ( int )sizeof( int ) );
		}
	} else {
		image->rowStart = NULL;
		image->rowSize = NULL;
	}

	return image;
}

void slSGIImageClose( slSGIImageRec *image ) {

	fclose( image->file );
	free( image->tmp );
	free( image->tmpR );
	free( image->tmpG );
	free( image->tmpB );
	free( image->rowStart );
	free( image->rowSize );
	free( image );
}

void slSGIImageGetRow( slSGIImageRec *image, unsigned char *buf, int y, int z ) {
	unsigned char *iPtr, *oPtr, pixel;
	int count;

	if (( image->type & 0xFF00 ) == 0x0100 ) {
		fseek( image->file, ( long ) image->rowStart[y+z*image->ysize], SEEK_SET );
		fread( image->tmp, 1, ( unsigned int )image->rowSize[y+z*image->ysize],
		       image->file );

		iPtr = image->tmp;
		oPtr = buf;

		for ( ;; ) {
			pixel = *iPtr++;
			count = ( int )( pixel & 0x7F );

			if (( iPtr - image->tmp ) >= image->rowSize[y+z*image->ysize] ) {
				return;
			}

			if ( !count ) {
				return;
			}

			if ( pixel & 0x80 ) {
				while ( count-- ) {
					if( oPtr - buf >= image->xsize ) {
						// row overflow?!
						return;
					}
					*oPtr++ = *iPtr++;
				}
			} else {
				pixel = *iPtr++;

				while ( count-- ) {
					if( oPtr - buf >= image->xsize ) {
						// row overflow?!
						return;
					}
					*oPtr++ = pixel;
				}
			}
		}
	} else {
		fseek( image->file, 512 + ( y*image->xsize ) + ( z*image->xsize*image->ysize ),
		       SEEK_SET );
		fread( buf, 1, image->xsize, image->file );
	}
}

unsigned char *slReadSGIImage( const char *name, int *width, int *height, int *components, int usealpha ) {
	unsigned char *base;
	unsigned *lptr;
	unsigned char *rbuf, *gbuf, *bbuf, *abuf;
	slSGIImageRec *image;
	int y;

	image = ImageOpen( name );

	if ( !image ) {
		slMessage( DEBUG_ALL, "error opening image file \"%s\": %s\n", name, strerror( errno ) );
		return NULL;
	}

	( *width ) = image->xsize;
	( *height ) = image->ysize;
	( *components ) = image->zsize;
	base = ( unsigned char* )slMalloc( image->xsize * image->ysize * sizeof( unsigned ) );
	rbuf = ( unsigned char * )malloc( image->xsize * sizeof( unsigned char ) );
	gbuf = ( unsigned char * )malloc( image->xsize * sizeof( unsigned char ) );
	bbuf = ( unsigned char * )malloc( image->xsize * sizeof( unsigned char ) );
	abuf = ( unsigned char * )malloc( image->xsize * sizeof( unsigned char ) );

	if ( !base || !rbuf || !gbuf || !bbuf )
		return NULL;

	lptr = ( unsigned* )base;

	printf(" x-size = %d\n", image->xsize );

	for ( y = 0; y < image->ysize; y++ ) {
		if ( image->zsize >= 4 ) {
			slSGIImageGetRow( image, rbuf, y, 0 );
			slSGIImageGetRow( image, gbuf, y, 1 );
			slSGIImageGetRow( image, bbuf, y, 2 );
			slSGIImageGetRow( image, abuf, y, 3 );
			rgbatorgba( rbuf, gbuf, bbuf, abuf, ( unsigned char * )lptr, image->xsize );
			lptr += image->xsize;
		} else if ( image->zsize == 3 ) {
			slSGIImageGetRow( image, rbuf, y, 0 );
			slSGIImageGetRow( image, gbuf, y, 1 );
			slSGIImageGetRow( image, bbuf, y, 2 );
			rgbtorgba( rbuf, gbuf, bbuf, ( unsigned char * )lptr, image->xsize, usealpha );
			lptr += image->xsize;
		} else if ( image->zsize == 2 ) {
			slSGIImageGetRow( image, rbuf, y, 0 );
			slSGIImageGetRow( image, abuf, y, 1 );
			latorgba( rbuf, abuf, ( unsigned char * )lptr, image->xsize );
			lptr += image->xsize;
		} else {
			slSGIImageGetRow( image, rbuf, y, 0 );
			bwtorgba( rbuf, ( unsigned char * )lptr, image->xsize );
			lptr += image->xsize;
		}
	}

	slSGIImageClose( image );

	free( rbuf );
	free( gbuf );
	free( bbuf );
	free( abuf );

	return base;
}

#if HAVE_LIBJPEG
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

	image = ( unsigned char* )slMalloc( 4 * cinfo.image_width * cinfo.image_height * sizeof( JSAMPLE ) );

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
				int total = ( int )(( buffer[0][m - 1] + buffer[0][m - 2] + buffer[0][m - 3] ) / 3.0 );
				image[( 4 * cinfo.output_width * row ) + n] = buffer[0][m - 1] = 0xff - total;
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

void slJPEGErrorExit( j_common_ptr cinfo ) {

	/* cinfo->err really points to a my_error_mgr struct, so coerce pointer */

	slJPEGError *myerr = ( slJPEGError* ) cinfo->err;

	/* Always display the message. */
	/* We could postpone this until after returning, if we chose. */
	// (*cinfo->err->output_message) (cinfo);

	/* Return control to the setjmp point */
	longjmp( myerr->setjmp_buffer, 1 );
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
	*width = info->width;
	*height = info->height;

	if ( !( info->color_type & PNG_COLOR_MASK_COLOR ) ) {
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

	rows = ( png_bytep* ) malloc( sizeof( png_bytep ) * info->height );

	for ( y = 0;y < info->height;y++ ) rows[y] = ( png_byte* ) malloc( info->rowbytes );

	png_read_image( png_ptr, rows );

	image = ( unsigned char* )slMalloc( 4 * info->height * info->width );

	for ( x = 0;x < info->height;x++ ) {
		int rowOffset = 0;

		for ( y = 0;y < info->width;y++ ) {
			image[x *( info->width * 4 ) + ( y * 4 ) + 0] = rows[( info->height - 1 ) - x][rowOffset++];
			image[x *( info->width * 4 ) + ( y * 4 ) + 1] = rows[( info->height - 1 ) - x][rowOffset++];
			image[x *( info->width * 4 ) + ( y * 4 ) + 2] = rows[( info->height - 1 ) - x][rowOffset++];

			if ( info->color_type & PNG_COLOR_MASK_ALPHA ) {
				image[x *( info->width * 4 ) + ( y * 4 ) + 3] = rows[( info->height - 1 ) - x][rowOffset++];
			} else {
				image[x *( info->width * 4 ) + ( y * 4 ) + 3] = 0xff;
			}
		}
	}

	for ( y = 0;y < info->height;y++ ) free( rows[y] );

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

	info_ptr->width = width;

	info_ptr->height = height;

	info_ptr->bit_depth = bit_depth;


	// printf("Bit depth: %d\n", bit_depth); 

	switch ( channels ) {

		case 1:
			info_ptr->color_type = PNG_COLOR_TYPE_GRAY;

			break;

		case 2:
			info_ptr->color_type = PNG_COLOR_TYPE_GRAY_ALPHA;

			break;

		case 3:
			info_ptr->color_type = PNG_COLOR_TYPE_RGB;

			break;

		case 4:
			info_ptr->color_type = PNG_COLOR_TYPE_RGB_ALPHA;

			break;
	}

	png_set_rows( png_ptr, info_ptr, rowPtrs );

	if(bit_depth > 8 ){
	
	  png_write_png( png_ptr, info_ptr, PNG_TRANSFORM_SWAP_ENDIAN, NULL );

	}

	else {
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
          int view[4];
	  
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
