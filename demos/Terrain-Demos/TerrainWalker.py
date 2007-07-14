
import breve

class Walker( breve.PhysicalControl ):
	def __init__( self ):
		breve.PhysicalControl.__init__( self )
		self.currentSeat = 0
		self.lockMenu = None
		self.locked = 0
		self.monkeys = breve.objectList()
		self.seats = breve.objectList()
		self.terrain = None
		self.wigglyThing = None
		Walker.init( self )

	def breedNewMonkeys( self ):

		print '''breeding monkeys...'''
		
		print '''driver ''', self.seats[ 0 ].getNumber(), self.seats[ 0 ].getDistance()
		print '''driver ''', self.seats[ 1 ].getNumber(), self.seats[ 1 ].getDistance()
		print '''driver ''', self.seats[ 2 ].getNumber(), self.seats[ 2 ].getDistance()
		print '''driver ''', self.seats[ 3 ].getNumber(), self.seats[ 3 ].getDistance()
		self.seats[ 0 ].breed( self.seats[ 1 ], self.seats[ 2 ] )
		self.seats[ 1 ].breed( self.seats[ 0 ], self.seats[ 3 ] )
		self.seats[ 2 ].getGenome().mutate()
		self.seats[ 3 ].getGenome().mutate()

	def changeDrivers( self ):
		newDriver = 0
		newOffset = breve.vector()

		if self.locked:
			return

		newOffset = ( breve.randomExpression( breve.vector( 30, 6, 30 ) ) + breve.vector( -15, 1, -15 ) )
		if ( breve.length( newOffset ) < 14 ):
			newOffset = ( ( 14 * newOffset ) / breve.length( newOffset ) )

		self.panCameraOffset( newOffset, 30 )
		self.seats[ self.currentSeat ].setDistance( breve.length( self.wigglyThing.getLocation() ) )
		self.wigglyThing.center()
		self.currentSeat = ( self.currentSeat + 1 )
		if ( self.currentSeat > 3 ):
			self.breedNewMonkeys()
			self.pickDrivers()


		newDriver = self.seats[ self.currentSeat ].getNumber()
		self.schedule( 'changeDrivers', ( self.getTime() + 20.000000 ) )
		self.displayCurrentDriver()

	def compareDistance( self, a, b ):
		result = 0

		result = ( b.getDistance() - a.getDistance() )
		return result

	def displayCurrentDriver( self ):
		currentNumber = 0

		currentNumber = self.seats[ self.currentSeat ].getNumber()
		self.setDisplayText( '''Driver #%s''' % (  currentNumber ), -0.950000, -0.900000 )

	def init( self ):
		number = 0
		item = None

		self.locked = 0
		self.setRandomSeedFromDevRandom()
		self.enableFastPhysics()
		self.setFastPhysicsIterations( 5 )
		self.enableLighting()
		self.enableSmoothDrawing()
		self.moveLight( breve.vector( 0, 40, 0 ) )
		self.enableShadowVolumes()
		self.setBackgroundColor( breve.vector( 0.400000, 0.600000, 0.900000 ) )
		self.setBackgroundTextureImage( breve.createInstances( breve.Image, 1 ).load( 'images/clouds.png' ) )
		self.terrain = breve.createInstances( breve.Terrain, 1 )
		self.terrain.generate( 0.400000, 15 )
		self.terrain.setScale( 3 )
		self.terrain.setET( 0.900000 )
		self.wigglyThing = breve.createInstances( breve.Creature, 1 )
		self.wigglyThing.move( breve.vector( 0, 6, 0 ) )
		self.offsetCamera( breve.vector( 3, 13, -13 ) )
		self.watch( self.wigglyThing )
		self.monkeys = breve.createInstances( breve.Monkeys, 15 )
		for item in self.monkeys:
			item.setNumber( number )
			number = ( number + 1 )


		self.pickDrivers()
		self.lockMenu = self.addMenu( '''Lock Driver''', 'toggleDriverLock' )
		self.addMenuSeparator()
		self.addMenu( '''Save Current Genome''', 'saveCurrentGenome' )
		self.addMenu( '''Load Into Current Genome''', 'loadIntoCurrentGenome' )
		self.schedule( 'changeDrivers', ( self.getTime() + 20.000000 ) )
		self.displayCurrentDriver()

	def iterate( self ):

		self.seats[ self.currentSeat ].control( self.wigglyThing, self.getTime() )
		breve.PhysicalControl.iterate( self )

	def loadIntoCurrentGenome( self ):

		self.seats[ self.currentSeat ].getGenome().loadWithDialog()

	def pickDrivers( self ):

		
		self.seats[ 0 ] = self.monkeys[ 0 ]
		self.seats[ 1 ] = self.monkeys[ 1 ]
		self.seats[ 2 ] = self.monkeys[ 2 ]
		self.seats[ 3 ] = self.monkeys[ 3 ]
		self.currentSeat = 0

	def randomSort( self, a, b ):

		return ( breve.randomExpression( 3 ) - 1 )

	def saveCurrentGenome( self ):

		self.seats[ self.currentSeat ].getGenome().saveWithDialog()

	def toggleDriverLock( self ):

		if ( self.locked == 1 ):
			self.locked = 0
			self.wigglyThing.center()
			self.schedule( 'changeDrivers', ( self.getTime() + 20.000000 ) )
			self.lockMenu.uncheck()

		else:
			self.locked = 1
			self.lockMenu.check()




