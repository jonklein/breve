

#include "url.h"

#ifdef HAVE_LIBCURL


struct brURLWriteData {
							brURLWriteData() {
								_maxSize = 512;
								_size = 0;
								_data = (char*)malloc( _maxSize );
							}

							~brURLWriteData() {
								free( _data );	
							}

    char 					*_data;
    size_t 					_maxSize, _size;
};

struct brURLReadData {
							brURLReadData( char *inData, size_t inSize ) {
								_data = inData;
								_size = inSize;
								_position = 0;
							}

	char					*_data;
	size_t					_size;
	size_t					_position;
};

size_t brURLWrite( void *ptr, size_t size, size_t nmemb, void *stream ) {
	brURLWriteData *data = ( brURLWriteData* )stream;

	const size_t len = size * nmemb;
	const size_t newSize = data->_size + len;

	if ( data->_maxSize <= newSize ) {
		data->_maxSize = newSize + 1;
		data->_data = ( char* )slRealloc( data->_data, data->_maxSize );
	 }
    
	memcpy( &data->_data[ data->_size ], ptr, len );

	data->_size = newSize;
	data->_data[ newSize ] = 0;

	return len;
}

size_t brURLRead( void *ptr, size_t size, size_t nmemb, void *stream ) {
	brURLReadData *data = ( brURLReadData* )stream;
	size_t len = size * nmemb;

	if( len + data -> _position > data -> _size ) 
		len = ( data -> _size - data -> _position );

	memcpy( ptr, &data-> _data[ data -> _position ], len );

	data -> _position += len;

	return len;
}




brURLFetcher::brURLFetcher() {
	_curl = curl_easy_init();
}

brURLFetcher::~brURLFetcher() {
	curl_easy_cleanup( _curl );
}

int brURLFetcher::fetchURL( char *inURL, std::map< std::string, std::string > *inVariables, char **outData, int *outSize ) {
	int result = 0;
	brURLWriteData writeData;

	curl_easy_setopt( _curl, CURLOPT_URL, inURL );
    curl_easy_setopt( _curl, CURLOPT_WRITEFUNCTION, brURLWrite );
    curl_easy_setopt( _curl, CURLOPT_WRITEDATA, &writeData );

	CURLcode cresult = curl_easy_perform( _curl );

	if( inVariables ) {
		
	}

	if( cresult == CURLE_OK ) {
		double size;

		curl_easy_getinfo( _curl, CURLINFO_SIZE_DOWNLOAD, &size );
		*outSize = (int)size;

		curl_easy_getinfo( _curl, CURLINFO_SIZE_DOWNLOAD, &size );

		*outSize = writeData._size;
		*outData = slStrdup( writeData._data );
	} else {
		*outData = NULL;
		*outSize = 0;

		result = -1;
	}

	return result;
}

int brURLFetcher::get( char *inURL, char **outData, int *outSize ) {
	curl_easy_reset( _curl );
    curl_easy_setopt( _curl, CURLOPT_HTTPGET, 1 );

	return fetchURL( inURL, NULL, outData, outSize );
}

int brURLFetcher::put( char *inURL, char *inPutData, int inPutSize, char **outData, int *outSize ) {
	brURLReadData readData( inPutData, inPutSize );

	curl_easy_reset( _curl );
    curl_easy_setopt( _curl, CURLOPT_UPLOAD, 1 );
    curl_easy_setopt( _curl, CURLOPT_INFILESIZE, inPutSize );
    curl_easy_setopt( _curl, CURLOPT_READFUNCTION, brURLRead );
    curl_easy_setopt( _curl, CURLOPT_READDATA, &readData );

	return fetchURL( inURL, NULL, outData, outSize );
}


#endif /* HAVE_LIBCURL */
