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

#include "slBrevePluginAPI.h"
#include "DigitizerFunctions.h"

int brDigitizerOpenCamera(brEval args[], brEval *target, void *i) {
	brDigitizer *data = new brDigitizer;

	data->instance = ccOpenCamera(data, BRINT(&args[0]), BRINT(&args[1]));

	if(!data->instance) {
		slFree(data);
		slMessage(0, "Error opening camera driver\n");
		return EC_ERROR;
	}

	data->width = BRINT(&args[0]);
	data->height = BRINT(&args[1]);

	data->map = ccNewIntensityMap(20);

	target->set( data );

	return EC_OK;
}

int brDigitizerFlip(brEval args[], brEval *target, void *i) {
	brDigitizer *b = (brDigitizer*)BRPOINTER(&args[0]);

	if(!b) return EC_ERROR;

	b->flip = !b->flip;

	target->set( b->flip );

	return EC_OK;
}

int brDigitizerUpdateFrame(brEval args[], brEval *target, void *i) { 
	brDigitizer *b = (brDigitizer*)BRPOINTER(&args[0]);
	unsigned char *output = (unsigned char*)BRPOINTER( &args[1] );
	int value;

	if(!b) return EC_ERROR;

	value = SGIdle(b->instance);

	ccGetPixels( b, output );

	ccIntensityMapARGB(b, b->map->active);
	ccIntensityMapDeltaARGB(b);

	return EC_OK;
}

int brDigitizerReferenceMap(brEval args[], brEval *target, void *i) { 
	brDigitizer *b = (brDigitizer*)BRPOINTER(&args[0]);

	if(!b) return EC_ERROR;
	ccIntensityMapARGB(b, !b->map->active);

	return EC_OK;
}

int brDigitizerHighestDelta(brEval args[], brEval *target, void *i) { 
	brDigitizer *b = (brDigitizer*)BRPOINTER(&args[0]);
	if(!b) return EC_ERROR;
	target->set( b->map->delta[b->map->highest] );
	return EC_OK;
}

int brDigitizerAverageDelta(brEval args[], brEval *target, void *i) { 
	brDigitizer *b = (brDigitizer*)BRPOINTER(&args[0]);
	if(!b) return EC_ERROR;
	target->set( b->map->average );
	return EC_OK;
}


int brDigitizerHighestDeltaLocation(brEval args[], brEval *target, void *i) { 
	brDigitizer *b = (brDigitizer*)BRPOINTER(&args[0]);
	slVector *v = &BRVECTOR(target);	

	if(!b) return EC_ERROR;

	// note that the camera shows a non-mirrored view, so we'll flip the coord

	v->x = b->map->size - (b->map->highest % b->map->size);
	v->y = b->map->size - (b->map->highest / b->map->size);

	v->x *= (b->width / b->map->size);
	v->y *= (b->height / b->map->size);

	v->z = 0;

	return EC_OK;
}

int brDigitizerIntensityMapValue(brEval args[], brEval *target, void *i) { 
	brDigitizer *b = (brDigitizer*)BRPOINTER(&args[0]);
	int x = BRINT(&args[1]);
	int y = BRINT(&args[2]);

	if(!b) return EC_ERROR;

	target->set( b->map->map[b->map->active][(b->map->size * y) + x] );

	return EC_OK;
	
}
int brDigitizerDeltaMapValue(brEval args[], brEval *target, void *i) { 
	brDigitizer *b = (brDigitizer*)BRPOINTER(&args[0]);
	int x = BRINT(&args[1]);
	int y = BRINT(&args[2]);

	if(!b) return EC_ERROR;
	target->set( b->map->delta[(b->map->size * y) + x] );

	return EC_OK;
}
	

int brDigitizerCloseCamera(brEval args[], brEval *target, void *i) { 
	brDigitizer *b = (brDigitizer*)BRPOINTER(&args[0]);

	if(!b) return EC_OK;

	ccCloseCamera(b->instance);
	slFree(b);

	return EC_OK;
}

