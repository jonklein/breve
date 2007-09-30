
# Note: this file was automatically converted to Python from the
# original steve-language source code.  Please see the original
# file for more detailed comments and documentation.


import breve

class myControl( breve.Control ):
	def __init__( self ):
		breve.Control.__init__( self )
		self.agent = None
		self.agents = breve.objectList()
		self.walkerShape = None
		self.wallShapeTest = None
		self.wallT = None
		myControl.init( self )

	def getWalkerShape( self ):
		return self.walkerShape

	def init( self ):
		s = None

		print '''Setting up the simulation.'''
		s = breve.createInstances( breve.TEFT4300Sensor, 1 )
		s.registerSensorTEFT4300()
		breve.deleteInstances( s )
		self.pointCamera( breve.vector( 0, 0, 0 ), breve.vector( 0, 100, 0 ) )
		self.walkerShape = breve.createInstances( breve.Cube, 1 ).initWith( breve.vector( 2.500000, 2.800000, 2.500000 ) )
		self.sensorTestDistance()

	def iterate( self ):
		breve.Control.iterate( self )
		self.updateNeighbors()

	def messageTestBidirectional( self ):
		agentcount = 0

		agentcount = 3
		self.agents = breve.createInstances( breve.testAgent, agentcount )
		self.agents[ 0 ].move( breve.vector( 0, 1.400000, 0 ) )
		self.agents[ 0 ].rotate( breve.vector( 0, 1, 0 ), ( ( -3.141593 ) / 2.000000 ) )
		self.agents[ 0 ].setMode( 6 )
		self.agents[ 1 ].setMode( 0 )
		self.agents[ 1 ].setMode( 24 )
		self.agents[ 1 ].move( breve.vector( 0, 1.400000, 40 ) )
		self.agents[ 1 ].rotate( breve.vector( 0, 1, 0 ), ( ( -3.141593 ) / 2.000000 ) )
		self.agents[ 1 ].setVelocity( breve.vector( 0, 0, -1 ) )
		self.agents[ 2 ].setMode( 0 )
		self.agents[ 2 ].setMode( 24 )
		self.agents[ 2 ].move( breve.vector( 0, 1.400000, -40 ) )
		self.agents[ 2 ].rotate( breve.vector( 0, 1, 0 ), ( ( 2 * 3.141593 ) / 2.000000 ) )
		self.agents[ 2 ].setVelocity( breve.vector( 0, 0, 1 ) )

	def messageTestAllSensors( self ):
		radius = 0
		i = 0
		agentcount = 0

		agentcount = 30
		radius = 40
		self.agents = breve.createInstances( breve.testAgent, ( agentcount + 1 ) )
		print 'messageTestAllSensors:'
		i = 0
		while ( i < agentcount ):
			self.agents[ i ].move( breve.vector( ( radius * breve.breveInternalFunctionFinder.sin( self, ( ( ( ( 360.000000 / agentcount ) * i ) * 3.141593 ) / 180.000000 ) ) ), 1.400000, ( radius * breve.breveInternalFunctionFinder.cos( self, ( ( ( ( 360.000000 / agentcount ) * i ) * 3.141593 ) / 180.000000 ) ) ) ) )
			self.agents[ i ].rotate( breve.vector( 0, 1, 0 ), ( ( ( ( ( 360.000000 / agentcount ) * i ) * 3.141593 ) / 180.000000 ) + ( 3.141593 / 2.000000 ) ) )
			self.agents[ i ].setMode( 5 )

			i = ( i + 1 )

		self.agents[ agentcount ].move( breve.vector( 0, 1.400000, 0 ) )
		self.agents[ agentcount ].rotate( breve.vector( 0, -1, 0 ), ( 3.141593 / 2.000000 ) )
		self.agents[ agentcount ].setMode( 6 )

	def messageTestBlock( self ):
		radius = 0
		i = 0
		agentcount = 0

		agentcount = 30
		radius = 40
		self.agents = breve.createInstances( breve.testAgent, ( agentcount + 5 ) )
		print ''' messageTest-block:'''
		i = 0
		while ( i < agentcount ):
			self.agents[ i ].move( breve.vector( ( radius * breve.breveInternalFunctionFinder.sin( self, ( ( ( ( 360.000000 / agentcount ) * i ) * 3.141593 ) / 180.000000 ) ) ), 1.400000, ( radius * breve.breveInternalFunctionFinder.cos( self, ( ( ( ( 360.000000 / agentcount ) * i ) * 3.141593 ) / 180.000000 ) ) ) ) )
			self.agents[ i ].rotate( breve.vector( 0, 1, 0 ), ( ( ( ( ( 360.000000 / agentcount ) * i ) * 3.141593 ) / 180.000000 ) + ( 3.141593 / 2.000000 ) ) )
			self.agents[ i ].setMode( 5 )

			i = ( i + 1 )

		self.agents[ agentcount ].move( breve.vector( 0, 1.400000, 0 ) )
		self.agents[ agentcount ].rotate( breve.vector( 0, -1, 0 ), ( 3.141593 / 2.000000 ) )
		self.agents[ agentcount ].setMode( 6 )
		self.agents[ ( agentcount + 1 ) ].setMode( 11 )
		self.agents[ ( agentcount + 1 ) ].setColor( breve.vector( 0.100000, 0.100000, 0.100000 ) )
		self.agents[ ( agentcount + 1 ) ].move( breve.vector( 3, 1.400000, -6 ) )
		self.agents[ ( agentcount + 2 ) ].setMode( 11 )
		self.agents[ ( agentcount + 2 ) ].setColor( breve.vector( 0.100000, 0.100000, 0.100000 ) )
		self.agents[ ( agentcount + 2 ) ].move( breve.vector( 1, 1.400000, -6 ) )
		self.agents[ ( agentcount + 3 ) ].setMode( 11 )
		self.agents[ ( agentcount + 3 ) ].setColor( breve.vector( 0.100000, 0.100000, 0.100000 ) )
		self.agents[ ( agentcount + 3 ) ].move( breve.vector( -1, 1.400000, -6 ) )
		self.agents[ ( agentcount + 4 ) ].setMode( 11 )
		self.agents[ ( agentcount + 4 ) ].setColor( breve.vector( 0.100000, 0.100000, 0.100000 ) )
		self.agents[ ( agentcount + 4 ) ].move( breve.vector( -3, 1.400000, -6 ) )

	def moveAroundTest( self ):
		agentcount = 0
		i = 0

		agentcount = 3
		self.agents = breve.createInstances( breve.testAgent, agentcount )
		breve.createInstances( breve.DemoStage, 1 )
		i = 0
		while ( i < agentcount ):
			self.agents[ i ].move( ( breve.randomExpression( breve.vector( 10, 1.400000, 10 ) ) + breve.vector( -15, 0, -15 ) ) )
			self.agents[ i ].rotate( breve.vector( 0, 1, 0 ), ( ( breve.randomExpression( 360 ) * 3.141593 ) / 360 ) )
			self.agents[ i ].setMode( 9 )

			i = ( i + 1 )


	def sensorTestAzimut( self, dist ):
		self.sensorTestInit()
		print '''sensorTest-Azimut distance:''', dist
		self.wallShapeTest = breve.createInstances( breve.Cube, 1 ).initWith( breve.vector( 20, 10, 1 ) )
		self.wallT = breve.createInstances( breve.Stationary, 1 )
		self.wallT.register( self.wallShapeTest, breve.vector( -40, 5, ( ( 2.500000 + 1 ) / 2 ) ) )
		self.wallT.setE( 0.100000 )
		self.wallT.setColor( breve.vector( 0.000000, 0.900000, 0.100000 ) )
		self.agent.setMode( 21 )
		self.agent.move( breve.vector( -40, 1.400000, ( -dist ) ) )

	def sensorTestDistance( self ):
		self.sensorTestInit()
		print 'sensorTestDistance:'
		self.wallShapeTest = breve.createInstances( breve.Cube, 1 ).initWith( breve.vector( 20, 10, 1 ) )
		self.wallT = breve.createInstances( breve.Stationary, 1 )
		self.wallT.register( self.wallShapeTest, breve.vector( 0, 5, 0 ) )
		self.wallT.setE( 0.100000 )
		self.wallT.setColor( breve.vector( 0.000000, 0.900000, 0.100000 ) )
		self.agent.setMode( 20 )
		self.agent.move( breve.vector( 0, 1.400000, 50 ) )
		self.agent.rotate( breve.vector( 0, 1, 0 ), ( 3.141593 / 2 ) )
		self.agent.setVelocity( ( self.agent.getRotation() * breve.vector( 1, 0, 0 ) ) )

	def sensorTestIncidence( self, dist ):
		print '''sensorTest-Incidence distance:''', dist
		self.sensorTestInit()
		self.wallShapeTest = breve.createInstances( breve.Cube, 1 ).initWith( breve.vector( 10, 10, 1 ) )
		self.wallT = breve.createInstances( breve.Stationary, 1 )
		self.wallT.register( self.wallShapeTest, breve.vector( 0, 5, 0 ) )
		self.wallT.setE( 0.100000 )
		self.wallT.setColor( breve.vector( 0.000000, 0.900000, 0.100000 ) )
		self.agent.setMode( 22 )
		self.agent.setIncidenceRadius( ( dist + ( ( 1 + 2.500000 ) / 2.000000 ) ) )

	def sensorTestInit( self ):
		breve.createInstances( breve.TestStage, 1 )
		self.agent = breve.createInstances( breve.testAgent, 1 )
		self.pointCamera( breve.vector( 0, 0, 0 ), breve.vector( 0, 250, 0 ) )

	def testInsideSensor( self ):
		agentcount = 0

		agentcount = 2
		self.agents = breve.createInstances( breve.testAgent, agentcount )
		self.agents[ 0 ].move( breve.vector( 0, 1.400000, 10 ) )
		self.agents[ 0 ].rotate( breve.vector( 0, 1, 0 ), ( 3.141593 / 2.000000 ) )
		self.agents[ 0 ].setMode( 23 )
		self.agents[ 1 ].setMode( 10 )
		self.agents[ 1 ].move( breve.vector( 0, 1.400000, 40 ) )
		self.agents[ 1 ].rotate( breve.vector( 0, 1, 0 ), ( 3.141593 / 2.000000 ) )


