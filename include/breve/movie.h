#include <stdio.h>

#ifdef HAVE_LIBAVCODEC
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
slMovie *slMovieCreate(char *filename, int width, int height, int framerate, float quality);
int slMovieAddFrame(slMovie *m, int flip);
int slMovieAddWorldFrame(slMovie *m, slWorld *w, slCamera *c);
int slMovieFinish(slMovie *m);
#ifdef __cplusplus
}
#endif // __cplusplus 
#endif // HAVE_LIBAVCODEC
