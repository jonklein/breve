/*****************************************************************************
 * The breve Simulation Environment                                          *
 * Copyright (C) 2000-2004 Jonathan Klein                                    *
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

/*
 * This library of breve functions provide robust matrix routines for
 * 2D and 3D matrices of large sizes.  The goal is to provide hardware
 * acceleration and eventually a full complement of matrix routines.
 * this may include vector and matrix operations, convolutions, ffts,
 * etc.  It is used internally to support both patches and diffusion
 * calculations.
 *
 */

#include "kernel.h"
#include "bigMatrix.hh"

#ifdef MACOSX 
#include <vecLib/vDSP.h>
#endif

#ifdef HAVE_LIBGSL
typedef slBigMatrix2DGSL brMatrix2D;
typedef slBigMatrix3DGSL brMatrix3D;
#define BRBIGMATRIX2D(ptr) (reinterpret_cast<brMatrix2D*>(BRPOINTER(ptr)))
#define BRBIGMATRIX3D(ptr) (reinterpret_cast<brMatrix3D*>(BRPOINTER(ptr)))
//inline brMatrix2D* BRBIGMATRIX2D(brEval* ptr) { return reinterpret_cast<brMatrix2D*>(((ptr)->values.pointerValue)); }
#else
// This can be replaced with alternative optimzation code 
#define BRBIGMATRIX2D(ptr) (reinterpret_cast<brMatrix2D*>(BRPOINTER(ptr)))
#define BRBIGMATRIX3D(ptr) (reinterpret_cast<brMatrix3D*>(BRPOINTER(ptr)))
//inline BRBIGMATRIX2D(brEval* ptr) { return reinterpret_cast<brMatrix2D*>(((ptr)->values.pointerValue)); }
typedef slBigMatrix2DGSL brMatrix2D;
typedef slBigMatrix3DGSL brMatrix3D;
#endif

#include "breveFunctionsImage.h"

int brIMatrix2DNew(brEval args[], brEval *target, brInstance *i) {
	BRPOINTER(target) = new brMatrix2D(BRINT(&args[0]), BRINT(&args[1]));

	return EC_OK;
}

int brIMatrix2DFree(brEval args[], brEval *target, brInstance *i) {
	if (BRBIGMATRIX2D(&args[0])) delete BRBIGMATRIX2D(&args[0]);

	return EC_OK;
}

int brIMatrix2DGet(brEval args[], brEval *target, brInstance *i) {
	BRDOUBLE(target) = double(BRBIGMATRIX2D(&args[0])->get(BRINT(&args[1]), BRINT(&args[2])));

	return EC_OK;
}

int brIMatrix2DSet(brEval args[], brEval *target, brInstance *i) {
	BRBIGMATRIX2D(&args[0])->set(BRINT(&args[1]), BRINT(&args[2]), float(BRDOUBLE(&args[3])));

	return EC_OK;
}

int brIMatrix2DSetAll(brEval args[], brEval *target, brInstance *i) {
	BRBIGMATRIX2D(&args[0])->setAll(float(BRDOUBLE(&args[1])));

	return EC_OK;
}

int brIMatrix2DClamp(brEval args[], brEval *target, brInstance *i) {
	BRBIGMATRIX2D(&args[0])->clamp(float(BRDOUBLE(&args[1])), float(BRDOUBLE(&args[2])));

	return EC_OK;
}

int brIMatrix2DCopy(brEval args[], brEval *target, brInstance *i) {

	static_cast<slVectorViewGSL*>(BRBIGMATRIX2D(&args[1]))->copy(*static_cast<slVectorViewGSL*>(BRBIGMATRIX2D(&args[0])));

	return EC_OK;
}

int brIMatrix2DGetAbsoluteSum(brEval args[], brEval *target, brInstance *i) {
	BRDOUBLE(target) = double(static_cast<slVectorViewGSL*>(BRBIGMATRIX2D(&args[0]))->absoluteSum());

	return EC_OK;
}