breve.myControl = myControl
class Stage( breve.Object ):
	def __init__( self ):
		breve.Object.__init__( self )
		self.floor = None
		self.floorShape = None
		self.wallShape0 = None
		self.wallShape1 = None
		self.wallN = None
		self.wallS = None

	def initWith( self, FLOOR_X, FLOOR_Z ):
		self.floorShape = breve.createInstances( breve.Cube, 1 ).initWith( breve.vector( FLOOR_X, 0.100000, FLOOR_Z ) )
		self.floor = breve.createInstances( breve.Stationary, 1 )
		self.floor.register( self.floorShape, breve.vector( 0, ( ( -0.100000 ) / 2 ), 0 ) )
		self.floor.setE( 0.100000 )
		self.wallShape0 = breve.createInstances( breve.Cube, 1 ).initWith( breve.vector( 1, 10.000000, ( FLOOR_Z + 1 ) ) )
		self.wallN = breve.createInstances( breve.Stationary, 1 )
		self.wallN.register( self.wallShape0, breve.vector( ( FLOOR_X / 2 ), ( ( 10.000000 / 2 ) - ( 0.100000 / 2 ) ), 0 ) )
		self.wallN.setE( 0.100000 )
		self.wallN.setColor( breve.vector( 0.000000, 0.900000, 0.100000 ) )
		self.wallS = breve.createInstances( breve.Stationary, 1 )
		self.wallS.register( self.wallShape0, breve.vector( ( ( -FLOOR_X ) / 2 ), ( ( 10.000000 / 2 ) - ( 0.100000 / 2 ) ), 0 ) )
		self.wallS.setE( 0.100000 )
		self.wallS.setColor( breve.vector( 0.500000, 0, 0 ) )
		self.wallShape1 = breve.createInstances( breve.Cube, 1 ).initWith( breve.vector( ( FLOOR_X + 1 ), 10.000000, 1 ) )
		self.wallN = breve.createInstances( breve.Stationary, 1 )
		self.wallN.register( self.wallShape1, breve.vector( 0, ( ( 10.000000 / 2 ) - ( 0.100000 / 2 ) ), ( FLOOR_Z / 2 ) ) )
		self.wallN.setE( 0.100000 )
		self.wallN.setColor( breve.vector( 0.500000, 0, 0 ) )
		self.wallS = breve.createInstances( breve.Stationary, 1 )
		self.wallS.register( self.wallShape1, breve.vector( 0, ( ( 10.000000 / 2 ) - ( 0.100000 / 2 ) ), ( ( -FLOOR_Z ) / 2 ) ) )
		self.wallS.setE( 0.100000 )
		self.wallS.setColor( breve.vector( 0.500000, 0, 0 ) )
		return self


breve.Stage = Stage
class LabyStage2( breve.Object ):
	def __init__( self ):
		breve.Object.__init__( self )
		self.floor = None
		self.floorShape = None
		self.wallShape0 = None
		self.wallShape1 = None
		self.wallN = None
		self.wallS = None

	def initWith( self, FLOOR_X, FLOOR_Z ):
		self.floorShape = breve.createInstances( breve.Cube, 1 ).initWith( breve.vector( FLOOR_X, 0.100000, FLOOR_Z ) )
		self.floor = breve.createInstances( breve.Stationary, 1 )
		self.floor.register( self.floorShape, breve.vector( 0, ( ( -0.100000 ) / 2 ), 0 ) )
		self.floor.setE( 0.100000 )
		self.wallShape0 = breve.createInstances( breve.Cube, 1 ).initWith( breve.vector( 20, 10.000000, 1 ) )
		self.wallShape1 = breve.createInstances( breve.Cube, 1 ).initWith( breve.vector( 1, 10.000000, 20 ) )
		self.wallN = breve.createInstances( breve.Stationary, 1 )
		self.wallN.register( self.wallShape0, breve.vector( -20, ( ( 10.000000 / 2 ) - ( 0.100000 / 2 ) ), 0 ) )
		self.wallN.setE( 0.100000 )
		self.wallN.setColor( breve.vector( 0.000000, 0.900000, 0.100000 ) )
		self.wallN = breve.createInstances( breve.Stationary, 1 )
		self.wallN.register( self.wallShape0, breve.vector( 0, ( ( 10.000000 / 2 ) - ( 0.100000 / 2 ) ), -20 ) )
		self.wallN.setE( 0.100000 )
		self.wallN.setColor( breve.vector( 0.000000, 0.900000, 0.100000 ) )
		self.wallN = breve.createInstances( breve.Stationary, 1 )
		self.wallN.register( self.wallShape0, breve.vector( 0, ( ( 10.000000 / 2 ) - ( 0.100000 / 2 ) ), 20 ) )
		self.wallN.setE( 0.100000 )
		self.wallN.setColor( breve.vector( 0.000000, 0.900000, 0.100000 ) )
		self.wallN = breve.createInstances( breve.Stationary, 1 )
		self.wallN.register( self.wallShape0, breve.vector( -20, ( ( 10.000000 / 2 ) - ( 0.100000 / 2 ) ), 40 ) )
		self.wallN.setE( 0.100000 )
		self.wallN.setColor( breve.vector( 0.000000, 0.900000, 0.100000 ) )
		self.wallN = breve.createInstances( breve.Stationary, 1 )
		self.wallN.register( self.wallShape0, breve.vector( 0, ( ( 10.000000 / 2 ) - ( 0.100000 / 2 ) ), 40 ) )
		self.wallN.setE( 0.100000 )
		self.wallN.setColor( breve.vector( 0.000000, 0.900000, 0.100000 ) )
		self.wallN = breve.createInstances( breve.Stationary, 1 )
		self.wallN.register( self.wallShape0, breve.vector( 20, ( ( 10.000000 / 2 ) - ( 0.100000 / 2 ) ), 40 ) )
		self.wallN.setE( 0.100000 )
		self.wallN.setColor( breve.vector( 0.000000, 0.900000, 0.100000 ) )
		self.wallN = breve.createInstances( breve.Stationary, 1 )
		self.wallN.register( self.wallShape0, breve.vector( -20, ( ( 10.000000 / 2 ) - ( 0.100000 / 2 ) ), -40 ) )
		self.wallN.setE( 0.100000 )
		self.wallN.setColor( breve.vector( 0.000000, 0.900000, 0.100000 ) )
		self.wallN = breve.createInstances( breve.Stationary, 1 )
		self.wallN.register( self.wallShape0, breve.vector( 0, ( ( 10.000000 / 2 ) - ( 0.100000 / 2 ) ), -40 ) )
		self.wallN.setE( 0.100000 )
		self.wallN.setColor( breve.vector( 0.000000, 0.900000, 0.100000 ) )
		self.wallN = breve.createInstances( breve.Stationary, 1 )
		self.wallN.register( self.wallShape0, breve.vector( 20, ( ( 10.000000 / 2 ) - ( 0.100000 / 2 ) ), -40 ) )
		self.wallN.setE( 0.100000 )
		self.wallN.setColor( breve.vector( 0.000000, 0.900000, 0.100000 ) )
		self.wallN = breve.createInstances( breve.Stationary, 1 )
		self.wallN.register( self.wallShape1, breve.vector( 30, ( ( 10.000000 / 2 ) - ( 0.100000 / 2 ) ), 30 ) )
		self.wallN.setE( 0.100000 )
		self.wallN.setColor( breve.vector( 0.000000, 0.900000, 0.100000 ) )
		self.wallS = breve.createInstances( breve.Stationary, 1 )
		self.wallS.register( self.wallShape1, breve.vector( 30, ( ( 10.000000 / 2 ) - ( 0.100000 / 2 ) ), 10 ) )
		self.wallS.setE( 0.100000 )
		self.wallS.setColor( breve.vector( 0.000000, 0.900000, 0.100000 ) )
		self.wallN = breve.createInstances( breve.Stationary, 1 )
		self.wallN.register( self.wallShape1, breve.vector( 30, ( ( 10.000000 / 2 ) - ( 0.100000 / 2 ) ), -10 ) )
		self.wallN.setE( 0.100000 )
		self.wallN.setColor( breve.vector( 0.000000, 0.900000, 0.100000 ) )
		self.wallS = breve.createInstances( breve.Stationary, 1 )
		self.wallS.register( self.wallShape1, breve.vector( 30, ( ( 10.000000 / 2 ) - ( 0.100000 / 2 ) ), -30 ) )
		self.wallS.setE( 0.100000 )
		self.wallS.setColor( breve.vector( 0.000000, 0.900000, 0.100000 ) )
		self.wallN = breve.createInstances( breve.Stationary, 1 )
		self.wallN.register( self.wallShape1, breve.vector( -30, ( ( 10.000000 / 2 ) - ( 0.100000 / 2 ) ), 30 ) )
		self.wallN.setE( 0.100000 )
		self.wallN.setColor( breve.vector( 0.000000, 0.900000, 0.100000 ) )
		self.wallS = breve.createInstances( breve.Stationary, 1 )
		self.wallS.register( self.wallShape1, breve.vector( -30, ( ( 10.000000 / 2 ) - ( 0.100000 / 2 ) ), 10 ) )
		self.wallS.setE( 0.100000 )
		self.wallS.setColor( breve.vector( 0.000000, 0.900000, 0.100000 ) )
		self.wallN = breve.createInstances( breve.Stationary, 1 )
		self.wallN.register( self.wallShape1, breve.vector( -30, ( ( 10.000000 / 2 ) - ( 0.100000 / 2 ) ), -10 ) )
		self.wallN.setE( 0.100000 )
		self.wallN.setColor( breve.vector( 0.000000, 0.900000, 0.100000 ) )
		self.wallS = breve.createInstances( breve.Stationary, 1 )
		self.wallS.register( self.wallShape1, breve.vector( -30, ( ( 10.000000 / 2 ) - ( 0.100000 / 2 ) ), -30 ) )
		self.wallS.setE( 0.100000 )
		self.wallS.setColor( breve.vector( 0.000000, 0.900000, 0.100000 ) )
		self.wallS = breve.createInstances( breve.Stationary, 1 )
		self.wallS.register( self.wallShape1, breve.vector( 10, ( ( 10.000000 / 2 ) - ( 0.100000 / 2 ) ), -10 ) )
		self.wallS.setE( 0.100000 )
		self.wallS.setColor( breve.vector( 0.000000, 0.900000, 0.100000 ) )
		self.wallS = breve.createInstances( breve.Stationary, 1 )
		self.wallS.register( self.wallShape1, breve.vector( 10, ( ( 10.000000 / 2 ) - ( 0.100000 / 2 ) ), 10 ) )
		self.wallS.setE( 0.100000 )
		self.wallS.setColor( breve.vector( 0.000000, 0.900000, 0.100000 ) )
		return self


