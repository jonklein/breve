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

#include <vector>

#define MAX(x, y) ((x)>(y)?(x):(y))

#define BUFFER_SIZE 512
 
#include "simulation.h"
#include "shadow.h"
#include "asciiart.h"
#include "glIncludes.h"
#include "tiger.h"

#define REFLECTION_ALPHA	.75

#define LIGHTSIZE 64

double gReflectionAlpha;

int glActive = 0;

/*!
	\brief Calls glMultMatrix with 3x3 orientation slMatrix.

	Swaps the rows and columns of the matrix (since GL matrices
	are the opposite of SL matrices).
*/

void slMatrixGLMult(double m[3][3]) {
	double d[4][4];

	d[0][0] = m[0][0]; d[0][1] = m[1][0]; d[0][2] = m[2][0]; d[0][3] = 0;
	d[1][0] = m[0][1]; d[1][1] = m[1][1]; d[1][2] = m[2][1]; d[1][3] = 0;
	d[2][0] = m[0][2]; d[2][1] = m[1][2]; d[2][2] = m[2][2]; d[2][3] = 0;
	d[3][0] = 0; d[3][1] = 0; d[3][2] = 0; d[3][3] = 1;

	glMultMatrixd((double*)d);
}

/*!
	\brief Creates the texture used for the "lightmaps".
*/

void slMakeLightTexture(GLubyte *lTexture, GLubyte *dlTexture) {
	int i, j;
	double x, y, temp, dtemp;

	for(i=0; i<LIGHTSIZE; i++){
		for(j=0; j<LIGHTSIZE; j++){
			x = (float)(i - LIGHTSIZE / 2.0) / (float)(LIGHTSIZE / 2.0);
			y = (float)(j - LIGHTSIZE / 2.0) / (float)(LIGHTSIZE / 2.0);

			temp = (1.0f - (float)(sqrt((x * x) + (y * y)))) * 1.2;
			dtemp = temp + slRandomDouble()/10.0;

			if(temp > 1.0f) temp = 1.0f;
			if(temp < 0.0f) temp = 0.0f;

			if(dtemp > 1.0f) dtemp = 1.0f;
			if(dtemp < 0.0f) dtemp = 0.0f;

			lTexture[(i * LIGHTSIZE * 2) + j * 2] = (unsigned char)(255.0f * temp * temp);
			lTexture[(i * LIGHTSIZE * 2) + (j * 2) + 1] = (unsigned char)(255.0f * temp * temp);
			dlTexture[(i * LIGHTSIZE * 2) + j * 2] = (unsigned char)(255.0f * dtemp * dtemp);
			dlTexture[(i * LIGHTSIZE * 2) + (j * 2) + 1] = (unsigned char)(255.0f * temp * temp);
		 }
	}
}
  
void slInitGL(slWorld *w, slCamera *c) {
	GLfloat specularColor[4] = { 0.2, 0.2, 0.2, 0.0 };
	GLubyte lt[LIGHTSIZE * LIGHTSIZE * 2];
	GLubyte glt[LIGHTSIZE * LIGHTSIZE * 2];

	glActive = 1;

	gReflectionAlpha = REFLECTION_ALPHA;

	slMakeLightTexture(&lt[0], &glt[0]);

	slUpdateTexture(c, slTextureNew(c), gBrickImage, TEXTURE_WIDTH, TEXTURE_HEIGHT, GL_RGBA);
	slUpdateTexture(c, slTextureNew(c), gPlaid, TEXTURE_WIDTH, TEXTURE_HEIGHT, GL_RGBA);
	slUpdateTexture(c, slTextureNew(c), lt, LIGHTSIZE, LIGHTSIZE, GL_LUMINANCE_ALPHA);
	slUpdateTexture(c, slTextureNew(c), glt, LIGHTSIZE, LIGHTSIZE, GL_LUMINANCE_ALPHA);

	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	glEnable(GL_DEPTH_TEST);

	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	glEnable(GL_LINE_SMOOTH);
	glEnable(GL_POINT_SMOOTH);
	glLineWidth(2);

	glPolygonOffset(-4.0f, -1.0f);

	glHint(GL_LINE_SMOOTH_HINT, GL_NICEST);
	glHint(GL_POLYGON_SMOOTH_HINT, GL_NICEST);
	glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);
	glEnable(GL_COLOR_MATERIAL);
	glColorMaterial(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE);

	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	glPixelStorei(GL_PACK_ALIGNMENT, 1);

	slClearGLErrors("init");

	c->cubeDrawList = glGenLists(1);
	slCompileCubeDrawList(c->cubeDrawList);

	glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, specularColor);
	glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, 90);
}

/*!
	\brief Initializes a cube drawing list.

	The cube list is used in the patch drawing.  It's built as part of initialization. 
*/

void slCompileCubeDrawList(int l) {
	glNewList(l, GL_COMPILE);

	// juh?

	glBegin(GL_TRIANGLE_STRIP);
		glVertex3f(0.0, 0.0, 0.0);
		glVertex3f(1.0, 0.0, 0.0);
		glVertex3f(0.0, 0.0, 1.0);
		glVertex3f(1.0, 0.0, 1.0);
		glVertex3f(1.0, 1.0, 1.0);
		glVertex3f(1.0, 0.0, 0.0);
		glVertex3f(1.0, 1.0, 0.0);
		glVertex3f(0.0, 0.0, 0.0);
		glVertex3f(0.0, 1.0, 0.0);
		glVertex3f(0.0, 0.0, 1.0);
		glVertex3f(0.0, 1.0, 1.0);
		glVertex3f(1.0, 1.0, 1.0);
		glVertex3f(0.0, 1.0, 0.0);
		glVertex3f(1.0, 1.0, 0.0);

	glEnd();

	glEndList();
}

/*!
	\brief Center the given pixels in a square buffer.

	Used for textures, which must be powers of two.
*/

void slCenterPixelsInSquareBuffer(unsigned char *pixels, int width, int height, unsigned char *buffer, int newwidth, int newheight) {
	int xstart, ystart;
	int y;

	xstart = (newwidth - width) / 2;
	ystart = (newheight - height) / 2;

	for(y=0;y<height;y++) {
		bcopy(&pixels[y * width * 4], &buffer[(y + ystart) * (newwidth * 4) + (xstart * 4)], width * 4);
	}
}

/*!
	\brief Allocates space for a new texture.
*/

unsigned int slTextureNew(slCamera *c) {
	GLuint texture;

	if(!glActive) return 0;

	if(c->activateContextCallback) c->activateContextCallback();

	glGenTextures(1, &texture);
	return texture;
}

