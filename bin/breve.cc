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

/*!
	\brief The GLUT-based breve application.

	This is a wrapper around the breve engine for the GLUT-based 
	steve-frontend version of the breve simulation environment.
*/

#include <stdio.h>
#include <sys/param.h>

#include <pthread.h>

#include <readline/readline.h>
#include <readline/history.h>

#include "steve.h"
#include "breve.h"

/*!
	\brief Callback data for additional rendering windows.
*/

struct slGLUTWindow {
	int id;
	slGraph *graph;
};

typedef struct slGLUTWindow slGLUTWindow;

#ifdef MINGW
extern char *optarg;
extern int optind;
#endif

extern char *interfaceID;

extern int gPaused;

int gWaiting = 0;

int contextMenu, mainMenu;

int height = 400, width = 400;
int xpos = 0, ypos = 0;

brInstance *gSelected = NULL;

extern char *gErrorNames[];

char keyDown[256];

char *gSimFile;

int gMainWindow;

void *workerThread(void *data);
int soundCallback(void *data);
int pauseCallback();
void brInterrupt(brEngine *engine);
void *newWindowCallback(char *name, slGraph *graph);
void freeWindowCallback(void *w);
void renderWindowCallback(void *w);

void graphDisplay();

pthread_mutex_t gEngineMutex;
pthread_mutex_t gThreadMutex;

int gThreadShouldExit = 0;
int gThreadRunning = 0; 
int gFull = 0;
int gFormat = 0;

brFrontend *frontend;

slGLUTWindow *gWindows[1024];

int main(int argc, char **argv) {
	pthread_t thread;
	char wd[10240];
	int index;
	int n;
	char *text;

	interfaceID = "glut/2.0";

	srand(time(NULL));
	srandom(time(NULL));

	pthread_mutex_init(&gEngineMutex, NULL);
	pthread_mutex_init(&gThreadMutex, NULL);

	gSimFile = NULL;

#ifdef MEMORY_DEBUG
	slMessage(DEBUG_ALL, "### Running with breve slMalloc debugging enabled.\n\n");
#ifdef MACOSX
	slMessage(DEBUG_ALL, "### Set the environment variable MallocStackLogging\n");
	slMessage(DEBUG_ALL, "### to see stack traces of unfreed pointers on exit.\n\n");
#endif /* MACOSX */
#endif /* MEMORY_DEBUG */

	/* parse the command line args. */

	index = brParseArgs(argc, argv);

	/* offset argc and argv to account for the options parsed out */

	argc -= index;
	argv += index;

	if((argc) < 2) {
		brPrintUsage(argv[0]);
	}

	text = slUtilReadFile(argv[1]);

	if(!text) {
		printf("Error reading file \"%s\": %s\n", argv[1], strerror(errno));
		exit(1);
	}

	if(gFormat) {
		char *newtext = slFormatText(text);
		slUtilWriteFile(argv[1], newtext);
		free(newtext);
		exit(0);
	}

	frontend = breveFrontendInit(argc, argv);

	brEngineSetIOPath(frontend->engine, getcwd(wd, 10239));

	frontend->engine->camera = slNewCamera(width, height, GL_POLYGON);
	frontend->engine->camera->enabled = 1;

	frontend->engine->camera->updated = 1;

	frontend->engine->argc = argc - 1;
	frontend->engine->argv = argv + 1;

	frontend->engine->getLoadname = getLoadname;
	frontend->engine->getSavename = getSavename;
	frontend->engine->soundCallback = soundCallback;
	frontend->engine->dialogCallback = brCLIDialogCallback;
	frontend->engine->interfaceTypeCallback = interfaceVersionCallback;
	frontend->engine->pauseCallback = pauseCallback;
	frontend->engine->newWindowCallback = newWindowCallback;
	frontend->engine->freeWindowCallback = freeWindowCallback;
	frontend->engine->renderWindowCallback = renderWindowCallback;

	slInitGlut(argc, argv, argv[1]);

	if(gSimFile) {
		if(breveFrontendLoadSavedSimulation(frontend, text, argv[1], gSimFile) != EC_OK) {
			slFree(text);
			slFree(gSimFile);
			brQuit(frontend->engine);
		}

		slFree(gSimFile);
	} else {
		if(breveFrontendLoadSimulation(frontend, text, argv[1]) != EC_OK) {
			slFree(text);
			brQuit(frontend->engine);
		}

	}

	slFree(text);

	brSetUpdateMenuCallback(frontend->engine->controller, (void*)brGLMainMenuUpdate);

	for(n=0;n<256;n++) keyDown[n] = 0;

	pthread_mutex_lock(&gThreadMutex);
	pthread_create(&thread, NULL, workerThread, NULL);

	if(gFull) glutFullScreen();

	glutMainLoop();

	return 0;
}

