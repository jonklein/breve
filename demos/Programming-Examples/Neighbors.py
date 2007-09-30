
# Note: this file was automatically converted to Python from the
# original steve-language source code.  Please see the original
# file for more detailed comments and documentation.


import breve

class NeighborDemo( breve.Control ):
	def __init__( self ):
		breve.Control.__init__( self )
		NeighborDemo.init( self )

	def click( self, item ):
		neighbors = breve.objectList()
		color = breve.vector()

		if ( not item ):
			return

		neighbors = item.getNeighbors()
		color = breve.randomExpression( breve.vector( 1, 1, 1 ) )
		neighbors.setColor( color )
		breve.Control.click( self , item )

	def init( self ):
		t = breve.objectList()

		t = breve.createInstances( breve.Things, 150 )
		self.pointCamera( breve.vector( 0, 0, 0 ), breve.vector( 0, 0, 50 ) )
		self.setDisplayText( '''Select an object to highlight its neighbors''', -0.950000, -0.900000 )
		self.enableSmoothDrawing()
		self.updateNeighbors()

	def iterate( self ):
		self.updateNeighbors()
		breve.Control.iterate( self )


breve.NeighborDemo = NeighborDemo
class Thing( breve.Mobile ):
	def __init__( self ):
		breve.Mobile.__init__( self )
		Thing.init( self )

	def init( self ):
		self.setShape( breve.createInstances( breve.Sphere, 1 ).initWith( ( 0.300000 + breve.randomExpression( 0.900000 ) ) ) )
		self.setColor( breve.randomExpression( breve.vector( 1.000000, 1.000000, 1.000000 ) ) )
		self.move( ( breve.randomExpression( breve.vector( 30, 30, 30 ) ) - breve.vector( 15, 15, 15 ) ) )
		self.setNeighborhoodSize( 10.000000 )


breve.Thing = Thing
# Add our newly created classes to the breve namespace

breve.Things = Thing


# Create an instance of our controller object to initialize the simulation

NeighborDemo()


