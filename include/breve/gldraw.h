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

#include "glIncludes.h"

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

#ifdef __cplusplus 
extern "C" {
#endif
void slInitGL(slWorld *, slCamera *);
unsigned int slTextureNew(slCamera *);
void slTextureFree(slCamera *, unsigned int);
int slUpdateTexture(slCamera *, GLuint, unsigned char *, int, int, int);
void slRenderScene(slWorld *, slCamera *, int);
int slVectorForDrag(slWorld *, slCamera *, slVector *, int, int, slVector *);
int slGlSelect(slWorld *, slCamera *, int, int);


#ifdef __cplusplus 
}
#endif

void slDrawFog(slWorld *, slCamera *);

void slMatrixGLMult(double [3][3]);

void slCompileCubeDrawList(int);
void slCompileSphereDrawList(int);

void slRenderWorld(slWorld *, slCamera *, int, int);
void slRenderWorldToBuffer(slWorld *, slCamera *, char *, char *, char *, char *);

void slDrawBackground(slCamera *, slWorld *);

void slStencilFloor(slWorld *, slCamera *);
void slReflectionPass(slWorld *, slCamera *);
void slShadowPass(slWorld *, slCamera *);

void slRenderLabels(slWorld *);

void slRenderObjects(slWorld *, slCamera *, unsigned int);
void slRenderLines(slWorld *, slCamera *);

void slRenderText(slWorld *, slCamera *, slVector *, slVector *, int);

void slText(double, double, char *, void *);
void slStrokeText(double, double, char *, double, void *);

void slDrawLights(slCamera *, int);
void slShadowMatrix(GLfloat [4][4], slPlane *, slVector *);

void slDrawShape(slCamera *, slShape *, slPosition *, double, int, int);

int slCompileShape(slShape *, int, double, int);
void slRenderShape(slShape *, int, double, int);
void slDrawAxis(double, double);

void slProcessBillboards(slWorld *, slCamera *);
void slRenderBillboards(slCamera *, int);
void slDrawFace(slFace *, int, double, int);

int slBreakdownFace(slFace *, double);
void slBreakdownTriangle(slVector *, double, int, slVector *, slVector *);

int slCompileBox(void);

void slGlError(void);

int slClearGLErrors(char *);

void slFreeGL(slWorld *, slCamera *);

void slDeleteMbGLLists(slMultibody *);

void slTransposeGLMatrix(GLfloat *);

void slClear(slWorld *, slCamera *);