void slTextureFree(slCamera *c, const unsigned int n) {
	if(!glActive) return;

	if(c->activateContextCallback) c->activateContextCallback();

	glDeleteTextures(1, &n);
}

/*!
	\brief Adds (or updates) a texture to the camera.

	Returns 0 if there was space, or -1 if all texture positions are used.
*/

int slUpdateTexture(slCamera *c, GLuint texture, unsigned char *pixels, int width, int height, int format) {
	unsigned char *newpixels;
	int newheight, newwidth;

	if(!glActive) return -1;

	if(c->activateContextCallback) c->activateContextCallback();

	newwidth = slNextPowerOfTwo(width); 
	newheight = slNextPowerOfTwo(height);

	newwidth = newheight = MAX(newwidth, newheight);

	if(newwidth != width || newheight != height) {
		newpixels = new unsigned char[newwidth * newheight * 4];
		bzero(newpixels, newwidth * newheight * 4);

		slCenterPixelsInSquareBuffer(pixels, width, height, newpixels, newwidth, newheight);
	} else {
		newpixels = pixels;
	}

	glBindTexture(GL_TEXTURE_2D, texture);
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	glTexImage2D(GL_TEXTURE_2D, 0, format, newwidth, newheight, 0, format, GL_UNSIGNED_BYTE, newpixels);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);

	if(newwidth != width || newheight != height) delete[] newpixels;

	if(slClearGLErrors("error adding texture")) return -1;

	return texture;
}

int slGlSelect(slWorld *w, slCamera *c, int x, int y) {
	GLuint selection_buffer[BUFFER_SIZE], *selections;
	GLint viewport[4];
	GLint hits;
	GLuint namesInHit;
	int n;
	unsigned int nearest = 0xffffffff, min;
	unsigned int hit = w->objects.size() + 1;
	slVector cam;

	viewport[0] = c->ox;
	viewport[1] = c->oy;
	viewport[2] = c->x;
	viewport[3] = c->y;

	glSelectBuffer(BUFFER_SIZE, selection_buffer);
	glRenderMode(GL_SELECT);
	slClearGLErrors("selected buffer");

	glInitNames();
	glPushName(0);

	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glLoadIdentity();

	gluPickMatrix((GLdouble)x, (GLdouble)(viewport[3] - y), 5.0, 5.0, viewport);
	slClearGLErrors("picked matrix");

	gluPerspective(40.0, c->fov, 0.01, c->zClip);

	// since the selection buffer uses unsigned ints for names, we can't 
	// use -1 to mean no selection -- we'll use the number of objects 
	// plus 1 to indicate that no selectable object was selected.

	glLoadName(w->objects.size() + 1);

	slClearGLErrors("about to select");

    slVectorAdd(&c->location, &c->target, &cam);
    
    gluLookAt(cam.x, cam.y, cam.z, c->target.x, c->target.y, c->target.z, 0.0, 1.0, 0.0);

	// Render the objects in the world, with name loading, and with billboards as spheres

	slRenderObjects(w, c, DO_BILLBOARDS_AS_SPHERES|DO_LOAD_NAMES);

	hits = glRenderMode(GL_RENDER);

	glMatrixMode(GL_PROJECTION);
	glPopMatrix();

	slClearGLErrors("picked");

	selections = &selection_buffer[0];

	for(n=0;n<hits;n++) {
		namesInHit = *selections++;
	   
		// skip over the z-max value 

		selections++;
		min = *selections++;

		if(min < nearest) {
			nearest = min;
		   
			while(namesInHit--) hit = *selections++;
		} else {
			while(namesInHit--) selections++;
		}
	}

	glPopName();

	if(hit == w->objects.size() + 1) return -1;

	return hit;
}

/*!
	\brief Computes the vector corresponding to a drag in the display.

	Computes the location in the same plane as dragVertex that the mouse 
	is being dragged to when the window mouse coordinates are x and y.
*/

int slVectorForDrag(slWorld *w, slCamera *c, slVector *dragVertex, int x, int y, slVector *dragVector) {
	slVector *t, cam;
	slPlane plane;
	GLdouble model[16];
	GLdouble proj[16];
	GLint view[4];
	GLdouble wx, wy;
	GLdouble oxf, oyf, ozf;
	int r;
	slVector end;
	double sD, eD;

	// set up the matrices for a regular draw--gluUnProject needs this 

	t = &c->target;

	view[0] = c->ox;
	view[1] = c->oy;
	view[2] = c->x;
	view[3] = c->y;

	glViewport(c->ox, c->oy, c->x, c->y);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();

	gluPerspective(40.0, c->fov, 0.1, c->zClip);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	slVectorAdd(&c->location, t, &cam);
	gluLookAt(cam.x, cam.y, cam.z, t->x, t->y, t->z, 0.0, 1.0, 0.0);

	// get the data for gluUnProject

	glGetDoublev(GL_MODELVIEW_MATRIX, model);
	glGetDoublev(GL_PROJECTION_MATRIX, proj);

	y = view[3] - y;

	wx = x;
	wy = y;

	// use gluUnProject to get the point in object space where we are clicking 
	// (and at the far reach of our zClip variable). 

	r = gluUnProject(wx, wy, 1.0, model, proj, view, &oxf, &oyf, &ozf);

	end.x = oxf;
	end.y = oyf;
	end.z = ozf;

	// define the plane where the object in question lies 

	slVectorCopy(&c->location, &plane.normal);
	slVectorNormalize(&plane.normal);
	slVectorCopy(dragVertex, &plane.vertex);

	//	compute the distance 
	//		1) from the camera to the object plane
	//		2) from the zClip plane to the object plane

	sD = slPlaneDistance(&plane, &cam);
	eD = slPlaneDistance(&plane, &end);

	eD = fabs(eD);
	sD = fabs(sD);

	// compute the vector from the camera to the end of the zClip plane.
	// this is the vector containing all of the candidate points that the
	// user is dragging--we need to figure out which one we're interested in

	slVectorSub(&end, &cam, dragVector);

	// compute the point on the object plane of the drag vector 

	slVectorMul(dragVector, (sD / (eD + sD)), dragVector);
	slVectorAdd(dragVector, &cam, dragVector);

	return 0;
}

void slRenderScene(slWorld *w, slCamera *c, int crosshair) {
	if(w->detectLightExposure) slDetectLightExposure(w, c, 200, NULL);

	slRenderWorld(w, c, crosshair, 0);

	if(w->cameras.size() != 0) slRenderWorldCameras(w);
}

