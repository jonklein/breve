#include "util.h"

/*
	this is a way to get a string from output formatted to a 
	FILE pointer.  so you can open a string stream, write to
	it with fprintf (or whatever), then retreive the text as 
	a string.

	presently, this is done using a temp file.

	we could use funopen() under Mac OS X.  
*/

#include <stdlib.h>

slStringStream *slOpenStringStream() {
	slStringStream *stream;
	int fd;
	char nameTemplate[128];

#ifdef WINDOWS
	// no mkstemp -- here a totally inaqaduate solution!
	sprintf(nameTemplate, "breve_temp.%d", random() % 10000);
#else 
	sprintf(nameTemplate, "/tmp/breve_temp.XXXXXX");
	fd = mkstemp(nameTemplate);
#endif

	stream = new slStringStream;
	stream->fp = fdopen(fd, "w");
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
