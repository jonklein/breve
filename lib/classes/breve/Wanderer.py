
import breve

class Wanderer( breve.Mobile ):
	'''This class is a work-in-progress.  A Wanderer is simply a real object  that wanders randomly around a certain space.  The range of wandering is adjusted with the METHOD(set-wander-range) method. <p> Subclasses of Wanderer should implement their own <b>iterate</b>  method to perform class specific tasks, and then call the method <b>super iterate</b> in order for the Wanderer object to handle the object's motion.'''

	__slots__ = [ 'topVelocity', 'wanderMag', 'wanderVec', ]

	def __init__( self ):
		breve.Mobile.__init__( self )
		self.topVelocity = 0
		self.wanderMag = 0
		self.wanderVec = breve.vector()
		Wanderer.init( self )

	def init( self ):
		''''''


		self.setWanderRange( breve.vector( 15, 15, 15 ) )
		self.topVelocity = 80

	def iterate( self ):
		''''''


		if ( breve.length( self.getLocation() ) > self.wanderMag ):
			self.setAcceleration( ( ( ( 8 * ( -self.topVelocity ) ) * self.getLocation() ) / breve.length( self.getLocation() ) ) )
			return


		else:
			self.setAcceleration( ( ( 8 * ( ( 2 * breve.randomExpression( self.wanderVec ) ) - self.wanderVec ) ) * self.topVelocity ) )


		if ( breve.length( self.getVelocity() ) > self.topVelocity ):
			self.setVelocity( ( ( self.topVelocity * self.getVelocity() ) / breve.length( self.getVelocity() ) ) )



	def randomizeLocation( self ):
		'''Move the Wanderer to a random location within the wander space.'''

		randomVector = breve.vector()

		self.move( ( self.wanderMag * ( ( 2 * breve.randomExpression( self.wanderVec ) ) - self.wanderVec ) ) )
		randomVector = ( breve.randomExpression( ( 2 * self.wanderVec ) ) - self.wanderVec )
		randomVector = ( randomVector / breve.length( randomVector ) )
		self.setVelocity( ( self.topVelocity * randomVector ) )

	def setTopVelocity( self, newValue ):
		'''Sets the top velocity of the wanderer to newValue.'''


		self.topVelocity = newValue

	def setWanderRange( self, wanderVector ):
		'''Sets the range from the origin that the object can wander.  The  distance the object will wander from the center is the length of  wanderVector.  The direction of the wandering is proportional to  the normalized vector--that is to say that if wanderVector is  (100, 100, 0) that motion will be restricted to the X/Y plane.'''


		self.wanderMag = breve.length( wanderVector )
		self.wanderVec = ( wanderVector / self.wanderMag )


breve.Wanderer = Wanderer



