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

/*! \addtogroup InternalFunctions */
/*@{*/

int brINewShape(brEval args[], brEval *target, brInstance *i) {
	BRPOINTER(target) = slNewShape();

	if(!BRPOINTER(target)) {
		slMessage(DEBUG_ALL, "newShape() failed\n");
		return EC_ERROR;
	}

	return EC_OK;
}

int brIAddShapeFace(brEval args[], brEval *target, brInstance *i) {
	int n = 0;
	slShape *s = BRPOINTER(&args[0]);
	brEvalListHead *list = BRLIST(&args[1]);
	brEvalList *h;
	slVector **face;

	if(!s) return EC_OK;

	if(list->count < 3) {
		slMessage(DEBUG_ALL, "addShapeFace() requires a list of at least 3-vectors");
		return EC_ERROR;
	}

	face = slMalloc(sizeof(slVector*) * list->count);

	h = list->start;

	while(h) {
		if(h->eval.type != AT_VECTOR) {
			slFree(face);
			slMessage(DEBUG_ALL, "addShapeFace() list may contain only vectors");
			return EC_ERROR;
		}

		face[n] = &BRVECTOR(&h->eval);
		
		n++;

		h = h->next;
	}

	slAddFace(s, face, list->count);

	slFree(face);

	return EC_OK;
}

int brIFinishShape(brEval args[], brEval *target, brInstance *i) {
	slShape *s = BRPOINTER(&args[0]);
	double density = BRDOUBLE(&args[1]);

	s = slInitNeighbors(s, density);

	if(!s) {
		slMessage(DEBUG_ALL, "finishShape() failed: error completing shape initialization\n");
		return EC_ERROR;
	}

	return EC_OK;
}

int brIShapeSetDensity(brEval args[], brEval *target, brInstance *i) {
	slShape *s = BRPOINTER(&args[0]);
	double density = BRDOUBLE(&args[1]);

	slShapeSetDensity(s, density);

	return EC_OK;
}

int brIShapeSetMass(brEval args[], brEval *target, brInstance *i) {
	slShape *s = BRPOINTER(&args[0]);
	double mass = BRDOUBLE(&args[1]);

	slShapeSetMass(s, mass);

	return EC_OK;
}

int brINewSphere(brEval args[], brEval *target, brInstance *i) {
	BRPOINTER(target) = slNewSphere(BRDOUBLE(&args[0]), BRDOUBLE(&args[1]));

	if(!BRINSTANCE(target)) {
		slMessage(DEBUG_ALL, "newSphere() failed\n");
		return EC_ERROR;
	}

	return EC_OK;
}

int brINewCube(brEval args[], brEval *target, brInstance *i) {
	BRPOINTER(target) = slNewCube(&BRVECTOR(&args[0]), BRDOUBLE(&args[1]));

	if(!BRPOINTER(target)) {
		slMessage(DEBUG_ALL, "newCube() failed\n");
		return EC_ERROR;
	}

	return EC_OK;
}

int brINewNGonDisc(brEval args[], brEval *target, brInstance *i) {
	BRPOINTER(target) = slNewNGonDisc(BRINT(&args[0]), BRDOUBLE(&args[1]), BRDOUBLE(&args[2]), BRDOUBLE(&args[3]));

	if(!BRPOINTER(target)) {
		slMessage(DEBUG_ALL, "newNGonDisc() failed\n");
		return EC_ERROR;
	}

	return EC_OK;
}

int brINewNGonCone(brEval args[], brEval *target, brInstance *i) {
	BRPOINTER(target) = slNewNGonCone(BRINT(&args[0]), BRDOUBLE(&args[1]), BRDOUBLE(&args[2]), BRDOUBLE(&args[3]));

	if(!BRPOINTER(target)) {
		slMessage(DEBUG_ALL, "newNGonCone() failed\n");
		return EC_ERROR;
	}

	return EC_OK;
}

int brIFreeShape(brEval args[], brEval *target, brInstance *i) {
	slFreeShape(BRPOINTER(&args[0]));

	return EC_OK;
}

int brIDataForShape(brEval args[], brEval *target, brInstance *i) {
	void *serialShape;
	int length;

	serialShape = slSerializeShape(BRPOINTER(&args[0]), &length);

	BRDATA(target) = brDataNew(serialShape, length);

	slFree(serialShape);

	return EC_OK;
}

