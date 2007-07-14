
import breve

class myController( breve.Control ):
	def __init__( self ):
		breve.Control.__init__( self )
		myController.init( self )

	def init( self ):

		breve.createInstances( breve.CustomShapeAgent, 1 )
		self.enableLighting()


breve.myController = myController
class CustomShapeAgent( breve.Mobile ):
	def __init__( self ):
		breve.Mobile.__init__( self )
		CustomShapeAgent.init( self )

	def init( self ):

		self.setShape( breve.createInstances( breve.myCustomShape, 1 ) )
		self.setColor( breve.randomExpression( breve.vector( 1, 1, 1 ) ) )
		self.setRotationalVelocity( breve.vector( 0.010000, 0.050000, 0.010000 ) )
		self.showBoundingBox()


breve.CustomShapeAgent = CustomShapeAgent
class myCustomShape( breve.CustomShape ):
	def __init__( self ):
		breve.CustomShape.__init__( self )
		self.vertices = breve.objectList()
		myCustomShape.init( self )

	def init( self ):

		self.vertices[ 0 ] = breve.vector( -1, 0, 0 )
		self.vertices[ 1 ] = breve.vector( 1.400000, 0, 1.400000 )
		self.vertices[ 2 ] = breve.vector( 1.400000, 0, -1.400000 )
		self.vertices[ 3 ] = breve.vector( 0, 1.400000, 0 )
		self.vertices[ 4 ] = breve.vector( 0, -1.400000, 0 )
		self.addFace( [ self.vertices[ 0 ], self.vertices[ 1 ], self.vertices[ 3 ] ] )
		self.addFace( [ self.vertices[ 1 ], self.vertices[ 2 ], self.vertices[ 3 ] ] )
		self.addFace( [ self.vertices[ 0 ], self.vertices[ 2 ], self.vertices[ 3 ] ] )
		self.addFace( [ self.vertices[ 0 ], self.vertices[ 1 ], self.vertices[ 4 ] ] )
		self.addFace( [ self.vertices[ 1 ], self.vertices[ 2 ], self.vertices[ 4 ] ] )
		self.addFace( [ self.vertices[ 0 ], self.vertices[ 2 ], self.vertices[ 4 ] ] )
		self.finishShape( 1.000000 )


breve.myCustomShape = myCustomShape


# Create an instance of our controller object to initialize the simulation

myController()


