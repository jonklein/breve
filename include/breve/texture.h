#include "util.h"
#include "glIncludes.h"

class slTexture2D {
	public:
		slTexture2D( std::string &image );
		~slTexture2D();

		void bind() { glBindTexture( GL_TEXTURE_2D, _textureID ); }

		void loadPixels( unsigned char *pixels, int width, int height );

	private:
		GLuint _textureID;
};
