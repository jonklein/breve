
import breve

class StarDust( breve.Control ):
	def __init__( self ):
		breve.Control.__init__( self )
		StarDust.init( self )

	def init( self ):
		lightmap = None
		star = None

		self.enableBlur()
		self.setBlurFactor( 0.200000 )
		self.setBackgroundColor( breve.vector( 0, 0, 0 ) )
		star = breve.createInstances( breve.Image, 1 ).load( 'images/star.png' )
		lightmap = breve.createInstances( breve.Image, 1 ).load( 'images/lightmap.png' )
		breve.createInstances( breve.Particles, 10 ).setBitmapImage( star )
		breve.createInstances( breve.Particles, 20 ).setLightmapImage( lightmap )
		self.pointCamera( breve.vector( 0, 0, 0 ), breve.vector( 20.000000, 2.100000, 0.000000 ) )

	def iterate( self ):

		self.pivotCamera( 0.001000, 0.002000 )
		breve.Control.iterate( self )


breve.StarDust = StarDust
class Particle( breve.Mobile ):
	def __init__( self ):
		breve.Mobile.__init__( self )
		self.offset = 0
		self.rate = 0
		self.rotation = 0
		self.shape = None
		Particle.init( self )

	def init( self ):

		self.rate = ( breve.randomExpression( 0.080000 ) - 0.040000 )
		self.offset = breve.randomExpression( 6.280000 )
		self.rotation = breve.randomExpression( 360 )
		self.shape = breve.createInstances( breve.Sphere, 1 ).initWith( ( breve.randomExpression( 5.000000 ) + 0.100000 ) )
		self.setShape( self.shape )
		self.setColor( breve.randomExpression( breve.vector( 1, 1, 1 ) ) )
		self.move( ( breve.randomExpression( breve.vector( 10, 10, 10 ) ) - breve.vector( 5, 5, 5 ) ) )

	def iterate( self ):

		self.rotation = ( self.rotation + self.rate )
		self.setBitmapHeading( self.rotation )
		self.setBitmapTransparency( ( ( breve.breveInternalFunctionFinder.sin( self, ( ( self.controller.getTime() / 30.000000 ) + self.offset ) ) + 1.000000 ) / 2.000000 ) )


breve.Particle = Particle
# Add our newly created classes to the breve namespace

breve.Particles = Particle


# Create an instance of our controller object to initialize the simulation

StarDust()


