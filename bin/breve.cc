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

#include "steve.h"
#include "breve.h"
#include "movie.h"

static void brInterrupt(brEngine *);
static void graphDisplay(void);
static void *workerThread(void *);

static void *newWindowCallback(char *, void *);
static void freeWindowCallback(void *);
static void renderWindowCallback(void *);

#if MINGW
extern char *optarg;
extern int optind;
#endif

static int contextMenu, mainMenu;

static int height = 400, width = 400;
static int xpos = 0, ypos = 0;

static breveFrontend *frontend;

static char keyDown[256];

static int gPaused = 1;
static int gWaiting = 0;

static int gThreadShouldExit = 0;

static pthread_mutex_t gEngineMutex;
static pthread_mutex_t gThreadMutex;
static pthread_cond_t gThreadPaused;

static char *gOptionArchiveFile = NULL;
static int gOptionFormat = 0;
static int gOptionFull = 0;
static int gOptionStdin = 0;

static int gMotionCrosshair = 0;

static int gLastX, gLastY, gMods, gSpecial;
static double gStartCamX;

static brInstance *gSelected = NULL;

static int gSlave = 0, gMaster = 0;
static char *gSlaveHost;

static std::map<int, void*> gWindowMap;

int main(int argc, char **argv) {
	pthread_t thread;
	char *text, *simulationFile;
	int index;
	char wd[MAXPATHLEN];

	interfaceID = "glut/2.0";

	srandom(time(NULL));

#if MINGW
	pthread_win32_process_attach_np();
#endif

	pthread_mutex_init(&gEngineMutex, NULL);
	pthread_mutex_init(&gThreadMutex, NULL);
	pthread_cond_init(&gThreadPaused, NULL);

	/* parse the command line args. */

	index = brParseArgs(argc, argv);

	// offset argc and argv to account for the options parsed out 

	argc -= index;
	argv += index;

	if (!gOptionStdin) {
		if (argc < 2)
			brPrintUsage(argv[0]);

		text = slUtilReadFile(argv[1]);
		simulationFile = argv[1];
	} else {
		text = slUtilReadStdin();
		simulationFile = "<stdin>";
	}

	if (!text) {
		printf("Error reading file \"%s\": %s\n", simulationFile, strerror(errno));
		exit(1);
	}

	if (gOptionFormat) {
		char *newtext = slFormatText(text);
		slUtilWriteFile(simulationFile, newtext);
		slFree(newtext);
		exit(0);
	}

#if HAVE_LIBAVFORMAT
	av_register_all();
#endif

	frontend = breveFrontendInit(argc, argv);
	frontend->data = breveFrontendInitData(frontend->engine);

	brEngineSetIOPath(frontend->engine, getcwd(wd, sizeof(wd)));

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

	slInitGlut(argc, argv, simulationFile);

	if (gOptionArchiveFile) {
		if (breveFrontendLoadSavedSimulation(frontend, text, simulationFile, gOptionArchiveFile) != EC_OK)
			brQuit(frontend->engine);
	} else if (breveFrontendLoadSimulation(frontend, text, simulationFile) != EC_OK)
		brQuit(frontend->engine);

	slFree(text);

	brEngineSetUpdateMenuCallback(frontend->engine, brGlutMenuUpdate);

	memset(keyDown, 0, sizeof(keyDown));

	pthread_mutex_lock(&gThreadMutex);
	pthread_create(&thread, NULL, workerThread, NULL);

	if (gMaster)
		slWorldStartNetsimServer(frontend->engine->world);

	if (gSlave) {
		slWorldStartNetsimSlave(frontend->engine->world, gSlaveHost);
		slFree(gSlaveHost);
	}

	if (gOptionFull)
		glutFullScreen();

	glutMainLoop();

	return 0;
}

void *workerThread(void *data) {
	for (;;) {
		pthread_mutex_lock(&gThreadMutex);

		if ((!gPaused && !frontend->engine->drawEveryFrame) ||
		    !pthread_cond_wait(&gThreadPaused, &gThreadMutex)) {
   		 	if (gThreadShouldExit) {
				pthread_mutex_unlock(&gThreadMutex);
				return NULL;
			}
			else if (brEngineIterate(frontend->engine) != EC_OK) {
				pthread_mutex_unlock(&gThreadMutex);
				brQuit(frontend->engine);
			}
		}

		pthread_mutex_unlock(&gThreadMutex);
	}
}

