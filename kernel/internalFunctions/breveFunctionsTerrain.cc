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

#define BRTERRAINPOINTER(p)  ((slTerrain*)BRPOINTER(p))

int brITerrainNew(brEval args[], brEval *target, brInstance *i) {
	slTerrain *t;

	t = new slTerrain(5, BRDOUBLE(&args[0]), i);

	slWorldAddObject(i->engine->world, t, WO_TERRAIN);

	BRPOINTER(target) = t;

	return EC_OK;
}

int brITerrainSetDesiredPolygonCount(brEval args[], brEval *target, brInstance *i) {
	slTerrain *t = BRTERRAINPOINTER(&args[0]);

	t->_desiredPolygons = BRINT(&args[1]);

	return EC_OK;
}

int brITerrainGenerateFractalTerrain(brEval args[], brEval *target, brInstance *i) {
	slTerrain *t = BRTERRAINPOINTER(&args[0]);
	double h = BRDOUBLE(&args[1]);
	double height = BRDOUBLE(&args[2]);

	t->generateFractalTerrain(h, height);

	return EC_OK;
}

int brITerrainSetDrawMode(brEval args[], brEval *target, brInstance *i) {
	slTerrain *t = BRTERRAINPOINTER(&args[0]);
	t->setDrawMode(BRINT(&args[1]));
	return EC_OK;
}


int brITerrainSetScale(brEval args[], brEval *target, brInstance *i) {
	slTerrain *t = BRTERRAINPOINTER(&args[0]);
	double x = BRDOUBLE(&args[1]);

	if (x < 0.01)
		return EC_OK;

	t->setScale(x);

	return EC_OK;
}

int brITerrainSetHeight(brEval args[], brEval *target, brInstance *i) {
	slTerrain *t = BRTERRAINPOINTER(&args[0]);
	int x = BRINT(&args[1]);
	int y = BRINT(&args[2]);
	double h = BRDOUBLE(&args[3]);

	if (!t)
		return EC_OK;

	slTerrainSetHeight(t, x, y, h);

	i->engine->camera->recompile = 1;

	return EC_OK;
}

int brITerrainGetHeight(brEval args[], brEval *target, brInstance *i) {
	slTerrain *t = BRTERRAINPOINTER(&args[0]);
	int x = BRINT(&args[1]);
	int y = BRINT(&args[2]);

	if (!t)
		return EC_OK;

	BRDOUBLE(target) = slTerrainGetHeight(t, x, y);

	return EC_OK;
}

int brITerrainGetHeightAtLocation(brEval args[], brEval *target, brInstance *i) {
	slTerrain *t = BRTERRAINPOINTER(&args[0]);
	double x = BRDOUBLE(&args[1]);
	double y = BRDOUBLE(&args[2]);

	if (!t) return EC_OK;

	BRDOUBLE(target) = t->getHeightAtLocation(x, y);

	return EC_OK;
}

int brITerrainGetSlope(brEval args[], brEval *target, brInstance *i) {
	slTerrain *t = BRTERRAINPOINTER(&args[0]);
	// double x1 = BRDOUBLE(&args[1]);
	// double y1 = BRDOUBLE(&args[2]);
	// double x2 = BRDOUBLE(&args[3]);
	// double y2 = BRDOUBLE(&args[4]);

	if (!t)
		return EC_OK;

	// slTerrainGetSlope(t, x1, y1, x2, y2, &BRVECTOR(target));

	return EC_OK;
}

int brITerrainSetPosition(brEval args[], brEval *target, brInstance *i) {
	slTerrain *t = BRTERRAINPOINTER(&args[0]);

	if (!t)
		return EC_OK;

	t->setLocation(&BRVECTOR(&args[1]));

	return EC_OK;
}

int brITerrainSetPeakColor(brEval args[], brEval *target, brInstance *i) {
	slTerrain *t = BRTERRAINPOINTER(&args[0]);

	if (!t)
		return EC_OK;

	slTerrainSetTopColor(t, &BRVECTOR(&args[1]));

	return EC_OK;
}

int brITerrainSetValleyColor(brEval args[], brEval *target, brInstance *i) {
	slTerrain *t = BRTERRAINPOINTER(&args[0]);

	if (!t)
		return EC_OK;

	slTerrainSetBottomColor(t, &BRVECTOR(&args[1]));

	return EC_OK;
}

int brITerrainLoadGeoTIFF(brEval args[], brEval *target, brInstance *i) {
	slTerrain *t = BRTERRAINPOINTER(&args[0]);
	char *file;

	if (!(file = brFindFile(i->engine, BRSTRING(&args[1]), NULL))) {
		slMessage(DEBUG_ALL, "Cannot locate file \"%s\"\n", file);
		return EC_OK;
	}

	t->loadGeoTIFF(file);

	slFree(file);
	
	return EC_OK;
}

int brITerrainFree(brEval args[], brEval *target, brInstance *i) {
	slTerrain *t = BRTERRAINPOINTER(&args[0]);

	delete t;

	return EC_OK;
}

/*@}*/

void breveInitTerrainFunctions(brNamespace *n) {
    brNewBreveCall(n, "terrainNew", brITerrainNew, AT_POINTER, AT_DOUBLE, 0);
    brNewBreveCall(n, "terrainSetDrawMode", brITerrainSetDrawMode, AT_NULL, AT_POINTER, AT_INT, 0);
    brNewBreveCall(n, "terrainSetPeakColor", brITerrainSetPeakColor, AT_NULL, AT_POINTER, AT_VECTOR, 0);
    brNewBreveCall(n, "terrainSetValleyColor", brITerrainSetValleyColor, AT_NULL, AT_POINTER, AT_VECTOR, 0);
    brNewBreveCall(n, "terrainGenerateFractalTerrain", brITerrainGenerateFractalTerrain, AT_NULL, AT_POINTER, AT_DOUBLE, AT_DOUBLE, 0);
    brNewBreveCall(n, "terrainSetPosition", brITerrainSetPosition, AT_NULL, AT_POINTER, AT_VECTOR, 0);
    brNewBreveCall(n, "terrainSetScale", brITerrainSetScale, AT_NULL, AT_POINTER, AT_DOUBLE, 0);
    brNewBreveCall(n, "terrainSetHeight", brITerrainSetHeight, AT_NULL, AT_POINTER, AT_INT, AT_INT, AT_DOUBLE, 0);
    brNewBreveCall(n, "terrainGetHeight", brITerrainGetHeight, AT_DOUBLE, AT_POINTER, AT_INT, AT_INT, 0);
    brNewBreveCall(n, "terrainGetHeightAtLocation", brITerrainGetHeightAtLocation, AT_DOUBLE, AT_POINTER, AT_DOUBLE, AT_DOUBLE, 0);
    brNewBreveCall(n, "terrainGetSlope", brITerrainGetSlope, AT_VECTOR, AT_POINTER, AT_DOUBLE, AT_DOUBLE, AT_DOUBLE, AT_DOUBLE, 0);
    brNewBreveCall(n, "terrainLoadGeoTIFF", brITerrainLoadGeoTIFF, AT_INT, AT_POINTER, AT_STRING, 0);
    brNewBreveCall(n, "terrainFree", brITerrainFree, AT_INT, AT_POINTER, AT_STRING, 0);
}
