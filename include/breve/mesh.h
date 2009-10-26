
#include <map>

#include "config.h"
#include "shape.h"
#include "texture.h"
#include "render.h"

/**
 * A submesh found inside of an \ref slMeshShape.
 */
class slMesh {
	friend class slAIImport;
	friend class slMeshNode;
	
	public:
									slMesh() { _material = NULL; }
	
	protected:
		slVertexBufferGL			_vertexBuffer;
		slIndexBufferGL				_indexBuffer;
		slMaterial*					_material;
};

/**
 * A mesh hierarchy node.
 */
class slMeshNode {
	friend class slAIImport;

	public:
	
		void 						draw( const slRenderGL& inRenderer );
	
	protected:
		std::vector< slMeshNode* >	_children;
		std::vector< slMesh* >		_meshes;
		float						_transform[ 16 ];
};

class slMeshShape : public slShape {
	friend class slAIImport;
	friend class slAIGeometryBuilder;

	public:
		slMeshShape();
		~slMeshShape();
		
		void 						bounds( const slPosition *position, slVector *min, slVector *max ) const;
		virtual void 				draw( const slRenderGL& inRender );

		virtual void				finishShape( double inDensity );
		virtual void				updateLastPosition( slPosition *inPosition );

	protected:
		int							createODEGeom();
	
		float*						_vertices;

		int*						_indices;
		
		slMeshNode*					_rootNode;

		std::vector< slMesh* >		_meshes;
		std::vector< slMaterial* >	_materialList;

		int 						_vertexCount;
		int 						_indexCount;

		dMatrix4                    _lastPositions[ 2 ];
		int                         _lastPositionIndex;

		float						_maxReach;
};

#ifdef HAVE_LIBASSIMP

#include <assimp/assimp.hpp>
#include <assimp/aiScene.h>
#include <assimp/aiPostProcess.h> 

class slAIImport {
	public:
		static slMeshShape*			Import( char *inFilename );		
		static slMaterial*			ImportMaterial( const aiMaterial *inMaterial, slMeshShape *inMeshShape, const char *inBaseDir );
		static slMesh*				ImportMesh( const aiMesh *inMesh, slMeshShape *inMeshShape );
		static slMeshNode* 			BuildNodeHierarchy( const aiNode* inNode, slMeshShape *inMeshShape );

};

/**
 * A helper class to build raw, fully processed vertex and index lists required 
 * for ODE trimesh geometries.
 */

class slAIGeometryBuilder {
	public:	
		void 						Build( const aiScene* inScene, const aiNode* inNode, aiMatrix4x4 *inTransformation = NULL );
		void 						SetShapeGeometry( slMeshShape *inMeshShape );
	
		std::vector< aiVector3D >	_vertices;
		std::vector< int >			_indices;
		
		aiVector3D					_center;
};

#endif // HAVE_LIBASSIMP
