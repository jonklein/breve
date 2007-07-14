
import breve

class DLA( breve.Control ):
	def __init__( self ):
		breve.Control.__init__( self )
		self.maxLocation = 0
		self.sphereShape = None
		DLA.init( self )

	def hit( self, location, c, agent ):
		newParticle = None

		newParticle = breve.createInstances( breve.Stationary, 1 )
		newParticle.register( self.sphereShape, location )
		newParticle.setColor( ( c + ( breve.randomExpression( breve.vector( 0.200000, 0.200000, 0.200000 ) ) - breve.vector( 0.100000, 0.100000, 0.100000 ) ) ) )
		newParticle.setTexture( -1 )
		if ( breve.length( location ) > self.maxLocation ):
			self.maxLocation = breve.length( location )

		agent.setMaximumDistance( self.maxLocation )
		agent.offsetFromCenter( ( self.maxLocation + 2 ) )

	def init( self ):
		firstAgent = None

		firstAgent = breve.createInstances( breve.Diffuser, 1 )
		breve.createInstances( breve.Diffuser, 2 )
		self.maxLocation = 0
		self.disableDrawEveryFrame()
		self.setIntegrationStep( 1.000000 )
		self.setIterationStep( 5.000000 )
		self.setBackgroundColor( breve.vector( 0, 0, 0 ) )
		self.sphereShape = breve.createInstances( breve.Sphere, 1 ).initWith( 1 )
		self.enableLighting()
		self.enableShadowVolumes()
		self.hit( breve.vector( 0, 0, 0 ), breve.vector( 0.800000, 0.800000, 0.800000 ), firstAgent )
		self.pointCamera( breve.vector( 0, 0, 0 ), breve.vector( 0, 0, 60 ) )
		self.setLightColor( breve.vector( 0.200000, 0.200000, 0.500000 ) )
		self.moveLight( breve.vector( 10, 0, 0 ) )


breve.DLA = DLA
class Diffuser( breve.Mobile ):
	def __init__( self ):
		breve.Mobile.__init__( self )
		self.direction = breve.vector()
		self.maxDist = 0
		self.roaming = 0
		self.sphereShape = None
		Diffuser.init( self )

	def collide( self, group ):

		if self.roaming:
			self.getController().hit( self.getLocation(), group.getColor(), self )

		self.roaming = 0

	def init( self ):

		self.handleCollisions( 'Stationary', 'collide' )
		self.move( ( breve.randomExpression( breve.vector( 10, 10, 10 ) ) - breve.vector( 5, 5, 5 ) ) )

	def iterate( self ):

		if ( breve.length( self.getLocation() ) > ( self.maxDist + 10 ) ):
			self.offsetFromCenter( ( self.maxDist + 2 ) )

		self.setVelocity( ( breve.randomExpression( breve.vector( 1.200000, 1.200000, 1.200000 ) ) - breve.vector( 0.600000, 0.600000, 0.600000 ) ) )
		self.roaming = 1

	def offsetFromCenter( self, offset ):

		self.direction = ( breve.randomExpression( breve.vector( 2, 2, 2 ) ) - breve.vector( 1, 1, 1 ) )
		self.direction = ( self.direction / breve.length( self.direction ) )
		self.move( ( self.direction * offset ) )

	def setMaximumDistance( self, max ):

		self.maxDist = max


breve.Diffuser = Diffuser


# Create an instance of our controller object to initialize the simulation

DLA()


