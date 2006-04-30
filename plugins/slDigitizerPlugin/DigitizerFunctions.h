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

#include <QuickTime/QuickTime.h>

typedef struct brDigitizer brDigitizer;
typedef struct ccIntensityMap ccIntensityMap;

struct ccIntensityMap {
	int size;
	int *map[2];
	int *referenceMap;
	int *delta;
	int active;
	int highest;
	int average;
};

struct brDigitizer {
    ComponentInstance instance;
    unsigned char *pixels;

    ccIntensityMap *map;

    int width;
    int height;
    int rowBytes;
    long size;
    int average;
	int flip;
};

ComponentInstance ccOpenCamera(brDigitizer *data, int width, int height);
unsigned char *ccGrabFrame(ComponentInstance co, long *size);
unsigned char *ccGrabFrameAsync(ComponentInstance co, long *size);

void ccCloseCamera(ComponentInstance co);

unsigned char ccAveragePixels(unsigned char *buffer, int size, int sampleSize);

void ccIntensityMapDeltaARGB(brDigitizer *b);
void ccIntensityMapARGB(brDigitizer *b, int active);

ccIntensityMap *ccNewIntensityMap(int size);

void ccGaussianBlurRGB(unsigned char *source, unsigned char *dest, int x, int y);
void ccColorRectRGB(unsigned char *source, int x, int y, int rx, int ry, int rw, int rh, int color, unsigned char value);
void ccGetPixels(brDigitizer *b, unsigned char *dst);

int brDigitizerOpenCamera(brEval args[], brEval *target, void *i);
int brDigitizerUpdateFrame(brEval args[], brEval *target, void *i);
int brDigitizerHighestIntensityDelta(brEval args[], brEval *target, void *i);
int brDigitizerHighestIntensityDeltaLocation(brEval args[], brEval *target, void *i);
int brDigitizerCloseCamera(brEval args[], brEval *target, void *i);
DLLEXPORT int slInitDigitizerFuncs(void *n);


