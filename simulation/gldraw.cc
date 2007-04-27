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
#include "gldraw.h"
#include "drawcommand.h"
#include "world.h"
#include "lightdetector.h"
#include "tiger.h"
#include "shadowvolume.h"
#include "asciiart.h"

#define REFLECTION_ALPHA	.75

#define LIGHTSIZE 64

double gReflectionAlpha;

int glActive = 0;

/*!
	\brief Calls glMultMatrix with 3x3 orientation slMatrix.

	Swaps the rows and columns of the matrix (since GL matrices
	are the opposite of SL matrices).
*/

void slMatrixGLMult( double m[3][3] ) {
	double d[4][4];

	d[0][0] = m[0][0];
	d[0][1] = m[1][0];
	d[0][2] = m[2][0];
	d[0][3] = 0;
	d[1][0] = m[0][1];
	d[1][1] = m[1][1];
	d[1][2] = m[2][1];
	d[1][3] = 0;
	d[2][0] = m[0][2];
	d[2][1] = m[1][2];
	d[2][2] = m[2][2];
	d[2][3] = 0;
	d[3][0] = 0;
	d[3][1] = 0;
	d[3][2] = 0;
	d[3][3] = 1;

	glMultMatrixd( (double*)d );
}

/*!
	\brief Creates the texture used for the "lightmaps".
*/

void slMakeLightTexture( GLubyte *lTexture, GLubyte *dlTexture ) {
	double x, y, temp, dtemp;
	int i, j;

	for ( i = 0; i < LIGHTSIZE; i++ ) {
		for ( j = 0; j < LIGHTSIZE; j++ ) {
			x = ( float )( i - LIGHTSIZE / 2.0 ) / ( float )( LIGHTSIZE / 2.0 );
			y = ( float )( j - LIGHTSIZE / 2.0 ) / ( float )( LIGHTSIZE / 2.0 );

			temp = ( 1.0f - ( float )( sqrt(( x * x ) + ( y * y ) ) ) ) * 1.2;
			dtemp = temp + slRandomDouble() / 10.0;

			if ( temp > 1.0f ) temp = 1.0f;
			if ( temp < 0.0f ) temp = 0.0f;

			if ( dtemp > 1.0f ) dtemp = 1.0f;
			if ( dtemp < 0.0f ) dtemp = 0.0f;

			lTexture[( i * LIGHTSIZE * 2 ) + j * 2] = ( unsigned char )( 255.0f * temp * temp );
			lTexture[( i * LIGHTSIZE * 2 ) + ( j * 2 ) + 1] = ( unsigned char )( 255.0f * temp * temp );

			dlTexture[( i * LIGHTSIZE * 2 ) + j * 2] = ( unsigned char )( 255.0f * dtemp * dtemp );
			dlTexture[( i * LIGHTSIZE * 2 ) + ( j * 2 ) + 1] = ( unsigned char )( 255.0f * temp * temp );
		}
	}
}

void slInitGL( slWorld *w, slCamera *c ) {
	GLfloat specularColor[4] = { 0.2, 0.2, 0.2, 0.0 };
	GLubyte lt[LIGHTSIZE * LIGHTSIZE * 2];
	GLubyte glt[LIGHTSIZE * LIGHTSIZE * 2];

	glActive = 1;

	gReflectionAlpha = REFLECTION_ALPHA;

	slMakeLightTexture( &lt[0], &glt[0] );

	slUpdateTexture( c, slTextureNew( c ), gBrickImage, TEXTURE_WIDTH, TEXTURE_HEIGHT, GL_RGBA );
	slUpdateTexture( c, slTextureNew( c ), gPlaid, TEXTURE_WIDTH, TEXTURE_HEIGHT, GL_RGBA );
	slUpdateTexture( c, slTextureNew( c ), lt, LIGHTSIZE, LIGHTSIZE, GL_LUMINANCE_ALPHA );
	slUpdateTexture( c, slTextureNew( c ), glt, LIGHTSIZE, LIGHTSIZE, GL_LUMINANCE_ALPHA );

	glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );

	glEnable( GL_DEPTH_TEST );

	glEnable( GL_CULL_FACE );
	glCullFace( GL_BACK );

	glMatrixMode( GL_MODELVIEW );
	glLoadIdentity();

	glEnable( GL_LINE_SMOOTH );
	glEnable( GL_POINT_SMOOTH );
	glLineWidth( 2 );

	glPolygonOffset( -4.0f, -1.0f );

	glHint( GL_LINE_SMOOTH_HINT, GL_NICEST );
	glHint( GL_POLYGON_SMOOTH_HINT, GL_NICEST );
	glHint( GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST );
	glEnable( GL_COLOR_MATERIAL );
	glColorMaterial( GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE );

	glPixelStorei( GL_UNPACK_ALIGNMENT, 1 );
	glPixelStorei( GL_PACK_ALIGNMENT, 1 );

	slClearGLErrors( "init" );

	glMaterialfv( GL_FRONT_AND_BACK, GL_SPECULAR, specularColor );
	glMaterialf( GL_FRONT_AND_BACK, GL_SHININESS, 90 );
}

/**
 * Initializes 3 sphere draw lists.
 */

void slCompileSphereDrawList( int l ) {
	GLUquadricObj *quad;

	for ( int n = 0; n < SPHERE_RESOLUTIONS; ++n ) {
		glNewList( l + n, GL_COMPILE );

		quad = gluNewQuadric();

		gluQuadricTexture( quad, GL_TRUE );
		gluQuadricOrientation( quad, GLU_OUTSIDE );
		gluSphere( quad, 100.0, 6 + n * 3, 6 + n * 3 );

		gluDeleteQuadric( quad );

		glEndList();
	}
}

/**
 * \brief Center the given pixels in a square buffer.
 * Used for textures, which must be powers of two.
 */

void slCenterPixelsInSquareBuffer( unsigned char *pixels, int width, int height, unsigned char *buffer, int newwidth, int newheight ) {
	int xstart, ystart;
	int y;

	xstart = ( newwidth - width ) / 2;
	ystart = ( newheight - height ) / 2;

	for ( y = 0; y < height; y++ )
		memcpy( &buffer[( y + ystart ) *( newwidth * 4 ) + ( xstart * 4 )],
		        &pixels[y * width * 4],
		        width * 4 );
}

/**
 * Allocates space for a new texture.
 */

