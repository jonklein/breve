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

/* the normal debug level of a user is should be 25     */
/* the normal debug level of a developer is 50          */

/* debug level 100 can generate 100s of lines of text in a single */
/* iteration--for example, every feature pair tested in collision */
/* detection */

enum debugLevels {
    NORMAL_OUTPUT = 0,  /* normal output                */
    DEBUG_ALL   = 0,
    DEBUG_ERROR = 10,   /* major errors                 */
    DEBUG_WARN  = 25,   /* warnings                     */
    DEBUG_INFO  = 50,   /* developer information        */
    DEBUG_GEN   = 100   /* waaaaay too much information */
};

#define slDebug(format, ...)	{ slMessage(0, "Error at source file \"%s\", line %d:\n", __FILE__, __LINE__); slMessage(0, format, ## __VA_ARGS__); }

void slSetDebugLevel(int level);

void slDebugFunction(char *file, int line, char *text, ...);

void slDebugMatrix(int level, double m[3][3]);

void slMessage(int level, char *format, ...);
void slFormattedMessage(int level, char *string);

void slStderrMessageCallback(char *string);

void slFatal(char *format, ...);
void slUtilPause();

void slStackTrace();

void slSetMessageCallbackFunction(void (f)(char *text));
