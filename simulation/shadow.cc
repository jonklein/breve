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

int slShadowVolumeForShape(slCamera *c, slShape *s, slPosition *p) {
	std::vector<slEdge*>::iterator ei;
	slVector light;
	slVector lNormal;

	if(s->type == ST_SPHERE) return slShadowVolumeForSphere(c, s, p);

	slVectorCopy(&c->lights[0].location, &light);
	slVectorCopy(&light, &lNormal);
	slVectorNormalize(&lNormal);
	slVectorMul(&light, 5, &light);

	glBegin(GL_QUADS);

	for(ei = s->edges.begin(); ei != s->edges.end(); ei++ ) {
		slEdge *e = *ei;
		double d1, d2;
		slFace *f1, *f2;
		slVector n1, n2;
		slVector tv;

		f1 = e->faces[0];
		f2 = e->faces[1];

		// look at this edge's faces, and calculate the dot product
		// with the light's vector.

		slVectorXform(p->rotation, &f1->plane.normal, &n1);
		slVectorXform(p->rotation, &f2->plane.normal, &n2);

		d1 = slVectorDot(&n1, &lNormal);
		d2 = slVectorDot(&n2, &lNormal);

		// are we at a critical edge, in which one face is facing
		// towards the light, and the other is facing away?

		if(d1 * d2 < 0.0 || (d1 * d2 == 0.0 && (d1 + d2) > 0.0)) {
			slVector *v, ts, te, sBottom, eBottom;
			slFace *topFace;
			int n;
			int flip = 0;

			if(d1 > 0.0) topFace = f1;
			else topFace = f2;

			for(n=0;n<topFace->edgeCount;n++) {
				if(topFace->neighbors[n] == e) {
					if(e->neighbors[0] == topFace->points[n]) flip = 1;
					n = topFace->edgeCount;
				}
			}

			if(!flip) {
				slVector tv;

				v = &((slPoint*)e->neighbors[0])->vertex;
				slVectorMul(v, 1.01, &tv);
				slPositionVertex(p, &tv, &ts);
				v = &((slPoint*)e->neighbors[1])->vertex;
				slVectorMul(v, 1.01, &tv);
				slPositionVertex(p, &tv, &te);
			} else {
				v = &((slPoint*)e->neighbors[1])->vertex;
				slVectorMul(v, 1.01, &tv);
				slPositionVertex(p, &tv, &ts);
				v = &((slPoint*)e->neighbors[0])->vertex;
				slVectorMul(v, 1.01, &tv);
				slPositionVertex(p, &tv, &te);
			}

			slVectorSub(&ts, &light, &sBottom);
			slVectorSub(&te, &light, &eBottom);

			glVertex3f(te.x, te.y, te.z);
			glVertex3f(eBottom.x, eBottom.y, eBottom.z);
			glVertex3f(sBottom.x, sBottom.y, sBottom.z);
			glVertex3f(ts.x, ts.y, ts.z);
		}
	}

	glEnd();

	return 0;
}

