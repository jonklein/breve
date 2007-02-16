
import breve

class Stationary( breve.Real ):
	'''Stationary objects are objects such as floors and walls that may  collide with other objects but which do not move in response to  collisions.  Stationary objects do not have their own velocities or acceleration. <p> Starting in breve 2.6, stationary objects can be moved and  rotated after being created using methods in OBJECT(Real).   The shapes can also be changed dynamically using the set-shape  method in OBJECT(Real).'''

	def __init__( self ):
		breve.Real.__init__( self )
		self.objectLocation = breve.vector()
		self.shadowCatcher = 0
		self.shape = None
		Stationary.init( self )

	def archive( self ):
		''''''


		return 1


	def catchShadows( self ):
		'''Informs this object that it should display shadows (and/or reflections)  of Mobile objects.  The shadows and reflections will always be shown on the plane of the object pointing upwards on the Y axis--that is to say, the plane with normal (0, 1, 0).  If the object does not have a plane with normal (0, 1, 0), the shadows and reflections will not be displayed correctly.  This method must be used in conjunction with the method  OBJECTMETHOD(Control:enable-shadows). <P> Before using this method, you should also refer to an improved shadowing technique outlined in OBJECTMETHOD(Control.tz:enable-shadow-volumes).  '''


		if ( not self.realWorldPointer ):
			raise Exception( '''method 'catch-shadows' cannot be called before Stationary object is registered.''' )


		self.shadowCatcher = 1
		breve.breveInternalFunctionFinder.setShadowCatcher( self, self.realWorldPointer, breve.vector( 0, 1, 0 ) )
		self.controller.setFloorDefined()

	def dearchive( self ):
		''''''


		self.register( self.shape, self.objectLocation )
		if self.shadowCatcher:
			self.catchShadows()

		return 1


	def getWorldObject( self ):
		'''Used internally to get the pointer to the world.  Do not use this method in user simulations.'''


		return self.realWorldPointer


	def init( self ):
		''''''


		self.realWorldPointer = breve.breveInternalFunctionFinder.addStationary( self, self.controller.getGenericShape().getPointer(), breve.vector( 0, 0, 0 ), breve.matrix(  1, 0, 0, 0, 1, 0, 0, 0, 1 ) )
		self.setTexture( 0 )
		return self


	def register( self, theShape, theLocation, theRotation = breve.matrix() ):
		'''Registers a stationary object using shape theShape at the location specified by theLocation.'''


		self.setShape( theShape )
		self.move( theLocation )
		self.setRotation( theRotation )
		return self



breve.Stationary = Stationary

class Floor( breve.Stationary ):
	'''A floor is a special case of the class OBJECT(Stationary).  It is a box  of size (1000, 5, 1000) with location (0, -2.5, 0), such that the ground  plane is placed at Y = 0.'''

	def __init__( self ):
		breve.Stationary.__init__( self )
		Floor.init( self )

	def init( self ):
		''''''


		self.register( breve.createInstances( breve.Cube, 1 ).initWith( breve.vector( 1000, 5, 1000 ) ), breve.vector( 0, -2.500000, 0 ) )


breve.Floor = Floor


