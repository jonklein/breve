/*****************************************************************************
 *                                                                           *
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

/*! \addtogroup InternalFunctions */
/*@{*/

#include "kernel.h"
#include "breveFunctionsImage.h"

/*!
	\brief Retuns the width of an image.

	int imageGetWidth(brImageData pointer).
*/

int brIImageGetWidth(brEval *args, brEval *result, brInstance *i) {
	brImageData *dm = BRPOINTER(&args[0]);

	BRINT(result) = dm->x;
	return EC_OK;
}

/*!
	\brief Retuns the height of an image.

	int imageGetHeight(brImageData pointer).
*/

int brIImageGetHeight(brEval *args, brEval *result, brInstance *i) {
	brImageData *dm = BRPOINTER(&args[0]);

	BRINT(result) = dm->y;
	return EC_OK;
}

/*!
	\brief Get the pixel value at the given coordinates.

	int imageGetValueAtCoordinates(brImageData pointer, int, int).

	The value is on a scale from 0.0 to 1.0.

	Since the data is RGB, you'll have to offset the X value accordingly
	to get the desired red, green or blue pixel.
*/

int brIImageGetValueAtCoordinates(brEval *args, brEval *result, brInstance *i) {
	brImageData *dm = BRPOINTER(&args[0]);
	int x = BRINT(&args[1]);
	int y = BRINT(&args[2]);

	if(x < 0 || x >= (dm->x * 4) || y < 0 || y >= dm->y) {
		slMessage(DEBUG_ALL, "data matrix access (%d, %d) out of bounds (%d, %d)\n", x, y, dm->x, dm->y);
		return EC_OK;
	}

	BRDOUBLE(result) = dm->data[y * (dm->x * 4) + x] / 255.0;

	return EC_OK;
}

/*!
	\brief Get the pixel value at the given coordinates.

	void imageSetValueAtCoordinates(brImageData pointer, int, int, double).

	The value is on a scale from 0.0 to 1.0.

	Since the data is RGB, you'll have to offset the X value accordingly
	to set the desired red, green or blue pixel.
*/

int brIImageSetValueAtCoordinates(brEval *args, brEval *result, brInstance *i) {
	brImageData *dm = BRPOINTER(&args[0]);
	int x = BRINT(&args[1]);
	int y = BRINT(&args[2]);
	int value = 255 * BRDOUBLE(&args[3]);

	if(x < 0 || x >= (dm->x * 4) || y < 0 || y >= dm->y) {
		slMessage(DEBUG_ALL, "data matrix access (%d, %d) out of bounds (%d, %d)\n", x, y, dm->x, dm->y);
		return EC_OK;
	}

	dm->data[y * (dm->x * 4) + x] = value;

	return EC_OK;
}

/*!
	\brief Reads Image data from the screen.
*/

int brIImageReadPixels(brEval *args, brEval *result, brInstance *i) {
	brImageData *dm = BRPOINTER(&args[0]);
	int x = BRINT(&args[1]);
	int y = BRINT(&args[2]);

	glReadPixels(x, y, dm->x, dm->y, GL_RGBA, GL_UNSIGNED_BYTE, dm->data);

	return EC_OK;
}

/*!
	\brief Loads an image from a file of a given name.

	brImageData pointer imageLoadFromFile(string).
*/

int brIImageLoadFromFile(brEval *args, brEval *result, brInstance *i) {
	brImageData *dm;
	char *file;
	int c;

	file = brFindFile(i->engine, BRSTRING(&args[0]), NULL);

	if(!file) {
		slMessage(DEBUG_ALL, "Error loading image file \"%s\": no such file\n", BRSTRING(&args[0]));
		BRPOINTER(result) = NULL;
		return EC_OK;
	}

	dm = slMalloc(sizeof(brImageData));

	dm->data = slReadImage(file, &dm->x, &dm->y, &c, 0);
	dm->textureNumber = -1;

	if(!file) {
		slMessage(DEBUG_ALL, "Error reading image from file \"%s\": unrecognized format or corrupt file\n", file);
		slFree(file);
		BRPOINTER(result) = NULL;
		return EC_OK;
	}

	slFree(file);

	BRPOINTER(result) = dm;

	return EC_OK;
}

/*!
	\brief Updates the texture and returns the texture number number associated with an image.
*/

