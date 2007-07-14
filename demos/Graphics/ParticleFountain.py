
import breve

class Fountain( breve.Control ):
	def __init__( self ):
		breve.Control.__init__( self )
		self.lightmap = None
		self.particles = breve.objectList()
		Fountain.init( self )

	def getLightmapImage( self ):

		return self.lightmap

	def init( self ):

		self.setIntegrationStep( 0.050000 )
		self.setIterationStep( 0.100000 )
		self.setBackgroundColor( breve.vector( 0, 0, 0 ) )
		self.setBlurFactor( 0.650000 )
		self.disableText()
		self.enableBlur()
		self.lightmap = breve.createInstances( breve.Image, 1 ).load( 'images/dirtylightmap.png' )
		self.particles = breve.createInstances( breve.Particles, 400 )
		self.pointCamera( breve.vector( 0, 9, 0 ), breve.vector( 80.000000, 2.100000, 0.000000 ) )

	def iterate( self ):

		self.pivotCamera( 0.001000, 0.001000 )
		breve.Control.iterate( self )


breve.Fountain = Fountain
class Particle( breve.Mobile ):
	def __init__( self ):
		breve.Mobile.__init__( self )
		self.range = 0
		self.shape = None
		Particle.init( self )

	def init( self ):

		self.shape = breve.createInstances( breve.Sphere, 1 ).initWith( ( breve.randomExpression( 2.000000 ) + 0.100000 ) )
		self.setShape( self.shape )
		self.drawAsPoint()
		self.range = 5
		self.reset()

	def iterate( self ):

		if ( self.getLocation().y < -16.000000 ):
			self.reset()


	def reset( self ):

		self.setColor( breve.randomExpression( breve.vector( 1, 1, 1 ) ) )
		self.move( breve.vector( 0, 0, 0 ) )
		self.setAcceleration( ( breve.randomExpression( breve.vector( 4, -15, 4 ) ) - breve.vector( 2, 1, 2 ) ) )
		self.setVelocity( ( breve.randomExpression( breve.vector( ( 2 * self.range ), 20, ( 2 * self.range ) ) ) + breve.vector( ( -self.range ), 4, ( -self.range ) ) ) )

	def setRange( self, n ):

		self.range = n


breve.Particle = Particle
# Add our newly created classes to the breve namespace

breve.Particles = Particle


# Create an instance of our controller object to initialize the simulation

Fountain()


