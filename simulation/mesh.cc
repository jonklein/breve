
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include <string>
#include <map>

#include "simulation.h"
#include "mesh.h"
#include "gldraw.h"

#ifdef HAVE_LIB3DS

sl3DSShape::sl3DSShape( char *inFilename, char *inMeshname, float inSize ) : slMeshShape() {
	_normals = NULL;
	_materials = NULL;

	if( !inFilename ) 
		throw slException( std::string( "Cannot locate 3DS Mesh file" ) );

	slMessage( 20, "Reading 3DS file \"%s\"\n", inFilename );

	Lib3dsFile *file = lib3ds_file_load( inFilename );

	if ( !file ) 
		throw slException( std::string( "Cannot open file \"" ) + inFilename + "\"" );

	lib3ds_file_eval( file, 0.0 );

	_directory = dirname( inFilename );
	
	_vertexCount = 0;
	_indexCount = 0;

	calculateSizes( file, file -> nodes, &_vertexCount, &_indexCount );

	_vertices = new float[ 3 * _vertexCount ];
	_texcoords = new float[ 2 * _vertexCount ];
	_indices = new int[ _indexCount ];
	_normals = new float[ _indexCount * 3 ];
	_materials = new int[ _indexCount ];

	_maxReach = 0.0;

	// First pass through the data, translate to the origin and get the length

	int pointStart = 0, indexStart = 0;

	slVectorSet( &_max, 0, 0, 0 );
	slVectorSet( &_center, 0, 0, 0 );

	processNodes( file, file -> nodes, &pointStart, &indexStart );

	// slVectorMul( &_center, 1.0 / _vertexCount, &_center );
	slVectorSub( &_max, &_center, &_max );
	_maxReach = slVectorLength( &_max );

	for( int n = 0; n < _vertexCount; n++ ) {
		_vertices[ n * 3     ] -= _center.x;
		_vertices[ n * 3 + 1 ] -= _center.y;
		_vertices[ n * 3 + 2 ] -= _center.z;

		if( inSize > 0.0f ) {
			_vertices[ n * 3     ] *= ( inSize / _maxReach );
			_vertices[ n * 3 + 1 ] *= ( inSize / _maxReach );
			_vertices[ n * 3 + 2 ] *= ( inSize / _maxReach );
		}
	}

	if( inSize > 0.0f )
		_maxReach = 1.2 * inSize;

	_maxReach = 100;

	slMessage( 20, "%d points and %d indices processed, max reach = %f\n", pointStart, indexStart, _maxReach );

	setDensity( 1 );
	createODEGeom();
}

