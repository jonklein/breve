
# Note: this file was automatically converted to Python from the
# original steve-language source code.  Please see the original
# file for more detailed comments and documentation.


import breve

class Swarm( breve.Control ):
	def __init__( self ):
		breve.Control.__init__( self )
		self.autoCameraMenu = None
		self.autoCameraMode = 0
		self.birds = breve.objectList()
		self.breveTexture = 0
		self.cloudTexture = None
		self.deaths = 0
		self.doMutations = 0
		self.drawEveryFrameMenu = None
		self.feeders = breve.objectList()
		self.item = None
		self.iteration = 0
		self.manualCameraMenu = None
		self.selection = None
		self.skipFramesIfNecessaryMenu = None
		Swarm.init( self )

	def bestFellow( self, bird ):
		otherBird = None
		otherScore = 0
		bestBird = None
		bestScore = 0
		species = 0

		bestScore = 0
		bestBird = bird
		species = bird.checkSpecies()
		for otherBird in self.birds:
			if ( otherBird.checkSpecies() == species ):
				otherScore = ( otherBird.getAge() * otherBird.getEnergy() )
				if ( otherScore > bestScore ):
					bestBird = otherBird
					bestScore = otherScore






		return bestBird

	def checkDoMutations( self ):
		return self.doMutations

	def click( self, item ):
		if self.selection:
			self.selection.hideNeighborLines()

		if item:
			item.showNeighborLines()

		self.selection = item
		breve.Control.click( self , item )

	def evolveWithoutRandomizing( self ):
		self.doMutations = 1

	def flockNicely( self ):
		for self.item in self.birds:
			self.item.flockNicely()

		self.doMutations = 0

	def getBirds( self ):
		return self.birds

	def init( self ):
		floor = None

		self.enableLighting()
		self.moveLight( breve.vector( 0, 20, 20 ) )
		self.autoCameraMenu = self.addMenu( '''Automatic Camera Control''', 'setAutoCameraMode' )
		self.manualCameraMenu = self.addMenu( '''Manual Camera Control''', 'setManualCameraMode' )
		self.autoCameraMode = 1
		self.autoCameraMenu.check()
		self.manualCameraMenu.uncheck()
		self.addMenuSeparator()
		self.addMenu( '''Flock Nicely''', 'flockNicely' )
		self.addMenu( '''Randomize and Evolve''', 'randomizeAndEvolve' )
		self.addMenu( '''Evolve without Randomizing''', 'evolveWithoutRandomizing' )
		self.addMenuSeparator()
		self.drawEveryFrameMenu = self.addMenu( '''Draw Every Frame''', 'setDrawEveryFrame' )
		self.skipFramesIfNecessaryMenu = self.addMenu( '''Skip Frames if Necessary''', 'setSkipFramesIfNecessaryMenu' )
		self.drawEveryFrameMenu.check()
		self.skipFramesIfNecessaryMenu.uncheck()
		self.cloudTexture = breve.createInstances( breve.Image, 1 ).load( 'images/clouds.png' )
		self.setBackgroundTextureImage( self.cloudTexture )
		floor = breve.createInstances( breve.Stationary, 1 )
		floor.register( breve.createInstances( breve.Cube, 1 ).initWith( breve.vector( 100, 2, 100 ) ), breve.vector( 0, -5, 0 ) )
		floor.catchShadows()
		self.birds = breve.createInstances( breve.Birds, 60 )
		self.feeders = breve.createInstances( breve.Feeders, 2 )
		self.randomizeAndEvolve()
		self.offsetCamera( breve.vector( 5, 1.500000, 6 ) )
		self.enableShadows()
		self.disableText()

	def iterate( self ):
		location = breve.vector()
		velocity = breve.vector()
		topDiff = 0

		self.updateNeighbors()
		for self.item in self.birds:
			self.item.fly( self.birds, self.feeders )
			location = ( location + self.item.getLocation() )


		location = ( location / breve.length( self.birds ) )
		topDiff = 0.000000
		for self.item in self.birds:
			if ( topDiff < breve.length( ( location - self.item.getLocation() ) ) ):
				topDiff = breve.length( ( location - self.item.getLocation() ) )



		if self.autoCameraMode:
			self.aimCamera( location )
			self.zoomCamera( ( ( 0.500000 * topDiff ) + 10 ) )


		for self.item in self.birds:
			if ( self.item.getEnergy() == 0 ):
				self.item.dieAndSpawn( self.bestFellow( self.item ) )
				self.deaths = ( self.deaths + 1 )




		self.iteration = ( self.iteration + 1 )
		if ( self.iteration > 100 ):
			print '''deaths: %s''' % (  self.deaths )
			self.iteration = 0
			self.deaths = 0


		for self.item in self.feeders:
			self.item.maybeTeleport()


		breve.Control.iterate( self )

	def randomizeAndEvolve( self ):
		for self.item in self.birds:
			self.item.flockRandomly()

		self.doMutations = 1

	def setAutoCameraMode( self ):
		self.autoCameraMode = 1
		self.autoCameraMenu.check()
		self.manualCameraMenu.uncheck()

	def setBirds( self, newBirds ):
		self.birds = newBirds

	def setDrawEveryFrame( self ):
		self.drawEveryFrameMenu.check()
		self.skipFramesIfNecessaryMenu.uncheck()
		self.enableDrawEveryFrame()

	def setManualCameraMode( self ):
		self.autoCameraMode = 0
		self.autoCameraMenu.uncheck()
		self.manualCameraMenu.check()

	def setSkipFramesIfNecessaryMenu( self ):
		self.drawEveryFrameMenu.uncheck()
		self.skipFramesIfNecessaryMenu.check()
		self.disableDrawEveryFrame()


