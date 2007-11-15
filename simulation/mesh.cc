
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include <string>

#include "simulation.h"
#include "mesh.h"
#include "glIncludes.h"

#ifdef HAVE_LIB3DS

slMesh::slMesh( char *filename, char *meshname, float inSize ) {
	Lib3dsFile *file;

	file = lib3ds_file_load( filename );

	if ( !file ) 
		throw slException( std::string( "cannot open file \"" ) + filename + "\"" );

	if ( !strcmp( meshname, "" ) ) {
		if( file -> meshes ) 
			meshname = file-> meshes -> name;
		else if( file -> nodes ) 
			meshname = file-> nodes -> name;
	}
	
	_mesh = lib3ds_file_mesh_by_name( file, meshname );

	if ( ! _mesh ) 
		throw slException( std::string( "cannot locate mesh in file \"" ) + meshname + "\"" );

	if ( _mesh ) {
		unsigned int n;

		_vertexCount = _mesh->points;
		_vertices = new float[ 3 * _vertexCount ];
		_normals = new float[ 3 * _vertexCount ];

		_maxReach = 0.0;

		// First pass through the data, translate to the origin and get the length

		for ( n = 0; n < _mesh->points; n++ ) {
			_mesh->pointL[n].pos[0] -= _mesh->matrix[3][0];
			_mesh->pointL[n].pos[1] -= _mesh->matrix[3][1];
			_mesh->pointL[n].pos[2] -= _mesh->matrix[3][2];

			slVector v;

			v.x = _mesh->pointL[ n ].pos[ 0 ];
			v.y = _mesh->pointL[ n ].pos[ 1 ];
			v.z = _mesh->pointL[ n ].pos[ 2 ];

			float size = slVectorLength( &v );

			if( size > _maxReach )
				_maxReach = size;
		}

		// Second pass, normalize by the size and note the vertices

		for ( n = 0; n < _mesh->points; n++ ) {
			_mesh->pointL[ n ].pos[ 0 ] /= ( _maxReach / inSize );
			_mesh->pointL[ n ].pos[ 1 ] /= ( _maxReach / inSize );
			_mesh->pointL[ n ].pos[ 2 ] /= ( _maxReach / inSize );

			_vertices[ n * 3     ] = _mesh->pointL[ n ].pos[ 0 ];
			_vertices[ n * 3 + 1 ] = _mesh->pointL[ n ].pos[ 1 ];
			_vertices[ n * 3 + 2 ] = _mesh->pointL[ n ].pos[ 2 ];
		}

		_maxReach = inSize;

		_indexCount = _mesh->faces * 3;

		_indices = new int[ _indexCount ];
		_normals = new float[ _indexCount ];

		for ( n = 0; n < _mesh -> faces; n++ ) {
			Lib3dsFace *f = &_mesh -> faceL[ n ];
			slVector points[ 3 ], vectors[ 3 ];

			for ( int m = 0; m < 3; m++ ) {
				_indices[ n * 3 + m ] = f -> points[ m ];

				slVectorSet( &points[ m ], 
					_vertices[ f -> points[ m ] * 3     ],
					_vertices[ f -> points[ m ] * 3 + 1 ],
					_vertices[ f -> points[ m ] * 3 + 2 ]
				);
			}

			slVectorSub( &points[ 1 ], &points[ 0 ], &vectors[ 0 ] );
			slVectorSub( &points[ 2 ], &points[ 0 ], &vectors[ 1 ] );

			slVectorCross( &vectors[ 0 ], &vectors[ 1 ], &vectors[ 2 ] );

			slVectorNormalize( &vectors[ 2 ] );

			_normals[ n * 3     ] = vectors[ 2 ].x;
			_normals[ n * 3 + 1 ] = vectors[ 2 ].y;
			_normals[ n * 3 + 2 ] = vectors[ 2 ].z;
    	}
	}

	_lastPositionIndex = 0;

}

slMesh::~slMesh() {
	delete[] _indices;
	delete[] _vertices;
}

double slMesh::maxReach() {
	return _maxReach;
}

void slMesh::draw() {
	unsigned int n;
	Lib3dsFace *f;

	glDisable( GL_CULL_FACE );
	glBegin( GL_TRIANGLES );

	for ( n = 0; n < _mesh->faces; n++ ) {
		f = &_mesh->faceL[ n ];

		glNormal3fv( &_normals[ n * 3 ] );

		glVertex3fv( _mesh->pointL[ f->points[ 0 ] ].pos );
		glVertex3fv( _mesh->pointL[ f->points[ 1 ] ].pos );
		glVertex3fv( _mesh->pointL[ f->points[ 2 ] ].pos );
	 }

	glEnd();
}

#endif
