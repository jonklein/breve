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

#if HAVE_LIBAVCODEC

static void initYUVLookupTable(void);
static void RGB2YUV420(int, int, unsigned char *, unsigned char *, unsigned char *, unsigned char *, int);

/*!
	\brief Opens a movie file for writing.
*/

slMovie *slMovieCreate(char *filename, int width, int height, int framerate, float quality) {
	slMovie *m;

	height &= ~1;
	width &= ~1;

	if (width < 1 || height < 1 || framerate < 1) {
		slMessage(DEBUG_ALL, "invalid parameters for new movie\n");
		return NULL;
	}

	initYUVLookupTable();

	avcodec_init();
	avcodec_register_all();

	m = new slMovie;

	if (!(m->codec = avcodec_find_encoder(CODEC_ID_MPEG1VIDEO))) {
		slMessage(DEBUG_ALL, "Could not find MPEG-1 video encoder\n");
		return NULL;
	}
	if (!(m->file = fopen(filename, "wb"))) {
		slMessage(DEBUG_ALL, "Could not open file \"%s\" for writing: %s\n", filename, strerror(errno));
		return NULL;
	}

	m->context = avcodec_alloc_context();
	m->picture = avcodec_alloc_frame();

	m->context->width = width;
	m->context->height = height;
	m->context->frame_rate = framerate;
	m->context->frame_rate_base = 1;

	m->context->bit_rate = 800000;
	m->context->flags = CODEC_FLAG_TRELLIS_QUANT | CODEC_FLAG_NORMALIZE_AQP;
	m->context->gop_size = 15;
	m->context->max_b_frames = 1;
	m->context->me_method = ME_EPZS;
	m->context->rc_qsquish = 1.0f;
	m->context->sample_aspect_ratio = av_d2q(1.0, 1);
	m->context->qblur = 0.0f;
	m->context->qcompress = 1.0f;

	if (avcodec_open(m->context, m->codec) < 0) {
		slMessage(DEBUG_ALL, "error opening movie output codec\n");
		return NULL;
	}

	m->bufferSize = height * width * 3;

	m->buffer = new unsigned char[m->bufferSize];
	m->RGBpictureBuffer = new unsigned char[m->bufferSize];
	m->YUVpictureBuffer = new unsigned char[m->bufferSize / 2];
	m->vvBuffer = new unsigned char[height * width];
	m->uuBuffer = new unsigned char[height * width];

	m->picture->data[0] = m->YUVpictureBuffer;
	m->picture->data[1] = m->picture->data[0] + (height * width);
	m->picture->data[2] = m->picture->data[1] + (height * width) / 4;
	m->picture->linesize[0] = width;
	m->picture->linesize[1] = width / 2;
	m->picture->linesize[2] = width / 2;
	m->picture->quality = 0;

	return m;
}

/*!
	\brief Adds a frame to a movie from pixel data.
*/


int slMovieAddFrame(slMovie *m, int flip) {
	int size;

	if (!m)
		return -1;

	RGB2YUV420(m->context->width, m->context->height, m->uuBuffer, m->vvBuffer, m->RGBpictureBuffer, m->YUVpictureBuffer, flip);

	if (!(size = avcodec_encode_video(m->context, m->buffer, m->bufferSize, m->picture)))
		return -1;

	fwrite(m->buffer, size, 1, m->file);

	return 0;
}

/*!
	\brief Reads from the current OpenGL context to add a frame to a movie.
*/

int slMovieAddWorldFrame(slMovie *m, slWorld *w, slCamera *c) {
	if (c->activateContextCallback && c->activateContextCallback()) {
		slMessage(DEBUG_ALL, "Cannot add frame to movie: no OpenGL context available\n");
		return -1;
	}
	if (c->renderContextCallback)
		c->renderContextCallback(w, c);

	glReadPixels(0, 0, m->context->width, m->context->height, GL_RGB, GL_UNSIGNED_BYTE, m->RGBpictureBuffer);

	return slMovieAddFrame(m, 1);
}

/*!
	\brief Finishes writing to a movie.
*/