void *workerThread(void *data) {
	int r;

	gThreadRunning = 1;

	pthread_mutex_lock(&gThreadMutex);

	while(1) {
		if(!gPaused && !frontend->engine->drawEveryFrame) {
			pthread_mutex_lock(&gEngineMutex);

   		 	r = brEngineIterate(frontend->engine);

			pthread_mutex_unlock(&gEngineMutex);

			if(!r) brQuit(frontend->engine);
		}

		if(gThreadShouldExit) {
			/* once we catch the exit symbol, change it back so the */
			/* caller knows that it's been received. */

			gThreadShouldExit = 0;
			return NULL;
		}
	}

	pthread_mutex_unlock(&gThreadMutex);
}

void brGlutLoop() {
	static int oldD = 1;

	gWaiting = 0;

	if(!gPaused) {
		if(frontend->engine->drawEveryFrame)
   	 		if(brEngineIterate(frontend->engine) != EC_OK) brQuit(frontend->engine);

		glutPostRedisplay();

		if(!frontend->engine->drawEveryFrame && oldD) pthread_mutex_unlock(&gThreadMutex);
		else if(frontend->engine->drawEveryFrame && !oldD) pthread_mutex_lock(&gThreadMutex);

		oldD = frontend->engine->drawEveryFrame;
	}
}

void brGLMenuStatus(int stat, int x, int y) {

}

void brGLMainMenuUpdate(brInstance *i) {
	int n, total;
	brMenuList *l;

	glutSetMenu(mainMenu);

	l = &i->menu;

	total = glutGet(GLUT_MENU_NUM_ITEMS);

	for(n=0;n<total;n++) {
		glutRemoveMenuItem(1);
	}

	for(n=0;n<l->count;n++) {
		char *message;

		if(l->list[n]->enabled) {
			message = slMalloc(strlen(l->list[n]->title) + 4);

			if(l->list[n]->checked) sprintf(message, "* %s", l->list[n]->title);
			else sprintf(message, "  %s", l->list[n]->title);

			glutAddMenuEntry(message, n);

			slFree(message);
		}

		else glutAddMenuEntry(" ", n);
	}
}

void brQuit(brEngine *e) {
	double diff;

	gThreadShouldExit = 1;

	/* wait for the thread to exit -- it will set gThreadShouldExit back to 0 */

	pthread_mutex_unlock(&gThreadMutex);
	while(gThreadRunning && gThreadShouldExit);

	brPauseTimer(e);

	diff = e->realTime.tv_sec + (e->realTime.tv_usec / 1000000.0);

	if(e->world->age != 0.0) {
		printf("%f simulated seconds elapsed\n", e->world->age);
		printf("%f real seconds elapsed\n", diff);
		printf("%f simulated/real\n", e->world->age/diff);
	}

	breveFrontendCleanup(frontend);

#ifdef MEMORY_DEBUG
	slMemoryReport();
	if(slUtilMemoryUnfreed()) slUtilPause();
#endif

	exit(0);
}