breve.LabyStage2 = LabyStage2
class LabyStage3( breve.Object ):
	def __init__( self ):
		breve.Object.__init__( self )
		self.floor = None
		self.floorShape = None
		self.wallShape0 = None
		self.wallShape1 = None
		self.wallN = None
		self.wallS = None
		LabyStage3.init( self )

	def init( self ):
		FLOOR_X = 0
		FLOOR_Z = 0

		FLOOR_X = 100
		FLOOR_Z = 80
		self.floorShape = breve.createInstances( breve.Cube, 1 ).initWith( breve.vector( FLOOR_X, 0.100000, FLOOR_Z ) )
		self.floor = breve.createInstances( breve.Stationary, 1 )
		self.floor.register( self.floorShape, breve.vector( 0, ( ( -0.100000 ) / 2 ), 0 ) )
		self.floor.setE( 0.100000 )
		self.wallShape0 = breve.createInstances( breve.Cube, 1 ).initWith( breve.vector( 20, 10.000000, 1 ) )
		self.wallShape1 = breve.createInstances( breve.Cube, 1 ).initWith( breve.vector( 1, 10.000000, 20 ) )
		self.wallN = breve.createInstances( breve.Stationary, 1 )
		self.wallN.register( self.wallShape0, breve.vector( -40, ( ( 10.000000 / 2 ) - ( 0.100000 / 2 ) ), -40 ) )
		self.wallN.setE( 0.100000 )
		self.wallN.setColor( breve.vector( 0.000000, 0.900000, 0.100000 ) )
		self.wallN = breve.createInstances( breve.Stationary, 1 )
		self.wallN.register( self.wallShape0, breve.vector( -40, ( ( 10.000000 / 2 ) - ( 0.100000 / 2 ) ), 40 ) )
		self.wallN.setE( 0.100000 )
		self.wallN.setColor( breve.vector( 0.000000, 0.900000, 0.100000 ) )
		self.wallN = breve.createInstances( breve.Stationary, 1 )
		self.wallN.register( self.wallShape0, breve.vector( 40, ( ( 10.000000 / 2 ) - ( 0.100000 / 2 ) ), -40 ) )
		self.wallN.setE( 0.100000 )
		self.wallN.setColor( breve.vector( 0.000000, 0.900000, 0.100000 ) )
		self.wallN = breve.createInstances( breve.Stationary, 1 )
		self.wallN.register( self.wallShape0, breve.vector( 40, ( ( 10.000000 / 2 ) - ( 0.100000 / 2 ) ), 40 ) )
		self.wallN.setE( 0.100000 )
		self.wallN.setColor( breve.vector( 0.000000, 0.900000, 0.100000 ) )
		self.wallN = breve.createInstances( breve.Stationary, 1 )
		self.wallN.register( self.wallShape0, breve.vector( -20, ( ( 10.000000 / 2 ) - ( 0.100000 / 2 ) ), 40 ) )
		self.wallN.setE( 0.100000 )
		self.wallN.setColor( breve.vector( 0.000000, 0.900000, 0.100000 ) )
		self.wallN = breve.createInstances( breve.Stationary, 1 )
		self.wallN.register( self.wallShape0, breve.vector( 0, ( ( 10.000000 / 2 ) - ( 0.100000 / 2 ) ), 40 ) )
		self.wallN.setE( 0.100000 )
		self.wallN.setColor( breve.vector( 0.000000, 0.900000, 0.100000 ) )
		self.wallN = breve.createInstances( breve.Stationary, 1 )
		self.wallN.register( self.wallShape0, breve.vector( 20, ( ( 10.000000 / 2 ) - ( 0.100000 / 2 ) ), 40 ) )
		self.wallN.setE( 0.100000 )
		self.wallN.setColor( breve.vector( 0.000000, 0.900000, 0.100000 ) )
		self.wallN = breve.createInstances( breve.Stationary, 1 )
		self.wallN.register( self.wallShape0, breve.vector( -20, ( ( 10.000000 / 2 ) - ( 0.100000 / 2 ) ), -40 ) )
		self.wallN.setE( 0.100000 )
		self.wallN.setColor( breve.vector( 0.000000, 0.900000, 0.100000 ) )
		self.wallN = breve.createInstances( breve.Stationary, 1 )
		self.wallN.register( self.wallShape0, breve.vector( 0, ( ( 10.000000 / 2 ) - ( 0.100000 / 2 ) ), -40 ) )
		self.wallN.setE( 0.100000 )
		self.wallN.setColor( breve.vector( 0.000000, 0.900000, 0.100000 ) )
		self.wallN = breve.createInstances( breve.Stationary, 1 )
		self.wallN.register( self.wallShape0, breve.vector( 20, ( ( 10.000000 / 2 ) - ( 0.100000 / 2 ) ), -40 ) )
		self.wallN.setE( 0.100000 )
		self.wallN.setColor( breve.vector( 0.000000, 0.900000, 0.100000 ) )
		self.wallS = breve.createInstances( breve.Stationary, 1 )
		self.wallS.register( self.wallShape1, breve.vector( -50, ( ( 10.000000 / 2 ) - ( 0.100000 / 2 ) ), 30 ) )
		self.wallS.setE( 0.100000 )
		self.wallS.setColor( breve.vector( 0.000000, 0.900000, 0.100000 ) )
		self.wallS = breve.createInstances( breve.Stationary, 1 )
		self.wallS.register( self.wallShape1, breve.vector( -50, ( ( 10.000000 / 2 ) - ( 0.100000 / 2 ) ), 10 ) )
		self.wallS.setE( 0.100000 )
		self.wallS.setColor( breve.vector( 0.000000, 0.900000, 0.100000 ) )
		self.wallS = breve.createInstances( breve.Stationary, 1 )
		self.wallS.register( self.wallShape1, breve.vector( -50, ( ( 10.000000 / 2 ) - ( 0.100000 / 2 ) ), -10 ) )
		self.wallS.setE( 0.100000 )
		self.wallS.setColor( breve.vector( 0.000000, 0.900000, 0.100000 ) )
		self.wallS = breve.createInstances( breve.Stationary, 1 )
		self.wallS.register( self.wallShape1, breve.vector( -50, ( ( 10.000000 / 2 ) - ( 0.100000 / 2 ) ), -30 ) )
		self.wallS.setE( 0.100000 )
		self.wallS.setColor( breve.vector( 0.000000, 0.900000, 0.100000 ) )
		self.wallS = breve.createInstances( breve.Stationary, 1 )
		self.wallS.register( self.wallShape1, breve.vector( 50, ( ( 10.000000 / 2 ) - ( 0.100000 / 2 ) ), 30 ) )
		self.wallS.setE( 0.100000 )
		self.wallS.setColor( breve.vector( 0.000000, 0.900000, 0.100000 ) )
		self.wallS = breve.createInstances( breve.Stationary, 1 )
		self.wallS.register( self.wallShape1, breve.vector( 50, ( ( 10.000000 / 2 ) - ( 0.100000 / 2 ) ), 10 ) )
		self.wallS.setE( 0.100000 )
		self.wallS.setColor( breve.vector( 0.000000, 0.900000, 0.100000 ) )
		self.wallS = breve.createInstances( breve.Stationary, 1 )
		self.wallS.register( self.wallShape1, breve.vector( 50, ( ( 10.000000 / 2 ) - ( 0.100000 / 2 ) ), -10 ) )
		self.wallS.setE( 0.100000 )
		self.wallS.setColor( breve.vector( 0.000000, 0.900000, 0.100000 ) )
		self.wallS = breve.createInstances( breve.Stationary, 1 )
		self.wallS.register( self.wallShape1, breve.vector( 50, ( ( 10.000000 / 2 ) - ( 0.100000 / 2 ) ), -30 ) )
		self.wallS.setE( 0.100000 )
		self.wallS.setColor( breve.vector( 0.000000, 0.900000, 0.100000 ) )
		return self