void slRenderWorld(slWorld *w, slCamera *c, int crosshair, int scissor) {
	slVector cam;
	int flags = 0;
	std::vector<slPatchGrid*>::iterator pi;

	if(!w || !c) return;

	glViewport(c->ox, c->oy, c->x, c->y);

	if(scissor) {
		flags |= DO_NO_AXIS|DO_NO_BOUND;
		glEnable(GL_SCISSOR_TEST);
		glScissor(c->ox, c->oy, c->x, c->y);
	}

	if(c->drawOutline) {
		flags |= DO_OUTLINE;
		flags |= DO_BILLBOARDS_AS_SPHERES;
	} 

	if(c->recompile) {
		c->recompile = 0;
		slCompileCubeDrawList(c->cubeDrawList);
		flags |= DO_RECOMPILE;
	}

	slClear(w, c);
	
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();

	if(w->backgroundTexture > 0 && !(flags & DO_OUTLINE)) slDrawBackground(c, w);

	gluPerspective(40.0, c->fov, 0.01, c->zClip);

	glEnable(GL_DEPTH_TEST);

	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glLoadIdentity();

	slVectorAdd(&c->location, &c->target, &cam);

	gluLookAt(cam.x, cam.y, cam.z, c->target.x, c->target.y, c->target.z, 0.0, 1.0, 0.0);

	slCameraUpdateFrustum(c);

	if(c->drawFog) slDrawFog(w, c);
	else glDisable(GL_FOG);

	// do a pass through to grab all the billboards--we want to sort them 
	// so that they can be rendered back to front and blended correctly

	if(c->drawLights) {
		if(c->drawShadowVolumes) slDrawLights(c, 1);
		else slDrawLights(c, 0);

		if(c->drawReflection || c->drawShadow) slStencilFloor(w, c);

		if(c->drawReflection && !(flags & DO_OUTLINE)) {
			slVector toCam;

			if(!c->drawShadowVolumes) gReflectionAlpha = REFLECTION_ALPHA;
			else gReflectionAlpha = REFLECTION_ALPHA - 0.1;

			slVectorSub(&cam, &c->shadowPlane.vertex, &toCam);

			if(slVectorDot(&toCam, &c->shadowPlane.normal) > 0.0) {
				slReflectionPass(w, c);
				if(c->drawShadowVolumes) slDrawLights(c, 1);
			}
		}
	} else {
		glDisable(GL_LIGHTING);
	}

	// render the mobile objects

	slRenderObjects(w, c, flags|DO_NO_ALPHA);
	slRenderLines(w, c);
	slClearGLErrors("drew multibodies and lines");

	// now we do transparent objects and billboards.  they have to come last 
	// because they are blended.

	glDepthMask(GL_FALSE);
	slRenderObjects(w, c, flags|DO_ONLY_ALPHA);
	glDepthMask(GL_TRUE);

	if(!(flags & DO_BILLBOARDS_AS_SPHERES)) {
		slProcessBillboards(w, c);
		slRenderBillboards(c, flags);
	}

	for(pi = w->patches.begin(); pi != w->patches.end(); pi++) (*pi)->draw(c);
	
	if(c->drawLights) {
		// do the shadows
		if(c->drawShadowVolumes) slRenderShadowVolume(w, c);
		else if(c->drawShadow) slShadowPass(w, c);
	}

	// The following elements are one time renders with no special effects.

	slWorldDrawSprings(w);

	slRenderLabels(w);

#ifdef HAVE_LIBENET
	slDrawNetsimBounds(w);
#endif

	if(w->gisData) w->gisData->draw(c);

	glPopMatrix();

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();

	if(c->drawText) slRenderText(w, c, &c->location, &c->target, crosshair);

	if(c->drawText && crosshair && !scissor) {
		glPushMatrix();
		glColor3f(0, 0, 0);
		slText(0, 0, "+", GLUT_BITMAP_9_BY_15);
		glPopMatrix();
	}

	if(scissor) {
		double minY, maxY, minX, maxX;

		minX = -1;
		maxX =  1;
		minY = -1;
		maxY =  1;

		glLineWidth(2.0);

		glMatrixMode(GL_PROJECTION);
		glLoadIdentity();

		glColor4f(0, 0, 0, .5);
		glBegin(GL_LINES);
			glVertex2f(minX, minY);
			glVertex2f(maxX, minY);

			glVertex2f(maxX, minY);
			glVertex2f(maxX, maxY);

			glVertex2f(maxX, maxY);
			glVertex2f(minX, maxY);

			glVertex2f(minX, maxY);
			glVertex2f(minX, minY);
		glEnd();

		glDisable(GL_SCISSOR_TEST);
	}
}

void slClear(slWorld *w, slCamera *c) {
	if(c->drawOutline) {
		glClearColor(1, 1, 1, 0);
	} else {
		glClearColor(w->backgroundColor.x, w->backgroundColor.y, w->backgroundColor.z, 1.0);
	}

	if(!c->blur) {
		glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT|GL_STENCIL_BUFFER_BIT);
	} else {
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

		glColor4f(w->backgroundColor.x, w->backgroundColor.y, w->backgroundColor.z, 0.5f - (c->blurFactor / 2.0));

		glMatrixMode(GL_PROJECTION);
		glPushMatrix();
		glLoadIdentity();
		gluPerspective(40.0, c->fov, 0.01, c->zClip);
		glBegin(GL_TRIANGLE_STRIP);
			glVertex3f(-5, -4, -3);
			glVertex3f(5, -4, -3);
			glVertex3f(-5, 4, -3);
			glVertex3f(5, 4, -3);
		glEnd();
		glPopMatrix();
		glDisable(GL_BLEND);
		glClear(GL_DEPTH_BUFFER_BIT|GL_STENCIL_BUFFER_BIT);
	}
}

void slDrawFog(slWorld *w, slCamera *c) {
	GLfloat fog_color[4];

	fog_color[0] = c->fogColor.x;
	fog_color[1] = c->fogColor.y;
	fog_color[2] = c->fogColor.z;
	fog_color[3] = 1.0;

	glEnable(GL_FOG);
	glFogf(GL_FOG_DENSITY, c->fogIntensity);
	glHint(GL_FOG_HINT, GL_NICEST);
	glFogi(GL_FOG_MODE, GL_LINEAR);
	glFogf(GL_FOG_START, c->fogStart);
	glFogf(GL_FOG_END, c->fogEnd);
	glFogfv(GL_FOG_COLOR, fog_color);
}

/*!
	\brief Puts 1 into the stencil buffer where the shadows and reflections should fall.
*/

void slStencilFloor(slWorld *w, slCamera *c) {
	glEnable(GL_STENCIL_TEST);
	glDisable(GL_DEPTH_TEST);
	glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);
	glStencilFunc(GL_ALWAYS, 1, 0xffffffff);
	glStencilOp(GL_ZERO, GL_ZERO, GL_REPLACE);

	c->shadowCatcher->draw(c);

	glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
	glEnable(GL_DEPTH_TEST);
	glDisable(GL_STENCIL_TEST);
}

