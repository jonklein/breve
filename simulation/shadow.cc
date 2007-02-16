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
#include "world.h"
#include "camera.h"
#include "shadow.h"
#include "shape.h"

void slShape::drawShadowVolume( slCamera *c, slPosition *p ) {
	std::vector<slEdge*>::iterator ei;
	slVector light;
	slVector lNormal;

	slVectorCopy( &c->_lights[ 0 ].location, &light );
	slVectorCopy( &light, &lNormal );
	slVectorNormalize( &lNormal );
	slVectorMul( &light, 5, &light );

	glBegin( GL_QUADS );

	for ( ei = edges.begin(); ei != edges.end(); ei++ ) {
		slEdge *e = *ei;
		double d1, d2;
		slFace *f1, *f2;
		slVector n1, n2;
		slVector tv;

		f1 = e->faces[0];
		f2 = e->faces[1];

		// look at this edge's faces, and calculate the dot product
		// with the light's vector.

		slVectorXform( p->rotation, &f1->plane.normal, &n1 );
		slVectorXform( p->rotation, &f2->plane.normal, &n2 );

		d1 = slVectorDot( &n1, &lNormal );
		d2 = slVectorDot( &n2, &lNormal );

		// are we at a critical edge, in which one face is facing
		// towards the light, and the other is facing away?

		if ( d1 * d2 < 0.0 || ( d1 * d2 == 0.0 && ( d1 + d2 ) > 0.0 ) ) {
			slVector *v, ts, te, sBottom, eBottom;
			slFace *topFace;
			int n;
			int flip = 0;

			if ( d1 > 0.0 ) topFace = f1;
			else topFace = f2;

			for ( n = 0;n < topFace->edgeCount;n++ ) {
				if ( topFace->neighbors[n] == e ) {
					if ( e->neighbors[0] == topFace->points[n] ) flip = 1;

					n = topFace->edgeCount;
				}
			}

			if ( !flip ) {
				slVector tv;

				v = &(( slPoint* )e->neighbors[0] )->vertex;
				slVectorMul( v, 1.01, &tv );
				slPositionVertex( p, &tv, &ts );
				v = &(( slPoint* )e->neighbors[1] )->vertex;
				slVectorMul( v, 1.01, &tv );
				slPositionVertex( p, &tv, &te );
			} else {
				v = &(( slPoint* )e->neighbors[1] )->vertex;
				slVectorMul( v, 1.01, &tv );
				slPositionVertex( p, &tv, &ts );
				v = &(( slPoint* )e->neighbors[0] )->vertex;
				slVectorMul( v, 1.01, &tv );
				slPositionVertex( p, &tv, &te );
			}

			slVectorSub( &ts, &light, &sBottom );

			slVectorSub( &te, &light, &eBottom );

			glVertex3f( te.x, te.y, te.z );
			glVertex3f( eBottom.x, eBottom.y, eBottom.z );
			glVertex3f( sBottom.x, sBottom.y, sBottom.z );
			glVertex3f( ts.x, ts.y, ts.z );
		}
	}

	glEnd();
}

