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

slShape *slNewShape() {
	slShape *s;

	s = slMalloc(sizeof(slShape));
	s->featureCount = 0;
	s->drawList = 0;
	s->type = ST_NORMAL;

	s->referenceCount = 1;

	s->maxFeatures = 8;
	s->features = slMalloc(s->maxFeatures * sizeof(slFeature*)); 

	slVectorSet(&s->max, 0, 0, 0);

	return s;
}

void slNextFeature(slShape *s) {
	if(s->featureCount == s->maxFeatures) {
		s->maxFeatures *= 2;
		s->features = slRealloc(s->features, s->maxFeatures * sizeof(slFeature*)); 
	}
}

slShape *slNewSphere(double radius, double mass) {
	slShape *s;

	s = slNewShape();

	s->referenceCount = 1;

	s->type = ST_SPHERE;
	s->radius = radius;

	s->mass = mass;

	slVectorSet(&s->max, radius, radius, radius);

	slSphereInertiaMatrix(radius, mass, s->inertia);

	return s;
}

slShape *slNewCube(slVector *size, double density) {
	slShape *s;

	s = slNewShape();

	if(!slSetCube(s, size, density)) {
		slFreeShape(s);
		return NULL;
	}

	return s;
}

slShape *slNewNGonDisc(int count, double radius, double height, double density) {
	slShape *s;

	s = slNewShape();

	if(!slSetNGonDisc(s, count, radius, height, density)) {
		slFreeShape(s);
		return NULL;
	}

	return s;
}

slShape *slNewNGonCone(int count, double radius, double height, double density) {
	slShape *s;

	s = slNewShape();

	if(!slSetNGonCone(s, count, radius, height, density)) {
		slFreeShape(s);
		return NULL;
	}

	return s;
}

/*!
	\brief Decrement a shapes reference count, freeing if the count reaches 0.
*/

void slFreeShape(slShape *s) {
	int n;

	if(--s->referenceCount) return;

	for(n=0;n<s->featureCount;n++) slFreeFeature(s->features[n]);

	if(s->drawList) glDeleteLists(s->drawList, 1);

	slFree(s->features);

	slFree(s);
}

