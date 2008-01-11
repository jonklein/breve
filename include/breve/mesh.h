
#include "config.h"
#include "shape.h"

#ifdef HAVE_LIB3DS

#include <lib3ds/file.h>
#include <lib3ds/camera.h>
#include <lib3ds/mesh.h>
#include <lib3ds/node.h>
#include <lib3ds/material.h>
#include <lib3ds/matrix.h>
#include <lib3ds/vector.h>
#include <lib3ds/light.h>

#include "texture.h"

#ifndef WINDOWS
#include <libgen.h>
#endif

struct slMaterial {
	slMaterial() {
					_texture = NULL;
	}

	~slMaterial() {
					if( _texture ) 
						delete _texture;
	}

	float 					_specular[ 4 ];
	float 					_ambient[ 4 ];
	float					_diffuse[ 4 ];
	float					_shininess;

	bool					_twosided;

	std::string				_texturePath;
	slTexture2D*			_texture;
};

class sl3DSShape : public slMeshShape {
	public:
		sl3DSShape( char *, char *, float inSize = 1.0 );
		~sl3DSShape();

		double 							maxReach();

		void 							draw();
		void							draw( slCamera *c, double textureScaleX, double textureScaleY, int mode, int flags );

		void 							calculateSizes( Lib3dsFile *inFile, Lib3dsNode *inNode, int *ioPointStart, int *ioFaceStart );
		void							processNodes( Lib3dsFile *inFile, Lib3dsNode *inNode, int *ioPointStart, int *ioIndexStart );

		Lib3dsMesh*						_mesh;

		int*							_materials;
		Lib3dsMatrix*					_transforms;

		std::vector< slMaterial >		_materialList;

		slVector						_center;
		slVector						_max;


	protected:
		int								addMaterial( std::string &inMaterialName, Lib3dsMaterial *inMaterial );


		float*							_normals;
		float*							_texcoords;

		std::map< std::string, int > 	_materialIndices;
		std::string						_directory;
};


#endif // HAVE_LIB3DS
