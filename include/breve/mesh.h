
#include <map>

#include "config.h"
#include "shape.h"
#include "texture.h"
#include "render.h"

class slMeshShape : public slShape {
	public:
		slMeshShape();
		~slMeshShape();
		
		void 						bounds( const slPosition *position, slVector *min, slVector *max ) const;

		virtual void				finishShape( double inDensity );
		virtual void				finishShapeWithMaxLength( double inDensity, float inMaxLength );

		virtual void				updateLastPosition( slPosition *inPosition );

	protected:
		int							createODEGeom();

		int*						_materials;
	
		float*						_vertices;
		float*						_normals;
		float*						_texcoords;

		int*						_indices;

		std::vector< slMaterial >	_materialList;

		int 						_vertexCount;
		int 						_indexCount;

		dMatrix4                    _lastPositions[ 2 ];
		int                         _lastPositionIndex;

		float						_maxReach;
};

#ifdef HAVE_LIB3DS

#include <lib3ds/file.h>
#include <lib3ds/camera.h>
#include <lib3ds/mesh.h>
#include <lib3ds/node.h>
#include <lib3ds/material.h>
#include <lib3ds/matrix.h>
#include <lib3ds/vector.h>
#include <lib3ds/light.h>

class sl3DSShape : public slMeshShape {
	public:
		sl3DSShape( char *, char *, float inSize = 1.0 );
		~sl3DSShape();

		double 							maxReach();

		virtual void 					draw( const slRenderGL& inRender );

		Lib3dsMesh*						_mesh;
		Lib3dsMatrix*					_transforms;


		slVector						_center;
		slVector						_max;


	protected:
		void 							calculateSizes( Lib3dsFile *inFile, Lib3dsNode *inNode, int *ioPointStart, int *ioFaceStart );
		void							processNodes( Lib3dsFile *inFile, Lib3dsNode *inNode, int *ioPointStart, int *ioIndexStart );

		int								addMaterial( std::string &inMaterialName, Lib3dsMaterial *inMaterial );

		std::map< std::string, int > 	_materialIndices;
		std::string						_directory;
};


#endif // HAVE_LIB3DS
