
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include <string>
#include <map>

#include "simulation.h"
#include "mesh.h"
#include "gldraw.h"

slMeshShape::slMeshShape() : slShape() {
	_vertices = NULL;
	_indices  = NULL;

	_indexCount  = 0;
	_vertexCount = 0;

	_lastPositionIndex = 0;

	_maxReach = 0.0;

	setDensity( 1.0 );
}

int slMeshShape::createODEGeom() {
	if( !_indices || !_vertices ) 
		return -1;

	slVectorSet( &_max, 0, 0, 0 );
	slVectorSet( &_min, 0, 0, 0 );

	for( int n = 0; n < _vertexCount; n++ ) {
		float x = _vertices[ n * 3 ];
		float y = _vertices[ n * 3 + 1 ];
		float z = _vertices[ n * 3 + 2 ];

		if( x > _max.x )
			_max.x = x;
		
		if( x < _min.x )
			_min.x = x;

		if( y > _max.y )
			_max.y = y;
		
		if( y < _min.y )
			_min.y = y;

		if( z > _max.z )
			_max.z = z;
		
		if( z < _min.z )
			_min.z = z;
	}

	if( _odeGeomID[ 0 ] )
		dGeomDestroy( _odeGeomID[ 0 ] );
	if( _odeGeomID[ 1 ] ) 
		dGeomDestroy( _odeGeomID[ 1 ] );

	dTriMeshDataID triMeshID1 = dGeomTriMeshDataCreate();
	dTriMeshDataID triMeshID2 = dGeomTriMeshDataCreate();
	
	dGeomTriMeshDataBuildSingle( triMeshID1, 
		_vertices, 3 * sizeof( float ), _vertexCount,
		_indices, _indexCount, 3 * sizeof( int ) );

	dGeomTriMeshDataBuildSingle( triMeshID2, 
		_vertices, 3 * sizeof( float ), _vertexCount,
		_indices, _indexCount, 3 * sizeof( int ) );

  	_odeGeomID[ 0 ] = dCreateTriMesh( 0, triMeshID1, 0, 0, 0);
  	_odeGeomID[ 1 ] = dCreateTriMesh( 0, triMeshID2, 0, 0, 0);

	dGeomSetData( _odeGeomID[ 0 ], triMeshID1 );
	dGeomSetData( _odeGeomID[ 1 ], triMeshID2 );

	dMass m;
	dMassSetTrimesh( &m, _density, _odeGeomID[ 0 ] );

    _inertia[ 0 ][ 0 ] = m.I[ 0 ];
    _inertia[ 0 ][ 1 ] = m.I[ 1 ];
    _inertia[ 0 ][ 2 ] = m.I[ 2 ];

    _inertia[ 1 ][ 0 ] = m.I[ 4 ];
    _inertia[ 1 ][ 1 ] = m.I[ 5 ];
    _inertia[ 1 ][ 2 ] = m.I[ 6 ];

    _inertia[ 2 ][ 0 ] = m.I[ 8 ];
    _inertia[ 2 ][ 1 ] = m.I[ 9 ];
    _inertia[ 2 ][ 2 ] = m.I[ 10 ];

	_mass = m.mass;

	return 0;
}

void slMeshShape::bounds( const slPosition *position, slVector *outMin, slVector *outMax ) const {
	float scale = MAX( _transform[ 0 ][ 0 ], MAX( _transform[ 1 ][ 1 ], _transform[ 2 ][ 2 ] ) );
	float reach = _maxReach * scale;

	outMax->x = position->location.x + reach;
	outMax->y = position->location.y + reach;
	outMax->z = position->location.z + reach;

	outMin->x = position->location.x - reach;
	outMin->y = position->location.y - reach;
	outMin->z = position->location.z - reach;
}

slMeshShape::~slMeshShape() {
	if( _vertices )
		delete[] _vertices;

	if( _indices )
		delete[] _indices;
}