void brMainMenu(int n) {
	brMenuCallback(frontend->engine, frontend->engine->controller, n);
	glutPostRedisplay();
}

void brContextMenu(int n) {
	if(gSelected) brMenuCallback(frontend->engine, gSelected, n);
	glutPostRedisplay();
}

void brClick(int n) {
	int total;
	brMenuList *l;

	if(n < frontend->engine->world->objectCount) gSelected = brClickCallback(frontend->engine, n);

	glutSetMenu(contextMenu);

	total = glutGet(GLUT_MENU_NUM_ITEMS);

	for(n=0;n<total;n++) 
		glutRemoveMenuItem(1);

	if(gSelected) {
		l = &gSelected->menu;

		for(n=0;n<l->count;n++)
			glutAddMenuEntry(l->list[n]->title, n);
	} 
}

int brParseArgs(int argc, char **argv) {
	int r, error = 0;
	int level;

	while((r = getopt(argc, argv, "w:s:d:r:l:mvhfu")) != EOF) {
		switch(r) {
			case 'd':
				level = atoi(optarg);
				printf("debug level: %d\n", level);
				slSetDebugLevel(level);
				break;
			case 'r':
				srandom(atoi(optarg));
				srand(atoi(optarg));
				printf("random seed: %d\n", atoi(optarg));
				break;
			case 's':
				sscanf(optarg, "%dx%d\n", &width, &height);
				if(width <= 0 || width > 1000 || height <= 0 || height > 1000) {
					printf("-s size flag must be in the form NxM, where N and M\n");
					printf("are between 0 and 1000\n");
					error++;
				}
				break;
			case 'l':
				gSimFile = slStrdup(optarg);
				break;
			case 'v':
				brPrintVersion();
				break;
			case 'h':
				brPrintUsage(argv[0]);
				break;
			case 'f':
				gFull = 1;
				break;
			case 'u':
				gPaused = 0;
				break;
			case 'm':
				gFormat = 1;
				break;
			case 'w':
				sscanf(optarg, "%d,%d", &xpos, &ypos);
				break;
			default:
				printf("unknown option: '%c'\n", r);
				error++;
		}
	}

	if(error) brPrintUsage(argv[0]);

	return optind - 1;
}

void brPrintUsage(char *name) {
	fprintf(stderr, "usage: %s [options] simulation_file\n", name);
	fprintf(stderr, "options:\n");
	fprintf(stderr, "  -r <seed>       Sets the random seed to <seed>.\n");
	fprintf(stderr, "  -l <xml_file>   Dearchive simulation from <xml_file>.  <xml_file> should be\n");
	fprintf(stderr, "                  an archive of the simulation contained in the input file.\n");
	fprintf(stderr, "  -f              Start breve in fullscreen mode.\n");
	fprintf(stderr, "  -v              Display the current version number.\n");
	fprintf(stderr, "  -h              Display this information.\n");
	fprintf(stderr, "  -u              Unpause: begin running simulation immediately.\n");
	fprintf(stderr, "  -m              Format the simulation file and quit.\n");
	fprintf(stderr, "  -w X,Y          Move the window to coordinates (X, Y).\n");
	fprintf(stderr, "  -s NxM          Create a window of size (N, M).\n");
	fprintf(stderr, "\n");
	fprintf(stderr, "For full documentation, or to submit a bug report, visit the breve homepage:\n");
	fprintf(stderr, "http://www.spiderland.org/breve\n\n");
	exit(1);
}

int gLastX, gLastY, gMods, gSpecial;
double gStartCamX;

int gPaused = 1;

int gMotionCrosshair = 0;

int gFrameNo, gClipNo, gRecording;

