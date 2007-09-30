
# Note: this file was automatically converted to Python from the
# original steve-language source code.  Please see the original
# file for more detailed comments and documentation.


import breve

class myController( breve.Control ):
	def __init__( self ):
		breve.Control.__init__( self )
		myController.init( self )

	def init( self ):
		brush = None

		self.disableText()
		self.setBackgroundColor( breve.vector( 0, 0, 0 ) )
		self.enableBlur()
		self.setBlurFactor( 1.000000 )
		brush = breve.createInstances( breve.Image, 1 ).load( 'images/brush.png' )
		breve.createInstances( breve.Painter, 100 ).setBitmapImage( brush )
		self.offsetCamera( breve.vector( 0, 0, ( 2 * 2.000000 ) ) )


breve.myController = myController
class Painter( breve.Mobile ):
	def __init__( self ):
		breve.Mobile.__init__( self )
		Painter.init( self )

	def getMandelbrotColor( self, x, y ):
		y2 = 0
		x2 = 0
		y0 = 0
		x0 = 0
		maxI = 0
		i = 0
		color = breve.vector()

		x0 = x
		y0 = y
		x2 = ( x * x )
		y2 = ( y * y )
		i = 0
		maxI = 128
		while ( ( ( x2 + y2 ) < 4 ) and ( i < maxI ) ):
			y = ( ( ( 2 * x ) * y ) + y0 )
			x = ( ( x2 - y2 ) + x0 )
			x2 = ( x * x )
			y2 = ( y * y )
			i = ( i + 1 )


		if ( i == maxI ):
			color = breve.vector( 0, 0, 0 )
		else:
			color = breve.vector( ( ( i / 8.000000 ) % 1.000000 ), ( ( i / 16.000000 ) % 1.000000 ), ( ( i / 32.000000 ) % 1.000000 ) )

		return color

	def init( self ):
		self.setShape( breve.createInstances( breve.Sphere, 1 ).initWith( 0.010000 ) )
		self.setColor( breve.vector( 0, 0, 0 ) )

	def iterate( self ):
		y = 0
		x = 0
		color = breve.vector()

		x = ( breve.randomExpression( ( 2.000000 * 2.000000 ) ) - 2.000000 )
		y = ( breve.randomExpression( ( 2.000000 * 2.000000 ) ) - 2.000000 )
		self.move( breve.vector( x, y, 0 ) )
		color = self.getMandelbrotColor( x, y )
		self.setColor( color )


breve.Painter = Painter
# Add our newly created classes to the breve namespace

breve.Painters = Painter


# Create an instance of our controller object to initialize the simulation

myController()


