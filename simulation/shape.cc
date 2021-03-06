/*****************************************************************************
 *                                                                           *
 * The breve Simulation Environment                                          *
 * Copyright (C) 2000, 2001, 2002, 2003 Jonathan Klein                       *
 *                                                                           *
 * This program is free software; you can redistribute it and/or modify      *
 * it under the terms of the GNU General Public License as published by      *
 * the Free Software Foundation; either version 2 of the License, or         *
 * (at your option) any later version.                                       *
 *                                                                           *
 * This program is distributed in the hope that it will be useful,           *
 * but WITHOUT ANY WARRANTY; without even the implied warranty of            *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the             *
 * GNU General Public License for more details.                              *
 *                                                                           *
 * You should have received a copy of the GNU General Public License         *
 * along with this program; if not, write to the Free Software               *
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA *
 *****************************************************************************/

#include "simulation.h"
#include "mesh.h"
#include "camera.h"
#include "volInt.h"
#include "vclip.h"
#include "vclipData.h"


slVector *slPositionVertex( const slPosition *p, const slVector *v, slVector *o ) {
	slVectorXform( p->rotation, v, o );
	slVectorAdd( &p->location, o, o );

	return o;
}

void slShape::draw( const slRenderGL& inRender ) {
	if( _vertexBuffer.size() == 0 )
		fillVertexBuffer();

	_vertexBuffer.bind();
	_vertexBuffer.draw();
	_vertexBuffer.unbind();
}

void slShape::fillVertexBuffer() {

 	int polys = 0;
 	std::vector<slFace*>::iterator fi;

 	for ( fi = faces.begin(); fi != faces.end(); fi++ ) {
 		slFace *f = *fi;
 		polys += f -> _pointCount;
 	}

 	_vertexBuffer.resize( polys * 3, VB_XYZ | VB_UV | VB_NORMAL );

// a wrap-around point indexer
#define POINT_INDEX( f, n ) ( ( f ) -> points[ ( n ) < ( f ) -> _pointCount ? ( n ) : ( n ) - ( f ) -> _pointCount ] )

	int pointIndex = 0;

	for ( fi = faces.begin(); fi != faces.end(); fi++ ) {
		slFace *f = *fi;
		slVector center;


		slVectorSet( &center, 0, 0, 0 );

		for( int i = 0; i < f -> _pointCount; i++ ) {
			slPoint *p = f -> points[ i ];
			slVectorAdd( &p -> vertex, &center, &center );
		}

		slVectorMul( &center, 1.0 / f -> _pointCount, &center );

		slVector *norm = &f -> plane.normal;
		slVector xAxis, yAxis;
		slPerpendicularVectors( norm, &xAxis, &yAxis );

		for( int p = 0; p < f -> _pointCount; p++ ) {
			slPoint *p1 = POINT_INDEX( f, p     );
			slPoint *p2 = POINT_INDEX( f, p + 1 );

			float *f;
			f = _vertexBuffer.vertex( pointIndex     );
			f[ 0 ] = p1 -> vertex.x;
			f[ 1 ] = p1 -> vertex.y;
			f[ 2 ] = p1 -> vertex.z;
			f = _vertexBuffer.vertex( pointIndex + 1 );
			f[ 0 ] = p2 -> vertex.x;
			f[ 1 ] = p2 -> vertex.y;
			f[ 2 ] = p2 -> vertex.z;
			f = _vertexBuffer.vertex( pointIndex + 2 );
			f[ 0 ] = center.x;
			f[ 1 ] = center.y;
			f[ 2 ] = center.z;

			f = _vertexBuffer.texcoord( pointIndex     );
			f[ 0 ] = slVectorDot( &p1 -> vertex, &xAxis );
			f[ 1 ] = slVectorDot( &p1 -> vertex, &yAxis );

			f = _vertexBuffer.texcoord( pointIndex + 1 );
			f[ 0 ] = slVectorDot( &p2 -> vertex, &xAxis );
			f[ 1 ] = slVectorDot( &p2 -> vertex, &yAxis );

			f = _vertexBuffer.texcoord( pointIndex + 2 );
			f[ 0 ] = slVectorDot( &center, &xAxis );
			f[ 1 ] = slVectorDot( &center, &yAxis );
			
			f = _vertexBuffer.normal( pointIndex     );
			f[ 0 ] = norm -> x;
			f[ 1 ] = norm -> y;
			f[ 2 ] = norm -> z;

			f = _vertexBuffer.normal( pointIndex + 1 );
			f[ 0 ] = norm -> x;
			f[ 1 ] = norm -> y;
			f[ 2 ] = norm -> z;

			f = _vertexBuffer.normal( pointIndex + 2 );
			f[ 0 ] = norm -> x;
			f[ 1 ] = norm -> y;
			f[ 2 ] = norm -> z;

			pointIndex += 3;
		}
	}

	_vertexBuffer.bind();
	_vertexBuffer.draw( VB_TRIANGLES );
	_vertexBuffer.unbind();
}

