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

#include <getopt.h>
#include <stdio.h>
#include <sys/param.h>
#include <pthread.h>

#include "graph.h"

#if defined(HAVE_LIBREADLINE) && defined(HAVE_LIBHISTORY)
#include <readline/readline.h>
#include <readline/history.h>
#endif

/*!
	\brief Callback data for additional rendering windows.
*/

struct slGLUTWindow {
	int id;
	slGraph *graph;
};

typedef struct slGLUTWindow slGLUTWindow;

struct option gCLIOptions[] = {
        { "debug",      required_argument, 0, 'd' },
        { "random",     required_argument, 0, 'r' },
        { "notify",     required_argument, 0, 'n' },
        { "archive",    required_argument, 0, 'a' }, 
        { "version",    no_argument,       0, 'v' }, 
        { "terminate",  required_argument, 0, 't' },
        { "fullscreen", no_argument,       0, 'f' },
        { "format",     no_argument,       0, 'F' },
        { "unpause",    no_argument,       0, 'u' },
        { "size",       required_argument, 0, 's' },
        { "position",   required_argument, 0, 'p' },
        { "help",       no_argument,       0, 'h' },
        { "stdin",      no_argument,       0, 'i' },

        { "slave",      required_argument, 0, 'S' },
        { "master",     no_argument,       0, 'M' }
};

int brParseArgs(int argc, char **argv);

void brPrintUsage(char *name);

void glutLoop();

void brQuit(brEngine *e);
void brMainMenu(int n); 
void brGLMenuStatus(int stat, int x, int y);
void brContextualMenu(int n); 
void brClick(int n);

void brGlutMenuUpdate(brInstance *l);

void slInitGlut(int argc, char **argv, char *name);

void slDemoReshape(int x, int y);
void slDemoDisplay();
void slDemoMouse(int button, int state, int x, int y);
void slDemoMotion(int x, int y);
void slDemoPassiveMotion(int x, int y);
void slDemoSpecial(int key, int x, int y);
void slDemoSpecialUp(int key, int x, int y);
void slDemoKeyboard(unsigned char key, int x, int y);
void slDemoKeyboardUp(unsigned char key, int x, int y);

int brCLIDialogCallback(char *title, char *message, char *b1, char *b2);
char *interfaceVersionCallback();
char *getSavename();
char *getLoadname();
int soundCallback();
int pauseCallback();
