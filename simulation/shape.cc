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
#include "glIncludes.h"
#include "gldraw.h"
#include "camera.h"
#include "volInt.h"
#include "vclip.h"
#include "vclipData.h"

void slShape::draw( slCamera *c, slPosition *pos, double inTScaleX, double inTScaleY, int mode, int flags ) {
	unsigned char bound, axis;

	bound = ( mode & DM_BOUND ) && !( flags & DO_NO_BOUND );
	axis = ( mode & DM_AXIS ) && !( flags & DO_NO_AXIS );

	if ( _drawList == 0 || _recompile || ( flags & DO_RECOMPILE ) ) 
		slCompileShape( this, c->_drawMode, flags );

	glPushMatrix();

	glTranslatef( pos->location.x, pos->location.y, pos->location.z );

	slMatrixGLMult( pos->rotation );

	glPushAttrib( GL_TRANSFORM_BIT );
	glMatrixMode( GL_TEXTURE );
	glLoadIdentity();

	glTranslatef( 0.5, 0.5, 0.0 );

	if( inTScaleX > 0.0 && inTScaleY > 0.0 )
		glScalef( 1.0 / inTScaleX, 1.0 / inTScaleY, 1.0 );

	glPopAttrib();

	if ( flags & DO_OUTLINE ) {
		glPushAttrib( GL_ENABLE_BIT );
		glDisable( GL_LIGHTING );
		glPushMatrix();
		glScalef( .99, .99, .99 );
		glPolygonOffset( 1, 2 );
		glEnable( GL_POLYGON_OFFSET_FILL );
		glCallList( _drawList );
		glPopMatrix();
		glEnable( GL_BLEND );
		glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );
		glColor4f( 0, 0, 0, .5 );
		glDepthMask( GL_FALSE );
		slRenderShape( this, GL_LINE_LOOP, 0 );
		glDepthMask( GL_FALSE );
		glDisable( GL_POLYGON_OFFSET_FILL );

		if ( _type == ST_SPHERE ) {
			glColor4f( 1, 1, 1, 0 );
			glDisable( GL_DEPTH_TEST );
			glScalef( .96, .96, .96 );
			glCallList( _drawList );
			glEnable( GL_DEPTH_TEST );
		}

		glPopAttrib();
	} else {
		glCallList( _drawList );
	}

	if ( bound || axis ) {
		glPushAttrib( GL_COLOR_BUFFER_BIT );
		glEnable( GL_BLEND );
		glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );
		glColor4f( 0.0, 0.0, 0.0, 0.5 );
		glScalef( 1.1, 1.1, 1.1 );

		if ( axis ) slDrawAxis( _max.x, _max.y );

		if ( bound ) slRenderShape( this, GL_LINE_LOOP, 0 );

		glPopAttrib();
	}

	glPopMatrix();
}

void slShape::slMatrixToODEMatrix( const double inM[ 3 ][ 3 ], dMatrix3 outM ) {
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

	slVectorSet( &_max, _radius, _radius, _radius );

	_mass = _density * M_PI * _radius * _radius * _radius * 4.0 / 3.0;

	slSphereInertiaMatrix( _radius, _mass, _inertia );

	_odeGeomID[ 0 ] = dCreateSphere( 0, _radius );
	_odeGeomID[ 1 ] = dCreateSphere( 0, _radius );
}

slBox::slBox( slVector *inSize, double inDensity ) {
	if ( inDensity < 0.0 )
		throw slException( std::string( "invalid density for new cube (density <= 0.0)" ) );

	if ( inSize->x <= 0.0 || inSize->y <= 0.0 || inSize->z <= 0 )
		throw slException( std::string( "invalid size for new sphere (side <= 0.0)" ) );

	if ( !slSetCube( this, inSize, inDensity ) ) {
		 release();
		throw slException( std::string( "could not create a cube with the specified parameters" ) );
	}

	_odeGeomID[ 0 ] = dCreateBox( 0, inSize -> x, inSize -> y, inSize -> z );
	_odeGeomID[ 1 ] = dCreateBox( 0, inSize -> x, inSize -> y, inSize -> z );
}

