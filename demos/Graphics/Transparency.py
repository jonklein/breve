
# Note: this file was automatically converted to Python from the
# original steve-language source code.  Please see the original
# file for more detailed comments and documentation.


import breve

class TransparencyExample( breve.Control ):
	def __init__( self ):
		breve.Control.__init__( self )
		TransparencyExample.init( self )

	def init( self ):
		breve.createInstances( breve.TransparentCube, 1 )
		self.setBackgroundColor( breve.vector( 0, 0, 0 ) )
		self.enableLighting()
		self.moveLight( breve.vector( 0, 20, 0 ) )


breve.TransparencyExample = TransparencyExample
class TransparentCube( breve.Mobile ):
	def __init__( self ):
		breve.Mobile.__init__( self )
		TransparentCube.init( self )

	def init( self ):
		self.setShape( breve.createInstances( breve.Cube, 1 ).initWith( breve.vector( 8, 8, 8 ) ) )
		self.setTextureImage( breve.createInstances( breve.Image, 1 ).load( 'images/water.png' ) )
		self.setTextureScale( 8 )
		self.setTransparency( 0.800000 )
		self.setRotationalVelocity( breve.vector( 0.010000, 0.020000, 0.050000 ) )

	def iterate( self ):
		t = 0

		t = ( self.controller.getTime() / 100.000000 )
		self.setColor( breve.vector( breve.breveInternalFunctionFinder.sin( self, t ), breve.breveInternalFunctionFinder.cos( self, t ), 1.000000 ) )


breve.TransparentCube = TransparentCube


# Create an instance of our controller object to initialize the simulation

TransparencyExample()


