#include "util.h"

/*
	this is a way to get a string from output formatted to a 
	FILE pointer.  so you can open a string stream, write to
	it with fprintf (or whatever), then retreive the text as 
	a string.

	presently, this is done using a temp file.

	we could use funopen() under Mac OS X.  
*/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>

slStringStream *slOpenStringStream(void) {
	slStringStream *stream;
	int fd;
	char nameTemplate[128];

#ifdef HAVE_MKSTEMP
	sprintf(nameTemplate, "/tmp/breve_temp.XXXXXXXXXX");
	fd = mkstemp(nameTemplate);
#else
	sprintf(nameTemplate, "breve_temp.%lu", (unsigned long)random() % 10000000);
	fd = open(nameTemplate, O_CREAT | O_EXCL | O_WRONLY, 0600);
#endif

	stream = new slStringStream;
	if ((stream->fp = fdopen(fd, "w")) == NULL) {
	    if (fd != -1) {
		unlink(nameTemplate);
		close(fd);
	    }
	    perror("couldn't open tmp file");
	    delete stream;
	    return NULL;
	}
	stream->filename = slStrdup(nameTemplate);

	return stream;
}

char *slCloseStringStream(slStringStream *stream) {
	char *result;

	fclose(stream->fp);

	result = slUtilReadFile(stream->filename);
	unlink(stream->filename);
	slFree(stream->filename);
	delete stream;

	return result;
}
