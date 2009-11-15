
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include <string>
#include <map>

#include "simulation.h"
#include "mesh.h"

slMeshShape::slMeshShape() : slShape() {
	_rootNode = NULL;

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

	slVectorSet( &_max, -FLT_MAX, -FLT_MAX, -FLT_MAX );
	slVectorSet( &_min,  FLT_MAX,  FLT_MAX,  FLT_MAX );

	for( int n = 0; n < _vertexCount; n++ ) {
		float x = _vertices[ n * 3     ];
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

	_maxReach = slMAX( slVectorLength( &_max ), slVectorLength( &_min ) );
	
	// We "double buffer" the ODE geoms here because ODE (GIMPACT?) does not
	// seem to correctly handle a trimesh collision test with itself

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
	float reach = _maxReach;

	outMax -> x = position->location.x + reach;
	outMax -> y = position->location.y + reach;
	outMax -> z = position->location.z + reach;

	outMin -> x = position->location.x - reach;
	outMin -> y = position->location.y - reach;
	outMin -> z = position->location.z - reach;
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

void slMeshShape::draw( const slRenderGL& inRenderer ) {
#ifdef HAVE_LIBASSIMP
	if( _rootNode ) {
		_rootNode -> draw( inRenderer );
	}
	else 
#endif
		slShape::draw( inRenderer );
}

#ifdef HAVE_LIBASSIMP

#include <assimp/assimp.hpp>
#include <assimp/aiScene.h>
#include <assimp/aiPostProcess.h> 

void slMeshNode::draw( const slRenderGL& inRenderer ) {
	inRenderer.PushMatrix( slMatrixGeometry );
	inRenderer.MulMatrix4( slMatrixGeometry, _transform );

	for( unsigned int n = 0; n < _meshes.size(); n++ ) {
		slMesh *mesh = _meshes[ n ];
		
		if( mesh -> _material )
			inRenderer.BindMaterial( *mesh -> _material );

		mesh -> _vertexBuffer.bind();
		mesh -> _vertexBuffer.draw( mesh -> _indexBuffer );
		mesh -> _vertexBuffer.unbind();

		if( mesh -> _material )
			inRenderer.UnbindMaterial( *mesh -> _material );
	
	}

	for( unsigned int n = 0; n < _children.size(); n++ )
		 _children[ n ] -> draw( inRenderer );
		 

	inRenderer.PopMatrix( slMatrixGeometry );
}

slMaterial *slAIImport::ImportMaterial( const aiMaterial *inMaterial, slMeshShape *inMeshShape, const char *inBaseDir ) {
	slMaterial *material = new slMaterial;
	
	aiString path;
	inMaterial -> GetTexture( aiTextureType_DIFFUSE, 0, &path, NULL, NULL, NULL, NULL, NULL );
	
	if( path.length > 0 ) {
		material -> _texturePath = std::string( inBaseDir ) + std::string( "/" ) + path.data;
		material -> _texture = new slTexture2D( &material -> _texturePath );
	} 
	
	aiColor4D color;

	inMaterial -> Get( AI_MATKEY_COLOR_SPECULAR, color );
	material -> _specular[ 0 ] = color.r;
	material -> _specular[ 1 ] = color.g;
	material -> _specular[ 2 ] = color.b;
	material -> _specular[ 3 ] = 1;

	inMaterial -> Get( AI_MATKEY_COLOR_AMBIENT, color );	
	material -> _ambient[ 0 ] = color.r;
	material -> _ambient[ 1 ] = color.g;
	material -> _ambient[ 2 ] = color.b;
	material -> _ambient[ 3 ] = 1;

	inMaterial -> Get( AI_MATKEY_COLOR_DIFFUSE, color );
	material -> _diffuse[ 0 ] = color.r;
	material -> _diffuse[ 1 ] = color.g;
	material -> _diffuse[ 2 ] = color.b;
	material -> _diffuse[ 3 ] = 1;
		
	inMeshShape -> _materialList.push_back( material );
	
	return material;
}

slMesh *slAIImport::ImportMesh( const aiMesh *inMesh, slMeshShape *inMeshShape ) {
	if( !inMesh -> HasFaces() ) 
		return NULL;
		
	slMesh *mesh = new slMesh;
	slVector center;

	int pixelFormat = VB_XYZ;
	
	if( inMesh -> HasNormals() ) 
		pixelFormat |= VB_NORMAL;	

	if( inMesh -> HasTextureCoords( 0 ) ) 
		pixelFormat |= VB_UV;
		
	mesh -> _vertexBuffer.resize( inMesh -> mNumVertices, pixelFormat );
						
	for( unsigned int i = 0; i < inMesh -> mNumVertices; i++ ) {
		const aiVector3D& vertex = inMesh -> mVertices[ i ];
		float *v = mesh -> _vertexBuffer.vertex( i );
		
		v[ 0 ] = vertex[ 0 ];
		v[ 1 ] = vertex[ 1 ];
		v[ 2 ] = vertex[ 2 ];
		
		center.x += vertex[ 0 ];
		center.y += vertex[ 1 ];
		center.z += vertex[ 1 ];

		if( pixelFormat & VB_NORMAL ) {
			const aiVector3D& normal = inMesh -> mNormals[ i ];
			float *n = mesh -> _vertexBuffer.normal( i );
			
			n[ 0 ] = normal[ 0 ];
			n[ 1 ] = normal[ 1 ];
			n[ 2 ] = normal[ 2 ];
		}
		
		if( pixelFormat & VB_UV ) {
			const aiVector3D& texcoord = inMesh -> mTextureCoords[ 0 ][ i ];
			float *t = mesh -> _vertexBuffer.texcoord( i );

			t[ 0 ] = texcoord[ 0 ];
			t[ 1 ] = texcoord[ 1 ];
		}
	}
	
	mesh -> _indexBuffer.resize( inMesh -> mNumFaces * 3 );
	unsigned short *indices = mesh -> _indexBuffer.indices();

	for( unsigned int i = 0; i < inMesh -> mNumFaces; i++ ) {
		aiFace &f = inMesh -> mFaces[ i ];
		
		indices[ i * 3 + 0 ] = f.mIndices[ 0 ];
		indices[ i * 3 + 1 ] = f.mIndices[ 1 ];
		indices[ i * 3 + 2 ] = f.mIndices[ 2 ];
	}
	
	mesh -> _material = inMeshShape -> _materialList[ inMesh -> mMaterialIndex ];
	
	slVectorMul( &center, 1.0 / inMesh -> mNumVertices, &center );
	// slMessage( DEBUG_ALL, "Center: %f, %f, %f\n", center.x, center.y, center.z );
	// slVectorCopy( &center, &mesh -> _center );
	
	inMeshShape -> _meshes.push_back( mesh );
	
	return mesh;
}

slMeshNode *slAIImport::BuildNodeHierarchy( const aiNode* inNode, slMeshShape *inMeshShape ) {
	slMeshNode *node = new slMeshNode;

	node -> _transform[ 0 ] = inNode -> mTransformation.a1;
	node -> _transform[ 1 ] = inNode -> mTransformation.a2;
	node -> _transform[ 2 ] = inNode -> mTransformation.a3;
	node -> _transform[ 3 ] = inNode -> mTransformation.a4;

	node -> _transform[ 4 ] = inNode -> mTransformation.b1;
	node -> _transform[ 5 ] = inNode -> mTransformation.b2;
	node -> _transform[ 6 ] = inNode -> mTransformation.b3;
	node -> _transform[ 7 ] = inNode -> mTransformation.b4;

	node -> _transform[ 8 ] = inNode -> mTransformation.c1;
	node -> _transform[ 9 ] = inNode -> mTransformation.c2;
	node -> _transform[ 10 ] = inNode -> mTransformation.c3;
	node -> _transform[ 11 ] = inNode -> mTransformation.c4;

	node -> _transform[ 12 ] = inNode -> mTransformation.d1;
	node -> _transform[ 13 ] = inNode -> mTransformation.d2;
	node -> _transform[ 14 ] = inNode -> mTransformation.d3;
	node -> _transform[ 15 ] = inNode -> mTransformation.d4;
	
	// slMessage( DEBUG_ALL, "Node %p has %d children and %d meshes\n", inNode, inNode -> mNumChildren, inNode -> mNumMeshes );
	
	for( unsigned int n = 0; n < inNode -> mNumChildren; n++ ) {
		slMeshNode *child = BuildNodeHierarchy( inNode -> mChildren[ n ], inMeshShape );
		
		node -> _children.push_back( child );
	}
	
	for( unsigned int m = 0; m < inNode -> mNumMeshes; m++ ) {
		unsigned int meshIndex = inNode -> mMeshes[ m ] ;
		// slMessage( DEBUG_ALL, "Mesh index: %d of %d\n", meshIndex, inMeshShape -> _meshes.size() );
		
		if( meshIndex < inMeshShape -> _meshes.size() )
			node -> _meshes.push_back( inMeshShape -> _meshes[ meshIndex ] );
	}
		
	return node;
}

void slAIGeometryBuilder::Build( const aiScene* inScene, const aiNode* inNode, aiMatrix4x4 *inTransformation  ) {

	aiMatrix4x4 transform;

	if( inTransformation )
		transform = *inTransformation * inNode -> mTransformation;
	else 
		transform = inNode -> mTransformation;
	
	for( unsigned int m = 0; m < inNode -> mNumMeshes; m++ ) {
		aiMesh *mesh = inScene -> mMeshes[ inNode -> mMeshes[ m ] ];
		int indexBase = _vertices.size();
		
		for( unsigned int v = 0; v < mesh -> mNumVertices; v++ ) {
			const aiVector3D& vertex = mesh -> mVertices[ v ];
			
			aiVector3D transformed = transform * vertex;	
			
			_vertices.push_back( transformed );
			_center.x += transformed.x;
			_center.y += transformed.y;
			_center.z += transformed.z;
		}
		

		for( unsigned int i = 0; i < mesh -> mNumFaces; i++ ) {
			aiFace &f = mesh -> mFaces[ i ];
			
			_indices.push_back( indexBase + f.mIndices[ 0 ] );
			_indices.push_back( indexBase + f.mIndices[ 1 ] );
			_indices.push_back( indexBase + f.mIndices[ 2 ] );
		}
	}

	for( unsigned int n = 0; n < inNode -> mNumChildren; n++ ) {
		Build( inScene, inNode -> mChildren[ n ], &transform );
	}	
}

void slAIGeometryBuilder::SetShapeGeometry( slMeshShape *inMeshShape ) {
	slVectorMul( &_center, 1.0 / _vertices.size(), &_center );

	inMeshShape -> _vertices = new float[ 3 * _vertices.size() ];
	inMeshShape -> _indices = new int[ _indices.size() ];
	
	inMeshShape -> _indexCount = _indices.size();
	inMeshShape -> _vertexCount = _vertices.size();

	for( unsigned int i = 0; i < _indices.size(); i++ ) {
		inMeshShape -> _indices[ i ] = _indices[ i ];
	}
	
	for( unsigned int i = 0; i < _vertices.size(); i++ ) {
		inMeshShape -> _vertices[ i * 3 + 0 ] = _vertices[ i ].x;
		inMeshShape -> _vertices[ i * 3 + 1 ] = _vertices[ i ].y;
		inMeshShape -> _vertices[ i * 3 + 2 ] = _vertices[ i ].z;
	}
	
	
}

slMeshShape *slAIImport::Import( char *inFilename ) {

	slMeshShape *shape = new slMeshShape();
	char *directory = slDirname( inFilename );

	Assimp::Importer importer;
	const aiScene* scene = importer.ReadFile( inFilename, aiProcess_CalcTangentSpace | aiProcess_Triangulate | aiProcess_JoinIdenticalVertices | aiProcess_SortByPType | aiProcess_GenUVCoords );

	slMessage( 0, "%s NUM Anim %d, Cam %d, Lig %d, Material %d, Mesh %d, Tex %d\n", inFilename,
		scene -> mNumAnimations,
		scene -> mNumCameras,
		scene -> mNumLights,
		scene -> mNumMaterials,
		scene -> mNumMeshes,
		scene -> mNumTextures
	);
	
	for( unsigned int n = 0; n < scene -> mNumMaterials; n++ ) {
		ImportMaterial( scene -> mMaterials[ n ], shape, directory );
	}

	for( unsigned int n = 0; n < scene -> mNumMeshes; n++ ) {
		ImportMesh( scene -> mMeshes[ n ], shape );
	}
	
	shape -> _rootNode = BuildNodeHierarchy( scene -> mRootNode, shape );
	
	slAIGeometryBuilder builder;
	builder.Build( scene, scene -> mRootNode );
	builder.SetShapeGeometry( shape );
	shape -> createODEGeom();
	
	return shape;
}

#endif