void slInitGlut(int argc, char **argv, char *title) {
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DOUBLE|GLUT_RGBA|GLUT_DEPTH|GLUT_STENCIL|GLUT_ALPHA);
	glutInitWindowSize(width, height);
	if(xpos || ypos) glutInitWindowPosition(xpos, ypos);

	glutCreateWindow(title);

	gMainWindow = glutGetWindow();

	glEnable(GL_NORMALIZE);

	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

	glutMouseFunc(slDemoMouse);
	glutMotionFunc(slDemoMotion);
	glutDisplayFunc(slDemoDisplay);
	glutKeyboardFunc(slDemoKeyboard);
	glutSpecialFunc(slDemoSpecial);
	glutSpecialUpFunc(slDemoSpecialUp);
	glutKeyboardUpFunc(slDemoKeyboardUp);
	glutReshapeFunc(slDemoReshape);

	mainMenu = glutCreateMenu(brMainMenu);
	glutAttachMenu(GLUT_RIGHT_BUTTON);
	contextMenu = glutCreateMenu(brContextMenu);
	glutAttachMenu(GLUT_MIDDLE_BUTTON);

	glutMenuStatusFunc(brGLMenuStatus);

	glutIdleFunc(brGlutLoop);

	slInitGL(frontend->engine->world);

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void slDemoReshape(int x, int y) {
	frontend->engine->camera->x = x; frontend->engine->camera->y = y;

	frontend->engine->camera->fov = (double)x/(double)y;
	glViewport(0, 0, x, y);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void slDemoDisplay() {
	pthread_mutex_lock(&gEngineMutex);

	slRenderWorld(frontend->engine->world, frontend->engine->camera, 0, GL_RENDER, gMotionCrosshair, 0);

	pthread_mutex_unlock(&gEngineMutex);
	glutSwapBuffers();
}

void slDemoMouse(int button, int state, int x, int y) {
	if(state == GLUT_DOWN) { 
		gLastX = x;
		gLastY = y;
		gStartCamX = frontend->engine->camera->rx;

		brClick(slGlSelect(frontend->engine->world, frontend->engine->camera, x, y));

		gMotionCrosshair = 1;
	} else { 
		gLastX = 0;
		gLastY = 0;
		gMotionCrosshair = 0;
	}

	gMods = glutGetModifiers();

	slDemoDisplay();
}

void slDemoMotion(int x, int y) {
	slVector yaxis, xaxis, tempV;
	double m[3][3], n[3][3], o[3][3];

	slVectorSet(&yaxis, 0, 1, 0);
	slVectorSet(&xaxis, 1, 0, 0);

	slRotationMatrix(&yaxis, frontend->engine->camera->rx, m);
	slRotationMatrix(&xaxis, frontend->engine->camera->ry, n);
	slMatrixMulMatrix(m, n, o);

	slVectorXform(o, &yaxis, &tempV);
	slVectorCopy(&tempV, &yaxis);
	slVectorXform(o, &xaxis, &tempV);
	slVectorCopy(&tempV, &xaxis);

	if(gLastX || gLastY) {
		if(gMods & GLUT_ACTIVE_SHIFT) { 
			brDragCallback(frontend->engine, x, y);
		} else if(gSpecial == GLUT_KEY_F2) {
			slZoomCameraWithMouseMovement(frontend->engine->camera, x - gLastX, y - gLastY);
		} else if(gSpecial == GLUT_KEY_F3) {
			slMoveCameraWithMouseMovement(frontend->engine->camera, x - gLastX, y - gLastY);
		} else {
			slRotateCameraWithMouseMovement(frontend->engine->camera, x - gLastX, y - gLastY, gStartCamX);
		}

		gLastX = x; 
		gLastY = y;
	}

	glutPostRedisplay();
}

void slDemoSpecial(int key, int x, int y) {
	gSpecial = key;

	switch(key) {
		case GLUT_KEY_F1:
			if(brEngineIterate(frontend->engine) != EC_OK) slFatal("engine iteration failed\n");
			break;
		case GLUT_KEY_UP:
			slDemoMotion(0, -10);
			break;
		case GLUT_KEY_DOWN:
			slDemoMotion(0, 10);
			break;
		case GLUT_KEY_LEFT:
			slDemoMotion(-10, 0);
			break;
		case GLUT_KEY_RIGHT:
			slDemoMotion(10, 0);
			break;
	}

	glutPostRedisplay();
}

void slDemoSpecialUp(int key, int x, int y) {
	gSpecial = 0;
}

void slDemoKeyboard(unsigned char key, int x, int y) {
	if(keyDown[key]) return;

	keyDown[key] = 1;

	switch(key) {
		case ' ':
			gPaused = !gPaused;

			if(gPaused) {
				brPauseTimer(frontend->engine);
			} else {
				brUnpauseTimer(frontend->engine);
			}

			break;
		case 0x1b:
			if(!gWaiting) brInterrupt(frontend->engine);
			break;
		default:
			brKeyCallback(frontend->engine, key, 1);
			break;
	}
}

void brInterrupt(brEngine *engine) {
	char *line;

	gWaiting = 1;

	if(gFull) {
		glutReshapeWindow(400, 400);
		gFull = 0;
		return;
	}

	brPauseTimer(engine);

	printf("\n\nSimulation interupted.  Type a steve command, 'x' to quit, or hit enter to continue\n");
	fflush(stdout);

	line = readline("> ");
	if(*line) add_history(line);

	if(line[0] == 'x') brQuit(engine);

	if(*line && line[0] != '\n') stRunSingleStatement(frontend->data, engine, line);

	if(gFull) glutFullScreen();

	brUnpauseTimer(engine);
}

/*!
	\brief The GLUT key-up callback.  Calls \ref brKeyCallback.
*/

void slDemoKeyboardUp(unsigned char key, int x, int y) {
	brKeyCallback(frontend->engine, key, 0);
	keyDown[key] = 0;
}

/*!
	\brief A lame way to get a yes or no from the user.

	This would normally be done with a dialog box, but 
	this is GLUT and we don't have one...
*/

int brCLIDialogCallback(void *data, char *title, char *message, char *b1, char *b2) {
	int result;

	while (1) {
		printf("%s\n\n%s\n[Y/N]? ", title, message);

		result = getc(stdin);
		getc(stdin);

		if(result == EOF) {
			printf("EOF found, assuming answer is N\n");
			return 0; 
		}

		if(result == 'n' || result == 'N') return 0;

		if(result == 'y' || result == 'Y') return 1;
	} 
}

char *interfaceVersionCallback(void *data) {
	return interfaceID;
}

char *getSavename(void *data) {
	char *name = slMalloc(1024);
	printf("filename to save: ");
	fgets(name, 1023, stdin);
	if(strlen(name) > 0) name[strlen(name) - 1] = 0;
	return name;
}

char *getLoadname(void *data) {
	char *name = slMalloc(1024);
	printf("filename to load: ");
	fgets(name, 1023, stdin);
	if(strlen(name) > 0) name[strlen(name) - 1] = 0;
	return name;
}

int soundCallback(void *data) {
	putchar(0x07);
	fflush(stdout);
	return 0;
}

int pauseCallback() {
	gPaused = 1;
	brPauseTimer(frontend->engine);
	return 0;
}

void *newWindowCallback(char *name, slGraph *graph) {
	slGLUTWindow *w;

	w = malloc(sizeof(slGLUTWindow));
	w->id = glutCreateWindow(name);
	w->graph = graph;

	gWindows[w->id] = w;

	glutDisplayFunc(graphDisplay);

	return w;
}

void freeWindowCallback(void *w) {
	slGLUTWindow *window = w;

	glutDestroyWindow(window->id);
	slFree(w);
}

void renderWindowCallback(void *w) {
	slGLUTWindow *window = w;
	slDrawGraph(window->graph);
}

void graphDisplay() {
	int windowid = glutGetWindow();

	if(windowid > 1024) return;

	//printf("displaying window #%d...\n", windowid);
	
	glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
	renderWindowCallback(gWindows[windowid]);
	glutSwapBuffers();
}