void slShape::slMatrixToODEMatrix( const double inM[ 3 ][ 3 ], dReal *outM ) {
    outM[ 0 ]  = inM[ 0 ][ 0 ];
    outM[ 1 ]  = inM[ 0 ][ 1 ];
    outM[ 2 ]  = inM[ 0 ][ 2 ];
    outM[ 3 ]  = 0.0;
    outM[ 4 ]  = inM[ 1 ][ 0 ];
    outM[ 5 ]  = inM[ 1 ][ 1 ];
    outM[ 6 ]  = inM[ 1 ][ 2 ];
    outM[ 7 ]  = 0.0;
    outM[ 8 ]  = inM[ 2 ][ 0 ];
    outM[ 9 ]  = inM[ 2 ][ 1 ];
    outM[ 10 ] = inM[ 2 ][ 2 ];
    outM[ 11 ] = 0.0;
}


slSphere::slSphere( double radius, double density ) : slShape() {
	_type = ST_SPHERE;
	_radius = radius;

	_density = density;

	if ( radius <= 0.0 ) 
		throw slException( std::string( "invalid size for new sphere (density <= 0.0)" ) );

	if ( density <= 0.0 ) 
		throw slException( std::string( "invalid density for new sphere (radius <= 0.0)" ) );

	_mass = _density * M_PI * _radius * _radius * _radius * 4.0 / 3.0;

	_odeGeomID[ 0 ] = dCreateSphere( 0, _radius );
	_odeGeomID[ 1 ] = dCreateSphere( 0, _radius );

	setDensity( _density );
}

slBox::slBox( slVector *inSize, double inDensity ) : slShape() {
	if ( inDensity < 0.0 )
		throw slException( std::string( "invalid density for new cube (density <= 0.0)" ) );

	if ( inSize->x <= 0.0 || inSize->y <= 0.0 || inSize->z <= 0 )
		throw slException( std::string( "invalid size for new sphere (side <= 0.0)" ) );

	slVectorCopy( inSize, &_size ); 

	if ( !slSetCube( this, inSize, inDensity ) ) {
		 release();
		throw slException( std::string( "could not create a cube with the specified parameters" ) );
	}
}

void slBox::finishShape( double inDensity ) {
	if( _odeGeomID[ 0 ] ) 	
		dGeomDestroy( _odeGeomID[ 0 ] );

	if( _odeGeomID[ 1 ] ) 	
		dGeomDestroy( _odeGeomID[ 1 ] );

	float sx = _size.x * _scale.x;
	float sy = _size.y * _scale.y;
	float sz = _size.z * _scale.z;

	_odeGeomID[ 0 ] = dCreateBox( 0, sx, sy, sz );
	_odeGeomID[ 1 ] = dCreateBox( 0, sx, sy, sz );

	slShape::finishShape( inDensity );
}

slShape *slNewNGonDisc( int count, double radius, double height, double density ) {
	if ( radius <= 0.0 || height <= 0.0 || density <= 0.0 || count < 3 ) return NULL;

	slMeshShape *s = new slMeshShape();

	if ( !slSetNGonDisc( s, count, radius, height, density ) ) {
		s->release();
		return NULL;
	}

	return s;
}

slShape *slNewNGonCone( int count, double radius, double height, double density ) {
	if ( radius <= 0.0 || height <= 0.0 || density <= 0.0 || count < 3 ) return NULL;

	slMeshShape *s = new slMeshShape();

	if ( !slSetNGonCone( s, count, radius, height, density ) ) {
		s->release();
		return NULL;
	}

	return s;
}

void slShape::retain() {
	_referenceCount++;
}

void slShape::release() {
	if ( --_referenceCount ) 
		return;
		
	delete this;
}

slShape::~slShape() {
	std::vector< slFeature* >::iterator fi;

	for ( fi = features.begin() ; fi != features.end(); fi++ ) 
		delete *fi;

	if( _odeGeomID[ 0 ] ) 
		dGeomDestroy( _odeGeomID[ 0 ] );

	if( _odeGeomID[ 1 ] ) 
		dGeomDestroy( _odeGeomID[ 1 ] );
}


int slShape::findPointIndex( slVector *inVertex ) {
   for ( unsigned int n = 0; n < points.size(); n++ ) {
        if ( !slVectorCompare( inVertex, &points[ n ]->vertex ) ) 
			return n;
    }

	return -1;
}


void slShape::finishShape( double density ) {
	_density = density;

	if( slSetMassProperties( this, density ) ) 
		throw slException( "an error occurred computing the mass for this shape" );
}

/**
 * \brief Sets the mass of the shape.
 * 
 * Using the shape's volume, sets the density of the shape so that
 * the desired mass is achived.
 */
void slShape::setMass( double newMass ) {
	double volume;

	// the existing volume...

	volume = _mass / _density;

	// the desired density...

	_density = newMass / volume;

	setDensity( _density );
}

/**
 * \brief Sets the density of the shape.
 * 
 * Sets the density of the change, which modifies the mass.
 */

