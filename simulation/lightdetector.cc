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
#include "world.h"
#include "camera.h"

#define WHITE_PIXEL	0x00ffffff

void slCamera::detectLightExposure( slWorld *w, int size, GLubyte *buffer ) {
	slWorldObject *wo;
	std::vector< slWorldObject* >::iterator wi;
	unsigned int n = 0;
	int x;

	static int bufferSize = 0;
	static GLubyte *staticBuffer = NULL;

	GLubyte *expMap;

	slVector *sun, *target;

	sun = &w->_lightExposureCamera._location;
	target = &w->_lightExposureCamera._target;

	if( activateContextCallback && activateContextCallback() != 0) {
		slMessage(DEBUG_ALL, "Cannot simulate light exposure: no OpenGL context available\n");
		return;
	}

	if( sun->y < target->y ) {
		// no exposure -- zero out the existing values
		for(wi = w->objects.begin(); wi != w->objects.end(); wi++ )
			(*wi)->_lightExposure = 0;

		return;
	}

	if( size * size * 3 > bufferSize ) {
		if(staticBuffer) delete[] staticBuffer;

		staticBuffer = new GLubyte[size * size * 3];

		bufferSize = size * size * 3;
	}

	if(w->objects.size() == 0) return;

	glDisable( GL_LIGHTING );
	glDisable( GL_BLEND );
	glDisable( GL_TEXTURE_2D );
	glEnable( GL_DEPTH_TEST );
	glDepthMask( GL_TRUE );
	glDepthFunc( GL_LESS );
	glShadeModel( GL_FLAT );


	glEnable(GL_SCISSOR_TEST);
	glScissor(  _originx, _originy, size, size );
	glViewport( _originx, _originy, size, size );

	glMatrixMode( GL_PROJECTION );
	glLoadIdentity();

	glOrtho(-155, 155, -155, 155, 1.0, 1000.0);

    // gluPerspective(90.0, 1.0, 1.0, 1000.0);


	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glLoadIdentity();

	gluLookAt( sun->x, sun->y, sun->z, 
		target->x, target->y, target->z, 0.0, 1.0, 0.0 );

	glClearColor( 1, 1, 1, 1 );
	glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);

	for(wi = w->objects.begin(); wi != w->objects.end(); wi++) {
		unsigned char br, bg, bb;

		br = n / (256 * 256);
		bg = n / 256;
		bb = n % 256;

		glColor4ub(br, bg, bb, 0xff);

		wo = *wi;
		wo->_lightExposure = 0;

		if( wo->_shape ) wo->_shape->draw( this, &wo->_position, 0, 0, 0, 0 );

		n++;
	}

	glPopMatrix();

	if(!buffer) {
		glReadPixels( _originx, _originy, size, size, GL_RGB, GL_UNSIGNED_BYTE, staticBuffer);
		expMap = staticBuffer;
	} else {
		expMap = buffer;
	}

	for(x=0;x<bufferSize;x+=3) {
		unsigned int label;

		label = (expMap[x] << 16) + (expMap[x+1] << 8) + expMap[x+2];

		if(label != WHITE_PIXEL && label < w->objects.size()) {
			wo = w->objects[label];
			wo->_lightExposure++;
		}
	}

	glDepthMask( GL_TRUE );

	glDisable(GL_SCISSOR_TEST);
}
