
import breve

class myController( breve.Control ):
	'''The controller object is created when the simulation starts.  All initialization code goes here.'''

	__slots__ = [  ]

	def __init__( self ):
		breve.Control.__init__( self )
		myController.init( self )

	def init( self ):
		''''''


		breve.createInstances( breve.myAgents, 40 )
		breve.createInstances( breve.Floor, 1 ).setSize( breve.vector( 50, 0.100000, 50 ) )
		self.pointCamera( breve.vector( 0, 0, 0 ), breve.vector( 0, 40, 80 ) )

	def internalUserInputMethod( self ):
		''''''



	def iterate( self ):
		''''''


		breve.Control.iterate( self )


breve.myController = myController
class myAgent( breve.Mobile ):
	''''''

	__slots__ = [  ]

	def __init__( self ):
		breve.Mobile.__init__( self )
		myAgent.init( self )

	def init( self ):
		''''''


		self.move( ( breve.randomExpression( breve.vector( 50, 0, 50 ) ) - breve.vector( 25, 0, 25 ) ) )
		self.setVelocity( ( breve.randomExpression( breve.vector( 2, 0, 2 ) ) - breve.vector( 1, 0, 1 ) ) )
		self.setColor( breve.randomExpression( breve.vector( 1, 1, 1 ) ) )

	def iterate( self ):
		''''''

		velocity = breve.vector()
		position = breve.vector()

		position = self.getLocation()
		velocity = self.getVelocity()
		if ( ( ( position.x > 25 ) and ( velocity.x > 0 ) ) or ( ( position.x < -25 ) and ( velocity.x < 0 ) ) ):
			self.setVelocity( breve.vector( ( -velocity.x ), 0, velocity.z ) )

		if ( ( ( position.z > 25 ) and ( velocity.z > 0 ) ) or ( ( position.z < -25 ) and ( velocity.z < 0 ) ) ):
			self.setVelocity( breve.vector( velocity.x, 0, ( -velocity.z ) ) )

		breve.Mobile.iterate( self )


breve.myAgent = myAgent
breve.myAgents = myAgent


myController()