int brIImageUpdateTexture(brEval *args, brEval *result, brInstance *i) { 
	brImageData *image = BRPOINTER(&args[0]);

	if(!image) {
		BRINT(result) = -1;
		return EC_OK;
	}

   	image->textureNumber = slAddTexture(i->engine->world, image->textureNumber, image->data, image->x, image->y, GL_RGBA);

	BRINT(result) = image->textureNumber;

	return EC_OK;
}

/*!
	\brief Gets a pointer to the raw pixel data of an image.

	char pointer imageGetPixelPointer(brImageData pointer).

	Used by plugins that need raw pixel access.
*/

int brIImageGetPixelPointer(brEval *args, brEval *result, brInstance *i) {
	brImageData *dm = BRPOINTER(&args[0]);

	if(!dm) {
		slMessage(DEBUG_ALL, "pixelPointer called with uninitialized image data\n");
		return EC_ERROR;
	}

	BRPOINTER(result) = dm->data;

	return EC_OK;
}

/*!
	\brief Writes an image buffer out to a file.

	int imageWriteToFile(brImageData pointer, string).
*/

int brIImageWriteToFile(brEval *args, brEval *result, brInstance *i) {
	brImageData *dm = BRPOINTER(&args[0]);
	char *file;

	file = brOutputPath(i->engine, BRSTRING(&args[1]));

	BRINT(result) = slWritePNGImage(file, dm->x, dm->y, dm->data, 4, 1);

	slFree(file);

	return EC_OK;

}

/*!
	\brief Initializes an empty image buffer of a given size.

	brImageData pointer imageDataInit(int, int).
*/

int brIImageDataInit(brEval *args, brEval *result, brInstance *i) {
	brImageData *dm;
	int x, y;

	int width = BRINT(&args[0]);
	int height = BRINT(&args[1]);

	dm = slMalloc(sizeof(brImageData));

	dm->data = slMalloc(sizeof(unsigned char) * width * height * 4);
	dm->x = width;
	dm->y = height;
	dm->textureNumber = -1;

	for(y=0;y<height;y++) {
		for(x=0;x<width;x++) {
			dm->data[(y * width * 4) + (x * 4) + 3] = 255;
		}
	}

	BRPOINTER(result) = dm;

	return EC_OK;
}

/*!
	\brief Frees image data.

	imageDataFree(brImageData).
*/

int brIImageDataFree(brEval *args, brEval *result, brInstance *i) {
	brImageData *dm = BRPOINTER(&args[0]);
	GLuint texture = dm->textureNumber;

	if(dm->textureNumber != -1) glDeleteTextures(1, &texture);

	slFree(dm->data);
	slFree(dm);

	return EC_OK;
}

/*@}*/

void breveInitImageFunctions(brNamespace *n) {
	brNewBreveCall(n, "imageGetWidth", brIImageGetWidth, AT_INT, AT_POINTER, 0);
	brNewBreveCall(n, "imageGetHeight", brIImageGetHeight, AT_INT, AT_POINTER, 0);
	brNewBreveCall(n, "imageGetPixelPointer", brIImageGetPixelPointer, AT_POINTER, AT_POINTER, 0);
	brNewBreveCall(n, "imageGetValueAtCoordinates", brIImageGetValueAtCoordinates, AT_DOUBLE, AT_POINTER, AT_INT, AT_INT, 0);
	brNewBreveCall(n, "imageSetValueAtCoordinates", brIImageSetValueAtCoordinates, AT_NULL, AT_POINTER, AT_INT, AT_INT, AT_DOUBLE, 0);
	brNewBreveCall(n, "imageLoadFromFile", brIImageLoadFromFile, AT_POINTER, AT_STRING, 0);
	brNewBreveCall(n, "imageWriteToFile", brIImageWriteToFile, AT_INT, AT_POINTER, AT_STRING, 0);
	brNewBreveCall(n, "imageDataFree", brIImageDataFree, AT_NULL, AT_POINTER, 0);
	brNewBreveCall(n, "imageDataInit", brIImageDataInit, AT_POINTER, AT_INT, AT_INT, 0);
	brNewBreveCall(n, "imageUpdateTexture", brIImageUpdateTexture, AT_INT, AT_POINTER, 0);
	brNewBreveCall(n, "imageReadPixels", brIImageReadPixels, AT_NULL, AT_POINTER, AT_INT, AT_INT, 0);
}