void slShape::setDensity( double newDensity ) {
	slSetMassProperties( this, newDensity );
}

/**
 * Sets the inertia matrix of the shape.
 */

void slShape::setInertiaMatrix( slMatrix inInertia ) {
	slMatrixCopy( inInertia, _inertia );
}

/**
 * \brief Sets the density of the sphere.
 * Sets the density of the change, which modifies the mass.
 */

void slSphere::setDensity( double newDensity ) {
	double volume = _mass / _density;

	_mass = newDensity * volume;
	_density = newDensity;

	slSphereInertiaMatrix( _radius, _mass, _inertia );
}

/*!
  \brief Adds a face to a shape.

	nPoints indicates how many points are on the given slFace.
	The last point given is assumed to connect to the first point.
*/

slFace *slAddFace( slShape *s, slVector **points, int nPoints ) {
	int n;
	slVector x, y, origin;
	slFace *f;
	slVector **rpoints = NULL;

	if ( nPoints < 3 ) 
		return NULL;

	f = new slFace;

	f -> _pointCount = nPoints;
	f -> neighbors = new slEdge*[nPoints];
	f -> voronoi = new slPlane[nPoints];
	f -> points = new slPoint*[nPoints];
	f -> faces = new slFace*[( nPoints + 1 )];
	f -> drawFlags = 0;

	slVectorSub( points[1], points[0], &x );
	slVectorSub( points[2], points[1], &y );
	slVectorCross( &x, &y, &f->plane.normal );
	slVectorNormalize( &f->plane.normal );
	slVectorCopy( points[1], &f->plane.vertex );

	slVectorSet( &origin, 0, 0, 0 );

	// uhoh, screwy plane, reverse-o!

	if ( slPlaneDistance( &f->plane, &origin ) > 0.0 ) {
		rpoints = new slVector*[nPoints];

		for ( n = 0;n < nPoints;n++ ) rpoints[n] = points[( nPoints - 1 ) - n];
		for ( n = 0;n < nPoints;n++ ) points[n] = rpoints[n];

		delete[] rpoints;

		slVectorSub( points[1], points[0], &x );
		slVectorSub( points[2], points[1], &y );
		slVectorCross( &x, &y, &f->plane.normal );
		slVectorNormalize( &f->plane.normal );
		slVectorCopy( points[1], &f->plane.vertex );
	}

	for ( n = 0;n < nPoints;n++ ) 
		f->points[n] = slAddPoint( s, points[ n ] );

	for ( n = 0;n < nPoints; n++ ) {
		int nextIndex = ( n + 1 ) % nPoints;

		f->neighbors[n] = slAddEdge( s, f, points[n], points[ nextIndex ] );

		// z is normal to the voronoi slPlane--that means it's parallel to the
		// slFace of the slPlane and perpendicular to the edge we're looking at

		slVectorSub( points[ nextIndex ], points[n], &x );
		slVectorCross( &f->plane.normal, &x, &y );

		slVectorNormalize( &y );
		slSetPlane( &f->voronoi[n], &y, points[n] );
	}

	s->faces.push_back( f );
	s->features.push_back( f );

	return f;
}

/**
 * \brief Add an edge to a shape.
 * 
 * The points of the edge should always be passed in moving clockwise
 * around the face of the plane, where the face of the clock is pointing
 * towards the normal (the "top" of the slPlane).
 */

slEdge *slAddEdge( slShape *s, slFace *f, slVector *start, slVector *end ) {
	slEdge *e, *originalEdge = NULL;
	slPoint *p1, *p2;
	slVector lineSegment, vNorm;
	int sameEdge;
	std::vector<slEdge*>::iterator ei;

	p1 = slAddPoint( s, start );
	p2 = slAddPoint( s, end );

	slVectorSub( start, end, &lineSegment );
	slVectorCross( &f->plane.normal, &lineSegment, &vNorm );

	// see if the edge already exists

	for ( ei = s->edges.begin() ; ei != s->edges.end(); ei++ ) {
		e = *ei;

		// if we've been here before, we're seeing the second slFace

		sameEdge = (( e->neighbors[0] == p1 && e->neighbors[1] == p2 ) ||
		            ( e->neighbors[1] == p1 && e->neighbors[0] == p2 ) );

		if ( sameEdge ) {
			originalEdge = e;

			// the first face is already there...

			originalEdge->faces[1] = f;
			originalEdge->neighbors[3] = f;

			/* and the other 3 voroni slPlanes have been added */

			slVectorNormalize( &vNorm );
			slSetPlane( &originalEdge->voronoi[3], &vNorm, start );

			return e;
		}
	}

	e = new slEdge;

	// if this is the mirror of another edge, copy in the neighbors
	// and voronoi slPlanes, bearing in mind that the order of the
	// neighboring points is reversed

	if ( originalEdge ) {
		e->neighbors[1] = originalEdge->neighbors[0];
		e->neighbors[0] = originalEdge->neighbors[1];
		e->neighbors[2] = originalEdge->neighbors[2];
		e->neighbors[3] = originalEdge->neighbors[3];

		e->points[0] = originalEdge->points[1];
		e->points[1] = originalEdge->points[0];

		e->faces[0] = originalEdge->faces[1];
		e->faces[1] = originalEdge->faces[0];

		memcpy( &e->voronoi[1], &originalEdge->voronoi[0], sizeof( slPlane ) );
		memcpy( &e->voronoi[0], &originalEdge->voronoi[1], sizeof( slPlane ) );
		memcpy( &e->voronoi[2], &originalEdge->voronoi[2], sizeof( slPlane ) );
		memcpy( &e->voronoi[3], &originalEdge->voronoi[3], sizeof( slPlane ) );
	} else {
		e->faces[0] = f;

		e->points[0] = p1;
		e->points[1] = p2;

		e->neighbors[0] = p1;
		e->neighbors[1] = p2;
		e->neighbors[2] = f;

		// first add the voronois for the ends

		slVectorNormalize( &lineSegment );
		slSetPlane( &e->voronoi[1], &lineSegment, end );

		slVectorMul( &lineSegment, -1, &lineSegment );
		slSetPlane( &e->voronoi[0], &lineSegment, start );

		// and now for the first slFace

		slVectorNormalize( &vNorm );
		slSetPlane( &e->voronoi[2], &vNorm, start );

		// the final face slPlane is added later
	}

	s->edges.push_back( e );
	s->features.push_back( e );

	return e;
}

