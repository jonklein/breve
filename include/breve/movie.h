#include <stdio.h>

#if HAVE_LIBAVCODEC
#include "ffmpeg/avcodec.h"

/*!
	\brief A struct used to export breve movie.
*/

#ifdef __cplusplus
class slMovie {
	public:
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
#endif

typedef struct slMovie slMovie;

#ifdef __cplusplus
extern "C" {
#endif
slMovie *slMovieCreate(char *, int, int, int, float);
int slMovieAddFrame(slMovie *, int);
int slMovieAddWorldFrame(slMovie *, slWorld *, slCamera *);
int slMovieFinish(slMovie *);
#ifdef __cplusplus
}
#endif
#endif