unsigned int slTextureNew( slCamera *c ) {
	GLuint texture;

	if ( !glActive ) return 0;

	if ( c->_activateContextCallback ) c->_activateContextCallback();

	glGenTextures( 1, &texture );

	return texture;
}

void slTextureFree( slCamera *c, const unsigned int n ) {
	if ( !glActive )
		return;

	if ( c->_activateContextCallback ) c->_activateContextCallback();

	glDeleteTextures( 1, ( GLuint * )&n );
}

/**
 * \brief Adds (or updates) a texture to the camera.
 * Returns 0 if there was space, or -1 if all texture positions are used.
 */

int slUpdateTexture( slCamera *c, GLuint texture, unsigned char *pixels, int width, int height, int format ) {
	unsigned char *newpixels = NULL;
	int newheight, newwidth;

	if ( !glActive )
		return -1;

	if ( c->_activateContextCallback )
		c->_activateContextCallback();

	newwidth = slNextPowerOfTwo( width );

	newheight = slNextPowerOfTwo( height );

	newwidth = newheight = MAX( newwidth, newheight );

	if ( newwidth != width || newheight != height ) {
		newpixels = new unsigned char[newwidth * newheight * 4];
		memset( newpixels, 0, newwidth * newheight * 4 );

		slCenterPixelsInSquareBuffer( pixels, width, height, newpixels, newwidth, newheight );
	} else
		newpixels = pixels;

	glBindTexture( GL_TEXTURE_2D, texture );

	glPixelStorei( GL_UNPACK_ALIGNMENT, 1 );

	glTexImage2D( GL_TEXTURE_2D, 0, format, newwidth, newheight, 0, format, GL_UNSIGNED_BYTE, newpixels );

	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT );

	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT );

	glTexEnvf( GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE );

	if ( newpixels != pixels )
		delete[] newpixels;

	if ( slClearGLErrors( "error adding texture" ) )
		return -1;

	return texture;
}

int slCamera::select( slWorld *w, int x, int y ) {

	slVector cam;
	GLuint *selections;
	GLuint namesInHit, selection_buffer[ BUFFER_SIZE ];
	GLint hits, viewport[4];
	unsigned int min, nearest = 0xffffffff;
	unsigned int hit = w->_objects.size() + 1;

	viewport[ 0 ] = _originx;
	viewport[ 1 ] = _originy;
	viewport[ 2 ] = _width;
	viewport[ 3 ] = _height;

	glSelectBuffer( BUFFER_SIZE, selection_buffer );
	glRenderMode( GL_SELECT );
	slClearGLErrors( "selected buffer" );

	glInitNames();
	glPushName( 0 );

	glMatrixMode( GL_PROJECTION );
	glPushMatrix();
	glLoadIdentity();

	gluPickMatrix(( GLdouble )x, ( GLdouble )( viewport[3] - y ), 5.0, 5.0, viewport );
	slClearGLErrors( "picked matrix" );

	gluPerspective( 40.0, _fov, _frontClip, _zClip );

	// since the selection buffer uses unsigned ints for names, we can't
	// use -1 to mean no selection -- we'll use the number of objects
	// plus 1 to indicate that no selectable object was selected.

	glLoadName( w->_objects.size() + 1 );

	slClearGLErrors( "about to select" );

	slVectorAdd( &_location, &_target, &cam );
	gluLookAt( cam.x, cam.y, cam.z, _target.x, _target.y, _target.z, 0.0, 1.0, 0.0 );

	// Render the objects in the world, with name loading, and with billboards as spheres

	renderObjects( w, DO_BILLBOARDS_AS_SPHERES | DO_LOAD_NAMES );

	hits = glRenderMode( GL_RENDER );

	glMatrixMode( GL_PROJECTION );
	glPopMatrix();

	slClearGLErrors( "picked" );

	selections = &selection_buffer[0];

	for ( int n = 0; n < hits; ++n ) {
		namesInHit = *selections++;

		// skip over the z-max value

		selections++;
		min = *selections++;

		if ( min < nearest ) {
			nearest = min;

			while ( namesInHit-- )
				hit = *selections++;
		} else
			while ( namesInHit-- )
				selections++;
	}

	glPopName();

	if ( hit == w->_objects.size() + 1 )
		return -1;

	return hit;
}

/**
 * Computes the vector corresponding to a drag in the display.
 * 
 * Computes the location in the same plane as dragVertex that the mouse
 * is being dragged to when the window mouse coordinates are x and y.
 */

int slCamera::vectorForDrag( slWorld *w, slVector *dragVertex, int x, int y, slVector *dragVector ) {
	slPlane plane;
	slVector cam, end;
	GLdouble model[16];
	GLdouble proj[16];
	GLdouble wx, wy;
	GLdouble oxf, oyf, ozf;
	double sD, eD;
	slVector *t;
	GLint view[4];

	// set up the matrices for a regular draw--gluUnProject needs this

	t = &_target;

	view[0] = _originx;
	view[1] = _originy;
	view[2] = _width;
	view[3] = _height;

	glViewport( _originx, _originy, _width, _height );
	glMatrixMode( GL_PROJECTION );
	glLoadIdentity();

	gluPerspective( 40.0, _fov, _frontClip, _zClip );

	glMatrixMode( GL_MODELVIEW );
	glLoadIdentity();

	slVectorAdd( &_location, t, &cam );
	gluLookAt( cam.x, cam.y, cam.z, t->x, t->y, t->z, 0.0, 1.0, 0.0 );

	// get the data for gluUnProject

	glGetDoublev( GL_MODELVIEW_MATRIX, model );
	glGetDoublev( GL_PROJECTION_MATRIX, proj );

	y = view[3] - y;

	wx = x;
	wy = y;

	// use gluUnProject to get the point in object space where we are clicking
	// (and at the far reach of our zClip variable).

	( void )gluUnProject( wx, wy, 1.0, model, proj, view, &oxf, &oyf, &ozf );

	end.x = oxf;
	end.y = oyf;
	end.z = ozf;

	// define the plane where the object in question lies

	slVectorCopy( &_location, &plane.normal );
	slVectorNormalize( &plane.normal );
	slVectorCopy( dragVertex, &plane.vertex );

	//	compute the distance
	//		1) from the camera to the object plane
	//		2) from the zClip plane to the object plane

	sD = slPlaneDistance( &plane, &cam );
	eD = slPlaneDistance( &plane, &end );

	eD = fabs( eD );
	sD = fabs( sD );

	// compute the vector from the camera to the end of the zClip plane.
	// this is the vector containing all of the candidate points that the
	// user is dragging--we need to figure out which one we're interested in

	slVectorSub( &end, &cam, dragVector );

	// compute the point on the object plane of the drag vector

	slVectorMul( dragVector, ( sD / ( eD + sD ) ), dragVector );
	slVectorAdd( dragVector, &cam, dragVector );

	glLoadIdentity();

	return 0;
}

