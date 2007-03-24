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
	= text.c -- lower level utility functions related to text processing.
*/

#include <sys/types.h>
#include <sys/stat.h>

#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "util.h"

/*!
	\brief Splits a string into several smaller strings based on a
	delimiter substring.

	slSplit works more or less like the perl split function.  it takes
	a query string, a substring and an index N, and returns the Nth
	element of the query string using the substring as a delimiter.

	The returned string is slMalloc'd, and should be slFree'd after use.
*/

char *slFileExtension( const char *inName ) {
	int n = strlen( inName ) - 1;

	while( n > 0 && inName[ n ] != '.' ) 
		n--;

	if( n == 0 ) 
		return slStrdup( "" );
	else
		return slStrdup( &inName[ n + 1 ] );
}

char *slDirname( const char *inName ) {
	int index = strlen( inName ) - 1;
	char *dir = slStrdup( inName );


	while( index > -1 ) {
		if( dir[ index ] == '\\' || dir[ index ] == '/' ) {
			dir[ index + 1 ] = '\0';
			return dir;
		}
			

		index--;
	}

	return dir;
}

char *slSplit( char *start, char *substr, int n ) {
	int count = 0;
	char *oldstart, *result;

	oldstart = start;

	while ( start && count <= n ) {
		oldstart = start;
		start = strstr( start, substr );

		if ( start ) start += strlen( substr );

		count++;
	}

	if ( count != n + 1 ) return NULL;

	if ( start ) {
		start -= strlen( substr );

		result = ( char* )slMalloc(( start - oldstart ) + 1 );
		strncpy( result, oldstart, start - oldstart );
		result[start - oldstart] = 0;
	} else result = slStrdup( oldstart );

	return result;
}

/*!
	\brief A wrapper around fwrite() that guarantees that all bytes are
	written before returning.
*/

int slUtilFwrite( const void *ptr, size_t size, size_t nmemb, FILE *stream ) {
	ssize_t n;
	size_t written = 0;

	while ( written < nmemb ) {
		n = fwrite(( char * )ptr + ( written * size ), size,
		           nmemb - written, stream );

		if ( n < 1 )
			return -1;

		written += n;
	}

	return written;
}

/*!
	\brief A wrapper around fread() that guarantees that all bytes are read
	before returning.
*/

int slUtilFread( void *ptr, size_t size, size_t nmemb, FILE *stream ) {
	ssize_t n;
	size_t readcount = 0;

	while ( readcount < nmemb ) {
		n = fread(( char * )ptr + ( readcount * size ), size,
		          nmemb - readcount, stream );

		if ( n < 1 )
			return -1;

		readcount += n;
	}

	return readcount;
}

/*!
	\brief A wrapper around gzread() that guarantees that all bytes are read
	before returning.
*/

int slUtilGzread( char *ptr, size_t size, gzFile stream ) {
	ssize_t n;
	size_t readcount = 0;

	do {
		n = gzread( stream, ptr + readcount, ( size - readcount ) );

		if ( n < 0 )
			return -1;

		readcount += n;
	} while ( n && size != readcount );

	return readcount;
}

/*!
	\brief Reads the (text) file at a certain path, returning a pointer to
	the data as a slMalloc'd string.
*/

char *slUtilReadFile( const char *path ) {
	char temp[2048];

	struct stat st;
	char *buffer;
	gzFile fp;
	int total = 0, n;

	if ( stat( path, &st ) )
		return NULL;

	if ( !( fp = gzopen( path, "r" ) ) ) {
		slMessage( DEBUG_ALL, "gzopen %s: %s\n", path, strerror( errno ) );
		return NULL;
	}

	/*
	 * Because uncompressed size is not predictable from stat(),
	 * which gives size of compressed file, buffer must be
	 * dynamically resized.
	 */

	buffer = ( char * )slMalloc( 1 );

	while (( n = slUtilGzread( temp, sizeof( temp ), fp ) ) ) {
		if ( n == -1 ) {
			slMessage( DEBUG_ALL, "slUtilGzread %s: error)\n", path );
			return NULL;
		}

		buffer = ( char * )slRealloc( buffer, total + n + 1 );

		memcpy( &buffer[total], temp, n );

		total += n;
	}

	buffer[total] = 0;

	gzclose( fp );

	return buffer;
}

