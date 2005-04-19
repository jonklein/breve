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

#include "kernel.h"

#define BRWORLDOBJECTPOINTER(p)	((slWorldObject*)BRPOINTER(p))

/*! \addtogroup InternalFunctions */
/*@{*/

/*!
	\brief Enables or disables fast-physics.

	setStepFast(int).
*/

int brISetStepFast(brEval args[], brEval *target, brInstance *i) {
	slWorldSetPhysicsMode(i->engine->world, BRINT(&args[0]));
	return EC_OK;
}

/*!
	\brief Sets the iterations parameter for quickstep.

	setStepFastIterations(int).
*/

int brISetStepFastIterations(brEval args[], brEval *target, brInstance *i) {
	slWorldSetQuickstepIterations(i->engine->world, BRINT(&args[0]));
	return EC_OK;
}

/*!
	\brief Enables or disables drawing of every frame.  

	setDrawEveryFrame(int).

	If enabled, every frame is drawn, even if it slows down the simulation.
	When disabled, frame may be dropped in order to allow the simulation to 
	run faster.
*/

int brISetDrawEveryFrame(brEval args[], brEval *target, brInstance *i) {
	i->engine->drawEveryFrame = BRINT(&args[0]);

	return EC_OK;
}

/*!
	\brief Sets the random seed for the simulation.

	void randomSeed(int).
*/

int brIRandomSeed(brEval args[], brEval *target, brInstance *i) {
	srandom(BRINT(&args[0]));

	return EC_OK;
}

/*!
	\brief Reads a random seed from /dev/random, if possible.

	int randomSeedFromDevRandom().

	Returns -1 if the seed could not be set from /dev/random.
*/

int brIRandomSeedFromDevRandom(brEval args[], brEval *target, brInstance *i) {
	FILE *f;
	unsigned int seed = 0;

	f = fopen("/dev/random", "r");

	if (!f || !fread(&seed, sizeof seed, 1, f) || !seed) {
		if (f) fclose(f);
		if ((f = fopen("/dev/urandom", "r")))
		    fread(&seed, sizeof seed, 1, f);
	}

	if (f) fclose(f);

	if (seed) {
		BRINT(target) = 0;
		slMessage(DEBUG_ALL, "read seed %u from random device\n", seed);
		srandom(seed);
	} else BRINT(target) = -1;

	return EC_OK;
}

/*!
	\brief Returns the current simulation time.

	double getTime().
*/

int brIGetTime(brEval args[], brEval *target, brInstance *i) {
	BRDOUBLE(target) = slWorldGetAge(i->engine->world);

	return EC_OK;
}

/*!
	\brief Loads a texture from an image file [deprecated].  

	USE OF THIS FUNCTION IS DEPRECATED.  The image loading methods from
	breveFunctionsImage.c are now used instead.
*/

int brILoadTexture(brEval args[], brEval *target, brInstance *i) {
	unsigned char *pixels = NULL;
	char *path;
	int w, h, c;
	int useAlpha = BRINT(&args[1]);

	path = brFindFile(i->engine, BRSTRING(&args[0]), NULL);

	if(!path) {
		slMessage(DEBUG_ALL, "Cannot locate image file for \"%s\"\n", BRSTRING(&args[0]));
		BRINT(target) = -1;
		return EC_OK;
	}

	pixels = slReadImage(path, &w, &h, &c, useAlpha);

	slFree(path);

	if(!pixels) {
		slMessage(DEBUG_ALL, "Unrecognized image format in file \"%s\"\n", BRSTRING(&args[0]));
		BRINT(target) = -1;
		return EC_OK;
	}

	BRINT(target) = slTextureNew(i->engine->camera);
	slUpdateTexture(i->engine->camera, BRINT(target), pixels, w, h, GL_RGBA);

	slFree(pixels);

	return EC_OK;
}

/*!
	\brief Steps the world simulation forward.

	void worldStep(double time, double integrationStep).

	The time parameter is the total time to step forward.  The integrationStep
	is a smaller value which says how fast the integrator should step forward.
*/

int brIWorldStep(brEval args[], brEval *target, brInstance *i) {
	double totalTime = BRDOUBLE(&args[0]);
	double stepSize = BRDOUBLE(&args[1]);
	int error;

	i->engine->iterationStepSize = totalTime;
	BRDOUBLE(target) = slRunWorld(i->engine->world, totalTime, stepSize, &error);

	if(error) {
		brEvalError(i->engine, EE_SIMULATION, "Error in world simulation");
		return EC_ERROR;
	}

	return EC_OK;
}