/**
 * \brief Adds a point to a shape.
 *
 * Called as part of slAddEdge and slAddFace, not typically called manually.
 */

slPoint *slAddPoint( slShape *s, slVector *vertex ) {
	slPoint *p;
	std::vector<slPoint*>::iterator pi;

	// see if the point already exists

	for ( pi = s->points.begin(); pi != s->points.end(); pi++ ) {
		p = *pi;

		if ( !slVectorCompare( vertex, &p->vertex ) ) {
			return p;
		}
	}

	p = new slPoint;

	slVectorCopy( vertex, &p->vertex );

	s -> points.push_back( p );
	s -> features.push_back( p );

	return p;
}

/**
 * \brief Fills in an slPlane struct.
 */

slPlane *slSetPlane( slPlane *p, slVector *normal, slVector *vertex ) {
	slVectorCopy( normal, &p->normal );
	slVectorCopy( vertex, &p->vertex );
	slVectorNormalize( &p->normal );

	return p;
}

/*!
	\brief Sets a shape to be a rectangular solid.
*/

slShape *slSetCube( slShape *s, slVector *size, double density ) {
	slVector *face[4];
	slVector h;

	/* x+ slFace:  x- slFace:
		p1 - p2	
				 p5 - p6

		p3 - p4	
				 p7 - p8
	*/

	slVector p1, p2, p3, p4, p5, p6, p7, p8;

	slVectorMul( size, .5, &h );

	slVectorSet( &p1, h.x, h.y, h.z );
	slVectorSet( &p2, h.x, -h.y, h.z );
	slVectorSet( &p3, h.x, h.y, -h.z );
	slVectorSet( &p4, h.x, -h.y, -h.z );

	slVectorSet( &p5, -h.x, h.y, h.z );
	slVectorSet( &p6, -h.x, -h.y, h.z );
	slVectorSet( &p7, -h.x, h.y, -h.z );
	slVectorSet( &p8, -h.x, -h.y, -h.z );

	face[0] = &p1;
	face[1] = &p2;
	face[2] = &p4;
	face[3] = &p3;

	slAddFace( s, face, 4 );

	face[0] = &p5;
	face[1] = &p6;
	face[2] = &p2;
	face[3] = &p1;

	slAddFace( s, face, 4 );

	face[0] = &p7;
	face[1] = &p8;
	face[2] = &p6;
	face[3] = &p5;

	slAddFace( s, face, 4 );

	face[0] = &p3;
	face[1] = &p4;
	face[2] = &p8;
	face[3] = &p7;

	slAddFace( s, face, 4 );

	face[0] = &p1;
	face[1] = &p3;
	face[2] = &p7;
	face[3] = &p5;

	slAddFace( s, face, 4 );

	face[0] = &p2;
	face[1] = &p6;
	face[2] = &p8;
	face[3] = &p4;

	slAddFace( s, face, 4 );

	s -> finishShape( density );

	return s;
}

/*!
	\brief Sets a shape to be an extruded polygon.
*/

