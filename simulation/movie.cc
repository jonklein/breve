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
#include "movie.h"

#if HAVE_LIBAVFORMAT

static int slMovieEncodeFrame( slMovie *m );

/*!
	\brief Opens a movie file for writing.
*/

slMovie *slMovieCreate( char *filename, int width, int height ) {
	AVCodec *codec;
	slMovie *m;

	height &= ~1;
	width &= ~1;

	if ( width < 1 || height < 1 ) {
		slMessage( DEBUG_ALL, "Invalid export size for new movie\n" );
		return NULL;
	}

	m = new slMovie;

	if ( !( m->_context = avcodec_alloc_context() ) ) {
		slMessage( DEBUG_ALL, "Could not allocate lavf context\n" );
		delete m;
		return NULL;
	}

	m->_fp = fopen( filename, "wb" );

	if ( !m->_fp ) {
		slMessage( DEBUG_ALL, "Could not open file \"%s\" for writing: %s\n", filename, strerror( errno ) );
		av_free( m->_context );
		delete m;
		return NULL;
	}

	m->_context->codec_id = CODEC_ID_MPEG1VIDEO;

	m->_context->codec_type = CODEC_TYPE_VIDEO;

	if ( !( codec = avcodec_find_encoder( m->_context->codec_id ) ) ) {
		slMessage( DEBUG_ALL, "Could not find MPEG-1 encoder\n" );
		av_free( m->_context );
		delete m;
		return NULL;
	}

	m->_context->pix_fmt = PIX_FMT_YUV420P;

	m->_context->width = width;
	m->_context->height = height;
#if (LIBAVCODEC_VERSION_INT>>16)>=51
	m->_context->time_base = ( AVRational ) {
		                         1, 25
	                         };

#else
	m->_context->frame_rate = 1000 * m->_context->frame_rate_base;
#endif
	m->_context->bit_rate = 800 * 1000;
	m->_context->gop_size = 15;
	m->_context->max_b_frames = 1;

	int err = 0;

	if (( err = avcodec_open( m->_context, codec ) ) < 0 ) {
		slMessage( DEBUG_ALL, "Error opening video output codec: %d\n", err );
		av_free( m->_context );
		delete m;
		return NULL;
	}

	if ( !( m->_rgb_pic = avcodec_alloc_frame() ) || !( m->_yuv_pic = avcodec_alloc_frame() ) ) {
		slMessage( DEBUG_ALL, "Could not allocate lavc frame\n" );
		av_free( m->_context );
		delete m;
		return NULL;
	}

	/*
	 * enc_buf holds the encoded frame, so enc_len must be at least
	 * a reasonable estimate of the size of the largest I-frame.
	 */

	m->_enc_len = width * height * 3;

	m->_enc_buf = new uint8_t[ m->_enc_len ];

	/* rgb_buf holds the unencoded frame in packed RGB from OpenGL. */

	m->_rgb_len = avpicture_get_size( PIX_FMT_RGB24, width, height );

	m->_rgb_buf = new uint8_t[ m->_rgb_len ];

	avpicture_fill(( AVPicture * )m->_rgb_pic, m->_rgb_buf, PIX_FMT_RGB24,
	               width, height );

	// yuv_buf holds the unencoded frame in codec's native pixel format.

	m->_yuv_len = avpicture_get_size( m->_context->pix_fmt, width, height );

	m->_yuv_buf = new uint8_t[m->_yuv_len];

	avpicture_fill(( AVPicture * )m->_yuv_pic, m->_yuv_buf, m->_context->pix_fmt,
	               width, height );

	m->_line = new uint8_t[width * 3]; /* one line of packed RGB pixels */

	return m;
}

/*!
	\brief Encodes a frame and potentially writes it to the movie file.
*/


int slMovieEncodeFrame( slMovie *m ) {
	AVCodecContext *c;
	int size;

	if ( !m ) return -1;

	c = m->_context;

	/*
	 * Colorspace conversion from RGB to codec's format (likely YUV420P).
	 * OpenGL's RGB pixel format is RGB24 in lavc.
	 */

	img_convert(( AVPicture * )m->_yuv_pic, c->pix_fmt,
	            ( AVPicture * )m->_rgb_pic, PIX_FMT_RGB24, c->width, c->height );

	/* Encode the frame yuv_pic storing the output in enc_buf. */

	size = avcodec_encode_video( c, m->_enc_buf, m->_enc_len, m->_yuv_pic );

	if ( size ) fwrite( m->_enc_buf, 1, size, m->_fp );

	return 0;
}

/*!
	\brief Reads from the current OpenGL context to add a frame to a movie.
*/

int slMovieAddWorldFrame( slMovie *m, slWorld *w, slCamera *cam ) {
	AVCodecContext *c = m->_context;
	unsigned char *a, *b, *tmp;
	size_t len;
	int y;

	if ( cam->_activateContextCallback && cam->_activateContextCallback() ) {
		slMessage( DEBUG_ALL, "Cannot add frame to movie: no OpenGL context available\n" );
		return -1;
	}

	if ( cam->_renderContextCallback ) cam->_renderContextCallback( w, cam );

	glReadPixels( 0, 0, c->width, c->height, GL_RGB, GL_UNSIGNED_BYTE, m->_rgb_buf );

	/* OpenGL reads bottom-to-top, but encoder expects top-to-bottom. */

	len = c->width * 3;

	tmp = ( unsigned char * )m->_line;

	for ( y = 0; y < c->height >> 1; ++y ) {
		a = ( unsigned char * ) & m->_rgb_buf[y * len];
		b = ( unsigned char * ) & m->_rgb_buf[( c->height - ( y + 1 ) ) * len ];

		memcpy( tmp, a, len );
		memcpy( a, b, len );
		memcpy( b, tmp, len );
	}

	return slMovieEncodeFrame( m );
}

/*!
	\brief Finishes writing to a movie.
*/

int slMovieFinish( slMovie *m ) {
	AVCodecContext *c;

	if ( !m )
		return -1;

	c = m->_context;

	// Flush any frames left in lavc buffer by encoding a NULL frame.

	for ( int i = 0; i <= c->max_b_frames; ++i ) {
		int size = avcodec_encode_video( c, m->_enc_buf, m->_enc_len, NULL );

		if ( size ) fwrite( m->_enc_buf, 1, size, m->_fp );
	}

	char trailer[4];

	trailer[0] = 0x00;
	trailer[1] = 0x00;
	trailer[2] = 0x01;
	trailer[3] = 0xb7;
	fwrite( trailer, 1, 4, m->_fp );

	fclose( m->_fp );

	avcodec_close( c );
	av_free( m->_context );
	av_free( m->_rgb_pic );
	av_free( m->_yuv_pic );

	delete[] m->_enc_buf;
	delete[] m->_rgb_buf;
	delete[] m->_yuv_buf;
	delete[] m->_line;

	delete m;

	return 0;
}

#endif