int brIMatrix2DAddScaled(brEval args[], brEval *target, brInstance *i) {
	static_cast<slVectorViewGSL*>(BRBIGMATRIX2D(&args[0]))->scaleAndAdd(float(BRDOUBLE(&args[2])), *static_cast<slVectorView*>(BRBIGMATRIX2D(&args[1])));

	return EC_OK;
}

int brIMatrix2DAddScalar(brEval args[], brEval *target, brInstance *i) {
	static_cast<slVectorViewGSL*>(BRBIGMATRIX2D(&args[0]))->inPlaceAdd(BRDOUBLE(&args[1]));

	return EC_OK;
}

int brIMatrix2DMulElements(brEval args[], brEval *target, brInstance *i) {
	static_cast<slVectorViewGSL*>(BRBIGMATRIX2D(&args[0]))->inPlaceMultiply(*static_cast<slVectorViewGSL*>(BRBIGMATRIX2D(&args[1])));

	return EC_OK;
}

/* Separate BLAS routines are deprecated
int brIMatrixBlasMul(brEval args[], brEval *target, brInstance *i) {
	return EC_OK;
}
*/

int brIMatrix2DScale(brEval args[], brEval *target, brInstance *i) {
	static_cast<slVectorViewGSL*>(BRBIGMATRIX2D(&args[0]))->inPlaceMultiply(BRDOUBLE(&args[1]));

	return EC_OK;
}

int brIMatrix2DDiffusePeriodic(brEval args[], brEval *target, brInstance *i) {
	brMatrix2D *diffTarget = BRBIGMATRIX2D(&args[1]);
	brMatrix2D *chemSource = BRBIGMATRIX2D(&args[0]);
	float scale = BRDOUBLE(&args[2]);
	float* diffData = diffTarget->getGSLVector()->data;
	float* chemData = chemSource->getGSLVector()->data;	
	float newVal = 0.0;
	unsigned int xDim = chemSource->xDim();
	unsigned int yDim = chemSource->yDim();
	unsigned int diffTDA = diffTarget->xDim(); // proxy for tda
	unsigned int chemTDA = chemSource->xDim(); // proxy for tda
	unsigned int x = 0, y = 0;
	int xp, xm, yp, ym;

	for (y = 0; y < yDim; y++)
		for (x = 0; x < xDim; x++) {
			xp = (x + 1) % xDim;
			yp = (y + 1) % yDim;
			xm = (x - 1) % xDim;
			ym = (y - 1) % yDim;

			if (xm < 0)
				xm += xDim;
			else if (xp >= (int)xDim)
				xp -= xDim;

			if (ym < 0)
				ym += yDim;
			else if (yp >= (int)yDim)
				yp -= yDim;

			newVal = scale * ((-4.0f * chemData[x * chemTDA + y]) +
			    chemData[xm * chemTDA + y] + chemData[xp * chemTDA + y] + 
			    chemData[x  * chemTDA + ym] + chemData[x * chemTDA + yp]);

			diffData[x * diffTDA + y] = newVal;
		}

	return EC_OK;
}

