#include "simulation.h"
#include "kernel.h"
#include "texture.h"

/*!
 * \brief Holds image data used by the breve image class.
 */

class brImageData {
    public:
        brImageData() {
            _data = NULL;
            _texture = NULL;
            _width = 0;
            _height = 0;
        }

        ~brImageData() {
            if( _data )
                delete _data;

            if( _texture )
                delete _texture;
        }

        void                    updateTexture();
        slTexture2D*			getTexture();

        unsigned char*          _data;
        int                     _width;
        int                     _height;

	protected:
        slTexture2D*            _texture;
};

void breveInitImageFuncs( brNamespace *inNamespace );
