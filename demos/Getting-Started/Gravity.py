
import breve

class Gravity( breve.PhysicalControl ):
	def __init__( self ):
		breve.PhysicalControl.__init__( self )
		self.theBall = None
		Gravity.init( self )

	def init( self ):

		self.setIntegrationStep( 0.000100 )
		breve.createInstances( breve.Step, 1 ).create( breve.vector( -0.500000, 0, 0 ), breve.vector( 1.000000, 0.020000, 1 ) )
		breve.createInstances( breve.Step, 1 ).create( breve.vector( 0.200000, -0.200000, 0 ), breve.vector( 0.200000, 0.020000, 1 ) )
		breve.createInstances( breve.Step, 1 ).create( breve.vector( 0.400000, -0.400000, 0 ), breve.vector( 0.200000, 0.020000, 1 ) )
		breve.createInstances( breve.Step, 1 ).create( breve.vector( 0.600000, -0.600000, 0 ), breve.vector( 0.200000, 0.020000, 1 ) )
		breve.createInstances( breve.Step, 1 ).create( breve.vector( 0.800000, -0.800000, 0 ), breve.vector( 0.200000, 0.020000, 1 ) )
		breve.createInstances( breve.Step, 1 ).create( breve.vector( 1.000000, -1.000000, 0 ), breve.vector( 0.200000, 0.020000, 1 ) )
		breve.createInstances( breve.Step, 1 ).create( breve.vector( 2.000000, -1.200000, 0 ), breve.vector( 2, 0.020000, 1 ) )
		breve.createInstances( breve.Balls, 30 )
		self.pointCamera( breve.vector( 1.000000, -0.800000, -0.600000 ), breve.vector( 3.500000, 1.100000, 5.000000 ) )
		self.enableShadowVolumes()
		self.addMenu( '''Reset Ball''', 'resetBall' )

	def resetBall( self ):

		self.theBall.reset()


breve.Gravity = Gravity
class Step( breve.Stationary ):
	def __init__( self ):
		breve.Stationary.__init__( self )

	def create( self, location, sizeVector ):
		stepShape = None

		stepShape = breve.createInstances( breve.Cube, 1 ).initWith( sizeVector )
		self.setShape( stepShape )
		self.move( location )


breve.Step = Step
class Ball( breve.Mobile ):
	def __init__( self ):
		breve.Mobile.__init__( self )
		Ball.init( self )

	def init( self ):

		self.setShape( breve.createInstances( breve.Sphere, 1 ).initWith( ( 0.050000 + breve.randomExpression( 0.150000 ) ) ) )
		self.enablePhysics()
		self.reset()

	def iterate( self ):

		if ( self.getLocation().y < -2.000000 ):
			self.reset()


	def reset( self ):

		self.setColor( breve.randomExpression( breve.vector( 1, 1, 1 ) ) )
		self.move( ( breve.vector( -0.800000, 0.500000, -0.300000 ) + breve.randomExpression( breve.vector( 0.500000, 0.800000, 0.600000 ) ) ) )
		self.setVelocity( breve.vector( ( 1 + breve.randomExpression( 1.000000 ) ), ( 1.000000 + breve.randomExpression( 1.000000 ) ), 0 ) )


breve.Ball = Ball
# Add our newly created classes to the breve namespace

breve.Balls = Ball


# Create an instance of our controller object to initialize the simulation

Gravity()