int brIMatrix2DDiffuse(brEval args[], brEval *target, brInstance *i) {
	brMatrix2D *diffTarget = BRBIGMATRIX2D(&args[1]);
	brMatrix2D *chemSource = BRBIGMATRIX2D(&args[0]);
	float scale = BRDOUBLE(&args[2]);
	float* diffData = diffTarget->getGSLVector()->data;
	float* chemData = chemSource->getGSLVector()->data;	
	float newVal = 0.0;
	unsigned int diffTDA = diffTarget->xDim(); // proxy for tda
	unsigned int chemTDA = chemSource->xDim(); // proxy for tda
	unsigned int xDim = chemSource->xDim();
	unsigned int yDim = chemSource->yDim();    
	unsigned int x = 0, y = 0;
	int xp, xm, yp, ym;

    // this will get moved to a seperate util class later
    // and will be converted to iterators when we migrate to gslmm based code
    
    ///// VERY TEMP local optimization
#ifdef MACOS
#if(0)
       // f3x3 is an altivec optimized convolution, but does not handle periodic
       // boundaries.  

       float kernel[9];

       kernel[0] = 0; kernel[1] = scale; kernel[2] = 0;
       kernel[3] = scale; kernel[4] = scale * -4; kernel[5] = scale;
       kernel[6] = 0; kernel[7] = scale; kernel[8] = 0;

       f3x3(m->data, m->tda, m->size2, kernel, n->data);

       return EC_OK;
#endif
#endif

    x = 0;
	for (y = 0; y < yDim ; y++)
	{
			xp = (x + 1);
			yp = (y + 1);
			ym = (y - 1);
			newVal = scale * ((-4.0f * chemData[x * chemTDA + y]) +
			    chemData[xp * chemTDA + y] + 
			    chemData[x  * chemTDA + ym] + chemData[x * chemTDA + yp]);

			diffData[x * diffTDA + y] = newVal;
    }
    x = xDim;
	for (y = 0; y < yDim; y++)
	{
			yp = (y + 1);
			xm = (x - 1);
			ym = (y - 1);
			newVal = scale * ((-4.0f * chemData[x * chemTDA + y]) +
			    chemData[xm * chemTDA + y] + 
			    chemData[x  * chemTDA + ym] + chemData[x * chemTDA + yp]);

			diffData[x * diffTDA + y] = newVal;
	}
	y = 0;
	for (x = 1; x < yDim - 1; x++)
	{
			xp = (x + 1);
			yp = (y + 1);
			xm = (x - 1);
			newVal = scale * ((-4.0f * chemData[x * chemTDA + y]) +
			    chemData[xm * chemTDA + y] + chemData[xp * chemTDA + y] + 
			    chemData[x * chemTDA + yp]);

			diffData[x * diffTDA + y] = newVal;
	}
	y = yDim;
	for (x = 1; x < yDim - 1; x++)
	{
			xp = (x + 1);
			xm = (x - 1);
			ym = (y - 1);
			newVal = scale * ((-4.0f * chemData[x * chemTDA + y]) +
			    chemData[xm * chemTDA + y] + chemData[xp * chemTDA + y] + 
			    chemData[x  * chemTDA + ym]);

			diffData[x * diffTDA + y] = newVal;
    }
	
	for (y = 1; y < yDim - 1; y++)
		for (x = 1; x < xDim - 1; x++) {
			xp = (x + 1);
			yp = (y + 1);
			xm = (x - 1);
			ym = (y - 1);

			newVal = scale * ((-4.0f * chemData[x * chemTDA + y]) +
			    chemData[xm * chemTDA + y] + chemData[xp * chemTDA + y] + 
			    chemData[x  * chemTDA + ym] + chemData[x * chemTDA + yp]);

			diffData[x * diffTDA + y] = newVal;
		}

	return EC_OK;
}

int brIMatrix2DCopyToImage(brEval args[], brEval *result, brInstance *i) {
	brMatrix2D *sourceMatrix = BRBIGMATRIX2D(&args[0]);
	brImageData *d = (brImageData*)BRPOINTER(&args[1]);
	float* sourceData = sourceMatrix->getGSLVector()->data;
	unsigned int sourceTDA = sourceMatrix->xDim();
	unsigned char *pdata;
	double scale = 255.0 * BRDOUBLE(&args[3]);
	int offset = BRINT(&args[2]);
	int r;
	int x, y, xmax, ymax;

	xmax = sourceMatrix->xDim();
	ymax = sourceMatrix->yDim();

	if (xmax > d->x)
		xmax = d->x;
	if (ymax > d->y)
		ymax = d->y;

	pdata = d->data + offset;

	for (y = 0; y < ymax; y++)
	 	for (x = 0; x < xmax; x++) {
			r = (int)(sourceData[x * sourceTDA + y] * scale);
			if (r > 255)
				*pdata = 255;
			else
				*pdata = r;
			pdata += 4;
	 	}

	if (d->textureNumber == -1)
		d->textureNumber = slTextureNew(i->engine->camera);

	slUpdateTexture(i->engine->camera, d->textureNumber, d->data, d->x, d->y, GL_RGBA);

	return EC_OK;
}

int brIMatrix3DNew(brEval args[], brEval *target, brInstance *i) {

	BRPOINTER(target) = new brMatrix3D(BRINT(&args[0]), BRINT(&args[1]), BRINT(&args[2]));

	return EC_OK;
}

