
# Note: this file was automatically converted to Python from the
# original steve-language source code.  Please see the original
# file for more detailed comments and documentation.


import breve

class Gatherers( breve.Control ):
	def __init__( self ):
		breve.Control.__init__( self )
		self.collectorShape = None
		self.foodList = breve.objectList()
		self.foodShape = None
		Gatherers.init( self )

	def getCollectorShape( self ):
		return self.collectorShape

	def getFoodShape( self ):
		return self.foodShape

	def init( self ):
		self.collectorShape = breve.createInstances( breve.Sphere, 1 ).initWith( 1 )
		self.foodShape = breve.createInstances( breve.Sphere, 1 ).initWith( 0.500000 )
		breve.createInstances( breve.Collectors, 50 )
		self.foodList = breve.createInstances( breve.Foods, 400 )
		self.foodList.setBitmap( 2 )
		self.setIntegrationStep( 0.050000 )
		self.addMenu( '''Randomize Food''', 'randomizeFood' )
		self.pointCamera( breve.vector( 0, 0, 0 ), breve.vector( 0, 0, -80 ) )

	def randomizeFood( self ):
		self.foodList.randomizeLocation()


breve.Gatherers = Gatherers
class Collector( breve.Wanderer ):
	def __init__( self ):
		breve.Wanderer.__init__( self )
		self.carrying = None
		self.justCollided = 0
		Collector.init( self )

	def collide( self, food ):
		newLoc = breve.vector()

		if food.getOwner():
			return

		if ( self.justCollided > 0 ):
			self.justCollided = 2
			return


		self.justCollided = 2
		if self.carrying:
			newLoc = food.getLocation()
			newLoc = ( newLoc + ( breve.randomExpression( breve.vector( 2, 2, 2 ) ) - breve.vector( 1, 1, 1 ) ) )
			self.carrying.move( newLoc )
			self.carrying.setOwner( 0 )
			self.carrying = 0
			return


		food.setOwner( self )
		self.carrying = food

	def init( self ):
		self.setShape( self.controller.getCollectorShape() )
		self.setColor( breve.vector( 1.000000, 1.000000, 1.000000 ) )
		self.setWanderRange( breve.vector( 20.000000, 20.000000, 20.000000 ) )
		self.randomizeLocation()
		self.handleCollisions( 'Food', 'collide' )

	def iterate( self ):
		if self.carrying:
			self.carrying.move( ( self.getLocation() - breve.vector( 1, 0, 0 ) ) )

		self.justCollided = ( self.justCollided - 1 )
		breve.Wanderer.iterate( self )


breve.Collector = Collector
class Food( breve.Mobile ):
	def __init__( self ):
		breve.Mobile.__init__( self )
		self.owner = None
		Food.init( self )

	def getOwner( self ):
		return self.owner

	def init( self ):
		self.setShape( self.controller.getFoodShape() )
		self.setColor( breve.vector( 0.800000, 0.200000, 0.200000 ) )
		self.randomizeLocation()

	def randomizeLocation( self ):
		self.move( ( breve.randomExpression( ( 2 * breve.vector( 20.000000, 20.000000, 20.000000 ) ) ) - breve.vector( 20.000000, 20.000000, 20.000000 ) ) )

	def setOwner( self, o ):
		self.owner = o


breve.Food = Food
# Add our newly created classes to the breve namespace

breve.Collectors = Collector
breve.Foods = Food


# Create an instance of our controller object to initialize the simulation

Gatherers()


