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

#include "graph.h"

/*!
	\brief Creates a new graph.

	Creates a new graph with the given background color and dimensions.
*/

slGraph *slGraphNew(slVector *color, float minX, float minY, float maxX, float maxY) {
	slGraph *g;

	g = new slGraph;

	slVectorCopy(color, &g->color);

	g->minX = minX;
	g->maxX = maxX;
	g->minY = minY;
	g->maxY = maxY;

	g->title = slStrdup("Graph");
	g->xAxis = slStrdup("X-Axis");
	g->yAxis = slStrdup("Y-Axis");

	g->scrolls = 1;

	return g;
}

/*!
	\brief Sets the name of the graph's title.

	The name is slStrdup'd and stored in the graph.
*/

void slGraphSetTitle(slGraph *graph, char *title) {
	if(graph->title) slFree(graph->title);
	graph->title = slStrdup(title);
}

/*!
	\brief Sets the name of the graph's x-axis.

	The name is slStrdup'd and stored in the graph.
*/

void slGraphSetXAxisName(slGraph *graph, char *title) {
	if(graph->xAxis) slFree(graph->xAxis);
	graph->xAxis = slStrdup(title);
}

/*!
	\brief Sets the name of the graph's y-axis.

	The name is slStrdup'd and stored in the graph.
*/

void slGraphSetYAxisName(slGraph *graph, char *title) {
	if(graph->yAxis) slFree(graph->yAxis);
	graph->yAxis = slStrdup(title);
}

int slGraphAddLine(slGraph *graph, slVector *color) {
	graph->lines.push_back(slGraphNewLine(graph, color));

	return graph->lines.size();
}

/*!
	\brief Frees an slGraph.
*/

void slGraphFree(slGraph *graph) {
	unsigned int n;

	for(n=0;n<graph->lines.size();n++)
		slGraphLineFree(graph->lines[n]);

	if(graph->title) slFree(graph->title);
	if(graph->xAxis) slFree(graph->xAxis);
	if(graph->yAxis) slFree(graph->yAxis);

	delete graph;
}

/*!
	\brief Adds a line to the graph.

	Adds a line of the specified color to the graph.  An integer is 
	returned which identifies the newly created line.
*/

slGraphLine *slGraphNewLine(slGraph *graph, slVector *color) {
	slGraphLine *line;

	line = new slGraphLine;

	line->graph = graph;

	line->start = 0;
	line->end = 0;

	line->lastX = 0;
	line->lastY = 0;

	slVectorCopy(color, &line->color);

	return line;
}

/*!
	\brief Adds a point on a graph line.
*/

void slGraphAddLineValue(slGraph *graph, int ln, float x, float y) {
	slGraphLine *line = graph->lines[ln];

	if(line->graph->scrolls) {
		line->xValues[line->end] = x;
		line->yValues[line->end] = y;

		line->end++;

		if(line->end >= line->maxValues) line->end = 0;

		if(line->start == line->end) line->start++;
		if(line->start >= line->maxValues) line->start = 0;

		line->lastX = x;
		line->lastY = y;

		/* scroll forward */

		if(x > graph->maxX) {
			float diff = x - graph->maxX;

			graph->minX += diff;
			graph->maxX += diff;
		}

		return;
	}

	line->xValues.push_back(x);
	line->yValues.push_back(y);

	if(line->xValues.size() > line->graph->maxX) line->graph->maxX *= 2;
}

/*
	\brief Frees a graph line.
*/

void slGraphLineFree(slGraphLine *line) {
	line->xValues.clear();
	line->yValues.clear();
	delete[] line;
}
