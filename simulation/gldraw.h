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

// options passed to slRenderObjects to control how objects are rendered. 

enum drawOptions {
    /* control what types of objects are drawn */

    DO_NO_STATIONARY			= 0x000001,
    DO_NO_LINK					= 0x000002,
    DO_NO_TERRAIN				= 0x000004,

    /* effects */

    DO_NO_LIGHTING 				= 0x000008,
    DO_NO_TEXTURE 				= 0x000010,
    DO_NO_BOUND					= 0x000020,
    DO_NO_AXIS					= 0x000040,
    DO_NO_COLOR					= 0x000080,
    DO_NO_STENCIL				= 0x000100,
    DO_NO_REFLECT				= 0x000200,
    DO_NO_BILLBOARD				= 0x000400,
    DO_ONLY_BILLBOARD			= 0x000800,
    DO_PROCESS_BILLBOARD		= 0x001000,
    DO_ONLY_MULTIBODIES			= 0x002000,
    DO_BILLBOARDS_AS_SPHERES	= 0x004000,
    DO_NO_NEIGHBOR_LINES		= 0x008000,    
    DO_OUTLINE					= 0x010000,
    DO_SHADOW_VOLUME			= 0x020000,
    DO_NO_FOG					= 0x040000,
    DO_NO_ALPHA					= 0x080000,
    DO_ONLY_ALPHA				= 0x100000,

    /* recompile flag */

    DO_RECOMPILE				= 0x200000
};

#ifdef __cplusplus 
extern "C" {
#endif
void slInitGL(slWorld *w);
unsigned int slTextureNew();
int slUpdateTexture(slWorld *w, GLuint texNum, unsigned char *pixels, int width, int height, int p);
void slRenderScene(slWorld *w, slCamera *c, int recompile, int mode, int crosshair, int scissor);
int slVectorForDrag(slWorld *w, slCamera *c, slVector *dragVertex, int x, int y, slVector *dragVector);
int slGlSelect(slWorld *w, slCamera *c, int x, int y);

void slReversePixelBuffer(unsigned char *source, unsigned char *dest, int width, int height);

#ifdef __cplusplus 
}
#endif

void slCompileCubeDrawList();

void slRenderWorld(slWorld *w, slCamera *c, int recompile, int render_mode, int crosshair, int scissor);
void slRenderWorldToBuffer(slWorld *w, slCamera *c, char *r, char *g, char *b, char *temp);

void slDrawBackground(slCamera *c, slWorld *w);

void slStencilFloor(slWorld *w, slCamera *c);
void slReflectionPass(slWorld *w, slCamera *c);
void slShadowPass(slWorld *w, slCamera *c);

void slRenderLabels(slWorld *w);

void slRenderObjects(slWorld *w, slCamera *c, int loadNames, int flags);
void slRenderLines(slWorld *w, slCamera *c, int flags);

void slRenderText(slWorld *w, slCamera *c, slVector *loc, slVector *target, int crosshair);

void slText(double x, double y, char *string, void *font);
void slStrokeText(double x, double y, char *string, double scale, void *font);

void slDrawLights(slCamera *c, int noDiff);
void slShadowMatrix(GLfloat shadowMat[4][4], slPlane *plane, slVector *light);

void slDrawShape(slCamera *c, slShape *s, slPosition *pos, slVector *color, int texture, double textureScale, int tmode, int mode, int flags, float bbRot, float alpha);

int slCompileShape(slShape *s, int drawMode, int texture, double textureScale, int flags);
void slRenderShape(slShape *s, int drawMode, int texture, double textureScale, int flags);
void slDrawAxis(double x, double y);
void slComputeBillboardVectors(slWorld *w, slCamera *c);
void slRenderBillboards(slCamera *c, int flags);
void slProcessBillboard(slCamera *c, slVector *color, slVector *loc, int bitmap, int mode, float size, float bbAngle, float alpha, unsigned char selected);
void slDrawFace(slFace *f, int drawMode, int texture, double textureScale, int flags);

int slBreakdownFace(slFace *f, int texture, double textureScale);
void slBreakdownTriangle(slVector *v, int texture, double textureScale, int level, slVector *xaxis, slVector *yaxis);

int slCompileBox();

void slGlError();

int slClearGLErrors(char *e);

void slFreeGL(slWorld *w, slCamera *c);

void slDeleteMbGLLists(slMultibody *m);

void slTransposeGLMatrix(GLfloat *m);

void slClear(slWorld *w, slCamera *c);

void slDrawNetsimBounds(slWorld *w);