breve.Walker = Walker
class Monkeys( breve.Object ):
	def __init__( self ):
		breve.Object.__init__( self )
		self.distanceTraveled = 0
		self.genome = None
		self.number = 0
		Monkeys.init( self )

	def breed( self, otherMonkey, child ):

		child.getGenome().crossover( otherMonkey.getGenome(), self.getGenome() )

	def control( self, theRobot, t ):

		theRobot.setJointVelocity0( ( 10 * self.genome.calculateTorque0( t ) ) )
		theRobot.setJointVelocity1( ( 10 * self.genome.calculateTorque1( t ) ) )
		theRobot.setJointVelocity2( ( 10 * ( -self.genome.calculateTorque2( t ) ) ) )
		theRobot.setJointVelocity3( ( 10 * ( -self.genome.calculateTorque3( t ) ) ) )

	def getDistance( self ):

		return self.distanceTraveled

	def getGenome( self ):

		return self.genome

	def getNumber( self ):

		return self.number

	def init( self ):

		self.genome = breve.createInstances( breve.MonkeyGenome, 1 )
		self.randomize()

	def randomize( self ):

		self.genome.randomize()

	def setDistance( self, value ):

		self.distanceTraveled = value

	def setNumber( self, n ):

		self.number = n


breve.Monkeys = Monkeys
class MonkeyGenome( breve.Object ):
	def __init__( self ):
		breve.Object.__init__( self )
		self.genomeData = breve.objectList()
		MonkeyGenome.init( self )

	def calculateTorque0( self, time ):

		return ( 0.500000 * ( breve.breveInternalFunctionFinder.sin( self, ( self.genomeData[ 8 ] * ( time + self.genomeData[ 4 ] ) ) ) - self.genomeData[ 0 ] ) )

	def calculateTorque1( self, time ):

		return ( 0.500000 * ( breve.breveInternalFunctionFinder.sin( self, ( self.genomeData[ 8 ] * ( time + self.genomeData[ 5 ] ) ) ) - self.genomeData[ 1 ] ) )

	def calculateTorque2( self, time ):

		return ( 0.500000 * ( breve.breveInternalFunctionFinder.sin( self, ( self.genomeData[ 8 ] * ( time + self.genomeData[ 6 ] ) ) ) - self.genomeData[ 2 ] ) )

	def calculateTorque3( self, time ):

		return ( 0.500000 * ( breve.breveInternalFunctionFinder.sin( self, ( self.genomeData[ 8 ] * ( time + self.genomeData[ 7 ] ) ) ) - self.genomeData[ 3 ] ) )

	def crossover( self, p1, p2 ):
		crossoverPoint = 0
		n = 0
		tmp = None

		if breve.randomExpression( 1 ):
			tmp = p2
			p2 = p1
			p1 = tmp


		crossoverPoint = breve.randomExpression( ( breve.length( self.genomeData ) - 1 ) )
		n = 0
		while ( n < breve.length( self.genomeData ) ):
			if ( n < crossoverPoint ):
				self.genomeData[ n ] = p1.getValue( n )
			else:
				self.genomeData[ n ] = p2.getValue( n )


			n = ( n + 1 )


	def getValue( self, n ):

		return self.genomeData[ n ]

	def init( self ):

		self.genomeData = [ 0, 0, 0, 0, 0, 0, 0, 0, 0 ]

	def mutate( self ):
		n = 0

		n = breve.randomExpression( 8 )
		if ( n < 4 ):
			self.genomeData[ n ] = ( breve.randomExpression( 2.000000 ) - 1.000000 )
		else:
			if ( n < 8 ):
				self.genomeData[ n ] = ( breve.randomExpression( 6.300000 ) - 3.150000 )
			else:
				self.genomeData[ n ] = ( breve.randomExpression( 5.000000 ) - 2.500000 )


		print '''mutated item %s of %s''' % (  n, self )

	def randomize( self ):

		self.genomeData[ 8 ] = ( breve.randomExpression( 5.000000 ) - 2.500000 )
		self.genomeData[ 0 ] = ( breve.randomExpression( 2.000000 ) - 1.000000 )
		self.genomeData[ 1 ] = ( breve.randomExpression( 2.000000 ) - 1.000000 )
		self.genomeData[ 2 ] = ( breve.randomExpression( 2.000000 ) - 1.000000 )
		self.genomeData[ 3 ] = ( breve.randomExpression( 2.000000 ) - 1.000000 )
		self.genomeData[ 4 ] = ( breve.randomExpression( 6.300000 ) - 3.150000 )
		self.genomeData[ 5 ] = ( breve.randomExpression( 6.300000 ) - 3.150000 )
		self.genomeData[ 6 ] = ( breve.randomExpression( 6.300000 ) - 3.150000 )
		self.genomeData[ 7 ] = ( breve.randomExpression( 6.300000 ) - 3.150000 )