breve.Swarm = Swarm
class Feeders( breve.Mobile ):
	def __init__( self ):
		breve.Mobile.__init__( self )
		self.driftLocation = breve.vector()
		self.drifting = 0
		Feeders.init( self )

	def drift( self, location ):
		self.drifting = 1
		self.driftLocation = location

	def init( self ):
		self.setShape( breve.createInstances( breve.Shape, 1 ).initWithPolygonDisk( 5, 0.100000, 1.000000 ) )
		self.move( ( breve.randomExpression( breve.vector( 20, 0, 20 ) ) - breve.vector( 10, 0, 10 ) ) )
		self.setVelocity( breve.vector( 0, 0, 0 ) )
		self.setColor( breve.vector( 1, 1, 0.300000 ) )

	def iterate( self ):
		if self.drifting:
			self.offset( ( 0.060000 * ( self.driftLocation - self.getLocation() ) ) )
			if ( breve.length( ( self.driftLocation - self.getLocation() ) ) < 0.001000 ):
				self.move( self.driftLocation )
				self.drifting = 0





	def maybeTeleport( self ):
		if ( breve.randomExpression( 250 ) == 0 ):
			self.drift( ( breve.randomExpression( breve.vector( 20, 0, 20 ) ) - breve.vector( 10, 0, 10 ) ) )




