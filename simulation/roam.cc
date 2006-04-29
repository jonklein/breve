#include "simulation.h"
#include "roam.h"
#include "camera.h"
#include "glIncludes.h"

#define MAX(x,y)    ((x)>(y)?(x):(y))


slRoamPatch::slRoamPatch(slTerrain *t) {
	_terrain = t;

	_triangleCount = 0;

	_baseLeft._baseNeighbor = &_baseRight;	
	_baseLeft._leftX = 0;
	_baseLeft._leftY = 0;
	_baseLeft._rightX = t->_side - 1;
	_baseLeft._rightY = t->_side - 1;
	_baseLeft._apexX = 0;
	_baseLeft._apexY = t->_side - 1;

	_baseRight._baseNeighbor = &_baseLeft;	
	_baseRight._leftX = t->_side - 1;
	_baseRight._leftY = t->_side - 1;
	_baseRight._rightX = 0;
	_baseRight._rightY = 0;
	_baseRight._apexX = t->_side - 1;
	_baseRight._apexY = 0;

	_frameVariance = 10.0;

	computeVariance();
}

/*!
	\brief Forces the detail to be set to the specified level, starting with
	the root.
*/

void slRoamPatch::setDetailLevel(int level) {
	setDetailLevel( &_baseRight, level - 1);
	setDetailLevel( &_baseLeft, level - 1);
}

/*!
	\brief A recursive method used in setting the detail level.
*/

void slRoamPatch::setDetailLevel(slRoamTriangle *triangle, int level) {
	if(level == 0) return;

	split( triangle);

	setDetailLevel(triangle->_leftChild, level - 1);
	setDetailLevel(triangle->_rightChild, level - 1);
}

/*!
	\brief Splits a triangle in two.
*/

void slRoamPatch::split(slRoamTriangle *triangle) {
	int centerX, centerY;

	if ( triangle->_leftChild ) return;

	if ( triangle->_baseNeighbor && triangle->_baseNeighbor->_baseNeighbor != triangle ) {
		split( triangle->_baseNeighbor);
	}

	// can this triangle not be split any further?
	if( abs(triangle->_leftX - triangle->_rightX) == 1 ) return; 

	centerX = (triangle->_leftX + triangle->_rightX) >> 1;
	centerY = (triangle->_leftY + triangle->_rightY) >> 1;

	triangle->_leftChild = nextTriangle();
	triangle->_rightChild = nextTriangle();

	if( !triangle->_leftChild || ! triangle->_rightChild) {
		triangle->_leftChild = triangle->_rightChild = NULL;
		return;
	}

	triangle->_leftChild->_leftX = triangle->_apexX;
	triangle->_leftChild->_leftY = triangle->_apexY;
	triangle->_leftChild->_rightX = triangle->_leftX;
	triangle->_leftChild->_rightY = triangle->_leftY;
	triangle->_leftChild->_apexX = centerX;
	triangle->_leftChild->_apexY = centerY;

	triangle->_rightChild->_leftX = triangle->_rightX;
	triangle->_rightChild->_leftY = triangle->_rightY;
	triangle->_rightChild->_rightX = triangle->_apexX;
	triangle->_rightChild->_rightY = triangle->_apexY;
	triangle->_rightChild->_apexX = centerX;
	triangle->_rightChild->_apexY = centerY;

	triangle->_rightChild->_rightNeighbor = triangle->_leftChild;
	triangle->_rightChild->_baseNeighbor = triangle->_rightNeighbor;

	triangle->_leftChild->_leftNeighbor = triangle->_rightChild;
	triangle->_leftChild->_baseNeighbor = triangle->_leftNeighbor;

	if(triangle->_leftNeighbor) {
		if(triangle->_leftNeighbor->_baseNeighbor == triangle) {
			triangle->_leftNeighbor->_baseNeighbor = triangle->_leftChild;
		} else if(triangle->_leftNeighbor->_leftNeighbor == triangle) {
			triangle->_leftNeighbor->_leftNeighbor = triangle->_leftChild;
		} else {
			triangle->_leftNeighbor->_rightNeighbor = triangle->_leftChild;
		}
	}

	if(triangle->_rightNeighbor) {
		if(triangle->_rightNeighbor->_baseNeighbor == triangle) {
			triangle->_rightNeighbor->_baseNeighbor = triangle->_rightChild;
		} else if(triangle->_rightNeighbor->_leftNeighbor == triangle) {
			triangle->_rightNeighbor->_leftNeighbor = triangle->_rightChild;
		} else {
			triangle->_rightNeighbor->_rightNeighbor = triangle->_rightChild;
		}
	}

	if(triangle->_baseNeighbor) {
		if (triangle->_baseNeighbor->_leftChild ) {	
			triangle->_baseNeighbor->_leftChild->_rightNeighbor = triangle->_rightChild;	
			triangle->_baseNeighbor->_rightChild->_leftNeighbor = triangle->_leftChild;	

			triangle->_leftChild->_rightNeighbor = triangle->_baseNeighbor->_rightChild;
			triangle->_rightChild->_leftNeighbor = triangle->_baseNeighbor->_leftChild;
		} else {
			split(triangle->_baseNeighbor);
		}
	} else {
		triangle->_leftChild->_rightNeighbor = NULL;
		triangle->_rightChild->_leftNeighbor = NULL;
	}

	makePoints(triangle->_leftChild);
	makePoints(triangle->_rightChild);
}