void brGlutLoop() {
	static int oldD = 1;
	int newD = frontend->engine->drawEveryFrame;

	gWaiting = 0;

	if (!gPaused) {
		if (newD && brEngineIterate(frontend->engine) != EC_OK)
			brQuit(frontend->engine);

		glutPostRedisplay();

		if (!newD && oldD) {
			pthread_mutex_unlock(&gThreadMutex);
			pthread_cond_signal(&gThreadPaused);
		} else if (newD && !oldD) 
			pthread_mutex_lock(&gThreadMutex);

		oldD = newD;
	}
}

void brGlutMenuUpdate(brInstance *i) {
	brMenuEntry *menu;
	char *message;
	unsigned int n, total;

	if (i != frontend->engine->controller)
		return;

	glutSetMenu(mainMenu);

	total = glutGet(GLUT_MENU_NUM_ITEMS);

	for (n = 0; n < total; ++n)
		glutRemoveMenuItem(1);

	for (n = 0; n < i->menus->count; ++n) {
		menu = (brMenuEntry *)i->menus->data[n];

		if (menu->enabled) {
			message = new char[strlen(menu->title) + 4];

			if (menu->checked)
				sprintf(message, "* %s", menu->title);
			else
				sprintf(message, "  %s", menu->title);

			glutAddMenuEntry(message, n);

			delete message;
		}

		else glutAddMenuEntry(" ", n);
	}
}

void brQuit(brEngine *e) {
	double diff, age;

	if (!gPaused && !frontend->engine->drawEveryFrame) {
		gThreadShouldExit = 1;
		pthread_mutex_lock(&gThreadMutex);
	}

	brPauseTimer(e);

	diff = e->realTime.tv_sec + (e->realTime.tv_usec / 1000000.0);

	age = slWorldGetAge(e->world);

	if (age != 0.0) {
		printf("%f simulated seconds elapsed\n", age);
		printf("%f real seconds elapsed\n", diff);
		printf("%f simulated/real\n", age/diff);
	}

	brEngineFree(frontend->engine);
	breveFrontendCleanupData(frontend->data);
	breveFrontendDestroy(frontend);

	exit(0);
}

void brMainMenu(int n) {
	brMenuCallback(frontend->engine, frontend->engine->controller, n);
	glutPostRedisplay();
}

void brContextMenu(int n) {
	if (gSelected)
		brMenuCallback(frontend->engine, gSelected, n);

	glutPostRedisplay();
}

void brClick(int n) {
	unsigned int m, total;

	gSelected = brClickCallback(frontend->engine, n);

	glutSetMenu(contextMenu);

	total = glutGet(GLUT_MENU_NUM_ITEMS);

	for(m = 0; m < total; ++m) 
		glutRemoveMenuItem(1);

	if (gSelected) {
		brMenuEntry *menu;

		for(m = 0; m < gSelected->menus->count; ++m) {
			menu = (brMenuEntry *)gSelected->menus->data[m];
			glutAddMenuEntry(menu->title, m);
		}
	} else
		glutSetMenu(mainMenu);

	glutAttachMenu(GLUT_RIGHT_BUTTON);
}

int brParseArgs(int argc, char **argv) {
	int level, r;
	int error = 0;

	const char *optstring = "a:d:fhip:r:s:uvFMS:";

#if HAVE_GETOPT_LONG
	while((r = getopt_long(argc, argv, optstring, gCLIOptions, NULL)) != -1)
#else 
	while((r = getopt(argc, argv, optstring)) != -1)
#endif
	{
		switch(r) {
		case 'a':
			gOptionArchiveFile = slStrdup(optarg);
			break;
		case 'd':
			level = atoi(optarg);
			printf("debug level: %d\n", level);
			slSetDebugLevel(level);
			break;
		case 'f':
			gOptionFull = 1;
			break;
		case 'i':
			gOptionStdin = 1;
			break;
		case 'p':
			sscanf(optarg, "%d,%d", &xpos, &ypos);
			break;
		case 'r':
			srandom(atoi(optarg));
			printf("random seed: %d\n", atoi(optarg));
			break;
		case 's':
			sscanf(optarg, "%dx%d\n", &width, &height);
			if (width <= 0 || width > 1000 ||
			    height <= 0 || height > 1000) {
				printf("-s size flag must be in the form NxM, \
				    where N and M are from 1 to 1000\n");
				error++;
			}
			break;
		case 'u':
			gPaused = 0;
			glutIdleFunc(brGlutLoop);
			break;
		case 'v':
			brPrintVersion();
			break;
		case 'F':
			gOptionFormat = 1;
			break;
		case 'M':
			gMaster = 1;
			break;
		case 'S':
			gSlaveHost = slStrdup(optarg);
			gSlave = 1;
			break;
		default:
			printf("unknown option: '%c'\n", r);
		case 'h':
			error++;
		}
	}

	if (error)
		brPrintUsage(argv[0]);

	return optind - 1;
}

