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

/*! \addtogroup InternalFunctions */
/*@{*/

#include "kernel.h"
#include "patch.h"
#include "world.h"

#define BRPATCHGRIDPOINTER(p)	((slPatchGrid*)BRPOINTER(p))
#define BRPATCHPOINTER(p)	((slPatch*)BRPOINTER(p))

/*!
	\brief Creates a new patch grid.

	void newPatchGrid(vector, vector, int, int, int).

	The first vector is the patch grid's center.  The second is the 
	size of an individual patch.  The three integers specify the x, y
	and z sizes of the grid.
*/

int brIPatchGridNew(brEval args[], brEval *target, brInstance *i) {
	target->set( i->engine->world->addPatchGrid( &BRVECTOR(&args[0]), &BRVECTOR(&args[1]), BRINT(&args[2]), BRINT(&args[3]), BRINT(&args[4]) ) );

	return EC_OK;
}

/*!
	\brief Frees a patch grid.

	void freePatchGrid(slPatchGrid pointer).
*/

int brIPatchGridFree(brEval args[], brEval *target, brInstance *i) {
	i->engine->world->removePatchGrid( BRPATCHGRIDPOINTER(&args[0]) );

	return EC_OK;
}

/*!
	\brief Sets the patch grid to be responsible for the collision detection
	pruning stage.
*/

int brIPatchGridSetClipGrid(brEval args[], brEval *target, brInstance *i) {
	i->engine->world->_clipGrid = BRPATCHGRIDPOINTER(&args[0]);
	return EC_OK;
}

/*!
	\brief Enables/disable smooth drawing for the patch grid.

	void patchGridSetSmoothDrawing(slPatchGrid pointer).
*/

int brIPatchGridSetSmoothDrawing(brEval args[], brEval *target, brInstance *i) {
	BRPATCHGRIDPOINTER(&args[0])->setSmoothDrawing(BRINT(&args[1]));
	return EC_OK;
}

/*!
	\brief Enables/disables textured patchgrid drawing.
*/

int brIPatchGridSetDrawWithTexture(brEval args[], brEval *target, brInstance *i) {
	BRPATCHGRIDPOINTER(&args[0])->setDrawWithTexture(BRINT(&args[1]));
	return EC_OK;
}

/*!
	\brief Returns the patch at the given (x, y, z) indices.

	slPatch pointer getPatchAtIndex(slPatchGrid pointer).
*/

int brIPatchAtIndex(brEval args[], brEval *target, brInstance *i) {
	slPatchGrid *grid = BRPATCHGRIDPOINTER(&args[0]);

	if (!grid) {
		brEvalError(i->engine, EE_SIMULATION, "Patch method called with uninitialized patch grid.");
		return EC_OK;
	}

	target->set( grid->getPatchAtIndex(BRINT(&args[1]), BRINT(&args[2]), BRINT(&args[3])) );

	return EC_OK;
}

/*!
	\brief Returns the object (if any) at the given location in 3D-space.

	instance objectAtLocation(slPatchGrid pointer, vector).
*/

int brIObjectAtLocation(brEval args[], brEval *target, brInstance *i) {
	slPatchGrid *grid = BRPATCHGRIDPOINTER(&args[0]);
	slPatch *patch;

	if (!grid) {
		brEvalError(i->engine, EE_SIMULATION, "Patch method called with uninitialized patch grid.");
		return EC_OK;
	}

	patch = grid->getPatchAtLocation(&BRVECTOR(&args[1]));

	if (patch) target->set( static_cast<brInstance*>( patch->getData() ) );
	else target->set( (void*)NULL );

	return EC_OK;
}

/*!
	\brief Returns the object at the given (x, y, z) indices.

	instance objectAtIndex(slPatchGrid pointer, int, int, int).
*/

int brIObjectAtIndex(brEval args[], brEval *target, brInstance *i) {
	slPatchGrid *grid = BRPATCHGRIDPOINTER(&args[0]);
	slPatch *patch;

	if (!grid) {
		brEvalError(i->engine, EE_SIMULATION, "Patch method called with uninitialized patch grid.");
		return EC_OK;
	}
	
	patch = grid->getPatchAtIndex(BRINT(&args[1]), BRINT(&args[2]), BRINT(&args[3]));

	if(patch) target->set( static_cast<brInstance*>(patch->getData()) );
	else target->set( (void*)NULL );

	return EC_OK;
}

/*!
	\brief Copies the contents of a matrix to a color channel in a patch grid.
*/