breve.LabyStage3 = LabyStage3
class LabyStage4( breve.Object ):
	def __init__( self ):
		breve.Object.__init__( self )
		self.floor = None
		self.floorShape = None
		self.wallShape0 = None
		self.wallShape1 = None
		self.wallN = None
		self.wallS = None
		LabyStage4.init( self )

	def init( self ):
		FLOOR_X = 0
		FLOOR_Z = 0

		FLOOR_X = 160
		FLOOR_Z = 120
		self.floorShape = breve.createInstances( breve.Cube, 1 ).initWith( breve.vector( FLOOR_X, 0.100000, FLOOR_Z ) )
		self.wallShape0 = breve.createInstances( breve.Cube, 1 ).initWith( breve.vector( 20, 10.000000, 1 ) )
		self.wallShape1 = breve.createInstances( breve.Cube, 1 ).initWith( breve.vector( 1, 10.000000, 20 ) )
		self.wallN = breve.createInstances( breve.Stationary, 1 )
		self.wallN.register( self.wallShape0, breve.vector( -80, ( ( 10.000000 / 2 ) - ( 0.100000 / 2 ) ), -60 ) )
		self.wallN.setE( 0.100000 )
		self.wallN.setColor( breve.vector( 0.000000, 0.900000, 0.100000 ) )
		self.wallN = breve.createInstances( breve.Stationary, 1 )
		self.wallN.register( self.wallShape0, breve.vector( -60, ( ( 10.000000 / 2 ) - ( 0.100000 / 2 ) ), -60 ) )
		self.wallN.setE( 0.100000 )
		self.wallN.setColor( breve.vector( 0.000000, 0.900000, 0.100000 ) )
		self.wallN = breve.createInstances( breve.Stationary, 1 )
		self.wallN.register( self.wallShape0, breve.vector( -40, ( ( 10.000000 / 2 ) - ( 0.100000 / 2 ) ), -60 ) )
		self.wallN.setE( 0.100000 )
		self.wallN.setColor( breve.vector( 0.000000, 0.900000, 0.100000 ) )
		self.wallN = breve.createInstances( breve.Stationary, 1 )
		self.wallN.register( self.wallShape0, breve.vector( -20, ( ( 10.000000 / 2 ) - ( 0.100000 / 2 ) ), -60 ) )
		self.wallN.setE( 0.100000 )
		self.wallN.setColor( breve.vector( 0.000000, 0.900000, 0.100000 ) )
		self.wallN = breve.createInstances( breve.Stationary, 1 )
		self.wallN.register( self.wallShape0, breve.vector( 0, ( ( 10.000000 / 2 ) - ( 0.100000 / 2 ) ), -60 ) )
		self.wallN.setE( 0.100000 )
		self.wallN.setColor( breve.vector( 0.000000, 0.900000, 0.100000 ) )
		self.wallN = breve.createInstances( breve.Stationary, 1 )
		self.wallN.register( self.wallShape0, breve.vector( 20, ( ( 10.000000 / 2 ) - ( 0.100000 / 2 ) ), -60 ) )
		self.wallN.setE( 0.100000 )
		self.wallN.setColor( breve.vector( 0.000000, 0.900000, 0.100000 ) )
		self.wallN = breve.createInstances( breve.Stationary, 1 )
		self.wallN.register( self.wallShape0, breve.vector( 40, ( ( 10.000000 / 2 ) - ( 0.100000 / 2 ) ), -60 ) )
		self.wallN.setE( 0.100000 )
		self.wallN.setColor( breve.vector( 0.000000, 0.900000, 0.100000 ) )
		self.wallN = breve.createInstances( breve.Stationary, 1 )
		self.wallN.register( self.wallShape0, breve.vector( 60, ( ( 10.000000 / 2 ) - ( 0.100000 / 2 ) ), -60 ) )
		self.wallN.setE( 0.100000 )
		self.wallN.setColor( breve.vector( 0.000000, 0.900000, 0.100000 ) )
		self.wallN = breve.createInstances( breve.Stationary, 1 )
		self.wallN.register( self.wallShape0, breve.vector( 80, ( ( 10.000000 / 2 ) - ( 0.100000 / 2 ) ), -60 ) )
		self.wallN.setE( 0.100000 )
		self.wallN.setColor( breve.vector( 0.000000, 0.900000, 0.100000 ) )
		self.wallN = breve.createInstances( breve.Stationary, 1 )
		self.wallN.register( self.wallShape0, breve.vector( -80, ( ( 10.000000 / 2 ) - ( 0.100000 / 2 ) ), 60 ) )
		self.wallN.setE( 0.100000 )
		self.wallN.setColor( breve.vector( 0.000000, 0.900000, 0.100000 ) )
		self.wallN = breve.createInstances( breve.Stationary, 1 )
		self.wallN.register( self.wallShape0, breve.vector( -60, ( ( 10.000000 / 2 ) - ( 0.100000 / 2 ) ), 60 ) )
		self.wallN.setE( 0.100000 )
		self.wallN.setColor( breve.vector( 0.000000, 0.900000, 0.100000 ) )
		self.wallN = breve.createInstances( breve.Stationary, 1 )
		self.wallN.register( self.wallShape0, breve.vector( -40, ( ( 10.000000 / 2 ) - ( 0.100000 / 2 ) ), 60 ) )
		self.wallN.setE( 0.100000 )
		self.wallN.setColor( breve.vector( 0.000000, 0.900000, 0.100000 ) )
		self.wallN = breve.createInstances( breve.Stationary, 1 )
		self.wallN.register( self.wallShape0, breve.vector( -20, ( ( 10.000000 / 2 ) - ( 0.100000 / 2 ) ), 60 ) )
		self.wallN.setE( 0.100000 )
		self.wallN.setColor( breve.vector( 0.000000, 0.900000, 0.100000 ) )
		self.wallN = breve.createInstances( breve.Stationary, 1 )
		self.wallN.register( self.wallShape0, breve.vector( 0, ( ( 10.000000 / 2 ) - ( 0.100000 / 2 ) ), 60 ) )
		self.wallN.setE( 0.100000 )
		self.wallN.setColor( breve.vector( 0.000000, 0.900000, 0.100000 ) )
		self.wallN = breve.createInstances( breve.Stationary, 1 )
		self.wallN.register( self.wallShape0, breve.vector( 20, ( ( 10.000000 / 2 ) - ( 0.100000 / 2 ) ), 60 ) )
		self.wallN.setE( 0.100000 )
		self.wallN.setColor( breve.vector( 0.000000, 0.900000, 0.100000 ) )
		self.wallN = breve.createInstances( breve.Stationary, 1 )
		self.wallN.register( self.wallShape0, breve.vector( 40, ( ( 10.000000 / 2 ) - ( 0.100000 / 2 ) ), 60 ) )
		self.wallN.setE( 0.100000 )
		self.wallN.setColor( breve.vector( 0.000000, 0.900000, 0.100000 ) )
		self.wallN = breve.createInstances( breve.Stationary, 1 )
		self.wallN.register( self.wallShape0, breve.vector( 60, ( ( 10.000000 / 2 ) - ( 0.100000 / 2 ) ), 60 ) )
		self.wallN.setE( 0.100000 )
		self.wallN.setColor( breve.vector( 0.000000, 0.900000, 0.100000 ) )
		self.wallN = breve.createInstances( breve.Stationary, 1 )
		self.wallN.register( self.wallShape0, breve.vector( 80, ( ( 10.000000 / 2 ) - ( 0.100000 / 2 ) ), 60 ) )
		self.wallN.setE( 0.100000 )
		self.wallN.setColor( breve.vector( 0.000000, 0.900000, 0.100000 ) )
		self.wallS = breve.createInstances( breve.Stationary, 1 )
		self.wallS.register( self.wallShape1, breve.vector( -80, ( ( 10.000000 / 2 ) - ( 0.100000 / 2 ) ), 50 ) )
		self.wallS.setE( 0.100000 )
		self.wallS.setColor( breve.vector( 0.000000, 0.900000, 0.100000 ) )
		self.wallS = breve.createInstances( breve.Stationary, 1 )
		self.wallS.register( self.wallShape1, breve.vector( -80, ( ( 10.000000 / 2 ) - ( 0.100000 / 2 ) ), 30 ) )
		self.wallS.setE( 0.100000 )
		self.wallS.setColor( breve.vector( 0.000000, 0.900000, 0.100000 ) )
		self.wallS = breve.createInstances( breve.Stationary, 1 )
		self.wallS.register( self.wallShape1, breve.vector( -80, ( ( 10.000000 / 2 ) - ( 0.100000 / 2 ) ), 10 ) )
		self.wallS.setE( 0.100000 )
		self.wallS.setColor( breve.vector( 0.000000, 0.900000, 0.100000 ) )
		self.wallS = breve.createInstances( breve.Stationary, 1 )
		self.wallS.register( self.wallShape1, breve.vector( -80, ( ( 10.000000 / 2 ) - ( 0.100000 / 2 ) ), -10 ) )
		self.wallS.setE( 0.100000 )
		self.wallS.setColor( breve.vector( 0.000000, 0.900000, 0.100000 ) )
		self.wallS = breve.createInstances( breve.Stationary, 1 )
		self.wallS.register( self.wallShape1, breve.vector( -80, ( ( 10.000000 / 2 ) - ( 0.100000 / 2 ) ), -30 ) )
		self.wallS.setE( 0.100000 )
		self.wallS.setColor( breve.vector( 0.000000, 0.900000, 0.100000 ) )
		self.wallS = breve.createInstances( breve.Stationary, 1 )
		self.wallS.register( self.wallShape1, breve.vector( -80, ( ( 10.000000 / 2 ) - ( 0.100000 / 2 ) ), -50 ) )
		self.wallS.setE( 0.100000 )
		self.wallS.setColor( breve.vector( 0.000000, 0.900000, 0.100000 ) )
		self.wallS = breve.createInstances( breve.Stationary, 1 )
		self.wallS.register( self.wallShape1, breve.vector( 80, ( ( 10.000000 / 2 ) - ( 0.100000 / 2 ) ), 50 ) )
		self.wallS.setE( 0.100000 )
		self.wallS.setColor( breve.vector( 0.000000, 0.900000, 0.100000 ) )
		self.wallS = breve.createInstances( breve.Stationary, 1 )
		self.wallS.register( self.wallShape1, breve.vector( 80, ( ( 10.000000 / 2 ) - ( 0.100000 / 2 ) ), 30 ) )
		self.wallS.setE( 0.100000 )
		self.wallS.setColor( breve.vector( 0.000000, 0.900000, 0.100000 ) )
		self.wallS = breve.createInstances( breve.Stationary, 1 )
		self.wallS.register( self.wallShape1, breve.vector( 80, ( ( 10.000000 / 2 ) - ( 0.100000 / 2 ) ), 10 ) )
		self.wallS.setE( 0.100000 )
		self.wallS.setColor( breve.vector( 0.000000, 0.900000, 0.100000 ) )
		self.wallS = breve.createInstances( breve.Stationary, 1 )
		self.wallS.register( self.wallShape1, breve.vector( 80, ( ( 10.000000 / 2 ) - ( 0.100000 / 2 ) ), -10 ) )
		self.wallS.setE( 0.100000 )
		self.wallS.setColor( breve.vector( 0.000000, 0.900000, 0.100000 ) )
		self.wallS = breve.createInstances( breve.Stationary, 1 )
		self.wallS.register( self.wallShape1, breve.vector( 80, ( ( 10.000000 / 2 ) - ( 0.100000 / 2 ) ), -30 ) )
		self.wallS.setE( 0.100000 )
		self.wallS.setColor( breve.vector( 0.000000, 0.900000, 0.100000 ) )
		self.wallS = breve.createInstances( breve.Stationary, 1 )
		self.wallS.register( self.wallShape1, breve.vector( 80, ( ( 10.000000 / 2 ) - ( 0.100000 / 2 ) ), -50 ) )
		self.wallS.setE( 0.100000 )
		self.wallS.setColor( breve.vector( 0.000000, 0.900000, 0.100000 ) )
		self.wallN = breve.createInstances( breve.Stationary, 1 )
		self.wallN.register( self.wallShape0, breve.vector( 40, ( ( 10.000000 / 2 ) - ( 0.100000 / 2 ) ), 40 ) )
		self.wallN.setE( 0.100000 )
		self.wallN.setColor( breve.vector( 0.000000, 0.900000, 0.100000 ) )
		self.wallN = breve.createInstances( breve.Stationary, 1 )
		self.wallN.register( self.wallShape0, breve.vector( 20, ( ( 10.000000 / 2 ) - ( 0.100000 / 2 ) ), 40 ) )
		self.wallN.setE( 0.100000 )
		self.wallN.setColor( breve.vector( 0.000000, 0.900000, 0.100000 ) )
		self.wallN = breve.createInstances( breve.Stationary, 1 )
		self.wallN.register( self.wallShape0, breve.vector( 20, ( ( 10.000000 / 2 ) - ( 0.100000 / 2 ) ), -20 ) )
		self.wallN.setE( 0.100000 )
		self.wallN.setColor( breve.vector( 0.000000, 0.900000, 0.100000 ) )
		self.wallN = breve.createInstances( breve.Stationary, 1 )
		self.wallN.register( self.wallShape0, breve.vector( 40, ( ( 10.000000 / 2 ) - ( 0.100000 / 2 ) ), -20 ) )
		self.wallN.setE( 0.100000 )
		self.wallN.setColor( breve.vector( 0.000000, 0.900000, 0.100000 ) )
		self.wallS = breve.createInstances( breve.Stationary, 1 )
		self.wallS.register( self.wallShape1, breve.vector( 50, ( ( 10.000000 / 2 ) - ( 0.100000 / 2 ) ), 30 ) )
		self.wallS.setE( 0.100000 )
		self.wallS.setColor( breve.vector( 0.000000, 0.900000, 0.100000 ) )
		self.wallS = breve.createInstances( breve.Stationary, 1 )
		self.wallS.register( self.wallShape1, breve.vector( 50, ( ( 10.000000 / 2 ) - ( 0.100000 / 2 ) ), 10 ) )
		self.wallS.setE( 0.100000 )
		self.wallS.setColor( breve.vector( 0.000000, 0.900000, 0.100000 ) )
		self.wallS = breve.createInstances( breve.Stationary, 1 )
		self.wallS.register( self.wallShape1, breve.vector( 50, ( ( 10.000000 / 2 ) - ( 0.100000 / 2 ) ), -10 ) )
		self.wallS.setE( 0.100000 )
		self.wallS.setColor( breve.vector( 0.000000, 0.900000, 0.100000 ) )
		self.wallS = breve.createInstances( breve.Stationary, 1 )
		self.wallS.register( self.wallShape1, breve.vector( 20, ( ( 10.000000 / 2 ) - ( 0.100000 / 2 ) ), 30 ) )
		self.wallS.setE( 0.100000 )
		self.wallS.setColor( breve.vector( 0.000000, 0.900000, 0.100000 ) )
		self.wallS = breve.createInstances( breve.Stationary, 1 )
		self.wallS.register( self.wallShape1, breve.vector( 20, ( ( 10.000000 / 2 ) - ( 0.100000 / 2 ) ), -10 ) )
		self.wallS.setE( 0.100000 )
		self.wallS.setColor( breve.vector( 0.000000, 0.900000, 0.100000 ) )
		return self


