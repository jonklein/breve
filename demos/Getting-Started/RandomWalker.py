
# Note: this file was automatically converted to Python from the
# original steve-language source code.  Please see the original
# file for more detailed comments and documentation.


import breve

class myControl( breve.Control ):
	def __init__( self ):
		breve.Control.__init__( self )
		self.walkerShape = None
		myControl.init( self )

	def getWalkerShape( self ):
		return self.walkerShape

	def init( self ):
		print '''Setting up the simulation.'''
		self.pointCamera( breve.vector( 0, 0, 0 ), breve.vector( 0, 60, 0 ) )
		self.walkerShape = breve.createInstances( breve.Sphere, 1 ).initWith( 1 )
		breve.createInstances( breve.RandomWalker, 200 )


breve.myControl = myControl
class RandomWalker( breve.Mobile ):
	def __init__( self ):
		breve.Mobile.__init__( self )
		RandomWalker.init( self )

	def init( self ):
		self.setShape( self.controller.getWalkerShape() )
		self.setColor( breve.randomExpression( breve.vector( 1.000000, 1.000000, 1.000000 ) ) )
		self.move( breve.randomExpression( breve.vector( 0.100000, 0.100000, 0.100000 ) ) )

	def iterate( self ):
		self.setVelocity( ( breve.randomExpression( breve.vector( 60, 60, 60 ) ) - breve.vector( 30, 30, 30 ) ) )


breve.RandomWalker = RandomWalker


# Create an instance of our controller object to initialize the simulation

myControl()


