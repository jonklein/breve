#include <stdio.h>

#ifdef HAVE_LIBAVCODEC
#include "ffmpeg/avcodec.h"

/*!
	\brief A struct used to export breve movie.
*/

struct slMovie {
	AVCodec *codec;	
	AVCodecContext *context;
	AVFrame *picture;

	unsigned char *buffer;
	int bufferSize;

	unsigned char *YUVpictureBuffer;
	unsigned char *RGBpictureBuffer;
	unsigned char *vvBuffer;
	unsigned char *uuBuffer;

	FILE *file;
};

typedef struct slMovie slMovie;

slMovie *slMovieCreate(char *filename, int width, int height, int framerate, float quality);
int slMovieAddFrame(slMovie *m, int flip);
int slMovieAddGLFrame(slMovie *m, slCamera *c);
int slMovieFinish(slMovie *m);
#endif
