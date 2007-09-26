
import breve

class myController( breve.Control ):
	def __init__( self ):
		breve.Control.__init__( self )
		self.myPythonObject = None
		myController.init( self )

	def init( self ):

		breve.createInstances( breve.PythonBridgeObject, 1 )
		breve.createInstances( breve.PythonBridgeObject, 1 ).archiveAsXml( 'python.xml' )


breve.myController = myController


# Create an instance of our controller object to initialize the simulation

myController()


