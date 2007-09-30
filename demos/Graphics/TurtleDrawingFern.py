
# Note: this file was automatically converted to Python from the
# original steve-language source code.  Please see the original
# file for more detailed comments and documentation.


import breve

class myController( breve.Control ):
	def __init__( self ):
		breve.Control.__init__( self )
		self.drawing = None
		myController.init( self )

	def fern( self, s ):
		if ( s >= 0.400000 ):
			self.drawing.pushState()
			self.drawing.setColor( breve.vector( breve.randomExpression( 1.000000 ), breve.randomExpression( 1.000000 ), breve.randomExpression( 1.000000 ) ) )
			self.drawing.turnUp( 5 )
			self.drawing.forward( ( s / 25 ) )
			self.drawing.turnLeft( 88 )
			self.fern( ( s * 0.300000 ) )
			self.drawing.turnRight( 82 )
			self.drawing.forward( ( s / 25 ) )
			self.drawing.turnRight( 98 )
			self.fern( ( s * 0.300000 ) )
			self.drawing.turnLeft( 79 )
			self.fern( ( s * 0.900000 ) )
			self.drawing.popState()



	def init( self ):
		self.setBackgroundColor( breve.vector( 1, 1, 1 ) )
		self.drawing = breve.createInstances( breve.TurtleDrawing, 1 )
		self.drawing.move( breve.vector( 0, -5, 0 ) )
		self.drawing.setLineWidth( 2.000000 )
		self.fern( 30 )


breve.myController = myController


# Create an instance of our controller object to initialize the simulation

myController()