void slSphere::drawShadowVolume( slCamera *c, slPosition *p ) {
	slVector light, lNormal, x1, x2, lastV;
	int n, first = 1;
	double diff;
	int divisions;

	static int inited;

	static float sineTable[361], cosineTable[361];

	slVectorSub( &c->_lights[0].location, &p->location, &light );

	// slVectorCopy(&c->_lights[0].location, &light);

	slVectorCopy( &light, &lNormal );

	slVectorNormalize( &lNormal );

	slVectorMul( &light, 5, &light );

	// we want two vectors perpendicular to lNormal
	// make a phony vector, find  phony x light

	if ( lNormal.x != lNormal.y ) slVectorSet( &x2, lNormal.y, lNormal.x, lNormal.z );
	else slVectorSet( &x2, lNormal.x, lNormal.z, lNormal.y );

	slVectorCross( &lNormal, &x2, &x1 );

	slVectorCross( &lNormal, &x1, &x2 );

	slVectorNormalize( &x1 );

	slVectorNormalize( &x2 );

	glBegin( GL_QUADS );

	divisions = ( int )( _radius * 5.0 );

	if ( divisions < MIN_SPHERE_VOLUME_DIVISIONS ) divisions = MIN_SPHERE_VOLUME_DIVISIONS;
	else if ( divisions > MAX_SPHERE_VOLUME_DIVISIONS ) divisions = MAX_SPHERE_VOLUME_DIVISIONS;

	if ( !inited ) {
		double step = ( 2.0 * M_PI / 360.0 );
		inited = 1;

		for ( n = 0; n < 361;n++ ) {

			sineTable[ n] = sin( step * n ) * 1.05;
			cosineTable[ n] = cos( step * n ) * 1.05;
		}
	}

	diff = 2.0 * M_PI / ( double )divisions;

	int step = ( 360 / divisions );

	for ( n = 0;n < divisions + 1;n++ ) {
		slVector dx, dy, v, vBottom, lBottom;

		slVectorMul( &x1, _radius * cosineTable[ n * step], &dx );
		slVectorMul( &x2, _radius * sineTable[n * step], &dy );

		slVectorAdd( &p->location, &dx, &v );
		slVectorAdd( &v, &dy, &v );

		if ( !first ) {
			slVectorSub( &v, &light, &vBottom );
			slVectorSub( &lastV, &light, &lBottom );

			glVertex3f( lastV.x, lastV.y, lastV.z );
			glVertex3f( lBottom.x, lBottom.y, lBottom.z );
			glVertex3f( vBottom.x, vBottom.y, vBottom.z );
			glVertex3f( v.x, v.y, v.z );
		}

		first = 0;

		slVectorCopy( &v, &lastV );
	}

	glEnd();

	glBegin( GL_POLYGON );

	for ( n = 0;n < divisions + 1;n++ ) {
		slVector dx, dy, v;

		slVectorMul( &x1, _radius * cosineTable[ n * step], &dx );
		slVectorMul( &x2, _radius * sineTable[n * step], &dy );

		slVectorAdd( &p->location, &dx, &v );
		slVectorAdd( &v, &dy, &v );

		glVertex3f( v.x, v.y, v.z );
	}

	glEnd();
}

void slCamera::renderShadowVolume( slWorld *w ) {
	glClear( GL_STENCIL_BUFFER_BIT );

	glColorMask( GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE );
	glDepthMask( GL_FALSE );

	// glEnable(GL_POLYGON_OFFSET_FILL);

	glEnable( GL_STENCIL_TEST );
	glStencilFunc( GL_ALWAYS, 1, 0xffffffff );
	glStencilOp( GL_KEEP, GL_KEEP, GL_INCR );

	// stencil up shadow volume front faces to 1
	renderObjectShadowVolumes( w );

	// stencil down shadow volume back faces to 0

	glStencilOp( GL_KEEP, GL_KEEP, GL_DECR );
	glCullFace( GL_FRONT );
	renderObjectShadowVolumes( w );

	// glClear(GL_DEPTH_BUFFER_BIT);

	drawLights( 0 );

	glColorMask( GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE );
	glDepthMask( GL_TRUE );
	glCullFace( GL_BACK );
	glDepthFunc( GL_LEQUAL );
	glStencilOp( GL_KEEP, GL_KEEP, GL_KEEP );

	glStencilFunc( GL_EQUAL, 0, 0xffffffff );

	// draw the scene again, with lighting, only where the value is 0

	// glColor3f(1, 0, 0);
	renderObjects( w, DO_NO_ALPHA );

	// transparent objects cause problems, since they cannot simply
	// be "drawn over" the way we do with the rest of the scene.
	// once we've drawn it the first time, it's there to stay.
	// what I think we should do here:
	// 1) do not render the alphas at all for the first pass
	// 2) render the unlit alphas where the stencil != 0

	renderObjects( w, DO_ONLY_ALPHA );

	if ( _billboardCount ) renderBillboards( 0 );

	glStencilFunc( GL_NOTEQUAL, 0, 0xffffffff );

	if ( _billboardCount ) renderBillboards( 0 );

	drawLights( 1 );

	renderObjects( w, DO_ONLY_ALPHA );

	glDisable( GL_STENCIL_TEST );

	glDisable( GL_LIGHTING );
}

void slCamera::renderObjectShadowVolumes( slWorld *w ) {
	std::vector<slWorldObject*>::iterator wi;

	glDisable( GL_BLEND );
	glColor4f( 0, 0, 0, 1 );

	for ( wi = w->_objects.begin(); wi != w->_objects.end(); wi++ ) {
		if ( *wi && ( *wi )->_shape && !( *wi )->_drawAsPoint && ( *wi )->_drawShadow ) {
			( *wi )->_shape->drawShadowVolume( this, &( *wi )->_position );
		}
	}
}