/*!
	\brief Draws a reflection of all multibody objects whereever the stencil buffer is equal to 1.
*/

void slReflectionPass(slWorld *w, slCamera *c) {
	glPushMatrix();

	glScalef(1.0, -1.0, 1.0);
	glTranslatef(0.0, -2*c->shadowPlane.vertex.y, 0.0);
	slDrawLights(c, 0);

	glCullFace(GL_FRONT);

	// render whereever the buffer is 1, but don't change the values 

	glEnable(GL_DEPTH_TEST);
	glEnable(GL_NORMALIZE);
	glEnable(GL_STENCIL_TEST);
	glEnable(GL_BLEND);

	glStencilFunc(GL_EQUAL, 1, 0xffffffff);  

	glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);
	slRenderObjects(w, c, DO_NO_STATIONARY|DO_NO_BOUND|DO_NO_AXIS|DO_NO_TERRAIN);
	slRenderBillboards(c, 0);

	glDisable(GL_NORMALIZE);

	glCullFace(GL_BACK);
	glDisable(GL_STENCIL_TEST);
	glDisable(GL_BLEND);

	glPopMatrix();

	slDrawLights(c, 0);
}

/*!
	\brief Shadows multibody objects on to the specified shadow plane, expecting
	that the stencil buffer has already been set to 3.

	The stencil buffer will be modified to 2 where the shadows are 
	drawn.
*/

void slShadowPass(slWorld *w, slCamera *c) {
	GLfloat shadowMatrix[4][4];
	slShadowMatrix(shadowMatrix, &c->shadowPlane, &c->lights[0].location);

	glPushAttrib(GL_ENABLE_BIT);

	glEnable(GL_STENCIL_TEST);
	glEnable(GL_DEPTH_TEST);
	glStencilFunc(GL_LESS, 0, 0xffffffff);  
	glStencilOp(GL_ZERO, GL_ZERO, GL_REPLACE);

	slClearGLErrors("about to enable polygon");

	glEnable(GL_POLYGON_OFFSET_FILL);

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	glColor4f(0.0, 0.0, 0.0, 0.3);
	// glColor4f(1.0, 0.0, 0.0, 1.0);

	glPushMatrix();
	glMultMatrixf((GLfloat*)shadowMatrix);
	slRenderObjects(w, c, DO_NO_COLOR|DO_NO_TEXTURE|DO_NO_STATIONARY|DO_NO_BOUND|DO_NO_AXIS|DO_NO_TERRAIN);
	glDisable(GL_LIGHTING);
	// glDisable(GL_CULL_FACE);
	slRenderBillboards(c, DO_NO_COLOR|DO_NO_BOUND);
	glPopMatrix();

	glPopAttrib();
}

/*!
	\brief Render the text associated with the current display.
*/

void slRenderText(slWorld *w, slCamera *c, slVector *location, slVector *target, int crosshair) {
	char textStr[128];
	double fromLeft;
	unsigned int n;

	glDisable(GL_DEPTH_TEST);
	glDisable(GL_STENCIL_TEST);
	glDisable(GL_BLEND);
	glColor4f(0.0, 0.0, 0.0, 1.0);
	snprintf(textStr, 127, "%.2f", w->age);

	fromLeft = -1.0 + (5.0 / c->x);
	slText(fromLeft, 1.0 - (20.0 / c->y), textStr, GLUT_BITMAP_HELVETICA_10);

	if(crosshair) {
		sprintf(textStr, "camera: (%.1f, %.1f, %.1f)", location->x, location->y, location->z);
		slText(fromLeft, -1.0 + (5.0 / c->y), textStr, GLUT_BITMAP_HELVETICA_10);
		sprintf(textStr, "target: (%.1f, %.1f, %.1f)", target->x, target->y, target->z);
		slText(fromLeft, -1.0 + (30.0 / c->y), textStr, GLUT_BITMAP_HELVETICA_10);
	} else {
		for(n=0;n<c->text.size();n++) {
			if(c->text[n].text) {
				glColor4f(c->text[n].color.x, c->text[n].color.y, c->text[n].color.z, 0.9);
				slStrokeText(c->text[n].x, c->text[n].y, c->text[n].text, c->textScale, GLUT_STROKE_ROMAN);
			}
		}
	}
}

/*!
	\brief Draw a texture as a background.
*/

void slDrawBackground(slCamera *c, slWorld *w) {
	static float transX = 0.0, transY = 0.0;
	GLfloat textColor[4];

	glDisable(GL_DEPTH_TEST);

	glPushAttrib(GL_LIGHTING_BIT|GL_TRANSFORM_BIT|GL_TEXTURE_BIT|GL_COLOR_BUFFER_BIT);

	glEnable(GL_BLEND);

	glColor4f(w->backgroundTextureColor.x, w->backgroundTextureColor.y, w->backgroundTextureColor.z, 1.0);

	textColor[0] = w->backgroundTextureColor.x;
	textColor[1] = w->backgroundTextureColor.y;
	textColor[2] = w->backgroundTextureColor.z;
	textColor[3] = 1.0;

	glDisable(GL_LIGHTING);
	glMatrixMode(GL_TEXTURE);
	glPushMatrix();
	glLoadIdentity();

	// this is likely an Apple driver bug, but on my machine it's
	// taking the color from the glColor, while on Nils' machine it's
	// taking in the color from the GL_TEXTURE_ENV_COLOR--we'll just 
	// set both.

	if(w->isBackgroundImage) {
		textColor[0] = 1.0;
		textColor[1] = 1.0;
		textColor[2] = 1.0;
		glEnable(GL_TEXTURE_2D);
		glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
		glTexEnvfv(GL_TEXTURE_ENV, GL_TEXTURE_ENV_COLOR, textColor);
	} else {
		glTexEnvfv(GL_TEXTURE_ENV, GL_TEXTURE_ENV_COLOR, textColor);
		glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_BLEND);
	}

	transX += c->backgroundScrollX;
	transY += c->backgroundScrollY;

	glTranslated(transX - (.8 * 2*c->ry), (.8 * 2*c->rx) - transY, 0);
	glDepthRange(1, .9);
	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, w->backgroundTexture);
	glBegin(GL_QUADS);
		glTexCoord2f(.0, .0); glVertex3f(-1, -1, -.1);
		glTexCoord2f(1.0, .0); glVertex3f(1, -1, -.1);
		glTexCoord2f(1.0, 1.0); glVertex3f(1, 1, -.1);
		glTexCoord2f(.0, 1.0); glVertex3f(-1, 1, -.1);
	glEnd();
	glDepthRange(0, 1);

	glDisable(GL_BLEND);
	glPopMatrix();
	glPopAttrib();
	glEnable(GL_DEPTH_TEST);
}

