
import breve

class myController( breve.Control ):
	def __init__( self ):
		breve.Control.__init__( self )
		self.myPythonObject = None
		myController.init( self )

	def init( self ):

		self.myPythonObject = breve.createInstances( breve.PythonBridgeObject, 1 )
		print self.myPythonObject.test_number( 10 )
		print self.myPythonObject.test_vector( breve.vector( 1, 2, 3 ) )
		print self.myPythonObject.test_list( [ 1, 2, [ 3, 4, 5 ] ] )
		print self.myPythonObject


breve.myController = myController


# Create an instance of our controller object to initialize the simulation

myController()