/*!
	\brief Updates the "neighbors" for neighbor-detecting objects.

	void updateNeighbors().
*/

int brIUpdateNeighbors(brEval args[], brEval *target, brInstance *i) {
	slNeighborCheck(i->engine->world);

	return EC_OK;
}

/*!
	\brief Sets the neighborhood size for an object.

	setNeighborhoodSize(slWorld

	The neighborhood size specifies how far an object looks when collecting 
	neighbor information.
*/

int brISetNeighborhoodSize(brEval args[], brEval *target, brInstance *i) {
	slWorldObject *wo = BRWORLDOBJECTPOINTER(&args[0]);
	double size = BRDOUBLE(&args[1]);

	if(!wo) {
		brEvalError(i->engine, EE_SIMULATION, "uninitialized pointer passed to setNeighborhoodSize");
		return EC_ERROR;
	}

	slWorldObjectSetNeighborhoodSize(wo, size);

	return EC_OK;
}

/*!
	\brief Set the collision properties (mu, e, eT) of a stationary object.

	void setCollisionProperties(slWorldObject pointer stationary, double e, double eT, double mu).
*/

int brISetCollisionProperties(brEval args[], brEval *target, brInstance *i) {
	slWorldObject *o = BRWORLDOBJECTPOINTER(&args[0]);
	
	double e = BRDOUBLE(&args[1]);
	double eT = BRDOUBLE(&args[2]);
	double mu = BRDOUBLE(&args[3]);

	if(!o) {
		slMessage(DEBUG_ALL, "null pointer passed to setCollisionProperties\n");
		return EC_ERROR;
	}

	slWorldObjectSetCollisionE(o, e);
	slWorldObjectSetCollisionET(o, eT);
	slWorldObjectSetCollisionMU(o, mu);

	return EC_OK;
}

/*!
	\brief Gets an object's neighbors.

	list getNeighbors(slWorldObject pointer).
*/

int brIGetNeighbors(brEval args[], brEval *target, brInstance *i) {
	slWorldObject *wo = BRWORLDOBJECTPOINTER(&args[0]);
	std::vector<slWorldObject*>::iterator wi;
	brEval eval;

	if(!wo) {
		brEvalError(i->engine, EE_SIMULATION, "uninitialized pointer passed to getNeighbors");
		return EC_ERROR;
	}
	
	BRLIST(target) = brEvalListNew();

	eval.type = AT_INSTANCE;

	std::vector<slWorldObject*> &neighbors = slWorldObjectGetNeighbors(wo);

	for(wi = neighbors.begin(); wi != neighbors.end(); wi++ ) {
		// grab the neighbor instances from the userData of the neighbors

		BRINSTANCE(&eval) = (brInstance*)slWorldObjectGetCallbackData(*wi);

		if(BRINSTANCE(&eval) && BRINSTANCE(&eval)->status == AS_ACTIVE) brEvalListInsert(BRLIST(target), 0, &eval);
	}

	return EC_OK;
}

/*!
	\brief Returns the light exposure of a single object in the world, if light
	exposure detection is in use.

	int worldObjectGetLightExposure(slWorldObject pointer).
*/

int brIWorldObjectGetLightExposure(brEval args[], brEval *target, brInstance *i) {
	slWorldObject *wo = BRWORLDOBJECTPOINTER(&args[0]);

	BRINT(target) = slWorldObjectGetLightExposure(wo);

	return EC_OK;
}

/*!
	\brief Removes an object from the world and frees it.

	removeObject(slWorldObject pointer).
*/

int brIRemoveObject(brEval args[], brEval *target, brInstance *i) {
	slRemoveObject(i->engine->world, BRWORLDOBJECTPOINTER(&args[0]));
	slWorldFreeObject(BRWORLDOBJECTPOINTER(&args[0]));
 
	return EC_OK;
}

/*!
	\brief Enables or disables drawing of the bounding box for an object.

	setBoundingBox(slWorldObject pointer, int).
*/

int brISetBoundingBox(brEval args[], brEval *target, brInstance *i) {
	slWorldObject *o = BRWORLDOBJECTPOINTER(&args[0]);
	int value = BRINT(&args[1]);

	if(!o) {
		slMessage(DEBUG_ALL, "null pointer passed to setBoundingBox\n");
		return EC_ERROR;
	}

	if(value) slWorldObjectAddDrawMode(o, DM_BOUND);
	else slWorldObjectRemoveDrawMode(o, DM_BOUND);

	return EC_OK;
}

