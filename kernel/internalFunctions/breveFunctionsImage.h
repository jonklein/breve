typedef struct brImageData brImageData;

/*!
	\brief Holds image data used by the breve image class.
*/

struct brImageData {
	unsigned char *data;
	int x;
	int y;	
	int textureNumber;
};

void breveInitImageFuncs(brNamespace *n);
