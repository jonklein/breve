#if HAVE_LIBAVCODEC

#include <ffmpeg/avformat.h>

/*!
	\brief A class used to export breve movie.
*/

class slMovie {
	public:
		AVFormatContext *context;
		AVFrame *rgb_pic, *yuv_pic;

		uint8_t *enc_buf, *rgb_buf, *yuv_buf, *line;
		unsigned int enc_len, rgb_len, yuv_len;
};

slMovie *slMovieCreate(char *, int, int);
int slMovieAddWorldFrame(slMovie *, slWorld *, slCamera *);
int slMovieFinish(slMovie *);

#endif