/*!
	\brief Enables or disables drawing of the axis lines for an object.

	setBoundingBox(slWorldObject pointer, int).
*/

int brISetDrawAxis(brEval args[], brEval *target, brInstance *i) {
	slWorldObject *o = BRWORLDOBJECTPOINTER(&args[0]);
	int value = BRINT(&args[1]);

	if(!o) {
		slMessage(DEBUG_ALL, "null pointer passed to setDrawAxis\n");
		return EC_OK;
	}

	if(value) slWorldObjectAddDrawMode(o, DM_AXIS);
	else slWorldObjectRemoveDrawMode(o, DM_AXIS);

	i->engine->camera->recompile = 1;

	return EC_OK;
}

int brISetNeighborLines(brEval args[], brEval *target, brInstance *i) {
	slWorldObject *o = BRWORLDOBJECTPOINTER(&args[0]);
	int value = BRINT(&args[1]);

	if(!o) {
		slMessage(DEBUG_ALL, "null pointer passed to setNeighborLines\n");
		return EC_ERROR;
	}

	if(value) slWorldObjectAddDrawMode(o, DM_NEIGHBOR_LINES);
	else slWorldObjectRemoveDrawMode(o, DM_NEIGHBOR_LINES);

	return EC_OK;
}

int brISetVisible(brEval args[], brEval *target, brInstance *i) {
	slWorldObject *o = BRWORLDOBJECTPOINTER(&args[0]);
	int visible = BRINT(&args[1]);

	if(!o) {
		slMessage(DEBUG_ALL, "null pointer passed to setVisible\n");
		return EC_ERROR;
	}

	if(!visible) slWorldObjectAddDrawMode(o, DM_INVISIBLE);
	else slWorldObjectRemoveDrawMode(o, DM_INVISIBLE);

	i->engine->camera->recompile = 1;

	return EC_OK;
}

int brISetTexture(brEval args[], brEval *target, brInstance *i) {
	slWorldObject *o = BRWORLDOBJECTPOINTER(&args[0]);
	int value = BRINT(&args[1]);

	if(!o) {
		slMessage(DEBUG_ALL, "null pointer passed to setTexture\n");
		return EC_ERROR;
	}

	slWorldObjectSetTexture(o, value);
	slWorldObjectSetTextureMode(o, BBT_NONE);
	i->engine->camera->recompile = 1;

	return EC_OK;
}

int brISetTextureScale(brEval args[], brEval *target, brInstance *i) {
	slWorldObject *o = BRWORLDOBJECTPOINTER(&args[0]);
	int value = BRINT(&args[1]);

	if(!o) {
		slMessage(DEBUG_ALL, "null pointer passed to setTexture\n");
		return EC_ERROR;
	}

	if(value < 1) {
		slMessage(DEBUG_ALL, "warning: texture scale must be positive #%d\n", value);
		value = 1;
	} 

	slWorldObjectSetTextureScale(o, value);

	return EC_OK;
}

int brISetBitmap(brEval args[], brEval *target, brInstance *i) {
	slWorldObject *o = BRWORLDOBJECTPOINTER(&args[0]);
	int texture = BRINT(&args[1]);

	if(!o) {
		slMessage(DEBUG_ALL, "null pointer passed to setBitmap\n");
		return EC_ERROR;
	}

	slWorldObjectSetTexture(o, texture);
	slWorldObjectSetTextureMode(o, BBT_BITMAP);

	return EC_OK;
}

int brISetBitmapRotation(brEval args[], brEval *target, brInstance *i) {
	slWorldObject *o = BRWORLDOBJECTPOINTER(&args[0]);

	if(!o) {
		slMessage(DEBUG_ALL, "null pointer passed to setBitmapRotation\n");
		return EC_ERROR;
	}

	slWorldObjectSetBitmapRotation(o, RADTODEG(BRDOUBLE(&args[1])));

	return EC_OK;
}

/*
	step 1: compute the vector projection on the viewer's perspective plane.
			this is c . (nc + v) where c is the camera vector, v is the 
			vector in question, and n is some constant.

			it becomes n * c.c + c.v = 0
		
			c.c = 1, so n = -c.v
		
*/
		
