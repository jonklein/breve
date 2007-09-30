
# Note: this file was automatically converted to Python from the
# original steve-language source code.  Please see the original
# file for more detailed comments and documentation.


import breve

class myController( breve.Control ):
	def __init__( self ):
		breve.Control.__init__( self )
		self.myPythonObject = None
		myController.init( self )

	def init( self ):
		self.myPythonObject = breve.createInstances( breve.PythonBridgeObject, 1 )
		print self.myPythonObject.testVector( breve.vector( 1, 1, 1 ) )


breve.myController = myController


# Create an instance of our controller object to initialize the simulation

myController()


