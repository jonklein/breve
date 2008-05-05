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

#include <string>

/*!
	\brief A class used to throw exceptions from the simulation engine 
	level to the steve language.
*/

class slException {
	public:
		slException( std::string m) {
			_message = m;
		}

		slException( std::string &m) {
			_message = m;
		}

		std::string _message;
};

/*!
	\brief Debug levels to allow different levels of output detail.
*/

enum slDebugLevels {
    NORMAL_OUTPUT 	= -1,  /*! normal output */
    DEBUG_ALL   	= 0,
    DEBUG_ERROR 	= 10,   /*! major errors */
    DEBUG_WARN  	= 25,   /*! warnings */
    DEBUG_INFO  	= 50,   /*! developer information */
    DEBUG_GEN   	= 100   /*! all output information */
};

#define slDebug(format, args...)	{ slMessage(0, "Error at source file \"%s\", line %d:\n", __FILE__, __LINE__); slMessage(0, format, ##args ); }

/**
 * Sets the debug filter level to one of \ref slDebugLevels.
 */

void slSetDebugLevel(int);

/**
 *Outputs a 3x3 matrix to the debug output.
 */

void slDebugMatrix(int, double [3][3]);

/**
 * Outputs a printf style message of a given error level to the active 
 * output function.
 */


DLLEXPORT void slMessage(int, const char *, ...);

/**
 * Outputs a formatted message of a given error level to the active 
 * output function.
 */

DLLEXPORT void slFormattedMessage( int level, const char *format );

/**
 * The default output functions, prints text to stderr/stdout.
 */

void slStderrMessageCallback( const char * );
void slStdoutMessageCallback( const char * );

/**
 * Set the message-output callback function.
 *
 * When output is produced, it will be passed to the message-output
 * callback function.  By default, the data will be printed to
 * stderr, but the callback function could ignore it, print it
 * to a file, etc.
 *
 * If slSetMessageCallbackFunctions is used, a separate handler
 * may be used for errors and for normal output.
 */

void slSetMessageCallbackFunction( void (*)( const char *) );
void slSetMessageCallbackFunctions( void (*)( const char *), void (*)( const char *) );