void slRenderLabels(slWorld *w) {
	slWorldObject *wo;
	slVector *l;
	std::vector<slWorldObject*>::iterator wi;

	glDisable(GL_DEPTH_TEST);

	glColor3f(0, 0, 0);

	for(wi = w->objects.begin(); wi != w->objects.end(); wi++) {
		wo = *wi;

		if(wo->label) {
			l = &wo->position.location;
			glPushMatrix();
			glTranslatef(l->x, l->y, l->z);
			slText(0, 0, wo->label, GLUT_BITMAP_HELVETICA_10);
			glPopMatrix();
		}
	}
	
	glEnable(GL_DEPTH_TEST);
}

/*!
	\brief Renders preprocessed billboards.
*/

void slRenderBillboards(slCamera *c, int flags) {
	slBillboardEntry *b;
	slVector normal;
	unsigned int n;
	int lastTexture = -1;

	slVectorCopy(&c->location, &normal);
	slVectorNormalize(&normal);

	if(!(flags & DO_NO_TEXTURE)) {
		glPushAttrib(GL_LIGHTING_BIT|GL_TRANSFORM_BIT|GL_TEXTURE_BIT|GL_COLOR_BUFFER_BIT);
		glEnable(GL_TEXTURE_2D);
		glEnable(GL_BLEND);
	}

	// we do want to have a depth test against other objects in the world. 
	// but we do our own back-to-front billboard sort and we do not want  
	// them fighting in the depth buffer.  so we'll disable depth-buffer  
	// writing so that no new info goes there. 

	glDepthMask(GL_FALSE);

	for(n=0;n<c->billboardCount;n++) {
		int bound;
		slWorldObject *object;

		glPushMatrix();

		b = c->billboards[n];

		object = b->object;

		bound = (object->drawMode & DM_BOUND);

		if(object->textureMode == BBT_LIGHTMAP) {
			glBlendFunc(GL_ONE, GL_ONE);
		} else {
	    	glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
			glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		}

		if(!(flags & DO_NO_COLOR)) {
			glColor4f(object->color.x, object->color.y, object->color.z, object->alpha);
		}

		if(lastTexture != object->texture) {
			glBindTexture(GL_TEXTURE_2D, object->texture);
		}

		lastTexture = object->texture;

		glTranslated(object->position.location.x, object->position.location.y, object->position.location.z);
		glRotatef(object->billboardRotation, normal.x, normal.y, normal.z);
		glScalef(b->size, b->size, b->size);

		glCallList(c->billboardDrawList);

		if(bound && !(flags & DO_NO_BOUND)) {
			if(!(flags & DO_NO_TEXTURE)) glDisable(GL_TEXTURE_2D);
			glPushMatrix();
			glScalef(1.1, 1.1, 1.1);
			glColor4f(0.0, 0.0, 0.0, 1.0);
			glBegin(GL_LINE_LOOP);
				glVertex3f(c->billboardX.x + c->billboardY.x, c->billboardX.y + c->billboardY.y, c->billboardX.z + c->billboardY.z);
				glVertex3f(-c->billboardX.x + c->billboardY.x, -c->billboardX.y + c->billboardY.y, -c->billboardX.z + c->billboardY.z);
				glVertex3f(-c->billboardX.x - c->billboardY.x, -c->billboardX.y - c->billboardY.y, -c->billboardX.z - c->billboardY.z);
				glVertex3f(c->billboardX.x - c->billboardY.x, c->billboardX.y - c->billboardY.y, c->billboardX.z - c->billboardY.z);
			glEnd();
			glPopMatrix();
			if(!(flags & DO_NO_TEXTURE)) glEnable(GL_TEXTURE_2D);
		}

		glPopMatrix();
	}

	glDepthMask(GL_TRUE);

	if(!(flags & DO_NO_TEXTURE)) glPopAttrib();
}

void slStrokeText(double x, double y, char *string, double scale, void *font) {
	int c;

	glPushMatrix();

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	glLineWidth(1.0);
	glTranslatef(x, y, 0);

	glScalef(scale * 0.0005, scale * 0.0005, scale * 0.0005);
	
	while((c = *(string++)) != 0) glutStrokeCharacter(font, c);

	glPopMatrix();
}

void slText(double x, double y, char *string, void *font) {
	int c;

	glRasterPos2f(x, y);

	while((c = *(string++)) != 0) glutBitmapCharacter(font, c);
}

/*!
	\brief Sets up lighting for a scene.

	If noDiff is set, no diffusion color is used.  This is used for 
	drawing the shadowed pass of a shadow volume algorithm.
*/

void slDrawLights(slCamera *c, int noDiffuse) {
	GLfloat dif[4];
	GLfloat dir[4];
	GLfloat amb[4];

	dir[0] = c->lights[0].location.x;
	dir[1] = c->lights[0].location.y;
	dir[2] = c->lights[0].location.z;
	dir[3] = 1.0;

	if(noDiffuse) {
		dif[0] = 0.0;
		dif[1] = 0.0;
		dif[2] = 0.0;
		dif[3] = 0.0;
	} else {
		dif[0] = c->lights[0].diffuse.x;
		dif[1] = c->lights[0].diffuse.y;
		dif[2] = c->lights[0].diffuse.z;
		dif[3] = 0.0;
	}

	amb[0] = c->lights[0].ambient.x;
	amb[1] = c->lights[0].ambient.y;
	amb[2] = c->lights[0].ambient.z;
	amb[3] = 0.0;

	if(c->drawSmooth) glShadeModel(GL_SMOOTH);
	else glShadeModel(GL_FLAT);

	glEnable(GL_LIGHTING);
	glEnable(GL_LIGHT0);

	glLightf(GL_LIGHT0, GL_CONSTANT_ATTENUATION, 0.1);
	glLightf(GL_LIGHT0, GL_LINEAR_ATTENUATION, 0.03);

	glLightfv(GL_LIGHT0, GL_DIFFUSE, dif);
	glLightfv(GL_LIGHT0, GL_AMBIENT, amb);
	glLightfv(GL_LIGHT0, GL_POSITION, dir);
	glLightfv(GL_LIGHT0, GL_SPECULAR, amb);
}

/*!
	\brief Set up the rendering matrix for flat shadows on a given plane.
*/