/*!
	\brief Writes text to a file.
*/

int slUtilWriteFile( const char *path, const char *text ) {
	FILE *f;

	if ( !( f = fopen( path, "w" ) ) )
		return -1;

	slUtilFwrite( text, strlen( text ), 1, f );

	fclose( f );

	return 0;
}

/*!
	\brief Reads a stream, returning the pointer to the data as a slMalloc'd
	string.
*/

char *slUtilReadStream( FILE *stream ) {
	char *buffer;
	char temp[2048];
	int total = 0, n;

	buffer = ( char * )slMalloc( 1 );

	while (( n = slUtilFread( temp, sizeof( temp ), 1, stream ) ) ) {
		if ( n == -1 ) {
			slMessage( DEBUG_ALL, "error reading from Stream\n" );
			return NULL;
		}

		buffer = ( char * )slRealloc( buffer, total + n + 1 );

		memcpy( &buffer[total], temp, n );

		total += n;
	}

	buffer[total] = 0;

	return buffer;
}

/*!
	\brief Strips the quotes off of a string and returns it as a slMalloc'd
	pointer.
*/

char *slDequote( char *d ) {
	char *n;
	int len;

	len = strlen( d );

	/* there was a time when we would return NULL for an empty quoted */
	/* string, "".  now we'll return a malloc'd string which is just  */
	/* a '\0'. */

	if ( len <= 1 ) return NULL;

	n = ( char* )slMalloc( len - 1 );

	strncpy( n, &d[1], len - 2 );

	n[len - 2] = 0;

	return n;
}

/*!
	\brief A wrapper around read() that guarantees that all bytes are read
	before returning.
*/

int slUtilRead( int socket, void *buffer, size_t size ) {
	ssize_t n;
	size_t readcount = 0;

	while ( readcount < size ) {
		n = read( socket, ( char * )buffer + readcount, size - readcount );

		if ( n < 1 )
			break;

		readcount += n;
	}

	return readcount;
}

/*!
	\brief A wrapper around write() that guarantees that all bytes are
	written before returning.
*/

int slUtilWrite( int socket, const void *buffer, size_t size ) {
	ssize_t n;
	size_t writecount = 0;

	while ( writecount < size ) {
		n = write( socket, ( char * )buffer + writecount, size - writecount );

		if ( n < 1 )
			break;

		writecount += n;
	}

	return size;
}

/*!
	\brief A homegrown fgets() replacement that deals with CR, NL,
	CRNL, NLCRLMPQ, WNOC and PBS.
*/

char *slFgets( char *str, int size, FILE *stream ) {
	int next, s;
	int n = 0;

	do {
		str[ n++ ] = s = fgetc( stream );

		if ( s == '\r' ) {
			// check for \r\n -- if there is a \n, eat it
			// otherwise, put it back in the stream

			next = fgetc( stream );

			if ( next != '\n' )
				ungetc( next, stream );
		}
	} while ( s != EOF && s != '\n' && s != '\r' );

	if ( s == EOF )
		n--;

	str[ n ] = 0;

	return str;
}

/*!
	\brief Reads all the text from stdin.
*/

char *slUtilReadStdin() {
	char *data = ( char * )slMalloc( 4096 );
	int n, total = 0;

	while ( !feof( stdin ) ) {
		n = fread( &data[total], 1, 4095, stdin );
		total += n;

		data[total] = 0;

		data = ( char * )slRealloc( data, total + 4096 );
	}

	return data;
}