int brIMatrix3DFree(brEval args[], brEval *target, brInstance *i) {
    if (BRBIGMATRIX3D(&args[0]))
    	delete BRBIGMATRIX3D(&args[0]);

	return EC_OK;
}

int brIMatrix3DGet(brEval args[], brEval *target, brInstance *i) {
	BRDOUBLE(target) = double(BRBIGMATRIX3D(&args[0])->get(BRINT(&args[1]), BRINT(&args[2]), BRINT(&args[3])));

	return EC_OK;
}

int brIMatrix3DSet(brEval args[], brEval *target, brInstance *i) {
	BRBIGMATRIX3D(&args[0])->set(BRINT(&args[1]), BRINT(&args[2]), BRINT(&args[3]), float(BRDOUBLE(&args[4])));

	return EC_OK;
}

int brIMatrix3DSetAll(brEval args[], brEval *target, brInstance *i) {
	BRBIGMATRIX3D(&args[0])->setAll(float(BRDOUBLE(&args[1])));

	return EC_OK;
}

int brIMatrix3DCopy(brEval args[], brEval *target, brInstance *i) {

	static_cast<slVectorViewGSL*>(BRBIGMATRIX3D(&args[1]))->copy(*static_cast<slVectorViewGSL*>(BRBIGMATRIX3D(&args[0])));

	return EC_OK;
}

int brIMatrix3DGetAbsoluteSum(brEval args[], brEval *target, brInstance *i) {
	BRDOUBLE(target) = double(static_cast<slVectorViewGSL*>(BRBIGMATRIX3D(&args[0]))->absoluteSum());

	return EC_OK;
}

int brIMatrix3DAddScaled(brEval args[], brEval *target, brInstance *i) {
	static_cast<slVectorViewGSL*>(BRBIGMATRIX3D(&args[0]))->scaleAndAdd(float(BRDOUBLE(&args[2])), *static_cast<slVectorView*>(BRBIGMATRIX3D(&args[1])));

	return EC_OK;
}

int brIMatrix3DAddScalar(brEval args[], brEval *target, brInstance *i) {
    
	static_cast<slVectorViewGSL*>(BRBIGMATRIX3D(&args[0]))->inPlaceAdd(BRDOUBLE(&args[1]));

	return EC_OK;
}

int brIMatrix3DMulElements(brEval args[], brEval *target, brInstance *i) {
	static_cast<slVectorViewGSL*>(BRBIGMATRIX3D(&args[0]))->inPlaceMultiply(*static_cast<slVectorViewGSL*>(BRBIGMATRIX3D(&args[1])));

	return EC_OK;
}

/* Seperate BLAS routines are be deprecated
int brIMatrixBlasMul(brEval args[], brEval *target, brInstance *i) {
	return EC_OK;
}
*/

int brIMatrix3DScale(brEval args[], brEval *target, brInstance *i) {
	static_cast<slVectorViewGSL*>(BRBIGMATRIX3D(&args[0]))->inPlaceMultiply(BRDOUBLE(&args[1]));

	return EC_OK;
}

