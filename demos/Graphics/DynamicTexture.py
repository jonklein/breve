
# Note: this file was automatically converted to Python from the
# original steve-language source code.  Please see the original
# file for more detailed comments and documentation.


import breve

class DynamicTextureExample( breve.Control ):
	def __init__( self ):
		breve.Control.__init__( self )
		DynamicTextureExample.init( self )

	def init( self ):
		breve.createInstances( breve.DynamicCube, 1 )
		self.setBackgroundColor( breve.vector( 0, 0, 0 ) )
		self.enableLighting()
		self.moveLight( breve.vector( 0, 20, 0 ) )


breve.DynamicTextureExample = DynamicTextureExample
class DynamicCube( breve.Mobile ):
	def __init__( self ):
		breve.Mobile.__init__( self )
		DynamicCube.init( self )

	def init( self ):
		self.setShape( breve.createInstances( breve.Cube, 1 ).initWith( breve.vector( 8, 8, 8 ) ) )
		self.setTextureImage( breve.createInstances( breve.DynamicTexture, 1 ) )
		self.setRotationalVelocity( breve.vector( 0.050000, 0.080000, 0.050000 ) )
		self.setTransparency( 0.900000 )


breve.DynamicCube = DynamicCube
class DynamicTexture( breve.Image ):
	def __init__( self ):
		breve.Image.__init__( self )
		self.n = 0
		DynamicTexture.init( self )

	def init( self ):
		self.initWith( 32, 32 )

	def iterate( self ):
		y = 0
		x = 0

		self.n = ( self.n + 1 )
		if ( self.n % 20 ):
			return

		x = 0
		while ( x < 32 ):
			y = 0
			while ( y < 32 ):
				self.setPixel( ( breve.randomExpression( 1.000000 ) * breve.vector( 1, 1, 1 ) ), x, y )

				y = ( y + 1 )


			x = ( x + 1 )

		breve.Image.iterate( self )


breve.DynamicTexture = DynamicTexture


# Create an instance of our controller object to initialize the simulation

DynamicTextureExample()


