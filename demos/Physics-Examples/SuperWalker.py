
# Note: this file was automatically converted to Python from the
# original steve-language source code.  Please see the original
# file for more detailed comments and documentation.


import breve

class Walker( breve.PhysicalControl ):
	def __init__( self ):
		breve.PhysicalControl.__init__( self )
		self.currentSeat = 0
		self.floor = None
		self.lockMenu = None
		self.locked = 0
		self.monkeys = breve.objectList()
		self.seats = breve.objectList()
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
		if ( breve.length( newOffset ) < 9 ):
			newOffset = ( ( 9 * newOffset ) / breve.length( newOffset ) )

		self.panCameraOffset( newOffset, 30 )
		self.seats[ self.currentSeat ].setDistance( breve.length( self.wigglyThing.getLocation() ) )
		self.currentSeat = ( self.currentSeat + 1 )
		if ( self.currentSeat > 3 ):
			self.breedNewMonkeys()
			self.pickDrivers()


		newDriver = self.seats[ self.currentSeat ].getNumber()
		if self.wigglyThing:
			breve.deleteInstances( self.wigglyThing )


		self.wigglyThing = breve.createInstances( breve.Creature, 1 )
		self.wigglyThing.initWith( self.seats[ self.currentSeat ].getGenome() )
		self.wigglyThing.move( breve.vector( 0, 3, 0 ) )
		self.watch( self.wigglyThing )
		self.schedule( 'changeDrivers', ( self.getTime() + 20.000000 ) )
		self.displayCurrentDriver()

	def compareDistance( self, a, b ):
		result = 0

		result = ( b.getDistance() - a.getDistance() )
		return result

	def displayCurrentDriver( self ):
		current = None

		current = self.seats[ self.currentSeat ]
		self.setDisplayText( '''Driver #%s''' % (  self.currentSeat ), -0.950000, -0.900000 )

	def init( self ):
		number = 0
		item = None

		self.locked = 0
		self.enableLighting()
		self.enableSmoothDrawing()
		self.enableFastPhysics()
		self.setFastPhysicsIterations( 5 )
		self.floor = breve.createInstances( breve.Floor, 1 )
		self.floor.catchShadows()
		self.floor.setET( 0.900000 )
		self.floor.setMu( 1.000000 )
		self.setBackgroundTextureImage( breve.createInstances( breve.Image, 1 ).load( 'images/clouds.png' ) )
		self.enableReflections()
		self.setBackgroundColor( breve.vector( 0.400000, 0.600000, 0.900000 ) )
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
		self.wigglyThing = breve.createInstances( breve.Creature, 1 )
		self.wigglyThing.initWith( self.seats[ 0 ].getGenome() )
		self.wigglyThing.move( breve.vector( 0, 3, 0 ) )
		self.offsetCamera( breve.vector( -3, 3, 13 ) )
		self.watch( self.wigglyThing )

	def iterate( self ):
		self.seats[ self.currentSeat ].control( self.wigglyThing, self.getTime() )
		breve.PhysicalControl.iterate( self )

	def loadDrivers( self ):
		n = 0

		n = 0
		while ( n < 15 ):
			self.monkeys[ n ].getGenome().loadFromXml( '''driver%s.xml''' % (  n ) )

			n = ( n + 1 )

		self.pickDrivers()
		print self.monkeys[ self.currentSeat ]

	def loadIntoCurrentGenome( self ):
		self.seats[ self.currentSeat ].getGenome().loadWithDialog()

	def pickDrivers( self ):
		
		self.seats[ 0 ] = self.monkeys[ 0 ]
		self.seats[ 1 ] = self.monkeys[ 1 ]
		self.seats[ 2 ] = self.monkeys[ 2 ]
		self.seats[ 3 ] = self.monkeys[ 3 ]
		self.currentSeat = 0

	def randomSort( self, a, b ):
		return ( breve.randomExpression( 2 ) - 1 )

	def saveCurrentGenome( self ):
		self.seats[ self.currentSeat ].getGenome().saveWithDialog()

	def saveDrivers( self ):
		driver = None
		n = 0

		for driver in self.monkeys:
			self.monkeys[ n ] = driver.getGenome().saveAsXml( '''driver%s.xml''' % (  n ) )
			n = ( n + 1 )



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
		n = 0

		while ( n < 8 ):
			theRobot.setJointVelocity( n, ( 10 * self.genome.calculateTorque( n, t ) ) )
			n = ( n + 1 )



	def getDistance( self ):
		return self.distanceTraveled

	def getGenome( self ):
		return self.genome

	def getNumber( self ):
		return self.number

	def init( self ):
		self.genome = breve.createInstances( breve.MonkeyGenome, 1 )
		self.genome.randomize()
		self.addDependency( self.genome )

	def setDistance( self, value ):
		self.distanceTraveled = value

	def setNumber( self, n ):
		self.number = n