slShape *slSetNGonDisc( slMeshShape *s, int sideCount, double radius, double height, double density ) {
	int n;
	slVector *tops, *bottoms, sides[4];
	slVector **topP, **bottomP, *sideP[4];

	if ( sideCount < 3 ) return NULL;

	tops = new slVector[sideCount + 1];

	bottoms = new slVector[sideCount + 1];

	topP = new slVector*[sideCount + 1];

	bottomP = new slVector*[sideCount + 1];

	float angleStart = .5 * 2.0 * M_PI / sideCount;

	for ( n = 0;n < sideCount;n++ ) {
		topP[sideCount - ( n + 1 )] = &tops[n];
		bottomP[n] = &bottoms[n];

		tops[n].y = ( height / 2 );
		bottoms[n].y = -( height / 2 );

		tops[n].x = radius * cos( angleStart + n * ( 2 * M_PI / sideCount ) );
		tops[n].z = radius * sin( angleStart + n * ( 2 * M_PI / sideCount ) );

		bottoms[n].x = radius * cos( angleStart + n * ( 2 * M_PI / sideCount ) );
		bottoms[n].z = radius * sin( angleStart + n * ( 2 * M_PI / sideCount ) );
	}

	slVectorCopy( &tops[0], &tops[n] );
	slVectorCopy( &bottoms[0], &bottoms[n] );

	slAddFace( s, topP, sideCount );
	slAddFace( s, bottomP, sideCount );

	sideP[0] = &sides[0];
	sideP[1] = &sides[1];
	sideP[2] = &sides[2];
	sideP[3] = &sides[3];

	for ( n = 0;n < sideCount;n++ ) {
		slVectorCopy( &tops[n], &sides[3] );
		slVectorCopy( &bottoms[n], &sides[2] );
		slVectorCopy( &bottoms[n + 1], &sides[1] );
		slVectorCopy( &tops[n + 1], &sides[0] );

		slAddFace( s, sideP, 4 );
	}

	delete[] tops;

	delete[] bottoms;
	delete[] topP;
	delete[] bottomP;

	s -> finishShape( density );

	return s;
}

/**
 * \brief Sets a shape to be a cone with a polygon base.
 */

slShape *slSetNGonCone( slMeshShape *s, int sideCount, double radius, double height, double density ) {
	int n;
	slVector top;
	slVector *bottoms, sides[3];
	slVector **bottomP, *sideP[3];

	if ( sideCount < 3 ) return NULL;

	bottoms = new slVector[sideCount + 1];

	bottomP = new slVector*[sideCount + 1];

	slVectorSet( &top, 0, height, 0 );

	for ( n = 0;n < sideCount;n++ ) {
		bottomP[n] = &bottoms[n];

		bottoms[n].y = -( height / 2 );

		bottoms[n].x = radius * cos( n * ( 2 * M_PI / sideCount ) );
		bottoms[n].z = radius * sin( n * ( 2 * M_PI / sideCount ) );
	}

	slVectorCopy( &bottoms[0], &bottoms[n] );

	slAddFace( s, bottomP, sideCount );

	sideP[0] = &sides[0];
	sideP[1] = &sides[1];
	sideP[2] = &sides[2];

	for ( n = 0;n < sideCount;n++ ) {
		slVectorCopy( &bottoms[n], &sides[2] );
		slVectorCopy( &bottoms[n + 1], &sides[1] );
		slVectorCopy( &top, &sides[0] );

		slAddFace( s, sideP, 3 );
	}

	delete[] bottoms;

	delete[] bottomP;

	s -> finishShape( density );

	return s;
}

void slSphereInertiaMatrix( double r, double mass, double i[3][3] ) {
	slMatrixZero( i );
	i[0][0] = i[1][1] = i[2][2] = 0.4 * mass * r * r;
}

/*!
	\brief Returns the point on the shape in the specified direction.

	This is useful when you want to stick a limb at the
	(0, 0, 1) [for example] side of the shape, even though
	you don't care how big the shape is.  this function
	will give the corresponding link point.

	point/normal plane eq:

	Ax + By + Cz - D = 0

	We use the existing point normal to create D.  Then we know that
	there is some point on the dir vector which satisfies:

	nx * px + ny * py + nz * pz = D

	Which means that:

	(nx * k dir.x) + (ny * k dir.y) + (nz * k dir.z) - D = 0.

	nx dir.x, ny dir.y, nz dir.z  are computed as X, Y, Z, so

	k * (X+Y+Z) = D.

	k = D/(X+Y+Z).
*/

int slSphere::pointOnShape( slVector *dir, slVector *point ) {
	slVectorMul( dir, _radius, point );
	return 0;
}

int slShape::pointOnShape( slVector *dir, slVector *point ) {
	double D, X, Y, Z, k;
	slVector pointOnPlane;
	slPosition pos;
	int update, result, planes = 0;
	double distance, best = -10000;
	std::vector<slFace*>::iterator fi;

	slVectorSet( point, 0.0, 0.0, 0.0 );

	slVectorSet( &pos.location, 0.0, 0.0, 0.0 );
	slMatrixIdentity( pos.rotation );

	slVectorNormalize( dir );

	for ( fi = faces.begin(); fi != faces.end(); fi++ ) {
		slFace *f = *fi;

		planes++;

		D = slVectorDot( &f->plane.normal, &f->plane.vertex );

		X = f->plane.normal.x * dir->x;
		Y = f->plane.normal.y * dir->y;
		Z = f->plane.normal.z * dir->z;

		k = D / ( X + Y + Z );

		if (( X + Y + Z ) != 0.0 && k > 0.0 ) {
			// we have the length of the matching vector on the plane of this face.

			slVectorMul( dir, k, &pointOnPlane );

			// distance = slPlaneDistance(&f->plane, &pointOnPlane);

			// now figure out if the point in question is within the face

			result = slClipPointMax( &pointOnPlane, f->voronoi, &pos, f->_pointCount, &update, &distance );

			// if this point is within the voronoi region of the plane, it must be on the face.
			// even if it's not, it might be due to small mathematical error, so we'll keep track
			// of the best of the failures.

			if ( result == 1 ) {
				slVectorCopy( &pointOnPlane, point );
				return 0;
			} else if ( distance > best ) {
				best = distance;
				slVectorCopy( &pointOnPlane, point );
			}
		}
	}

	printf( "warning: no shape point could be found\n" );

	return -1;
}

