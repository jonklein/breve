
# Note: this file was automatically converted to Python from the
# original steve-language source code.  Please see the original
# file for more detailed comments and documentation.


import breve

class Swarm( breve.Control ):
	def __init__( self ):
		breve.Control.__init__( self )
		self.autoCameraMenu = None
		self.autoCameraMode = 0
		self.births = 0
		self.cameraHasBeenAimed = 0
		self.corpseShape = None
		self.deaths = 0
		self.decisions = 0
		self.discrepancies = 0
		self.drawEveryFrameMenu = None
		self.feeders = breve.objectList()
		self.foodAccum = 0
		self.foodDeviations = 0
		self.friendExploits = 0
		self.friendFeedings = 0
		self.iteration = 0
		self.manualCameraMenu = None
		self.maxAimDelta = 0
		self.maxZoomDelta = 0
		self.naturalBirths = 0
		self.nextID = 0
		self.otherExploits = 0
		self.otherFeedings = 0
		self.prevCameraAim = breve.vector()
		self.prevCameraZoom = 0
		self.printVelocitiesVar = 0
		self.provisionalNewZoom = 0
		self.randomBirdsAdded = 0
		self.reproMutationRates = 0
		self.reproMutations = 0
		self.reproductiveDiscounts = 0
		self.runID = ''
		self.selection = None
		self.servos = 0
		self.shareMode = 0
		self.skipFramesIfNecessaryMenu = None
		self.stability = 0
		self.unitDriftMode = 0
		Swarm.init( self )

	def addRandomBirdsIfNecessary( self ):
		numBirds = 0

		numBirds = breve.length( breve.allInstances( "Birds" ) )
		if ( numBirds < 10 ):
			breve.createInstances( breve.Bird, 1 ).initializeRandomly()
			self.randomBirdsAdded = ( self.randomBirdsAdded + 1 )
			self.addRandomBirdsIfNecessary()



	def adjustCamera( self, location ):
		topDiff = 0
		item = None

		topDiff = 0.000000
		for item in breve.allInstances( "Birds" ):
			if ( topDiff < breve.length( ( location - item.getLocation() ) ) ):
				topDiff = breve.length( ( location - item.getLocation() ) )



		self.provisionalNewZoom = ( ( 0.500000 * topDiff ) + 10 )
		if ( self.cameraHasBeenAimed == 0 ):
			self.aimCamera( location )
			self.zoomCamera( self.provisionalNewZoom )
			self.prevCameraAim = location
			self.prevCameraZoom = self.provisionalNewZoom
			self.cameraHasBeenAimed = 1

		else:
			if ( breve.length( ( self.prevCameraAim - location ) ) < self.maxAimDelta ):
				self.aimCamera( location )
				self.prevCameraAim = location

			else:
				self.aimCamera( ( ( ( ( location - self.prevCameraAim ) / breve.length( ( location - self.prevCameraAim ) ) ) * self.maxAimDelta ) + self.prevCameraAim ) )
				self.prevCameraAim = ( ( ( ( location - self.prevCameraAim ) / breve.length( ( location - self.prevCameraAim ) ) ) * self.maxAimDelta ) + self.prevCameraAim )


			if ( breve.breveInternalFunctionFinder.abs( self, ( self.prevCameraZoom - self.provisionalNewZoom ) ) < self.maxZoomDelta ):
				self.zoomCamera( self.provisionalNewZoom )
				self.prevCameraZoom = self.provisionalNewZoom

			else:
				if ( ( self.prevCameraZoom - self.provisionalNewZoom ) < 0 ):
					self.zoomCamera( ( self.prevCameraZoom + self.maxZoomDelta ) )
					self.prevCameraZoom = ( self.prevCameraZoom + self.maxZoomDelta )

				else:
					self.zoomCamera( ( self.prevCameraZoom - self.maxZoomDelta ) )
					self.prevCameraZoom = ( self.prevCameraZoom - self.maxZoomDelta )







	def catchInterfaceId100( self, s ):
		self.controller.setStability( s )
		self.setInterfaceItem( 102, s )

	def catchInterfaceId102( self, s ):
		self.controller.setStability( s )
		self.setInterfaceItem( 100, s )

	def catchInterfaceId300( self, s ):
		if ( s == 1 ):
			self.setAutoCameraMode()
		else:
			if ( s == 0 ):
				self.setManualCameraMode()



	def catchInterfaceId600( self, s ):
		self.controller.setUnitDriftMode( s )

	def click( self, item ):
		self.selection = item
		if ( item and item.isA( 'Bird' ) ):
			print item.getPushCode().getString()

		breve.Control.click( self , item )

	def displayFoodSupply( self ):
		total = 0
		displayTotal = 0
		feeder = None

		total = 0.000000
		for feeder in self.feeders:
			total = ( total + feeder.getEnergy() )


		total = ( total / breve.length( self.feeders ) )
		displayTotal = ( total * 100 )
		self.setInterfaceItem( 101, '''%s''' % (  displayTotal ) )
		self.foodAccum = ( self.foodAccum + total )

	def displayIteration( self ):
		self.setInterfaceItem( 124, '''%s''' % (  self.iteration ) )

	def displayPopulation( self ):
		popsize = 0

		popsize = breve.allInstances( "Birds" )
		self.setInterfaceItem( 123, '''%s''' % (  popsize ) )

	def doPrintVelocities( self ):
		self.printVelocitiesVar = 1

	def dontPrintVelocities( self ):
		self.printVelocitiesVar = 0

	def dumpPrograms( self ):
		b = None

		for b in breve.allInstances( "Birds" ):
			b.getPushCode().dump()


	def getCorpseShape( self ):
		return self.corpseShape

	def getDecisions( self ):
		return self.decisions

	def getFeeders( self ):
		return self.feeders

	def getFoodDeviations( self ):
		return self.foodDeviations

	def getNextID( self ):
		return self.nextID

	def getShareMode( self ):
		return self.shareMode

	def getStability( self ):
		return self.stability

	def getUnitDriftMode( self ):
		return self.unitDriftMode

	def incrementBirths( self ):
		self.births = ( self.births + 1 )

	def incrementDeaths( self ):
		self.deaths = ( self.deaths + 1 )

	def incrementDiscrepancies( self, d ):
		self.discrepancies = ( self.discrepancies + d )

	def incrementFriendExploits( self ):
		self.friendExploits = ( self.friendExploits + 1 )

	def incrementFriendFeedings( self ):
		self.friendFeedings = ( self.friendFeedings + 1 )

	def incrementNaturalBirths( self ):
		self.naturalBirths = ( self.naturalBirths + 1 )

	def incrementNextID( self ):
		self.nextID = ( self.nextID + 1 )

	def incrementOtherExploits( self ):
		self.otherExploits = ( self.otherExploits + 1 )

	def incrementOtherFeedings( self ):
		self.otherFeedings = ( self.otherFeedings + 1 )

	def incrementReproMutationRates( self, r ):
		self.reproMutationRates = ( self.reproMutationRates + r )

	def incrementReproMutations( self ):
		self.reproMutations = ( self.reproMutations + 1 )

	def incrementReproductiveDiscounts( self ):
		self.reproductiveDiscounts = ( self.reproductiveDiscounts + 1 )

	def incrementServos( self ):
		self.servos = ( self.servos + 1 )

	def init( self ):
		floor = None
		i = 0

		self.stability = 1100
		if ( self.getArgumentCount() > 1 ):
			self.runID = self.getArgument( 1 )
			self.shareMode = self.getArgument( 2 )
			print '''running with runID = %s''' % (  self.runID )
			print '''running with shareMode = %s''' % (  self.shareMode )


		self.nextID = 0
		self.births = 0
		breve.createInstances( breve.Birds, 10 )
		breve.allInstances( "Birds" ).initializeRandomly()
		self.feeders = breve.createInstances( breve.Feeders, 20 )
		self.randomBirdsAdded = 10
		floor = breve.createInstances( breve.Stationary, 1 )
		floor.register( breve.createInstances( breve.Cube, 1 ).initWith( breve.vector( 100, 2, 100 ) ), breve.vector( 0, -5, 0 ) )
		floor.catchShadows()
		self.deaths = 0
		self.decisions = 0
		self.foodDeviations = 0
		self.friendFeedings = 0
		self.otherFeedings = 0
		self.reproductiveDiscounts = 0
		self.reproMutations = 0
		self.reproMutationRates = 0.000000
		self.discrepancies = 0.000000
		self.naturalBirths = 0
		self.corpseShape = breve.createInstances( breve.PolygonCone, 1 ).initWith( 5, 0.200000, 0.060000 )
		self.servos = 0
		self.enableLighting()
		self.moveLight( breve.vector( 0, 20, 20 ) )
		self.autoCameraMenu = self.addMenu( '''Automatic Camera Control''', 'setAutoCameraMode' )
		self.manualCameraMenu = self.addMenu( '''Manual Camera Control''', 'setManualCameraMode' )
		self.autoCameraMode = 1
		self.autoCameraMenu.check()
		self.manualCameraMenu.uncheck()
		self.unitDriftMode = 0
		i = 0
		self.setInterfaceItem( 666, '''%s''' % (  i ) )
		self.addMenuSeparator()
		self.drawEveryFrameMenu = self.addMenu( '''Draw Every Frame''', 'setDrawEveryFrame' )
		self.skipFramesIfNecessaryMenu = self.addMenu( '''Skip Frames if Necessary''', 'setSkipFramesIfNecessaryMenu' )
		self.drawEveryFrameMenu.check()
		self.skipFramesIfNecessaryMenu.uncheck()
		self.setBackgroundTextureImage( breve.createInstances( breve.Image, 1 ).load( 'images/clouds.png' ) )
		self.offsetCamera( breve.vector( 5, 1.500000, 6 ) )
		self.cameraHasBeenAimed = 0
		self.maxZoomDelta = 0.050000
		self.maxAimDelta = 0.050000
		self.enableShadows()
		self.disableText()

	def iterate( self ):
		item = None
		swarmCenter = breve.vector()
		numBirds = 0

		numBirds = breve.length( breve.allInstances( "Birds" ) )
		self.iteration = ( self.iteration + 1 )
		if ( self.iteration > 6000 ):
			raise Exception( '''%s iterations complete''' % (  self.iteration ) )


		self.updateNeighbors()
		swarmCenter = breve.vector( 0, 0, 0 )
		for item in breve.allInstances( "Birds" ):
			if item:
				item.fly()

			if item:
				swarmCenter = ( swarmCenter + item.getLocation() )



		swarmCenter = ( swarmCenter / numBirds )
		if self.autoCameraMode:
			self.adjustCamera( swarmCenter )

		for item in breve.allInstances( "Birds" ):
			item.adjustSize()

		for item in breve.allInstances( "Birds" ):
			if ( ( item.getEnergy() == 0 ) or ( item.getAge() > 150 ) ):
				item.dropDead()


		self.addRandomBirdsIfNecessary()
		self.feeders.maybeTeleport()
		self.controller.displayFoodSupply()
		self.controller.displayPopulation()
		self.controller.displayIteration()
		if ( ( self.iteration % 100 ) == 0 ):
			self.report()

		breve.Control.iterate( self )

	def printVelocities( self ):
		return self.printVelocitiesVar

	def randomFeeder( self ):
		return self.feeders[ breve.randomExpression( ( self.feeders - 1 ) ) ]

	def report( self ):
		item2 = None
		item = None
		avgFood = 0
		devFood = 0
		numOthers = 0
		hueDiversity = 0
		sizeDiversity = 0
		avProgSize = 0
		numBirds = 0
		oCount = 0
		fCount = 0
		i = 0

		numBirds = breve.length( breve.allInstances( "Birds" ) )
		avgFood = ( self.foodAccum / 100 )
		if ( self.decisions == 0 ):
			devFood = 0.000000

		else:
			devFood = ( self.foodDeviations / self.decisions )


		hueDiversity = 0.000000
		for item in breve.allInstances( "Birds" ):
			numOthers = 0
			for item2 in breve.allInstances( "Birds" ):
				if item.isOther( item2 ):
					numOthers = ( numOthers + 1 )


			hueDiversity = ( hueDiversity + ( numOthers / ( numBirds - 1 ) ) )


		hueDiversity = ( hueDiversity / numBirds )
		sizeDiversity = 0.000000
		for item in breve.allInstances( "Birds" ):
			fCount = ( fCount + item.askIfFriendFeeder() )
			oCount = ( oCount + item.askIfOtherFeeder() )
			if item:
				numOthers = 0
				for item2 in breve.allInstances( "Birds" ):
					if item2:
						if ( ( item.getPushCode().getSize() == item2.getPushCode().getSize() ) == 0 ):
							numOthers = ( numOthers + 1 )



				sizeDiversity = ( sizeDiversity + ( numOthers / ( numBirds - 1 ) ) )




		sizeDiversity = ( sizeDiversity / numBirds )
		avProgSize = 0.000000
		for item in breve.allInstances( "Birds" ):
			if item:
				avProgSize = ( avProgSize + item.getPushCode().getSize() )


		avProgSize = ( avProgSize / numBirds )
		if ( self.naturalBirths > 0 ):
			self.discrepancies = ( self.discrepancies / self.naturalBirths )
			self.reproMutationRates = ( self.reproMutationRates / self.naturalBirths )
			self.reproMutationRates = ( 1 / ( self.reproMutationRates + 1 ) )


		print '''iteration: %s, population size: %s, hue diversity: %s, size diversity: %s, average program size: %s''' % (  self.iteration, numBirds, hueDiversity, sizeDiversity, avProgSize )
		print '''  [ epoch: b: %s, d: %s, spnt: %s, rmut: %s, rate: %s, diversification: %s, food: %s, fdev: %s, feed/f: %s, /o: %s, discounts: %s, servos: %s, fCount: %s, oCount: %s ]''' % (  self.births, self.deaths, self.randomBirdsAdded, self.reproMutations, self.reproMutationRates, self.discrepancies, avgFood, devFood, self.friendFeedings, self.otherFeedings, self.reproductiveDiscounts, self.servos, fCount, oCount )
		print ''' '''
		i = ( self.iteration / 100 )
		self.setInterfaceItem( 699, '''%s''' % (  i ) )
		self.setInterfaceItem( 700, '''%s''' % (  self.births ) )
		self.setInterfaceItem( 698, '''%s''' % (  self.randomBirdsAdded ) )
		self.setInterfaceItem( 701, '''%s''' % (  self.deaths ) )
		self.setInterfaceItem( 702, '''%s''' % (  self.reproMutations ) )
		self.setInterfaceItem( 703, '''%s''' % (  self.reproMutationRates ) )
		self.setInterfaceItem( 704, '''%s''' % (  self.discrepancies ) )
		self.setInterfaceItem( 705, '''%s''' % (  self.friendFeedings ) )
		self.setInterfaceItem( 706, '''%s''' % (  self.otherFeedings ) )
		self.setInterfaceItem( 707, '''%s''' % (  self.servos ) )
		self.births = 0
		self.deaths = 0
		self.foodAccum = 0
		self.foodDeviations = 0
		self.decisions = 0
		self.friendFeedings = 0
		self.otherFeedings = 0
		self.friendExploits = 0
		self.otherExploits = 0
		self.randomBirdsAdded = 0
		self.reproductiveDiscounts = 0
		self.reproMutations = 0
		self.reproMutationRates = 0.000000
		self.discrepancies = 0.000000
		self.naturalBirths = 0
		self.servos = 0

	def scode( self ):
		self.controller.getSelection().getPushCode().dump()

	def setAutoCameraMode( self ):
		self.autoCameraMode = 1
		self.autoCameraMenu.check()
		self.manualCameraMenu.uncheck()

	def setDecisions( self, d ):
		self.decisions = d

	def setDrawEveryFrame( self ):
		self.drawEveryFrameMenu.check()
		self.skipFramesIfNecessaryMenu.uncheck()
		self.enableDrawEveryFrame()

	def setFoodDeviations( self, d ):
		self.foodDeviations = d

	def setManualCameraMode( self ):
		self.autoCameraMode = 0
		self.autoCameraMenu.uncheck()
		self.manualCameraMenu.check()

	def setSkipFramesIfNecessaryMenu( self ):
		self.drawEveryFrameMenu.uncheck()
		self.skipFramesIfNecessaryMenu.check()
		self.disableDrawEveryFrame()

	def setStability( self, value ):
		self.stability = value

	def setUnitDriftMode( self, m ):
		self.unitDriftMode = m