void slCamera::renderScene( slWorld *w, int crosshair ) {
	std::vector< slCamera* >::iterator ci;

	if ( w->detectLightExposure() && !w->drawLightExposure() )
		detectLightExposure( w, 200, NULL );

	renderWorld( w, crosshair, 0 );

	for ( ci = w->_cameras.begin(); ci != w->_cameras.end(); ci++ )
		if ( *ci != this )( *ci )->renderWorld( w, 0, 1 );

	if ( w->detectLightExposure() && w->drawLightExposure() )
		detectLightExposure( w, 200, NULL );
}

void slCamera::renderWorld( slWorld *w, int crosshair, int scissor ) {
	slVector cam;
	int flags = 0;

	if ( !w ) return;

	glViewport( _originx, _originy, _width, _height );

	if ( scissor ) {
		flags |= DO_NO_AXIS | DO_NO_BOUND;
		glEnable( GL_SCISSOR_TEST );
		glScissor( _originx, _originy, _width, _height );
	}

	if ( _drawOutline )
		flags |= DO_OUTLINE | DO_BILLBOARDS_AS_SPHERES;

	if ( _recompile ) {
		_recompile = 0;
		flags |= DO_RECOMPILE;
	}

	clear( w );

	glMatrixMode( GL_PROJECTION );
	glLoadIdentity();

	if ( w->backgroundTexture > 0 && !( flags & DO_OUTLINE ) )
		drawBackground( w );

	gluPerspective( 40.0, _fov, _frontClip, _zClip );

	glEnable( GL_DEPTH_TEST );

	glMatrixMode( GL_MODELVIEW );

	glPushMatrix();

	glLoadIdentity();

	slVectorAdd( &_location, &_target, &cam );

	gluLookAt( cam.x, cam.y, cam.z, _target.x, _target.y, _target.z, 0.0, 1.0, 0.0 );

	updateFrustum();

	// w->_skybox.draw( &cam );

	drawFog();

	//
	// Lines and draw-commands are special objects which will be rendered before lighting is setup
	//

	std::vector<slDrawCommandList*>::iterator di;
	for ( di = w->_drawings.begin(); di != w->_drawings.end(); di++ )( *di )->draw( this );

	// renderLines( w );

	//
	// Setup lighting and effects for the normal objects
	//

	if ( _drawLights ) {
		if ( _drawShadowVolumes ) 
			drawLights( 1 );
		else 
			drawLights( 0 );

		if ( _shadowCatcher && ( _drawReflection || _drawShadow ) )
			stencilFloor();

		if ( _drawReflection && !( flags & DO_OUTLINE ) ) {
			slVector toCam;

			if ( !_drawShadowVolumes ) gReflectionAlpha = REFLECTION_ALPHA;
			else gReflectionAlpha = REFLECTION_ALPHA - 0.1;

			slVectorSub( &cam, &_shadowPlane.vertex, &toCam );

			if ( slVectorDot( &toCam, &_shadowPlane.normal ) > 0.0 ) {
				reflectionPass( w );

				if ( _drawShadowVolumes ) 
					drawLights( 1 );
			}
		}
	} else
		glDisable( GL_LIGHTING );

	renderObjects( w, flags | DO_NO_ALPHA );

	slClearGLErrors( "drew multibodies and lines" );

	// now we do transparent objects and billboards.  they have to come last
	// because they are blended.

	if ( !( flags & DO_BILLBOARDS_AS_SPHERES ) ) {
		processBillboards( w );
		renderBillboards( flags );
	}

	std::vector<slPatchGrid*>::iterator pi;

	for ( pi = w->_patches.begin(); pi != w->_patches.end(); pi++ )
		( *pi )->draw( this );

	if ( _drawLights ) {
		if ( _drawShadowVolumes ) renderShadowVolume( w );
		else if ( _drawShadow ) shadowPass( w );
	}

	glDepthMask( GL_FALSE );

	renderObjects( w, flags | DO_ONLY_ALPHA );

	glDepthMask( GL_TRUE );

	renderLabels( w );

#if HAVE_LIBENET
	slDrawNetsimBounds( w );
#endif

	if ( w->gisData ) w->gisData->draw( this );

	glPopMatrix();

	glMatrixMode( GL_PROJECTION );

	glLoadIdentity();

	if ( _drawText ) 
		renderText( w, crosshair );

	if ( _drawText && crosshair && !scissor ) {
		glPushMatrix();
		glColor3f( 0, 0, 0 );
		slText( 0, 0, "+", GLUT_BITMAP_9_BY_15 );
		glPopMatrix();
	}

	if ( scissor ) {
		double minY, maxY, minX, maxX;

		minX = -1;
		maxX =  1;
		minY = -1;
		maxY =  1;

		glLineWidth( 2.0 );

		glMatrixMode( GL_PROJECTION );
		glLoadIdentity();

		glColor4f( 0, 0, 0, .5 );
		glBegin( GL_LINES );
		glVertex2f( minX, minY );
		glVertex2f( maxX, minY );

		glVertex2f( maxX, minY );
		glVertex2f( maxX, maxY );

		glVertex2f( maxX, maxY );
		glVertex2f( minX, maxY );

		glVertex2f( minX, maxY );
		glVertex2f( minX, minY );
		glEnd();

		glDisable( GL_SCISSOR_TEST );
	}
}

void slCamera::clear( slWorld *w ) {
	if ( _drawOutline )
		glClearColor( 1, 1, 1, 0 );
	else
		glClearColor( w->backgroundColor.x, w->backgroundColor.y, w->backgroundColor.z, 1.0 );

	if ( !_drawBlur )
		glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT );
	else {
		glEnable( GL_BLEND );
		glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );

		glColor4f( w->backgroundColor.x, w->backgroundColor.y, w->backgroundColor.z, 0.5f - ( _blurFactor / 2.0 ) );

		glMatrixMode( GL_PROJECTION );
		glPushMatrix();
		glLoadIdentity();
		gluPerspective( 40.0, _fov, _frontClip, _zClip );
		glBegin( GL_TRIANGLE_STRIP );
		glVertex3f( -5, -4, -3 );
		glVertex3f( 5, -4, -3 );
		glVertex3f( -5, 4, -3 );
		glVertex3f( 5, 4, -3 );
		glEnd();
		glPopMatrix();
		glDisable( GL_BLEND );
		glClear( GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT );
	}
}

