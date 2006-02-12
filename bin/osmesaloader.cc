#include <stdio.h>
#include <GL/osmesa.h>
#include <GL/gl.h>

/**
 * A simple loadable plugin to use OSMesa if it is available.  This is 
 * implemented as a plugin so that the light exposure detection decision
 * can be made at runtime instead of compile time.
 */

extern "C" {

OSMesaContext osContext;
int osSize;
unsigned char *osBuffer;

/**
 * Creates an OSMesa rendering context.
 * @param[in] buffer   The offscreen RGBA rendering buffer.  Should be of size ( 4 * size * size ).
 * @param[in] size     The size of one side of the rendering window.
 */

void slOSMesaCreate( unsigned char *buffer, int size ) {
	osContext = OSMesaCreateContext(OSMESA_RGBA, NULL);
	osBuffer = buffer;
	osSize = size;
	printf( "OSMesa extension loaded: offscreen rendering enabled\n" );
}


/**
 * Activates the offscren rendering context.
 * @return  0 if the context was activated successfully, -1 otherwise.
 */

int slOSMesaMakeCurrentContext( ) {
	if (!OSMesaMakeCurrent( osContext, osBuffer, GL_UNSIGNED_BYTE, osSize, osSize ) ) {
		return -1;
	}

	return 0;
}

}