slShape *slNewNGonDisc( int count, double radius, double height, double density ) {
	if ( radius <= 0.0 || height <= 0.0 || density <= 0.0 || count < 3 ) return NULL;

	slMeshShape *s = new slMeshShape();

	// if ( !slSetNGonDisc( s, count, radius, height, density ) ) {
	slVector v;

	slVectorSet( &v, height, radius, radius );
	if ( !slSetCube( s, &v, density ) ) {
		s->release();
		return NULL;
	}

	s -> finishShape( density );

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

void slShapeFree( slShape *s ) {
	s->release();
}

void slShape::retain() {
	_referenceCount++;
}

void slShape::release() {
	if ( --_referenceCount ) return;
	delete this;
}

slShape::~slShape() {
	std::vector< slFeature* >::iterator fi;

	for ( fi = features.begin() ; fi != features.end(); fi++ ) delete *fi;

	if ( _drawList ) 
		glDeleteLists( _drawList, 1 );

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

void slMeshShape::draw( slCamera *c, slPosition *pos, double inTScaleX, double inTScaleY, int mode, int flags ) {
	slShape::draw( c, pos, inTScaleX, inTScaleY, mode, flags );

	updateLastPosition( pos );
}


void slMeshShape::updateLastPosition( slPosition *inPosition ) {
	dMatrix4 transform = {
		inPosition -> rotation[0][0], inPosition -> rotation[0][1], inPosition -> rotation[0][2], 0,
		inPosition -> rotation[1][0], inPosition -> rotation[1][1], inPosition -> rotation[1][2], 0,
		inPosition -> rotation[2][0], inPosition -> rotation[2][1], inPosition -> rotation[2][2], 0,
		inPosition -> location.x, inPosition -> location.y, inPosition -> location.z,  1
	};

	memcpy( _lastPositions[ _lastPositionIndex ], transform, sizeof( dMatrix4 ) );

	dGeomTriMeshSetLastTransform( _odeGeomID[ 0 ], _lastPositions[ _lastPositionIndex ] );
	dGeomTriMeshSetLastTransform( _odeGeomID[ 1 ], _lastPositions[ _lastPositionIndex ] );

	_lastPositionIndex = !_lastPositionIndex;
}


void slMeshShape::finishShape( double density ) {
	int triCount = 0;

	for( unsigned int n = 0; n < faces.size(); n++ ) {
		triCount += faces[ n ] -> _pointCount - 2;
	}

	_vertexCount = points.size();
	_indexCount = 3 * triCount;

	_vertices = new float[ _vertexCount * 3 ];
	_indices = new int[ _indexCount ];

	for( int n = 0; n < _vertexCount; n++ ) {
		slPoint *p = points[ n ];

		_vertices[ n * 3     ] = p -> vertex.x;
		_vertices[ n * 3 + 1 ] = p -> vertex.y;
		_vertices[ n * 3 + 2 ] = p -> vertex.z;
	}

	int tri = 0;

	for( unsigned int n = 0; n < faces.size(); n++ ) {
		slFace *f = faces[ n ];	

		// printf( "FACE %d\n", n );

		int i1 = findPointIndex( &f -> points[ 0 ] -> vertex );

		for( int m = 1; m < f -> _pointCount - 1; m++ ) {
			int i2 = findPointIndex( &f -> points[ m ] -> vertex );
			int i3 = findPointIndex( &f -> points[ m + 1 ] -> vertex );

			_indices[ tri * 3     ] = i1;
			_indices[ tri * 3 + 1 ] = i2;
			_indices[ tri * 3 + 2 ] = i3;

			slVector v1, v2, normal;

			v1.x = _vertices[ i2 * 3 ] - _vertices[ i1 * 3 ];
			v1.y = _vertices[ i2 * 3 + 1 ] - _vertices[ i1 * 3 + 1 ];
			v1.z = _vertices[ i2 * 3 + 2 ] - _vertices[ i1 * 3 + 2 ];

			v2.x = _vertices[ i3 * 3 ] - _vertices[ i1 * 3 ];
			v2.y = _vertices[ i3 * 3 + 1 ] - _vertices[ i1 * 3 + 1 ];
			v2.z = _vertices[ i3 * 3 + 2 ] - _vertices[ i1 * 3 + 2 ];

			slVectorCross( &v1, &v2, &normal );

			// printf( "tri %d (%f %f %f) (%f %f %f) (%f %f %f)\n", tri,
			// 	points[ i1 ] -> vertex.x, points[ i1 ] -> vertex.y, points[ i1 ] -> vertex.z,
			// 	points[ i2 ] -> vertex.x, points[ i2 ] -> vertex.y, points[ i2 ] -> vertex.z,
			// 	points[ i3 ] -> vertex.x, points[ i3 ] -> vertex.y, points[ i3 ] -> vertex.z );
			// printf("NORMAL: %f %f %f\n", normal.x, normal.y, normal.z );


			tri++;
		}
	}

	for( int n = 0; n < _vertexCount; n++ ) {
		//printf( "\t%f, %f, %f\n", _vertices[ n * 3 ], _vertices[ n * 3 + 1 ], _vertices[ n * 3 + 2 ] );
	}

	for( int n = 0; n < _indexCount; n += 3 ) {
		// printf( "\t%d, %d, %d\n", _indices[ n ], _indices[ n + 1 ], _indices[ n + 2 ] );
	}

	_maxReach = 30;
	_density = density;
	createODEGeom();
}

slShape *slFinishShape( slShape *s, double density ) {

	s->_density = density;

	slVectorZero( &s->_max );

	for( unsigned int i = 0; i < s -> points.size(); i++ ) {
		slPoint *p = s -> points[ i ];

		if ( ! p->neighbors ) p->neighbors = new slEdge*[p->edgeCount];

		if ( ! p->faces ) p->faces = new slFace*[p->edgeCount];

		if ( ! p->voronoi ) p->voronoi = new slPlane[p->edgeCount];

		// now check to see if it's a maximum for the shape

		if ( p->vertex.x > s->_max.x ) s->_max.x = p->vertex.x;
		if ( p->vertex.y > s->_max.y ) s->_max.y = p->vertex.y;
		if ( p->vertex.z > s->_max.z ) s->_max.z = p->vertex.z;
	}

	if ( slSetMassProperties( s, density ) ) return NULL;

	return s;
}

/*!
	\brief Sets the mass of the shape.

	Using the shape's volume, sets the density of the shape so that
	the desired mass is achived.
*/

void slShape::setMass( double newMass ) {
	double volume;

	// the existing volume...

	volume = _mass / _density;

	// the desired density...

	_density = newMass / volume;

	setDensity( _density );
}

/*!
	\brief Sets the density of the shape.

	Sets the density of the change, which modifies the mass.
*/

void slShape::setDensity( double newDensity ) {
	slSetMassProperties( this, newDensity );
}

/*!
	\brief Sets the density of the sphere.

	Sets the density of the change, which modifies the mass.
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

	f->type = FT_FACE;

	f -> _pointCount = nPoints;

	f->edgeCount = 0;

	f->neighbors = new slEdge*[nPoints];

	f->voronoi = new slPlane[nPoints];

	f->points = new slPoint*[nPoints];

	f->faces = new slFace*[( nPoints + 1 )];

	f->drawFlags = 0;

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

		printf( "new dist %f\n", slPlaneDistance( &f->plane, &origin )  );
	}

	for ( n = 0;n < nPoints;n++ ) 
		f->points[n] = slAddPoint( s, points[ n ] );

	f->edgeCount = nPoints;

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

/*!
	\brief Add an edge to a shape.

	The points of the edge should always be passed in moving clockwise
	around the face of the plane, where the face of the clock is pointing
	towards the normal (the "top" of the slPlane).
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

	e->type = FT_EDGE;

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

/*!
	\brief Adds a point to a shape.

	Called as part of slAddEdge and slAddFace, not typically called manually.
*/

slPoint *slAddPoint( slShape *s, slVector *vertex ) {
	slPoint *p;
	std::vector<slPoint*>::iterator pi;

	// see if the point already exists

	for ( pi = s->points.begin(); pi != s->points.end(); pi++ ) {
		p = *pi;

		if ( !slVectorCompare( vertex, &p->vertex ) ) {
			p->edgeCount++;

			return p;
		}
	}

	p = new slPoint;

	p->type = FT_POINT;

	if ( !p ) return NULL;

	slVectorCopy( vertex, &p->vertex );

	p->edgeCount = 1;

	s->points.push_back( p );

	s->features.push_back( p );

	return p;
}

/*!
	\brief Fills in an slPlane struct.
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

	return slFinishShape( s, density );
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

	return slFinishShape( s, density );
}

/*!
	\brief Sets a shape to be a cone with a polygon base.
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

	return slFinishShape( s, density );
}

void slCubeInertiaMatrix( slVector *c, double mass, double i[3][3] ) {
	slMatrixZero( i );

	i[0][0] = 1.0 / 12.0 * mass * ( c->y * c->y + c->z * c->z );
	i[1][1] = 1.0 / 12.0 * mass * ( c->x * c->x + c->z * c->z );
	i[2][2] = 1.0 / 12.0 * mass * ( c->x * c->x + c->y * c->y );
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

		// printf("d = %f\n", D);

		X = f->plane.normal.x * dir->x;
		Y = f->plane.normal.y * dir->y;
		Z = f->plane.normal.z * dir->z;

		k = D / ( X + Y + Z );

		if (( X + Y + Z ) != 0.0 && k > 0.0 ) {
			// we have the length of the matching vector on the plane of this face.

			slVectorMul( dir, k, &pointOnPlane );

			// distance = slPlaneDistance(&f->plane, &pointOnPlane);

			// now figure out if the point in question is within the face

			result = slClipPointMax( &pointOnPlane, f->voronoi, &pos, f->edgeCount, &update, &distance );

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

		//printf("d = %f\n", D);

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

			result = slClipPoint( &checkPointOnPlane, f->voronoi, &pos, f->edgeCount, &update, &distance );

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

void slSphere::scale( slVector *scale ) {
	_recompile = 1;
	_radius *= scale->x;
	slFinishShape( this, this->_density );
}

void slShape::scale( slVector *scale ) {
	std::vector<slFace*>::iterator fi;
	std::vector<slPoint*>::iterator pi;
	slPoint *p;
	slFace *f;

	slVector v;
	double m[3][3];

	slMatrixIdentity( m );
	m[0][0] = scale->x;
	m[1][1] = scale->y;
	m[2][2] = scale->z;

	_recompile = 1;

	for ( pi = points.begin(); pi != points.end(); pi++ ) {
		p = *pi;

		slVectorCopy( &p->vertex, &v );
		slVectorXform( m, &v, &p->vertex );
	}

	for ( fi = faces.begin(); fi != faces.end(); fi++ ) {
		f = *fi;

		slVectorCopy( &f->plane.vertex, &v );
		slVectorXform( m, &v, &f->plane.vertex );
		slVectorCopy( &f->plane.normal, &v );
		slVectorXform( m, &v, &f->plane.normal );
		slVectorNormalize( &f->plane.normal );
	}

	slFinishShape( this, _density );
}

/*!
	\brief Serializes a shape and returns data that can later be deserialized.

	This function returns an slMalloc'd pointer.
*/

slSerializedShapeHeader *slSerializeShape( slShape *s, int *length ) {
	return s->serialize( length );
}

slSerializedShapeHeader *slSphere::serialize( int *length ) {
	slSerializedShapeHeader *header;

	*length = sizeof( slSerializedShapeHeader );
	header = ( slSerializedShapeHeader* )slMalloc( sizeof( slSerializedShapeHeader ) );

	header->type = ST_SPHERE;
	header->radius = _radius;
	header->mass = _mass;
	header->density = _density;

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

		facePointCount += face->edgeCount;
	}

	// figure out the total length of the data

	*length = sizeof( slSerializedShapeHeader ) +            // header
	          ( faceCount * sizeof( slSerializedFaceHeader ) ) + // face headers
	          ( facePointCount * sizeof( slVector ) );       // face points

	data = ( char* )slMalloc( *length );

	header = ( slSerializedShapeHeader* )data;

	/* fill in the header */

	header->faceCount = faceCount;

	header->type = _type;

	slMatrixCopy( _inertia, header->inertia );

	header->density = _density;

	header->mass = _mass;

	slVectorCopy( &_max, &header->max );

	/* skip over the header */

	position = data + sizeof( slSerializedShapeHeader );

	for ( fi = faces.begin(); fi != faces.end(); fi++ ) {
		face = *fi;
		fhead = ( slSerializedFaceHeader* )position;

		fhead->vertexCount = face->edgeCount;

		position += sizeof( slSerializedFaceHeader );

		for ( p = 0;p < face->edgeCount;p++ ) {
			v = ( slVector* )position;

			slVectorCopy( &(( slPoint* )face->points[p] )->vertex, v );
			position += sizeof( slVector );
		}
	}

	return ( slSerializedShapeHeader* )data;
}

slShape *slDeserializeShape( slSerializedShapeHeader *header, int length ) {
	slShape *s;
	slSerializedFaceHeader *face;
	char *p;
	int n, vind;
	slVector *v, *vectors, **vectorp;

	if ( header->type == ST_SPHERE ) return new slSphere( header->radius, header->mass );

	s = new slShape();

	// offset the pointer to the endge of the header

	p = ( char* )header + sizeof( slSerializedShapeHeader );

	for ( n = 0;n < header->faceCount;n++ ) {
		face = ( slSerializedFaceHeader* )p;

		vectors = new slVector[face->vertexCount];
		vectorp = new slVector*[face->vertexCount];

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

	slFinishShape( s, header->density );

	return s;
}

void slSphere::bounds( const slPosition *position, slVector *min, slVector *max ) const {
	max->x = position->location.x + ( _radius );
	max->y = position->location.y + ( _radius );
	max->z = position->location.z + ( _radius );

	min->x = position->location.x - ( _radius );
	min->y = position->location.y - ( _radius );
	min->z = position->location.z - ( _radius );

}

void slShape::bounds( const slPosition *position, slVector *min, slVector *max ) const {
	std::vector< slPoint* >::const_iterator pi;

	slVectorSet( max, INT_MIN, INT_MIN, INT_MIN );
	slVectorSet( min, INT_MAX, INT_MAX, INT_MAX );

	for ( pi = points.begin(); pi != points.end(); pi++ ) {
		slPoint *p = *pi;
		slVector loc;

		slVectorXform( position->rotation, &p->vertex, &loc );
		slVectorAdd( &loc, &position->location, &loc );

		if ( loc.x > max->x ) max->x = loc.x;
		if ( loc.y > max->y ) max->y = loc.y;
		if ( loc.z > max->z ) max->z = loc.z;

		if ( loc.x < min->x ) min->x = loc.x;
		if ( loc.y < min->y ) min->y = loc.y;
		if ( loc.z < min->z ) min->z = loc.z;
	}
}

double slShape::getMass() {
	return _mass;
}

double slShape::getDensity() {
	return _density;
}

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

	if( _odeGeomID[ 0 ] )
		dGeomDestroy( _odeGeomID[ 0 ] );
	if( _odeGeomID[ 1 ] ) 
		dGeomDestroy( _odeGeomID[ 1 ] );

	dTriMeshDataID triMeshID = dGeomTriMeshDataCreate();
	
	dGeomTriMeshDataBuildSingle( triMeshID, 
		_vertices, 3 * sizeof( float ), _vertexCount,
		_indices, _indexCount, 3 * sizeof( int ) );

  	_odeGeomID[ 0 ] = dCreateTriMesh( 0, triMeshID, 0, 0, 0);
  	_odeGeomID[ 1 ] = dCreateTriMesh( 0, triMeshID, 0, 0, 0);

//	_odeGeomID[ 1 ] = _odeGeomID[ 0 ];

	dGeomSetData( _odeGeomID[ 0 ], triMeshID );
	dGeomSetData( _odeGeomID[ 1 ], triMeshID );

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


void slMeshShape::bounds( const slPosition *position, slVector *min, slVector *max ) const {
	/*
	if( points.size() > 0 ) {
		slShape::bounds( position, min, max );


		return;
	}
	*/

	max->x = position->location.x + _maxReach;
	max->y = position->location.y + _maxReach;
	max->z = position->location.z + _maxReach;

	min->x = position->location.x - _maxReach;
	min->y = position->location.y - _maxReach;
	min->z = position->location.z - _maxReach;
}




slMeshShape::~slMeshShape() {
	if( _vertices )
		delete[] _vertices;

	if( _indices )
		delete[] _indices;
}


slVector *slPositionVertex( const slPosition *p, const slVector *v, slVector *o ) {
	slVectorXform( p->rotation, v, o );
	slVectorAdd( &p->location, o, o );

	return o;
}