void brPrintUsage(char *name) {
	fprintf(stderr, "usage: %s [options] simulation_file\n", name);
	fprintf(stderr, "options:\n");
	fprintf(stderr, "  -r <seed>       Sets the random seed to <seed>.\n");
	fprintf(stderr, "  -a <xml_file>   Dearchive simulation from <xml_file>.  <xml_file> should be\n");
	fprintf(stderr, "                  an archive of the simulation contained in the input file.\n");
	fprintf(stderr, "  -f              Start breve in fullscreen mode.\n");
	fprintf(stderr, "  -u              Unpause: begin running simulation immediately.\n");
	fprintf(stderr, "  -F              Format the simulation file and quit.\n");
	fprintf(stderr, "  -w X,Y          Move the window to coordinates (X, Y).\n");
	fprintf(stderr, "  -s NxM          Create a window of size (N, M).\n");
	fprintf(stderr, "  -v              Display the current version number.\n");
	fprintf(stderr, "  -h              Display this information.\n");
	fprintf(stderr, "\n");
	fprintf(stderr, "For full documentation, or to submit a bug report, visit the breve homepage:\n");
	fprintf(stderr, "http://www.spiderland.org/breve\n\n");
	exit(1);
}

void slInitGlut(int argc, char **argv, char *title) {
	glutInitWindowSize(width, height);
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DOUBLE|GLUT_RGBA|GLUT_DEPTH|GLUT_STENCIL);
	glutCreateWindow(title);

	if (xpos || ypos)
		glutInitWindowPosition(xpos, ypos);

	glutGetWindow();

	glutMouseFunc(slDemoMouse);
	glutMotionFunc(slDemoMotion);
	glutPassiveMotionFunc(slDemoPassiveMotion);
	glutDisplayFunc(slDemoDisplay);
	glutSpecialFunc(slDemoSpecial);
	glutSpecialUpFunc(slDemoSpecialUp);
	glutReshapeFunc(slDemoReshape);
	glutKeyboardUpFunc(slDemoKeyboardUp);
	glutKeyboardFunc(slDemoKeyboard);

	mainMenu = glutCreateMenu(brMainMenu);
	glutAttachMenu(GLUT_RIGHT_BUTTON);
	contextMenu = glutCreateMenu(brContextMenu);
	glutAttachMenu(GLUT_MIDDLE_BUTTON);

	slInitGL(frontend->engine->world, frontend->engine->camera);

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void slDemoReshape(int x, int y) {
	frontend->engine->camera->x = x;
	frontend->engine->camera->y = y;

	frontend->engine->camera->fov = x / (double)y;
	glViewport(0, 0, x, y);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void slDemoDisplay() {
	brEngineLock(frontend->engine);
	brEngineRenderWorld(frontend->engine, gMotionCrosshair);
	brEngineUnlock(frontend->engine);

	glutSwapBuffers();
}

void slDemoMouse(int button, int state, int x, int y) {
	gMods = glutGetModifiers();

	if (state == GLUT_DOWN) { 
		gLastX = x;
		gLastY = y;
		gStartCamX = frontend->engine->camera->rx;

		brClick(slGlSelect(frontend->engine->world, frontend->engine->camera, x, y));
		brGlutMenuUpdate(frontend->engine->controller);

		gMotionCrosshair = 1;

		// if(gMods & GLUT_ACTIVE_SHIFT) brBeginDrag(frontend->engine, gSelected);

	} else { 
		gLastX = 0;
		gLastY = 0;
		gMotionCrosshair = 0;

		// brEndDrag(frontend->engine, gSelected);
	}


	slDemoDisplay();
}

void slDemoPassiveMotion(int x, int y) {
	frontend->engine->mouseX = x;
	frontend->engine->mouseY = frontend->engine->camera->y - y;
}

void slDemoMotion(int x, int y) {
	if(gLastX || gLastY) {
		if((gMods & GLUT_ACTIVE_SHIFT) || (gSpecial == GLUT_KEY_F4)) { 
			brDragCallback(frontend->engine, x, y);
		} else if((gMods & GLUT_ACTIVE_ALT) || (gSpecial == GLUT_KEY_F2)) {
			slZoomCameraWithMouseMovement(frontend->engine->camera, x - gLastX, y - gLastY);
		} else if((gMods & GLUT_ACTIVE_CTRL) || (gSpecial == GLUT_KEY_F3)) {
			slMoveCameraWithMouseMovement(frontend->engine->camera, x - gLastX, y - gLastY);
		} else {
			slRotateCameraWithMouseMovement(frontend->engine->camera, x - gLastX, y - gLastY);
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
			if (brEngineIterate(frontend->engine) != EC_OK)
				slFatal("engine iteration failed\n");
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
	if (keyDown[key])
		return;

	keyDown[key] = 1;

	switch(key) {
		case ' ':
			gPaused = !gPaused;

			if (gPaused) {
				brPauseTimer(frontend->engine);
				glutIdleFunc(NULL);
			} else {
				brUnpauseTimer(frontend->engine);
				pthread_cond_signal(&gThreadPaused);
				glutIdleFunc(brGlutLoop);
			}

			break;
		case 0x1b:
			if (!gWaiting)
				brInterrupt(frontend->engine);
			break;
		default:
			brKeyCallback(frontend->engine, key, 1);
			break;
	}
}

void brInterrupt(brEngine *engine) {
	char *line;
	char staticLine[MAXPATHLEN];	

	gWaiting = 1;

	if (gOptionFull) {
		glutReshapeWindow(400, 400);
		gOptionFull = 0;
		return;
	}

	brPauseTimer(engine);

	fflush(stdin);
	printf("\n\nSimulation interupted.  Type a steve command, 'x' to quit, or hit enter to continue\n");
	fflush(stdout);

#if HAVE_LIBREADLINE && HAVE_LIBHISTORY
	line = readline("breve> ");
	if (line && *line)
		add_history(line);
#else
	printf("breve> ");
	fflush(stdout);
	line = fgets(staticLine, sizeof(staticLine), stdin);
#endif

	if (!line || *line == 'x')
		brQuit(engine);

	if (*line != '\0' && *line != '\n')
		stRunSingleStatement((stSteveData *)frontend->data, engine, line);

	if (line != staticLine)
		free(line);

	if (gOptionFull)
		glutFullScreen();

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

int brCLIDialogCallback(char *title, char *message, char *b1, char *b2) {
	int result;

	for (;;) {
		printf("%s\n\n%s\n[Y/N]? ", title, message);

		result = getchar();
		(void)getchar();

		switch(result) {
		case EOF:
		case 'n':
		case 'N':
			return 0;
		case 'y':
		case 'Y':
			return 1;
		}
	} 
}

char *interfaceVersionCallback() {
	return interfaceID;
}

char *getSavename() {
	char name[MAXPATHLEN];

	printf("filename to save: ");
	fgets(name, sizeof(name), stdin);

	if (*name)
		name[strlen(name) - 1] = 0;

	return slStrdup(name);
}

char *getLoadname() {
	char name[MAXPATHLEN];

	printf("filename to load: ");
	fgets(name, sizeof(name), stdin);

	if (*name)
		name[strlen(name) - 1] = '\0';

	return slStrdup(name);
}

int soundCallback() {
	putchar(0x07);
	fflush(stdout);
	return 0;
}

int pauseCallback() {
	gPaused = 1;
	brPauseTimer(frontend->engine);
	glutIdleFunc(NULL);
	return 0;
}

void *newWindowCallback(char *name, void *graph) {
	slGLUTWindow *w;

	w = new slGLUTWindow;
	w->id = glutCreateWindow(name);
	w->graph = (slGraph *)graph;

	gWindowMap[w->id] = w;

	glutDisplayFunc(graphDisplay);

	return w;
}

void freeWindowCallback(void *w) {
	slGLUTWindow *window = (slGLUTWindow *)w;

	glutDestroyWindow(window->id);
	delete window;
}

void renderWindowCallback(void *w) {
	slGLUTWindow *window = (slGLUTWindow *)w;
	slDrawGraph(window->graph);
}

void graphDisplay() {
	int windowid = glutGetWindow();

	if (windowid > 1024)
		return;

	//printf("displaying window #%d...\n", windowid);
	
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	renderWindowCallback(gWindowMap[windowid]);
	glutSwapBuffers();
}