breve.Monkeys = Monkeys
class MonkeyGenome( breve.Object ):
	def __init__( self ):
		breve.Object.__init__( self )
		self.ampShifts = breve.objectList()
		self.limbLengths = breve.objectList()
		self.phaseShifts = breve.objectList()
		self.waveCompression = 0
		MonkeyGenome.init( self )

	def calculateTorque( self, jointNum, time ):
		return ( ( 0.500000 * breve.breveInternalFunctionFinder.sin( self, ( self.waveCompression * ( time + self.phaseShifts[ jointNum ] ) ) ) ) - self.ampShifts[ jointNum ] )

	def getLimbLength( self, n ):
		return self.limbLengths[ n ]

	def init( self ):
		self.randomize()

	def mutate( self ):
		n = 0

		n = breve.randomExpression( 25 )
		if ( n < 8 ):
			self.ampShifts[ n ] = ( breve.randomExpression( 2.000000 ) - 1.000000 )
		else:
			if ( n < 16 ):
				self.phaseShifts[ ( n - 8 ) ] = ( breve.randomExpression( 6.300000 ) - 3.150000 )
			else:
				if ( n < 24 ):
					self.limbLengths[ ( n - 16 ) ] = ( breve.randomExpression( 2.000000 ) + 0.500000 )
				else:
					self.waveCompression = ( breve.randomExpression( 5.000000 ) - 2.500000 )



		print '''mutated item %s of %s''' % (  n, self )

	def randomize( self ):
		n = 0

		n = 0
		while ( n < 8 ):
			self.phaseShifts[ n ] = ( breve.randomExpression( 6.300000 ) - 3.150000 )
			self.ampShifts[ n ] = ( breve.randomExpression( 1.000000 ) - 0.500000 )
			self.limbLengths[ n ] = ( breve.randomExpression( 2.000000 ) + 0.500000 )

			n = ( n + 1 )

		self.waveCompression = ( breve.randomExpression( 5.000000 ) - 2.500000 )