int brISetBitmapRotationTowardsVector(brEval args[], brEval *target, brInstance *i) {
	slWorldObject *o = BRWORLDOBJECTPOINTER(&args[0]);
	slVector *v = &BRVECTOR(&args[1]);
	slVector offset, vdiff, vproj, up, axis;
	double dot, rotation;

	slVectorSet(&up, 0, 1, 0);

	slVectorCopy(&i->engine->camera->location, &offset);
	slVectorNormalize(&offset);
	slVectorNormalize(v);

	slVectorCross(&up, &offset, &axis);
	slVectorCross(&offset, &axis, &up);

	/* c.v = -n */

	dot = -slVectorDot(&offset, v);

	slVectorMul(&offset, dot, &vdiff);
	slVectorAdd(&vdiff, v, &vproj);
	
	if(!o) {
		slMessage(DEBUG_ALL, "null pointer passed to setBitmapRotationTowardsVector\n");
		return EC_ERROR;
	}

	/* we need to get the scalar rotation of v about offset vector */

	rotation = acos(slVectorDot(&up, &vproj));

	if(slVectorDot(&axis, &vproj) > 0.0) rotation = 2*M_PI - rotation;

	slWorldObjectSetBitmapRotation(o, RADTODEG(rotation));

	return EC_OK;
}

int brISetAlpha(brEval args[], brEval *target, brInstance *i) {
	slWorldObject *o = BRWORLDOBJECTPOINTER(&args[0]);

	if(!o) {
		slMessage(DEBUG_ALL, "null pointer passed to setAlpha\n");
		return EC_ERROR;
	}

	slWorldObjectSetAlpha(o, BRDOUBLE(&args[1]));

	return EC_OK;
}

int brISetLightmap(brEval args[], brEval *target, brInstance *i) {
	slWorldObject *o = BRWORLDOBJECTPOINTER(&args[0]);
	int value = BRINT(&args[1]);

	if(!o) {
		slMessage(DEBUG_ALL, "null pointer passed to setLightmap\n");
		return EC_ERROR;
	}

	slWorldObjectSetTexture(o, value);
	slWorldObjectSetTextureMode(o, BBT_LIGHTMAP);

	return EC_OK;
}

/*!
	\brief Sets the color of an object.
*/

int brISetColor(brEval args[], brEval *target, brInstance *i) {
	slWorldObject *o = BRWORLDOBJECTPOINTER(&args[0]);
	slVector *color = &BRVECTOR(&args[1]);

	if(!o) {
		slMessage(DEBUG_ALL, "null pointer passed to setColor\n");
		return EC_ERROR;
	}

	slWorldObjectSetColor(o, color);

	return EC_OK;
}

int brIGetMainCameraPointer(brEval args[], brEval *target, brInstance *i) {
	BRPOINTER(target) = i->engine->camera;
	return EC_OK;
}

int brICameraSetTarget(brEval args[], brEval *target, brInstance *i) {
	slVector *tar = &BRVECTOR(&args[0]);

	slVectorCopy(tar, &i->engine->camera->target);

	slUpdateCamera(i->engine->camera);

	return EC_OK;
}

int brICameraSetOffset(brEval args[], brEval *target, brInstance *i) {
	slVector *loc = &BRVECTOR(&args[0]);
	double x;

	i->engine->camera->zoom = slVectorLength(loc);

	slVectorNormalize(loc);

	// don't ask me to explain the following.  please.
	// it took me a while to work out on paper and i didn't 
	// save my notes.  i never learn.  never never.

	// and now, a year later, there's a bug in this code! 
	// looks like i'll have to figure it out. 

	// if we have a unit vector (0, 0, 1), what are the X and Y
	// rotations to get to the target vector?

	// how much of a rotation about the x-axis would we make to
	// get a Y value of loc->y? 

	i->engine->camera->rx = -asin(loc->y);

	// now--how much would we rotate the resulting vector in order
	// to get the X value where we want it?

	// now consider that we're at (0, .707, 0) and that the target 
	// is (.707, .707, 0).  That's 90 degree turn away, but if we  
	// just took asin(.707) we'd get 45.  We have to 'normalize'   
	// the X value by dividing it from our own distance from the   
	// (0, 1, 0) vector. 

	x = loc->x / cos(i->engine->camera->rx);

	if(x > 1.0) x = 1.0;
	if(x < -1.0) x = -1.0;

	i->engine->camera->ry = asin(x);

	// there are two x-rot values which fulfill this situation: 
	// (x, y, z) and (x, y, -z) 

	if(loc->z < 0.0) i->engine->camera->ry = M_PI - i->engine->camera->ry;

	slUpdateCamera(i->engine->camera);

	return EC_OK;
}

