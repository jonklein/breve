
import breve

class AggressorController( breve.BraitenbergControl ):
	def __init__( self ):
		breve.BraitenbergControl.__init__( self )
		self.leftSensor = None
		self.leftWheel = None
		self.n = 0
		self.rightSensor = None
		self.rightWheel = None
		self.vehicle = None
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


breve.AggressorController = AggressorController


# Create an instance of our controller object to initialize the simulation

AggressorController()