breve.LabyStage4 = LabyStage4
class LabyStage5( breve.Object ):
	def __init__( self ):
		breve.Object.__init__( self )
		self.floor = None
		self.floorShape = None
		self.wallShape0 = None
		self.wallShape1 = None
		self.wallN = None
		self.wallS = None
		LabyStage5.init( self )

	def init( self ):
		FLOOR_X = 0
		FLOOR_Z = 0

		FLOOR_X = 160
		FLOOR_Z = 120
		self.floorShape = breve.createInstances( breve.Cube, 1 ).initWith( breve.vector( FLOOR_X, 0.100000, FLOOR_Z ) )
		self.wallShape0 = breve.createInstances( breve.Cube, 1 ).initWith( breve.vector( 20, 10.000000, 1 ) )
		self.wallShape1 = breve.createInstances( breve.Cube, 1 ).initWith( breve.vector( 1, 10.000000, 20 ) )
		self.wallN = breve.createInstances( breve.Stationary, 1 )
		self.wallN.register( self.wallShape0, breve.vector( -80, ( ( 10.000000 / 2 ) - ( 0.100000 / 2 ) ), -60 ) )
		self.wallN.setE( 0.100000 )
		self.wallN.setColor( breve.vector( 0.000000, 0.900000, 0.100000 ) )
		self.wallN = breve.createInstances( breve.Stationary, 1 )
		self.wallN.register( self.wallShape0, breve.vector( -60, ( ( 10.000000 / 2 ) - ( 0.100000 / 2 ) ), -60 ) )
		self.wallN.setE( 0.100000 )
		self.wallN.setColor( breve.vector( 0.000000, 0.900000, 0.100000 ) )
		self.wallN = breve.createInstances( breve.Stationary, 1 )
		self.wallN.register( self.wallShape0, breve.vector( -40, ( ( 10.000000 / 2 ) - ( 0.100000 / 2 ) ), -60 ) )
		self.wallN.setE( 0.100000 )
		self.wallN.setColor( breve.vector( 0.000000, 0.900000, 0.100000 ) )
		self.wallN = breve.createInstances( breve.Stationary, 1 )
		self.wallN.register( self.wallShape0, breve.vector( -20, ( ( 10.000000 / 2 ) - ( 0.100000 / 2 ) ), -60 ) )
		self.wallN.setE( 0.100000 )
		self.wallN.setColor( breve.vector( 0.000000, 0.900000, 0.100000 ) )
		self.wallN = breve.createInstances( breve.Stationary, 1 )
		self.wallN.register( self.wallShape0, breve.vector( 0, ( ( 10.000000 / 2 ) - ( 0.100000 / 2 ) ), -60 ) )
		self.wallN.setE( 0.100000 )
		self.wallN.setColor( breve.vector( 0.000000, 0.900000, 0.100000 ) )
		self.wallN = breve.createInstances( breve.Stationary, 1 )
		self.wallN.register( self.wallShape0, breve.vector( 20, ( ( 10.000000 / 2 ) - ( 0.100000 / 2 ) ), -60 ) )
		self.wallN.setE( 0.100000 )
		self.wallN.setColor( breve.vector( 0.000000, 0.900000, 0.100000 ) )
		self.wallN = breve.createInstances( breve.Stationary, 1 )
		self.wallN.register( self.wallShape0, breve.vector( 40, ( ( 10.000000 / 2 ) - ( 0.100000 / 2 ) ), -60 ) )
		self.wallN.setE( 0.100000 )
		self.wallN.setColor( breve.vector( 0.000000, 0.900000, 0.100000 ) )
		self.wallN = breve.createInstances( breve.Stationary, 1 )
		self.wallN.register( self.wallShape0, breve.vector( 60, ( ( 10.000000 / 2 ) - ( 0.100000 / 2 ) ), -60 ) )
		self.wallN.setE( 0.100000 )
		self.wallN.setColor( breve.vector( 0.000000, 0.900000, 0.100000 ) )
		self.wallN = breve.createInstances( breve.Stationary, 1 )
		self.wallN.register( self.wallShape0, breve.vector( 80, ( ( 10.000000 / 2 ) - ( 0.100000 / 2 ) ), -60 ) )
		self.wallN.setE( 0.100000 )
		self.wallN.setColor( breve.vector( 0.000000, 0.900000, 0.100000 ) )
		self.wallN = breve.createInstances( breve.Stationary, 1 )
		self.wallN.register( self.wallShape0, breve.vector( -80, ( ( 10.000000 / 2 ) - ( 0.100000 / 2 ) ), 60 ) )
		self.wallN.setE( 0.100000 )
		self.wallN.setColor( breve.vector( 0.000000, 0.900000, 0.100000 ) )
		self.wallN = breve.createInstances( breve.Stationary, 1 )
		self.wallN.register( self.wallShape0, breve.vector( -60, ( ( 10.000000 / 2 ) - ( 0.100000 / 2 ) ), 60 ) )
		self.wallN.setE( 0.100000 )
		self.wallN.setColor( breve.vector( 0.000000, 0.900000, 0.100000 ) )
		self.wallN = breve.createInstances( breve.Stationary, 1 )
		self.wallN.register( self.wallShape0, breve.vector( -40, ( ( 10.000000 / 2 ) - ( 0.100000 / 2 ) ), 60 ) )
		self.wallN.setE( 0.100000 )
		self.wallN.setColor( breve.vector( 0.000000, 0.900000, 0.100000 ) )
		self.wallN = breve.createInstances( breve.Stationary, 1 )
		self.wallN.register( self.wallShape0, breve.vector( -20, ( ( 10.000000 / 2 ) - ( 0.100000 / 2 ) ), 60 ) )
		self.wallN.setE( 0.100000 )
		self.wallN.setColor( breve.vector( 0.000000, 0.900000, 0.100000 ) )
		self.wallN = breve.createInstances( breve.Stationary, 1 )
		self.wallN.register( self.wallShape0, breve.vector( 0, ( ( 10.000000 / 2 ) - ( 0.100000 / 2 ) ), 60 ) )
		self.wallN.setE( 0.100000 )
		self.wallN.setColor( breve.vector( 0.000000, 0.900000, 0.100000 ) )
		self.wallN = breve.createInstances( breve.Stationary, 1 )
		self.wallN.register( self.wallShape0, breve.vector( 20, ( ( 10.000000 / 2 ) - ( 0.100000 / 2 ) ), 60 ) )
		self.wallN.setE( 0.100000 )
		self.wallN.setColor( breve.vector( 0.000000, 0.900000, 0.100000 ) )
		self.wallN = breve.createInstances( breve.Stationary, 1 )
		self.wallN.register( self.wallShape0, breve.vector( 40, ( ( 10.000000 / 2 ) - ( 0.100000 / 2 ) ), 60 ) )
		self.wallN.setE( 0.100000 )
		self.wallN.setColor( breve.vector( 0.000000, 0.900000, 0.100000 ) )
		self.wallN = breve.createInstances( breve.Stationary, 1 )
		self.wallN.register( self.wallShape0, breve.vector( 60, ( ( 10.000000 / 2 ) - ( 0.100000 / 2 ) ), 60 ) )
		self.wallN.setE( 0.100000 )
		self.wallN.setColor( breve.vector( 0.000000, 0.900000, 0.100000 ) )
		self.wallN = breve.createInstances( breve.Stationary, 1 )
		self.wallN.register( self.wallShape0, breve.vector( 80, ( ( 10.000000 / 2 ) - ( 0.100000 / 2 ) ), 60 ) )
		self.wallN.setE( 0.100000 )
		self.wallN.setColor( breve.vector( 0.000000, 0.900000, 0.100000 ) )
		self.wallS = breve.createInstances( breve.Stationary, 1 )
		self.wallS.register( self.wallShape1, breve.vector( -80, ( ( 10.000000 / 2 ) - ( 0.100000 / 2 ) ), 50 ) )
		self.wallS.setE( 0.100000 )
		self.wallS.setColor( breve.vector( 0.000000, 0.900000, 0.100000 ) )
		self.wallS = breve.createInstances( breve.Stationary, 1 )
		self.wallS.register( self.wallShape1, breve.vector( -80, ( ( 10.000000 / 2 ) - ( 0.100000 / 2 ) ), 30 ) )
		self.wallS.setE( 0.100000 )
		self.wallS.setColor( breve.vector( 0.000000, 0.900000, 0.100000 ) )
		self.wallS = breve.createInstances( breve.Stationary, 1 )
		self.wallS.register( self.wallShape1, breve.vector( -80, ( ( 10.000000 / 2 ) - ( 0.100000 / 2 ) ), 10 ) )
		self.wallS.setE( 0.100000 )
		self.wallS.setColor( breve.vector( 0.000000, 0.900000, 0.100000 ) )
		self.wallS = breve.createInstances( breve.Stationary, 1 )
		self.wallS.register( self.wallShape1, breve.vector( -80, ( ( 10.000000 / 2 ) - ( 0.100000 / 2 ) ), -10 ) )
		self.wallS.setE( 0.100000 )
		self.wallS.setColor( breve.vector( 0.000000, 0.900000, 0.100000 ) )
		self.wallS = breve.createInstances( breve.Stationary, 1 )
		self.wallS.register( self.wallShape1, breve.vector( -80, ( ( 10.000000 / 2 ) - ( 0.100000 / 2 ) ), -30 ) )
		self.wallS.setE( 0.100000 )
		self.wallS.setColor( breve.vector( 0.000000, 0.900000, 0.100000 ) )
		self.wallS = breve.createInstances( breve.Stationary, 1 )
		self.wallS.register( self.wallShape1, breve.vector( -80, ( ( 10.000000 / 2 ) - ( 0.100000 / 2 ) ), -50 ) )
		self.wallS.setE( 0.100000 )
		self.wallS.setColor( breve.vector( 0.000000, 0.900000, 0.100000 ) )
		self.wallS = breve.createInstances( breve.Stationary, 1 )
		self.wallS.register( self.wallShape1, breve.vector( 80, ( ( 10.000000 / 2 ) - ( 0.100000 / 2 ) ), 50 ) )
		self.wallS.setE( 0.100000 )
		self.wallS.setColor( breve.vector( 0.000000, 0.900000, 0.100000 ) )
		self.wallS = breve.createInstances( breve.Stationary, 1 )
		self.wallS.register( self.wallShape1, breve.vector( 80, ( ( 10.000000 / 2 ) - ( 0.100000 / 2 ) ), 30 ) )
		self.wallS.setE( 0.100000 )
		self.wallS.setColor( breve.vector( 0.000000, 0.900000, 0.100000 ) )
		self.wallS = breve.createInstances( breve.Stationary, 1 )
		self.wallS.register( self.wallShape1, breve.vector( 80, ( ( 10.000000 / 2 ) - ( 0.100000 / 2 ) ), 10 ) )
		self.wallS.setE( 0.100000 )
		self.wallS.setColor( breve.vector( 0.000000, 0.900000, 0.100000 ) )
		self.wallS = breve.createInstances( breve.Stationary, 1 )
		self.wallS.register( self.wallShape1, breve.vector( 80, ( ( 10.000000 / 2 ) - ( 0.100000 / 2 ) ), -10 ) )
		self.wallS.setE( 0.100000 )
		self.wallS.setColor( breve.vector( 0.000000, 0.900000, 0.100000 ) )
		self.wallS = breve.createInstances( breve.Stationary, 1 )
		self.wallS.register( self.wallShape1, breve.vector( 80, ( ( 10.000000 / 2 ) - ( 0.100000 / 2 ) ), -30 ) )
		self.wallS.setE( 0.100000 )
		self.wallS.setColor( breve.vector( 0.000000, 0.900000, 0.100000 ) )
		self.wallS = breve.createInstances( breve.Stationary, 1 )
		self.wallS.register( self.wallShape1, breve.vector( 80, ( ( 10.000000 / 2 ) - ( 0.100000 / 2 ) ), -50 ) )
		self.wallS.setE( 0.100000 )
		self.wallS.setColor( breve.vector( 0.000000, 0.900000, 0.100000 ) )
		return self


