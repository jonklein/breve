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

/*!
	\brief Creates a new camera of a given size.
*/

slCamera *slCameraNew(int x, int y) {
	slCameraText text;
	slCameraText &t = text;

	slCamera *c;
	unsigned int n;

	c = new slCamera;
	memset(c, 0, sizeof(slCamera));

	c->text.insert(c->text.begin(), 8, t);

	c->textScale = 1;

	c->x = x;
	c->y = y;
	c->ox = 0;
	c->oy = 0;

	c->zClip = 500.0;

	c->fov = (double)x/(double)y;

	// billboarding works poorly when all the billboards 
	// are on the same plane, so we'll offset the camera 
	// just slightly.  enough so that the alpha bending 
	// works correctly, but not so that it's visable. 

	c->rx = 0.001;
	c->ry = 0.001;

	c->drawMode = GL_POLYGON;

	slVectorSet(&c->target, 0, 0, 0);

	slVectorSet(&c->location, 0, 10, 100);

	c->drawSmooth = 0;
	c->drawLights = 0;
	c->drawShadow = 0;
	c->drawOutline = 0;
	c->drawReflection = 0;
	c->drawText = 1;
	c->blur = 0;
	c->blurFactor = 0.1;

	c->billboardCount = 0;
	c->maxBillboards = 8;
	c->billboards = (slBillboardEntry**)slMalloc(sizeof(slBillboardEntry*) * c->maxBillboards);
	c->billboardDrawList = 0;

	c->fogIntensity = .1;
	c->fogStart = 10;	
	c->fogEnd = 40;	

	for(n=0;n<c->maxBillboards;n++) c->billboards[n] = new slBillboardEntry;

	c->nLights = 1;

	slVectorSet(&c->lights[0].location, 0, 0, 0);
	slVectorSet(&c->lights[0].ambient, .6, .6, .6);
	slVectorSet(&c->lights[0].diffuse, .6, .9, .9);

	return c;
}

void slCamera::updateFrustum() {
	slVector loc;
	float proj[16], frust[16], modl[16];

	glGetFloatv( GL_PROJECTION_MATRIX, proj );
	glGetFloatv( GL_MODELVIEW_MATRIX, modl );

	// combine the two matrices (multiply projection by modelview) 
	frust[ 0] = modl[ 0] * proj[ 0] + modl[ 1] * proj[ 4] + modl[ 2] * proj[ 8] + modl[ 3] * proj[12];
	frust[ 1] = modl[ 0] * proj[ 1] + modl[ 1] * proj[ 5] + modl[ 2] * proj[ 9] + modl[ 3] * proj[13];
	frust[ 2] = modl[ 0] * proj[ 2] + modl[ 1] * proj[ 6] + modl[ 2] * proj[10] + modl[ 3] * proj[14];
	frust[ 3] = modl[ 0] * proj[ 3] + modl[ 1] * proj[ 7] + modl[ 2] * proj[11] + modl[ 3] * proj[15];

	frust[ 4] = modl[ 4] * proj[ 0] + modl[ 5] * proj[ 4] + modl[ 6] * proj[ 8] + modl[ 7] * proj[12];
	frust[ 5] = modl[ 4] * proj[ 1] + modl[ 5] * proj[ 5] + modl[ 6] * proj[ 9] + modl[ 7] * proj[13];
	frust[ 6] = modl[ 4] * proj[ 2] + modl[ 5] * proj[ 6] + modl[ 6] * proj[10] + modl[ 7] * proj[14];
	frust[ 7] = modl[ 4] * proj[ 3] + modl[ 5] * proj[ 7] + modl[ 6] * proj[11] + modl[ 7] * proj[15];

	frust[ 8] = modl[ 8] * proj[ 0] + modl[ 9] * proj[ 4] + modl[10] * proj[ 8] + modl[11] * proj[12];
	frust[ 9] = modl[ 8] * proj[ 1] + modl[ 9] * proj[ 5] + modl[10] * proj[ 9] + modl[11] * proj[13];
	frust[10] = modl[ 8] * proj[ 2] + modl[ 9] * proj[ 6] + modl[10] * proj[10] + modl[11] * proj[14];
	frust[11] = modl[ 8] * proj[ 3] + modl[ 9] * proj[ 7] + modl[10] * proj[11] + modl[11] * proj[15];

	frust[12] = modl[12] * proj[ 0] + modl[13] * proj[ 4] + modl[14] * proj[ 8] + modl[15] * proj[12];
	frust[13] = modl[12] * proj[ 1] + modl[13] * proj[ 5] + modl[14] * proj[ 9] + modl[15] * proj[13];
	frust[14] = modl[12] * proj[ 2] + modl[13] * proj[ 6] + modl[14] * proj[10] + modl[15] * proj[14];
	frust[15] = modl[12] * proj[ 3] + modl[13] * proj[ 7] + modl[14] * proj[11] + modl[15] * proj[15];

	slVectorAdd(&target, &location, &loc);
	slVectorCopy(&loc, &frustumPlanes[0].vertex);
	slVectorCopy(&loc, &frustumPlanes[1].vertex);
	slVectorCopy(&loc, &frustumPlanes[2].vertex);
	slVectorCopy(&loc, &frustumPlanes[3].vertex);

	frustumPlanes[0].normal.x = frust[3 ] - frust[0];
	frustumPlanes[0].normal.y = frust[7 ] - frust[4];
	frustumPlanes[0].normal.z = frust[11] - frust[8];

	frustumPlanes[1].normal.x = frust[3 ] + frust[0];
	frustumPlanes[1].normal.y = frust[7 ] + frust[4];
	frustumPlanes[1].normal.z = frust[11] + frust[8];

	frustumPlanes[2].normal.x = frust[3 ] - frust[1];
	frustumPlanes[2].normal.y = frust[7 ] - frust[5];
	frustumPlanes[2].normal.z = frust[11] - frust[9];

	frustumPlanes[3].normal.x = frust[3 ] + frust[1];
	frustumPlanes[3].normal.y = frust[7 ] + frust[5];
	frustumPlanes[3].normal.z = frust[11] + frust[9];

	slVectorNormalize(&frustumPlanes[0].normal);
	slVectorNormalize(&frustumPlanes[1].normal);
	slVectorNormalize(&frustumPlanes[2].normal);
	slVectorNormalize(&frustumPlanes[3].normal);
}