int brIMatrix3DDiffusePeriodic(brEval args[], brEval *target, brInstance *i) {
	brMatrix3D *diffTarget = BRBIGMATRIX3D(&args[1]);
	brMatrix3D *chemSource = BRBIGMATRIX3D(&args[0]);
	float scale = BRDOUBLE(&args[2]);
	float* diffData = diffTarget->getGSLVector()->data;
	float* chemData = chemSource->getGSLVector()->data;	
	float newVal = 0.0;
	unsigned int xDim = chemSource->xDim();
	unsigned int yDim = chemSource->yDim();
	unsigned int zDim = chemSource->zDim();
	unsigned int diffTDA = diffTarget->xDim(); // proxy for tda
	unsigned int chemTDA = chemSource->xDim(); // proxy for tda
	unsigned int chemXY = chemSource->xDim() * chemSource->yDim();
	unsigned int x = 0, y = 0, z = 0;
	int xp, xm, yp, ym, zp, zm;

    for (z = 0; z < zDim; z++)
        for (y = 0; y < yDim; y++)
            for (x = 0; x < xDim; x++) {
                xp = (x + 1) % xDim;
                yp = (y + 1) % yDim;
                zp = (z + 1) % zDim;
                xm = (x - 1) % xDim;
                ym = (y - 1) % yDim;
                zm = (z - 1) % zDim;
    
                if (xm < 0)
                    xm += xDim;
                else if (xp >= (int)xDim)
                    xp -= xDim;
    
                if (ym < 0)
                    ym += yDim;
                else if (yp >= (int)yDim)
                    yp -= yDim;
    
                if (zm < 0)
                    zm += zDim;
                else if (zp >= (int)zDim)
                    zp -= zDim;
    
                newVal = scale * ((-6.0f * chemData[(z * chemXY) + (x * chemTDA) + y]) +
                    chemData[(z * chemXY) + (xm * chemTDA) + y] +
                    chemData[(z * chemXY) + (xp * chemTDA) + y] + 
                    chemData[(z * chemXY) + (x * chemTDA) + ym] +
                    chemData[(z * chemXY) + (x * chemTDA) + yp] +
                    chemData[(zm * chemXY) + (x  * chemTDA) + y] +
                    chemData[(zp * chemXY) + (x * chemTDA) + y]);
    
                diffData[(z * chemXY) + x * diffTDA + y] = newVal;
            }

	return EC_OK;
}

int brIMatrix3DDiffuse(brEval args[], brEval *target, brInstance *i) {
	brMatrix3D *diffTarget = BRBIGMATRIX3D(&args[1]);
	brMatrix3D *chemSource = BRBIGMATRIX3D(&args[0]);
	float scale = BRDOUBLE(&args[2]);
	float* diffData = diffTarget->getGSLVector()->data;
	float* chemData = chemSource->getGSLVector()->data;	
	float newVal = 0.0;
	unsigned int diffTDA = diffTarget->xDim(); // proxy for tda
	unsigned int chemTDA = chemSource->xDim(); // proxy for tda
	unsigned int xDim = chemSource->xDim();
	unsigned int yDim = chemSource->yDim();    
	unsigned int x = 0, y = 0;
	int xp, xm, yp, ym;

    // this will get moved to a seperate util class later
    // and will be converted to iterators when we migrate to gslmm based code
    
    ///// VERY TEMP local optimization
#ifdef MACOS
#if(0)
       // f3x3 is an altivec optimized convolution, but does not handle periodic
       // boundaries.  

       float kernel[9];

       kernel[0] = 0; kernel[1] = scale; kernel[2] = 0;
       kernel[3] = scale; kernel[4] = scale * -4; kernel[5] = scale;
       kernel[6] = 0; kernel[7] = scale; kernel[8] = 0;

       f3x3(m->data, m->tda, m->size2, kernel, n->data);

       return EC_OK;
#endif
#endif

    x = 0;
	for (y = 0; y < yDim ; y++)
	{
			xp = (x + 1);
			yp = (y + 1);
			ym = (y - 1);
			newVal = scale * ((-4.0f * chemData[x * chemTDA + y]) +
			    chemData[xp * chemTDA + y] + 
			    chemData[x  * chemTDA + ym] + chemData[x * chemTDA + yp]);

			diffData[x * diffTDA + y] = newVal;
    }
    x = xDim;
	for (y = 0; y < yDim; y++)
	{
			yp = (y + 1);
			xm = (x - 1);
			ym = (y - 1);
			newVal = scale * ((-4.0f * chemData[x * chemTDA + y]) +
			    chemData[xm * chemTDA + y] + 
			    chemData[x  * chemTDA + ym] + chemData[x * chemTDA + yp]);

			diffData[x * diffTDA + y] = newVal;
	}
	y = 0;
	for (x = 1; x < yDim - 1; x++)
	{
			xp = (x + 1);
			yp = (y + 1);
			xm = (x - 1);
			newVal = scale * ((-4.0f * chemData[x * chemTDA + y]) +
			    chemData[xm * chemTDA + y] + chemData[xp * chemTDA + y] + 
			    chemData[x * chemTDA + yp]);

			diffData[x * diffTDA + y] = newVal;
	}
	y = yDim;
	for (x = 1; x < yDim - 1; x++)
	{
			xp = (x + 1);
			xm = (x - 1);
			ym = (y - 1);
			newVal = scale * ((-4.0f * chemData[x * chemTDA + y]) +
			    chemData[xm * chemTDA + y] + chemData[xp * chemTDA + y] + 
			    chemData[x  * chemTDA + ym]);

			diffData[x * diffTDA + y] = newVal;
    }
	
	for (y = 1; y < yDim - 1; y++)
		for (x = 1; x < xDim - 1; x++) {
			xp = (x + 1);
			yp = (y + 1);
			xm = (x - 1);
			ym = (y - 1);

			newVal = scale * ((-4.0f * chemData[x * chemTDA + y]) +
			    chemData[xm * chemTDA + y] + chemData[xp * chemTDA + y] + 
			    chemData[x  * chemTDA + ym] + chemData[x * chemTDA + yp]);

			diffData[x * diffTDA + y] = newVal;
		}

	return EC_OK;
}