slShape *slInitNeighbors(slShape *s, double density) {
	int n, m, o, faceCount;
	slPoint *p, *start, *end;
	slEdge *e;
	slFeature *f;
	slFace *face;
	slVector normal;
	int edges = 0, en;

	s->density = density;

	for(n=0;n<s->featureCount;n++) {
		if(s->features[n]->type == FT_POINT) {
			p = s->features[n]->data;

			p->neighbors = slMalloc(sizeof(slFeature*) * p->edgeCount);
			p->faces = slMalloc(sizeof(slFeature*) * p->edgeCount);
			p->voronoi = slMalloc(sizeof(slPlane) * p->edgeCount);

			if(!p->neighbors) {
				slFreeShape(s);
				return NULL;
			}

			/* now check to see if it's a maximum for the shape */

			if(p->vertex.x > s->max.x) s->max.x = p->vertex.x;
			if(p->vertex.y > s->max.y) s->max.y = p->vertex.y;
			if(p->vertex.z > s->max.z) s->max.z = p->vertex.z;

			/* reset edgeCount to 0 so it can act as a counter as the 
			   actual edges are added below */
		
			p->edgeCount = 0;
		}
	}

	/* for each edge, add the edge as a neighbor to the point on each side */

	for(n=0;n<s->featureCount;n++) {
		if(s->features[n]->type == FT_EDGE) {
			f = s->features[n];
			e = f->data;
			
			/* left point... */			
			
			start = e->neighbors[0]->data;
			end = e->neighbors[1]->data;
			
			start->neighbors[start->edgeCount] = f;
			end->neighbors[end->edgeCount] = f;

			slVectorSub(&start->vertex, &end->vertex, &normal);
			slVectorNormalize(&normal);
			slSetPlane(&start->voronoi[start->edgeCount], &normal, &start->vertex);
			
			slVectorSub(&end->vertex, &start->vertex, &normal);
			slVectorNormalize(&normal);
			slSetPlane(&end->voronoi[end->edgeCount], &normal, &end->vertex);
			
			start->edgeCount++;
			end->edgeCount++;

			edges++;

			for(en=0;en<4;en++) {
				if(!e->neighbors[en]) {
					slMessage(DEBUG_ALL, "error initializing shape neighbors: edge %p is missing adjacent features\n", e);
					return NULL;
				}
			}
		}
	}

	// Sort so that the points are all the way to the left.
	// We use this when computing the bounding box.

	qsort(s->features, s->featureCount, sizeof(slFeature*), slFeatureSort);

	/* add the face neighbors for the all the faces */

	for(n=0;n<s->featureCount;n++) {
		if(s->features[n]->type == FT_FACE) {
			face = s->features[n]->data;

			for(m=0;m<face->edgeCount;m++) {
				slVector *eStart, *eFinish, edgeVector, normal;

				/* grab the edge its vertices */

				e = face->neighbors[m]->data;
				eStart = &((slPoint*)e->neighbors[0]->data)->vertex;
				eFinish = &((slPoint*)e->neighbors[1]->data)->vertex;

				if(e->neighbors[2]->data != face) face->faces[m] = e->neighbors[2];
				else face->faces[m] = e->neighbors[3];

				/* set the voronoi plane for this edge */

				slVectorSub(eStart, eFinish, &edgeVector);
				slVectorCross(&face->plane.normal, &edgeVector, &normal);

				if(slVectorDot(&normal, eStart) > 0.0) slVectorMul(&normal, -1, &normal);

				slSetPlane(&face->voronoi[m], &normal, eFinish);
			}

			face->faces[face->edgeCount] = s->features[n];
		}

	}
 
	/* find the first point in the feature list */

	for(n=0;n<s->featureCount;n++) {
		if(s->features[n]->type == FT_POINT) {
			s->firstPoint = n;
			n = s->featureCount;
		}
	}

	/* update the neighbors for the points */

	for(n=0;n<s->featureCount;n++) {
		if(s->features[n]->type == FT_POINT) {
			p = s->features[n]->data;

			faceCount = 0;

			for(m=0;m<p->edgeCount;m++) {
				int found2 = 0, found3 = 0;
				e = p->neighbors[m]->data;

				for(o=0;o<faceCount;o++) if(p->faces[o] == e->neighbors[2]) found2 = 1;
				for(o=0;o<faceCount;o++) if(p->faces[o] == e->neighbors[3]) found3 = 1;

				if(!found2) p->faces[faceCount++] = e->neighbors[2];
				if(!found3) p->faces[faceCount++] = e->neighbors[3];
			}
		}
	}

	if(slSetMassProperties(s, density)) return NULL;

	/*
	if(s->mass < 0.1) {
		density = 0.1 / (s->mass/density);
		slMessage(DEBUG_ALL, "shape mass < 0.1, resetting density to %f\n", s->mass, density);

		if(slSetMassProperties(s, density)) return NULL;
	}
	*/

	return s;
}

/*!
	\brief Sets the mass of the shape.

	Using the shape's volume, sets the density of the shape so that 
	the desired mass is achived.
*/

void slShapeSetMass(slShape *shape, double mass) {
	double volume;

	/* the existing volume... */

	volume = shape->mass / shape->density;

	/* the desired density... */

	shape->density = mass / volume;

	slShapeSetDensity(shape, shape->density);
}

/*!
	\brief Sets the density of the shape.

	Sets the density of the change, which modifies the mass.
*/

void slShapeSetDensity(slShape *shape, double density) {
	slSetMassProperties(shape, density);
}

/*! 
  \brief Adds a face to a shape.  

	nPoints indicates how many points are on the given slFace.
	The last point given is assumed to connect to the first point.
*/