void slShadowMatrix(GLfloat matrix[4][4], slPlane *p, slVector *light) {
	GLfloat dot;

	GLfloat groundplane[4], lightpos[4];

	lightpos[0] = light->x;
	lightpos[1] = light->y;
	lightpos[2] = light->z;
	lightpos[3] = 0.0;

	groundplane[0] = p->normal.x;
	groundplane[1] = p->normal.y;
	groundplane[2] = p->normal.z;
	groundplane[3] = -slVectorDot(&p->normal, &p->vertex);
	  
	// Find dot product between light position vector and ground plane normal.

	dot = groundplane[0] * lightpos[0] + groundplane[1] * lightpos[1] + groundplane[2] * lightpos[2] + groundplane[3] * lightpos[3];
   
	matrix[0][0] = dot - lightpos[0] * groundplane[0];
	matrix[1][0] = 0.f - lightpos[0] * groundplane[1];
	matrix[2][0] = 0.f - lightpos[0] * groundplane[2];
	matrix[3][0] = 0.f - lightpos[0] * groundplane[3];

	matrix[0][1] = 0.f - lightpos[1] * groundplane[0];
	matrix[1][1] = dot - lightpos[1] * groundplane[1];
	matrix[2][1] = 0.f - lightpos[1] * groundplane[2];
	matrix[3][1] = 0.f - lightpos[1] * groundplane[3];

	matrix[0][2] = 0.f - lightpos[2] * groundplane[0];
	matrix[1][2] = 0.f - lightpos[2] * groundplane[1];
	matrix[2][2] = dot - lightpos[2] * groundplane[2];
	matrix[3][2] = 0.f - lightpos[2] * groundplane[3];

	matrix[0][3] = 0.f - lightpos[3] * groundplane[0];
	matrix[1][3] = 0.f - lightpos[3] * groundplane[1];
	matrix[2][3] = 0.f - lightpos[3] * groundplane[2];
	matrix[3][3] = dot - lightpos[3] * groundplane[3];
}

/*!
	\brief Draws a shape by setting up its transformations and calling its drawlist.
*/

void slDrawShape(slCamera *c, slShape *s, slPosition *pos, double textureScale, int mode, int flags) {
	unsigned char bound, axis;

	bound = (mode & DM_BOUND) && !(flags & DO_NO_BOUND);
	axis = (mode & DM_AXIS) && !(flags & DO_NO_AXIS);

	if(s->drawList == 0 || s->recompile || (flags & DO_RECOMPILE)) slCompileShape(s, c->drawMode, textureScale, flags);

	glPushMatrix();
	glTranslated(pos->location.x, pos->location.y, pos->location.z);
	slMatrixGLMult(pos->rotation);

	if(flags & DO_OUTLINE) {
		glPushAttrib(GL_ENABLE_BIT);
		glDisable(GL_LIGHTING);
		glPushMatrix();
		glScalef(.99, .99, .99);
		glPolygonOffset(1, 2);
		glEnable(GL_POLYGON_OFFSET_FILL);
		glCallList(s->drawList);
		glPopMatrix();
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		glColor4f(0, 0, 0, .5);
		glDepthMask(GL_FALSE);
		slRenderShape(s, GL_LINE_LOOP, 0, 0);
		glDepthMask(GL_TRUE);
		glDisable(GL_POLYGON_OFFSET_FILL);

		if(s->_type == ST_SPHERE) {
			glColor4f(1, 1, 1, 0);
			glDisable(GL_DEPTH_TEST);
			glScalef(.96, .96, .96);
			glCallList(s->drawList);
			glEnable(GL_DEPTH_TEST);
		}

		glPopAttrib();
	} else {
		glCallList(s->drawList);
	}

	if(bound || axis) {
		glPushAttrib(GL_COLOR_BUFFER_BIT);
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		glColor4f(0.0, 0.0, 0.0, 0.5);
		glScalef(1.1, 1.1, 1.1);
		if(axis) slDrawAxis(s->max.x, s->max.y);
		if(bound) slRenderShape(s, GL_LINE_LOOP, 0, 0);
		glPopAttrib();
	}

	glPopMatrix();
}

/*!
	\brief Renders a stationary object.
*/

void slProcessBillboards(slWorld *w, slCamera *c) {
	GLfloat matrix[16];
	slSphere *ss;
	std::vector<slWorldObject*>::iterator wi;

	glGetFloatv(GL_MODELVIEW_MATRIX, matrix);

	c->billboardCount = 0;

	for(wi = w->objects.begin(); wi != w->objects.end(); wi++ ) {
		slWorldObject *wo = *wi;

		if(wo->textureMode != BBT_NONE && wo->shape && wo->shape->_type == ST_SPHERE) {
			double z = 0;

			ss = wo->shape;

			z = matrix[2] * wo->position.location.x + matrix[6] * wo->position.location.y + matrix[10] * wo->position.location.z;

			slAddBillboard(c, wo, ss->_radius, z);
		}
	}

	slSortBillboards(c);

	c->billboardX.x = matrix[0];
	c->billboardX.y = matrix[4];
	c->billboardX.z = matrix[8];

	c->billboardY.x = matrix[1];
	c->billboardY.y = matrix[5];
	c->billboardY.z = matrix[9];

	c->billboardZ.x = matrix[2];
	c->billboardZ.y = matrix[6];
	c->billboardZ.z = matrix[10];

	if(c->billboardDrawList == 0) c->billboardDrawList = glGenLists(1);

	glNewList(c->billboardDrawList, GL_COMPILE);

	glBegin(GL_TRIANGLE_STRIP);
		glTexCoord2f(1.0, 1.0); 
		glVertex3f(c->billboardX.x + c->billboardY.x, c->billboardX.y + c->billboardY.y, c->billboardX.z + c->billboardY.z);
		glTexCoord2f(0.0, 1.0);
		glVertex3f(-c->billboardX.x + c->billboardY.x, -c->billboardX.y + c->billboardY.y, -c->billboardX.z + c->billboardY.z);
		glTexCoord2f(1.0, 0.0); 
		glVertex3f(c->billboardX.x - c->billboardY.x, c->billboardX.y - c->billboardY.y, c->billboardX.z - c->billboardY.z);
		glTexCoord2f(0.0, 0.0);
		glVertex3f(-c->billboardX.x - c->billboardY.x, -c->billboardX.y - c->billboardY.y, -c->billboardX.z - c->billboardY.z);
	glEnd();

	glEndList();
}

/*! 
	\brief Renders the objects, assuming that all necessary transformations 
	have been set up.
*/