int brIShapeForData(brEval args[], brEval *target, brInstance *i) {
	brData *d = BRDATA(&args[0]);

	if(!d) {
		slMessage(DEBUG_ALL, "NULL pointer passed to shapeForData\n");
		return EC_ERROR;
	}

	BRPOINTER(target) = slDeserializeShape(d->data, d->length);

	return EC_OK;
}

int brIScaleShape(brEval args[], brEval *target, brInstance *i) {
	slShape *s = BRPOINTER(&args[0]);
	slVector *scale = &BRVECTOR(&args[1]);

	if(!s) {
		slMessage(DEBUG_ALL, "scaleShape() called with invalid shape\n");
		return EC_ERROR;
	}

	slScaleShape(s, scale);

	return EC_OK;
}

int brIGetMass(brEval args[], brEval *target, brInstance *i) {
	slShape *s = BRPOINTER(&args[0]);

	if(!s) {
		slMessage(DEBUG_ALL, "NULL pointer passed to getMass\n");
		return EC_ERROR;
	}

	BRDOUBLE(target) = s->mass;

	return EC_OK;
}

int brIGetDensity(brEval args[], brEval *target, brInstance *i) {
	slShape *s = BRPOINTER(&args[0]);

	if(!s) {
		slMessage(DEBUG_ALL, "NULL pointer passed to getDensity\n");
		return EC_ERROR;
	}

	BRDOUBLE(target) = s->density;

	return EC_OK;
}


int brIPointOnShape(brEval args[], brEval *target, brInstance *i) {
	slShape *shape = BRPOINTER(&args[0]);
	slVector *location = &BRVECTOR(&args[1]);
	int result;

	result = slPointOnShape(shape, location, &BRVECTOR(target));

	return EC_OK;
}
/*@}*/

/*!
	\brief Initializes internal breve functions related to shapes.
*/

void breveInitShapeFunctions(brNamespace *n) {
	brNewBreveCall(n, "newShape", brINewShape, AT_POINTER, 0);
	brNewBreveCall(n, "addShapeFace", brIAddShapeFace, AT_NULL, AT_POINTER, AT_LIST, 0);
	brNewBreveCall(n, "finishShape", brIFinishShape, AT_NULL, AT_POINTER, AT_DOUBLE, 0);
	brNewBreveCall(n, "shapeSetDensity", brIShapeSetDensity, AT_NULL, AT_POINTER, AT_DOUBLE, 0);
	brNewBreveCall(n, "shapeSetMass", brIShapeSetMass, AT_NULL, AT_POINTER, AT_DOUBLE, 0);
	brNewBreveCall(n, "scaleShape", brIScaleShape, AT_NULL, AT_POINTER, AT_VECTOR, 0);
	brNewBreveCall(n, "newSphere", brINewSphere, AT_POINTER, AT_DOUBLE, AT_DOUBLE, 0);
	brNewBreveCall(n, "newCube", brINewCube, AT_POINTER, AT_VECTOR, AT_DOUBLE, 0);
	brNewBreveCall(n, "newNGonDisc", brINewNGonDisc, AT_POINTER, AT_INT, AT_DOUBLE, AT_DOUBLE, AT_DOUBLE, 0);
	brNewBreveCall(n, "newNGonCone", brINewNGonCone, AT_POINTER, AT_INT, AT_DOUBLE, AT_DOUBLE, AT_DOUBLE, 0);
	brNewBreveCall(n, "freeShape", brIFreeShape, AT_NULL, AT_POINTER, 0);
	brNewBreveCall(n, "pointOnShape", brIPointOnShape, AT_VECTOR, AT_POINTER, AT_VECTOR, 0);
	brNewBreveCall(n, "dataForShape", brIDataForShape, AT_DATA, AT_POINTER, 0);
	brNewBreveCall(n, "shapeForData", brIShapeForData, AT_POINTER, AT_DATA, 0);
	brNewBreveCall(n, "getMass", brIGetMass, AT_DOUBLE, AT_POINTER, 0);
	brNewBreveCall(n, "getDensity", brIGetDensity, AT_DOUBLE, AT_POINTER, 0);
}
