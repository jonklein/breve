
# Note: this file was automatically converted to Python from the
# original steve-language source code.  Please see the original
# file for more detailed comments and documentation.


import breve

class Fireworks( breve.Control ):
	def __init__( self ):
		breve.Control.__init__( self )
		self.lightmap = None
		Fireworks.init( self )

	def catchKeyNDown( self ):
		breve.createInstances( breve.Shooter, 1 )

	def getLightmapImage( self ):
		return self.lightmap

	def init( self ):
		self.setIntegrationStep( 0.200000 )
		self.setIterationStep( 0.200000 )
		self.setBackgroundColor( breve.vector( 0, 0, 0 ) )
		self.lightmap = breve.createInstances( breve.Image, 1 ).load( 'images/lightmap.png' )
		self.enableBlur()
		self.setBlurFactor( 0.850000 )
		self.pointCamera( breve.vector( 0, 0, 0 ), breve.vector( 120, 2.100000, 0.000000 ) )

	def iterate( self ):
		if ( breve.randomExpression( 30 ) == 0 ):
			breve.createInstances( breve.Shooter, 1 )

		breve.Control.iterate( self )


breve.Fireworks = Fireworks
class Shooter( breve.Mobile ):
	def __init__( self ):
		breve.Mobile.__init__( self )
		self.age = 0
		self.limit = 0
		Shooter.init( self )

	def init( self ):
		self.setShape( breve.createInstances( breve.Sphere, 1 ).initWith( ( 0.100000 + breve.randomExpression( 1.400000 ) ) ) )
		self.setLightmapImage( self.controller.getLightmapImage() )
		self.setColor( ( breve.randomExpression( breve.vector( 0.200000, 0.200000, 0.200000 ) ) + breve.vector( 0.800000, 0.800000, 0.800000 ) ) )
		self.move( ( breve.randomExpression( breve.vector( 6, 0, 6 ) ) - breve.vector( 3, 30, 3 ) ) )
		self.setVelocity( ( breve.randomExpression( breve.vector( 4, 3, 4 ) ) + breve.vector( -2, 9, -2 ) ) )
		self.setAcceleration( breve.vector( 0, -1, 0 ) )
		self.limit = ( 30 + breve.randomExpression( 15 ) )

	def iterate( self ):
		self.age = ( self.age + 1 )
		if ( self.age > self.limit ):
			breve.createInstances( breve.Particle, 80 ).move( self.getLocation() )
			breve.deleteInstances( self )




breve.Shooter = Shooter
class Particle( breve.Mobile ):
	def __init__( self ):
		breve.Mobile.__init__( self )
		self.color = breve.vector()
		self.intensity = 0
		self.rate = 0
		Particle.init( self )

	def init( self ):
		self.setShape( breve.createInstances( breve.Sphere, 1 ).initWith( ( 0.100000 + breve.randomExpression( 4.000000 ) ) ) )
		self.intensity = 1.000000
		self.rate = ( 0.900000 + breve.randomExpression( 0.080000 ) )
		self.setLightmapImage( self.controller.getLightmapImage() )
		self.color = breve.randomExpression( breve.vector( 1, 1, 1 ) )
		self.setColor( self.color )
		self.setVelocity( ( breve.randomExpression( breve.vector( 5, 5, 5 ) ) - breve.vector( 2.500000, 2.500000, 2.500000 ) ) )
		self.setAcceleration( breve.vector( 0, ( -breve.randomExpression( 0.500000 ) ), 0 ) )

	def iterate( self ):
		self.intensity = ( self.intensity * self.rate )
		self.setColor( ( self.color * self.intensity ) )
		if ( self.intensity < 0.100000 ):
			breve.deleteInstances( self )



breve.Particle = Particle
# Add our newly created classes to the breve namespace

breve.Shooters = Shooter


# Create an instance of our controller object to initialize the simulation

Fireworks()