DLLEXPORT int slInitDigitizerFuncs(void *n) {
	brNewBreveCall(n, "digitizerOpenCamera", brDigitizerOpenCamera, AT_POINTER, AT_INT, AT_INT, 0);
	brNewBreveCall(n, "digitizerUpdateFrame", brDigitizerUpdateFrame, AT_NULL, AT_POINTER, AT_POINTER, 0);
	brNewBreveCall(n, "digitizerReferenceMap", brDigitizerReferenceMap, AT_NULL, AT_POINTER, 0);
	brNewBreveCall(n, "digitizerFlip", brDigitizerFlip, AT_INT, AT_POINTER, 0);
	brNewBreveCall(n, "digitizerHighestDelta", brDigitizerHighestDelta, AT_INT, AT_POINTER, 0);
	brNewBreveCall(n, "digitizerAverageDelta", brDigitizerAverageDelta, AT_INT, AT_POINTER, 0);
	brNewBreveCall(n, "digitizerIntensityMapValue", brDigitizerIntensityMapValue, AT_INT, AT_POINTER, AT_INT, AT_INT, 0);
	brNewBreveCall(n, "digitizerDeltaMapValue", brDigitizerDeltaMapValue, AT_INT, AT_POINTER, AT_INT, AT_INT, 0);
	brNewBreveCall(n, "digitizerHighestDeltaLocation", brDigitizerHighestDeltaLocation, AT_VECTOR, AT_POINTER, 0);
	brNewBreveCall(n, "digitizerCloseCamera", brDigitizerCloseCamera, AT_NULL, AT_POINTER, 0);
}

ComponentInstance ccOpenCamera(brDigitizer *data, int width, int height) {
	OSErr err;
	ComponentInstance co;
	SGChannel videoChannel;
	Rect theBounds;
	GWorldPtr pixelWorld;
	
	SetRect(&theBounds, 0, 0, width, height);
	
	NewGWorld( &pixelWorld, 32, &theBounds, NULL, NULL, 0 );
	
	data->rowBytes = (*GetGWorldPixMap(pixelWorld))->rowBytes & 0x3FFF;
	data->pixels = (unsigned char*)GetPixBaseAddr(GetGWorldPixMap(pixelWorld));
	
	co = OpenDefaultComponent('barg', 0);
	
	if(!co) {
		fprintf(stderr, "Error opening QuickTime component!\n");
		return NULL;
	}
	
	err = SGInitialize(co);
	
	if(err) {
		fprintf(stderr, "Error initializing QuickTime sequence grabber: %d\n", err);
		return NULL;
	}
	
	err = SGSetGWorld(co, pixelWorld, NULL);
	
	if(err) {
		fprintf(stderr, "Error configuring QuickTime sequence grabber: %d\n", err);
		return NULL;
	}
	
	err = SGNewChannel(co, VideoMediaType, &videoChannel);

	if(err) {
		fprintf(stderr, "Error creating QuickTime sequence grabber video channel: %d\n", err);
		return NULL;
	}
	
	err = SGSetChannelUsage(videoChannel, seqGrabPreview);

	if(err) {
		fprintf(stderr, "Error initializing QuickTime sequence grabber video channel: %d\n", err);
		return NULL;
	}
	
	err = SGSetChannelBounds(videoChannel, &theBounds);

	if(err) {
		fprintf(stderr, "Error configuring QuickTime sequence grabber video channel: %d\n", err);
		return NULL;
	}
	
	
	SGStartPreview(co);
	
	return co;
}

void ccGetPixels(brDigitizer *b, unsigned char *dst) {
	int srowBytes = b->width * 4;
	int drowBytes = b->width * 4;
	int h, w, dindex = 0;
	int hh;

	for(h=0;h<b->height;h++) {
		dindex = ((b->height - h) * drowBytes) - 1;
		for(w=0;w<b->width;w++) {
			if(b->flip) hh = b->height - (h + 1);
			else hh = h;

			// input = ARGB
			// output = RGBA

			dst[dindex--] = b->pixels[hh*b->rowBytes + w*4 + 0];
			dst[dindex--] = b->pixels[hh*b->rowBytes + w*4 + 3];
			dst[dindex--] = b->pixels[hh*b->rowBytes + w*4 + 2];
			dst[dindex--] = b->pixels[hh*b->rowBytes + w*4 + 1];
		}
	}
}

void ccCloseCamera(ComponentInstance co) {
	CloseComponent(co);
}

unsigned char ccAveragePixels(unsigned char *buffer, int size, int sampleSize) {
	int total = 0;
	int n;
	int increment = 3 * sampleSize;

	for(n=0;n<size;n+=increment) total += buffer[n];

	return (total / (size / 1));
}

