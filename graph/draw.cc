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

#define TEXT_HEIGHT 119.05

void slDrawGraph(slGraph *graph) {
	unsigned int l;
	int v;
	char label[1024];
	double marginX, marginY;

	double textScaleX, textScaleY;

	marginX = 4000.0 / graph->width;
	marginY = 4000.0 / graph->height;

	textScaleX = 10.0 / graph->width;
	textScaleY = 10.0 / graph->height;

	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glEnable(GL_BLEND);
	glEnable(GL_LINE_SMOOTH);
	glHint(GL_LINE_SMOOTH_HINT, GL_NICEST);

	glClearColor(graph->color.x, graph->color.y, graph->color.z, 0.0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(0, (int)(graph->maxX - graph->minX), 0, (int)(graph->maxY - graph->minY), 0, 10);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	glColor4f(0.0, 0.0, 0.0, 0.5);

	glPushMatrix();
	glLoadIdentity();
	glTranslatef(marginX - (TEXT_HEIGHT * textScaleX), (graph->maxY - graph->minY) / 2.0, 0.0);
	glScaled(textScaleX, textScaleY, 1.0);
	glRotatef(90, 0.0, 0.0, 1.0);
	slStrokeCenteredString(0.0, 0.0, graph->yAxis);
	glPopMatrix();

	glPushMatrix();
	glLoadIdentity();
	glTranslatef((graph->maxX - graph->minX) / 2.0, marginY - (2 * TEXT_HEIGHT * textScaleY), 0.0);
	glScaled(textScaleX, textScaleY, 1.0);
	slStrokeCenteredString(0.0, 0.0, graph->xAxis);
	glPopMatrix();

	glPushMatrix();
	glLoadIdentity();
	glLineWidth(2.0);
	glTranslatef((graph->maxX - graph->minX) / 2.0, (graph->maxY - graph->minY) - (3 * TEXT_HEIGHT * textScaleY), 0.0);
	glScaled(2 * textScaleX, 2 * textScaleY, 1.0);
	slStrokeCenteredString(0.0, 0.0, graph->title);
	glLineWidth(1.0);
	glPopMatrix();

	glPushMatrix();

	glLoadIdentity();

	for(l=0;l<graph->lines.size();l++) {
		int drawing = 0;

		glColor4f(graph->lines[l]->color.x, graph->lines[l]->color.y, graph->lines[l]->color.z, 0.5);
	
		glBegin(GL_LINE_STRIP);

		if(graph->scrolls) {
			v = graph->lines[l]->start;

			while(v != graph->lines[l]->end) {
				float xVal, yVal;
				/* if we're over or under the graph bounds, then draw a final point */
				/* and stop the drawing. */

				xVal = graph->lines[l]->xValues[v];
				yVal = graph->lines[l]->yValues[v];

				if(yVal < graph->minY) {
					if(drawing) {
						slPlotPoint(xVal, graph->minY, marginX, marginY, graph);
						glColor4f(graph->lines[l]->color.x, graph->lines[l]->color.y, graph->lines[l]->color.z, 0.0);
						drawing = 0;
					}
				} else if(yVal > graph->maxY) {
					if(drawing) {
						slPlotPoint(xVal, graph->maxY, marginX, marginY, graph);
						glColor4f(graph->lines[l]->color.x, graph->lines[l]->color.y, graph->lines[l]->color.z, 0.0);
						drawing = 0;
					}
				} else {
					if(!drawing) {
						glColor4f(graph->lines[l]->color.x, graph->lines[l]->color.y, graph->lines[l]->color.z, 0.5);
						drawing = 1;
					}

					slPlotPoint(xVal, yVal, marginX, marginY, graph);
				}

				v++;

				if(v >= graph->lines[l]->maxValues) v = 0;
			}

		} else {
			for(v=0;v<graph->lines[l]->xValues.size();v++)
				slPlotPoint(graph->lines[l]->xValues[v], graph->lines[l]->yValues[v], marginX, marginY, graph);
		}


		glEnd();
	}

	glPopMatrix();

	/* the axes */

	glColor4f(0.0, 0.0, 0.0, 0.5);

	glBegin(GL_LINES);
	glVertex2f(.5 * marginX, marginY);
	glVertex2f((graph->maxX - graph->minX) - marginX, marginY);
	glVertex2f(marginX, .5 * marginY);
	glVertex2f(marginX, (graph->maxY - graph->minY) - marginY);
	glEnd();

	sprintf(label, "%.2f", graph->maxY);
	glPushMatrix();
	glLoadIdentity();
	glTranslatef(marginX, (graph->maxY - graph->minY) - (.5 * marginY), 0.0);
	glScaled(textScaleX / 2.0, textScaleY / 2.0, 1.0);
	slStrokeCenteredString(0.0, 0.0, label);
	glPopMatrix();

	sprintf(label, "%.2f", graph->maxX);
	glPushMatrix();
	glLoadIdentity();
	glTranslatef((graph->maxX - graph->minX) - (.5 * marginX), marginY - (TEXT_HEIGHT * textScaleY)/4.0, 0.0);
	glScaled(textScaleX / 2.0, textScaleY / 2.0, 1.0);
	slStrokeCenteredString(0.0, 0.0, label);
	glPopMatrix();
}

void slStrokeCenteredString(float x, float y, char *text) {
	float offset = glutStrokeLength(GLUT_STROKE_ROMAN, (const char*)text) / 2.0;
	slStrokeString(x - offset, y, text);
}

void slStrokeString(float x, float y, char *text) {
	char *p;

	glPushMatrix();
	glTranslatef(x, y, 0.0);

	for (p = text; *p; p++) glutStrokeCharacter(GLUT_STROKE_ROMAN, *p);

	glPopMatrix();
}

int slPlotPoint(float x, float y, float marginX, float marginY, slGraph *g) {
	if(x < g->minX || x > g->maxX) return -1;

	x -= (float)g->minX;

	if(y < g->minY) y = g->minY;
	if(y > g->maxY) y = g->maxY;

	y -= (float)g->minY;

	x = marginX + (x / (g->maxX - g->minX)) * ((g->maxX - g->minX) - 2 * marginX);
	y = marginY + (y / (g->maxY - g->minY)) * ((g->maxY - g->minY) - 2 * marginY);

	glVertex2f(x, y);

	return 0;
}
