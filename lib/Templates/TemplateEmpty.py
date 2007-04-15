
import breve

class myController( breve.Control ):
	'''The controller object is created when the simulation starts.  All initialization code goes here.'''

	__slots__ = [  ]

	def __init__( self ):
		breve.Control.__init__( self )
		myController.init( self )

	def init( self ):
		''''''


		breve.createInstances( breve.Floor, 1 ).move( breve.vector( 0, -10, 0 ) )
		print '''simulation started'''

	def internalUserInputMethod( self ):
		''''''



	def iterate( self ):
		''''''


		breve.Control.iterate( self )


breve.myController = myController
class myAgent( breve.Mobile ):
	'''myAgent is an empty template for agents in the simulation'''

	__slots__ = [  ]

	def __init__( self ):
		breve.Mobile.__init__( self )
		myAgent.init( self )

	def init( self ):
		''''''



	def iterate( self ):
		''''''




breve.myAgent = myAgent
breve.myAgents = myAgent


myController()