/*
	\brief Tests whether a single point is inside the camera frustum.
*/

int slCamera::pointInFrustum(slVector *test) {
	int n;

	for(n=0;n<4;n++) {
		if(slPlaneDistance(&frustumPlanes[n], test) < 0.0) return 0;
	}

	return 1;
}

/*
	\brief Tests whether the min and max vectors of an object are inside the camera frustum.
*/

int slCamera::minMaxInFrustum(slVector *min, slVector *max) {
	return 1;
}

/*!
	\brief Tests whether a polygon is inside the camera frustum.
*/

int slCamera::polygonInFrustum(slVector *test, int n) {
	int x;
	char violations[4] = { 0, 0, 0, 0 };
	int v = 0;

	for(x=0;x<n;x++) {
		int plane;

		for(plane=0;plane<4;plane++) {
			if(slPlaneDistance(&frustumPlanes[plane], &test[x]) < 0.0) {
				violations[plane] = 1;
				v++;
				plane = 4;
			} 
		}
	}

	// no violations -- the polygon is entirely in the frustum

	if(v == 0) return 1;

	// violation on only one side -- the polygon is perfectly excluded

	if((v == n) && violations[0] + violations[1] + violations[2] + violations[3] == 1) return 0;

	// multiple violations -- the polygon is possibly interesting the frustum

	return 2;
}

/*!
	\brief Resizes the camera.
*/

void slCameraResize(slCamera *c, int x, int y) {
	c->fov = (double)x/(double)y;
	c->x = x; 
	c->y = y;
}

/*!
	\brief Frees the camera.
*/

void slCameraFree(slCamera *c) {
	unsigned int n;

	for(n=0;n<c->maxBillboards;n++) delete c->billboards[n];
	slFree(c->billboards);

	for(n=0;n<c->text.size();n++) if(c->text[n].text) slFree(c->text[n].text);

	delete c;
}

/*!
	\brief Updates the camera's internal state after changes have been made.

	Used to update the camera's internal state after changes have been made
	to the rotation or zoom settings.
*/

void slUpdateCamera(slCamera *c) {
	double m[3][3], n[3][3];
	slVector yaxis, xaxis, unit;

	slVectorSet(&unit, 0, 0, 1);

	/* build individual rotation matrices for rotation around */
	/* x and y, and then multiply them together */

	slVectorSet(&yaxis, 0, 1, 0);
	slVectorSet(&xaxis, 1, 0, 0);

	if(isnan(c->rx)) c->rx = 0.0;
	if(isnan(c->ry)) c->ry = 0.0;

	c->rx = fmod(c->rx, M_PI * 2.0);
	c->ry = fmod(c->ry, M_PI * 2.0);

	if(c->rx < 0.0) c->rx += 2.0 * M_PI;
	if(c->ry < 0.0) c->ry += 2.0 * M_PI;

	slRotationMatrix(&yaxis, c->ry, m);
	slRotationMatrix(&xaxis, c->rx, n);

	slMatrixMulMatrix(m, n, c->rotation);

	// preform the rotation around the unit vector 

	slVectorXform(c->rotation, &unit, &c->location);

	// apply the zoom
	
	slVectorMul(&c->location, c->zoom, &c->location);

	// slCameraUpdateFrustum(c);
}

/*!
	\brief Adds a string of text to the camera's output display. 
*/

void slSetCameraText(slCamera *c, int n, char *string, float x, float y, slVector *v) {
	if((unsigned int)n >= c->text.size() || n < 0) {
	    slMessage(DEBUG_ALL, "out of bounds text position %d in slSetCameraText\n", n);
	    return;
	}

	if(c->text[n].text) slFree(c->text[n].text);

	c->text[n].text = slStrdup(string);
	c->text[n].x = x;
	c->text[n].y = y;

	if(v) slVectorCopy(v, &c->text[n].color);
	else slVectorZero(&c->text[n].color);
}

