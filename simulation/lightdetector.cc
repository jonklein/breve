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

#define WHITE_PIXEL	0x00ffffff

// ( pssst.  i can't believe this crap works! )

void slDetectLightExposure(slWorld *w, slCamera *c, int size, GLubyte *buffer) {
	slWorldObject *wo;
	std::vector<slWorldObject*>::iterator wi;
	unsigned int n;
	int x;

	static int bufferSize = 0;
	static GLubyte *staticBuffer = NULL;

	GLubyte *expMap;

	slVector *sun, *target;

	sun = &w->lightExposureSource;
	target = &w->lightExposureTarget;

	if(sun->y < target->y) {
		// no exposure -- zero out the existing values
		for(wi = w->objects.begin(); wi != w->objects.end(); wi++ )
			(*wi)->lightExposure = 0;

		return;
	}

	if(size * size * 3 > bufferSize) {
		if(staticBuffer) delete[] staticBuffer;

		staticBuffer = new GLubyte[size * size * 3];

		bufferSize = size * size * 3;
	}

	if(w->objects.size() == 0) return;

	glDisable(GL_LIGHTING);
	glDisable(GL_BLEND);
	glDisable(GL_TEXTURE_2D);
	glEnable(GL_DEPTH_TEST);
	glDepthMask(GL_TRUE);
	glDepthFunc(GL_LESS);
	glShadeModel(GL_FLAT);

	glViewport(c->ox, c->oy, size, size);

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();

	gluPerspective(90.0, 1.0, 1.0, 1000.0);

	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glLoadIdentity();

	gluLookAt(sun->x, sun->y, sun->z, 
		target->x, target->y, target->z, 0.0, 1.0, 0.0);

	glClearColor(1, 1, 1, 1);
	glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);

	for(wi = w->objects.begin(); wi != w->objects.end(); wi++) {
		unsigned char br, bg, bb;
		slVector color;

		br = n / (256 * 256);
		bg = n / 256;
		bb = n % 256;

		glColor4ub(br, bg, bb, 0xff);

		wo = *wi;
		wo->lightExposure = 0;

		if(wo->shape) slDrawShape(c, wo->shape, &wo->position, &color, 0, 0, 0, 0, DO_NO_LIGHTING|DO_NO_COLOR|DO_NO_TEXTURE, 0, 1.0);
	}

	glPopMatrix();

	if(!buffer) {
		glReadPixels(c->ox, c->oy, size, size, GL_RGB, GL_UNSIGNED_BYTE, staticBuffer);
		expMap = staticBuffer;
	} else {
		expMap = buffer;
	}

	for(x=0;x<bufferSize;x+=3) {
		unsigned int label;

		label = (expMap[x] << 16) + (expMap[x+1] << 8) + expMap[x+2];

		if(label != WHITE_PIXEL && label < w->objects.size()) {
			wo = w->objects[label];
			wo->lightExposure++;
		}
	}

	glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
	glDepthMask(GL_TRUE);
}
