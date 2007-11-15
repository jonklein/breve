
#include "config.h"

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

class sl3DSScene {
	
};

class slMesh {
	public:
		slMesh( char *, char *, float inSize = 1.0 );
		~slMesh();

		double maxReach();

		void draw();

		Lib3dsMesh *_mesh;

		float*						_vertices;
		float*						_normals;
		int							_vertexCount;

		int* 						_indices;
		int							_indexCount;

		float						_maxReach;


		dMatrix4					_lastPositions[ 2 ];
		int 						_lastPositionIndex;
};

#endif // HAVE_LIB3DS
