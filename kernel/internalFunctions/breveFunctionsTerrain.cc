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

int brINewTerrain(brEval args[], brEval *target, brInstance *i) {
	slTerrain *t;
	slWorldObject *wo;

	t = slNewTerrain(5, BRDOUBLE(&args[0]));

	wo = slAddObject(i->engine->world, t, WO_TERRAIN, NULL);

	BRPOINTER(target) = wo;
	wo->userData = i;

	return EC_OK;
}

int brIGenerateFractalTerrain(brEval args[], brEval *target, brInstance *i) {
	slTerrain *t;
	slWorldObject *wo = BRPOINTER(&args[0]);
	double h = BRDOUBLE(&args[1]);
	double height = BRDOUBLE(&args[2]);

	t = wo->data;

	slGenerateFractalTerrain(t, h, height);

	return EC_OK;
}

int brISetTerrainDrawMode(brEval args[], brEval *target, brInstance *i) {
	slTerrain *t;
	slWorldObject *wo = BRPOINTER(&args[0]);

	t = wo->data;

	if(!t) return EC_OK;

	t->drawMode = BRINT(&args[1]);

	return EC_OK;
}

int brISetTerrainScale(brEval args[], brEval *target, brInstance *i) {
	slTerrain *t;
	slWorldObject *wo = BRPOINTER(&args[0]);
	double x = BRDOUBLE(&args[1]);

	t = wo->data;

	if(x < 0.01) return EC_OK;

	slTerrainSetScale(t, x);

	return EC_OK;
}

int brISetTerrainHeight(brEval args[], brEval *target, brInstance *i) {
	slTerrain *t;
	slWorldObject *wo = BRPOINTER(&args[0]);
	int x = BRINT(&args[1]);
	int y = BRINT(&args[2]);
	double h = BRDOUBLE(&args[3]);

	t = wo->data;

	if(x > 0 && y > 0 && x < t->side && y < t->side) {
		t->matrix[x][y] = h;
		t->initialized = 0;
	}

	i->engine->camera->recompile = 1;

	return EC_OK;
}

int brIGetTerrainHeight(brEval args[], brEval *target, brInstance *i) {
	slTerrain *t;
	slWorldObject *wo = BRPOINTER(&args[0]);
	int x = BRINT(&args[1]);
	int y = BRINT(&args[2]);

	t = wo->data;

	if(x > 0 && y > 0 && x < t->side && y < t->side) BRDOUBLE(target) = t->matrix[x][y];
	else BRDOUBLE(target) = 0.0;

	return EC_OK;
}

int brISetTerrainPosition(brEval args[], brEval *target, brInstance *i) {
	slTerrain *t;
	slWorldObject *wo = BRPOINTER(&args[0]);

	t = wo->data;

	if(!t) return EC_OK;

	slTerrainSetLocation(t, &BRVECTOR(&args[1]));

	return EC_OK;
}

int brISetPeakColor(brEval args[], brEval *target, brInstance *i) {
	slTerrain *t;
	slWorldObject *wo = BRPOINTER(&args[0]);

	t = wo->data;

	slVectorCopy(&BRVECTOR(&args[1]), &t->topColor);

	return EC_OK;
}

int brISetValleyColor(brEval args[], brEval *target, brInstance *i) {
	slTerrain *t;
	slWorldObject *wo = BRPOINTER(&args[0]);

	t = wo->data;

	slVectorCopy(&BRVECTOR(&args[1]), &t->bottomColor);

	return EC_OK;
}

/*@}*/

void breveInitTerrainFunctions(brNamespace *n) {
    brNewBreveCall(n, "newTerrain", brINewTerrain, AT_POINTER, AT_DOUBLE, 0);
    brNewBreveCall(n, "setPeakColor", brISetPeakColor, AT_NULL, AT_POINTER, AT_VECTOR, 0);
    brNewBreveCall(n, "setValleyColor", brISetValleyColor, AT_NULL, AT_POINTER, AT_VECTOR, 0);
    brNewBreveCall(n, "generateFractalTerrain", brIGenerateFractalTerrain, AT_NULL, AT_POINTER, AT_DOUBLE, AT_DOUBLE, 0);
    brNewBreveCall(n, "setTerrainDrawMode", brISetTerrainDrawMode, AT_NULL, AT_POINTER, AT_INT, 0);
    brNewBreveCall(n, "setTerrainPosition", brISetTerrainPosition, AT_NULL, AT_POINTER, AT_VECTOR, 0);
    brNewBreveCall(n, "setTerrainScale", brISetTerrainScale, AT_NULL, AT_POINTER, AT_DOUBLE, 0);
    brNewBreveCall(n, "setTerrainHeight", brISetTerrainHeight, AT_NULL, AT_POINTER, AT_INT, AT_INT, AT_DOUBLE, 0);
    brNewBreveCall(n, "getTerrainHeight", brIGetTerrainHeight, AT_DOUBLE, AT_POINTER, AT_INT, AT_INT, 0);
}
