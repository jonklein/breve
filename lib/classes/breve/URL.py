
# Note: this file was automatically converted to Python from the
# original steve-language source code.  Please see the original
# file for more detailed comments and documentation.


import breve

class URL( breve.Abstract ):
	'''URL is a class used for sending and receiving data with URLs.  To fetch data from an http:// of ftp:// URL, use METHOD(get).  To put data to  a web server via the HTTP PUT protocol, use METHOD(put).'''

	def __init__( self ):
		breve.Abstract.__init__( self )

	def get( self, urlString ):
		'''Fetches a URL using the HTTP GET protocol.  Returns any data sent from the server, or an empty string if there was no data or an error occurred.'''

		return breve.breveInternalFunctionFinder.brIURLGet( self, urlString )

	def put( self, dataString, urlString ):
		'''Puts data to a URL using the HTTP PUT protocol.  Returns any data sent from the server, or an empty string if there was no data or an error occurred.'''

		return breve.breveInternalFunctionFinder.brIURLPut( self, urlString, dataString )


breve.URL = URL