slFeature *slAddFace(slShape *s, slVector **points, int nPoints) {
	int n;
	slVector x, y, origin;
	slFace *f;
	slFeature *thisFeature;
	slVector **rpoints = NULL;
   
	if(nPoints < 3) return NULL;

	f = slMalloc(sizeof(slFace));
	if(!f) return NULL;
   
	f->edgeCount = 0;
	f->neighbors = slMalloc(sizeof(slEdge*) * nPoints);
	f->voronoi = slMalloc(sizeof(slPlane) * nPoints);
	f->points = slMalloc(sizeof(slFeature*) * nPoints);
	f->faces = slMalloc(sizeof(slFeature*) * (nPoints + 1));

	f->drawFlags = 0;

	thisFeature = slNewFeature(FT_FACE, f);

	if(!f->neighbors) {
		slFree(f);
		return NULL;
	}

	slVectorSub(points[1], points[0], &x); 
	slVectorSub(points[2], points[1], &y); 
	slVectorCross(&x, &y, &f->plane.normal);

	slVectorNormalize(&f->plane.normal);

	slVectorCopy(points[1], &f->plane.vertex);

	slVectorSet(&origin, 0, 0, 0);

	/* uhoh, screwy plane, reverse-o! */

	if(slPlaneDistance(&f->plane, &origin) > 0.0) {
		slVectorMul(&f->plane.normal, -1, &f->plane.normal);

		rpoints = slMalloc(sizeof(slVector*) * nPoints);

		for(n=0;n<nPoints;n++) rpoints[n] = points[(nPoints - 1) - n];
		for(n=0;n<nPoints;n++) points[n] = rpoints[n];
	
		slFree(rpoints);

		slVectorCopy(points[1], &f->plane.vertex);
	}

	for(n=0;n<nPoints;n++) f->points[n] = slAddPoint(s, points[n]);

	f->edgeCount = nPoints;

	for(n=0;n<nPoints - 1;n++) { 
		f->neighbors[n] = slAddEdge(s, thisFeature, points[n], points[n + 1]);
				
		/* z is normal to the voronoi slPlane--that means it's parallel to the slFace */
		/* of the slPlane and perpendicular to the edge we're looking at			*/
	
		slVectorSub(points[n + 1], points[n], &x);
		slVectorCross(&f->plane.normal, &x, &y);

		slVectorNormalize(&y);
		slSetPlane(&f->voronoi[n], &y, points[n]); 
	}

	f->neighbors[n] = slAddEdge(s, thisFeature, points[n], points[0]);

	slVectorSub(points[0], points[nPoints - 1], &x);
	slVectorCross(&f->plane.normal, &x, &y);

	slVectorNormalize(&y);
	slSetPlane(&f->voronoi[nPoints - 1], &y, points[nPoints - 1]); 

	slNextFeature(s);

	s->features[s->featureCount] = thisFeature;

	return s->features[s->featureCount++];
}

/*!
	\brief Add an edge to a shape.

	The points of the edge should always be passed in moving clockwise 
	around the face of the plane, where the face of the clock is pointing 
	towards the normal (the "top" of the slPlane).
*/

slFeature *slAddEdge(slShape *s, slFeature *theFace, slVector *start, slVector *end) {
	slEdge *e, *originalEdge = NULL;
	slFeature *pf1, *pf2; // point slFeatures 1 and 2...
	slPoint *p1, *p2; // and the points they slFeature.  haw haw haw
	slVector lineSegment, vNorm;
	int n, sameEdge;
	slFace *f;
	
	if(theFace->type != FT_FACE) return NULL;
	
	f = theFace->data;
 
	pf1 = slAddPoint(s, start);
	pf2 = slAddPoint(s, end);

	p1 = pf1->data;
	p2 = pf2->data;
	
	slVectorSub(start, end, &lineSegment);
	slVectorCross(&f->plane.normal, &lineSegment, &vNorm); 

	/* see if the point already exists */

	for(n=0;n<s->featureCount;n++) {
		if(s->features[n]->type == FT_EDGE) {
			e = (slEdge*)s->features[n]->data;

			/* if we've been here before, we're seeing the second slFace */

			sameEdge = (e->neighbors[0]->data == p1 && e->neighbors[1]->data == p2);

			if(sameEdge || (e->neighbors[1]->data == p1 && e->neighbors[0]->data == p2)) {
				originalEdge = e;

				/* the first slFace is already there... */
				   
				originalEdge->neighbors[3] = theFace;
				   
				/* and the other 3 voroni slPlanes have been added */
				   
				slVectorNormalize(&vNorm);
				slSetPlane(&originalEdge->voronoi[3], &vNorm, start);

				return s->features[n];
			}
		}
	}

	e = slMalloc(sizeof(slEdge));

	/* if this is the mirror of another edge, copy in the neighbors */
	/* and voronoi slPlanes, bearing in mind that the order of the	*/
	/* neighboring points is reversed */

	if(originalEdge) {
		e->neighbors[1] = originalEdge->neighbors[0];
		e->neighbors[0] = originalEdge->neighbors[1];
		e->neighbors[2] = originalEdge->neighbors[2];
		e->neighbors[3] = originalEdge->neighbors[3];

		bcopy(&originalEdge->voronoi[0], &e->voronoi[1], sizeof(slPlane));
		bcopy(&originalEdge->voronoi[1], &e->voronoi[0], sizeof(slPlane));
		bcopy(&originalEdge->voronoi[2], &e->voronoi[2], sizeof(slPlane));
		bcopy(&originalEdge->voronoi[3], &e->voronoi[3], sizeof(slPlane));
	} else {
		e->neighbors[0] = pf1;
		e->neighbors[1] = pf2;
		e->neighbors[2] = theFace;

		/* first add the voronois for the ends */
   
		slVectorNormalize(&lineSegment);
		slSetPlane(&e->voronoi[1], &lineSegment, end);

		slVectorMul(&lineSegment, -1, &lineSegment);
		slSetPlane(&e->voronoi[0], &lineSegment, start);

		/* and now for the first slFace */

		/* how are we assured that this is traveling away from the slFace? */

		slVectorNormalize(&vNorm);
		slSetPlane(&e->voronoi[2], &vNorm, start);

		/* the final slFace slPlane is added later */
	}

	slNextFeature(s);
	
	s->features[s->featureCount] = slNewFeature(FT_EDGE, e);
	return s->features[s->featureCount++];
}