breve.Swarm = Swarm
class Feeders( breve.Mobile ):
	def __init__( self ):
		breve.Mobile.__init__( self )
		self.driftLocation = breve.vector()
		self.drifting = 0
		self.energy = 0
		self.lastScale = 0
		self.shape = None
		Feeders.init( self )

	def adjustEnergy( self, d ):
		self.energy = ( self.energy + d )
		if ( self.energy < 0 ):
			self.energy = 0

		if ( self.energy > 1 ):
			self.energy = 1


	def drift( self, location ):
		self.drifting = 1
		self.driftLocation = location

	def getEnergy( self ):
		return self.energy

	def init( self ):
		self.shape = breve.createInstances( breve.Sphere, 1 ).initWith( 0.300000 )
		self.setShape( self.shape )
		self.move( ( breve.randomExpression( breve.vector( 30, 0, 30 ) ) - breve.vector( 15, 0, 15 ) ) )
		self.setVelocity( breve.vector( 0, 0, 0 ) )
		self.energy = 1
		self.setColor( breve.vector( 1, 1, 1 ) )
		self.lastScale = 1

	def iterate( self ):
		newScale = 0
		radius = 0

		if self.drifting:
			if self.controller.getUnitDriftMode():
				self.move( self.driftLocation )
				self.drifting = 0

			else:
				self.offset( ( 0.060000 * ( self.driftLocation - self.getLocation() ) ) )
				if ( breve.length( ( self.driftLocation - self.getLocation() ) ) < 0.001000 ):
					self.move( self.driftLocation )
					self.drifting = 0





		self.energy = ( self.energy + 0.001000 )
		if ( self.energy > 1 ):
			self.energy = 1

		radius = breve.breveInternalFunctionFinder.sqrt( self, self.energy )
		newScale = ( ( radius * 2 ) + 0.000010 )
		if ( newScale == self.lastScale ):
			return

		newScale = ( newScale / self.lastScale )
		self.shape.scale( breve.vector( newScale, newScale, newScale ) )
		self.lastScale = ( ( radius * 2 ) + 0.000010 )

	def maybeTeleport( self ):
		loc = breve.vector()

		if ( breve.randomExpression( self.controller.getStability() ) == 0 ):
			if self.controller.getUnitDriftMode():
				loc = self.getLocation()
				self.drift( breve.vector( ( loc.x + ( 0.100000 * ( breve.randomExpression( 2 ) - 1 ) ) ), 0, ( loc.z + ( 0.100000 * ( breve.randomExpression( 2 ) - 1 ) ) ) ) )

			else:
				self.drift( ( breve.randomExpression( breve.vector( 20, 0, 20 ) ) - breve.vector( ( 20 / 2 ), 0, ( 20 / 2 ) ) ) )