void slMeshShape::updateLastPosition( slPosition *inPosition ) {
	dMatrix4 transform;

	slMatrixToODEMatrix( inPosition -> rotation, &transform[ 0 ] );

	transform[ 12 ] = inPosition -> location.x;
	transform[ 13 ] = inPosition -> location.y;
	transform[ 14 ] = inPosition -> location.z;
	transform[ 15 ] = 1.0;

	memcpy( _lastPositions[ _lastPositionIndex ], transform, sizeof( dMatrix4 ) );

	dGeomTriMeshSetLastTransform( _odeGeomID[ 0 ], _lastPositions[ _lastPositionIndex ] );
	dGeomTriMeshSetLastTransform( _odeGeomID[ 1 ], _lastPositions[ _lastPositionIndex ] );

	_lastPositionIndex = !_lastPositionIndex;
}

struct slTriangleIndex {
	int _i1, _i2, _i3;
};

struct slTriangleBreakdownData {
	std::vector< slVector > 			_vertices;
	std::vector< slTriangleIndex > 		_indices;

	void 								breakdown( int inA, int inB, int inStart );
};

#define TRYSPLIT( I1, I2 ) \
	if( ( _indices[ n ].I1 == inA && _indices[ n ].I2 == inB ) || ( _indices[ n ].I1 == inB && _indices[ n ].I2 == inA ) ) {	\
		newtri.I2 = newindex;					\
		_indices[ n ].I1 = newindex;			\
		_indices.push_back( newtri );			\
		count++; \
	} 


void slTriangleBreakdownData::breakdown( int inA, int inB, int inStart ) {
	slVector newpt;
	slVectorAdd( &_vertices[ inA ], &_vertices[ inB ], &newpt );
	slVectorMul( &newpt, .5, &newpt );
	_vertices.push_back( newpt );
	int newindex = _vertices.size() - 1;
	int count = 0;

	for( unsigned int n = inStart; n < _indices.size(); n++ ) {
		slTriangleIndex newtri = _indices[ n ];

		TRYSPLIT( _i1, _i2 )
		TRYSPLIT( _i1, _i3 )
		TRYSPLIT( _i2, _i3 )
	}
}

void slMeshShape::finishShapeWithMaxLength( double inDensity, float inMaxSize ) {
	slTriangleBreakdownData d;

	for( int i = 0; i < _vertexCount * 3; i += 3 ) { 
		slVector v;

		slVectorSet( &v, _vertices[ i ], _vertices[ i + 1 ], _vertices[ i + 2 ] );
		d._vertices.push_back( v );
	}

	for( int i = 0; i < _indexCount; i += 3 ) { 
		slTriangleIndex t;

		t._i1 = _indices[ i ];
		t._i2 = _indices[ i + 1 ];
		t._i3 = _indices[ i + 2 ];

		d._indices.push_back( t );
	}

	int breakdowns = 0;

	do {
		breakdowns = 0;
		
		for( unsigned int n = 0; n < d._indices.size(); n++ ) {
			slTriangleIndex *triangle = &d._indices[ n ];
			int maxIndex = -1;
			float size, maxSize = inMaxSize;

			if( ( size = slVectorDist( &d._vertices[ triangle -> _i1 ], &d._vertices[ triangle -> _i2 ] ) ) > maxSize ) {
				maxSize = size;
				maxIndex = 0;
			}

			if( ( size = slVectorDist( &d._vertices[ triangle -> _i2 ], &d._vertices[ triangle -> _i3 ] ) ) > maxSize ) {
				maxSize = size;
				maxIndex = 1;
			}

			if( ( size = slVectorDist( &d._vertices[ triangle -> _i1 ], &d._vertices[ triangle -> _i3 ] ) ) > maxSize ) {
				maxSize = size;
				maxIndex = 2;
			}

			switch( maxIndex )  {
				case 0:
					breakdowns++;
					d.breakdown( triangle -> _i1, triangle -> _i2, n );
					break;
				case 1:
					breakdowns++;
					d.breakdown( triangle -> _i2, triangle -> _i3, n );
					break;
				case 2:
					breakdowns++;
					d.breakdown( triangle -> _i1, triangle -> _i3, n );
					break;
			}
		}

	} while( breakdowns > 0 );

	_vertexCount = d._vertices.size();	
	_indexCount = d._indices.size() * 3;
	delete[] _vertices;
	delete[] _indices;

	_vertices = new float[ _vertexCount * 3 ];
	_indices  = new int[ _indexCount ];

	for( int n = 0; n < _vertexCount; n++ ) {
		_vertices[ n * 3     ] = d._vertices[ n ].x;
		_vertices[ n * 3 + 1 ] = d._vertices[ n ].y;
		_vertices[ n * 3 + 2 ] = d._vertices[ n ].z;
	}

	for( unsigned int i = 0; i < d._indices.size(); i++ ) {
		_indices[ i * 3     ] = d._indices[ i ]._i1;
		_indices[ i * 3 + 1 ] = d._indices[ i ]._i2;
		_indices[ i * 3 + 2 ] = d._indices[ i ]._i3;
	}
}

