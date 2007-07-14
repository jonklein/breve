
import breve

class Tunnel( breve.Control ):
	def __init__( self ):
		breve.Control.__init__( self )
		self.lightmap = None
		self.particles = breve.objectList()
		Tunnel.init( self )

	def getLightmap( self ):

		return self.lightmap

	def init( self ):

		self.setIntegrationStep( 0.100000 )
		self.setIterationStep( 0.200000 )
		self.setBackgroundColor( breve.vector( 0, 0, 0 ) )
		self.setZClip( 1000 )
		self.disableText()
		self.lightmap = breve.createInstances( breve.Image, 1 ).load( 'images/lightmap.png' )
		self.particles = breve.createInstances( breve.Particles, 700 )
		self.pointCamera( breve.vector( 0, 0, 0 ), breve.vector( 0.000000, 0.000000, 100.000000 ) )


breve.Tunnel = Tunnel
class Particle( breve.Mobile ):
	def __init__( self ):
		breve.Mobile.__init__( self )
		self.shape = None
		Particle.init( self )

	def init( self ):

		self.shape = breve.createInstances( breve.Sphere, 1 ).initWith( ( 0.300000 + breve.randomExpression( 1.000000 ) ) )
		self.setShape( self.shape )
		self.reset()
		self.setLightmapImage( self.controller.getLightmap() )

	def iterate( self ):

		if ( self.getLocation().z > 100.000000 ):
			self.reset()


	def reset( self ):

		self.setColor( breve.randomExpression( breve.vector( 1, 1, 1 ) ) )
		self.move( ( breve.randomExpression( breve.vector( 600, 600, 0 ) ) - breve.vector( 300, 300, ( 300 + breve.randomExpression( 1000 ) ) ) ) )
		self.setVelocity( breve.vector( 0, 0, 200 ) )


breve.Particle = Particle
# Add our newly created classes to the breve namespace

breve.Particles = Particle


# Create an instance of our controller object to initialize the simulation

Tunnel()