/**
 * \brief Returns the point where a ray send from location with direction hit the shape.
 * The location and the direction must be given int the frame of the shape. But the returned
 * point is relative to the location. This function should only be use by the slWorldObjectRaytrace
 * function.
 */

int slRayHitsShape( slShape *s, slVector *dir, slVector *target, slVector *point ) {
	slVector invert;
	slVectorMul( dir, -1, &invert );
	return s->rayHitsShape( &invert, target, point );
}

int slSphere::rayHitsShape( slVector *dir, slVector *target, slVector *point ) {
	slVector current;
	slVectorMul( dir, -1, &current );
	slVectorSet( point, 0.0, 0.0, 0.0 );

	double v1 = current.x;
	double v2 = current.y;
	double v3 = current.z;
	double u1 = target->x;
	double u2 = target->y;
	double u3 = target->z;

	double a = v1 * v1   + v2 * v2   + v3 * v3;
	double b = 2 * v1 * u1 + 2 * v2 * u2 + 2 * v3 * u3;
	double c = u1 * u1   + u2 * u2   + u3 * u3   - _radius * _radius;

	double test = b * b - 4 * a * c;

	if (( test < 0 ) || ( a == 0 ) )
		return -1;

	double x1 = ( -b + sqrt( test ) ) / ( 2 * a );

	double x2 = ( -b - sqrt( test ) ) / ( 2 * a );

	if (( x1 < 0 ) && ( x2 < 0 ) )
		return -1;

	double erg = x1 < x2 ? x1 : x2;

	slVectorNormalize( &current );

	slVectorMul( &current, erg, point );

	return 0;
}


int slShape::rayHitsShape( slVector *dir, slVector *target, slVector *point ) {
	double D, X, Y, Z, k;
	slVector pointOnPlane;
	slPosition pos;
	int update, result, planes = 0;
	double distance;
	double minDistance = 1.0e10;
	bool isFirst = true;
	std::vector<slFace*>::iterator fi;


	slVectorSet( point, 0.0, 0.0, 0.0 );

	slVectorSet( &pos.location, 0.0, 0.0, 0.0 );
	slMatrixIdentity( pos.rotation );

	slVectorNormalize( dir );

	for ( fi = faces.begin(); fi != faces.end(); fi++ ) {
		slFace *f = *fi;

		planes++;

		slPlane newPlane;
		slVectorMul( &f->plane.normal, -1, &newPlane.normal );
		slVectorAdd( &f->plane.vertex, target, &newPlane.vertex );

		D = slVectorDot( &newPlane.normal, &newPlane.vertex );

		X = newPlane.normal.x * dir->x;
		Y = newPlane.normal.y * dir->y;
		Z = newPlane.normal.z * dir->z;

		k = D / ( X + Y + Z );

		// k>0 means that dir is pointing to the right direction

		if (( X + Y + Z ) != 0.0 && k > 0.0 ) {
			/* we have the length k of the matching vector on the plane of this */
			/* face. */

			slVectorMul( dir, k, &pointOnPlane );

			/* discribe the vector in respect to the target frame */
			slVector checkPointOnPlane;
			slVectorSub( &pointOnPlane, target, &checkPointOnPlane );

			// distance = slPlaneDistance(&f->plane, &pointOnPlane);

			/* now figure out if the point in question is within the face */

			result = slClipPoint( &checkPointOnPlane, f->voronoi, &pos, f->_pointCount, &update, &distance );

			/* if this point is within the voronoi region of the plane, it must be */
			/* on the face */

			if ( result == 1 ) {
				// sets the first one found as minDistance

				if ( isFirst ) {
					minDistance = k;
					isFirst = false;
					slVectorCopy( &pointOnPlane, point );
				} else if ( k < minDistance ) {
					//slMessage(DEBUG_ALL, " Raytrace plane:[ %f, %f, %f ]  vector: [%f, %f, %f] distance: %f result: %d\n\n", f->plane.vertex.x, f->plane.vertex.y, f->plane.vertex.z, dir->x, dir->y, dir->z, k, result );
					slVectorCopy( &pointOnPlane, point );
					minDistance = k;
				}
			}
		}
	}

	// we hit a face

	if ( !isFirst ) return 0;

	return -1;
}

