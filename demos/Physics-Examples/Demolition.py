
# Note: this file was automatically converted to Python from the
# original steve-language source code.  Please see the original
# file for more detailed comments and documentation.


import breve

class BraitenbergDemolition( breve.BraitenbergControl ):
	def __init__( self ):
		breve.BraitenbergControl.__init__( self )
		self.blocks = breve.objectList()
		self.vehicle = None
		self.wheels = breve.objectList()
		BraitenbergDemolition.init( self )

	def catchKeyRDown( self ):
		self.reset()

	def init( self ):
		rampShape = None
		ramp = None

		rampShape = breve.createInstances( breve.Cube, 1 ).initWith( breve.vector( 10, 0.100000, 15 ) )
		self.enableFastPhysics()
		self.setFastPhysicsIterations( 15 )
		ramp = breve.createInstances( breve.Stationary, 1 )
		ramp.register( rampShape, breve.vector( 50, 1.400000, 0 ), breve.matrix(  0.965960, -0.258690, 0.000000, 0.258690, 0.965960, 0.000000, 0.000000, 0.000000, 1.000000 ) )
		self.vehicle = breve.createInstances( breve.BraitenbergHeavyVehicle, 1 )
		self.wheels.append( self.vehicle.addWheel( breve.vector( 1.500000, 0, -1.500000 ) ) )
		self.wheels.append( self.vehicle.addWheel( breve.vector( 1.500000, 0, 1.500000 ) ) )
		self.wheels.append( self.vehicle.addWheel( breve.vector( -1.500000, 0, -1.500000 ) ) )
		self.wheels.append( self.vehicle.addWheel( breve.vector( -1.500000, 0, 1.500000 ) ) )
		self.wheels.setNaturalVelocity( 20.000000 )
		breve.allInstances( "Stationaries" ).setMu( 40.000000 )
		breve.allInstances( "BraitenbergWheels" ).setMu( 40.000000 )
		self.watch( self.vehicle )
		self.setCameraOffset( breve.vector( -11.000000, 4.500000, 5.500000 ) )
		self.panCameraOffset( breve.vector( 20, 10, 9 ), 80 )
		self.blocks = breve.createInstances( breve.Bricks, 10 )
		self.reset()

	def reset( self ):
		breve.allInstances( "Mobiles" ).setVelocity( breve.vector( 0, 0, 0 ) )
		self.vehicle.rotate( breve.vector( 1, 0, 0 ), 0 )
		self.blocks[ 0 ].move( breve.vector( 65, 0.600000, -1.650000 ) )
		self.blocks[ 1 ].move( breve.vector( 65, 0.600000, -0.550000 ) )
		self.blocks[ 2 ].move( breve.vector( 65, 0.600000, 0.550000 ) )
		self.blocks[ 3 ].move( breve.vector( 65, 0.600000, 1.650000 ) )
		self.blocks[ 4 ].move( breve.vector( 65, 1.610000, -1.100000 ) )
		self.blocks[ 5 ].move( breve.vector( 65, 1.610000, 0 ) )
		self.blocks[ 6 ].move( breve.vector( 65, 1.610000, 1.100000 ) )
		self.blocks[ 7 ].move( breve.vector( 65, 2.620000, -0.550000 ) )
		self.blocks[ 8 ].move( breve.vector( 65, 2.620000, 0.550000 ) )
		self.blocks[ 9 ].move( breve.vector( 65, 3.630000, 0 ) )
		self.blocks.setRotationMatrix( breve.matrix(  1, 0, 0, 0, 1, 0, 0, 0, 1 ) )
		self.vehicle.move( breve.vector( 0, 0.800000, 0 ) )


breve.BraitenbergDemolition = BraitenbergDemolition
class Brick( breve.Mobile ):
	def __init__( self ):
		breve.Mobile.__init__( self )
		self.shape = None
		Brick.init( self )

	def init( self ):
		self.setShape( breve.createInstances( breve.Cube, 1 ).initWith( breve.vector( 1, 1, 1 ) ) )
		self.enablePhysics()


breve.Brick = Brick
# Add our newly created classes to the breve namespace

breve.Bricks = Brick


# Create an instance of our controller object to initialize the simulation

BraitenbergDemolition()