void slCamera::drawFog() {
	if ( _drawFog ) {
		GLfloat color[4];

		color[0] = _fogColor.x;
		color[1] = _fogColor.y;
		color[2] = _fogColor.z;
		color[3] = 1.0;

		glEnable( GL_FOG );
		glFogf( GL_FOG_DENSITY, _fogIntensity );
		glHint( GL_FOG_HINT, GL_NICEST );
		glFogi( GL_FOG_MODE, GL_LINEAR );
		glFogf( GL_FOG_START, _fogStart ) ;
		glFogf( GL_FOG_END, _fogEnd );
		glFogfv( GL_FOG_COLOR, color );
	} else {
		glDisable( GL_FOG );
	}
}

/*!
	\brief Puts 1 into the stencil buffer where the shadows and reflections should fall.
*/

void slCamera::stencilFloor() {
	glEnable( GL_STENCIL_TEST );
	glDisable( GL_DEPTH_TEST );
	glColorMask( GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE );
	glStencilFunc( GL_ALWAYS, 1, 0xffffffff );
	glStencilOp( GL_ZERO, GL_ZERO, GL_REPLACE );

	_shadowCatcher->draw( this );

	glColorMask( GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE );
	glEnable( GL_DEPTH_TEST );
	glDisable( GL_STENCIL_TEST );
}

/*!
	\brief Draws a reflection of all multibody objects whereever the stencil buffer is equal to 1.
*/

void slCamera::reflectionPass( slWorld *w ) {
	glPushMatrix();

	glScalef( 1.0, -1.0, 1.0 );
	glTranslatef( 0.0, -2 * _shadowPlane.vertex.y, 0.0 );
	drawLights( 0 );

	glCullFace( GL_FRONT );

	// render whereever the buffer is 1, but don't change the values

	glEnable( GL_DEPTH_TEST );
	glEnable( GL_NORMALIZE );
	glEnable( GL_STENCIL_TEST );
	glEnable( GL_BLEND );

	glStencilFunc( GL_EQUAL, 1, 0xffffffff );

	glStencilOp( GL_KEEP, GL_KEEP, GL_KEEP );
	renderObjects( w, DO_NO_STATIONARY | DO_NO_BOUND | DO_NO_AXIS | DO_NO_TERRAIN );

	renderBillboards( 0 );

	glDisable( GL_NORMALIZE );

	glCullFace( GL_BACK );
	glDisable( GL_STENCIL_TEST );
	glDisable( GL_BLEND );

	glPopMatrix();

	drawLights( 0 );
}

/**
 * \brief Shadows multibody objects on to the specified shadow plane, expecting
 * that the stencil buffer has already been set to 3.
 *
 * The stencil buffer will be modified to 2 where the shadows are drawn.
 */

void slCamera::shadowPass( slWorld *w ) {
	GLfloat shadowMatrix[4][4];

	slShadowMatrix( shadowMatrix, &_shadowPlane, &_lights[0].location );

	glPushAttrib( GL_ENABLE_BIT );

	glEnable( GL_STENCIL_TEST );
	glEnable( GL_DEPTH_TEST );
	glStencilFunc( GL_LESS, 0, 0xffffffff );
	glStencilOp( GL_ZERO, GL_ZERO, GL_REPLACE );

	slClearGLErrors( "about to enable polygon" );

	glEnable( GL_POLYGON_OFFSET_FILL );

	glEnable( GL_BLEND );
	glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );

	glColor4f( 0.0, 0.0, 0.0, 0.3 );

	glPushMatrix();
	glMultMatrixf( (GLfloat*)shadowMatrix );
	glDisable( GL_LIGHTING );
	renderObjects( w, DO_NO_COLOR | DO_NO_TEXTURE | DO_NO_STATIONARY | DO_NO_BOUND | DO_NO_AXIS | DO_NO_TERRAIN );

	renderBillboards( DO_NO_COLOR | DO_NO_BOUND );

	glPopMatrix();

	glPopAttrib();
}

/*!
	\brief Render the text associated with the current display.
*/

void slCamera::renderText( slWorld *w, int crosshair ) {
	double fromLeft;
	unsigned int n;
	char textStr[128];

	glDisable( GL_DEPTH_TEST );
	glDisable( GL_STENCIL_TEST );
	glDisable( GL_BLEND );
	glColor4f( _textColor.x, _textColor.y, _textColor.z, 1.0 );
	snprintf( textStr, sizeof( textStr ), "%.2f", w->_age );

	fromLeft = -1.0 + ( 5.0 / _width );
	slText( fromLeft, 1.0 - ( 20.0 / _height ), textStr, GLUT_BITMAP_HELVETICA_10 );

	if ( crosshair ) {
		snprintf( textStr, sizeof( textStr ), "camera: (%.1f, %.1f, %.1f)",
		          _location.x, _location.y, _location.z );
		slText( fromLeft, -1.0 + ( 5.0 / _height ), textStr, GLUT_BITMAP_HELVETICA_10 );
		snprintf( textStr, sizeof( textStr ), "target: (%.1f, %.1f, %.1f)",
		          _target.x, _target.y, _target.z );
		slText( fromLeft, -1.0 + ( 30.0 / _height ), textStr, GLUT_BITMAP_HELVETICA_10 );
	} else {
		for ( n = 0; n < _text.size(); n++ ) {
			glColor4f( _text[n].color.x, _text[n].color.y, _text[n].color.z, 0.9 );
			slStrokeText( _text[n].x, _text[n].y, _text[n].text.c_str(), _textScale, GLUT_STROKE_ROMAN );
		}
	}
}

/*!
	\brief Draw a texture as a background.
*/

