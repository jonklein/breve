
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

class sl3DSScene {
	
};

class slMesh {
	public:
		slMesh(char *, char *);
		~slMesh();

		double maxReach();

		void draw();

		Lib3dsMesh *_mesh;
};

#endif
