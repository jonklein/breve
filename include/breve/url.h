
#ifndef _URL_H
#define _URL_H

#include "slutil.h"

#ifdef HAVE_LIBCURL

#include <curl/curl.h>
#include <map>
#include <string>

class brURLFetcher {
	public:
							brURLFetcher();
							~brURLFetcher();

		int 				get( char *inURL, char **outData = NULL, int *outSize = NULL );
		int 				put( char *inURL, char *inPutData, int inSize, char **outData = NULL, int *outSize = NULL );

	private:
		int 				fetchURL( char *inURL, std::map< std::string, std::string > *inVariables, char **outData = NULL, int *outSize = NULL );

		CURL*				_curl;
};

#endif // HAVE_LIBCURL
#endif // _URL_H