void slCamera::drawBackground( slWorld *w ) {
	static float transX = 0.0, transY = 0.0;

	GLfloat textColor[4];

	glDisable( GL_DEPTH_TEST );

	glPushAttrib( GL_LIGHTING_BIT | GL_TRANSFORM_BIT | GL_TEXTURE_BIT | GL_COLOR_BUFFER_BIT );

	glEnable( GL_BLEND );

	glColor4f( w->backgroundTextureColor.x, w->backgroundTextureColor.y, w->backgroundTextureColor.z, 1.0 );

	textColor[0] = w->backgroundTextureColor.x;
	textColor[1] = w->backgroundTextureColor.y;
	textColor[2] = w->backgroundTextureColor.z;
	textColor[3] = 1.0;

	glDisable( GL_LIGHTING );

	glMatrixMode( GL_TEXTURE );

	glLoadIdentity();

	glPushMatrix();

	// this is likely an Apple driver bug, but on my machine it's
	// taking the color from the glColor, while on Nils' machine it's
	// taking in the color from the GL_TEXTURE_ENV_COLOR--we'll just
	// set both.

	if ( w->isBackgroundImage ) {
		textColor[0] = 1.0;
		textColor[1] = 1.0;
		textColor[2] = 1.0;
		glEnable( GL_TEXTURE_2D );
		glTexEnvf( GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE );
		glTexEnvfv( GL_TEXTURE_ENV, GL_TEXTURE_ENV_COLOR, textColor );
	} else {
		glTexEnvfv( GL_TEXTURE_ENV, GL_TEXTURE_ENV_COLOR, textColor );
		glTexEnvf( GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_BLEND );
	}

	transX += _backgroundScrollX;
	transY += _backgroundScrollY;

	glTranslated( transX - ( .8 * 2*_ry ), ( .8 * 2*_rx ) - transY, 0 );
	glDepthRange( 1, .9 );
	glEnable( GL_TEXTURE_2D );
	glBindTexture( GL_TEXTURE_2D, w->backgroundTexture );
	glBegin( GL_QUADS );
	glTexCoord2f( .0, .0 );
	glVertex3f( -1, -1, -.1 );
	glTexCoord2f( 1.0, .0 );
	glVertex3f( 1, -1, -.1 );
	glTexCoord2f( 1.0, 1.0 );
	glVertex3f( 1, 1, -.1 );
	glTexCoord2f( .0, 1.0 );
	glVertex3f( -1, 1, -.1 );
	glEnd();
	glDepthRange( 0, 1 );

	glDisable( GL_BLEND );
	glPopMatrix();
	glPopAttrib();
	glEnable( GL_DEPTH_TEST );
}

void slCamera::renderLabels( slWorld *w ) {
	slWorldObject *wo;
	slVector *l;
	std::vector<slWorldObject*>::iterator wi;

	glDisable( GL_DEPTH_TEST );

	glColor3f( 0, 0, 0 );

	for ( wi = w->_objects.begin(); wi != w->_objects.end(); wi++ ) {
		wo = *wi;

		if ( wo && !wo->_label.empty() ) {
			l = &wo->_position.location;
			glPushMatrix();
			glTranslatef( l->x, l->y, l->z );
			slText( 0, 0, wo->_label.c_str() , GLUT_BITMAP_HELVETICA_10 );
			glPopMatrix();
		}
	}

	glEnable( GL_DEPTH_TEST );
}

/*!
	\brief Renders preprocessed billboards.
*/

void slCamera::renderBillboards( int flags ) {
	slVector normal;
	slBillboardEntry *b;
	unsigned int n;
	int lastTexture = -1;

	slVectorCopy( &_location, &normal );
	slVectorNormalize( &normal );

	if ( !( flags & DO_NO_TEXTURE ) ) {
		glPushAttrib( GL_LIGHTING_BIT | GL_TRANSFORM_BIT | GL_TEXTURE_BIT | GL_COLOR_BUFFER_BIT );
		glEnable( GL_TEXTURE_2D );
		glEnable( GL_BLEND );
		glDisable( GL_LIGHTING );
	}

	glPushAttrib( GL_TRANSFORM_BIT );

	glMatrixMode( GL_TEXTURE );
	glLoadIdentity();
	glPopAttrib();

	// we do want to have a depth test against other objects in the world.
	// but we do our own back-to-front billboard sort and we do not want
	// them fighting in the depth buffer.  so we'll disable depth-buffer
	// writing so that no new info goes there.

	glDepthMask( GL_FALSE );

	for ( n = 0; n < _billboardCount; n++ ) {
		slWorldObject *object;
		int bound;

		glPushMatrix();

		b = _billboards[ n ];

		object = b->object;

		bound = ( object->_drawMode & DM_BOUND );

		if ( !( flags & DO_NO_COLOR ) )
			glColor4f( object->_color.x, object->_color.y, object->_color.z, object->_alpha );

		if ( lastTexture != object->_texture )
			glBindTexture( GL_TEXTURE_2D, object->_texture );

		if ( object->_textureMode == BBT_LIGHTMAP ) {
			glTexEnvf( GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE );
			glBlendFunc( GL_ONE, GL_ONE );
		} else {
			glTexEnvf( GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE );
			glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );
		}

		lastTexture = object->_texture;

		glTranslated( object->_position.location.x, object->_position.location.y, object->_position.location.z );
		glRotatef( object->_billboardRotation, normal.x, normal.y, normal.z );
		glScalef( b->size, b->size, b->size );

		glCallList( _billboardDrawList );

		if ( bound && !( flags & DO_NO_BOUND ) ) {
			if ( !( flags & DO_NO_TEXTURE ) ) 
				glDisable( GL_TEXTURE_2D );

			glPushMatrix();

			glScalef( 1.1, 1.1, 1.1 );
			glColor4f( 0.0, 0.0, 0.0, 1.0 );

			glBegin( GL_LINE_LOOP );
			glVertex3f( _billboardX.x + _billboardY.x,  _billboardX.y + _billboardY.y,  _billboardX.z + _billboardY.z );
			glVertex3f( -_billboardX.x + _billboardY.x, -_billboardX.y + _billboardY.y, -_billboardX.z + _billboardY.z );
			glVertex3f( -_billboardX.x - _billboardY.x, -_billboardX.y - _billboardY.y, -_billboardX.z - _billboardY.z );
			glVertex3f( _billboardX.x - _billboardY.x,  _billboardX.y - _billboardY.y,  _billboardX.z - _billboardY.z );
			glEnd();

			glPopMatrix();

			if ( !( flags & DO_NO_TEXTURE ) ) 
				glEnable( GL_TEXTURE_2D );
		}

		glPopMatrix();
	}

	glDepthMask( GL_TRUE );

	if ( !( flags & DO_NO_TEXTURE ) ) glPopAttrib();
}

