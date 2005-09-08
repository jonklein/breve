/*****************************************************************************
 *                                                                           *
 * The breve Simulation Environment                                          *
 * Copyright (C) 2000-2005 Jonathan Klein                                    *
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

/**
 *  @file breveFunctionsVector.cc
 *  @breif A vector manipulation class.
 *  @author Eric DeWitt
 *  Copyright (C) 2005 Eric DeWitt, Jonathan Klein
 *
 * This library of breve functions provide robust vector routines for
 * use with 2D and 3D matrices of large sizes.  The goal is to provide 
 * hardware acceleration and eventually a full complement of matrix
 * routines.  This may include vector and matrix operations, convolutions,
 * ffts, etc.
 *
 */

#include "kernel.h"
#include "breveFunctionsImage.h"
#include "bigMatrix.hh"

#ifdef HAVE_LIBGSL
typedef slBigVectorGSL brVector;
#define BRBIGVECTOR(ptr) (reinterpret_cast<brVector*>(BRPOINTER(ptr)))
//inline brVector* BRBIGVECTOR(brEval* ptr) { return reinterpret_cast<brVector*>(((ptr)->values.pointerValue)); }
#else
typedef slBigVectorGSL brVector;
// This can be replaced with alternative optimzation code 
#define BRBIGVECTOR(ptr) (reinterpret_cast<brMatrix2D*>(BRPOINTER(ptr)))
//inline BRBIGVECTOR(brEval* ptr) { return reinterpret_cast<brMatrix2D*>(((ptr)->values.pointerValue)); }
#endif

int brIVectorNew(brEval args[], brEval *target, brInstance *i) {
    BRPOINTER(target) = new brVector(BRINT(&args[0]));
    
    return EC_OK;
}
int brIVectorFree(brEval args[], brEval *target, brInstance *i) {
    if(BRBIGVECTOR(&args[0])) delete BRBIGVECTOR(&args[0]);
    
    return EC_OK;
}
int brIVectorGet(brEval args[], brEval *target, brInstance *i) {
	BRDOUBLE(target) = double(BRBIGVECTOR(&args[0])->get(BRINT(&args[1])));

	return EC_OK;
}

int brIVectorSet(brEval args[], brEval *target, brInstance *i) {
	BRBIGVECTOR(&args[0])->set(BRINT(&args[1]), float(BRDOUBLE(&args[2])));

	return EC_OK;
}

int brIVectorSetAll(brEval args[], brEval *target, brInstance *i) {
	static_cast<slVectorViewGSL*>(BRBIGVECTOR(&args[0]))->setAll(float(BRDOUBLE(&args[1])));

	return EC_OK;
}

int brIVectorClamp(brEval args[], brEval *target, brInstance *i) {
	BRBIGVECTOR(&args[0])->clamp(float(BRDOUBLE(&args[1])), float(BRDOUBLE(&args[2])), float(BRDOUBLE(&args[3])));

	return EC_OK;
}

int brIVectorCopy(brEval args[], brEval *target, brInstance *i) {

	static_cast<slVectorViewGSL*>(BRBIGVECTOR(&args[1]))->copyData(*static_cast<slVectorViewGSL*>(BRBIGVECTOR(&args[0])));

	return EC_OK;
}

int brIVectorGetAbsoluteSum(brEval args[], brEval *target, brInstance *i) {
	BRDOUBLE(target) = double(static_cast<slVectorViewGSL*>(BRBIGVECTOR(&args[0]))->absoluteSum());

	return EC_OK;
}

int brIVectorAddScaled(brEval args[], brEval *target, brInstance *i) {
	static_cast<slVectorViewGSL*>(BRBIGVECTOR(&args[0]))->inPlaceScaleAndAdd(float(BRDOUBLE(&args[2])), *static_cast<slVectorView*>(BRBIGVECTOR(&args[1])));

	return EC_OK;
}

int brIVectorAddScalar(brEval args[], brEval *target, brInstance *i) {
	static_cast<slVectorViewGSL*>(BRBIGVECTOR(&args[0]))->inPlaceAdd(BRDOUBLE(&args[1]));

	return EC_OK;
}