void slMeshShape::finishShape( double density ) {
	int triCount = 0;

	_maxReach = 0.0;

	for( unsigned int n = 0; n < faces.size(); n++ )
		triCount += faces[ n ] -> _pointCount;

	_vertexCount = points.size() + faces.size();
	_indexCount = 3 * triCount;

	_vertices = new float[ _vertexCount * 3 ];
	_indices = new int[ _indexCount ];

	for( unsigned int n = 0; n < points.size(); n++ ) {
		slPoint *p = points[ n ];

		_vertices[ n * 3     ] = p -> vertex.x;
		_vertices[ n * 3 + 1 ] = p -> vertex.y;
		_vertices[ n * 3 + 2 ] = p -> vertex.z;

		if( slVectorLength( &p -> vertex ) > _maxReach )
			_maxReach = slVectorLength( &p -> vertex );
	}

	for( unsigned int n = 0; n < faces.size(); n++ ) {
		slFace *f = faces[ n ];
		slVector total;

		slVectorSet( &total, 0, 0, 0 );

		for( int m = 0; m < f -> _pointCount; m++ )
			slVectorAdd( &total, &f -> points[ m ] -> vertex, &total );

		slVectorMul( &total, 1.0 / f -> _pointCount, &total );

		int vstart = ( points.size() * 3 ) + n * 3;

		_vertices[ vstart     ] = total.x;
		_vertices[ vstart + 1 ] = total.y;
		_vertices[ vstart + 2 ] = total.z;
	}

	int tri = 0;

	for( unsigned int n = 0; n < faces.size(); n++ ) {
		slFace *f = faces[ n ];	

		int i1 = points.size() + n;

		for( int m = 0; m < f -> _pointCount; m++ ) {
			int mplus = ( m + 1 )  % f -> _pointCount;

			int i2 = findPointIndex( &f -> points[ m     ] -> vertex );
			int i3 = findPointIndex( &f -> points[ mplus ] -> vertex );

			_indices[ tri * 3     ] = i1;
			_indices[ tri * 3 + 1 ] = i2;
			_indices[ tri * 3 + 2 ] = i3;

			tri++;
		}
	}

	_density = density;
	createODEGeom();

	slShape::finishShape( density );
}

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

	_directory = slDirname( inFilename );
	
	_vertexCount = 0;
	_indexCount = 0;

	calculateSizes( file, file -> nodes, &_vertexCount, &_indexCount );

	_vertices = new float[ 3 * _vertexCount ];
	_texcoords = new float[ 2 * _vertexCount ];
	_indices = new int[ _indexCount ];
	_normals = new float[ _indexCount * 3 ];
	_materials = new int[ _indexCount ];

	memset( _materials, 0, sizeof( int ) * _indexCount );
	memset( _normals,   0, sizeof( float ) * _indexCount * 3 );

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

	material._ambient[ 3 ]  = 1.0 - inMaterial -> transparency;
	material._diffuse[ 3 ]  = 1.0 - inMaterial -> transparency;
	material._specular[ 3 ] = 1.0 - inMaterial -> transparency;
	material._twosided = inMaterial -> two_sided;

	// printf( "material transparency: %f\n", inMaterial -> transparency );

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

void sl3DSShape::draw( const slRenderGL& inRenderer ) {
	for( unsigned int n = 0; n < _materialList.size(); n++ ) {
		slMaterial *material = &_materialList[ n ];

		if( !material -> _texture && material -> _texturePath.size() > 0 )
			material -> _texture = new slTexture2D( &material -> _texturePath );
	}


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

			glColor4fv( material -> _diffuse );

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

#endif
