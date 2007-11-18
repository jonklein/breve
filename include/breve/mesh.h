
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


#include <ode/ode.h>

class sl3DSShape : public slMeshShape {
	public:
		sl3DSShape( char *, char *, float inSize = 1.0 );
		~sl3DSShape();

		double 						maxReach();

		void 						draw();
		void						draw( slCamera *c, slPosition *pos, double textureScaleX, double textureScaleY, int mode, int flags );

		void 						calculateSizes( Lib3dsNode *inNode );

		Lib3dsMesh*					_mesh;

		float*						_normals;
};

#endif // HAVE_LIB3DS
