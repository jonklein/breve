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

#if HAVE_SYS_PARAM_H
#include <sys/param.h>
#endif

#include <getopt.h>
#include <stdio.h>
#include <pthread.h>

#if HAVE_LIBREADLINE && HAVE_LIBHISTORY
#include <readline/readline.h>
#include <readline/history.h>
#endif

#include "graph.h"

#ifndef MAXPATHLEN
#define MAXPATHLEN 10240
#endif

#if defined(__GNUC__) && !defined(__STRICT_ANSI__)
#define __dead __attribute__((__noreturn__))
#else
#define __dead
#endif

/*!
	\brief Callback data for additional rendering windows.
*/

struct slGLUTWindow {
	int id;
	slGraph *graph;
};

typedef struct slGLUTWindow slGLUTWindow;

const struct option gCLIOptions[] = {
        { "archive",    required_argument, 0, 'a' }, 
        { "debug",      required_argument, 0, 'd' },
        { "fullscreen", no_argument,       0, 'f' },
        { "help",       no_argument,       0, 'h' },
        { "stdin",      no_argument,       0, 'i' },
        { "notify",     required_argument, 0, 'n' },
        { "position",   required_argument, 0, 'p' },
        { "random",     required_argument, 0, 'r' },
        { "size",       required_argument, 0, 's' },
        { "terminate",  required_argument, 0, 't' },
        { "unpause",    no_argument,       0, 'u' },
        { "version",    no_argument,       0, 'v' }, 
        { "format",     no_argument,       0, 'F' },
        { "master",     no_argument,       0, 'M' },
        { "slave",      required_argument, 0, 'S' }
};

int brParseArgs(int, char **);

__dead void brPrintUsage(char *);

__dead void brQuit(brEngine *);

void brClick(int);

void brContextualMenu(int); 
void brMainMenu(int); 

void brGlutMenuUpdate(brInstance *);

void brGlutLoop(void);

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
