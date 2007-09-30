
# Note: this file was automatically converted to Python from the
# original steve-language source code.  Please see the original
# file for more detailed comments and documentation.


import breve

class NetworkServer( breve.Abstract ):
	'''NetworkServer sets up a breve simulation to listen on a port to accept connections from other breve engines or from web  browsers.  To start a NetworkServer, use the method  METHOD(listen). <p> There are two ways to use the NetworkServer object, and a single instance can simultaneously handle both kinds of functionality. <ul> <li> breve-to-breve object transfer <li> web-based control of breve simulations </ul> <p> <b>breve-to-breve object transfer</b>: once the network server is running, users can send objects from one breve simulation to another using the OBJECT(Object) method  OBJECTMETHOD(Object:send-over-network).  The instance is  automatically added to the server's simulation.  In addition, the server's controller object (or any other desired object--see the  method METHOD(set-upload-recipient)) will receive a call to the method  "accept-upload".  This method must be  implemented by the user with the following prototype: <pre>+ to accept-upload of-instance i (object) from-host h (string):</pre> <P> Determining how an object is to be sent over the network is a bit complicated and is determined by the object's "dependencies".  These  are described in detail in the documentation dealing with archiving and dearchiving objects. <p> <b>web-based control of breve simulations</b>: the NetworkServer object can also accept connections from regular web-browsers.  To  get the address that clients should connect to, use the method  METHOD(get-url).  When users connect to the server, they can either execute a method in the controller instance or request an HTML file. <P> To execute a method in the controller instance, simply append the  method name to the end of the URL.  If you wish to pass in either int or double arguments, they can be added afterwards, delimited by underscores.  Other types may currently not be passed via the web interface.  Here are some examples: <br><tt>http://myserver:33333/turn-agent-blue</tt> <br><tt>http://myserver:33333/set-agent-color_.2_.4_.6</tt> <P> If the text at the end of the URL contains the string "html", it  is presumed to be a request for an HTML file.  If the file is found, it is sent to the web-browser.  By writing your own HTML files, and  including your own breve-compatible links (described above), you can provide novel interfaces to users. <P> If a method is executed, it may return a string which, depending on  the presence of the string "html", is either interpreted as raw  HTML output, or as a filename that should be served to the browser. So when the user clicks on a link to execute a command, they can be directed to an HTML file of your choosing. <P> Credit for implementing a great deal of the networking functionality is due to Seth Raphael, Poornima Muralidhar and William Taysom.'''

	def __init__( self ):
		breve.Abstract.__init__( self )
		self.serverData = None

	def destroy( self ):
		if self.serverData:
			breve.breveInternalFunctionFinder.closeServer( self, self.serverData )


	def getUrl( self ):
		'''Returns the URL for this server.'''

		return breve.breveInternalFunctionFinder.getServerURL( self, self.serverData )

	def listen( self, portNumber ):
		'''Listens for connections from other breve engines on port portNumber.  portNumber must be greater than 1024, but  otherwise can be any unused part that the client and server breve users agree on.'''

		self.serverData = breve.breveInternalFunctionFinder.listenOnPort( self, portNumber )
		if ( not self.serverData ):
			print '''error: network error for %s listening on port %s''' % (  self, portNumber )
			return 0


		return self

	def setIndexPage( self, indexPage ):
		'''Sets the index page to indexPage.  The indexPage is what's  given when a user connects to a running breve simulation without specifying a specific page or command.'''

		breve.breveInternalFunctionFinder.setIndexPage( self, self.serverData, indexPage )

	def setUploadRecipient( self, newRecipient ):
		'''Sets the object which will receive the "accept-upload" message. The default upload recipient is the controller.'''

		breve.breveInternalFunctionFinder.networkSetRecipient( self, self.serverData, newRecipient )


breve.NetworkServer = NetworkServer
# Add our newly created classes to the breve namespace

breve.NetworkServers = NetworkServer



