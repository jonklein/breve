
#include <stdio.h>                        
#include <stdlib.h>                        
#include <math.h>

#include "util.h"
#include "mesh.h"
#include "glIncludes.h"

#ifdef HAVE_LIB3DS

slMesh::slMesh(char *filename, char *meshname) {
	Lib3dsFile *file;

	file = lib3ds_file_load(filename);

	if(!file) throw slException(std::string("cannot open file \"") + meshname + "\"");

	meshname = "faucet";

	if(! meshname) meshname = file->nodes->name;

	_mesh = lib3ds_file_mesh_by_name(file, meshname);

	if( !_mesh) meshname = file->nodes->name;

	_mesh = lib3ds_file_mesh_by_name(file, meshname);

	if( ! _mesh) throw slException(std::string("cannot locate mesh in file \"") + meshname + "\"");

	if(_mesh) {
		int n;

		printf("found node %s, %d faces\n", meshname, _mesh->faces);
		printf("at: %f, %f, %f\n", _mesh->matrix[3][0], _mesh->matrix[3][1], _mesh->matrix[3][2]);

		// translate the mesh to the origin

		for(n = 0; n < _mesh->points; n++ ) {
			_mesh->pointL[n].pos[0] -= _mesh->matrix[3][0];
			_mesh->pointL[n].pos[1] -= _mesh->matrix[3][1];
			_mesh->pointL[n].pos[2] -= _mesh->matrix[3][2];
		}
	}
}

slMesh::~slMesh() {
	
}

double slMesh::maxReach() {
	Lib3dsFace *f;
	double maxD = 0;
	unsigned int n, m;

	for(n = 0; n < _mesh->faces; n++ ) {
		slVector d;

		f = &_mesh->faceL[n];

		for(m=0; m < 3; m++) {
			d.x = _mesh->pointL[f->points[m]].pos[0];
			d.y = _mesh->pointL[f->points[m]].pos[1];
			d.z = _mesh->pointL[f->points[m]].pos[2];

			if(slVectorLength(&d) > maxD) maxD = slVectorLength(&d);
		}
	}

	return maxD;
}

void slMesh::draw() {
	unsigned int n;
	Lib3dsFace *f;

	glDisable(GL_CULL_FACE);
	glBegin(GL_TRIANGLES);
		
	for(n = 0; n < _mesh->faces; n++ ) {
		f = &_mesh->faceL[n];

		glVertex3fv(_mesh->pointL[f->points[0]].pos);
		glVertex3fv(_mesh->pointL[f->points[1]].pos);
		glVertex3fv(_mesh->pointL[f->points[2]].pos);
	}

	glEnd();	

	// printf("drew %d faces\n", _mesh->faces);
	// printf("last at %f, %f, %f\n", _mesh->pointL[f->points[2]].pos[0], _mesh->pointL[f->points[2]].pos[1], _mesh->pointL[f->points[2]].pos[2]);
}

#endif
