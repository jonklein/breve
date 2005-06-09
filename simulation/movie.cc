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

static int slMovieEncodeFrame(slMovie *m);

/*!
	\brief Opens a movie file for writing.
*/

slMovie *slMovieCreate(char *filename, int width, int height) {
	AVCodecContext *c;
	AVCodec *codec;
	AVStream *st;
	slMovie *m;
	int n;

	height &= ~1;
	width &= ~1;

	if (width < 1 || height < 1) {
		slMessage(DEBUG_ALL, "invalid parameters for new movie\n");
		return NULL;
	}

	m = new slMovie;

#if FFMPEG_VERSION_INT <= 0x408
	if (!(m->context = (AVFormatContext *)av_mallocz(sizeof(AVFormatContext))) ||
#else
	if (!(m->context = av_alloc_format_context()) ||
#endif
	    !(st = av_new_stream(m->context, 0))) {
		slMessage(DEBUG_ALL, "Could not allocate lavf context\n");
		delete m;
		return NULL;
	}

	n = snprintf(m->context->filename, sizeof(m->context->filename), "file:%s", filename);
	if (n < 0 || (size_t)n >= sizeof(m->context->filename) ||
	    url_fopen(&m->context->pb, m->context->filename, URL_WRONLY) < 0) {
		slMessage(DEBUG_ALL, "Could not open file \"%s\" for writing: %s\n", filename, strerror(errno));
		av_free(st);
		av_free(m->context);
		delete m;
		return NULL;
	}
	if (!(m->context->oformat = guess_format("mpeg", filename, NULL))) {
		slMessage(DEBUG_ALL, "Could not find MPEG muxer\n");
		av_free(st);
		av_free(m->context);
		delete m;
		return NULL;
	}

	c = &st->codec;
	c->codec_id = CODEC_ID_MPEG1VIDEO;
	c->codec_type = CODEC_TYPE_VIDEO;

	if (!(codec = avcodec_find_encoder(c->codec_id))) {
		slMessage(DEBUG_ALL, "Could not find MPEG-1 encoder\n");
		av_free(st);
		av_free(m->context);
		delete m;
		return NULL;
	}

#if FFMPEG_VERSION_INT <= 0x409
	c->pix_fmt = PIX_FMT_YUV420P;
#else
	c->pix_fmt = codec->pix_fmts[0];
#endif

	c->width = width;
	c->height = height;
	c->frame_rate = 30000; /* NTSC */
	c->frame_rate_base = 1001;

	/* Use lavc ratecontrol to get 1-pass constant bit-rate. */

	c->rc_max_rate = c->rc_min_rate = c->bit_rate = 800 * 1000;

	c->bit_rate_tolerance = c->bit_rate * 10;
	c->gop_size = 15;
	c->rc_buffer_aggressivity = 1.0f;
	c->rc_buffer_size = 320 * 1024;
	c->rc_strategy = 2;
	c->rc_qsquish = 1.0f;

#if FFMPEG_VERSION_INT > 0x408
	c->flags = CODEC_FLAG_TRELLIS_QUANT;
	c->max_b_frames = 1;
	c->rc_initial_buffer_occupancy = 320 * 768;

	/* Give the muxer the magic numbers. */

	// m->context->max_delay = (int)(0.7 * AV_TIME_BASE);
	// m->context->mux_rate = 1411200;
	// m->context->packet_size = 2324;
	// m->context->preload = (int)(0.44 * AV_TIME_BASE);
#endif

	if (av_set_parameters(m->context, NULL) < 0 ||
	    avcodec_open(c, codec) < 0) {
		slMessage(DEBUG_ALL, "error opening video output codec\n");
		av_free(st);
		av_free(m->context);
		delete m;
		return NULL;
	}
	if (!(m->rgb_pic = avcodec_alloc_frame()) ||
	    !(m->yuv_pic = avcodec_alloc_frame())) {
		slMessage(DEBUG_ALL, "Could not allocate lavc frame\n");
		avcodec_close(c);
		av_free(st);
		av_free(m->context);
		delete m;
		return NULL;
	}

	/*
	 * enc_buf holds the encoded frame, so enc_len must be at least
	 * a reasonable estimate of the size of the largest I-frame.
	 */

	m->enc_len = width * height * 3;
	m->enc_buf = new uint8_t[m->enc_len];

	/* rgb_buf holds the unencoded frame in packed RGB from OpenGL. */

	m->rgb_len = avpicture_get_size(PIX_FMT_RGB24, width, height);
	m->rgb_buf = new uint8_t[m->rgb_len];
	avpicture_fill((AVPicture *)m->rgb_pic, m->rgb_buf, PIX_FMT_RGB24,
	    width, height);

	/* yuv_buf holds the unencoded frame in codec's native pixel format. */

	m->yuv_len = avpicture_get_size(c->pix_fmt, width, height);
	m->yuv_buf = new uint8_t[m->yuv_len];
	avpicture_fill((AVPicture *)m->yuv_pic, m->yuv_buf, c->pix_fmt,
	    width, height);

	m->line = new uint8_t[width * 3]; /* one line of packed RGB pixels */

	av_write_header(m->context);

	return m;
}

/*!
	\brief Encodes a frame and potentially writes it to the movie file.
*/


int slMovieEncodeFrame(slMovie *m) {
	AVCodecContext *c;
	AVStream *st;
	int size;

	if (!m)
		return -1;

	st = m->context->streams[0];
	c = &st->codec;

	/*
	 * Colorspace conversion from RGB to codec's format (likely YUV420P).
	 * OpenGL's RGB pixel format is RGB24 in lavc.
	 */

	img_convert((AVPicture *)m->yuv_pic, c->pix_fmt,
	    (AVPicture *)m->rgb_pic, PIX_FMT_RGB24, c->width, c->height);

	/* Encode the frame yuv_pic storing the output in enc_buf. */

	size = avcodec_encode_video(c, m->enc_buf, m->enc_len, m->yuv_pic);

	/*
	 * If size is 0, the frame is buffered internally by lavc to
	 * allow B-frames to be properly reordered.
	 */

	if (size) {
#if FFMPEG_VERSION_INT <= 0x408
		av_write_frame(m->context, st->index, m->enc_buf, size);
#else
		AVPacket pkt;

		av_init_packet(&pkt);

		pkt.data = m->enc_buf;
		pkt.size = size;
		pkt.stream_index = st->index;

		if (c->coded_frame->key_frame)
			pkt.flags |= PKT_FLAG_KEY;
		pkt.pts = c->coded_frame->pts;

		av_write_frame(m->context, &pkt);
#endif
	}

	return 0;
}

/*!
	\brief Reads from the current OpenGL context to add a frame to a movie.
*/

int slMovieAddWorldFrame(slMovie *m, slWorld *w, slCamera *cam) {
	AVCodecContext *c = &m->context->streams[0]->codec;
	unsigned char *a, *b, *tmp;
	size_t len;
	int y;

	if (cam->activateContextCallback && cam->activateContextCallback()) {
		slMessage(DEBUG_ALL, "Cannot add frame to movie: no OpenGL context available\n");
		return -1;
	}
	if (cam->renderContextCallback)
		cam->renderContextCallback(w, cam);

	glReadPixels(0, 0, c->width, c->height, GL_RGB, GL_UNSIGNED_BYTE, m->rgb_buf);

	/* OpenGL reads bottom-to-top, but encoder expects top-to-bottom. */

	len = c->width * 3;
	tmp = (unsigned char *)m->line;
	for (y = 0; y < c->height >> 1; ++y) {
		a = (unsigned char *)&m->rgb_buf[y * len];
		b = (unsigned char *)&m->rgb_buf[(c->height - y) * len];

		memcpy(tmp, a, len);
		memcpy(a, b, len);
		memcpy(b, tmp, len);
	}

	return slMovieEncodeFrame(m);
}

/*!
	\brief Finishes writing to a movie.
*/

int slMovieFinish(slMovie *m) {
	AVCodecContext *c;
	AVStream *st;
	int n;

	if (!m)
		return -1;

	st = m->context->streams[0];
	c = &st->codec;

	/* Flush any frames left in lavc buffer by encoding a NULL frame. */

	for (int i = 0; i <= c->max_b_frames; ++i) {
		n = avcodec_encode_video(c, m->enc_buf, m->enc_len, NULL);

		if (n) {
#if FFMPEG_VERSION_INT <= 0x408
			av_write_frame(m->context, st->index, m->enc_buf, n);
#else
			AVPacket pkt;

			av_init_packet(&pkt);

			pkt.data = m->enc_buf;
			pkt.size = n;
			pkt.stream_index = st->index;

			if (c->coded_frame->key_frame)
				pkt.flags |= PKT_FLAG_KEY;
			pkt.pts = c->coded_frame->pts;

			av_write_frame(m->context, &pkt);
#endif
		}
	}

	av_write_trailer(m->context);
	url_fclose(&m->context->pb);

	avcodec_close(c);
	av_free(st);
	av_free(m->context);
	av_free(m->rgb_pic);
	av_free(m->yuv_pic);

	delete[] m->enc_buf;
	delete[] m->rgb_buf;
	delete[] m->yuv_buf;
	delete[] m->line;

	delete m;

	return 0;
}

#endif
