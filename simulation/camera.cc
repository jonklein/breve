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

slCamera::slCamera(int x, int y) {
	slCameraText tx;
	slCameraText &t = tx;

	unsigned int n;

	text.insert(text.begin(), 8, t);

	textScale = 1;

	x = x;
	y = y;
	ox = 0;
	oy = 0;

	zClip = 500.0;

	if(y != 0.0) fov = (double)x/(double)y;
	else fov = 40;

	// billboarding works poorly when all the billboards 
	// are on the same plane, so we'll offset the camera 
	// just slightly.  enough so that the alpha bending 
	// works correctly, but not so that it's visable. 

	rx = 0.001;
	ry = 0.001;

	drawMode = GL_POLYGON;

	slVectorSet(&target, 0, 0, 0);

	slVectorSet(&location, 0, 10, 100);

	drawSmooth = 0;
	drawLights = 0;
	drawShadow = 0;
	drawOutline = 0;
	drawReflection = 0;
	drawText = 1;
	blur = 0;
	blurFactor = 0.1;

	billboardCount = 0;
	maxBillboards = 8;
	billboards = (slBillboardEntry**)slMalloc(sizeof(slBillboardEntry*) * maxBillboards);
	billboardDrawList = 0;

	fogIntensity = .1;
	fogStart = 10;	
	fogEnd = 40;	

	for(n=0;n<maxBillboards;n++) billboards[n] = new slBillboardEntry;

	nLights = 1;

	slVectorSet(&lights[0].location, 0, 0, 0);
	slVectorSet(&lights[0].ambient, .6, .6, .6);
	slVectorSet(&lights[0].diffuse, .6, .9, .9);
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
	slVectorCopy(&loc, &frustumPlanes[4].vertex);
	slVectorCopy(&loc, &frustumPlanes[5].vertex);

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

	slVectorMul(&location, -1.0, &frustumPlanes[4].normal);

	slVectorNormalize(&frustumPlanes[0].normal);
	slVectorNormalize(&frustumPlanes[1].normal);
	slVectorNormalize(&frustumPlanes[2].normal);
	slVectorNormalize(&frustumPlanes[3].normal);
	slVectorNormalize(&frustumPlanes[4].normal);
	slVectorNormalize(&frustumPlanes[5].normal);
}

/*
	\brief Tests whether a single point is inside the camera frustum.
*/

int slCamera::pointInFrustum(slVector *test) {
	int n;

	for(n=0;n<5;n++) 
		if(slPlaneDistance(&frustumPlanes[n], test) < 0.0) return 0;

	return 1;
}

/*
	\brief Tests whether the min and max vectors of an object are inside the camera frustum.
*/

int slCamera::minMaxInFrustum(slVector *min, slVector *max) {
	return (pointInFrustum(min) || pointInFrustum(max));
}

/*!
	\brief Tests whether a polygon is inside the camera frustum.
*/

int slCamera::polygonInFrustum(slVector *test, int n) {
	int x;
	char violations[6] = { 0, 0, 0, 0, 0, 0 };
	int v = 0;

	for(x=0;x<n;x++) {
		int plane;

		for(plane=0;plane<5;plane++) {
			if(slPlaneDistance(&frustumPlanes[plane], &test[x]) < 0.0) {
				violations[plane]++;
				v++;
			} 
		}
	}

	// no violations -- the polygon is entirely in the frustum

	if(v == 0) return 1;

	// all violating on one side -- the polygon is perfectly excluded

	if(violations[0] == n || violations[1] == n || violations[2] == n || violations[3] == n || violations[4] == n) return 0;

	// multiple violations -- the polygon is possibly interesting the frustum

	return 1;
}

/*!
	\brief Resizes the camera.
*/

void slCamera::resize(int nx, int ny) {
	x = nx; 
	y = ny;
	fov = (double)x/(double)y;
}

/*!
	\brief Frees the camera.
*/

slCamera::~slCamera() {
	unsigned int n;

	for(n=0;n<maxBillboards;n++) delete billboards[n];
	slFree(billboards);
}

