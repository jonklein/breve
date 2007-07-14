
import breve

class myController( breve.Control ):
	def __init__( self ):
		breve.Control.__init__( self )
		myController.init( self )

	def init( self ):

		breve.createInstances( breve.PushTest, 1 ).go()


breve.myController = myController
class PushTest( breve.Object ):
	def __init__( self ):
		breve.Object.__init__( self )
		self.code = None
		self.code2 = None
		self.interpreter = None
		PushTest.init( self )

	def callback( self ):

		print '''%s is calling the breve callback function, pushing 7, ''' % (  self )
		self.interpreter.pushInteger( 7 )
		self.interpreter.printStacks()

	def callback2( self ):

		print '''%s is calling the breve callback2 function, pushing 8, ''' % (  self )
		self.interpreter.pushInteger( 8 )
		self.interpreter.printStacks()

	def go( self ):

		self.interpreter.pushInteger( 100 )
		self.interpreter.clearStacks()
		self.code = breve.createInstances( breve.PushProgram, 1 ).parse( '''( 0 0 INTEGER./ )''' )
		self.code2 = breve.createInstances( breve.PushProgram, 1 ).parse( '''( 1 ( 3 ) ( 4 5 6 ) 8 4 CALLBACK-TEST CALLBACK2-TEST ( 1 ) ( INTEGER.* ) )''' )
		self.testSetFromList()
		print 'Discrepancy', self.code.getDiscrepancy( self.code2 )
		print '''Top level diff''', self.code.getTopLevelDifference( self.code2 )
		print self.code2.getString()
		self.interpreter.run( self.code2 )
		print '''Effort = ''', self.interpreter.getEffort()
		self.interpreter.printStacks()
		self.interpreter.printConfig()
		self.interpreter.clearStacks()
		print self.interpreter.getIntegerStackTop()
		breve.deleteInstances( self.interpreter )
		self.controller.endSimulation()

	def init( self ):

		self.interpreter = breve.createInstances( breve.PushInterpreter, 1 )
		self.interpreter.clearStacks()
		self.interpreter.readConfig( 'PushTest.config' )
		self.interpreter.addInstruction( 'callback2', self, 'CALLBACKTEST2' )
		self.interpreter.addInstruction( 'callback', self, 'CALLBACKTEST' )

	def testSetFromList( self ):

		self.code = breve.createInstances( breve.PushProgram, 1 ).parse( '''( INTEGER./ 1 1 1 1 1 1 1 1)''' )
		self.code2.setFrom( [ 'X', self.code, 'Y', self.code, 'Z', self.code ] )
		self.code.mutate( self.interpreter )
		print '''c2: ''', self.code2.getString()
		print '''c1: ''', self.code.getString()


breve.PushTest = PushTest


# Create an instance of our controller object to initialize the simulation

myController()