void slStrokeText( double x, double y, const char *string, double scale, void *font ) {
	int c;

	glPushMatrix();

	glEnable( GL_BLEND );
	glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );

	glLineWidth( 1.0 );
	glTranslatef( x, y, 0 );

	glScalef( scale * 0.0005, scale * 0.0005, scale * 0.0005 );

	while (( c = *( string++ ) ) != 0 )
		glutStrokeCharacter( font, c );

	glPopMatrix();
}

void slText( double x, double y, const char *string, void *font ) {

	int c;

	glRasterPos2f( x, y );

	while (( c = *( string++ ) ) != 0 ) 
		glutBitmapCharacter( font, c );
}

/*!
	\brief Sets up lighting for a scene.

	If noDiff is set, no diffusion color is used.  This is used for
	drawing the shadowed pass of a shadow volume algorithm.
*/

void slCamera::drawLights( int noDiffuse ) {
	GLfloat dif[4];
	GLfloat dir[4];
	GLfloat amb[4];

	dir[0] = _lights[0].location.x;
	dir[1] = _lights[0].location.y;
	dir[2] = _lights[0].location.z;
	dir[3] = 1.0;

	if ( noDiffuse ) {
		dif[0] = 0.0;
		dif[1] = 0.0;
		dif[2] = 0.0;
		dif[3] = 0.0;
	} else {
		dif[0] = _lights[0].diffuse.x;
		dif[1] = _lights[0].diffuse.y;
		dif[2] = _lights[0].diffuse.z;
		dif[3] = 0.0;
	}

	amb[0] = _lights[0].ambient.x;
	amb[1] = _lights[0].ambient.y;
	amb[2] = _lights[0].ambient.z;
	amb[3] = 0.0;

	if ( _drawSmooth ) glShadeModel( GL_SMOOTH );
	else glShadeModel( GL_FLAT );

	glEnable( GL_LIGHTING );
	glEnable( GL_LIGHT0 );

	glLightf( GL_LIGHT0, GL_CONSTANT_ATTENUATION, 1.0 );
	glLightf( GL_LIGHT0, GL_LINEAR_ATTENUATION, 0.00 );
	glLightfv( GL_LIGHT0, GL_DIFFUSE, dif );
	glLightfv( GL_LIGHT0, GL_AMBIENT, amb );
	glLightfv( GL_LIGHT0, GL_POSITION, dir );
	glLightfv( GL_LIGHT0, GL_SPECULAR, amb );
}

/*!
	\brief Set up the rendering matrix for flat shadows on a given plane.
*/

void slShadowMatrix( GLfloat matrix[4][4], slPlane *p, slVector *light ) {
	GLfloat dot, groundplane[4], lightpos[4];

	lightpos[0] = light->x;
	lightpos[1] = light->y;
	lightpos[2] = light->z;
	lightpos[3] = 0.0;

	groundplane[0] = p->normal.x;
	groundplane[1] = p->normal.y;
	groundplane[2] = p->normal.z;
	groundplane[3] = -slVectorDot( &p->normal, &p->vertex );

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
	\brief Renders a stationary object.
*/

void slCamera::processBillboards( slWorld *w ) {
	GLfloat matrix[16];
	std::vector<slWorldObject*>::iterator wi;
	slSphere *ss;

	glGetFloatv( GL_MODELVIEW_MATRIX, matrix );

	_billboardCount = 0;

	for ( wi = w->_objects.begin(); wi != w->_objects.end(); wi++ ) {
		slWorldObject *wo = *wi;

		if ( wo && wo->_textureMode != BBT_NONE && wo->_displayShape && wo->_displayShape->_type == ST_SPHERE ) {
			double z = 0;

			ss = static_cast< slSphere* >( wo->_displayShape );

			z = matrix[2] * wo->_position.location.x + matrix[6] * wo->_position.location.y + matrix[10] * wo->_position.location.z;

			addBillboard( wo, ss->_radius, z );
		}
	}

	sortBillboards();

	_billboardX.x = matrix[0];
	_billboardX.y = matrix[4];
	_billboardX.z = matrix[8];

	_billboardY.x = matrix[1];
	_billboardY.y = matrix[5];
	_billboardY.z = matrix[9];

	_billboardZ.x = matrix[2];
	_billboardZ.y = matrix[6];
	_billboardZ.z = matrix[10];

	if ( _billboardDrawList == 0 ) 
		_billboardDrawList = glGenLists( 1 );

	glNewList( _billboardDrawList, GL_COMPILE );

	glBegin( GL_TRIANGLE_STRIP );
		glTexCoord2f( 1.0, 1.0 );
		glVertex3f( _billboardX.x + _billboardY.x,  _billboardX.y + _billboardY.y,  _billboardX.z + _billboardY.z );
		glTexCoord2f( 0.0, 1.0 );
		glVertex3f( -_billboardX.x + _billboardY.x, -_billboardX.y + _billboardY.y, -_billboardX.z + _billboardY.z );
		glTexCoord2f( 1.0, 0.0 );
		glVertex3f( _billboardX.x - _billboardY.x,  _billboardX.y - _billboardY.y,  _billboardX.z - _billboardY.z );
		glTexCoord2f( 0.0, 0.0 );
		glVertex3f( -_billboardX.x - _billboardY.x, -_billboardX.y - _billboardY.y, -_billboardX.z - _billboardY.z );
	glEnd();

	glNormal3f( _billboardZ.x, _billboardZ.y, _billboardZ.z );

	glEndList();
}

/*!
	\brief Renders the objects, assuming that all necessary transformations
	have been set up.
*/

void slCamera::renderObjects( slWorld *w, unsigned int flags ) {
	slWorldObject *wo;
	unsigned int n;
	bool color = 1;

	const int loadNames = ( flags & DO_LOAD_NAMES );
	const int doNoAlpha = ( flags & DO_NO_ALPHA );
	const int doOnlyAlpha = ( flags & DO_ONLY_ALPHA );
	const int doNoStationary = ( flags & DO_NO_STATIONARY );
	const int doNoTerrain = ( flags & DO_NO_TERRAIN );
	const int doNoTexture = ( flags & DO_NO_TEXTURE );

	_points.clear();

	if ( doOnlyAlpha ) {
		glEnable( GL_BLEND );
		glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );
	}

	if ( flags & ( DO_OUTLINE | DO_NO_COLOR ) ) color = 0;
	if ( flags & DO_OUTLINE ) glColor4f( 1, 1, 1, 1 );

	glTexEnvf( GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE );

	for ( n = 0; n < w->_objects.size(); ++n ) {
		int skip = 0;
		wo = w->_objects[n];

		if ( !wo ) skip = 1;
		else if ( wo->_drawMode == DM_INVISIBLE ) skip = 1;
		else if ( doNoAlpha && wo->_alpha != 1.0 ) skip = 1;
		else if ( doOnlyAlpha && wo->_alpha == 1.0 ) skip = 1;
		else if ( doNoStationary && wo->_type == WO_STATIONARY ) skip = 1;
		else if ( doNoTerrain && wo->_type == WO_TERRAIN ) skip = 1;
		else if ( wo->_textureMode != BBT_NONE && !( flags & DO_BILLBOARDS_AS_SPHERES ) ) skip = 1;

		if ( !skip ) {
			if ( loadNames )
				glLoadName( n );

			if ( !wo->_drawAsPoint ) {
				if ( color )
					glColor4f( wo->_color.x, wo->_color.y, wo->_color.z, wo->_alpha );

				if ( wo->_alpha != 1.0 ) {
					if ( !doNoTexture && wo->_texture > 0 )
						glBlendFunc( GL_SRC_ALPHA, GL_ONE );
					else
						glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );

					glDisable( GL_CULL_FACE );
				}

				// 0 or -1 can be used to indicate no texture

				if ( !doNoTexture && wo->_texture > 0 ) {
					glBindTexture( GL_TEXTURE_2D, wo->_texture );
					glEnable( GL_TEXTURE_2D );
				}

				wo->draw( this );

				if ( wo->_alpha != 1.0 )
					glEnable( GL_CULL_FACE );

				if ( wo->_texture > 0 )
					glDisable( GL_TEXTURE_2D );
			} else
				_points.push_back( std::pair< slVector, slVector>( wo->getPosition().location, wo->_color ) );
		}
	}

	if ( !doOnlyAlpha ) {
		glEnable( GL_BLEND );
		glPointSize( 2.0 );
		glEnable( GL_POINT_SMOOTH );
		glBegin( GL_POINTS );

		for ( n = 0; n < _points.size(); ++n ) {
			slVector &v = _points[n].first;
			slVector &c = _points[n].second;

			glColor4d( c.x, c.y, c.z, 1.0 );
			glVertex3d( v.x, v.y, v.z );
		}

		glEnd();
	}

	if ( doOnlyAlpha )
		glDisable( GL_BLEND );

	if ( loadNames )
		glLoadName( w->_objects.size() + 1 );
}