int brIVectorMulElements(brEval args[], brEval *target, brInstance *i) {
	static_cast<slVectorViewGSL*>(BRBIGVECTOR(&args[0]))->inPlaceMultiply(*static_cast<slVectorViewGSL*>(BRBIGVECTOR(&args[1])));

	return EC_OK;
}

int brIVectorScale(brEval args[], brEval *target, brInstance *i) {
	static_cast<slVectorViewGSL*>(BRBIGVECTOR(&args[0]))->inPlaceMultiply(BRDOUBLE(&args[1]));

	return EC_OK;
}

int brIVectorCopyToImage(brEval args[], brEval *result, brInstance *i) {
	brVector *sourceVector = BRBIGVECTOR(&args[0]);
	brImageData *d = (brImageData*)BRPOINTER(&args[1]);
	float* sourceData = sourceVector->getGSLVector()->data;
	unsigned char *pdata;
	float scale = 255.0 * BRDOUBLE(&args[3]);
	int offset = BRINT(&args[2]);
	int r;
	int x, y, xmax, ymax;
    int yStride = d->x * 4;
    int xStride = d->y * 4;

	xmax = sourceVector->dim();
	ymax = d->y;
 
	if (xmax > d->x)
		xmax = d->x;

	if (ymax > d->y)
		ymax = d->y;

//	pdata = d->data;
	pdata = d->data + offset;

	for (y = 0; y < ymax; y++)
	 	for (x = 0; x < xmax; x++) {
			r = (int)(sourceData[x] * scale);
			if (r > 255)
//				pdata[(y * yStride) + (y << 2) + offset] = 255;
				*pdata = 255;
			else
//				pdata[(y * yStride) + (y << 2) + offset] = r;
				*pdata = r;
			pdata += 4;
	 	}

	if (d->textureNumber == -1)
		d->textureNumber = slTextureNew(i->engine->camera);

	slUpdateTexture(i->engine->camera, d->textureNumber, d->data, d->x, d->y, GL_RGBA);

	return EC_OK;
}

void breveInitVectorFunctions(brNamespace *n) {
    // vector interface
	brNewBreveCall(n, "vectorNew", brIVectorNew, AT_POINTER, AT_INT, 0);
	brNewBreveCall(n, "vectorFree", brIVectorFree, AT_NULL, AT_POINTER, 0);
	brNewBreveCall(n, "vectorGet", brIVectorGet, AT_DOUBLE, AT_POINTER, AT_INT, 0);
	brNewBreveCall(n, "vectorSet", brIVectorSet, AT_NULL, AT_POINTER, AT_INT, AT_DOUBLE, 0);
	brNewBreveCall(n, "vectorSetAll", brIVectorSetAll, AT_NULL, AT_POINTER, AT_DOUBLE, 0);
	brNewBreveCall(n, "vectorClamp", brIVectorClamp, AT_NULL, AT_POINTER, AT_DOUBLE, AT_DOUBLE, AT_DOUBLE, 0);
	brNewBreveCall(n, "vectorCopy", brIVectorCopy, AT_NULL, AT_POINTER, AT_POINTER, 0);
	brNewBreveCall(n, "vectorScale", brIVectorScale, AT_NULL, AT_POINTER, AT_DOUBLE, 0);
	brNewBreveCall(n, "vectorGetAbsoluteSum", brIVectorGetAbsoluteSum, AT_DOUBLE, AT_POINTER, 0);
	brNewBreveCall(n, "vectorAddScaled", brIVectorAddScaled, AT_NULL, AT_POINTER, AT_POINTER, AT_DOUBLE, 0);
	brNewBreveCall(n, "vectorAddScalar", brIVectorAddScalar, AT_NULL, AT_POINTER, AT_DOUBLE, 0);
	brNewBreveCall(n, "vectorMulElements", brIVectorMulElements, AT_NULL, AT_POINTER, AT_POINTER, 0);
	brNewBreveCall(n, "vectorCopyToImage", brIVectorCopyToImage, AT_NULL, AT_POINTER, AT_POINTER, AT_INT, AT_DOUBLE, 0);

}
