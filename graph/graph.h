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

#ifndef _GRAPH_H
#define _GRAPH_H

#include "util.h"
#include "glIncludes.h"

#include "graphTypedefs.h"

struct slGraph {
	slVector color;
	slGraphLine **lines;

	void *userData;

	int nLines;
	int maxLines;

	char *title;
	char *xAxis;
	char *yAxis;

	/* graph bounds */

	float minX;
	float minY;
	float maxX;
	float maxY;

	/* window bounds */

	float height;
	float width;

	int scrolls;
};

struct slGraphLine {
	slVector color;

	slGraph *graph;
	
	int nValues;
	int maxValues;

	int start;
	int end;

	float *xValues;
	float *yValues;

	float lastX;
	float lastY;
};

slGraph *slGraphNew(slVector *color, float minX, float minY, float maxX, float maxY);

void slGraphFree(slGraph *graph);

int slGraphAddLine(slGraph *graph, slVector *color);
slGraphLine *slGraphNewLine(slGraph *graph, slVector *color);

void slGraphSetXAxisName(slGraph *graph, char *title);
void slGraphSetYAxisName(slGraph *graph, char *title);
void slGraphSetTitle(slGraph *graph, char *title);

void slGraphAddLineValue(slGraph *graph, int ln, float x, float y);

void slGraphLineFree(slGraphLine *graphLine);

#include "draw.h"

#endif