/*
*/

slRoamTriangle *slRoamPatch::nextTriangle() {
	slRoamTriangle *t;

	if(_triangleCount >= TRIANGLE_COUNT) return NULL;

	t = &_triangles[ _triangleCount++ ];

	t->reset();

	return t;
}

double slRoamPatch::computeVariance() {
	int size = _terrain->_side - 1;

	_currentVariance = _varianceLeft;
	computeVariance(0, 0, _terrain->_matrix[0][0],
					size, size, _terrain->_matrix[size][size],
					0, size, _terrain->_matrix[0][size], 1);

	_currentVariance = _varianceRight;
	computeVariance(size, size, _terrain->_matrix[size][size],
					0, 0, _terrain->_matrix[0][0],
					size, 0, _terrain->_matrix[size][0], 1);

	return 0;
}

/*!
	\brief Recursively computes variance values.
*/

double slRoamPatch::computeVariance(int lx, int lz, float lh, 
									int rx, int rz, float rh,
									int ax, int az, float ah, int node) {
	float variance;

	int cx = (lx + rx) >> 1;
	int cz = (lz + rz) >> 1;

	float ch = _terrain->_matrix[cx][cz];

	variance = fabs( ch - (lh + rh) / 2.0 );

	if(abs(lx - rx) >= 8 || abs(lz - rz) >= 8) {
		double c; 

		c = computeVariance( ax, az, ah, lx, lz, lh, cx, cz, ch, node<<1);
		variance = MAX( variance, c);

		c = computeVariance( rx, rz, rh, ax, az, ah, cx, cz, ch, (node<<1) + 1);
		variance = MAX( variance, c);
	}

	if (node < (1<<VARIANCE_DEPTH)) _currentVariance[node] = 1.0 + variance;

	return variance;
}

/*!
	\brief Tessellates the terrain by recursing down the triangle structure.
*/

void slRoamPatch::tessellate(slCamera *c) {
	double delta;

	_triangleCount = 0;

	if( abs( _terrain->_polygonsDrawn - _terrain->_desiredPolygons ) > 100 ) {
		delta = (_terrain->_polygonsDrawn - _terrain->_desiredPolygons) / (float)_terrain->_desiredPolygons;
		_frameVariance += delta;

		// printf("delta = %f, var = %f\n", delta, _frameVariance);

		if(_frameVariance < 0.0) _frameVariance = 0.0;
	}

	makePoints( &_baseLeft);
	makePoints( &_baseRight);

	_baseLeft._leftChild = _baseLeft._rightChild = NULL;
	_baseLeft._leftNeighbor = _baseLeft._rightNeighbor = NULL;
	_baseLeft._baseNeighbor = &_baseRight;
	_baseLeft._clipped = false;

	_baseRight._leftChild = _baseRight._rightChild = NULL;
	_baseRight._leftNeighbor = _baseRight._rightNeighbor = NULL;
	_baseRight._baseNeighbor = &_baseLeft;
	_baseRight._clipped = false;

	_currentVariance = _varianceLeft;
	tessellate( c, &_baseLeft, 1);

	_currentVariance = _varianceRight;
	tessellate( c, &_baseRight, 1);
}