/*!
	\brief Renders object neighbor lines.
*/

void slCamera::renderLines( slWorld *w ) {
	slWorldObject *neighbor;
	slVector *x, *y;
	unsigned int n;

		return;

	glLineWidth( 1.2 );
	glEnable( GL_BLEND );
	glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );
	glDepthFunc( GL_ALWAYS );
	glEnable( GL_LINE_SMOOTH );
	glDepthMask( GL_FALSE );

	for ( n = 0; n < w->_objects.size(); ++n ) {
		if ( w->_objects[n] && !( w->_objects[n]->_drawMode & DM_INVISIBLE ) ) {
			if ( w && w->_objects[n]->_drawMode & DM_NEIGHBOR_LINES ) {
				std::vector<slWorldObject*>::iterator wi;

				glColor4f( 0.0, 0.0, 0.0, 0.5 );

				x = &w->_objects[n]->_position.location;

				glBegin( GL_LINES );

				for ( wi = w->_objects[n]->_neighbors.begin(); wi != w->_objects[n]->_neighbors.end(); wi++ ) {
					neighbor = *wi;

					if ( neighbor ) {
						y = &neighbor->_position.location;

						glVertex3f( x->x, x->y, x->z );
						glVertex3f( y->x, y->y, y->z );
					}
				}

				glEnd();
			}
		}
	}

	glLineWidth( 1.0 );

	std::vector< slObjectConnection* >::iterator li;

	for ( li = w->_connections.begin(); li != w->_connections.end(); li++ ) {
		( *li )->draw( this );
	}

	glDepthFunc( GL_LESS );
	glDepthMask( GL_TRUE );
}

/*!
	\brief Renders an x/y axis of a given length.
*/

void slDrawAxis( double x, double y ) {
	x += .02;
	y += .02;

	glBegin( GL_LINES );
	glVertex3f( 0, 0, 0 );
	glVertex3f( x, 0, 0 );
	glVertex3f( 0, y, 0 );
	glVertex3f( 0, 0, 0 );
	glEnd();

	glPushMatrix();
	glTranslated( x + 0.2, -0.2, 0 );
	glScaled( .005, .005, .005 );
	glutStrokeCharacter( GLUT_STROKE_ROMAN, 'x' );
	glPopMatrix();

	glPushMatrix();
	glTranslated( -0.2, y + 0.2, 0 );
	glScaled( .005, .005, .005 );
	glutStrokeCharacter( GLUT_STROKE_ROMAN, 'y' );
	glPopMatrix();
}

/*!
	\brief Compiles a shape into a draw list.

	The draw list is generated or updated as needed.
*/

int slCompileShape( slShape *s, int drawMode, int flags ) {
	if ( !s->_drawList ) s->_drawList = glGenLists( 1 );

	s->_recompile = 0;

	glNewList( s->_drawList, GL_COMPILE );

	slRenderShape( s, drawMode, flags );

	glEndList();

	return s->_drawList;
}

/*!
	\brief Render a shape.

	Typically only called when compiling a drawlist for a shape, this does
	the actual rendering.
*/

void slRenderShape( slShape *s, int drawMode, int flags ) {
	GLUquadricObj *quad;
	int divisions;

	if ( s->_type == ST_SPHERE ) {
		double radius = (( slSphere * )s )->_radius;

		if ( radius < 20 )
			divisions = 20;
		else
			divisions = ( int )radius;

		quad = gluNewQuadric();

		if ( drawMode != GL_POLYGON )
			gluQuadricDrawStyle( quad, GLU_LINE );

		gluQuadricTexture( quad, GL_TRUE );
		gluQuadricOrientation( quad, GLU_OUTSIDE );
		gluSphere( quad, radius, divisions, divisions );
		gluDeleteQuadric( quad );
	} else {
		std::vector<slFace*>::iterator fi;

		for ( fi = s->faces.begin(); fi != s->faces.end(); fi++ )
			slDrawFace( *fi, drawMode, flags );
	}
}