/*!
	\brief Updates the camera's internal state after changes have been made.

	Used to update the camera's internal state after changes have been made
	to the rotation or zoom settings.
*/

void slCamera::update() {
	double m[3][3], n[3][3];
	slVector yaxis, xaxis, unit;

	slVectorSet(&unit, 0, 0, 1);

	/* build individual rotation matrices for rotation around */
	/* x and y, and then multiply them together */

	slVectorSet(&yaxis, 0, 1, 0);
	slVectorSet(&xaxis, 1, 0, 0);

	if(isnan(rx)) rx = 0.0;
	if(isnan(ry)) ry = 0.0;

	rx = fmod(rx, M_PI * 2.0);
	ry = fmod(ry, M_PI * 2.0);

	if(rx < 0.0) rx += 2.0 * M_PI;
	if(ry < 0.0) ry += 2.0 * M_PI;

	slRotationMatrix(&yaxis, ry, m);
	slRotationMatrix(&xaxis, rx, n);

	slMatrixMulMatrix(m, n, rotation);

	// preform the rotation around the unit vector 

	slVectorXform(rotation, &unit, &location);

	// apply the zoom
	
	slVectorMul(&location, zoom, &location);
}

/*!
	\brief Adds a string of text to the camera's output display. 
*/

void slSetCameraText(slCamera *c, int n, char *string, float x, float y, slVector *v) {
	if((unsigned int)n >= c->text.size() || n < 0) {
	    slMessage(DEBUG_ALL, "out of bounds text position %d in slSetCameraText\n", n);
	    return;
	}

	c->text[n].text = string;
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
	\brief Sets the size of the camera window.
*/

void slCamera::setBounds(unsigned int nx, unsigned int ny) {
	x = nx;
	y = ny;
	fov = (double)x/(double)y;
}

/*!
	\brief Gets the size of the camera window.
*/

void slCamera::getBounds(unsigned int *nx, unsigned int *ny) {
	*nx = x;
	*ny = y;	
}

/*!
	\brief Gets the camera's x and y rotation.
*/

void slCamera::getRotation(double *x, double *y) {
	*x = rx;
	*y = ry;
}

/*!
	\brief Sets the recompile flag for this camera, indicating that drawlists
	need to be recompiled.
*/

void slCamera::setRecompile() {
	recompile = 1;
}

void slCameraSetActivateContextCallback(slCamera *c, int (*f)()) {
	c->activateContextCallback = f;
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
	slVectorMul(&xaxis, camera->zoom * -dx / 100.0, &tempV);
	slVectorSub(&camera->target, &tempV, &camera->target);

	slVectorCross(&location, &xaxis, &tempV);
	slVectorNormalize(&tempV);                
	if(tempV.y > 0) slVectorMul(&tempV, -1, &tempV);
	slVectorMul(&tempV, camera->zoom * -dy / 100.0, &tempV);
	slVectorAdd(&camera->target, &tempV, &camera->target);    
}

/*!
	\brief Rotates the camera in resoponse to a mouse movement.

	This is called by the interface when a mouse movement is made
	with the rotate modifier key pressed, or with the rotate tool is 
	sineelected.
*/

void slRotateCameraWithMouseMovement(slCamera *camera, double dx, double dy) {
	if(camera->rx > M_PI/2.0 && camera->rx < 3.0/2.0 * M_PI) dy *= -1;

	camera->ry -= dx * .01;
	camera->rx -= dy * .01;

	camera->update();
}

/*!
	\brief Zooms the camera in resoponse to a mouse movement.

	This is called by the interface when a mouse movement is made
	with the zoom modifier key pressed, or with the zoom tool is 
	selected.
*/

void slZoomCameraWithMouseMovement(slCamera *camera, double dx, double dy) {
	if(0.1 * dy < camera->zoom) {
		camera->zoom -= 0.002 * camera->zoom * dy;
		camera->update();
	} 
}