void slRenderObjects(slWorld *w, slCamera *camera, unsigned int flags) {
	unsigned int n;
	slWorldObject *wo;
	int color = 1;
	int texture = 1;
	int loadNames = (flags & DO_LOAD_NAMES);

	if(flags & DO_NO_TEXTURE) texture = 0;

	if(flags & DO_ONLY_ALPHA) {
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		// glBlendFunc(GL_ONE_MINUS_DST_ALPHA,GL_DST_ALPHA);
	} 

	if((flags & DO_OUTLINE) || (flags & DO_NO_COLOR)) color = 0;
	if(flags & DO_OUTLINE) glColor4f(1, 1, 1, 0);

   	glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
   	// glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_DECAL);

	for(n=0;n<w->objects.size();n++) {
		int skip = 0;
		wo = w->objects[n];

		if(!wo || (wo->drawMode & DM_INVISIBLE)) skip = 1;
		else if(wo->alpha != 1.0 && (flags & DO_NO_ALPHA)) skip = 1;
		else if(wo->alpha == 1.0 && (flags & DO_ONLY_ALPHA)) skip = 1;
		else if(wo->type == WO_STATIONARY && (flags & DO_NO_STATIONARY)) skip = 1;
		else if(wo->type == WO_LINK && (flags & DO_NO_LINK)) skip = 1;
		else if(wo->type == WO_TERRAIN && (flags & DO_NO_TERRAIN)) skip = 1;
		else if(wo->textureMode != BBT_NONE && !(flags & DO_BILLBOARDS_AS_SPHERES)) skip = 1;

		if(!skip) {
			if(loadNames) glLoadName(n);

			if(color) glColor4f(wo->color.x, wo->color.y, wo->color.z, wo->alpha);

			if(wo->alpha != 1.0) {
			// 	glBlendFunc(GL_SRC_ALPHA, GL_ONE);
				glDisable(GL_CULL_FACE);
			}

			// 0 or -1 can be used to indicate no texture

			if(texture && wo->texture > 0) {
				glBindTexture(GL_TEXTURE_2D, wo->texture);
				glEnable(GL_TEXTURE_2D);
			} 

			wo->draw(camera);

			if(wo->alpha != 1.0) glEnable(GL_CULL_FACE);

			if(wo->texture > 0) glDisable(GL_TEXTURE_2D);
		}
	}

	if(flags & DO_ONLY_ALPHA) glDisable(GL_BLEND);

	if(loadNames) glLoadName(w->objects.size() + 1);
}

/*!
	\brief Renders object neighbor lines.
*/

void slRenderLines(slWorld *w, slCamera *c) {
	unsigned int n;
	unsigned int m;
	slVector *x, *y;
	slWorldObject *neighbor;

	glLineWidth(1.2);

	for(n=0;n<w->objects.size();n++) {
		if(w->objects[n] && !(w->objects[n]->drawMode & DM_INVISIBLE)) {
			if(w->objects[n]->drawMode & DM_NEIGHBOR_LINES) {
				glEnable(GL_BLEND);
				glColor4f(0.0, 0.0, 0.0, 0.5);

				x = &w->objects[n]->position.location;

				glBegin(GL_LINES);

				for(m=0;m<w->objects[n]->neighbors->count;m++) {
					neighbor = w->objects[n]->neighbors->data[m];

					if(neighbor) {
						y = &neighbor->position.location;

						glVertex3f(x->x, x->y, x->z);
						glVertex3f(y->x, y->y, y->z);
					}
				}

				glEnd();
				glDisable(GL_BLEND);
			}
		}
	}

	glDisable(GL_LIGHTING);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	std::vector<slObjectLine>::iterator li;

	for(li = w->connections.begin(); li != w->connections.end(); li++ ) {
		slObjectLine &line = *li;

		x = &line._src->position.location;
		y = &line._dst->position.location;

		if(line._stipple) {
			glLineStipple(2, line._stipple);
			glEnable(GL_LINE_STIPPLE);
		}

		glColor4f(line._color.x, line._color.y, line._color.z, 0.8);

		glBegin(GL_LINES);

		glVertex3f(x->x, x->y, x->z);
		glVertex3f(y->x, y->y, y->z);

		glEnd();

		glDisable(GL_LINE_STIPPLE);
	}
}

/*!
	\brief Renders an x/y axis of a given length.
*/

void slDrawAxis(double x, double y) {
	x += .02;
	y += .02;

	glBegin(GL_LINES);
		glVertex3f(0, 0, 0);
		glVertex3f(x, 0, 0);
		glVertex3f(0, y, 0);
		glVertex3f(0, 0, 0);
	glEnd();

	glPushMatrix();
	glTranslated(x + 0.2, -0.2, 0);
	glScaled(.005, .005, .005);
	glutStrokeCharacter(GLUT_STROKE_ROMAN, 'x');
	glPopMatrix();

	glPushMatrix();
	glTranslated(-0.2, y + 0.2, 0);
	glScaled(.005, .005, .005);
	glutStrokeCharacter(GLUT_STROKE_ROMAN, 'y');
	glPopMatrix();
}

/*!
	\brief Compiles a shape into a draw list.

	The draw list is generated or updated as needed.
*/

int slCompileShape(slShape *s, int drawMode, double textureScale, int flags) {
	if(s->drawList == 0) s->drawList = glGenLists(1);
	s->recompile = 0;

	glNewList(s->drawList, GL_COMPILE);

	slRenderShape(s, drawMode, textureScale, flags);

	glEndList();

	return s->drawList;
}

/*!
	\brief Render a shape.

	Typically only called when compiling a drawlist for a shape, this does
	the actual rendering.
*/

void slRenderShape(slShape *s, int drawMode, double textureScale, int flags) {
	int divisions;
	GLUquadricObj *quad;

	if(s->_type == ST_SPHERE) {
		double radius = ((slSphere*)s)->_radius;

		if(radius < 10) divisions = 10;
		else divisions = (int)radius;

		quad = gluNewQuadric();

		if(drawMode != GL_POLYGON) gluQuadricDrawStyle(quad, GLU_LINE);
		gluQuadricTexture(quad, GL_TRUE);

		gluQuadricOrientation(quad, GLU_OUTSIDE);

		gluSphere(quad, radius, divisions, divisions);

		gluDeleteQuadric(quad);
	} else {
		std::vector<slFace*>::iterator fi;

		for(fi = s->faces.begin(); fi != s->faces.end(); fi++ )
			slDrawFace(*fi, drawMode, textureScale, flags);
	}
}

/*!
	\brief Gives vectors perpendicular to v.

	Uses cross-products to find two vectors perpendicular to v.
	Uses either (0, 1, 0) or (1, 0, 0) as the first cross product
	vector (depending on whether v is already set to one of these).
*/

void slPerpendicularVectors(slVector *v, slVector *p1, slVector *p2) {
	slVector neg;

	slVectorSet(p1, 0, 1, 0);

	slVectorMul(v, -1, &neg);

	if(!slVectorCompare(p1, v) || !slVectorCompare(&neg, p1))  slVectorSet(p1, 1, 0, 0);
	
	slVectorCross(p1, v, p2);
	slVectorCross(p2, v, p1);
	slVectorNormalize(p1);
	slVectorNormalize(p2);
}