breve.MonkeyGenome = MonkeyGenome
class Creature( breve.MultiBody ):
	def __init__( self ):
		breve.MultiBody.__init__( self )
		self.bodyLink = None
		self.joints = breve.objectList()
		self.links = breve.objectList()
		Creature.init( self )

	def center( self ):

		self.move( breve.vector( 0, 10, 0 ) )

	def getRoot( self ):

		return self.bodyLink

	def init( self ):
		bodyShape = None
		lowerLinkShape = None
		linkShape = None

		self.addMenu( '''Send to Center''', 'center' )
		lowerLinkShape = breve.createInstances( breve.Cube, 1 ).initWith( breve.vector( 0.360000, 1, 0.360000 ) )
		linkShape = breve.createInstances( breve.Cube, 1 ).initWith( breve.vector( 0.380000, 1, 0.380000 ) )
		bodyShape = breve.createInstances( breve.Cube, 1 ).initWith( breve.vector( 4, 3, 0.400000 ) )
		self.links = breve.createInstances( breve.Links, 8 )
		self.joints = breve.createInstances( breve.RevoluteJoints, 8 )
		self.links[ 0 ].setShape( linkShape )
		self.links[ 2 ].setShape( linkShape )
		self.links[ 4 ].setShape( linkShape )
		self.links[ 6 ].setShape( linkShape )
		self.links[ 1 ].setShape( lowerLinkShape )
		self.links[ 3 ].setShape( lowerLinkShape )
		self.links[ 5 ].setShape( lowerLinkShape )
		self.links[ 7 ].setShape( lowerLinkShape )
		self.links.setColor( breve.randomExpression( breve.vector( 1.000000, 1.000000, 1.000000 ) ) )
		self.bodyLink = breve.createInstances( breve.Link, 1 )
		self.bodyLink.setShape( bodyShape )
		self.joints[ 0 ].link( self.bodyLink, self.links[ 0 ], breve.vector( 0, 0, 1 ), breve.vector( 2.000000, -1.500000, 0 ), breve.vector( 0, 0.500000, 0 ) )
		self.joints[ 1 ].link( self.links[ 0 ], self.links[ 1 ], breve.vector( 1, 0, 0 ), breve.vector( 0, -0.500000, 0 ), breve.vector( 0, 0.500000, 0 ) )
		self.joints[ 4 ].link( self.bodyLink, self.links[ 4 ], breve.vector( 0, 0, 1 ), breve.vector( -2.000000, -1.500000, 0 ), breve.vector( 0, 0.500000, 0 ) )
		self.joints[ 5 ].link( self.links[ 4 ], self.links[ 5 ], breve.vector( 1, 0, 0 ), breve.vector( 0, -0.500000, 0 ), breve.vector( 0, 0.500000, 0 ) )
		self.joints[ 2 ].link( self.bodyLink, self.links[ 2 ], breve.vector( 0, 0, 1 ), breve.vector( 2.000000, 1.500000, 0 ), breve.vector( 0, -0.500000, 0 ) )
		self.joints[ 3 ].link( self.links[ 2 ], self.links[ 3 ], breve.vector( 1, 0, 0 ), breve.vector( 0, 0.500000, 0 ), breve.vector( 0, -0.500000, 0 ) )
		self.joints[ 6 ].link( self.bodyLink, self.links[ 6 ], breve.vector( 0, 0, 1 ), breve.vector( -2.000000, 1.500000, 0 ), breve.vector( 0, -0.500000, 0 ) )
		self.joints[ 7 ].link( self.links[ 6 ], self.links[ 7 ], breve.vector( 1, 0, 0 ), breve.vector( 0, 0.500000, 0 ), breve.vector( 0, -0.500000, 0 ) )
		self.register( self.bodyLink )
		self.rotate( breve.vector( 1, 0, 0 ), 1.570000 )
		self.joints.setDoubleSpring( 400, 0.800000, -0.800000 )
		self.joints.setStrengthLimit( 300 )

	def setJointVelocity0( self, value ):

		self.joints[ 0 ].setJointVelocity( value )
		self.joints[ 6 ].setJointVelocity( ( -value ) )

	def setJointVelocity1( self, value ):

		self.joints[ 1 ].setJointVelocity( value )
		self.joints[ 7 ].setJointVelocity( ( -value ) )

	def setJointVelocity2( self, value ):

		self.joints[ 2 ].setJointVelocity( value )
		self.joints[ 4 ].setJointVelocity( ( -value ) )

	def setJointVelocity3( self, value ):

		self.joints[ 3 ].setJointVelocity( value )
		self.joints[ 5 ].setJointVelocity( ( -value ) )


breve.Creature = Creature


# Create an instance of our controller object to initialize the simulation

Walker()


