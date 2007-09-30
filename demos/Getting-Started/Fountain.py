
# Note: this file was automatically converted to Python from the
# original steve-language source code.  Please see the original
# file for more detailed comments and documentation.


import breve

class Fountain( breve.Control ):
	def __init__( self ):
		breve.Control.__init__( self )
		self.m1 = None
		self.m2 = None
		self.m3 = None
		self.particles = breve.objectList()
		Fountain.init( self )

	def init( self ):
		self.setIntegrationStep( 0.050000 )
		self.setIterationStep( 0.100000 )
		self.enableLighting()
		self.enableSmoothDrawing()
		self.setBackgroundColor( breve.vector( 0, 0, 0 ) )
		self.particles = breve.createInstances( breve.Particle, 250 )
		self.pointCamera( breve.vector( 0, 9, 0 ), breve.vector( 40.000000, 2.100000, 0.000000 ) )
		self.m1 = self.addMenu( '''Thin Fountain''', 'setSmallRange' )
		self.m2 = self.addMenu( '''Normal Fountain''', 'setNormalRange' )
		self.m3 = self.addMenu( '''Large Fountain''', 'setLargeRange' )
		self.setNormalRange()

	def setLargeRange( self ):
		self.m1.uncheck()
		self.m2.uncheck()
		self.m3.check()
		self.setParticleRange( 8 )

	def setNormalRange( self ):
		self.m1.uncheck()
		self.m2.check()
		self.m3.uncheck()
		self.setParticleRange( 3 )

	def setParticleRange( self, n ):
		self.particles.setRange( n )

	def setSmallRange( self ):
		self.m1.check()
		self.m2.uncheck()
		self.m3.uncheck()
		self.setParticleRange( 1 )


breve.Fountain = Fountain
class Particle( breve.Mobile ):
	def __init__( self ):
		breve.Mobile.__init__( self )
		self.range = 0
		Particle.init( self )

	def init( self ):
		self.setShape( breve.createInstances( breve.Cube, 1 ).initWith( ( breve.randomExpression( breve.vector( 1.000000, 1.000000, 1.000000 ) ) + breve.vector( 0.100000, 0.100000, 0.100000 ) ) ) )
		self.setAcceleration( breve.vector( 0, -9.800000, 0.000000 ) )
		self.range = 5
		self.reset()

	def iterate( self ):
		if ( self.getLocation().y < -6.000000 ):
			self.reset()


	def reset( self ):
		self.setColor( breve.randomExpression( breve.vector( 0, 1, 1 ) ) )
		self.move( breve.vector( 0, 0, 0 ) )
		self.setVelocity( ( breve.randomExpression( breve.vector( ( 2 * self.range ), 20, ( 2 * self.range ) ) ) + breve.vector( ( -self.range ), 4, ( -self.range ) ) ) )
		self.setRotationalVelocity( breve.randomExpression( breve.vector( 0.600000, 0.600000, 0.600000 ) ) )

	def setRange( self, n ):
		self.range = n


breve.Particle = Particle


# Create an instance of our controller object to initialize the simulation

Fountain()