void slRoamPatch::makePoints(slRoamTriangle *t) {
	slVector tmp, tmp2;

	t->_points[0].x = t->_leftX * _terrain->_xscale + _terrain->_position.location.x;
	t->_points[0].y = _terrain->_matrix[t->_leftX][t->_leftY] + _terrain->_position.location.y;
	t->_points[0].z = t->_leftY * _terrain->_xscale + _terrain->_position.location.z;

	t->_points[1].x = t->_apexX * _terrain->_xscale + _terrain->_position.location.x;
	t->_points[1].y = _terrain->_matrix[t->_apexX][t->_apexY] + _terrain->_position.location.y;
	t->_points[1].z = t->_apexY * _terrain->_xscale + _terrain->_position.location.z;

	t->_points[2].x = t->_rightX * _terrain->_xscale + _terrain->_position.location.x;
	t->_points[2].y = _terrain->_matrix[t->_rightX][t->_rightY] + _terrain->_position.location.y;
	t->_points[2].z = t->_rightY * _terrain->_xscale + _terrain->_position.location.z;

	slVectorSub(&t->_points[2], &t->_points[1], &tmp);
	slVectorSub(&t->_points[2], &t->_points[0], &tmp2);

	slVectorCross(&tmp2, &tmp, &t->_normal);

	slVectorNormalize(&t->_normal);
}

/*
	\brief Recursively tessellates the terrain.
*/

void slRoamPatch::tessellate(slCamera *c, slRoamTriangle *t, int node) {
	float variance = 0;
	double dot, distance = 1000;
	slVector loc, toPoly;

	if( !c->polygonInFrustum(t->_points, 3) ) {
		t->_clipped = true;
		return;
	}

	slVectorAdd(&c->_location, &c->_target, &loc);

	slVectorSub(&t->_points[1], &loc, &toPoly);

	slVectorNormalize(&toPoly);

	dot = slVectorDot(&t->_normal, &toPoly);

	if (dot > 0.5) {
		t->_clipped = true;
	 	// return;
	}
	
	if( node < (1 << VARIANCE_DEPTH) ) {
		slVector dist, center;

		center.x = (t->_points[0].x + t->_points[1].x) / 2;
		center.y = (t->_points[0].y + t->_points[1].y) / 2;
		center.z = (t->_points[0].z + t->_points[1].z) / 2;

		slVectorSub(&center, &loc, &dist);
		distance = 1.0 + slVectorLength(&dist);

		variance = (_currentVariance[node] * _terrain->_xscale * (_terrain->_side - 1)) / distance;	
	} 

	if( node >= (1 << VARIANCE_DEPTH) || variance > _frameVariance) {
		split(t);

		if(t->_leftChild) {
			tessellate( c, t->_leftChild, node<<1);
			tessellate( c, t->_rightChild, (node<<1) + 1);
		}
	} 

	if (dot > 0.0) t->_clipped = true;
}

/*!
	\brief Recursively renders the terrain starting at the root.
*/

int slRoamPatch::render( slCamera *c, int mode) {
	int n;

	glDisable(GL_CULL_FACE);
	//glDisable(GL_LIGHTING);

	if( mode) {
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
		glDisable(GL_BLEND);
		glColor4f(1, 1, 1, 1.0);
		glLineWidth(0.8);
	} else {
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
		glEnable(GL_BLEND);
		glColor4f(0, 0, 0, 0.5);
		glLineWidth(0.8);
	}

	glShadeModel(GL_SMOOTH);

	glBegin(GL_TRIANGLES);

	n = render( &_baseLeft, c) + render( &_baseRight, c);

	glEnd();

	if( !mode ) glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

	return n;
}

/*!
	\brief Recursively renders the terrain starting at the 
	provided \ref slRoamTriangle.

	Note on the coordinate naming scheme: "up" in breve is considered the 
	y-axis.  But in dealing with a 2D heightmap, we refer to the X/Y plane.
	So the height is always drawn on the Y-axis, even though it leads to 
	confusion below in which the Y-heighmap coordinate is used to generate
	the actual Z-coordinate.
*/

int slRoamPatch::render( slRoamTriangle *triangle, slCamera *c) {
	double color;

	if( triangle->_leftChild )
		return render(triangle->_leftChild, c) + render(triangle->_rightChild, c);

	color = (triangle->_points[0].y) / 100;

	glColor3f(1.0 - color, 1.0, 1.0 - color);

	// if( triangle->_clipped) return 0;

	glTexCoord3f(triangle->_leftX / _terrain->_textureScaleX,  triangle->_leftY / _terrain->_textureScaleY, 0);
	glVertex3f(triangle->_points[0].x, triangle->_points[0].y, triangle->_points[0].z);

	glTexCoord3f(triangle->_apexX / _terrain->_textureScaleX, triangle->_apexY / _terrain->_textureScaleY, 0);
	glVertex3f(triangle->_points[1].x, triangle->_points[1].y, triangle->_points[1].z);

	glTexCoord3f(triangle->_rightX / _terrain->_textureScaleX, triangle->_rightY / _terrain->_textureScaleY, 0);
	glVertex3f(triangle->_points[2].x, triangle->_points[2].y, triangle->_points[2].z);

	return 1;
}
