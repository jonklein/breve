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

#define SG_MAX_TEXTURES	128

/* these options can be passed to slRenderObjects to control */
/* how objects are rendered. */

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

enum linkDrawOptions {
	LD_BOUND					= 0x01,
	LD_HIGHLIGHT				= 0x02,
	LD_INVISIBLE				= 0x04
};

void slInitGL(slWorld *w);
void slCompileCubeDrawList();

int slLoadTexture(slWorld *w);
int slAddTexture(slWorld *w, int texNum, unsigned char *pixels, int width, int height, int p);
int slAddMipmap(slWorld *w, unsigned char *pixels, int width, int height, int p);

void slRenderWorld(slWorld *w, slCamera *c, int recompile, int mode, int crosshair, int scissor);
void slDrawWorld(slWorld *w, slCamera *c, int recompile, int render_mode, int crosshair, int scissor);

void slDrawWorldToBuffer(slWorld *w, slCamera *c, char *r, char *g, char *b, char *temp);
void slDrawBackground(slCamera *c, slWorld *w);

void slDrawPatches(slPatchGrid *patches, slVector *cam, slVector *target);

void slStencilFloor(slWorld *w, slCamera *c);
void slReflectionPass(slWorld *w, slCamera *c);
void slShadowPass(slWorld *w, slCamera *c);

void slRenderLabels(slWorld *w);

int slRenderObjects(slWorld *w, slCamera *c, int loadNames, int flags);
void slRenderLines(slWorld *w, slCamera *c, int flags);

void slRenderText(slWorld *w, slCamera *c, slVector *loc, slVector *target, int crosshair);

void slPointAtNextObject(slWorld *w, slCamera *c);

void slText(double x, double y, char *string, void *font);
void slStrokeText(double x, double y, char *string, double scale, void *font);

void slDefaultLighting();
void slDrawLights(slCamera *c, int noDiff);
void slShadowMatrix(GLfloat shadowMat[4][4], slPlane *plane, slVector *light);

void slDrawStationary(slWorld *w, slStationary *m, slCamera *c, slVector *color, int texture, int textureScale, int textureMode, float alpha, int mode, int flags);
inline void slDrawMultibody(slWorld *w, slMultibody *m, slCamera *c, slVector *color, int texture, int textureScale, int billboard, float bbRot, float bbAlpha, int mode, int flags);
void slDrawShape(slWorld *w, slCamera *c, slShape *s, slPosition *pos, slVector *color, int texture, int textureScale, int tmode, int mode, int flags, float bbRot, float alpha);

int slCompileShape(slWorld *w, slShape *s, int drawMode, int texture, int textureScale, int flags);
void slRenderShape(slWorld *w, slShape *s, int drawMode, int texture, int textureScale, int flags);
void slDrawAxis(double x, double y);
void slComputeBillboardVectors(slWorld *w, slCamera *c);
void slRenderBillboards(slWorld *w, slCamera *c, int flags);
void slProcessBillboard(slWorld *w, slCamera *c, slVector *color, slVector *loc, int bitmap, int mode, float size, float bbAngle, float alpha, unsigned char selected);
void slDrawFace(slFace *f, int drawMode, int texture, int textureScale, int flags);

int slBreakdownFace(slFace *f, int texture, int textureScale);
void slBreakdownTriangle(slVector *v, int texture, int textureScale, int level, slVector *xaxis, slVector *yaxis);

int slGlSelect(slWorld *w, slCamera *c, int x, int y);
int slVectorForDrag(slWorld *w, slCamera *c, slVector *dragVertex, int x, int y, slVector *dragVector);

int slCompileBox();

void slGlError();
void slInitializeFog ( float r, float g, float b, float a,
                     float near, float far, float density);

int slClearGLErrors(char *e);

void slFreeGL(slWorld *w, slCamera *c);

void slDeleteMbGLLists(slMultibody *m);

void slReversePixelBuffer(char *source, char *dest, int width, int height);

void slTransposeGLMatrix(GLfloat *m);

void slClear(slWorld *w, slCamera *c);
