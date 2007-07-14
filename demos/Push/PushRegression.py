
import breve

class RegressionController( breve.Control ):
	def __init__( self ):
		breve.Control.__init__( self )
		RegressionController.init( self )

	def init( self ):

		breve.createInstances( breve.RegressionGP, 1 )


breve.RegressionController = RegressionController
class RegressionGP( breve.PushGP ):
	def __init__( self ):
		breve.PushGP.__init__( self )
		self.currentInput = 0
		RegressionGP.init( self )

	def computeError( self, n, interpreter ):
		desiredResult = 0

		n = ( n + 1 )
		desiredResult = ( ( ( ( ( 8 * n ) * n ) * n ) + ( ( 3 * n ) * n ) ) + n )
		return ( breve.length( ( interpreter.getFloatStackTop() - desiredResult ) ) * breve.length( ( interpreter.getFloatStackTop() - desiredResult ) ) )

	def countFitnessCases( self ):

		return 10

	def init( self ):

		self.readInterpreterConfig( 'PushRegression.config' )
		self.setPopulationSize( 2000 )
		self.setSpatialRadius( 10 )
		self.readInterpreterConfig( 'PushRegression.config' )
		self.interpreter.printConfig()
		self.getInterpreter().addInstruction( self, 'input' )

	def input( self ):

		self.getInterpreter().pushFloat( self.currentInput )

	def setupFitnessTest( self, n, interpreter ):

		n = ( n + 1 )
		self.currentInput = n
		interpreter.pushFloat( n )


breve.RegressionGP = RegressionGP


# Create an instance of our controller object to initialize the simulation

RegressionController()