int brIPatchGridCopyColorFrom3DMatrix(brEval args[], brEval *target, brInstance *i) {
	slPatchGrid *g = BRPATCHGRIDPOINTER(&args[0]);

	g->copyColorFrom3DMatrix(static_cast<slBigMatrix3DGSL*>(BRPOINTER(&args[1])), BRINT(&args[2]), BRDOUBLE(&args[3]));

	return EC_OK;
}

/*!
	\brief Sets the object which corresponds to the patch at a given
	(x, y, z) indices.

	setObjectAtIndex(slPatchGrid pointer, instance, int, int, int).
*/

int brISetObjectAtIndex(brEval args[], brEval *target, brInstance *i) {
	slPatchGrid *grid = BRPATCHGRIDPOINTER(&args[0]);

	if (!grid) {
		brEvalError(i->engine, EE_SIMULATION, "Patch method called with uninitialized patch grid.");
		return EC_OK;
	}

	grid->setDataAtIndex(BRINT(&args[2]), BRINT(&args[3]), BRINT(&args[4]), BRINSTANCE(&args[1]));

	return EC_OK;
}

/*!
	\brief Sets the color of a patch.

	void setPatchColor(slPatch pointer, vector).
*/

int brISetPatchColor(brEval args[], brEval *target, brInstance *i) {
	BRPATCHPOINTER(&args[0])->setColor(&BRVECTOR(&args[1]));

	return EC_OK;
}

/*!
	\brief Gets the location of a patch.

	vector getPatchLocation(slPatch pointer).
*/

int brIGetPatchLocation(brEval args[], brEval *target, brInstance *i) {
	BRPATCHPOINTER(&args[0])->getLocation(&BRVECTOR(target));

	return EC_OK;
}

/*!
	\brief Sets the transparency of a patch on a scale from 0.0 to 1.0.

	void setPatchTransparency(slPatch pointer, double).
*/

int brISetPatchTransparency(brEval args[], brEval *target, brInstance *i) {
	BRPATCHPOINTER(&args[0])->setTransparency(BRDOUBLE(&args[1]));

	return EC_OK;
}
/*@}*/

void breveInitPatchFunctions(brNamespace *n) {
    brNewBreveCall(n, "patchGridNew", brIPatchGridNew, AT_POINTER, AT_VECTOR, AT_VECTOR, AT_INT, AT_INT, AT_INT, 0);
    brNewBreveCall(n, "patchGridFree", brIPatchGridFree, AT_NULL, AT_POINTER, 0);
    brNewBreveCall(n, "patchGridSetClipGrid", brIPatchGridSetClipGrid, AT_NULL, AT_POINTER, 0);
    brNewBreveCall(n, "patchGridSetSmoothDrawing", brIPatchGridSetSmoothDrawing, AT_NULL, AT_POINTER, AT_INT, 0);
    brNewBreveCall(n, "patchGridSetDrawWithTexture", brIPatchGridSetDrawWithTexture, AT_NULL, AT_POINTER, AT_INT, 0);
    brNewBreveCall(n, "setPatchObjectAtIndex", brISetObjectAtIndex, AT_NULL, AT_POINTER, AT_INSTANCE, AT_INT, AT_INT, AT_INT, 0);
    brNewBreveCall(n, "patchObjectAtIndex", brIObjectAtIndex, AT_INSTANCE, AT_POINTER, AT_INT, AT_INT, AT_INT, 0);
    brNewBreveCall(n, "objectAtLocation", brIObjectAtLocation, AT_INSTANCE, AT_POINTER, AT_VECTOR, 0);
    brNewBreveCall(n, "getPatchAtIndex", brIPatchAtIndex, AT_POINTER, AT_POINTER, AT_INT, AT_INT, AT_INT, 0);
    brNewBreveCall(n, "setPatchColor", brISetPatchColor, AT_NULL, AT_POINTER, AT_VECTOR, 0);
    brNewBreveCall(n, "setPatchTransparency", brISetPatchTransparency, AT_NULL, AT_POINTER, AT_DOUBLE, 0);
    brNewBreveCall(n, "patchGridCopyColorFrom3DMatrix", brIPatchGridCopyColorFrom3DMatrix, AT_NULL, AT_POINTER, AT_POINTER, AT_INT, AT_DOUBLE, 0);
    brNewBreveCall(n, "getPatchLocation", brIGetPatchLocation, AT_VECTOR, AT_POINTER, 0);
}
