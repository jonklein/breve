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

#ifndef _GLDRAW_H
#define _GLDRAW_H

#include "glIncludes.h"
#include "camera.h"

class slWorld;
class slStationary;

typedef float slColor[4];

// options passed to slRenderObjects to control how objects are rendered. 

enum drawOptions {
    // control what types of objects are drawn 

    DO_NO_STATIONARY			= 0x000001,
    DO_NO_LINK					= 0x000002,
    DO_NO_TERRAIN				= 0x000004,

    // effects 

    DO_NO_TEXTURE 				= 0x000008,
    DO_NO_BOUND					= 0x000010,
    DO_NO_AXIS					= 0x000020,
    DO_NO_COLOR					= 0x000040,
    DO_NO_BILLBOARD				= 0x000080,
    DO_ONLY_MULTIBODIES			= 0x000100,
    DO_BILLBOARDS_AS_SPHERES	= 0x000200,
    DO_NO_NEIGHBOR_LINES		= 0x000400,    
    DO_OUTLINE					= 0x000800,
    DO_NO_ALPHA					= 0x001000,
    DO_ONLY_ALPHA				= 0x002000,

	// load names for selections

    DO_LOAD_NAMES				= 0x004000,

    // recompile flag 

    DO_RECOMPILE				= 0x008000
};

void slInitGL(slWorld *, slCamera *);
unsigned int slTextureNew(slCamera *);
void slTextureFree(slCamera *, unsigned int);
int slUpdateTexture(slCamera *, GLuint, unsigned char *, int, int, int);

void slMatrixGLMult(double [3][3]);

void slRenderWorldToBuffer(slWorld *, slCamera *, char *, char *, char *, char *);


void slText(double, double, const char *, void *);
void slStrokeText(double, double, const char *, double, void *);

/**
 * Computes the flat shadow matrix for a plane and lightsource.
 * @param[out] matrix      The resulting shadow matrix.
 * @param[in] shadowPlane  The plane catching the shadows.
 * @param[in] lightSource  The location of the light source.
 */

void slShadowMatrix(GLfloat matrix[4][4], slPlane *shadowPlane, slVector *lightSource);

int slCompileShape(slShape *, int, int);
void slRenderShape(slShape *, int, int);
void slDrawAxis(double, double);

void slProcessBillboards(slWorld *, slCamera *);
void slRenderBillboards(slCamera *, int);

void slDrawFace(slFace *face, int drawMode, int flags);

/**
 * Recursively breaks down a face for rendering.
 * @param[in] face  The face to break down.
 */

int slBreakdownFace(slFace *face);

/**
 * Recursively breaks down a triangle into smaller polygons.
 * @param[in] vertices       The 3 vertices defining the triangle.
 * @param[in] level          The current recursion depth.
 * @param[in] xaxis          A unit vector along the X axis for texturing.
 * @param[in] yaxis          A unit vector along the Y axis for texturing.
 */

void slBreakdownTriangle(slVector *vertices, int depth, slVector *xaxis, slVector *yaxis);

void slGlError(void);

int slClearGLErrors(char *);

void slFreeGL(slWorld *, slCamera *);

void slDeleteMbGLLists(slMultibody *);

void slTransposeGLMatrix(GLfloat *);

#endif /* _GLDRAW_H */