/*!
	\brief Set the rotation of the camera.

	void cameraSetRotation(double x, double y).
*/

int brICameraSetRotation(brEval args[], brEval *target, brInstance *i) {
	double rx = BRDOUBLE(&args[0]);
	double ry = BRDOUBLE(&args[1]);

	i->engine->camera->rx = rx;
	i->engine->camera->ry = ry;

	slUpdateCamera(i->engine->camera);

	return EC_OK;
}

/*!
	\brief Set the size of the main camera's text.

	void cameraSetTextScale(double x, double y).
*/

int brICameraSetTextScale(brEval args[], brEval *target, brInstance *i) {
	i->engine->camera->textScale = BRDOUBLE(&args[0]);
	return EC_OK;
}

/*!
	\brief Set a string for display in the main camera.

	void cameraSetText(string message, int slotNumber, double x, double y, vector color).
*/

int brICameraSetText(brEval args[], brEval *target, brInstance *i) {
	slSetCameraText(i->engine->camera, BRINT(&args[1]), BRSTRING(&args[0]), BRDOUBLE(&args[2]), BRDOUBLE(&args[3]), &BRVECTOR(&args[4]));

	return EC_OK;
}

/*!
	\brief Sets the camera zoom.

	void cameraSetZoom(double zoom).
*/

int brICameraSetZoom(brEval args[], brEval *target, brInstance *i) {
	double z = BRDOUBLE(&args[0]);

	i->engine->camera->zoom = z;

	slUpdateCamera(i->engine->camera);

	return EC_OK;
}

/*!
	\brief Returns the zoom of the camera.
*/

int brICameraGetZoom(brEval args[], brEval *target, brInstance *i) {
	BRDOUBLE(target) = i->engine->camera->zoom;
	return EC_OK;
}

/*!
	\brief Returns the y-rotation of the camera.

	double cameraGetRY().
*/

int brICameraGetRY(brEval args[], brEval *target, brInstance *i) {
	BRDOUBLE(target) = i->engine->camera->ry;
	return EC_OK;
}

/*!
	\brief Returns the x-rotation of the camera.

	double cameraGetRX().
*/

int brICameraGetRX(brEval args[], brEval *target, brInstance *i) {
	BRDOUBLE(target) = i->engine->camera->rx;
	return EC_OK;
}

/*!
	\brief Returns the current offset of the camera relative to its target.

	vector cameraGetOffset().
*/

int brICameraGetOffset(brEval args[], brEval *target, brInstance *i) {
	slVectorCopy(&i->engine->camera->location, &BRVECTOR(target));
	return EC_OK;
}

/*!
	\brief Returns the current target of the camera.

	vector cameraGetTarget().
*/

int brICameraGetTarget(brEval args[], brEval *target, brInstance *i) {
	slVectorCopy(&i->engine->camera->target, &BRVECTOR(target));
	return EC_OK;
}

/*!
	\brief Sets the background color of the display.

	vector setBackgroundColor().
*/

int brISetBackgroundColor(brEval args[], brEval *target, brInstance *i) {
	slWorldSetBackgroundColor(i->engine->world, &BRVECTOR(&args[0]));
	return EC_OK;
}

int brISetBackgroundTextureColor(brEval args[], brEval *target, brInstance *i) {
	slWorldSetBackgroundTextureColor(i->engine->world, &BRVECTOR(&args[0]));
	return EC_OK;
}

/*!
	\brief Sets the background texture.

	void setBackgroundTexture(image number).
*/

int brISetBackgroundTexture(brEval args[], brEval *target, brInstance *i) {
	slWorldSetBackgroundTexture(i->engine->world, BRINT(&args[0]), 0);
	return EC_OK;
}

/*!
	\brief Sets the background image.

	void setBackgroundImage(image number).
*/

int brISetBackgroundImage(brEval args[], brEval *target, brInstance *i) {
	slWorldSetBackgroundTexture(i->engine->world, BRINT(&args[0]), 1);
	return EC_OK;
}

/*!
	\brief Gets the position of the main light.

	vector getLightPosition().
*/

int brIGetLightPosition(brEval args[], brEval *target, brInstance *i) {
	slVectorCopy(&i->engine->camera->lights[0].location, &BRVECTOR(target));
	return EC_OK;
}

/*!
	\brief Sets a the position of the main light.

	setLightPosition(vector position).
*/