void sl3DSShape::processNodes( Lib3dsFile *inFile, Lib3dsNode *inNode, int *ioPointStart, int *ioIndexStart ) {
	unsigned int n;

	while( inNode ) {
		processNodes( inFile, inNode -> childs, ioPointStart, ioIndexStart );

		if( inNode -> type == LIB3DS_OBJECT_NODE ) {
			if( !strcmp( inNode -> name, "$$$DUMMY" ) )
				return;

			Lib3dsMesh *mesh = lib3ds_file_mesh_by_name( inFile, inNode->name );

			Lib3dsMatrix M, N;
			lib3ds_matrix_copy( M, mesh->matrix );
			lib3ds_matrix_inv( M );
			lib3ds_matrix_copy( N, inNode->matrix );

			slMessage( 20, "Node %s\n", inNode -> name );
			slMessage( 20, "Node translation: %f, %f, %f\n", inNode->matrix[ 3 ][ 0 ],  inNode->matrix[ 3 ][ 1 ], inNode->matrix[ 3 ][ 2 ] );
			slMessage( 20, "Mesh translation: %f, %f, %f\n", M[ 3 ][ 0 ],  M[ 3 ][ 1 ], M[ 3 ][ 2 ] );
			slMessage( 20, "Pivot translation: %f, %f, %f\n", inNode->data.object.pivot[ 0 ], inNode->data.object.pivot[ 1 ], inNode->data.object.pivot[ 2 ] );

			slMatrix trans = { 
				{ 0, -1, 0 },
				{ 0, 0, 1 },
				{ 1, 0, 0 } };

			for ( n = 0; n < mesh -> points; n++ ) {
				slVector v1, v2;

				v1.x = mesh->pointL[ n ].pos[ 0 ];
				v1.y = mesh->pointL[ n ].pos[ 1 ];
				v1.z = mesh->pointL[ n ].pos[ 2 ];

				v2.x = M[ 0 ][ 0 ] * v1.x + M[ 1 ][ 0 ] * v1.y + M[ 2 ][ 0 ] * v1.z + M[ 3 ][ 0 ];
				v2.y = M[ 0 ][ 1 ] * v1.x + M[ 1 ][ 1 ] * v1.y + M[ 2 ][ 1 ] * v1.z + M[ 3 ][ 1 ];
				v2.z = M[ 0 ][ 2 ] * v1.x + M[ 1 ][ 2 ] * v1.y + M[ 2 ][ 2 ] * v1.z + M[ 3 ][ 2 ];

				slVectorCopy( &v2, &v1 );

				v2.x = N[ 0 ][ 0 ] * v1.x + N[ 1 ][ 0 ] * v1.y + N[ 2 ][ 0 ] * v1.z + N[ 3 ][ 0 ];
				v2.y = N[ 0 ][ 1 ] * v1.x + N[ 1 ][ 1 ] * v1.y + N[ 2 ][ 1 ] * v1.z + N[ 3 ][ 1 ];
				v2.z = N[ 0 ][ 2 ] * v1.x + N[ 1 ][ 2 ] * v1.y + N[ 2 ][ 2 ] * v1.z + N[ 3 ][ 2 ];

				// slVectorCopy( &v2, &v1 );
				// slVectorXform( trans, &v1, &v2 );

				_vertices[ ( *ioPointStart + n ) * 3     ] = v2.x;
				_vertices[ ( *ioPointStart + n ) * 3 + 1 ] = v2.y;
				_vertices[ ( *ioPointStart + n ) * 3 + 2 ] = v2.z;

				float tx = 0, ty = 0;

				if( mesh->texelL ) {
					tx = mesh->texelL[ n ][ 0 ];
					ty = mesh->texelL[ n ][ 1 ];
				}

				_texcoords[ ( *ioPointStart + n ) * 2     ] = tx;
				_texcoords[ ( *ioPointStart + n ) * 2 + 1 ] = ty;

				if( slVectorLength( &v2 ) > slVectorLength( &_max ) )
					slVectorCopy( &v2, &_max );

				slVectorMul( &v2, 1.0 / _vertexCount, &v2 );
				slVectorAdd( &_center, &v2, &_center );
			}

			Lib3dsVector *normals = new Lib3dsVector[ _indexCount ];
			lib3ds_mesh_calculate_normals( mesh, normals );

			for( unsigned int n = 0; n < mesh -> faces; n++ ) {
				Lib3dsFace *f = &mesh -> faceL[ n ];

				for ( int m = 0; m < 3; m++ ) {
					_indices[ *ioIndexStart + n * 3 + m ] = f -> points[ m ] + *ioPointStart;

					_normals[ ( *ioIndexStart + n * 3 + m ) * 3     ] = normals[ n * 3 + m ][ 0 ];
					_normals[ ( *ioIndexStart + n * 3 + m ) * 3 + 1 ] = normals[ n * 3 + m ][ 1 ];
					_normals[ ( *ioIndexStart + n * 3 + m ) * 3 + 2 ] = normals[ n * 3 + m ][ 2 ];
				}

				// check for the material

				if( f -> material && strlen( f -> material ) ) {
					std::string materialName = std::string( f -> material );

					if( !_materialIndices[ materialName ] ) {
						Lib3dsMaterial *faceMaterial = lib3ds_file_material_by_name( inFile, f -> material );
						_materialIndices[ materialName ] = addMaterial( materialName, faceMaterial );

					}
				
					_materials[ *ioIndexStart / 3 + n ] = _materialIndices[ materialName ] - 1;

					if( _materialList[ _materialIndices[ materialName ] - 1 ]._texturePath.size() > 0 && !mesh -> texelL ) 
						printf( "found a texture for the face, but no tex coords\n" );
				}

			}

			*ioPointStart += mesh->points;

			*ioIndexStart += mesh->faces * 3;
   		}

		inNode = inNode -> next;
	}
}

int sl3DSShape::addMaterial( std::string &inMaterialName, Lib3dsMaterial *inMaterial ) {

	if( !inMaterial ) 
		return 0;

	slMaterial material;

	slMessage( 20, "Found material: %s\n", inMaterialName.c_str() );
	memcpy( material._specular, inMaterial -> specular, sizeof( float ) * 4 );
	memcpy( material._diffuse, inMaterial -> diffuse, sizeof( float ) * 4 );
	memcpy( material._ambient, inMaterial -> ambient, sizeof( float ) * 4 );

	material._ambient[ 3 ] = 1.0;
	material._diffuse[ 3 ] = 1.0;
	material._specular[ 3 ] = 1.0;

	material._shininess = inMaterial -> shininess;

	if( inMaterial -> texture1_map.name && strlen( inMaterial -> texture1_map.name ) ) {
		material._texturePath = _directory + "/";
		material._texturePath += inMaterial -> texture1_map.name;

		printf( "Texture name = %s\n", material._texturePath.c_str() );
	}

	_materialList.push_back( material );

	return _materialList.size();

}