int brIMatrix3DCopyToImage(brEval args[], brEval *result, brInstance *i) {
	brMatrix3D *sourceMatrix = BRBIGMATRIX3D(&args[0]);
	brImageData *d = (brImageData*)BRPOINTER(&args[2]);
	float* sourceData = sourceMatrix->getGSLVector()->data;
	unsigned int sourceTDA = sourceMatrix->xDim();
	unsigned int zOffset;
	unsigned char *pdata;
	double scale = 255.0 * BRDOUBLE(&args[4]);
	unsigned int offset = BRINT(&args[3]);
	int r;
	int x, y, xmax, ymax, zmax;

	xmax = sourceMatrix->xDim();
	ymax = sourceMatrix->yDim();
    zmax = sourceMatrix->zDim();
    zOffset = xmax * ymax * BRINT(&args[1]);
    
	if (xmax > d->x)
		xmax = d->x;
	if (ymax > d->y)
		ymax = d->y;
		
	pdata = d->data + offset;

	for (y = 0; y < ymax; y++)
	 	for (x = 0; x < xmax; x++) {
			r = (int)(sourceData[zOffset + x * sourceTDA + y] * scale);
			if (r > 255)
				*pdata = 255;
			else
				*pdata = r;
			pdata += 4;
	 	}

	if (d->textureNumber == -1)
		d->textureNumber = slTextureNew(i->engine->camera);

	slUpdateTexture(i->engine->camera, d->textureNumber, d->data, d->x, d->y, GL_RGBA);

	return EC_OK;
}

// end matrix interface