int brISetLightPosition(brEval args[], brEval *target, brInstance *i) {
	slVectorCopy(&BRVECTOR(&args[0]), &i->engine->camera->lights[0].location);
	i->engine->camera->lights[0].changed = 1;
	return EC_OK;
}

/*!
	\brief Sets light exposure detection on/off.

	setDetectLightExposure(int state).
*/

int brISetDetectLightExposure(brEval args[], brEval *target, brInstance *i) {
	slWorldSetLightExposureDetection(i->engine->world, BRINT(&args[0]));
	return EC_OK;
}

/*!
	\brief Sets a the light exposure detection source.

	setLightExposureSource(vector color).
*/

int brISetLightExposureSource(brEval args[], brEval *target, brInstance *i) {
	slWorldSetLightExposureSource(i->engine->world, &BRVECTOR(&args[0]));
	return EC_OK;
}

/*!
	\brief Sets a the ambient color for the light.

	setLightAmbientColor(vector color).
*/

int brISetLightAmbientColor(brEval args[], brEval *target, brInstance *i) {
	slVectorCopy(&BRVECTOR(&args[0]), &i->engine->camera->lights[0].ambient);
	i->engine->camera->lights[0].changed = 1;
	return EC_OK;
}

/*!
	\brief Sets a the diffuse color for the light.

	setLightDiffuseColor(vector color).
*/

int brISetLightDiffuseColor(brEval args[], brEval *target, brInstance *i) {
	slVectorCopy(&BRVECTOR(&args[0]), &i->engine->camera->lights[0].diffuse);
	i->engine->camera->lights[0].changed = 1;
	return EC_OK;
}

/*!
	\brief Sets a stationary object to be a shadow-catcher.

	The plane normal specifies which plane of the object should catch the
	shadows.

	void setShadowCatcher(slWorldObject pointer object, vector planeNormal).
*/

int brISetShadowCatcher(brEval args[], brEval *target, brInstance *i) {
	slStationary *st = (slStationary*)BRPOINTER(&args[0]);
	slVector *norm = &BRVECTOR(&args[1]);

	slWorldObjectAddDrawMode(st, DM_STENCIL);

	slSetShadowCatcher(i->engine->camera, st, norm);

	return EC_OK;
}

/*!
	\brief Enables or disables point drawing for an object.
*/

int brISetDrawAsPoint(brEval args[], brEval *target, brInstance *i) {
	BRWORLDOBJECTPOINTER(&args[0])->_drawAsPoint = BRINT(&args[1]);
	return EC_OK;
}

/*!
	\brief Adds a rendered line between two objects.

	void addObjectLine(slWorldObject pointer source, slWorldObject pointer dest, vector color, int style).
*/

int brIAddObjectLine(brEval args[], brEval *target, brInstance *i) {
	slWorldObject *src = BRWORLDOBJECTPOINTER(&args[0]);
	slWorldObject *dst = BRWORLDOBJECTPOINTER(&args[1]);
	slVector *color = &BRVECTOR(&args[2]);
	char *patternString = BRSTRING(&args[3]);
	int pattern = 0, n = 0;

	n = strlen(patternString);

	if(n > 16) n = 16;

	while(n--) {
		pattern <<= 1;
		pattern |= (*(patternString++) == '-');
	}

	BRPOINTER(target) = slWorldAddObjectLine(i->engine->world, src, dst, pattern, color);

	return EC_OK;
}

/*!
	\brief Sets the stipple pattern of an object line.
*/

int brIObjectLineSetStipple(brEval args[], brEval *target, brInstance *i) {
	slObjectLine *line = (slObjectLine*)BRPOINTER(&args[0]);
	char *patternString = BRSTRING(&args[1]);

	int n, pattern = 0;

	n = strlen(patternString);
    
	if(n > 16) n = 16;

	while(n--) {
		pattern <<= 1;
		pattern |= (*(patternString++) == '-');
	}

	line->setStipple(pattern);

	return EC_OK;
}

/*!
	\brief Removes a specific object line between two objects.

	void removeObjectLine(slWorldObject pointer source, slWorldObject pointer dest).
*/

int brIRemoveObjectLine(brEval args[], brEval *target, brInstance *i) {
	slObjectConnection *line = (slObjectConnection*)BRPOINTER(&args[0]);

	slWorldRemoveConnection(i->engine->world, line);

	return EC_OK;
}

/*!
	\brief Sets whether bounds-only collision detection should be used.

	void setBoundsOnlyCollisionDetection(int state).
*/

