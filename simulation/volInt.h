/*
   ============================================================================
   constants
   ============================================================================
*/

#define MAX_VERTS 200		/* maximum number of polyhedral vertices */
#define MAX_FACES 200		/* maximum number of polyhedral faces */
#define MAX_POLYGON_SZ 100	/* maximum number of verts per polygonal face */

/*
   ============================================================================
   macros
   ============================================================================
*/

#define SQR(x) ((x)*(x))
#define CUBE(x) ((x)*(x)*(x))

struct slMPFace {
  int numVerts;         
  double norm[3];
  double w;
  int verts[MAX_POLYGON_SZ];
  slMPPolyhedron *poly; 
} ;
        
struct slMPPolyhedron {
  int numVerts, numFaces;
  double verts[MAX_VERTS][3];
  slMPFace faces[MAX_FACES];
}; 

int slFindPointNumber(slShape *s, slPoint *p);
int slConvertShapeToPolyhedron(slShape *s, slMPPolyhedron *p);
int slSetMassProperties(slShape *s, double density);