void slShape::setScale( slVector *scale ) {
	_scale = *scale;

	// printf( "scale = %f %f, %f\n", scale -> x, scale -> y, scale -> y );
	// printf( "total scale = %f %f, %f\n", _transform[ 0 ][ 0 ], _transform[ 1 ][ 1 ], _transform[ 2 ][ 2 ] );

	finishShape( _density );
}

slSerializedShapeHeader *slSphere::serialize( int *length ) {
	slSerializedShapeHeader *header;

	*length = sizeof( slSerializedShapeHeader );
	header = ( slSerializedShapeHeader* )slMalloc( sizeof( slSerializedShapeHeader ) );

	header->_type 		= ST_SPHERE;
	header->_radius 	= _radius;
	header->_density 	= _density;

	return header;
}

slSerializedShapeHeader *slBox::serialize( int *length ) {
	slSerializedShapeHeader *header;

	*length = sizeof( slSerializedShapeHeader );
	header = ( slSerializedShapeHeader* )slMalloc( sizeof( slSerializedShapeHeader ) );

	header->_type 		= ST_BOX;
	header->_density 	= _density;

	slVectorCopy( &_size, &header->_max );

	return header;
}

slSerializedShapeHeader *slShape::serialize( int *length ) {
	int p, faceCount = 0, facePointCount = 0;
	slSerializedShapeHeader *header;
	slSerializedFaceHeader *fhead;
	slFace *face;
	char *data, *position;
	slVector *v;
	std::vector<slFace*>::iterator fi;

	// count the different points and faces we have to add

	for ( fi = faces.begin(); fi != faces.end(); fi++ ) {
		face = *fi;

		faceCount++;

		// the number of edges on the face == number of points

		facePointCount += face->_pointCount;
	}

	// figure out the total length of the data

	*length = sizeof( slSerializedShapeHeader ) +            // header
	          ( faceCount * sizeof( slSerializedFaceHeader ) ) + // face headers
	          ( facePointCount * sizeof( slVector ) );       // face points

	data = ( char* )slMalloc( *length );

	header = ( slSerializedShapeHeader* )data;

	// fill in the header 

	header->_faceCount 	= faceCount;
	header->_type 		= _type;
	header->_density 	= _density;

	slVectorCopy( &_max, &header->_max );

	// skip over the header 

	position = data + sizeof( slSerializedShapeHeader );

	for ( fi = faces.begin(); fi != faces.end(); fi++ ) {
		face = *fi;
		fhead = ( slSerializedFaceHeader* )position;

		fhead->vertexCount = face->_pointCount;

		position += sizeof( slSerializedFaceHeader );

		for ( p = 0;p < face->_pointCount;p++ ) {
			v = ( slVector* )position;

			slVectorCopy( &(( slPoint* )face->points[p] )->vertex, v );
			position += sizeof( slVector );
		}
	}

	return ( slSerializedShapeHeader* )data;
}

slShape *slShape::deserialize( slSerializedShapeHeader *header ) {
	slShape *s;
	slSerializedFaceHeader *face;
	char *p;
	int n, vind;
	slVector *v, *vectors, **vectorp;

	if ( header->_type == ST_BOX ) 
		return new slBox( &header->_max, header->_density );

	if ( header->_type == ST_SPHERE ) 
		return new slSphere( header->_radius, header->_density );


	s = new slMeshShape();

	// offset the pointer to the endge of the header

	p = ( char* )header + sizeof( slSerializedShapeHeader );

	for ( n = 0;n < header->_faceCount;n++ ) {
		face = ( slSerializedFaceHeader* )p;

		vectors = new slVector[ face->vertexCount ];
		vectorp = new slVector*[ face->vertexCount ];

		p += sizeof( slSerializedFaceHeader );

		for ( vind = 0;vind < face->vertexCount;vind++ ) {
			v = ( slVector* )p;

			slVectorCopy( v, &vectors[vind] );

			vectorp[vind] = &vectors[vind];

			p += sizeof( slVector );
		}

		slAddFace( s, vectorp, face->vertexCount );

		delete[] vectors;
		delete[] vectorp;
	}

	s -> finishShape( header->_density );

	return s;
}

void slSphere::bounds( const slPosition *position, slVector *outMin, slVector *outMax ) const {
	slVector min, max;
	float r = _radius * _transform[ 0 ][ 0 ];

	slVectorSet( &max,  r,  r,  r );
	slVectorSet( &min, -r, -r, -r );

	slVectorAdd( &max, &position -> location, outMax );
	slVectorAdd( &min, &position -> location, outMin );
}