/*
		1
	1	2	1
		1

		=

		1/6
	1/6	1/5	1/6
		1/6
*/


void ccGaussianBlurRGB(unsigned char *source, unsigned char *dest, int x, int y) {
	int n, m;
	int dindex = 0;
	int rowBytes = x * 3;

	memset(dest, 0, y*rowBytes);

	for(n=0;n<y;n++) {
		for(m=0;m<rowBytes;m++) {
			dest[dindex] = source[dindex] / 3;

			if(n > 0) dest[dindex] += source[dindex - rowBytes] / 6;
			if(n < (x - 1)) dest[dindex] += source[dindex - rowBytes] / 6;

			if(m > 3) dest[dindex] += source[dindex - 3] / 6;
			if(m < (rowBytes - 4)) dest[dindex] += source[dindex + 3] / 6;

			dindex++;
		}
	}
}

void ccFilterMaskRGB(unsigned char *source, unsigned char *dest, int width, int height, int *mask, int mSize) {
	int x, y;
	int mx, my;
	int maskOffset = (mSize - 1) / 2;
	int targetX, targetY;
	int pixel;
	int sums;

	for(x=0;x<width;x++) {
		for(y=0;y<height;y++) {

			pixel = 0;
			sums = 0;

			for(mx=0;mx<mSize;mx++) {
				for(my=0;my<mSize;my++) {
					targetX = x + (mx - maskOffset);
					targetY = y + (my - maskOffset);

					pixel += mask[mx + my * mSize];
					sums += mask[mx + my * mSize];
				}
			}

		}
	}
}

ccIntensityMap *ccNewIntensityMap(int size) {
	ccIntensityMap *map;

	map = new ccIntensityMap;
	map->map[0] = new int[ size * size ];
	map->map[1] = new int[ size * size ];
	map->delta = new int[ size * size ];

	memset(map->delta, 0, size * size * sizeof(int));
	memset(map->map[0], 0, size * size * sizeof(int));
	memset(map->map[1], 0, size * size * sizeof(int));

	map->active = 0;
	map->size = size;

	return map;
}

void ccIntensityMapDeltaARGB(brDigitizer *b) {
	int n;
	int highestValue = 0;
	int x = b->width;
	int y = b->height;
	ccIntensityMap *map = b->map;
	unsigned char *source = b->pixels;
	int a = 0;

	map->average = 0;

	for(n=0;n<map->size*map->size;n++) {
		// Honestly, who knows what's going on here?
		
		map->delta[n] = ( int )( map->delta[ n ] * .7 );
		map->delta[n] += ( map->map[0][n] - map->map[1][n] );

		map->average += abs(map->delta[n]);

		if(map->delta[n] > highestValue) {
			highestValue = map->delta[n];
			map->highest = n;
		}
	}

	map->average /= map->size * map->size;
}

void ccIntensityMapARGB(brDigitizer *b, int active) {
	int rowBytes;
	int xscale;
	int yscale;
	int pixelCount;
	int mapX, mapY;
	int m, n;
	int total;
	unsigned char *start;
	ccIntensityMap *map = b->map;
	unsigned char *source = b->pixels;
	
	xscale = (b->width * 4) / map->size;
	yscale = b->height / map->size;

	pixelCount = (3 * b->width / map->size) * yscale;

	for(mapX=0;mapX<map->size;mapX++) {
		for(mapY=0;mapY<map->size;mapY++) {
			total = 0;
			start = &source[mapY * yscale * b->rowBytes + (xscale * mapX)];

			for(m=0;m<yscale;m++) {
				for(n=0;n<xscale;n++) {
					if(n % 4 == 2) total += start[m * b->rowBytes + n];
				}
			}

			map->map[active][(mapY * map->size) + mapX] = total / (pixelCount / 3);
		}
	}
}

void ccColorRectRGBA(unsigned char *source, int x, int y, int rx, int ry, int rw, int rh, int color, unsigned char value) {
	int rowBytes = x * 3;
	int n;
	int m;

	unsigned char *start;

	start = &source[ry * rowBytes + (rx * 4)];

	for(n=0;n<rw;n++) {
		for(m=0;m<rh;m++) {
			start[m * rowBytes + (n * 4) + color] = value;
		}
	}
}
