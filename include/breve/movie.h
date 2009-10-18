#if HAVE_LIBAVCODEC && HAVE_LIBAVFORMAT && HAVE_LIBAVUTIL && HAVE_LIBSWSCALE

#define HAVE_MOVIE_EXPORT

#include "glIncludes.h"
#include "camera.h"

extern "C" {
	#include <libavformat/avformat.h>
	#include <libavcodec/avcodec.h>  
	#include <libswscale/swscale.h>  
}

/**
 * \brief A class used to export breve movie.
 */

class slMovie {
	public:
		AVCodecContext *_context;
		AVFrame *_rgb_pic, *_yuv_pic;

		FILE *_fp;

		uint8_t *_enc_buf, *_rgb_buf, *_yuv_buf, *_line;
		unsigned int _enc_len, _rgb_len, _yuv_len;
};

slMovie *slMovieCreate(char *, int, int);
int slMovieAddWorldFrame(slMovie *, slWorld *, slCamera *);
int slMovieFinish(slMovie *);

#endif