/*!
	\brief Adds a point to a shape.

	Called as part of slAddEdge and slAddFace, not typically called manually.
*/

slFeature *slAddPoint(slShape *s, slVector *vertex) {
	slPoint *p;
	int n;

	/* see if the point already exists */

	for(n=0;n<s->featureCount;n++) {
		if(s->features[n]->type == FT_POINT) {
			p = s->features[n]->data;

			if(!slVectorCompare(vertex, &p->vertex)) { 
				p->edgeCount++;
				return s->features[n];
			}			
		}
	}

	p = slMalloc(sizeof(slPoint));
	if(!p) return NULL;

	slVectorCopy(vertex, &p->vertex);
	p->edgeCount = 1;
	p->neighbors = NULL;

	slNextFeature(s);

	s->features[s->featureCount] = slNewFeature(FT_POINT, p);
	return s->features[s->featureCount++];
}

/*!
	\brief Fills in an slPlane struct.
*/

slPlane *slSetPlane(slPlane *p, slVector *normal, slVector *vertex) {   
	slVectorCopy(normal, &p->normal);
	slVectorCopy(vertex, &p->vertex); 
	slVectorNormalize(&p->normal);
	
	return p; 
}

/*!
	\brief Allocates an slFeature struct.
*/

slFeature *slNewFeature(int type, void *data) {
	slFeature *f;
	
	f = slMalloc(sizeof(slFeature));
	if(!f) return NULL;

	f->type = type;
	f->data = data;

	return f;
}

/*!
	\brief Frees an slFeature.
*/

void slFreeFeature(slFeature *f) {
	switch(f->type) {
		case FT_POINT:
			slFree(((slPoint*)f->data)->neighbors);
			slFree(((slPoint*)f->data)->voronoi);
			slFree(((slPoint*)f->data)->faces);
			break;
		case FT_EDGE:
			break;
		case FT_FACE:
			slFree(((slFace*)f->data)->neighbors);
			slFree(((slFace*)f->data)->voronoi);
			slFree(((slFace*)f->data)->faces);
			slFree(((slFace*)f->data)->points);
			break;
	}

	slFree(f->data);

	slFree(f);
}

/*!
	\brief Sets a shape to be a rectangular solid.
*/