/*!
	\brief Sets an object to catch shadows and reflections.

	Take a certain shape and find the plane whose normal matches 
	closest to a given normal, and set the plane to be the shadow
	catcher.
*/

void slSetShadowCatcher(slCamera *c, slStationary *s, slVector *normal) {
	slFace *face, *bestFace = NULL;
	double best = 0.0, dot;
	std::vector<slFace*>::iterator fi;

	for(fi = s->shape->faces.begin(); fi != s->shape->faces.end(); fi++ ) {
        face = *fi;

        dot = slVectorDot(&face->plane.normal, normal);

        if(dot > best) {
            bestFace = face;
            best = dot;
        }
	}

	if(!bestFace) {
	    // this shouldn't happen 
	    c->drawShadow = 0;
	    return;
	}

	memcpy(&c->shadowPlane, &bestFace->plane, sizeof(slPlane));

	slVectorAdd(&c->shadowPlane.vertex, &s->position.location, &c->shadowPlane.vertex);

	c->recompile = 1;

	c->shadowCatcher = s;

	bestFace->drawFlags |= SD_STENCIL|SD_REFLECT;
}

/*!
	\brief Adds an entry for a billboard.

	This is used dynamically while rendering billboards in order to correctly render
	them from back to front.
*/

void slAddBillboard(slCamera *c, slWorldObject *object, float size, float z) {
	unsigned int n, last;

	if(c->billboardCount == c->maxBillboards) {
	    last = c->maxBillboards;
	    c->maxBillboards *= 2;

	    c->billboards = (slBillboardEntry**)slRealloc(c->billboards, sizeof(slBillboardEntry*) * c->maxBillboards);

	    for(n=last;n<c->maxBillboards;n++) c->billboards[n] = new slBillboardEntry;
	}

	c->billboards[c->billboardCount]->z = z;
	c->billboards[c->billboardCount]->size = size;
	c->billboards[c->billboardCount]->object = object;

	c->billboardCount++;
}

/*!
	\brief The sort function used to sort billboards from back to front.
*/

bool slBillboardCompare(const slBillboardEntry *a, const slBillboardEntry *b) {
	return a->z < b->z;
}

/*!
	\brief Sorts the billboards from back to front.
*/

void slSortBillboards(slCamera *c) {
	std::sort(c->billboards, c->billboards + c->billboardCount, slBillboardCompare);
}

/*!
	\brief Moves the camera in resoponse to a mouse movement.

	This is called by the interface when a mouse movement is made
	with the motion modifier key pressed, or with the motion tool is 
	selected.
*/

void slMoveCameraWithMouseMovement(slCamera *camera, double dx, double dy) {
	slVector xaxis, yaxis, tempV, location;

    slVectorSet(&yaxis, 0, 1, 0);

	slVectorCopy(&camera->location, &location);

	slVectorCross(&location, &yaxis, &xaxis);
	slVectorNormalize(&xaxis);
	slVectorMul(&xaxis, -dx / 10.0, &tempV);
	slVectorSub(&camera->target, &tempV, &camera->target);

	slVectorCross(&location, &xaxis, &tempV);
	slVectorNormalize(&tempV);                
	if(tempV.y > 0) slVectorMul(&tempV, -1, &tempV);
	slVectorMul(&tempV, -dy / 10.0, &tempV);
	slVectorAdd(&camera->target, &tempV, &camera->target);    
}

/*!
	\brief Rotates the camera in resoponse to a mouse movement.

	This is called by the interface when a mouse movement is made
	with the rotate modifier key pressed, or with the rotate tool is 
	selected.
*/

void slRotateCameraWithMouseMovement(slCamera *camera, double dx, double dy, double startCamX) {
	if(startCamX > M_PI/2.0 && startCamX < 3.0/2.0 * M_PI) dy *= -1;

	camera->ry -= dx * .01;
	camera->rx -= dy * .01;

	slUpdateCamera(camera);
}

/*!
	\brief Zooms the camera in resoponse to a mouse movement.

	This is called by the interface when a mouse movement is made
	with the zoom modifier key pressed, or with the zoom tool is 
	selected.
*/

void slZoomCameraWithMouseMovement(slCamera *camera, double dx, double dy) {
	if(0.1 * dy < camera->zoom) {
		camera->zoom -= 0.1 * dy;
		slUpdateCamera(camera);
	} 
}

void slCameraSetBounds(slCamera *c, unsigned int x, unsigned int y) {
	c->x = x;
	c->y = y;
	c->fov = (double)c->x/(double)c->y;
}

void slCameraGetBounds(slCamera *c, unsigned int *x, unsigned int *y) {
	*x = c->x;	
	*y = c->y;	
}

void slCameraGetRotation(slCamera *c, double *x, double *y) {
	*x = c->rx;
	*y = c->ry;
}

void slCameraSetRecompile(slCamera *c) {
	c->recompile = 1;
}

void slCameraSetActivateContextCallback(slCamera *c, int (*f)()) {
	c->activateContextCallback = f;
}


