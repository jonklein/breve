
import breve

class myController( breve.Control ):
	def __init__( self ):
		breve.Control.__init__( self )
		self.sound = None
		myController.init( self )

	def init( self ):

		self.sound = breve.createInstances( breve.Sound, 1 )
		self.sound.load( 'sounds/cat.wav' )

	def iterate( self ):

		if ( breve.randomExpression( 1000 ) == 0 ):
			self.sound.play( ( 0.800000 + breve.randomExpression( 1.200000 ) ) )

		breve.Control.iterate( self )


breve.myController = myController


# Create an instance of our controller object to initialize the simulation

myController()