/*!
	\brief Gives vectors perpendicular to v.

	Uses cross-products to find two vectors perpendicular to v.
	Uses either (0, 1, 0) or (1, 0, 0) as the first cross product
	vector (depending on whether v is already set to one of these).
*/

void slPerpendicularVectors( slVector *v, slVector *p1, slVector *p2 ) {
	slVector neg;

	slVectorSet( p1, 0, 1, 0 );

	slVectorMul( v, -1, &neg );

	if ( !slVectorCompare( p1, v ) || !slVectorCompare( &neg, p1 ) )
		slVectorSet( p1, 1, 0, 0 );

	slVectorCross( p1, v, p2 );

	slVectorCross( p2, v, p1 );

	slVectorNormalize( p1 );

	slVectorNormalize( p2 );
}

/*!
	\brief Draws a face, breaking it down into smaller triangles if necessary.
*/

void slDrawFace( slFace *f, int drawMode, int flags ) {
	slVector xaxis, yaxis;
	slVector *norm, *v;
	slPoint *p;
	int edgeCount;

	norm = &f->plane.normal;

	slPerpendicularVectors( norm, &xaxis, &yaxis );

	glNormal3f( norm->x, norm->y, norm->z );

	if ( f->drawFlags & SD_REFLECT ) {
		GLfloat v[4];

		glPushAttrib( GL_ENABLE_BIT );

		glGetFloatv( GL_CURRENT_COLOR, v );
		v[3] = gReflectionAlpha;
		glColor4fv( v );
		glEnable( GL_BLEND );
	}

	// if they're drawing lines, or if the face
	// isn't broken down, do a normal polygon

	if ( drawMode == GL_LINE_LOOP || !slBreakdownFace( f ) ) {
		glBegin( drawMode );

		for ( edgeCount = 0;edgeCount < f->edgeCount;edgeCount++ ) {
			p = ( slPoint * )f->points[edgeCount];

			v = &(( slPoint * )p )->vertex;

			glTexCoord2f( slVectorDot( v, &xaxis ), slVectorDot( v, &yaxis ) );

			glVertex3d( v->x, v->y, v->z );
		}

		glEnd();
	}

	if ( f->drawFlags & SD_REFLECT )
		glPopAttrib();
}

/*!
	\brief Recursively break down and draw a face.

	Breaks down faces into smaller polygons in order to improve
	the quality of lighting and other effects.
*/

int slBreakdownFace( slFace *f ) {
	slVector diff, middle, subv[3], total, xaxis, yaxis;
	double length = 0;
	slVector *v1, *v2;
	slPoint *p;
	int n, n2;

	slPerpendicularVectors( &f->plane.normal, &xaxis, &yaxis );

	slVectorZero( &total );

	for ( n = 0; n < f->edgeCount; ++n ) {
		n2 = n + 1;

		if ( n2 == f->edgeCount )
			n2 = 0;

		p = f->points[n];

		v1 = &p->vertex;

		p = f->points[n2];

		v2 = &p->vertex;

		slVectorSub( v1, v2, &diff );

		slVectorAdd( v1, &total, &total );

		length += slVectorLength( &diff );
	}

	slVectorMul( &total, 1.0 / f->edgeCount, &total );

	if ( length < 30 ) return 0;

	glBegin( GL_TRIANGLES );

	for ( n = 0; n < f->edgeCount; ++n ) {
		n2 = n + 1;

		if ( n2 == f->edgeCount )
			n2 = 0;

		p = f->points[n];

		v1 = &p->vertex;

		p = f->points[n2];

		v2 = &p->vertex;

		slVectorSub( v2, v1, &diff );
		slVectorMul( &diff, .5, &diff );
		slVectorAdd( v1, &diff, &middle );

		slVectorCopy( v1, &subv[0] );
		slVectorCopy( &middle, &subv[1] );
		slVectorCopy( &total, &subv[2] );

		slBreakdownTriangle( &subv[0], 0, &xaxis, &yaxis );

		slVectorCopy( &middle, &subv[0] );
		slVectorCopy( v2, &subv[1] );
		slVectorCopy( &total, &subv[2] );

		slBreakdownTriangle( &subv[0], 0, &xaxis, &yaxis );
	}

	glEnd();

	return 1;
}

void slBreakdownTriangle( slVector *v, int level, slVector *xaxis, slVector *yaxis ) {

	slVector diff, mids[3], subv[3];
	double length = 0;
	int n, n2;

	for ( n = 0; n < 3; ++n ) {
		n2 = n + 1;

		if ( n2 == 3 )
			n2 = 0;

		slVectorSub( &v[n2], &v[n], &diff );

		length += slVectorLength( &diff );

		slVectorMul( &diff, .5, &diff );

		slVectorAdd( &v[n], &diff, &mids[n] );
	}

	if ( length < 200 || level > 3 ) {
		for ( n = 0; n < 3; ++n ) {
			glTexCoord2f( slVectorDot( &v[n], xaxis ), slVectorDot( &v[n], yaxis ) );
			glVertex3f( v[n].x, v[n].y, v[n].z );
		}

		return;
	} else {
		slVectorCopy( &v[0], &subv[0] );
		slVectorCopy( &mids[0], &subv[1] );
		slVectorCopy( &mids[2], &subv[2] );

		slBreakdownTriangle( &subv[0], level + 1, xaxis, yaxis );

		slVectorCopy( &mids[0], &subv[0] );
		slVectorCopy( &v[1], &subv[1] );
		slVectorCopy( &mids[1], &subv[2] );

		slBreakdownTriangle( &subv[0], level + 1, xaxis, yaxis );

		slVectorCopy( &mids[1], &subv[0] );
		slVectorCopy( &v[2], &subv[1] );
		slVectorCopy( &mids[2], &subv[2] );

		slBreakdownTriangle( &subv[0], level + 1, xaxis, yaxis );

		slVectorCopy( &mids[0], &subv[0] );
		slVectorCopy( &mids[1], &subv[1] );
		slVectorCopy( &mids[2], &subv[2] );

		slBreakdownTriangle( &subv[0], level + 1, xaxis, yaxis );
	}
}

/*!
	\brief Prints out and clears OpenGL errors.
*/

inline int slClearGLErrors( char *id ) {
	unsigned int n;
	int c = 0;

	while (( n = glGetError() ) ) {
		slMessage( DEBUG_ALL, "%s: OpenGL error %s\n", id, gluErrorString( n ) );
		c++;
	}

	return c;
}
