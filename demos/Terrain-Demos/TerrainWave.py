
# Note: this file was automatically converted to Python from the
# original steve-language source code.  Please see the original
# file for more detailed comments and documentation.


import breve

class TerrainWave( breve.PhysicalControl ):
	def __init__( self ):
		breve.PhysicalControl.__init__( self )
		self.terrain = None
		TerrainWave.init( self )

	def dropBall( self ):
		breve.createInstances( breve.Ball, 1 )

	def init( self ):
		y = 0
		x = 0

		self.setBackgroundTextureImage( breve.createInstances( breve.Image, 1 ).load( 'images/clouds.png' ) )
		self.enableShadowVolumes()
		self.terrain = breve.createInstances( breve.Terrain, 1 )
		x = 0
		while ( x < 100 ):
			y = 0
			while ( y < 100 ):
				self.terrain.setHeight( x, y, ( ( 6 * breve.breveInternalFunctionFinder.sin( self, ( x / 4.000000 ) ) ) * breve.breveInternalFunctionFinder.cos( self, ( y / 4.000000 ) ) ) )

				y = ( y + 1 )


			x = ( x + 1 )

		self.pointCamera( breve.vector( 0, 0, 0 ), breve.vector( 19, 30, 34 ) )
		self.moveLight( breve.vector( 0, 30, 0 ) )

	def iterate( self ):
		if ( breve.randomExpression( 50 ) == 0 ):
			self.dropBall()

		breve.PhysicalControl.iterate( self )


breve.TerrainWave = TerrainWave
class Ball( breve.Mobile ):
	def __init__( self ):
		breve.Mobile.__init__( self )
		Ball.init( self )

	def init( self ):
		self.setShape( breve.createInstances( breve.Sphere, 1 ).initWith( breve.randomExpression( 1.000000 ) ) )
		self.setColor( breve.randomExpression( breve.vector( 1, 1, 1 ) ) )
		self.move( ( breve.randomExpression( breve.vector( 10, 0, 10 ) ) + breve.vector( -5, 15, -5 ) ) )
		self.enablePhysics()

	def iterate( self ):
		if ( self.getAge() > 15 ):
			breve.deleteInstances( self )



breve.Ball = Ball


# Create an instance of our controller object to initialize the simulation

TerrainWave()


