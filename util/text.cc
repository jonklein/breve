/*****************************************************************************
 *																		     *
 * The breve Simulation Environment										     *
 * Copyright (C) 2000, 2001, 2002, 2003 Jonathan Klein					     * 
 *																		     *
 * This program is free software; you can redistribute it and/or modify	     *
 * it under the terms of the GNU General Public License as published by	     *
 * the Free Software Foundation; either version 2 of the License, or		 *
 * (at your option) any later version.									     *
 *																		     *
 * This program is distributed in the hope that it will be useful,		     *
 * but WITHOUT ANY WARRANTY; without even the implied warranty of			 *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the			 *
 * GNU General Public License for more details.							     *
 *																		     *
 * You should have received a copy of the GNU General Public License		 *
 * along with this program; if not, write to the Free Software			     *
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA *
 *****************************************************************************/

/*!
	= text.c -- lower level utility functions related to text processing.
	= most of the hardcore text processing for steve is done with lex and 
	= yacc and is found in the steve source tree.
*/

extern int errno;

#include "util.h"
#include <string.h>
#include <sys/types.h>

#ifndef WINDOWS
#include <sys/socket.h>
#endif

/*!
	\brief An implementation of strdup returning an slMalloc'd string.

	Returns a copy of str, allocated by slMalloc.  The string must be
	freed later using slFree.
*/
	
char *slStrdup(char *str) {
	char *copy;
 
	if(!str) return NULL;
 
	copy = slMalloc(strlen(str) + 1);
 
	if(!copy) return NULL;
 
	strcpy(copy, str);
 
	return copy;
}

/*!
	\brief Splits a string into several smaller strings based on a 
	delimiter substring.

	slSplit works more or less like the perl split function.  it takes 
	a query string, a substring and an index N, and returns the Nth
	element of the query string using the substring as a delimiter.

	The returned string is slMalloc'd, and should be slFree'd after use.
*/

char *slSplit(char *start, char *substr, int n) {
	int count = 0;
	char *oldstart, *result;

	oldstart = start;

	while(start && count <= n) {
		oldstart = start;
		start = strstr(start, substr);

		if(start) start += strlen(substr);

		count++;
	}

	if(count != n + 1) return NULL;

	if(start) {
		start -= strlen(substr);

		result = slMalloc((start - oldstart) + 1);
		strncpy(result, oldstart, start - oldstart);
		result[start - oldstart] = 0;
	} else result = slStrdup(oldstart);

	return result;
}

/*!
	\brief A wrapper around fwrite() that guarantees that all bytes are 
	read before returning.
*/

int slUtilFwrite(void *ptr, size_t size, size_t nmemb, FILE *stream) {
	unsigned int n, written = 0;

	while(written < nmemb) {
		n = fwrite(ptr + (written * size), size, nmemb - written, stream);

		if(n < 1) return -1;

		written += n;
	}

	return nmemb;
}

/*!
	\brief A wrapper around fread() that guarantees that all bytes are read 
	before returning.
*/

int slUtilFread(void *ptr, size_t size, size_t nmemb, FILE *stream) {
	unsigned int n, readcount = 0; 
  
	while(readcount < nmemb) {
		n = fread(ptr + (readcount * size), size, nmemb - readcount, stream);
 
		if(n<1) return -1;
  
		readcount += n;
	}
 
	return nmemb;
} 

/*!
	\brief A wrapper around gzread() that guarantees that all bytes are read 
	before returning.
*/

int slUtilGzread(char *ptr, int size, gzFile stream) {
	int n, readcount = 0; 
  
	do {
		n = gzread(stream, ptr + readcount, (size - readcount));
 
		if(n < 0) return -1;
  
		readcount += n;
	} while(n && size != readcount);
 
	return readcount;
} 

/*!
	\brief Reads the (text) file at a certain path, returning a pointer to 
	the data as a slMalloc'd string.
*/

char *slUtilReadFile(char *path) {
	struct stat st;
	char *buffer;
	char temp[2048];
	gzFile fp;
	int total = 0, n;

	if(stat(path, &st)) {
		// unable to stat 
		return NULL;
	}

	if(!(fp = gzopen(path, "r"))) {
		slMessage(DEBUG_ALL, "error opening file \"%s\": %s\n", path, strerror(errno));
		return NULL;
	}

	buffer = slMalloc(1);

	while((n = slUtilGzread(temp, 2048, fp))) {
		if(n == -1) {
			slMessage(DEBUG_ALL, "error reading from file \"%s\"\n", path);
			return NULL;
		}

		buffer = slRealloc(buffer, total + n + 1);

		bcopy(temp, &buffer[total], n);
		
		total += n;
	}

	buffer[total] = 0;

	gzclose(fp);
	
	return buffer;
} 

/*!
	\brief Writes text to a file.
*/

int slUtilWriteFile(char *path, char *text) {
	FILE *f;

	f = fopen(path, "w");

	if(!f) return -1;

	slUtilFwrite(text, strlen(text), 1, f);

	fclose(f);

	return 0;
}

/*!
	\brief Reads a stream, returning the pointer to the data as a slMalloc'd string.
*/

char *slUtilReadStream(FILE *stream) {
	char *buffer;
	char temp[2048];
	int total = 0, n;

	buffer = slMalloc(1);

	while((n = slUtilFread(temp, 2048, 1, stream))) { // TODO:  is 1 the right value
		if(n == -1) {
			slMessage(DEBUG_ALL, "error reading from Stream\n");
			return NULL;
		}

		buffer = slRealloc(buffer, total + n + 1);

		bcopy(temp, &buffer[total], n);
		
		total += n;
	}

	buffer[total] = 0;
	
	return buffer;
} 

/*!
	\brief Strips the quotes off of a string and returns it as a slMalloc'd
	pointer.
*/

char *slDequote(char *d) {
	char *n; 
	int len;
	
	len = strlen(d);  

	/* there was a time when we would return NULL for an empty quoted */
	/* string, "".  now we'll return a malloc'd string which is just  */
	/* a '\0'. */
			
	if(len <= 1) return NULL;
	
	n = slMalloc(len - 1);
		
	strncpy(n, &d[1], len - 2);
	n[len - 2] = 0;  

	return n;
}

/*!
	\brief A wrapper around read() that guarantees that all bytes are read 
	before returning.
*/

int slUtilRead(int socket, void *buffer, size_t size ) {
	int n, readcount = 0; 
  
	while(readcount < size) {
		n = read(socket, buffer + readcount, size - readcount);
		if(n < 1) return readcount;
  
		readcount += n;
	}
 
	return readcount;
} 

/*!
	\brief A wrapper around write() that guarantees that all bytes are written 
	before returning.
*/

int slUtilWrite(int socket, void *buffer, size_t size ) {
	int n, writecount = 0; 
  
	while(writecount < size) {
		n = write(socket, buffer + writecount, size - writecount);
		if(n < 1) return writecount;
  
		writecount += n;
	}
 
	return size;
} 

/*!
	\brief A homegrown fgets() replacement that deals with CR, NL,
	CRNL, NLCRLMPQ, WNOC and PBS.
*/
	
char *slFgets(char *str, int size, FILE *stream) {
	int n = 0;
	int s;

	do {
		str[n++] = s = fgetc(stream);

		if(s == '\r') {
			// check for \r\n -- if there is a \n, eat it
			// otherwise, put it back in the stream

			int next = fgetc(stream);

			if(next != '\n') ungetc(next, stream);
		}
	} while(s != EOF && s != '\n' && s != '\r');

	if(s == EOF) n--;
	str[n] = 0;

	return str;
}
