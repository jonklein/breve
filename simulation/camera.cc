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

slCamera *slNewCamera(int x, int y, int drawMode) {
	slCamera *c;
	int n;

	c = new slCamera;
	bzero(c, sizeof(slCamera));

	c->enabled = CM_NOT_UPDATED;

	c->text = new slCameraText[8]; 

	c->maxText = 8;
	c->textCount = 0;
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

	c->drawMode = drawMode;

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
	c->onlyMultibodies = 0;

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
	int n;

	for(n=0;n<c->maxBillboards;n++) delete c->billboards[n];
	slFree(c->billboards);

	for(n=0;n<c->textCount;n++) if(c->text[n].text) slFree(c->text[n].text);

	delete[] c->text;
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

	/* preform the rotation around the unit vector */

	slVectorXform(c->rotation, &unit, &c->location);

	/* apply the zoom */
	
	slVectorMul(&c->location, c->zoom, &c->location);
}

/*!
	\brief Adds a string of text to the camera's output display. 
*/

void slSetCameraText(slCamera *c, int n, char *string, float x, float y, slVector *v) {
	if(n >= c->maxText || n < 0) {
	    slMessage(DEBUG_ALL, "out of bounds text position %d in slSetCameraText\n", n);
	    return;
	}

	if(c->textCount < (n+1)) c->textCount = (n+1);

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
	    /* this shouldn't happen */

	    c->drawShadow = 0;
	    return;
	}

	bcopy(&bestFace->plane, &c->shadowPlane, sizeof(slPlane));

	slVectorAdd(&c->shadowPlane.vertex, &s->position.location, &c->shadowPlane.vertex);

	c->recompile = 1;

	bestFace->drawFlags |= SD_STENCIL|SD_REFLECT;
}

/*!
	\brief Adds an entry for a billboard.

	This is used dynamically while rendering billboards in order to correctly render
	them from back to front.
*/

void slAddBillboard(slCamera *c, slVector *color, slVector *loc, float size, float rotation, float alpha, int bitmap, int textureMode, float z, unsigned char selected) {
	int n, last;

	if(c->billboardCount == c->maxBillboards) {
	    last = c->maxBillboards;
	    c->maxBillboards *= 2;

	    c->billboards = (slBillboardEntry**)slRealloc(c->billboards, sizeof(slBillboardEntry*) * c->maxBillboards);

	    for(n=last;n<c->maxBillboards;n++) c->billboards[n] = new slBillboardEntry;
	}

	slVectorCopy(loc, &c->billboards[c->billboardCount]->location);
	slVectorCopy(color, &c->billboards[c->billboardCount]->color);

	c->billboards[c->billboardCount]->bitmap = bitmap;
	c->billboards[c->billboardCount]->z = z;
	c->billboards[c->billboardCount]->size = size;
	c->billboards[c->billboardCount]->rotation = rotation;
	c->billboards[c->billboardCount]->alpha = alpha;
	c->billboards[c->billboardCount]->selected = selected;
	c->billboards[c->billboardCount]->mode = textureMode;

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
	std::sort(c->billboards, c->billboards + c->billboardCount + 1, slBillboardCompare);
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