breve.Feeders = Feeders
class Corpse( breve.Mobile ):
	def __init__( self ):
		breve.Mobile.__init__( self )
		self.age = 0
		self.landed = 0
		Corpse.init( self )

	def init( self ):
		self.setShape( self.controller.getCorpseShape() )
		self.handleCollisions( 'Stationary', 'land' )
		self.setColor( breve.vector( 0.200000, 0.200000, 0.200000 ) )
		self.age = 0
		self.landed = 0

	def iterate( self ):
		self.age = ( self.age + 1 )
		if ( self.landed == 0 ):
			self.setAcceleration( breve.vector( 0, -10, 0 ) )

		if ( self.age > 10 ):
			breve.deleteInstances( self )


	def land( self, ground ):
		self.setAcceleration( breve.vector( 0, 0, 0 ) )
		self.setVelocity( breve.vector( 0, 0, 0 ) )
		self.landed = 1
		self.offset( breve.vector( 0, 0.100000, 0 ) )


breve.Corpse = Corpse
class Bird( breve.Mobile ):
	def __init__( self ):
		breve.Mobile.__init__( self )
		self.ID = 0
		self.age = 0
		self.energy = 0
		self.friendFeeder = 0
		self.hue = 0
		self.landed = 0
		self.lastScale = 0
		self.otherFeeder = 0
		self.pushCode = None
		self.pushInterpreter = None
		self.servoGain = 0
		self.servoSetpoint = 0
		self.shape = None
		Bird.init( self )

	def adjustEnergy( self, d ):
		self.energy = ( self.energy + d )
		if ( self.energy < 0 ):
			self.energy = 0

		if ( self.energy > 1 ):
			self.energy = 1


	def adjustSize( self ):
		newScale = 0

		newScale = ( ( self.energy * 2 ) + 0.500000 )
		self.shape.scale( breve.vector( ( newScale / self.lastScale ), 1, ( newScale / self.lastScale ) ) )
		self.lastScale = newScale

	def askIfFriendFeeder( self ):
		return self.friendFeeder

	def askIfOtherFeeder( self ):
		return self.otherFeeder

	def bump( self, bird ):
		self.adjustEnergy( ( 0 - 0.010000 ) )
		bird.adjustEnergy( ( 0 - 0.010000 ) )

	def checkLanded( self ):
		return self.landed

	def closestFriend( self ):
		friends = breve.objectList()
		item = None
		closestDistance = 0
		thisDistance = 0
		closestFriend = None

		for item in self.getNeighbors():
			if item.isA( 'Bird' ):
				if self.isFriend( item ):
					friends.append( item )





		if ( breve.length( friends ) < 1 ):
			closestFriend = self

		else:
			closestDistance = 1000000
			for item in friends:
				thisDistance = breve.length( ( item.getLocation() - self.getLocation() ) )
				if ( thisDistance < closestDistance ):
					closestDistance = thisDistance
					closestFriend = item






		return closestFriend

	def closestOther( self ):
		others = breve.objectList()
		item = None
		closestDistance = 0
		thisDistance = 0
		closestOther = None

		for item in self.getNeighbors():
			if item.isA( 'Bird' ):
				if ( self.isFriend( item ) == 0 ):
					others.append( item )





		if ( breve.length( others ) < 1 ):
			closestOther = self

		else:
			closestDistance = 1000000
			for item in others:
				thisDistance = breve.length( ( item.getLocation() - self.getLocation() ) )
				if ( thisDistance < closestDistance ):
					closestDistance = thisDistance
					closestOther = item






		return closestOther

	def crossover( self ):
		c3 = None
		c2 = None
		c1 = None

		c1 = breve.createInstances( breve.PushProgram, 1 )
		c2 = breve.createInstances( breve.PushProgram, 1 )
		c3 = breve.createInstances( breve.PushProgram, 1 )
		self.pushInterpreter.copyCodeStackTop( c1 )
		if ( c1.getSize() > 0 ):
			self.pushInterpreter.popCodeStack()
			self.pushInterpreter.copyCodeStackTop( c2 )
			if ( c2.getSize() > 0 ):
				self.pushInterpreter.popCodeStack()
				c3.crossover( c1, c2, self.pushInterpreter )
				self.pushInterpreter.pushCode( c3 )

			else:
				self.pushInterpreter.pushCode( c1 )




		breve.deleteInstances( c1 )
		breve.deleteInstances( c2 )
		breve.deleteInstances( c3 )

	def dropDead( self ):
		c = None

		self.controller.incrementDeaths()
		c = breve.createInstances( breve.Corpse, 1 )
		c.move( self.getLocation() )
		c.point( breve.vector( 0, 1, 0 ), self.getVelocity() )
		breve.deleteInstances( self.pushCode )
		breve.deleteInstances( self.pushInterpreter )
		breve.deleteInstances( self.shape )
		breve.deleteInstances( self )

	def eat( self, feeder ):
		if ( feeder.getEnergy() > 0.050000 ):
			self.adjustEnergy( ( 0.050000 * 10 ) )
			feeder.adjustEnergy( ( 0 - 0.050000 ) )



	def feedFriend( self ):
		bird = None
		mode = 0

		self.friendFeeder = 1
		self.controller.incrementFriendFeedings()
		bird = self.closestFriend()
		if ( bird.getPushCode().getDiscrepancy( self.pushCode ) > 9 ):
			return

		if ( bird.getEnergy() < self.energy ):
			self.adjustEnergy( -0.010000 )
			bird.adjustEnergy( 0.010000 )



	def feedOther( self ):
		bird = None
		mode = 0

		self.otherFeeder = 1
		self.controller.incrementOtherFeedings()
		bird = self.closestOther()
		if ( bird.getEnergy() < self.energy ):
			self.adjustEnergy( -0.010000 )
			bird.adjustEnergy( 0.010000 )



	def fly( self ):
		acceleration = breve.vector()
		newVelocity = breve.vector()
		takeOff = 0
		topFloat = 0
		topVector = breve.vector()
		savedToFood = breve.vector()
		hsv = breve.vector()
		dev = 0

		self.age = ( self.age + 1 )
		self.energy = ( self.energy - 0.000100 )
		if ( self.energy < 0 ):
			self.energy = 0

		if self.landed:
			takeOff = breve.randomExpression( 40 )
			if ( takeOff == 1 ):
				self.landed = 0
				self.setVelocity( ( breve.randomExpression( breve.vector( 0.100000, 1.100000, 0.100000 ) ) - breve.vector( 0.050000, 0, 0.050000 ) ) )

			else:
				return




		if ( ( breve.randomExpression( 0 ) == 0 ) or ( self.age < 2 ) ):
			self.pushInterpreter.clearStacks()
			self.toFood()
			savedToFood = self.pushInterpreter.getVectorStackTop()
			self.pushInterpreter.run( self.pushCode )
			topFloat = self.pushInterpreter.getFloatStackTop()
			topVector = self.pushInterpreter.getVectorStackTop()
			if ( breve.breveInternalFunctionFinder.isinf( self, topFloat ) or breve.breveInternalFunctionFinder.isnan( self, topFloat ) ):
				topFloat = 0.000000


			if ( ( ( ( ( breve.breveInternalFunctionFinder.isinf( self, topVector.x ) or breve.breveInternalFunctionFinder.isnan( self, topVector.x ) ) or breve.breveInternalFunctionFinder.isinf( self, topVector.y ) ) or breve.breveInternalFunctionFinder.isnan( self, topVector.y ) ) or breve.breveInternalFunctionFinder.isinf( self, topVector.z ) ) or breve.breveInternalFunctionFinder.isnan( self, topVector.z ) ):
				topVector = breve.vector( 0.000000, 0.000000, 0.000000 )


			self.hue = ( topFloat % 1.000000 )
			hsv = breve.vector( ( self.hue * 360 ), 1, 1 )
			self.setColor( self.controller.getRgbColor( hsv ) )
			acceleration = topVector
			self.setAcceleration( acceleration )
			self.controller.setDecisions( ( self.controller.getDecisions() + 1 ) )
			if ( ( breve.length( topVector ) != 0.000000 ) and ( breve.length( savedToFood ) != 0.000000 ) ):
				dev = breve.breveInternalFunctionFinder.angle( self, topVector, savedToFood )

			if ( breve.breveInternalFunctionFinder.isnan( self, dev ) or breve.breveInternalFunctionFinder.isinf( self, dev ) ):
				dev = 0.000000
				print savedToFood, topVector


			self.controller.setFoodDeviations( ( self.controller.getFoodDeviations() + dev ) )


		newVelocity = self.getVelocity()
		if ( breve.length( newVelocity ) > 2 ):
			newVelocity = ( ( 2 * newVelocity ) / breve.length( newVelocity ) )
			self.setVelocity( newVelocity )


		if self.controller.printVelocities():
			print '''velocity: %s''' % (  newVelocity )

		self.point( breve.vector( 0, 1, 0 ), newVelocity )

	def foodIntensity( self ):
		toClosestFood = breve.vector()
		toThisFood = breve.vector()
		item = None
		highestStrength = 0
		strengthThisFood = 0

		highestStrength = 0
		for item in self.controller.getFeeders():
			toThisFood = ( item.getLocation() - self.getLocation() )
			strengthThisFood = ( item.getEnergy() / ( ( breve.length( toThisFood ) * breve.length( toThisFood ) ) + 0.001000 ) )
			if ( strengthThisFood > highestStrength ):
				highestStrength = strengthThisFood
				toClosestFood = toThisFood




		self.pushInterpreter.pushFloat( highestStrength )

	def friendAge( self ):
		self.pushInterpreter.pushInteger( self.closestFriend().getAge() )

	def friendEnergy( self ):
		self.pushInterpreter.pushInteger( self.closestFriend().getEnergy() )

	def friendHue( self ):
		self.pushInterpreter.pushInteger( self.closestFriend().getHue() )

	def friendLocation( self ):
		self.pushInterpreter.pushVector( self.closestFriend().getLocation() )

	def friendProgram( self ):
		c = None

		c = breve.createInstances( breve.PushProgram, 1 )
		c.parse( self.closestFriend().getPushCode().getString() )
		self.pushInterpreter.pushCode( c )
		breve.deleteInstances( c )

	def friendVelocity( self ):
		self.pushInterpreter.pushVector( self.closestFriend().getVelocity() )

	def getID( self ):
		return self.ID

	def getAge( self ):
		return self.age

	def getAngle( self, otherMobile ):
		tempVector = breve.vector()

		tempVector = ( otherMobile.getLocation() - self.getLocation() )
		if ( breve.length( tempVector ) == 0 ):
			return 0

		return breve.breveInternalFunctionFinder.angle( self, self.getVelocity(), tempVector )

	def getEnergy( self ):
		return self.energy

	def getFedByFriend( self ):
		if ( not self.friendFeeder ):
			self.controller.incrementFriendExploits()


	def getFedByOther( self ):
		if ( not self.otherFeeder ):
			self.controller.incrementOtherExploits()


	def getHue( self ):
		return self.hue

	def getPushCode( self ):
		return self.pushCode

	def getPushInterpreter( self ):
		return self.pushInterpreter

	def hueDiff( self, bird ):
		otherHue = 0

		otherHue = bird.getHue()
		return breve.breveInternalFunctionFinder.min( self, ( breve.breveInternalFunctionFinder.max( self, self.hue, otherHue ) - breve.breveInternalFunctionFinder.min( self, self.hue, otherHue ) ), ( breve.breveInternalFunctionFinder.min( self, self.hue, otherHue ) + ( 1 - breve.breveInternalFunctionFinder.max( self, self.hue, otherHue ) ) ) )

	def init( self ):
		self.shape = breve.createInstances( breve.PolygonCone, 1 ).initWith( 5, 0.200000, 0.100000 )
		self.setShape( self.shape )
		self.handleCollisions( 'Stationary', 'land' )
		self.handleCollisions( 'Feeders', 'eat' )
		self.handleCollisions( 'Birds', 'bump' )
		self.setNeighborhoodSize( 1.000000 )
		self.age = 0
		self.lastScale = 1
		self.controller.incrementBirths()
		self.servoSetpoint = 0.500000
		self.servoGain = 0.100000
		self.ID = self.controller.getNextID()
		self.controller.incrementNextID()
		self.pushInterpreter = breve.createInstances( breve.PushInterpreter, 1 )
		self.pushInterpreter.readConfig( 'SwarmEvolve.config' )
		self.pushInterpreter.addInstruction( self, 'mutate' )
		self.pushInterpreter.addInstruction( self, 'crossover' )
		self.pushInterpreter.addInstruction( self, 'spawn' )
		self.pushInterpreter.addInstruction( self, 'randI' )
		self.pushInterpreter.addInstruction( self, 'randF' )
		self.pushInterpreter.addInstruction( self, 'randV' )
		self.pushInterpreter.addInstruction( self, 'randC' )
		self.pushInterpreter.addInstruction( self, 'setServoSetpoint' )
		self.pushInterpreter.addInstruction( self, 'setServoGain' )
		self.pushInterpreter.addInstruction( self, 'servo' )
		self.pushInterpreter.addInstruction( self, 'toFood' )
		self.pushInterpreter.addInstruction( self, 'foodIntensity' )
		self.pushInterpreter.addInstruction( self, 'myAge' )
		self.pushInterpreter.addInstruction( self, 'myEnergy' )
		self.pushInterpreter.addInstruction( self, 'myHue' )
		self.pushInterpreter.addInstruction( self, 'myVelocity' )
		self.pushInterpreter.addInstruction( self, 'myLocation' )
		self.pushInterpreter.addInstruction( self, 'myProgram' )
		self.pushInterpreter.addInstruction( self, 'toFriend' )
		self.pushInterpreter.addInstruction( self, 'friendAge' )
		self.pushInterpreter.addInstruction( self, 'friendEnergy' )
		self.pushInterpreter.addInstruction( self, 'friendHue' )
		self.pushInterpreter.addInstruction( self, 'friendVelocity' )
		self.pushInterpreter.addInstruction( self, 'friendLocation' )
		self.pushInterpreter.addInstruction( self, 'friendProgram' )
		self.pushInterpreter.addInstruction( self, 'feedFriend' )
		self.pushInterpreter.addInstruction( self, 'toOther' )
		self.pushInterpreter.addInstruction( self, 'otherAge' )
		self.pushInterpreter.addInstruction( self, 'otherEnergy' )
		self.pushInterpreter.addInstruction( self, 'otherHue' )
		self.pushInterpreter.addInstruction( self, 'otherVelocity' )
		self.pushInterpreter.addInstruction( self, 'otherLocation' )
		self.pushInterpreter.addInstruction( self, 'otherProgram' )
		self.pushInterpreter.addInstruction( self, 'feedOther' )
		self.pushInterpreter.setEvaluationLimit( 50 )
		self.pushInterpreter.setListLimit( 50 )
		self.pushCode = breve.createInstances( breve.PushProgram, 1 )

	def initializeRandomly( self ):
		self.pushCode.makeRandomCode( self.pushInterpreter, 80 )
		self.move( ( breve.randomExpression( breve.vector( 10, 0, 10 ) ) - breve.vector( 5, 3, 5 ) ) )
		self.setVelocity( breve.vector( 0, 0, 0 ) )
		self.hue = breve.randomExpression( 1.000000 )
		self.setColor( self.controller.getRgbColor( breve.vector( ( self.hue * 360 ), 1, 1 ) ) )
		self.energy = ( 0.800000 + breve.randomExpression( 0.200000 ) )

	def isFriend( self, bird ):
		if ( self.hueDiff( bird ) <= 0.100000 ):
			return 1
		else:
			return 0


	def isOther( self, bird ):
		if ( self.hueDiff( bird ) > 0.100000 ):
			return 1
		else:
			return 0


	def land( self, ground ):
		self.setAcceleration( breve.vector( 0, 0, 0 ) )
		self.setVelocity( breve.vector( 0, 0, 0 ) )
		self.point( breve.vector( 0, 1, 0 ), breve.vector( 0, 1, 0 ) )
		self.landed = 1
		self.offset( breve.vector( 0, 0.100000, 0 ) )

	def mutate( self ):
		c = None
		size = 0

		size = ( self.pushInterpreter.getIntegerStackTop() % 15 )
		if ( size > 0 ):
			c = breve.createInstances( breve.PushProgram, 1 )
			self.pushInterpreter.copyCodeStackTop( c )
			c.mutate( self.pushInterpreter, size )
			self.pushInterpreter.popIntegerStack()
			self.pushInterpreter.popCodeStack()
			self.pushInterpreter.pushCode( c )
			breve.deleteInstances( c )



	def myAge( self ):
		self.pushInterpreter.pushInteger( self.age )

	def myEnergy( self ):
		self.pushInterpreter.pushFloat( self.energy )

	def myHue( self ):
		self.pushInterpreter.pushFloat( self.hue )

	def myLocation( self ):
		self.pushInterpreter.pushVector( self.getLocation() )

	def myProgram( self ):
		c = None

		c = breve.createInstances( breve.PushProgram, 1 )
		c.parse( self.pushCode.getString() )
		self.pushInterpreter.pushCode( c )
		breve.deleteInstances( c )

	def myVelocity( self ):
		self.pushInterpreter.pushVector( self.getVelocity() )

	def otherAge( self ):
		self.pushInterpreter.pushInteger( self.closestOther().getAge() )

	def otherEnergy( self ):
		self.pushInterpreter.pushInteger( self.closestOther().getEnergy() )

	def otherHue( self ):
		self.pushInterpreter.pushInteger( self.closestOther().getHue() )

	def otherLocation( self ):
		self.pushInterpreter.pushVector( self.closestOther().getLocation() )

	def otherProgram( self ):
		c = None

		c = breve.createInstances( breve.PushProgram, 1 )
		c.parse( self.closestOther().getPushCode().getString() )
		self.pushInterpreter.pushCode( c )
		breve.deleteInstances( c )

	def otherVelocity( self ):
		self.pushInterpreter.pushVector( self.closestOther().getVelocity() )

	def randC( self ):
		c = None

		c = breve.createInstances( breve.PushProgram, 1 )
		c.makeRandomCode( self.pushInterpreter, 80 )
		self.pushInterpreter.pushCode( c )
		breve.deleteInstances( c )

	def randF( self ):
		self.pushInterpreter.pushFloat( ( breve.randomExpression( ( 2 * 1.000000 ) ) - 1.000000 ) )

	def randI( self ):
		self.pushInterpreter.pushInteger( ( breve.randomExpression( ( 2 * 100 ) ) - 100 ) )

	def randV( self ):
		self.pushInterpreter.pushVector( ( breve.randomExpression( ( 2 * breve.vector( 1.000000, 1.000000, 1.000000 ) ) ) - breve.vector( 1.000000, 1.000000, 1.000000 ) ) )

	def scaleColor( self, c, m ):
		return ( m + ( c * ( 1.000000 - m ) ) )

	def servo( self ):
		val = 0
		err = 0

		val = self.pushInterpreter.getFloatStackTop()
		err = ( val - self.servoSetpoint )
		self.pushInterpreter.popFloatStack()
		self.pushInterpreter.pushFloat( ( val + ( err * ( -self.servoGain ) ) ) )
		self.controller.incrementServos()

	def setEnergy( self, e ):
		self.energy = e

	def setHue( self, h ):
		self.hue = h

	def setPushCode( self, c ):
		self.pushCode = c

	def setServoGain( self ):
		g = 0

		g = self.pushInterpreter.getFloatStackTop()
		if ( ( g < -0.010000 ) or ( g > 0.010000 ) ):
			self.servoGain = g

		self.pushInterpreter.popFloatStack()

	def setServoSetpoint( self ):
		self.servoSetpoint = self.pushInterpreter.getFloatStackTop()
		self.pushInterpreter.popFloatStack()

	def spawn( self ):
		child = None
		tempCode = None
		numBirds = 0
		mutRate = 0

		numBirds = breve.allInstances( "Birds" )
		if ( self.energy > ( 2 * 0.150000 ) ):
			self.controller.incrementNaturalBirths()
			if ( numBirds < 0 ):
				self.adjustEnergy( ( 0 - ( 0.150000 - ( ( ( 0 - numBirds ) * 0.150000 ) / ( 0 - 10 ) ) ) ) )
				self.controller.incrementReproductiveDiscounts()

			else:
				self.adjustEnergy( ( 0 - 0.150000 ) )

			child = breve.createInstances( breve.Bird, 1 )
			child.setEnergy( 0.150000 )
			child.setHue( self.getHue() )
			child.setColor( self.controller.getRgbColor( breve.vector( ( child.getHue() * 360 ), 1, 1 ) ) )
			child.move( ( self.getLocation() + ( breve.randomExpression( breve.vector( ( 1.000000 * 2 ), ( 1.000000 * 2 ), ( 1.000000 * 2 ) ) ) - breve.vector( 1.000000, 1.000000, 1.000000 ) ) ) )
			child.setVelocity( breve.vector( 0, 0, 0 ) )
			child.point( breve.vector( 0, 1, 0 ), breve.randomExpression( breve.vector( 1, 1, 1 ) ) )
			tempCode = breve.createInstances( breve.PushProgram, 1 )
			self.pushInterpreter.copyCodeStackTop( tempCode )
			mutRate = self.pushInterpreter.getIntegerStackTop()
			self.pushInterpreter.popIntegerStack()
			if ( mutRate < 0 ):
				mutRate = ( -mutRate )

			self.controller.incrementReproMutationRates( mutRate )
			if ( breve.randomExpression( mutRate ) == 0 ):
				tempCode.mutate( self.pushInterpreter, 15 )
				self.controller.incrementReproMutations()


			child.getPushCode().parse( tempCode.getString() )
			breve.deleteInstances( tempCode )



	def toFood( self ):
		toClosestFood = breve.vector()
		toThisFood = breve.vector()
		item = None
		highestStrength = 0
		strengthThisFood = 0

		highestStrength = 0
		for item in self.controller.getFeeders():
			toThisFood = ( item.getLocation() - self.getLocation() )
			strengthThisFood = ( item.getEnergy() / ( ( breve.length( toThisFood ) * breve.length( toThisFood ) ) + 0.001000 ) )
			if ( strengthThisFood > highestStrength ):
				highestStrength = strengthThisFood
				toClosestFood = toThisFood




		self.pushInterpreter.pushVector( toClosestFood )

	def toFriend( self ):
		self.pushInterpreter.pushVector( ( self.closestFriend().getLocation() - self.getLocation() ) )

	def toOther( self ):
		self.pushInterpreter.pushVector( ( self.closestOther().getLocation() - self.getLocation() ) )


breve.Bird = Bird
# Add our newly created classes to the breve namespace

breve.Birds = Bird


# Create an instance of our controller object to initialize the simulation

Swarm()


