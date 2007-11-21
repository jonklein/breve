
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include <string>

#include "simulation.h"
#include "mesh.h"
#include "gldraw.h"

#ifdef HAVE_LIB3DS

sl3DSShape::sl3DSShape( char *inFilename, char *inMeshname, float inSize ) : slMeshShape() {
	
	if( !inFilename ) 
		throw slException( std::string( "Cannot locate 3DS Mesh file" ) );

	Lib3dsFile *file = lib3ds_file_load( inFilename );

	if ( !file ) 
		throw slException( std::string( "Cannot open file \"" ) + inFilename + "\"" );

	calculateSizes( file -> nodes );

	if ( !strcmp( inMeshname, "" ) ) {
		if( file -> meshes ) 
			inMeshname = file-> meshes -> name;
	}

	_mesh = lib3ds_file_mesh_by_name( file, inMeshname );

	if ( ! _mesh ) 
		throw slException( std::string( "cannot locate mesh in file \"" ) + inMeshname + "\"" );

	unsigned int n;

	_vertexCount = _mesh->points;
	_indexCount = _mesh->faces * 3;

	_vertices = new float[ 3 * _vertexCount ];
	_indices = new int[ _indexCount ];
	_normals = new float[ _mesh -> faces * 3 * 3 ];

	_maxReach = 0.0;

	// First pass through the data, translate to the origin and get the length

	// slMatrix transform = { 
	//	_mesh->matrix[ 0 ][ 0 ], _mesh->matrix[ 1 ][ 0 ], _mesh->matrix[ 2 ][ 0 ],
	//	_mesh->matrix[ 0 ][ 1 ], _mesh->matrix[ 1 ][ 1 ], _mesh->matrix[ 2 ][ 1 ],
	//	_mesh->matrix[ 0 ][ 2 ], _mesh->matrix[ 1 ][ 2 ], _mesh->matrix[ 2 ][ 2 ] };

	slVector translate = { -_mesh->matrix[ 3 ][ 0 ], -_mesh->matrix[ 3 ][ 1 ], -_mesh->matrix[ 3 ][ 2 ] };

	for ( n = 0; n < _mesh->points; n++ ) {
		_mesh->pointL[ n ].pos[ 0 ] += translate.x;
		_mesh->pointL[ n ].pos[ 1 ] += translate.y;
		_mesh->pointL[ n ].pos[ 2 ] += translate.z;

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

	Lib3dsVector *normals = new Lib3dsVector[ _indexCount ];
	lib3ds_mesh_calculate_normals( _mesh, normals );

	for ( n = 0; n < _mesh -> faces; n++ ) {
		Lib3dsFace *f = &_mesh -> faceL[ n ];

		for ( int m = 0; m < 3; m++ ) {
			_indices[ n * 3 + m ] = f -> points[ m ];

			_normals[ ( n * 3 + m ) * 3     ] = normals[ n * 3 + m ][ 0 ];
			_normals[ ( n * 3 + m ) * 3 + 1 ] = normals[ n * 3 + m ][ 1 ];
			_normals[ ( n * 3 + m ) * 3 + 2 ] = normals[ n * 3 + m ][ 2 ];
		}
   	}

	setDensity( 1 );
	createODEGeom();
}

sl3DSShape::~sl3DSShape() {
}

void sl3DSShape::calculateSizes( Lib3dsNode *inNode ) {
	while( inNode ) {
		
		calculateSizes( inNode -> childs );

		if( inNode -> type == LIB3DS_OBJECT_NODE ) {
			// printf( "Node name = %s, %s\n", inNode -> name, inNode -> data.object.morph  );
			// Lib3dsMesh *mesh = lib3ds_file_mesh_by_name(file, node->name);
		}

		inNode = inNode -> next;
	}
}

double sl3DSShape::maxReach() {
	return _maxReach;
}

void sl3DSShape::draw() {
	unsigned int n;
	Lib3dsFace *f;

	// glNormalPointer( GL_FLOAT, 0, _normals );

	glBegin( GL_TRIANGLES );

	for ( n = 0; n < _mesh->faces; n++ ) {
		f = &_mesh->faceL[ n ];

		glNormal3fv( &_normals[ n * 3 * 3     ] );
		glVertex3fv( _mesh->pointL[ f->points[ 0 ] ].pos );
		glNormal3fv( &_normals[ n * 3 * 3 + 3 ] );
		glVertex3fv( _mesh->pointL[ f->points[ 1 ] ].pos );
		glNormal3fv( &_normals[ n * 3 * 3 + 6 ] );
		glVertex3fv( _mesh->pointL[ f->points[ 2 ] ].pos );
	 }

	glEnd();
}

void sl3DSShape::draw( slCamera *c, slPosition *pos, double textureScaleX, double textureScaleY, int mode, int flags ) {
	float scale[4] = { 1.0 / textureScaleX, 1.0 / textureScaleY, 1.0, 1.0 };

	glPushMatrix();
	glTranslated( pos->location.x, pos->location.y, pos->location.z );
	slMatrixGLMult( pos->rotation );

	glTexGeni( GL_S, GL_TEXTURE_GEN_MODE, GL_OBJECT_LINEAR );
	glTexGenfv( GL_S, GL_OBJECT_PLANE, scale );
	glTexGeni( GL_T, GL_TEXTURE_GEN_MODE, GL_OBJECT_LINEAR );
	glTexGenfv( GL_T, GL_OBJECT_PLANE, scale );

	glEnable( GL_TEXTURE_GEN_S );
	glEnable( GL_TEXTURE_GEN_T );

	if ( _drawList == 0 || _recompile ) {
		if ( _drawList == 0 ) 
			_drawList = glGenLists( 1 );

		glNewList( _drawList, GL_COMPILE );

		draw();

		glEndList();
	}

	glCallList( _drawList );

	glDisable( GL_TEXTURE_GEN_S );
	glDisable( GL_TEXTURE_GEN_T );

	glPopMatrix();

	updateLastPosition( pos );
}

#endif
