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

int brITerrainNew(brEval args[], brEval *target, brInstance *i) {
	slTerrain *t;

	// t = slTerrainNew(5, BRDOUBLE(&args[0]), i);
	t = slTerrainNew(8, BRDOUBLE(&args[0]), i);

	slWorldAddObject(i->engine->world, t, WO_TERRAIN);

	BRPOINTER(target) = t;

	return EC_OK;
}

int brIGenerateFractalTerrain(brEval args[], brEval *target, brInstance *i) {
	slTerrain *t = BRPOINTER(&args[0]);
	double h = BRDOUBLE(&args[1]);
	double height = BRDOUBLE(&args[2]);

	slGenerateFractalTerrain(t, h, height);

	return EC_OK;
}

int brISetTerrainScale(brEval args[], brEval *target, brInstance *i) {
	slTerrain *t = BRPOINTER(&args[0]);
	double x = BRDOUBLE(&args[1]);

	if(x < 0.01) return EC_OK;

	slTerrainSetScale(t, x);

	return EC_OK;
}

int brISetTerrainHeight(brEval args[], brEval *target, brInstance *i) {
	slTerrain *t = BRPOINTER(&args[0]);
	int x = BRINT(&args[1]);
	int y = BRINT(&args[2]);
	double h = BRDOUBLE(&args[3]);

	if(!t) return EC_OK;

	slTerrainSetHeight(t, x, y, h);

	i->engine->camera->recompile = 1;

	return EC_OK;
}

int brIGetTerrainHeight(brEval args[], brEval *target, brInstance *i) {
	slTerrain *t = BRPOINTER(&args[0]);
	int x = BRINT(&args[1]);
	int y = BRINT(&args[2]);

	if(!t) return EC_OK;

	BRDOUBLE(target) = slTerrainGetHeight(t, x, y);

	return EC_OK;
}

int brISetTerrainPosition(brEval args[], brEval *target, brInstance *i) {
	slTerrain *t = BRPOINTER(&args[0]);

	if(!t) return EC_OK;

	slTerrainSetLocation(t, &BRVECTOR(&args[1]));

	return EC_OK;
}

int brISetPeakColor(brEval args[], brEval *target, brInstance *i) {
	slTerrain *t = BRPOINTER(&args[0]);

	if(!t) return EC_OK;

	slTerrainSetTopColor(t, &BRVECTOR(&args[1]));

	return EC_OK;
}

int brISetValleyColor(brEval args[], brEval *target, brInstance *i) {
	slTerrain *t = BRPOINTER(&args[0]);

	if(!t) return EC_OK;

	slTerrainSetBottomColor(t, &BRVECTOR(&args[1]));

	return EC_OK;
}

int brILoadGeoTIFF(brEval args[], brEval *target, brInstance *i) {
	slTerrain *t = BRPOINTER(&args[0]);
	char *file = brFindFile(i->engine, BRSTRING(&args[1]), NULL);
	
	if(!file) {
		slMessage(DEBUG_ALL, "Cannot locate file \"%s\"\n", file);
		return EC_OK;
	}

	slTerrainLoadGeoTIFF(t, file);
	slFree(file);
	
	return EC_OK;
}

/*@}*/

void breveInitTerrainFunctions(brNamespace *n) {
    brNewBreveCall(n, "newTerrain", brITerrainNew, AT_POINTER, AT_DOUBLE, 0);
    brNewBreveCall(n, "setPeakColor", brISetPeakColor, AT_NULL, AT_POINTER, AT_VECTOR, 0);
    brNewBreveCall(n, "setValleyColor", brISetValleyColor, AT_NULL, AT_POINTER, AT_VECTOR, 0);
    brNewBreveCall(n, "generateFractalTerrain", brIGenerateFractalTerrain, AT_NULL, AT_POINTER, AT_DOUBLE, AT_DOUBLE, 0);
    brNewBreveCall(n, "setTerrainPosition", brISetTerrainPosition, AT_NULL, AT_POINTER, AT_VECTOR, 0);
    brNewBreveCall(n, "setTerrainScale", brISetTerrainScale, AT_NULL, AT_POINTER, AT_DOUBLE, 0);
    brNewBreveCall(n, "setTerrainHeight", brISetTerrainHeight, AT_NULL, AT_POINTER, AT_INT, AT_INT, AT_DOUBLE, 0);
    brNewBreveCall(n, "getTerrainHeight", brIGetTerrainHeight, AT_DOUBLE, AT_POINTER, AT_INT, AT_INT, 0);
    brNewBreveCall(n, "loadGeoTIFF", brILoadGeoTIFF, AT_INT, AT_POINTER, AT_STRING, 0);
}