sl3DSShape::~sl3DSShape() {
	if( _normals )
		delete[] _normals;

	if( _texcoords )
		delete[] _texcoords ;

	if( _materials )
		delete[] _materials;
}

void sl3DSShape::calculateSizes( Lib3dsFile *inFile, Lib3dsNode *inNode, int *outPoints, int *outIndices ) {

	while( inNode ) {
		calculateSizes( inFile, inNode -> childs, outPoints, outIndices );

		if( inNode -> type == LIB3DS_OBJECT_NODE ) {
			if( !strcmp( inNode -> name, "$$$DUMMY" ) )
				return;

			Lib3dsMesh *mesh = lib3ds_file_mesh_by_name( inFile, inNode->name );

			if( mesh ) {
				*outPoints += mesh->points;
				*outIndices += mesh->faces * 3;
			}
		}

		inNode = inNode -> next;
	}
}

double sl3DSShape::maxReach() {
	return _maxReach;
}

void sl3DSShape::draw() {
	slMaterial *material = NULL;

	glPushAttrib( GL_TRANSFORM_BIT );

	for ( int n = 0; n < _indexCount; n += 3 ) {
		int materialIndex = n / 3;

		if( _materials[ materialIndex ] > -1 && &_materialList[ _materials[ materialIndex ] ] != material ) {
			material = &_materialList[ _materials[ materialIndex ] ];
		
			glMaterialfv( GL_FRONT_AND_BACK, GL_AMBIENT, material -> _ambient );
			glMaterialfv( GL_FRONT_AND_BACK, GL_DIFFUSE, material -> _diffuse );
			glMaterialfv( GL_FRONT_AND_BACK, GL_SPECULAR, material -> _specular );

			float shine = pow( 2.0, 10.0 * material -> _shininess );

			if( shine > 128.0 )
				shine = 128.0;

			glMaterialf( GL_FRONT_AND_BACK, GL_SHININESS, shine ); 

			// glColor4fv( material -> _diffuse );

			// I CAN HAZ TEXTURZ?

			if( material -> _texture ) {
			 	material -> _texture -> bind();
				glMatrixMode( GL_TEXTURE );
				glLoadIdentity();
				glScalef( material -> _texture -> _unitX, material -> _texture -> _unitY, 1 );
			} else 
				glDisable( GL_TEXTURE_2D );

			glTexEnvf( GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE );
		}

		int i1 = _indices[ n ];
		int i2 = _indices[ n + 1 ];
		int i3 = _indices[ n + 2 ];


		glBegin( GL_TRIANGLES );
		glNormal3fv( &_normals[ n * 3 ] );
		glTexCoord2fv( &_texcoords[ i1 * 2 ] );
		glVertex3fv( &_vertices[ i1 * 3 ] );

		glNormal3fv( &_normals[ n * 3 + 3 ] );
		glTexCoord2fv( &_texcoords[ i2 * 2 ] );
		glVertex3fv( &_vertices[ i2 * 3 ] );

		glNormal3fv( &_normals[ n * 3 + 6 ] );
		glTexCoord2fv( &_texcoords[ i3 * 2 ] );
		glVertex3fv( &_vertices[ i3 * 3 ] );
		glEnd();
	 }

	glPopAttrib();
}

void sl3DSShape::draw( slCamera *c, double textureScaleX, double textureScaleY, int mode, int flags ) {
	for( int n = 0; n < _materialList.size(); n++ ) {
		slMaterial *material = &_materialList[ n ];

		if( !material -> _texture && material -> _texturePath.size() > 0 )
			material -> _texture = new slTexture2D( material -> _texturePath );
	}

	glDisable( GL_CULL_FACE );

	if ( _drawList == 0 || _recompile ) {
		if ( _drawList == 0 ) 
			_drawList = glGenLists( 1 );

		glNewList( _drawList, GL_COMPILE );

		draw();

		glEndList();

		_recompile = 0;
	}

	if( !( flags & DO_NO_COLOR ) ) 
		glDisable( GL_COLOR_MATERIAL );

	glCallList( _drawList );

	glEnable( GL_COLOR_MATERIAL );
}

#endif
