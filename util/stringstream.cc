#include "util.h"

/*
	this is a way to get a string from output formatted to a 
	FILE pointer.  so you can open a string stream, write to
	it with fprintf (or whatever), then retreive the text as 
	a string.

	presently, this is done using a temp file.

	we could use funopen() under Mac OS X.  
*/

slStringStream *slOpenStringStream() {
	slStringStream *stream;
	int fd;
	char template[128];

#ifdef WINDOWS
	// no mkstemp -- here's an inaqaduate solution!
	sprintf(template, "breve_temp.%d", random() % 10000);
#else 
	sprintf(template, "/tmp/breve_temp.XXXXXX");
	fd = mkstemp(template);
#endif

	stream = slMalloc(sizeof(slStringStream));	
	stream->fp = fdopen(fd, "w");
	stream->filename = slStrdup(template);

	return stream;
}

char *slCloseStringStream(slStringStream *stream) {
	char *result;

	fclose(stream->fp);

	result = slUtilReadFile(stream->filename);
	unlink(stream->filename);
	slFree(stream->filename);
	slFree(stream);

	return result;
}