/*!
	\brief Draws a face, breaking it down into smaller triangles if necessary.
*/

void slDrawFace(slFace *f, int drawMode, double textureScale, int flags) {
	slVector *v, *norm, xaxis, yaxis;
	slPoint *p;
	int edgeCount;

	norm = &f->plane.normal;

	slPerpendicularVectors(norm, &xaxis, &yaxis);

	glNormal3f(norm->x, norm->y, norm->z);

	if(f->drawFlags & SD_REFLECT) {
		GLfloat v[4];

		glPushAttrib(GL_ENABLE_BIT);

		glGetFloatv(GL_CURRENT_COLOR, v);
		v[3] = gReflectionAlpha;
		glColor4fv(v);
		glEnable(GL_BLEND);
	}

	// if they're drawing lines, or if the face 
	// isn't broken down, do a normal polygon

	if(drawMode == GL_LINE_LOOP || !slBreakdownFace(f, textureScale)) {
		glBegin(drawMode);
	
		for(edgeCount=0;edgeCount<f->edgeCount;edgeCount++) {
			p = (slPoint*)f->points[edgeCount];

			v = &((slPoint*)p)->vertex;
	
			glTexCoord2f(slVectorDot(v, &xaxis) / textureScale + 0.5, slVectorDot(v, &yaxis) / textureScale + 0.5);

			glVertex3d(v->x, v->y, v->z);
		} 

		glEnd();
	}

	if(f->drawFlags & SD_REFLECT) {
		glPopAttrib();
	}
}

/*!
	\brief Recursively break down and draw a face.
	
	Breaks down faces into smaller polygons in order to improve 
	the quality of lighting and other effects.
*/

int slBreakdownFace(slFace *f, double textureScale) {
	slVector diff, total, subv[3], middle;
	slVector *v1, *v2;
	slVector xaxis, yaxis;
	slPoint *p;
	int n, n2;
	double length = 0;

	slPerpendicularVectors(&f->plane.normal, &xaxis, &yaxis);

	slVectorZero(&total);
	for(n=0;n<f->edgeCount;n++) {
		n2 = n+1;
		if(n2 == f->edgeCount) n2 = 0;
	
		p = f->points[n];
		v1 = &p->vertex;
		p = f->points[n2];
		v2 = &p->vertex;
	
		slVectorSub(v1, v2, &diff);
		slVectorAdd(v1, &total, &total);

		length += slVectorLength(&diff);
	} 

	slVectorMul(&total, 1.0 / f->edgeCount, &total);

	if(length < 30) return 0;

	glBegin(GL_TRIANGLES);
	
	for(n=0;n<f->edgeCount;n++) {
		n2 = n+1;
		if(n2 == f->edgeCount) n2 = 0;
	
		p = f->points[n];
		v1 = &p->vertex;
		p = f->points[n2];
		v2 = &p->vertex;

		slVectorSub(v2, v1, &diff);
		slVectorMul(&diff, .5, &diff);
		slVectorAdd(v1, &diff, &middle);

		slVectorCopy(v1, &subv[0]);
		slVectorCopy(&middle, &subv[1]);
		slVectorCopy(&total, &subv[2]);

		slBreakdownTriangle(&subv[0], textureScale, 0, &xaxis, &yaxis);

		slVectorCopy(&middle, &subv[0]);
		slVectorCopy(v2, &subv[1]);
		slVectorCopy(&total, &subv[2]);

		slBreakdownTriangle(&subv[0], textureScale, 0, &xaxis, &yaxis);
	}
	
	glEnd();

	return 1;
}

/*!
	\brief Recursively break down and draw a triangle.

	Used by \ref slBreakdownFace to break down and draw large polygons
	in order to improve the quality of lighting and other effects.
*/

void slBreakdownTriangle(slVector *v, double textureScale, int level, slVector *xaxis, slVector *yaxis) {
	slVector subv[3];
	slVector mids[3];
	slVector diff;
	double length = 0;
	int n, n2;

	for(n=0;n<3;n++) {
		n2 = n+1;
		if(n2 == 3) n2 = 0;

		slVectorSub(&v[n2], &v[n], &diff);

		length += slVectorLength(&diff);

		slVectorMul(&diff, .5, &diff);
		slVectorAdd(&v[n], &diff, &mids[n]);
	}

	if(length < 200 || level > 3) {
		for(n=0;n<3;n++) {
			glTexCoord2f(slVectorDot(&v[n], xaxis) / textureScale + 0.5, slVectorDot(&v[n], yaxis) / textureScale + 0.5);
			glVertex3f(v[n].x, v[n].y, v[n].z);
		}

		return;
	} else {
		slVectorCopy(&v[0], &subv[0]);
		slVectorCopy(&mids[0], &subv[1]);
		slVectorCopy(&mids[2], &subv[2]);

		slBreakdownTriangle(&subv[0], textureScale, level+1, xaxis, yaxis);

		slVectorCopy(&mids[0], &subv[0]);
		slVectorCopy(&v[1], &subv[1]);
		slVectorCopy(&mids[1], &subv[2]);

		slBreakdownTriangle(&subv[0], textureScale, level+1, xaxis, yaxis);

		slVectorCopy(&mids[1], &subv[0]);
		slVectorCopy(&v[2], &subv[1]);
		slVectorCopy(&mids[2], &subv[2]);

		slBreakdownTriangle(&subv[0], textureScale, level+1, xaxis, yaxis);

		slVectorCopy(&mids[0], &subv[0]);
		slVectorCopy(&mids[1], &subv[1]);
		slVectorCopy(&mids[2], &subv[2]);

		slBreakdownTriangle(&subv[0], textureScale, level+1, xaxis, yaxis);
	}
}

/*!
	\brief Prints out and clears OpenGL errors.
*/

__inline__ int slClearGLErrors(char *id) {
	unsigned int n;
	int c = 0;

	while((n = glGetError())) {
		slMessage(DEBUG_ALL, "%s: OpenGL error %s\n", id, gluErrorString(n));
		c++;
	}

	return c;
}

/*
	\brief Frees memory associated with OpenGL draw lists and textures
*/

void slFreeGL(slWorld *w, slCamera *c) {
	unsigned int n;

	for(n=0;n<w->objects.size();n++) {
		if(w->objects[n] && w->objects[n]->shape) {
			glDeleteLists(w->objects[n]->shape->drawList, 1);
			w->objects[n]->shape->drawList = 0;
		}
	}
}