breve.LabyStage5 = LabyStage5
class DemoStage( breve.Object ):
	def __init__( self ):
		breve.Object.__init__( self )
		self.floor = None
		self.floorShape = None
		self.wallShape0 = None
		self.wallShape1 = None
		self.wallN = None
		self.wallS = None
		DemoStage.init( self )

	def init( self ):
		self.floorShape = breve.createInstances( breve.Cube, 1 ).initWith( breve.vector( 45, 0.100000, 55 ) )
		self.floor = breve.createInstances( breve.Stationary, 1 )
		self.floor.register( self.floorShape, breve.vector( 0, ( ( -0.100000 ) / 2 ), 0 ) )
		self.floor.setE( 0.100000 )
		self.wallShape0 = breve.createInstances( breve.Cube, 1 ).initWith( breve.vector( 20, 10.000000, 1 ) )
		self.wallShape1 = breve.createInstances( breve.Cube, 1 ).initWith( breve.vector( 1, 10.000000, 20 ) )
		self.wallN = breve.createInstances( breve.Stationary, 1 )
		self.wallN.register( self.wallShape0, breve.vector( 13, ( ( 10.000000 / 2 ) - ( 0.100000 / 2 ) ), 13 ) )
		self.wallN.setE( 0.100000 )
		self.wallN.setColor( breve.vector( 0.000000, 0.900000, 0.100000 ) )
		self.wallN = breve.createInstances( breve.Stationary, 1 )
		self.wallN.register( self.wallShape0, breve.vector( -13, ( ( 10.000000 / 2 ) - ( 0.100000 / 2 ) ), 0 ) )
		self.wallN.setE( 0.100000 )
		self.wallN.setColor( breve.vector( 0.000000, 0.900000, 0.100000 ) )
		self.wallN = breve.createInstances( breve.Stationary, 1 )
		self.wallN.register( self.wallShape0, breve.vector( 13, ( ( 10.000000 / 2 ) - ( 0.100000 / 2 ) ), -13 ) )
		self.wallN.setE( 0.100000 )
		self.wallN.setColor( breve.vector( 0.000000, 0.900000, 0.100000 ) )
		self.wallN = breve.createInstances( breve.Stationary, 1 )
		self.wallN.register( self.wallShape0, breve.vector( -13, ( ( 10.000000 / 2 ) - ( 0.100000 / 2 ) ), 28 ) )
		self.wallN.setE( 0.100000 )
		self.wallN.setColor( breve.vector( 0.000000, 0.900000, 0.100000 ) )
		self.wallN = breve.createInstances( breve.Stationary, 1 )
		self.wallN.register( self.wallShape0, breve.vector( 0, ( ( 10.000000 / 2 ) - ( 0.100000 / 2 ) ), 28 ) )
		self.wallN.setE( 0.100000 )
		self.wallN.setColor( breve.vector( 0.000000, 0.900000, 0.100000 ) )
		self.wallN = breve.createInstances( breve.Stationary, 1 )
		self.wallN.register( self.wallShape0, breve.vector( 13, ( ( 10.000000 / 2 ) - ( 0.100000 / 2 ) ), 28 ) )
		self.wallN.setE( 0.100000 )
		self.wallN.setColor( breve.vector( 0.000000, 0.900000, 0.100000 ) )
		self.wallN = breve.createInstances( breve.Stationary, 1 )
		self.wallN.register( self.wallShape0, breve.vector( -13, ( ( 10.000000 / 2 ) - ( 0.100000 / 2 ) ), -28 ) )
		self.wallN.setE( 0.100000 )
		self.wallN.setColor( breve.vector( 0.000000, 0.900000, 0.100000 ) )
		self.wallN = breve.createInstances( breve.Stationary, 1 )
		self.wallN.register( self.wallShape0, breve.vector( 0, ( ( 10.000000 / 2 ) - ( 0.100000 / 2 ) ), -28 ) )
		self.wallN.setE( 0.100000 )
		self.wallN.setColor( breve.vector( 0.000000, 0.900000, 0.100000 ) )
		self.wallN = breve.createInstances( breve.Stationary, 1 )
		self.wallN.register( self.wallShape0, breve.vector( 13, ( ( 10.000000 / 2 ) - ( 0.100000 / 2 ) ), -28 ) )
		self.wallN.setE( 0.100000 )
		self.wallN.setColor( breve.vector( 0.000000, 0.900000, 0.100000 ) )
		self.wallN = breve.createInstances( breve.Stationary, 1 )
		self.wallN.register( self.wallShape1, breve.vector( 23, ( ( 10.000000 / 2 ) - ( 0.100000 / 2 ) ), 18.500000 ) )
		self.wallN.setE( 0.100000 )
		self.wallN.setColor( breve.vector( 0.000000, 0.900000, 0.100000 ) )
		self.wallS = breve.createInstances( breve.Stationary, 1 )
		self.wallS.register( self.wallShape1, breve.vector( 23, ( ( 10.000000 / 2 ) - ( 0.100000 / 2 ) ), 10 ) )
		self.wallS.setE( 0.100000 )
		self.wallS.setColor( breve.vector( 0.000000, 0.900000, 0.100000 ) )
		self.wallN = breve.createInstances( breve.Stationary, 1 )
		self.wallN.register( self.wallShape1, breve.vector( 23, ( ( 10.000000 / 2 ) - ( 0.100000 / 2 ) ), -10 ) )
		self.wallN.setE( 0.100000 )
		self.wallN.setColor( breve.vector( 0.000000, 0.900000, 0.100000 ) )
		self.wallS = breve.createInstances( breve.Stationary, 1 )
		self.wallS.register( self.wallShape1, breve.vector( 23, ( ( 10.000000 / 2 ) - ( 0.100000 / 2 ) ), -18.500000 ) )
		self.wallS.setE( 0.100000 )
		self.wallS.setColor( breve.vector( 0.000000, 0.900000, 0.100000 ) )
		self.wallN = breve.createInstances( breve.Stationary, 1 )
		self.wallN.register( self.wallShape1, breve.vector( -23, ( ( 10.000000 / 2 ) - ( 0.100000 / 2 ) ), 18.500000 ) )
		self.wallN.setE( 0.100000 )
		self.wallN.setColor( breve.vector( 0.000000, 0.900000, 0.100000 ) )
		self.wallS = breve.createInstances( breve.Stationary, 1 )
		self.wallS.register( self.wallShape1, breve.vector( -23, ( ( 10.000000 / 2 ) - ( 0.100000 / 2 ) ), 10 ) )
		self.wallS.setE( 0.100000 )
		self.wallS.setColor( breve.vector( 0.000000, 0.900000, 0.100000 ) )
		self.wallN = breve.createInstances( breve.Stationary, 1 )
		self.wallN.register( self.wallShape1, breve.vector( -23, ( ( 10.000000 / 2 ) - ( 0.100000 / 2 ) ), -10 ) )
		self.wallN.setE( 0.100000 )
		self.wallN.setColor( breve.vector( 0.000000, 0.900000, 0.100000 ) )
		self.wallS = breve.createInstances( breve.Stationary, 1 )
		self.wallS.register( self.wallShape1, breve.vector( -23, ( ( 10.000000 / 2 ) - ( 0.100000 / 2 ) ), -18.500000 ) )
		self.wallS.setE( 0.100000 )
		self.wallS.setColor( breve.vector( 0.000000, 0.900000, 0.100000 ) )
		return self


