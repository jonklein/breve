/*!
	\brief A structure that allows data written to a stream to be retrieved as a string.

	A string-stream is a file stream whose output is returned as a string when the 
	stream is closed.
*/

struct slStringStream {
	char *filename;
	FILE *fp;
};

slStringStream *slOpenStringStream();
char *slCloseStringStream(slStringStream *stream);

