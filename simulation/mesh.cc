
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

	Lib3dsFile *file = lib3ds_file_load( inFilename );

	if ( !file ) 
		throw slException( std::string( "Cannot open file \"" ) + inFilename + "\"" );

	_directory = dirname( inFilename );
	
	_vertexCount = 0;
	_indexCount = 0;

	calculateSizes( file, file -> nodes, &_vertexCount, &_indexCount );

	printf( "%d points, %d faces\n", _vertexCount, _indexCount );

	_vertices = new float[ 3 * _vertexCount ];
	_texcoords = new float[ 2 * _vertexCount ];
	_indices = new int[ _indexCount ];
	_normals = new float[ _indexCount * 3 ];
	_materials = new int[ _indexCount ];

	_maxReach = 0.0;

	// First pass through the data, translate to the origin and get the length

	int pointStart = 0, indexStart = 0;
	processNodes( file, file -> nodes, &pointStart, &indexStart );

	printf( "%d points and %d indices processed, max reach = %f\n", pointStart, indexStart, _maxReach );

	if( inSize > 0 ) {
		for( int n = 0; n < 3 * _vertexCount; n++ ) {
			_vertices[ n ] *= ( inSize / _maxReach );
		}

		_maxReach = inSize;
	}


	setDensity( 1 );
	createODEGeom();
}

int sl3DSShape::processNodes( Lib3dsFile *inFile, Lib3dsNode *inNode, int *ioPointStart, int *ioIndexStart ) {
	unsigned int n;

        while( inNode ) {
                processNodes( inFile, inNode -> childs, ioPointStart, ioIndexStart );

                if( inNode -> type == LIB3DS_OBJECT_NODE ) {
			if( !strcmp( inNode -> name, "$$$DUMMY" ) )
				return;

                        Lib3dsMesh *mesh = lib3ds_file_mesh_by_name( inFile, inNode->name );

			// slMatrix transform = { 
			//	_mesh->matrix[ 0 ][ 0 ], _mesh->matrix[ 1 ][ 0 ], _mesh->matrix[ 2 ][ 0 ],
			//	_mesh->matrix[ 0 ][ 1 ], _mesh->matrix[ 1 ][ 1 ], _mesh->matrix[ 2 ][ 1 ],
			//	_mesh->matrix[ 0 ][ 2 ], _mesh->matrix[ 1 ][ 2 ], _mesh->matrix[ 2 ][ 2 ] };
			// slVector translate = { -mesh->matrix[ 3 ][ 0 ], -mesh->matrix[ 3 ][ 1 ], -mesh->matrix[ 3 ][ 2 ] };

			for ( n = 0; n < mesh -> points; n++ ) {
				_vertices[ ( *ioPointStart + n ) * 3     ] = mesh->pointL[ n ].pos[ 0 ];
				_vertices[ ( *ioPointStart + n ) * 3 + 1 ] = mesh->pointL[ n ].pos[ 1 ];
				_vertices[ ( *ioPointStart + n ) * 3 + 2 ] = mesh->pointL[ n ].pos[ 2 ];

				float tx = 0, ty = 0;

				if( mesh->texelL ) {
					tx = mesh->texelL[ n ][ 0 ];
					ty = 1.0 - mesh->texelL[ n ][ 1 ];
				}

				_texcoords[ ( *ioPointStart + n ) * 2     ] = tx;
				_texcoords[ ( *ioPointStart + n ) * 2 + 1 ] = ty;

				slVector v = { mesh->pointL[ n ].pos[ 0 ], mesh->pointL[ n ].pos[ 1 ], mesh->pointL[ n ].pos[ 2 ] };

				if( slVectorLength( &v ) > _maxReach )
					_maxReach = slVectorLength( &v );
			}

			Lib3dsVector *normals = new Lib3dsVector[ _indexCount ];
			lib3ds_mesh_calculate_normals( mesh, normals );

			for( int n = 0; n < mesh -> faces; n++ ) {
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

	printf( "Found material: %s\n", inMaterialName.c_str() );
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

			printf( "Node name = %s [ %s ]\n", inNode -> name, inNode -> data.object.morph  );
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
	glDisable( GL_COLOR_MATERIAL );

	slMaterial *material = NULL;

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

			glColor4fv( material -> _ambient );
			
			// I CAN HAZ TEXTURING?

			if( !material -> _texture && material -> _texturePath.size() > 0 )
				material -> _texture = new slTexture2D( material -> _texturePath );

			if( material -> _texture ) {
			 	material -> _texture -> bind();
				glMatrixMode( GL_TEXTURE );
				glLoadIdentity();
				glScalef( material -> _texture -> _unitX, material -> _texture -> _unitY, 1 );
			}
			else 
				glDisable( GL_TEXTURE_2D );

			glTexEnvf( GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE  );

		}

		int i1 = _indices[ n ];
		int i2 = _indices[ n + 1 ];
		int i3 = _indices[ n + 2 ];

		if( 1 || material -> _texture ) {
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
	 }

	// glDrawElements( GL_TRIANGLES, _indexCount, GL_UNSIGNED_INT _indices );

	glEnable( GL_COLOR_MATERIAL );


	glMatrixMode( GL_MODELVIEW );
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