void slShape::bounds( const slPosition *position, slVector *outMin, slVector *outMax ) const {
	std::vector< slPoint* >::const_iterator pi;

	slVector min, max;

	slVectorSet( &max, INT_MIN, INT_MIN, INT_MIN );
	slVectorSet( &min, INT_MAX, INT_MAX, INT_MAX );

	for ( pi = points.begin(); pi != points.end(); pi++ ) {
		slPoint *p = *pi;
		slVector loc, tloc;

		tloc.x = p -> vertex.x * _scale.x;
		tloc.y = p -> vertex.y * _scale.y;
		tloc.z = p -> vertex.z * _scale.z;
		
		slVectorXform( position->rotation, &tloc, &loc );

		if ( loc.x > max.x ) max.x = loc.x;
		if ( loc.y > max.y ) max.y = loc.y;
		if ( loc.z > max.z ) max.z = loc.z;

		if ( loc.x < min.x ) min.x = loc.x;
		if ( loc.y < min.y ) min.y = loc.y;
		if ( loc.z < min.z ) min.z = loc.z;
	}

	slVectorAdd( &min, &position->location, outMin );
	slVectorAdd( &max, &position->location, outMax );
}

double slShape::getMass() {
	return _mass;
}

double slShape::getDensity() {
	return _density;
}






#define X .525731112119133606 
#define Z .850650808352039932

static GLfloat sphereVertexData[12][3] = {    
	{-X, 0.0, Z}, {X, 0.0, Z}, {-X, 0.0, -Z}, {X, 0.0, -Z},	
	{0.0, Z, X}, {0.0, Z, -X}, {0.0, -Z, X}, {0.0, -Z, -X},	
	{Z, X, 0.0}, {-Z, X, 0.0}, {Z, -X, 0.0}, {-Z, -X, 0.0} 
};

static GLuint sphereIndices[20][3] = { 
	{0,4,1}, {0,9,4}, {9,5,4}, {4,5,8}, {4,8,1},	
	{8,10,1}, {8,3,10}, {5,3,8}, {5,2,3}, {2,7,3},	
	{7,10,3}, {7,6,10}, {7,11,6}, {11,0,6}, {0,1,6}, 
	{6,1,10}, {9,0,11}, {9,11,2}, {9,2,5}, {7,2,11} };

void normalize(GLfloat *a) {
	GLfloat d=sqrt(a[0]*a[0]+a[1]*a[1]+a[2]*a[2]);
	a[0]/=d; a[1]/=d; a[2]/=d;
}

int	slSphere::addTriangles( GLfloat *inX, GLfloat *inY, GLfloat *inZ, int inDiv, float inR, int inIndex ) {
	if ( inDiv <= 0 ) {
		float *v;

		v = _vertexBuffer.normal( inIndex	 );
		v[ 0 ] = inX[ 0 ]; v[ 1 ] = inX[ 1 ]; v[ 2 ] = inX[ 2 ];

		v = _vertexBuffer.normal( inIndex + 1 );
		v[ 0 ] = inY[ 0 ]; v[ 1 ] = inY[ 1 ]; v[ 2 ] = inY[ 2 ];

		v = _vertexBuffer.normal( inIndex + 2 );
		v[ 0 ] = inZ[ 0 ]; v[ 1 ] = inZ[ 1 ]; v[ 2 ] = inZ[ 2 ];

		v = _vertexBuffer.vertex( inIndex	 );
		v[ 0 ] = inR * inX[ 0 ]; v[ 1 ] = inR * inX[ 1 ]; v[ 2 ] = inR * inX[ 2 ];

		v = _vertexBuffer.vertex( inIndex + 1 );
		v[ 0 ] = inR * inY[ 0 ]; v[ 1 ] = inR * inY[ 1 ]; v[ 2 ] = inR * inY[ 2 ];

		v = _vertexBuffer.vertex( inIndex + 2 );
		v[ 0 ] = inR * inZ[ 0 ]; v[ 1 ] = inR * inZ[ 1 ]; v[ 2 ] = inR * inZ[ 2 ];
		
		inIndex += 3;

    } else {
        GLfloat ab[3], ac[3], bc[3];

        for( int i = 0; i < 3; i++ ) {
            ab[ i ]=( inX[ i ] + inY[ i ] ) / 2;
            ac[ i ]=( inX[ i ] + inZ[ i ] ) / 2;
            bc[ i ]=( inY[ i ] + inZ[ i ] ) / 2;
        }

        normalize( ab ); normalize( ac ); normalize( bc );
        inIndex = addTriangles( inX, ab, ac, inDiv - 1, inR, inIndex );
        inIndex = addTriangles( inY, bc, ab, inDiv - 1, inR, inIndex );
        inIndex = addTriangles( inZ, ac, bc, inDiv - 1, inR, inIndex );
        inIndex = addTriangles(  ab, bc, ac, inDiv - 1, inR, inIndex );  
    }  
	
	return inIndex;
}

void slSphere::fillVertexBuffer() {
	int divisions = 1;
	int vertices = 0;

 	_vertexBuffer.resize( 20 * 3 * (int)pow( 4, ( divisions ) ), VB_XYZ | VB_NORMAL );

    for( int i = 0; i < 20; i++ )
        vertices = addTriangles( sphereVertexData[ sphereIndices[ i ][ 0 ] ], sphereVertexData[ sphereIndices[ i ][ 1 ] ], sphereVertexData[ sphereIndices[ i ][ 2 ] ], divisions, _radius, vertices );
}