breve.DemoStage = DemoStage
class TestStage( breve.Object ):
	def __init__( self ):
		breve.Object.__init__( self )
		self.floor = None
		self.floorShape = None
		self.wallShapeTest = None
		self.wallShape0 = None
		self.wallShape1 = None
		self.wallN = None
		self.wallS = None
		self.wallT = None
		TestStage.init( self )

	def init( self ):
		FLOOR_X = 0
		FLOOR_Z = 0

		FLOOR_X = 160
		FLOOR_Z = 120
		self.floorShape = breve.createInstances( breve.Cube, 1 ).initWith( breve.vector( FLOOR_X, 0.100000, FLOOR_Z ) )
		self.floor = breve.createInstances( breve.Stationary, 1 )
		self.floor.register( self.floorShape, breve.vector( 0, ( ( -0.100000 ) / 2 ), 0 ) )
		self.floor.setE( 0.100000 )
		self.wallShape0 = breve.createInstances( breve.Cube, 1 ).initWith( breve.vector( 20, 10.000000, 1 ) )
		self.wallShape1 = breve.createInstances( breve.Cube, 1 ).initWith( breve.vector( 1, 10.000000, 20 ) )
		self.wallN = breve.createInstances( breve.Stationary, 1 )
		self.wallN.register( self.wallShape0, breve.vector( -80, ( ( 10.000000 / 2 ) - ( 0.100000 / 2 ) ), -60 ) )
		self.wallN.setE( 0.100000 )
		self.wallN.setColor( breve.vector( 0.000000, 0.900000, 0.100000 ) )
		self.wallN = breve.createInstances( breve.Stationary, 1 )
		self.wallN.register( self.wallShape0, breve.vector( -60, ( ( 10.000000 / 2 ) - ( 0.100000 / 2 ) ), -60 ) )
		self.wallN.setE( 0.100000 )
		self.wallN.setColor( breve.vector( 0.000000, 0.900000, 0.100000 ) )
		self.wallN = breve.createInstances( breve.Stationary, 1 )
		self.wallN.register( self.wallShape0, breve.vector( -40, ( ( 10.000000 / 2 ) - ( 0.100000 / 2 ) ), -60 ) )
		self.wallN.setE( 0.100000 )
		self.wallN.setColor( breve.vector( 0.000000, 0.900000, 0.100000 ) )
		self.wallN = breve.createInstances( breve.Stationary, 1 )
		self.wallN.register( self.wallShape0, breve.vector( -20, ( ( 10.000000 / 2 ) - ( 0.100000 / 2 ) ), -60 ) )
		self.wallN.setE( 0.100000 )
		self.wallN.setColor( breve.vector( 0.000000, 0.900000, 0.100000 ) )
		self.wallN = breve.createInstances( breve.Stationary, 1 )
		self.wallN.register( self.wallShape0, breve.vector( 0, ( ( 10.000000 / 2 ) - ( 0.100000 / 2 ) ), -60 ) )
		self.wallN.setE( 0.100000 )
		self.wallN.setColor( breve.vector( 0.000000, 0.900000, 0.100000 ) )
		self.wallN = breve.createInstances( breve.Stationary, 1 )
		self.wallN.register( self.wallShape0, breve.vector( 20, ( ( 10.000000 / 2 ) - ( 0.100000 / 2 ) ), -60 ) )
		self.wallN.setE( 0.100000 )
		self.wallN.setColor( breve.vector( 0.000000, 0.900000, 0.100000 ) )
		self.wallN = breve.createInstances( breve.Stationary, 1 )
		self.wallN.register( self.wallShape0, breve.vector( 40, ( ( 10.000000 / 2 ) - ( 0.100000 / 2 ) ), -60 ) )
		self.wallN.setE( 0.100000 )
		self.wallN.setColor( breve.vector( 0.000000, 0.900000, 0.100000 ) )
		self.wallN = breve.createInstances( breve.Stationary, 1 )
		self.wallN.register( self.wallShape0, breve.vector( 60, ( ( 10.000000 / 2 ) - ( 0.100000 / 2 ) ), -60 ) )
		self.wallN.setE( 0.100000 )
		self.wallN.setColor( breve.vector( 0.000000, 0.900000, 0.100000 ) )
		self.wallN = breve.createInstances( breve.Stationary, 1 )
		self.wallN.register( self.wallShape0, breve.vector( 80, ( ( 10.000000 / 2 ) - ( 0.100000 / 2 ) ), -60 ) )
		self.wallN.setE( 0.100000 )
		self.wallN.setColor( breve.vector( 0.000000, 0.900000, 0.100000 ) )
		self.wallN = breve.createInstances( breve.Stationary, 1 )
		self.wallN.register( self.wallShape0, breve.vector( -80, ( ( 10.000000 / 2 ) - ( 0.100000 / 2 ) ), 60 ) )
		self.wallN.setE( 0.100000 )
		self.wallN.setColor( breve.vector( 0.000000, 0.900000, 0.100000 ) )
		self.wallN = breve.createInstances( breve.Stationary, 1 )
		self.wallN.register( self.wallShape0, breve.vector( -60, ( ( 10.000000 / 2 ) - ( 0.100000 / 2 ) ), 60 ) )
		self.wallN.setE( 0.100000 )
		self.wallN.setColor( breve.vector( 0.000000, 0.900000, 0.100000 ) )
		self.wallN = breve.createInstances( breve.Stationary, 1 )
		self.wallN.register( self.wallShape0, breve.vector( -40, ( ( 10.000000 / 2 ) - ( 0.100000 / 2 ) ), 60 ) )
		self.wallN.setE( 0.100000 )
		self.wallN.setColor( breve.vector( 0.000000, 0.900000, 0.100000 ) )
		self.wallN = breve.createInstances( breve.Stationary, 1 )
		self.wallN.register( self.wallShape0, breve.vector( -20, ( ( 10.000000 / 2 ) - ( 0.100000 / 2 ) ), 60 ) )
		self.wallN.setE( 0.100000 )
		self.wallN.setColor( breve.vector( 0.000000, 0.900000, 0.100000 ) )
		self.wallN = breve.createInstances( breve.Stationary, 1 )
		self.wallN.register( self.wallShape0, breve.vector( 0, ( ( 10.000000 / 2 ) - ( 0.100000 / 2 ) ), 60 ) )
		self.wallN.setE( 0.100000 )
		self.wallN.setColor( breve.vector( 0.000000, 0.900000, 0.100000 ) )
		self.wallN = breve.createInstances( breve.Stationary, 1 )
		self.wallN.register( self.wallShape0, breve.vector( 20, ( ( 10.000000 / 2 ) - ( 0.100000 / 2 ) ), 60 ) )
		self.wallN.setE( 0.100000 )
		self.wallN.setColor( breve.vector( 0.000000, 0.900000, 0.100000 ) )
		self.wallN = breve.createInstances( breve.Stationary, 1 )
		self.wallN.register( self.wallShape0, breve.vector( 40, ( ( 10.000000 / 2 ) - ( 0.100000 / 2 ) ), 60 ) )
		self.wallN.setE( 0.100000 )
		self.wallN.setColor( breve.vector( 0.000000, 0.900000, 0.100000 ) )
		self.wallN = breve.createInstances( breve.Stationary, 1 )
		self.wallN.register( self.wallShape0, breve.vector( 60, ( ( 10.000000 / 2 ) - ( 0.100000 / 2 ) ), 60 ) )
		self.wallN.setE( 0.100000 )
		self.wallN.setColor( breve.vector( 0.000000, 0.900000, 0.100000 ) )
		self.wallN = breve.createInstances( breve.Stationary, 1 )
		self.wallN.register( self.wallShape0, breve.vector( 80, ( ( 10.000000 / 2 ) - ( 0.100000 / 2 ) ), 60 ) )
		self.wallN.setE( 0.100000 )
		self.wallN.setColor( breve.vector( 0.000000, 0.900000, 0.100000 ) )
		self.wallS = breve.createInstances( breve.Stationary, 1 )
		self.wallS.register( self.wallShape1, breve.vector( -80, ( ( 10.000000 / 2 ) - ( 0.100000 / 2 ) ), 50 ) )
		self.wallS.setE( 0.100000 )
		self.wallS.setColor( breve.vector( 0.000000, 0.900000, 0.100000 ) )
		self.wallS = breve.createInstances( breve.Stationary, 1 )
		self.wallS.register( self.wallShape1, breve.vector( -80, ( ( 10.000000 / 2 ) - ( 0.100000 / 2 ) ), 30 ) )
		self.wallS.setE( 0.100000 )
		self.wallS.setColor( breve.vector( 0.000000, 0.900000, 0.100000 ) )
		self.wallS = breve.createInstances( breve.Stationary, 1 )
		self.wallS.register( self.wallShape1, breve.vector( -80, ( ( 10.000000 / 2 ) - ( 0.100000 / 2 ) ), 10 ) )
		self.wallS.setE( 0.100000 )
		self.wallS.setColor( breve.vector( 0.000000, 0.900000, 0.100000 ) )
		self.wallS = breve.createInstances( breve.Stationary, 1 )
		self.wallS.register( self.wallShape1, breve.vector( -80, ( ( 10.000000 / 2 ) - ( 0.100000 / 2 ) ), -10 ) )
		self.wallS.setE( 0.100000 )
		self.wallS.setColor( breve.vector( 0.000000, 0.900000, 0.100000 ) )
		self.wallS = breve.createInstances( breve.Stationary, 1 )
		self.wallS.register( self.wallShape1, breve.vector( -80, ( ( 10.000000 / 2 ) - ( 0.100000 / 2 ) ), -30 ) )
		self.wallS.setE( 0.100000 )
		self.wallS.setColor( breve.vector( 0.000000, 0.900000, 0.100000 ) )
		self.wallS = breve.createInstances( breve.Stationary, 1 )
		self.wallS.register( self.wallShape1, breve.vector( -80, ( ( 10.000000 / 2 ) - ( 0.100000 / 2 ) ), -50 ) )
		self.wallS.setE( 0.100000 )
		self.wallS.setColor( breve.vector( 0.000000, 0.900000, 0.100000 ) )
		self.wallS = breve.createInstances( breve.Stationary, 1 )
		self.wallS.register( self.wallShape1, breve.vector( 80, ( ( 10.000000 / 2 ) - ( 0.100000 / 2 ) ), 50 ) )
		self.wallS.setE( 0.100000 )
		self.wallS.setColor( breve.vector( 0.000000, 0.900000, 0.100000 ) )
		self.wallS = breve.createInstances( breve.Stationary, 1 )
		self.wallS.register( self.wallShape1, breve.vector( 80, ( ( 10.000000 / 2 ) - ( 0.100000 / 2 ) ), 30 ) )
		self.wallS.setE( 0.100000 )
		self.wallS.setColor( breve.vector( 0.000000, 0.900000, 0.100000 ) )
		self.wallS = breve.createInstances( breve.Stationary, 1 )
		self.wallS.register( self.wallShape1, breve.vector( 80, ( ( 10.000000 / 2 ) - ( 0.100000 / 2 ) ), 10 ) )
		self.wallS.setE( 0.100000 )
		self.wallS.setColor( breve.vector( 0.000000, 0.900000, 0.100000 ) )
		self.wallS = breve.createInstances( breve.Stationary, 1 )
		self.wallS.register( self.wallShape1, breve.vector( 80, ( ( 10.000000 / 2 ) - ( 0.100000 / 2 ) ), -10 ) )
		self.wallS.setE( 0.100000 )
		self.wallS.setColor( breve.vector( 0.000000, 0.900000, 0.100000 ) )
		self.wallS = breve.createInstances( breve.Stationary, 1 )
		self.wallS.register( self.wallShape1, breve.vector( 80, ( ( 10.000000 / 2 ) - ( 0.100000 / 2 ) ), -30 ) )
		self.wallS.setE( 0.100000 )
		self.wallS.setColor( breve.vector( 0.000000, 0.900000, 0.100000 ) )
		self.wallS = breve.createInstances( breve.Stationary, 1 )
		self.wallS.register( self.wallShape1, breve.vector( 80, ( ( 10.000000 / 2 ) - ( 0.100000 / 2 ) ), -50 ) )
		self.wallS.setE( 0.100000 )
		self.wallS.setColor( breve.vector( 0.000000, 0.900000, 0.100000 ) )
		return self


breve.TestStage = TestStage


# Create an instance of our controller object to initialize the simulation

myControl()