int slMovieFinish(slMovie *m) {
	int size;
	char outbuf[4];

	if (!m)
		return -1;

	while ((size = avcodec_encode_video(m->context, m->buffer, m->bufferSize, NULL)))
		fwrite(m->buffer, size, 1, m->file);

	outbuf[0] = 0x00;
	outbuf[1] = 0x00;
	outbuf[2] = 0x01;
	outbuf[3] = 0xb7;
	fwrite(outbuf, 4, 1, m->file);
	fclose(m->file);

	avcodec_close(m->context);
	av_free(m->context);
	av_free(m->picture);

	delete[] m->buffer;
	delete[] m->YUVpictureBuffer;
	delete[] m->RGBpictureBuffer;
	delete[] m->vvBuffer;
	delete[] m->uuBuffer;

	delete m;

	return 0;
}

/*!
	\brief Translate from RGB to YUV.

	I have no freakin' clue.  I found this on the net somewhere.  I also
	found about six different verions of the same thing, and this is the 
	only thing that worked.
*/

static int RGB2YUV_YR[256], RGB2YUV_YG[256], RGB2YUV_YB[256];
static int RGB2YUV_UR[256], RGB2YUV_UG[256], RGB2YUV_UBVR[256];
static int RGB2YUV_VG[256], RGB2YUV_VB[256];

void RGB2YUV420(int x_dim, int y_dim, unsigned char *uu, unsigned char *vv, unsigned char *bmp, unsigned char *yuv, int flip) {
	unsigned char *r, *g, *b;
	unsigned char *y, *u, *v;
	unsigned char *pu1, *pu2,*pu3,*pu4;
	unsigned char *pv1, *pv2,*pv3,*pv4;
	int i, j, row;

	y = yuv;
	u = uu;
	v = vv;

	for (i = 0; i < y_dim; ++i) {
		if (flip)
			row = ((y_dim - 1) - i);
		else
			row = i;

		r = bmp + 3 * x_dim * row;
		g = r + 1;
		b = r + 2;

		for (j = 0; j < x_dim; ++j) {

			*y++ = ( RGB2YUV_YR[*r]  + RGB2YUV_YG[*g] + RGB2YUV_YB[*b] + 1048576) >> 16;
			*u++ = (-RGB2YUV_UR[*r]  - RGB2YUV_UG[*g] + RGB2YUV_UBVR[*b] + 8388608) >> 16;
			*v++ = ( RGB2YUV_UBVR[*r]- RGB2YUV_VG[*g] - RGB2YUV_VB[*b] + 8388608) >> 16;

			r += 3;
			g += 3;
			b += 3;
		}
	}

	//dimension reduction for U and V components
	u = yuv + x_dim * y_dim;
	v = u + x_dim * y_dim / 4;

	pu1 = uu;
	pu2 = pu1 + 1;
	pu3 = pu1 + x_dim;
	pu4 = pu3 + 1;

	pv1 = vv;
	pv2 = pv1 + 1;
	pv3 = pv1 + x_dim;
	pv4 = pv3 + 1;

	for (i = 0; i < y_dim; i += 2) {
		for (j = 0; j < x_dim; j += 2) {
			*u++ = ((int)*pu1 + *pu2 + *pu3 + *pu4) >> 2;
			*v++ = ((int)*pv1 + *pv2 + *pv3 + *pv4) >> 2;
			pu1 += 2;
			pu2 += 2;
			pu3 += 2;
			pu4 += 2;
			pv1 += 2;
			pv2 += 2;
			pv3 += 2;
			pv4 += 2;
		}

		pu1 += x_dim;
		pu2 += x_dim;
		pu3 += x_dim;
		pu4 += x_dim;
		pv1 += x_dim;
		pv2 += x_dim;
		pv3 += x_dim;
		pv4 += x_dim;
	}
}

void initYUVLookupTable() {
	int i;

	for (i = 0; i < 256; i++) RGB2YUV_YR[i] = (int)(65.481f * (i << 8));
	for (i = 0; i < 256; i++) RGB2YUV_YG[i] = (int)(128.553f * (i << 8));
	for (i = 0; i < 256; i++) RGB2YUV_YB[i] = (int)(24.966f * (i << 8));
	for (i = 0; i < 256; i++) RGB2YUV_UR[i] = (int)(37.797f * (i << 8));
	for (i = 0; i < 256; i++) RGB2YUV_UG[i] = (int)(74.203f * (i << 8));
	for (i = 0; i < 256; i++) RGB2YUV_VG[i] = (int)(93.786f * (i << 8));
	for (i = 0; i < 256; i++) RGB2YUV_VB[i] = (int)(18.214f * (i << 8));
	for (i = 0; i < 256; i++) RGB2YUV_UBVR[i] = (int)(112.0f * (i << 8));
}

#endif