slShape *slSetCube(slShape *s, slVector *size, double density) {
	slVector *face[4];
	slVector h;

	/* x+ slFace:  x- slFace:
		p1 - p2	 
				 p5 - p6

		p3 - p4	 
				 p7 - p8
	*/

	slVector p1, p2, p3, p4, p5, p6, p7, p8;

	slVectorMul(size, .5, &h);

	slVectorSet(&p1, h.x, h.y, h.z);
	slVectorSet(&p2, h.x, -h.y, h.z);
	slVectorSet(&p3, h.x, h.y, -h.z);
	slVectorSet(&p4, h.x, -h.y, -h.z);

	slVectorSet(&p5, -h.x, h.y, h.z);
	slVectorSet(&p6, -h.x, -h.y, h.z);
	slVectorSet(&p7, -h.x, h.y, -h.z);
	slVectorSet(&p8, -h.x, -h.y, -h.z);

	face[0] = &p1;
	face[1] = &p2;
	face[2] = &p4;
	face[3] = &p3;

	slAddFace(s, face, 4);

	face[0] = &p5;
	face[1] = &p6;
	face[2] = &p2;
	face[3] = &p1;

	slAddFace(s, face, 4);

	face[0] = &p7;
	face[1] = &p8;
	face[2] = &p6;
	face[3] = &p5;

	slAddFace(s, face, 4);

	face[0] = &p3;
	face[1] = &p4;
	face[2] = &p8;
	face[3] = &p7;

	slAddFace(s, face, 4);

	face[0] = &p1;
	face[1] = &p3;
	face[2] = &p7;
	face[3] = &p5;

	slAddFace(s, face, 4);

	face[0] = &p2;
	face[1] = &p6;
	face[2] = &p8;
	face[3] = &p4;
 
	slAddFace(s, face, 4);

	return slInitNeighbors(s, density);
}

/*!
	\brief Sets a shape to be an extruded polygon.
*/

slShape *slSetNGonDisc(slShape *s, int sideCount, double radius, double height, double density) {
	int n;
	slVector *tops, *bottoms, sides[4];
	slVector **topP, **bottomP, *sideP[4];

	if(sideCount < 3) return NULL;

	tops = slMalloc(sizeof(slVector) * (sideCount + 1));
	bottoms = slMalloc(sizeof(slVector) * (sideCount + 1));
	topP = slMalloc(sizeof(slVector*) * (sideCount + 1));
	bottomP = slMalloc(sizeof(slVector*) * (sideCount + 1));

	for(n=0;n<sideCount;n++) {
		topP[sideCount - (n + 1)] = &tops[n];
		bottomP[n] = &bottoms[n];

		tops[n].y = (height / 2);
		bottoms[n].y = -(height / 2);

		tops[n].x = radius * cos(n * (2*M_PI/sideCount));
		tops[n].z = radius * sin(n * (2*M_PI/sideCount));

		bottoms[n].x = radius * cos(n * (2*M_PI/sideCount));
		bottoms[n].z = radius * sin(n * (2*M_PI/sideCount));
	}

	slVectorCopy(&tops[0], &tops[n]);
	slVectorCopy(&bottoms[0], &bottoms[n]);

	slAddFace(s, topP, sideCount);
	slAddFace(s, bottomP, sideCount);

	sideP[0] = &sides[0];
	sideP[1] = &sides[1];
	sideP[2] = &sides[2];
	sideP[3] = &sides[3];

	for(n=0;n<sideCount;n++) {
		slVectorCopy(&tops[n], &sides[3]);
		slVectorCopy(&bottoms[n], &sides[2]);
		slVectorCopy(&bottoms[n + 1], &sides[1]);
		slVectorCopy(&tops[n + 1], &sides[0]);

		slAddFace(s, sideP, 4);
	}

	slFree(tops);
	slFree(bottoms);
	slFree(topP);
	slFree(bottomP);

	return slInitNeighbors(s, density);
}

/*!
	\brief Sets a shape to be a cone with a polygon base.
*/

slShape *slSetNGonCone(slShape *s, int sideCount, double radius, double height, double density) {
	int n;
	slVector top;
	slVector *bottoms, sides[3];
	slVector **bottomP, *sideP[3];

	if(sideCount < 3) return NULL;

	bottoms = slMalloc(sizeof(slVector) * (sideCount + 1));
	bottomP = slMalloc(sizeof(slVector*) * (sideCount + 1));

	slVectorSet(&top, 0, height, 0);

	for(n=0;n<sideCount;n++) {
		bottomP[n] = &bottoms[n];

		bottoms[n].y = -(height / 2);

		bottoms[n].x = radius * cos(n * (2*M_PI/sideCount));
		bottoms[n].z = radius * sin(n * (2*M_PI/sideCount));
	}

	slVectorCopy(&bottoms[0], &bottoms[n]);

	slAddFace(s, bottomP, sideCount);

	sideP[0] = &sides[0];
	sideP[1] = &sides[1];
	sideP[2] = &sides[2];

	for(n=0;n<sideCount;n++) {
		slVectorCopy(&bottoms[n], &sides[2]);
		slVectorCopy(&bottoms[n + 1], &sides[1]);
		slVectorCopy(&top, &sides[0]);

		slAddFace(s, sideP, 3);
	}

	slFree(bottoms);
	slFree(bottomP);

	return slInitNeighbors(s, density);
}

