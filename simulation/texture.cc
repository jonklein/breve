#include "texture.h"
#include "image.h"

#define MAX( x, y ) ( (x)>(y)?(x):(y) )

slTexture2D::slTexture2D( std::string &file ) {
	unsigned char *pixels;
	int height, width, components;

	pixels = slReadImage( file.c_str(), &width, &height, &components, 1 );

	if( !pixels ) throw 
		slException( std::string( "Could not read image file " ) + file );

	glGenTextures( 1, &_textureID );

	if( !_textureID ) throw slException( "Could not generate texture ID" );

	loadPixels( pixels, width, height );

	delete[] pixels;
}

slTexture2D::~slTexture2D() {
	glDeleteTextures( 1, &_textureID );
}

void slTexture2D::loadPixels( unsigned char *pixels, int width, int height ) {
    int newwidth = slNextPowerOfTwo(width);
    int newheight = slNextPowerOfTwo(height);

    newwidth = newheight = MAX(newwidth, newheight);

	bind();

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);

    if (newwidth != width || newheight != height) {
    	int xstart, ystart;

        unsigned char *newpixels = new unsigned char[newwidth * newheight * 4];

        memset(newpixels, 0, newwidth * newheight * 4);

	    xstart = (newwidth - width) / 2;
		ystart = (newheight - height) / 2;

	    for( int y = 0; y < height; y++)
			memcpy( &pixels[(y + ystart) * (newwidth * 4) + (xstart * 4)], &pixels[y * width * 4], width * 4 );

		glTexImage2D( GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, newpixels );

		delete[] newpixels;
	} else {
		glTexImage2D( GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, pixels );
	}
}

#undef MAX
