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

	char *buffer;
	int bufferSize;

	char *pictureBuffer;

	FILE *file;
};

typedef struct slMovie slMovie;

slMovie *slMovieCreate(char *filename, int width, int height, int framerate, float quality);
int slMovieAddFrame(slMovie *m, unsigned char *pixels);
int slMovieAddGLFrame(slMovie *m, slCamera *c);
int slMovieFinish(slMovie *m);
#endif
