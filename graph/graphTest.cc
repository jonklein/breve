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

#include "util.h"
#include "graph.h"

void slInitGlut(int argc, char **argv, char *title);
void brGlutLoop();
void slDemoDisplay();

slGraph *graph;
int redLine, blueLine;

int gX = 0;
int last = 50;

int main(int argc, char **argv) {
	slVector v;

	slInitGlut(argc, argv, "graphTest");

	slVectorSet(&v, 0.9, 0.9, 0.9);

	graph = slGraphNew(&v, 0, 0, 100, 100);
	graph->width = 400;
	graph->height = 400;

	slVectorSet(&v, 1.0, 0.0, 0.0);
	redLine = slGraphAddLine(graph, &v);
	slVectorSet(&v, 0.0, 0.0, 1.0);
	blueLine = slGraphAddLine(graph, &v);

	glutMainLoop();

	return 0;
}

void brGlutLoop() {
	gX++;

	last += (random() % 31) - 15;

	slGraphAddLineValue(graph, redLine, gX, last);
	slGraphAddLineValue(graph, blueLine, gX, 6 * sin(gX / 3.0) + 5);

	usleep(30000);

	glutPostRedisplay();
}

void slDemoReshape(int x, int y) {
	graph->height = y;
	graph->width = x;
	glViewport(0, 0, x, y);
}

void slInitGlut(int argc, char **argv, char *title) {
	glutInitWindowSize(400, 400);
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DOUBLE|GLUT_RGBA|GLUT_DEPTH|GLUT_STENCIL);
	glutCreateWindow(title);
	glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
 
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

	glutDisplayFunc(slDemoDisplay);
	glutReshapeFunc(slDemoReshape);

	glutIdleFunc(brGlutLoop);

	glViewport(0, 0, 400, 400);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}


void slDemoDisplay() {
	slDrawGraph(graph);
	glutSwapBuffers();
}
