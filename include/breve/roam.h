#ifndef _ROAM_H
#define _ROAM_H

#define VARIANCE_DEPTH 21 

#define TRIANGLE_COUNT 50000

/*!
	\brief A class used for recursive subdivisions of a \ref slTerrain.
*/

#ifdef __cplusplus

class slRoamTriangle {
	public:
		slRoamTriangle() { 
			reset();
		}

		void reset() {
			_leftChild = NULL;
			_rightChild = NULL;
			_leftNeighbor = NULL;
			_rightNeighbor = NULL;
			_baseNeighbor = NULL;

			_clipped = false;
		};

		slRoamTriangle *_leftChild;	
		slRoamTriangle *_rightChild;

		slRoamTriangle *_leftNeighbor;	
		slRoamTriangle *_rightNeighbor;
		slRoamTriangle *_baseNeighbor;

		int _leftX, _leftY;
		int _rightX, _rightY;
		int _apexX, _apexY;

		slVector _points[3];
		slVector _normal;

		bool _clipped;
};

/*!
	\brief A class used to manage a hierarchy of \brief slRoamTriangle objects.
*/

class slRoamPatch {
	public:
		slRoamPatch(slTerrain *t);

		void reset();

		int render( slCamera *c, int mode);

		int render( slRoamTriangle *triangle, slCamera *c );
		void split( slRoamTriangle *triangle);
		void join( slRoamTriangle *triangle);

		void setDetailLevel(slRoamTriangle *triangle, int level);
		void setDetailLevel(int level);

		void makePoints(slRoamTriangle *t);

		void tessellate(slCamera *c);
		void tessellate(slCamera *c, slRoamTriangle *t, int node);

		double computeVariance();

		double computeVariance(int lx, int lz, float lh,
								int rx, int rz, float rh,
								int ax, int ay, float ah, int node);

		slRoamTriangle *nextTriangle();

		slRoamTriangle _triangles[ 50000];
		int _triangleCount;

		slTerrain *_terrain;

		float *_height;

		float _varianceLeft[1 << VARIANCE_DEPTH];
		float _varianceRight[1 << VARIANCE_DEPTH];

		float *_currentVariance;

		slRoamTriangle _baseLeft;
		slRoamTriangle _baseRight;

		double _frameVariance;
};

#endif /* __cplusplus */

#endif /* _ROAM_H */