int brISetBoundsOnlyCollisionDetection(brEval args[], brEval *target, brInstance *i) {
	int value = BRINT(&args[0]);

	slWorldSetBoundsOnlyCollisionDetection(i->engine->world, value);

	return EC_OK;
}

/*!
	\brief Loads data from a Tiger/Line file.
*/

int brILoadTigerData(brEval args[], brEval *target, brInstance *i) {
	slWorldLoadTigerFile(i->engine->world, BRSTRING(&args[0]));
	return EC_OK;

}

int brIRaytrace(brEval args[], brEval *target, brInstance *i) {
	slWorldObject *o    = BRWORLDOBJECTPOINTER(&args[0]);
 	slVector *location  = &BRVECTOR(&args[1]);
        slVector *direction = &BRVECTOR(&args[2]); 

	if(!o) {
		slMessage(DEBUG_ALL, "null pointer passed to raytrace\n");
		return EC_ERROR;
	}

	int error = slWorldObjectRaytrace(o, location, direction, &BRVECTOR(target));

        if (error == -2)
           return EC_ERROR;
        
	return EC_OK;
}

/*@}*/

/*!
	\brief Initializes internal breve functions related to the simulated world.
*/

void breveInitWorldFunctions(brNamespace *n) {
	brNewBreveCall(n, "worldObjectGetLightExposure", brIWorldObjectGetLightExposure, AT_INT, AT_POINTER, 0);

	brNewBreveCall(n, "setStepFast", brISetStepFast, AT_POINTER, AT_INT, 0);
	brNewBreveCall(n, "setStepFastIterations", brISetStepFastIterations, AT_POINTER, AT_INT, 0);
	brNewBreveCall(n, "setDrawEveryFrame", brISetDrawEveryFrame, AT_POINTER, AT_INT, 0);
	brNewBreveCall(n, "randomSeed", brIRandomSeed, AT_NULL, AT_INT, 0);
	brNewBreveCall(n, "randomSeedFromDevRandom", brIRandomSeedFromDevRandom, AT_INT, 0);
	brNewBreveCall(n, "getTime", brIGetTime, AT_DOUBLE, 0);
	brNewBreveCall(n, "worldStep", brIWorldStep, AT_DOUBLE, AT_DOUBLE, AT_DOUBLE, 0);
	brNewBreveCall(n, "updateNeighbors", brIUpdateNeighbors, AT_NULL, 0);
	brNewBreveCall(n, "getNeighbors", brIGetNeighbors, AT_LIST, AT_POINTER, 0);
	brNewBreveCall(n, "setNeighborhoodSize", brISetNeighborhoodSize, AT_NULL, AT_POINTER, AT_DOUBLE, 0);

	brNewBreveCall(n, "removeObject", brIRemoveObject, AT_NULL, AT_POINTER, 0);

	brNewBreveCall(n, "setBoundingBox", brISetBoundingBox, AT_NULL, AT_POINTER, AT_INT, 0);
	brNewBreveCall(n, "setDrawAxis", brISetDrawAxis, AT_NULL, AT_POINTER, AT_INT, 0);
	brNewBreveCall(n, "setNeighborLines", brISetNeighborLines, AT_NULL, AT_POINTER, AT_INT, 0);
	brNewBreveCall(n, "setVisible", brISetVisible, AT_NULL, AT_POINTER, AT_INT, 0);
	brNewBreveCall(n, "setTexture", brISetTexture, AT_NULL, AT_POINTER, AT_INT, 0);
	brNewBreveCall(n, "setTextureScale", brISetTextureScale, AT_NULL, AT_POINTER, AT_INT, 0);
	brNewBreveCall(n, "setBitmap", brISetBitmap, AT_NULL, AT_POINTER, AT_INT, 0);
	brNewBreveCall(n, "setBitmapRotation", brISetBitmapRotation, AT_NULL, AT_POINTER, AT_DOUBLE, 0);
	brNewBreveCall(n, "setBitmapRotationTowardsVector", brISetBitmapRotationTowardsVector, AT_NULL, AT_POINTER, AT_VECTOR, 0);
	brNewBreveCall(n, "setAlpha", brISetAlpha, AT_NULL, AT_POINTER, AT_DOUBLE, 0);
	brNewBreveCall(n, "setLightmap", brISetLightmap, AT_NULL, AT_POINTER, AT_INT, 0);
	brNewBreveCall(n, "setColor", brISetColor, AT_NULL, AT_POINTER, AT_VECTOR, 0);

	brNewBreveCall(n, "loadTexture", brILoadTexture, AT_INT, AT_STRING, AT_INT, 0);

	brNewBreveCall(n, "getMainCameraPointer", brIGetMainCameraPointer, AT_POINTER, 0);
	brNewBreveCall(n, "cameraSetOffset", brICameraSetOffset, AT_NULL, AT_VECTOR, 0);
	brNewBreveCall(n, "cameraSetTarget", brICameraSetTarget, AT_NULL, AT_VECTOR, 0);

	brNewBreveCall(n, "cameraSetRotation", brICameraSetRotation, AT_NULL, AT_DOUBLE, AT_DOUBLE, 0);
	brNewBreveCall(n, "cameraSetZoom", brICameraSetZoom, AT_NULL, AT_DOUBLE, 0);
	brNewBreveCall(n, "cameraGetZoom", brICameraGetZoom, AT_DOUBLE, 0);
	brNewBreveCall(n, "cameraGetRX", brICameraGetRX, AT_DOUBLE, 0);
	brNewBreveCall(n, "cameraGetRY", brICameraGetRY, AT_DOUBLE, 0);
	brNewBreveCall(n, "cameraGetTarget", brICameraGetTarget, AT_VECTOR, 0);
	brNewBreveCall(n, "cameraGetOffset", brICameraGetOffset, AT_VECTOR, 0);
	brNewBreveCall(n, "cameraSetText", brICameraSetText, AT_NULL, AT_STRING, AT_INT, AT_DOUBLE, AT_DOUBLE, AT_VECTOR, 0);
	brNewBreveCall(n, "cameraSetTextScale", brICameraSetTextScale, AT_NULL, AT_DOUBLE, 0);

	brNewBreveCall(n, "getLightPosition", brIGetLightPosition, AT_VECTOR, 0);
	brNewBreveCall(n, "setLightPosition", brISetLightPosition, AT_NULL, AT_VECTOR, 0);
	brNewBreveCall(n, "setDetectLightExposure", brISetDetectLightExposure, AT_NULL, AT_INT, 0);
	brNewBreveCall(n, "setLightExposureSource", brISetLightExposureSource, AT_NULL, AT_VECTOR, 0);

	brNewBreveCall(n, "setBackgroundColor", brISetBackgroundColor, AT_NULL, AT_VECTOR, 0);
	brNewBreveCall(n, "setBackgroundTexture", brISetBackgroundTexture, AT_NULL, AT_INT, 0);
	brNewBreveCall(n, "setBackgroundImage", brISetBackgroundImage, AT_NULL, AT_INT, 0);
	brNewBreveCall(n, "setBackgroundTextureColor", brISetBackgroundTextureColor, AT_NULL, AT_VECTOR, 0);

	brNewBreveCall(n, "setShadowCatcher", brISetShadowCatcher, AT_NULL, AT_POINTER, AT_VECTOR, 0);
	brNewBreveCall(n, "setDrawAsPoint", brISetDrawAsPoint, AT_NULL, AT_POINTER, AT_INT, 0);

	brNewBreveCall(n, "setLightAmbientColor", brISetLightAmbientColor, AT_NULL, AT_VECTOR, 0);
	brNewBreveCall(n, "setLightDiffuseColor", brISetLightDiffuseColor, AT_NULL, AT_VECTOR, 0);

	brNewBreveCall(n, "addObjectLine", brIAddObjectLine, AT_POINTER, AT_POINTER, AT_POINTER, AT_VECTOR, AT_STRING, 0);
	brNewBreveCall(n, "removeObjectLine", brIRemoveObjectLine, AT_NULL, AT_POINTER, 0);
	brNewBreveCall(n, "objectLineSetStipple", brIObjectLineSetStipple, AT_NULL, AT_POINTER, AT_STRING, 0);

	brNewBreveCall(n, "setBoundsOnlyCollisionDetection", brISetBoundsOnlyCollisionDetection, AT_NULL, AT_INT, 0);

	brNewBreveCall(n, "loadTigerData", brILoadTigerData, AT_NULL, AT_STRING, 0);

	brNewBreveCall(n, "setCollisionProperties", brISetCollisionProperties, AT_NULL, AT_POINTER, AT_DOUBLE, AT_DOUBLE, AT_DOUBLE, 0);

	brNewBreveCall(n, "raytrace", brIRaytrace, AT_VECTOR, AT_POINTER, AT_VECTOR, AT_VECTOR, 0);
}
