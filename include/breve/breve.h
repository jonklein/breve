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

int brParseArgs(int, char **);

void brPrintUsage(char *);

void glutLoop(void);

void brQuit(brEngine *);
void brMainMenu(int); 
void brGLMenuStatus(int, int, int);
void brContextualMenu(int); 
void brClick(int);

void brGlutMenuUpdate(brInstance *);

void slInitGlut(int, char **, char *);

void slDemoReshape(int, int);
void slDemoDisplay(void);
void slDemoMouse(int, int, int, int);
void slDemoMotion(int, int);
void slDemoPassiveMotion(int, int);
void slDemoSpecial(int, int, int);
void slDemoSpecialUp(int, int, int);
void slDemoKeyboard(unsigned char, int, int);
void slDemoKeyboardUp(unsigned char, int, int);

int brCLIDialogCallback(char *, char *, char *, char *);
char *interfaceVersionCallback(void);
char *getSavename(void);
char *getLoadname(void);
int soundCallback(void);
int pauseCallback(void);