breve.MonkeyGenome = MonkeyGenome
class Creature( breve.MultiBody ):
	def __init__( self ):
		breve.MultiBody.__init__( self )
		self.bodyLink = None
		self.bodyShape = None
		self.joints = breve.objectList()
		self.linkShape = None
		self.links = breve.objectList()
		self.lowerLinkShape = None

	def center( self ):
		currentLocation = breve.vector()

		currentLocation = self.getLocation()
		self.move( breve.vector( 0, currentLocation.y, 0 ) )

	def destroy( self ):
		breve.deleteInstances( self.links )
		breve.deleteInstances( self.joints )
		breve.deleteInstances( self.bodyLink )
		breve.deleteInstances( self.linkShape )
		breve.deleteInstances( self.lowerLinkShape )
		breve.deleteInstances( self.bodyShape )
		breve.MultiBody.destroy( self )

	def getRoot( self ):
		return self.bodyLink

	def initWith( self, g ):
		y = 0
		x = 0
		upperLimbSize = breve.vector()
		lowerLimbSize = breve.vector()
		upperLimbLinkPoint = breve.vector()
		lowerLimbLinkPoint = breve.vector()
		counter = 0

		self.addMenu( '''Send to Center''', 'center' )
		y = g.getLimbLength( 0 )
		x = breve.breveInternalFunctionFinder.sqrt( self, ( 0.160000 / y ) )
		x = breve.breveInternalFunctionFinder.sqrt( self, ( 0.160000 / y ) )
		lowerLimbSize = breve.vector( x, y, x )
		lowerLimbLinkPoint = breve.vector( 0, ( y / 2 ), 0 )
		y = g.getLimbLength( 1 )
		x = breve.breveInternalFunctionFinder.sqrt( self, ( 0.160000 / y ) )
		x = breve.breveInternalFunctionFinder.sqrt( self, ( 0.160000 / y ) )
		upperLimbSize = breve.vector( x, y, x )
		upperLimbLinkPoint = breve.vector( 0, ( y / 2 ), 0 )
		self.lowerLinkShape = breve.createInstances( breve.Cube, 1 ).initWith( lowerLimbSize )
		self.linkShape = breve.createInstances( breve.Cube, 1 ).initWith( upperLimbSize )
		self.bodyShape = breve.createInstances( breve.Cube, 1 ).initWith( breve.vector( 4, 3, 0.400000 ) )
		counter = 0
		self.links = breve.createInstances( breve.Links, 8 )
		self.joints = breve.createInstances( breve.RevoluteJoints, 8 )
		self.links[ 0 ].setShape( self.linkShape )
		self.links[ 2 ].setShape( self.linkShape )
		self.links[ 4 ].setShape( self.linkShape )
		self.links[ 6 ].setShape( self.linkShape )
		self.links[ 1 ].setShape( self.lowerLinkShape )
		self.links[ 3 ].setShape( self.lowerLinkShape )
		self.links[ 5 ].setShape( self.lowerLinkShape )
		self.links[ 7 ].setShape( self.lowerLinkShape )
		self.links.setColor( breve.randomExpression( breve.vector( 1.000000, 1.000000, 1.000000 ) ) )
		self.bodyLink = breve.createInstances( breve.Link, 1 )
		self.bodyLink.setShape( self.bodyShape )
		self.setRoot( self.bodyLink )
		self.joints[ 0 ].link( breve.vector( 0, 0, 1 ), breve.vector( 2.000000, -1.500000, 0 ), upperLimbLinkPoint, self.links[ 0 ], self.bodyLink )
		self.joints[ 1 ].link( breve.vector( 1, 0, 0 ), ( -upperLimbLinkPoint ), lowerLimbLinkPoint, self.links[ 1 ], self.links[ 0 ] )
		self.joints[ 4 ].link( breve.vector( 0, 0, 1 ), breve.vector( -2.000000, -1.500000, 0 ), upperLimbLinkPoint, self.links[ 4 ], self.bodyLink )
		self.joints[ 5 ].link( breve.vector( 1, 0, 0 ), ( -upperLimbLinkPoint ), lowerLimbLinkPoint, self.links[ 5 ], self.links[ 4 ] )
		self.joints[ 2 ].link( breve.vector( 0, 0, 1 ), breve.vector( 2.000000, 1.500000, 0 ), ( -upperLimbLinkPoint ), self.links[ 2 ], self.bodyLink )
		self.joints[ 3 ].link( breve.vector( 1, 0, 0 ), upperLimbLinkPoint, ( -lowerLimbLinkPoint ), self.links[ 3 ], self.links[ 2 ] )
		self.joints[ 6 ].link( breve.vector( 0, 0, 1 ), breve.vector( -2.000000, 1.500000, 0 ), ( -upperLimbLinkPoint ), self.links[ 6 ], self.bodyLink )
		self.joints[ 7 ].link( breve.vector( 1, 0, 0 ), upperLimbLinkPoint, ( -lowerLimbLinkPoint ), self.links[ 7 ], self.links[ 6 ] )
		self.rotate( breve.vector( 1, 0, 0 ), 1.570000 )
		self.joints.setDoubleSpring( 200, 0.600000, -0.600000 )
		self.joints.setStrengthLimit( 250 )
		self.joints.setCfm( 0.010000 )
		self.controller.setCfm( 0.010000 )
		self.enableSelfCollisions()

	def setJointVelocity( self, jointNum, value ):
		self.joints[ jointNum ].setJointVelocity( value )


breve.Creature = Creature


# Create an instance of our controller object to initialize the simulation

Walker()