void breveInitMatrixFunctions(brNamespace *n) {
    // 2d matrix interface
	brNewBreveCall(n, "matrix2DNew", brIMatrix2DNew, AT_POINTER, AT_INT, AT_INT, 0);
	brNewBreveCall(n, "matrix2DFree", brIMatrix2DFree, AT_NULL, AT_POINTER, 0);
	brNewBreveCall(n, "matrix2DGet", brIMatrix2DGet, AT_DOUBLE, AT_POINTER, AT_INT, AT_INT, 0);
	brNewBreveCall(n, "matrix2DSet", brIMatrix2DSet, AT_NULL, AT_POINTER, AT_INT, AT_INT, AT_DOUBLE, 0);
	brNewBreveCall(n, "matrix2DSetAll", brIMatrix2DSetAll, AT_NULL, AT_POINTER, AT_DOUBLE, 0);
	brNewBreveCall(n, "matrix2DClamp", brIMatrix2DClamp, AT_NULL, AT_POINTER, AT_DOUBLE, AT_DOUBLE, 0);
	brNewBreveCall(n, "matrix2DCopy", brIMatrix2DCopy, AT_NULL, AT_POINTER, AT_POINTER, 0);
	brNewBreveCall(n, "matrix2DDiffusePeriodic", brIMatrix2DDiffusePeriodic, AT_NULL, AT_POINTER, AT_POINTER, AT_DOUBLE, 0);
	brNewBreveCall(n, "matrix2DDiffuse", brIMatrix2DDiffuse, AT_NULL, AT_POINTER, AT_POINTER, AT_DOUBLE, 0);
	brNewBreveCall(n, "matrix2DCopyToImage", brIMatrix2DCopyToImage, AT_NULL, AT_POINTER, AT_POINTER, AT_INT, AT_DOUBLE, 0);
	brNewBreveCall(n, "matrix2DScale", brIMatrix2DScale, AT_NULL, AT_POINTER, AT_DOUBLE, 0);
	brNewBreveCall(n, "matrix2DGetAbsoluteSum", brIMatrix2DGetAbsoluteSum, AT_DOUBLE, AT_POINTER, 0);
	brNewBreveCall(n, "matrix2DAddScaled", brIMatrix2DAddScaled, AT_NULL, AT_POINTER, AT_POINTER, AT_DOUBLE, 0);
	brNewBreveCall(n, "matrix2DAddScalar", brIMatrix2DAddScalar, AT_NULL, AT_POINTER, AT_DOUBLE, 0);
	brNewBreveCall(n, "matrix2DMulElements", brIMatrix2DMulElements, AT_NULL, AT_POINTER, AT_POINTER, 0);
	//brNewBreveCall(n, "matrixBlasMul", brIMatrixBlasMul, AT_NULL, AT_POINTER, AT_POINTER, AT_POINTER, 0);
    // 3d matrix interface
	brNewBreveCall(n, "matrix3DNew", brIMatrix3DNew, AT_POINTER, AT_INT, AT_INT, AT_INT, 0);
	brNewBreveCall(n, "matrix3DFree", brIMatrix3DFree, AT_NULL, AT_POINTER, 0);	
	brNewBreveCall(n, "matrix3DGet", brIMatrix3DGet, AT_DOUBLE, AT_POINTER, AT_INT, AT_INT, AT_INT, 0);
	brNewBreveCall(n, "matrix3DSet", brIMatrix3DSet, AT_NULL, AT_POINTER, AT_INT, AT_INT, AT_INT, AT_DOUBLE, 0);
	brNewBreveCall(n, "matrix3DSetAll", brIMatrix3DSetAll, AT_NULL, AT_POINTER, AT_DOUBLE, 0);
	brNewBreveCall(n, "matrix3DCopy", brIMatrix3DCopy, AT_NULL, AT_POINTER, AT_POINTER, 0);
	brNewBreveCall(n, "matrix3DDiffusePeriodic", brIMatrix3DDiffusePeriodic, AT_NULL, AT_POINTER, AT_POINTER, AT_DOUBLE, 0);
	brNewBreveCall(n, "matrix3DCopyToImage", brIMatrix3DCopyToImage, AT_NULL, AT_POINTER, AT_INT, AT_POINTER, AT_INT, AT_DOUBLE, 0);
//	brNewBreveCall(n, "matrix3DDiffuse", brIMatrix3DDiffuse, AT_NULL, AT_POINTER, AT_POINTER, AT_DOUBLE, 0);
	brNewBreveCall(n, "matrix3DScale", brIMatrix3DScale, AT_NULL, AT_POINTER, AT_DOUBLE, 0);
	brNewBreveCall(n, "matrix3DGetAbsoluteSum", brIMatrix3DGetAbsoluteSum, AT_DOUBLE, AT_POINTER, 0);
	brNewBreveCall(n, "matrix3DAddScaled", brIMatrix3DAddScaled, AT_NULL, AT_POINTER, AT_POINTER, AT_DOUBLE, 0);
	brNewBreveCall(n, "matrix3DAddScalar", brIMatrix3DAddScalar, AT_NULL, AT_POINTER, AT_DOUBLE, 0);
	brNewBreveCall(n, "matrix3DMulElements", brIMatrix3DMulElements, AT_NULL, AT_POINTER, AT_POINTER, 0);
	//brNewBreveCall(n, "matrixBlasMul", brIMatrixBlasMul, AT_NULL, AT_POINTER, AT_POINTER, AT_POINTER, 0);
}
