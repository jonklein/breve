
# Note: this file was automatically converted to Python from the
# original steve-language source code.  Please see the original
# file for more detailed comments and documentation.


import breve

class AggressorController( breve.BraitenbergControl ):
	def __init__( self ):
		breve.BraitenbergControl.__init__( self )
		self.depth = None
		self.frameCount = 0
		self.leftSensor = None
		self.leftWheel = None
		self.n = 0
		self.rightSensor = None
		self.rightWheel = None
		self.simSpeed = 0
		self.startTime = 0
		self.vehicle = None
		self.video = None
		AggressorController.init( self )

	def init( self ):
		self.n = 0
		while ( self.n < 10 ):
			breve.createInstances( breve.BraitenbergLight, 1 ).move( breve.vector( ( 20 * breve.breveInternalFunctionFinder.sin( self, ( ( self.n * 6.280000 ) / 10 ) ) ), 1, ( 20 * breve.breveInternalFunctionFinder.cos( self, ( ( self.n * 6.280000 ) / 10 ) ) ) ) )
			self.n = ( self.n + 1 )

		self.vehicle = breve.createInstances( breve.BraitenbergVehicle, 1 )
		self.watch( self.vehicle )
		self.vehicle.move( breve.vector( 0, 2, 18 ) )
		self.leftWheel = self.vehicle.addWheel( breve.vector( -0.500000, 0, -1.500000 ) )
		self.rightWheel = self.vehicle.addWheel( breve.vector( -0.500000, 0, 1.500000 ) )
		self.leftWheel.setNaturalVelocity( 0.000000 )
		self.rightWheel.setNaturalVelocity( 0.000000 )
		self.rightSensor = self.vehicle.addSensor( breve.vector( 2.000000, 0.400000, 1.500000 ) )
		self.leftSensor = self.vehicle.addSensor( breve.vector( 2.000000, 0.400000, -1.500000 ) )
		self.leftSensor.link( self.rightWheel )
		self.rightSensor.link( self.leftWheel )
		self.leftSensor.setBias( 15.000000 )
		self.rightSensor.setBias( 15.000000 )
		self.video = breve.createInstances( breve.Image, 1 )
		self.video.setSize( 176, 144 )
		self.depth = breve.createInstances( breve.Image, 1 )
		self.depth.setSize( 176, 144 )
		self.startTime = self.getRealTime()

	def postIterate( self ):
		self.frameCount = ( self.frameCount + 1 )
		self.simSpeed = ( self.getTime() / ( self.getRealTime() - self.startTime ) )
		print '''Simulation speed = %s''' % (  self.simSpeed )
		self.video.readPixels( 0, 0 )
		self.depth.readDepth( 0, 0, 1, 50 )
		if ( self.frameCount < 10 ):
			self.video.write( '''imgs/video-%s.png''' % (  self.frameCount ) )

		self.depth.write16BitGrayscale( '''imgs/depth-%s.png''' % (  self.frameCount ) )


breve.AggressorController = AggressorController


# Create an instance of our controller object to initialize the simulation

AggressorController()