/*!
	\brief qsort callback to sort the points all the way to the left.

	Used when computing the shape's bounding box.
*/

int slFeatureSort(const void *a, const void *b) {
	slFeature *fa, *fb;

	fa = *(slFeature**)a;
	fb = *(slFeature**)b;

	if(fa->type == FT_POINT) return -1;
	if(fb->type == FT_POINT) return 1;

	return 0;
}

void slCubeInertiaMatrix(slVector *c, double mass, double i[3][3]) {
	slMatrixZero(i);

	i[0][0] = 1.0/12.0 * mass * (c->y*c->y + c->z * c->z);
	i[1][1] = 1.0/12.0 * mass * (c->x*c->x + c->z * c->z);
	i[2][2] = 1.0/12.0 * mass * (c->x*c->x + c->y * c->y);
}

void slSphereInertiaMatrix(double r, double mass, double i[3][3]) {
	slVector q;

	q.x = q.y = q.z = r;

	slCubeInertiaMatrix(&q, mass, i);
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

int slPointOnShape(slShape *s, slVector *dir, slVector *point) {
	int n;
	double D, X, Y, Z, k;
	slVector pointOnPlane;
	slPosition pos;
	int update, result, planes = 0;
	double distance;

	slVectorSet(point, 0.0, 0.0, 0.0);

	slVectorSet(&pos.location, 0.0, 0.0, 0.0);
	slMatrixIdentity(pos.rotation);

	slVectorNormalize(dir);

	if(s->type == ST_SPHERE) {
		slVectorMul(dir, s->radius, point);
		return 0;
	} 

	for(n=0;n<s->featureCount;n++) {
		/* go through all of the faces */

		if(s->features[n]->type == FT_FACE) {
			slFace *f = s->features[n]->data;
			planes++;

			D = slVectorDot(&f->plane.normal, &f->plane.vertex);

			// printf("d = %f\n", D);

			X = f->plane.normal.x * dir->x;
			Y = f->plane.normal.y * dir->y;
			Z = f->plane.normal.z * dir->z;

			k = D/(X+Y+Z);

			// printf("%p: k = %f [%d]\n", s->features[n], k, (!slIsinf(k) && k > 0.0));

			if(!slIsinf(k) && k > 0.0) {
				/* we have the length of the matching vector on the plane of this */
				/* face. */

				slVectorMul(dir, k, &pointOnPlane);

				// distance = slPlaneDistance(&f->plane, &pointOnPlane);

				/* now figure out if the point in question is within the face */

				result = slClipPoint(&pointOnPlane, f->voronoi, &pos, f->edgeCount, &update, &distance);

				/* if this point is within the voronoi region of the plane, it must be */
				/* on the face */

				if(result == 1) {
					slVectorCopy(&pointOnPlane, point);
					return n;
				} 
			}
		}
	}

	return -1;
}

void slScaleShape(slShape *s, slVector *scale) {
	slPoint *p;
	slFace *f;
	int n;

	slVector v;
	double m[3][3];

	slMatrixIdentity(m);
	m[0][0] = scale->x;
	m[1][1] = scale->y;
	m[2][2] = scale->z;

	s->recompile = 1;

	if(s->type == ST_SPHERE) {
		s->radius *= scale->x;
		slInitNeighbors(s, s->density);
		return;
	}

	for(n=0;n<s->featureCount;n++) {
		if(s->features[n]->type == FT_POINT) {
			p = s->features[n]->data;

			slFree(p->neighbors);
			slFree(p->faces);
			slFree(p->voronoi);

			slVectorCopy(&p->vertex, &v);
			slVectorXform(m, &v, &p->vertex);
		}

		if(s->features[n]->type == FT_FACE) {
			f = s->features[n]->data;
			slVectorCopy(&f->plane.vertex, &v);
			slVectorXform(m, &v, &f->plane.vertex);
		}
	}

	slInitNeighbors(s, s->density);
}

slSerializedShapeHeader *slSerializeShape(slShape *s, int *length) {
	int n, p, faceCount = 0, facePointCount = 0;
	slSerializedShapeHeader *header;
	slSerializedFaceHeader *fhead;
	slFace *face;
	char *data, *position;
	slVector *v;

	if(s->type == ST_SPHERE) { 
		*length = sizeof(slSerializedShapeHeader);
		header = slMalloc(*length);

		header->type = ST_SPHERE;
		header->radius = s->radius;
		header->mass = s->mass;
		header->density = s->density;

		return header;
	}

	/* count the different points and faces we have to add */

	for(n=0;n<s->featureCount;n++) {
		if(s->features[n]->type == FT_FACE) {
			face = s->features[n]->data;
			faceCount++;

			/* the number of edges on the face == number of points */

			facePointCount += face->edgeCount;
		}
	}

	/* figure out the total length of the data */

	*length = sizeof(slSerializedShapeHeader) + 					/* the header        */
              (faceCount * sizeof(slSerializedFaceHeader)) + 		/* the face headers  */
              (facePointCount * sizeof(slVector));					/* the face vertices */

	data = slMalloc(*length);
	header = (slSerializedShapeHeader*)data;

	/* fill in the header */

	header->faceCount = faceCount;
	header->type = s->type;
	slMatrixCopy(s->inertia, header->inertia);
	header->density = s->density;
	header->mass = s->mass;
	slVectorCopy(&s->max, &header->max);

	/* skip over the header */

	position = data + sizeof(slSerializedShapeHeader);

	for(n=0;n<s->featureCount;n++) {
		if(s->features[n]->type == FT_FACE) {
			face = s->features[n]->data;
			fhead = (slSerializedFaceHeader*)position;

			fhead->vertexCount = face->edgeCount;

			position += sizeof(slSerializedFaceHeader);

			for(p=0;p<face->edgeCount;p++) {
				v = (slVector*)position;

				slVectorCopy(&((slPoint*)face->points[p]->data)->vertex, v);
				position += sizeof(slVector);
			}
		}
	}

	return (slSerializedShapeHeader*)data;
}

slShape *slDeserializeShape(slSerializedShapeHeader *header, int length) {
	slShape *s;
	slSerializedFaceHeader *face;
	char *p;
	int n, vind;
	slVector *v, *vectors, **vectorp;

	if(header->type == ST_SPHERE) return slNewSphere(header->radius, header->mass);

	s = slNewShape();

	p = (void*)header + sizeof(slSerializedShapeHeader);

	for(n=0;n<header->faceCount;n++) {
		face = (slSerializedFaceHeader*)p;

		vectors = slMalloc(sizeof(slVector) * face->vertexCount);
		vectorp = slMalloc(sizeof(slVector*) * face->vertexCount);

		p += sizeof(slSerializedFaceHeader);

		for(vind=0;vind<face->vertexCount;vind++) {
			v = (slVector*)p;

			slVectorCopy(v, &vectors[vind]);

			vectorp[vind] = &vectors[vind];

			p += sizeof(slVector);
		}

		slAddFace(s, vectorp, face->vertexCount);

		slFree(vectors);
		slFree(vectorp);
	}

	slInitNeighbors(s, header->density);

	return s;
}

void slShapeBounds(slShape *shape, slPosition *position, slVector *min, slVector *max) {
	int n;

	slVectorSet(max, INT_MIN, INT_MIN, INT_MIN);
	slVectorSet(min, INT_MAX, INT_MAX, INT_MAX);

    if(shape->type == ST_SPHERE) {
        max->x = position->location.x + (shape->radius);
        max->y = position->location.y + (shape->radius);
        max->z = position->location.z + (shape->radius);

        min->x = position->location.x - (shape->radius);
        min->y = position->location.y - (shape->radius);
        min->z = position->location.z - (shape->radius);
    } else for(n=0;n<shape->featureCount;n++) {
        slFeature *f = shape->features[n];
		slVector loc;

        if(f->type == FT_POINT) {
            slVectorXform(position->rotation, &((slPoint*)f->data)->vertex, &loc);
            slVectorAdd(&loc, &position->location, &loc);

            if(loc.x > max->x) max->x = loc.x;
            if(loc.y > max->y) max->y = loc.y;
            if(loc.z > max->z) max->z = loc.z;

            if(loc.x < min->x) min->x = loc.x;
            if(loc.y < min->y) min->y = loc.y;
            if(loc.z < min->z) min->z = loc.z;
        } else n = shape->featureCount;
    }
}