breve.Feeders = Feeders
class Birds( breve.Mobile ):
	def __init__( self ):
		breve.Mobile.__init__( self )
		self.age = 0
		self.centerConstant = 0
		self.cruiseDistance = 0
		self.energy = 0
		self.feederConstant = 0
		self.landed = 0
		self.maxAcceleration = 0
		self.maxVelocity = 0
		self.myVelocity = breve.vector()
		self.spacingConstant = 0
		self.species = 0
		self.velocityConstant = 0
		self.wanderConstant = 0
		self.worldCenterConstant = 0
		self.xenoDistance = 0
		self.xenoSpacingConstant = 0
		Birds.init( self )

	def adjustColor( self ):
		myColor = breve.vector()
		newRed = 0
		newGreen = 0
		newBlue = 0

		myColor = self.getColor()
		if ( self.species == 0 ):
			newRed = myColor.x
			newGreen = myColor.y
			newBlue = self.scaleColor( self.energy, 0.300000 )

		else:
			if ( self.species == 1 ):
				newRed = self.scaleColor( self.energy, 0.300000 )
				newGreen = myColor.y
				newBlue = myColor.z

			else:
				if ( self.species == 2 ):
					newRed = self.scaleColor( self.energy, 0.300000 )
					newGreen = myColor.y
					newBlue = self.scaleColor( self.energy, 0.300000 )




		self.setColor( breve.vector( newRed, newGreen, newBlue ) )

	def checkLanded( self ):
		return self.landed

	def checkSpecies( self ):
		return self.species

	def checkVisibility( self, item ):
		if ( item == self ):
			return 0

		if ( not item.isA( 'Birds' ) ):
			return 0

		if ( self.getAngle( item ) > 2.000000 ):
			return 0

		if item.checkLanded():
			return 0

		return 1

	def cross( self, v1, v2 ):
		z = 0
		y = 0
		x = 0

		x = ( ( v1.y * v2.z ) - ( v1.z * v2.y ) )
		y = ( ( v1.z * v2.x ) - ( v1.x * v2.z ) )
		z = ( ( v1.x * v2.y ) - ( v1.y * v2.x ) )
		return breve.vector( x, y, z )

	def dieAndSpawn( self, p ):
		parentNewEnergy = 0

		self.energy = ( 0.500000 + breve.randomExpression( 0.200000 ) )
		self.age = 0
		self.wanderConstant = self.mutate( p.getWanderConstant(), 0.500000, 15.000000, 0.100000 )
		self.worldCenterConstant = self.mutate( p.getWorldCenterConstant(), 0.500000, 15.000000, 0.100000 )
		self.centerConstant = self.mutate( p.getCenterConstant(), 0.500000, 15.000000, 0.100000 )
		self.velocityConstant = self.mutate( p.getVelocityConstant(), 0.500000, 15.000000, 0.100000 )
		self.spacingConstant = self.mutate( p.getSpacingConstant(), 0.500000, 15.000000, 0.100000 )
		self.feederConstant = self.mutate( p.getFeederConstant(), 0.500000, 15.000000, 0.100000 )
		self.xenoSpacingConstant = self.mutate( p.getXenoSpacingConstant(), 0.500000, 15.000000, 0.100000 )
		self.cruiseDistance = self.mutate( p.getCruiseDistance(), 0.500000, 15.000000, 0.100000 )
		self.xenoDistance = self.mutate( p.getXenoDistance(), 0.500000, 15.000000, 0.100000 )
		self.maxVelocity = self.mutate( p.getMaxVelocity(), 0.500000, 15.000000, 0.100000 )
		self.maxAcceleration = self.mutate( p.getMaxAcceleration(), 0.500000, 15.000000, 0.100000 )
		parentNewEnergy = p.getEnergy()
		parentNewEnergy = ( parentNewEnergy - 0.001000 )
		if ( parentNewEnergy < 0 ):
			parentNewEnergy = 0

		p.setEnergy( parentNewEnergy )
		p.adjustColor()

	def eat( self, feeder ):
		self.energy = ( self.energy + 0.020000 )
		if ( self.energy > 1 ):
			self.energy = 1


	def flockNicely( self ):
		self.wanderConstant = 3.000000
		self.worldCenterConstant = 4.000000
		self.centerConstant = 1.000000
		self.velocityConstant = 3.000000
		self.spacingConstant = 4.000000
		self.feederConstant = 3.000000
		self.xenoSpacingConstant = 20.000000
		self.cruiseDistance = 0.400000
		self.xenoDistance = 5.000000
		self.maxVelocity = 15
		self.maxAcceleration = 15

	def flockRandomly( self ):
		self.wanderConstant = ( breve.randomExpression( 14.900000 ) + 0.100000 )
		self.worldCenterConstant = ( breve.randomExpression( 14.900000 ) + 0.100000 )
		self.centerConstant = ( breve.randomExpression( 14.900000 ) + 0.100000 )
		self.velocityConstant = ( breve.randomExpression( 14.900000 ) + 0.100000 )
		self.spacingConstant = ( breve.randomExpression( 14.900000 ) + 0.100000 )
		self.feederConstant = ( breve.randomExpression( 14.900000 ) + 0.100000 )
		self.xenoSpacingConstant = ( breve.randomExpression( 14.900000 ) + 0.100000 )
		self.cruiseDistance = ( breve.randomExpression( 14.900000 ) + 0.100000 )
		self.xenoDistance = ( breve.randomExpression( 14.900000 ) + 0.100000 )
		self.maxVelocity = ( breve.randomExpression( 14.900000 ) + 0.100000 )
		self.maxAcceleration = ( breve.randomExpression( 14.900000 ) + 0.100000 )

	def fly( self, flock, feeders ):
		bird = None
		toNeighbor = breve.vector()
		centerUrge = breve.vector()
		worldCenterUrge = breve.vector()
		velocityUrge = breve.vector()
		spacingUrge = breve.vector()
		xenoSpacingUrge = breve.vector()
		wanderUrge = breve.vector()
		feederUrge = breve.vector()
		acceleration = breve.vector()
		newVelocity = breve.vector()
		takeOff = 0
		conspecifics = breve.objectList()
		others = breve.objectList()
		neighborConspecifics = breve.objectList()
		neighborOthers = breve.objectList()

		self.age = ( self.age + 1 )
		self.energy = ( self.energy - 0.010000 )
		if ( self.energy < 0 ):
			self.energy = 0

		for bird in self.getNeighbors():
			if bird.isA( 'Birds' ):
				if ( bird.checkSpecies() == self.species ):
					neighborConspecifics.append( bird )

				else:
					neighborOthers.append( bird )


				if ( breve.length( ( self.getLocation() - bird.getLocation() ) ) < 0.250000 ):
					self.energy = ( self.energy - 0.250000 )
					if ( self.energy < 0 ):
						self.energy = 0







		if ( neighborOthers > neighborConspecifics ):
			self.energy = ( self.energy - 0.100000 )
			if ( self.energy < 0 ):
				self.energy = 0



		self.adjustColor()
		for bird in self.getNeighbors():
			if self.checkVisibility( bird ):
				if ( self.checkSpecies() == bird.checkSpecies() ):
					conspecifics.append( bird )
				else:
					others.append( bird )




		if self.landed:
			takeOff = breve.randomExpression( 40 )
			if ( takeOff == 1 ):
				self.landed = 0
				self.setVelocity( ( breve.randomExpression( breve.vector( 0.100000, 1.100000, 0.100000 ) ) - breve.vector( 0.050000, 0, 0.050000 ) ) )

			else:
				return




		centerUrge = self.getCenterUrge( conspecifics )
		velocityUrge = self.getVelocityUrge( conspecifics )
		for bird in conspecifics:
			toNeighbor = ( self.getLocation() - bird.getLocation() )
			if ( breve.length( toNeighbor ) < self.cruiseDistance ):
				spacingUrge = ( spacingUrge + toNeighbor )



		for bird in others:
			toNeighbor = ( self.getLocation() - bird.getLocation() )
			if ( breve.length( toNeighbor ) < self.xenoDistance ):
				xenoSpacingUrge = ( xenoSpacingUrge + toNeighbor )



		if ( breve.length( self.getLocation() ) > 10 ):
			worldCenterUrge = ( -self.getLocation() )

		wanderUrge = ( breve.randomExpression( breve.vector( 2, 2, 2 ) ) - breve.vector( 1, 1, 1 ) )
		feederUrge = self.getFeederUrge( feeders )
		if breve.length( spacingUrge ):
			spacingUrge = ( spacingUrge / breve.length( spacingUrge ) )

		if breve.length( xenoSpacingUrge ):
			xenoSpacingUrge = ( xenoSpacingUrge / breve.length( xenoSpacingUrge ) )

		if breve.length( worldCenterUrge ):
			worldCenterUrge = ( worldCenterUrge / breve.length( worldCenterUrge ) )

		if breve.length( velocityUrge ):
			velocityUrge = ( velocityUrge / breve.length( velocityUrge ) )

		if breve.length( centerUrge ):
			centerUrge = ( centerUrge / breve.length( centerUrge ) )

		if breve.length( wanderUrge ):
			wanderUrge = ( wanderUrge / breve.length( wanderUrge ) )

		if breve.length( feederUrge ):
			feederUrge = ( feederUrge / breve.length( feederUrge ) )

		wanderUrge = ( wanderUrge * self.wanderConstant )
		worldCenterUrge = ( worldCenterUrge * self.worldCenterConstant )
		centerUrge = ( centerUrge * self.centerConstant )
		velocityUrge = ( velocityUrge * self.velocityConstant )
		spacingUrge = ( spacingUrge * self.spacingConstant )
		xenoSpacingUrge = ( xenoSpacingUrge * self.xenoSpacingConstant )
		feederUrge = ( feederUrge * self.feederConstant )
		acceleration = ( ( ( ( ( ( worldCenterUrge + centerUrge ) + velocityUrge ) + spacingUrge ) + wanderUrge ) + feederUrge ) + xenoSpacingUrge )
		if ( breve.length( acceleration ) != 0 ):
			acceleration = ( acceleration / breve.length( acceleration ) )

		self.setAcceleration( ( self.maxAcceleration * acceleration ) )
		newVelocity = self.getVelocity()
		if ( breve.length( newVelocity ) > self.maxVelocity ):
			newVelocity = ( ( self.maxVelocity * newVelocity ) / breve.length( newVelocity ) )

		self.setVelocity( newVelocity )
		self.myPoint( breve.vector( 0, 1, 0 ), newVelocity )

	def getAge( self ):
		return self.age

	def getAngle( self, otherMobile ):
		tempVector = breve.vector()

		tempVector = ( otherMobile.getLocation() - self.getLocation() )
		if ( ( breve.length( self.getVelocity() ) == 0.000000 ) or ( breve.length( tempVector ) == 0.000000 ) ):
			return 0.000000

		return breve.breveInternalFunctionFinder.angle( self, self.getVelocity(), tempVector )

	def getCenterUrge( self, flock ):
		item = None
		count = 0
		center = breve.vector()

		for item in flock:
			count = ( count + 1 )
			center = ( center + item.getLocation() )


		if ( count == 0 ):
			return breve.vector( 0, 0, 0 )

		center = ( center / count )
		return ( center - self.getLocation() )

	def getCenterConstant( self ):
		return self.centerConstant

	def getCruiseDistance( self ):
		return self.cruiseDistance

	def getEnergy( self ):
		return self.energy

	def getFeederUrge( self, feeders ):
		closestFood = breve.vector()
		thisFood = breve.vector()
		item = None
		closestDistance = 0
		thisDistance = 0

		closestDistance = 10000.000000
		for item in feeders:
			thisFood = ( item.getLocation() - self.getLocation() )
			thisDistance = breve.length( thisFood )
			if ( thisDistance < closestDistance ):
				closestFood = thisFood
				closestDistance = thisDistance




		return closestFood

	def getFeederConstant( self ):
		return self.feederConstant

	def getMaxAcceleration( self ):
		return self.maxAcceleration

	def getMaxVelocity( self ):
		return self.maxVelocity

	def getSpacingConstant( self ):
		return self.spacingConstant

	def getVelocityUrge( self, flock ):
		item = None
		count = 0
		velocity = breve.vector()

		for item in flock:
			count = ( count + 1 )
			velocity = ( velocity + item.getVelocity() )


		if ( count == 0 ):
			return breve.vector( 0, 0, 0 )

		velocity = ( velocity / count )
		return ( velocity - self.getVelocity() )

	def getVelocityConstant( self ):
		return self.velocityConstant

	def getWanderConstant( self ):
		return self.wanderConstant

	def getWorldCenterConstant( self ):
		return self.worldCenterConstant

	def getXenoDistance( self ):
		return self.xenoDistance

	def getXenoSpacingConstant( self ):
		return self.xenoSpacingConstant

	def init( self ):
		self.setShape( breve.createInstances( breve.Shape, 1 ).initWithPolygonCone( 5, 0.200000, 0.100000 ) )
		self.move( ( breve.randomExpression( breve.vector( 10, 10, 10 ) ) - breve.vector( 5, -5, 5 ) ) )
		self.setVelocity( ( breve.randomExpression( breve.vector( 20, 20, 20 ) ) - breve.vector( 10, 10, 10 ) ) )
		self.species = breve.randomExpression( 2 )
		if ( self.species == 0 ):
			self.setColor( breve.vector( 0.100000, 0.100000, 1 ) )

		else:
			if ( self.species == 1 ):
				self.setColor( breve.vector( 1, 0.100000, 0.100000 ) )

			else:
				if ( self.species == 2 ):
					self.setColor( breve.vector( 1, 0.100000, 1 ) )




		self.energy = ( 0.500000 + breve.randomExpression( 0.200000 ) )
		self.handleCollisions( 'Stationary', 'land' )
		self.handleCollisions( 'Feeders', 'eat' )
		self.setNeighborhoodSize( 3.000000 )
		self.age = 0

	def land( self, ground ):
		self.setAcceleration( breve.vector( 0, 0, 0 ) )
		self.setVelocity( breve.vector( 0, 0, 0 ) )
		self.myPoint( breve.vector( 0, 1, 0 ), breve.vector( 0, 1, 0 ) )
		self.landed = 1
		self.offset( breve.vector( 0, 0.100000, 0 ) )

	def max( self, num1, num2 ):
		if ( num1 < num2 ):
			return num2

		else:
			return num1



	def min( self, num1, num2 ):
		if ( num1 < num2 ):
			return num1

		else:
			return num2



	def mutate( self, n, d, max, min ):
		newValue = 0

		if self.controller.checkDoMutations():
			newValue = ( n + ( breve.randomExpression( ( 2 * d ) ) - d ) )

		else:
			newValue = n


		if ( newValue < min ):
			newValue = min

		if ( newValue > max ):
			newValue = max

		return newValue

	def myPoint( self, theVertex, theLocation ):
		v = breve.vector()
		a = 0

		v = self.cross( theVertex, theLocation )
		a = breve.breveInternalFunctionFinder.angle( self, theVertex, theLocation )
		if ( breve.length( v ) == 0.000000 ):
			self.rotate( theVertex, 0.100000 )
			return


		self.rotate( v, a )

	def scaleColor( self, c, m ):
		return ( m + ( c * ( 1.000000 - m ) ) )

	def setEnergy( self, e ):
		self.energy = e


breve.Birds = Birds


# Create an instance of our controller object to initialize the simulation

Swarm()