int slShadowVolumeForSphere(slCamera *c, slShape *s, slPosition *p) {
	slVector light, lNormal, x1, x2, lastV;
	int n, first = 1;
	double diff;
	int divisions;

	slVectorCopy(&c->lights[0].location, &light);

	slVectorCopy(&light, &lNormal);
	slVectorNormalize(&lNormal);
	slVectorMul(&light, 5, &light);

	// we want two vectors perpendicular to lNormal
	// make a phony vector, find  phony x light 

	if(lNormal.x != lNormal.y) slVectorSet(&x2, lNormal.y, lNormal.x, lNormal.z);
	else slVectorSet(&x2, lNormal.x, lNormal.z, lNormal.y);

	slVectorCross(&lNormal, &x2, &x1);
	slVectorCross(&lNormal, &x1, &x2);

	slVectorNormalize(&x1);
	slVectorNormalize(&x2);

	glBegin(GL_QUADS);

	divisions = (int)(s->radius * 5.0);

	if(divisions < MIN_SPHERE_VOLUME_DIVISIONS) divisions = MIN_SPHERE_VOLUME_DIVISIONS;
	else if(divisions > MAX_SPHERE_VOLUME_DIVISIONS) divisions = MAX_SPHERE_VOLUME_DIVISIONS;

	diff = 2.0*M_PI/(double)divisions;

	for(n=0;n<divisions + 1;n++) {
		slVector dx, dy, v, vBottom, lBottom;

		slVectorMul(&x1, s->radius * cos(n*diff) * 1.05, &dx);
		slVectorMul(&x2, s->radius * sin(n*diff) * 1.05, &dy);

		slVectorAdd(&p->location, &dx, &v);
		slVectorAdd(&v, &dy, &v);

		if(!first) {
			slVectorSub(&v, &light, &vBottom);
			slVectorSub(&lastV, &light, &lBottom);

			glVertex3f(lastV.x, lastV.y, lastV.z);
			glVertex3f(lBottom.x, lBottom.y, lBottom.z);
			glVertex3f(vBottom.x, vBottom.y, vBottom.z);
			glVertex3f(v.x, v.y, v.z);
		}

		first = 0;

		slVectorCopy(&v, &lastV);
	}

	glEnd();

	glBegin(GL_POLYGON);

	for(n=0;n<divisions + 1;n++) {
		slVector dx, dy, v;

		slVectorMul(&x1, s->radius * cos(n*diff) * 1.05, &dx);
		slVectorMul(&x2, s->radius * sin(n*diff) * 1.05, &dy);

		slVectorAdd(&p->location, &dx, &v);
		slVectorAdd(&v, &dy, &v);

		glVertex3f(v.x, v.y, v.z);
	}

	glEnd();

	return 0;
}

void slRenderShadowVolume(slWorld *w, slCamera *c) {
	glClear(GL_STENCIL_BUFFER_BIT);

	glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);
	glDepthMask(GL_FALSE);

	// glEnable(GL_POLYGON_OFFSET_FILL);

	glEnable(GL_STENCIL_TEST);
	glStencilFunc(GL_ALWAYS, 1, 0xffffffff);
	glStencilOp(GL_KEEP, GL_KEEP, GL_INCR);

	// stencil up shadow volume front faces to 1 
	slRenderObjects(w, c, 0, DO_NO_LIGHTING|DO_SHADOW_VOLUME|DO_NO_TERRAIN|DO_NO_REFLECT|DO_NO_STENCIL);
	
	// stencil down shadow volume back faces to 0

	glStencilOp(GL_KEEP, GL_KEEP, GL_DECR);
	glCullFace(GL_FRONT);
	slRenderObjects(w, c, 0, DO_NO_LIGHTING|DO_SHADOW_VOLUME|DO_NO_TERRAIN|DO_NO_REFLECT|DO_NO_STENCIL);

	// glClear(GL_DEPTH_BUFFER_BIT);

	slDrawLights(c, 0);

	glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
	glDepthMask(GL_TRUE);
	glCullFace(GL_BACK);
	glDepthFunc(GL_LEQUAL);
	glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);
	
	glStencilFunc(GL_EQUAL, 0, 0xffffffff);

	/* draw the scene again, with lighting, only where the value is 0 */
	
	slRenderObjects(w, c, 0, DO_NO_ALPHA);

	// transparent objects cause problems, since they cannot simply 
	// be "drawn over" the way we do with the rest of the scene.  
	// once we've drawn it the first time, it's there to stay.
	// what I think we should do here:
	// 1) do not render the alphas at all for the first pass
	// 2) render the unlit alphas where the stencil != 0 

	slRenderObjects(w, c, 0, DO_ONLY_ALPHA);
	if(c->billboardCount) slRenderBillboards(c, 0);

	glStencilFunc(GL_NOTEQUAL, 0, 0xffffffff);
	if(c->billboardCount) slRenderBillboards(c, 0);
	
	slDrawLights(c, 1);
	slRenderObjects(w, c, 0, DO_ONLY_ALPHA);

	glDisable(GL_STENCIL_TEST);
	glDisable(GL_LIGHTING);
}
