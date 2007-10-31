
# Note: this file was automatically converted to Python from the
# original steve-language source code.  Please see the original
# file for more detailed comments and documentation.


import breve

class BraitenbergStepfield( breve.BraitenbergControl ):
	def __init__( self ):
		breve.BraitenbergControl.__init__( self )
		self.body = None
		self.depth = None
		self.frameCount = 0
		self.leftFrontWheel = None
		self.leftRearWheel = None
		self.leftSpeed = 0
		self.neighbours = breve.objectList()
		self.rightFrontWheel = None
		self.rightRearWheel = None
		self.rightSpeed = 0
		self.stepfield = breve.objectList()
		self.vehicle = None
		self.video = None
		self.wheels = breve.objectList()
		BraitenbergStepfield.init( self )

	def catchKeyDownDown( self ):
		self.goBackward()

	def catchKeyLeftDown( self ):
		self.turnLeft()

	def catchKeyRightDown( self ):
		self.turnRight()

	def catchKeyUpDown( self ):
		self.goForward()

	def goBackward( self ):
		self.leftSpeed = self.rightSpeed = -5.000000
		self.wheels.setNaturalVelocity( -5.000000 )

	def goForward( self ):
		self.leftSpeed = self.rightSpeed = 7.000000
		self.wheels.setNaturalVelocity( 7.000000 )

	def init( self ):
		self.enableFastPhysics()
		self.setFastPhysicsIterations( 15 )
		self.vehicle = breve.createInstances( breve.BraitenbergHeavyVehicle, 1 )
		self.body = self.vehicle.getRoot()
		self.body.setNeighborhoodSize( 10.000000 )
		self.wheels.append( self.leftFrontWheel = self.vehicle.addWheel( breve.vector( 1.500000, 0, -1.500000 ) ) )
		self.wheels.append( self.rightFrontWheel = self.vehicle.addWheel( breve.vector( 1.500000, 0, 1.500000 ) ) )
		self.wheels.append( self.leftRearWheel = self.vehicle.addWheel( breve.vector( -1.500000, 0, -1.500000 ) ) )
		self.wheels.append( self.rightRearWheel = self.vehicle.addWheel( breve.vector( -1.500000, 0, 1.500000 ) ) )
		self.leftFrontWheel.setNeighborhoodSize( 3.000000 )
		self.rightFrontWheel.setNeighborhoodSize( 3.000000 )
		self.leftRearWheel.setNeighborhoodSize( 3.000000 )
		self.rightRearWheel.setNeighborhoodSize( 3.000000 )
		self.leftSpeed = 0.000000
		self.rightSpeed = 0.000000
		self.wheels.setNaturalVelocity( 0.000000 )
		breve.allInstances( "Stationaries" ).setMu( 40.000000 )
		breve.allInstances( "BraitenbergWheels" ).setMu( 40.000000 )
		self.watch( self.vehicle )
		self.setCameraOffset( breve.vector( -11.000000, 10.000000, 0 ) )
		self.reset()
		self.video = breve.createInstances( breve.Image, 1 )

	def iterate( self ):
		breve.BraitenbergControl.iterate( self )
		self.updateNeighbors()
		self.neighbours = self.body.getNeighbors()

	def postIterate( self ):
		self.frameCount = ( self.frameCount + 1 )
		self.controller.saveSnapshot( '''imgs/video-%s.png''' % (  self.frameCount ) )
		self.controller.saveDepthSnapshot( '''imgs/depth-%s.png''' % (  self.frameCount ), 1, 10 )

	def reset( self ):
		breve.allInstances( "Mobiles" ).setVelocity( breve.vector( 0, 0, 0 ) )
		self.vehicle.rotate( breve.vector( 1, 0, 0 ), 0 )
		self.vehicle.move( breve.vector( -10, 0.800000, 0 ) )
		self.stepfield = [ breve.createInstances( breve.Pallet, 1 ).initWith( 0, -5 ) ]

	def turnLeft( self ):
		self.leftSpeed = -5.000000
		self.rightSpeed = 10.000000
		self.leftFrontWheel.setNaturalVelocity( self.leftSpeed )
		self.leftRearWheel.setNaturalVelocity( self.leftSpeed )
		self.rightFrontWheel.setNaturalVelocity( self.rightSpeed )
		self.rightRearWheel.setNaturalVelocity( self.rightSpeed )

	def turnRight( self ):
		self.leftSpeed = 10.000000
		self.rightSpeed = -5.000000
		self.leftFrontWheel.setNaturalVelocity( self.leftSpeed )
		self.leftRearWheel.setNaturalVelocity( self.leftSpeed )
		self.rightFrontWheel.setNaturalVelocity( self.rightSpeed )
		self.rightRearWheel.setNaturalVelocity( self.rightSpeed )


breve.BraitenbergStepfield = BraitenbergStepfield
class Frame( breve.Object ):
	def __init__( self ):
		breve.Object.__init__( self )
		self.back = None
		self.front = None
		self.height = 0
		self.left = None
		self.right = None

	def destroy( self ):
		breve.deleteInstances( self.front )
		breve.deleteInstances( self.back )
		breve.deleteInstances( self.left )
		breve.deleteInstances( self.right )

	def getHeight( self ):
		return self.height

	def initWith( self, xpos, zpos ):
		self.height = ( 2 * 0.250000 )
		self.front = breve.createInstances( breve.Mobile, 1 )
		self.front.setShape( breve.createInstances( breve.Cube, 1 ).initWith( breve.vector( 1, self.height, 11 ) ) )
		self.front.move( breve.vector( ( xpos - 1 ), ( ( 0.500000 * self.height ) + 0.100000 ), ( ( zpos + ( 11.000000 / 2.000000 ) ) - 0.500000 ) ) )
		self.front.setColor( breve.vector( 0.000000, 0.600000, 0.100000 ) )
		self.front.enablePhysics()
		self.back = breve.createInstances( breve.Mobile, 1 )
		self.back.setShape( breve.createInstances( breve.Cube, 1 ).initWith( breve.vector( 1, self.height, 11 ) ) )
		self.back.move( breve.vector( ( xpos + 11 ), ( ( 0.500000 * self.height ) + 0.100000 ), ( ( zpos + ( 11.000000 / 2.000000 ) ) - 0.500000 ) ) )
		self.back.setColor( breve.vector( 0.000000, 0.600000, 0.100000 ) )
		self.back.enablePhysics()
		self.left = breve.createInstances( breve.Mobile, 1 )
		self.left.setShape( breve.createInstances( breve.Cube, 1 ).initWith( breve.vector( 11, self.height, 1 ) ) )
		self.left.move( breve.vector( ( xpos + ( ( 11.000000 / 2.000000 ) - 0.500000 ) ), ( ( 0.500000 * self.height ) + 0.100000 ), ( zpos - 1 ) ) )
		self.left.setColor( breve.vector( 0.000000, 0.600000, 0.100000 ) )
		self.left.enablePhysics()
		self.right = breve.createInstances( breve.Mobile, 1 )
		self.right.setShape( breve.createInstances( breve.Cube, 1 ).initWith( breve.vector( 11, self.height, 1 ) ) )
		self.right.move( breve.vector( ( xpos + ( ( 11.000000 / 2.000000 ) - 0.500000 ) ), ( ( 0.500000 * self.height ) + 0.100000 ), ( zpos + 11 ) ) )
		self.right.setColor( breve.vector( 0.000000, 0.600000, 0.100000 ) )
		self.right.enablePhysics()
		return self


breve.Frame = Frame
class Pallet( breve.Object ):
	def __init__( self ):
		breve.Object.__init__( self )
		self.blocks = breve.objectList()
		self.outsideFrame = None
		self.texture = None

	def destroy( self ):
		blockList = breve.objectList()
		block = None

		breve.deleteInstances( self.outsideFrame )
		for blockList in self.blocks:
			for block in blockList:
				breve.deleteInstances( block )





	def initWith( self, xpos, zpos ):
		z = 0
		x = 0
		row = breve.objectList()
		height = 0

		self.outsideFrame = breve.createInstances( breve.Frame, 1 ).initWith( ( xpos + 0.100000 ), ( zpos + 0.100000 ) )
		x = 0
		while ( x < 11 ):
			row = breve.createInstances( breve.Bricks, 11 )
			z = 0
			while ( z < 11 ):
				height = row[ z ].getHeight()
				row[ z ].move( breve.vector( ( ( xpos + x ) + 0.100000 ), ( ( 0.500000 * height ) + 0.100000 ), ( ( zpos + z ) + 0.100000 ) ) )
				row[ z ].setColor( breve.vector( 0.300000, 0.300000, 0.200000 ) )

				z = ( z + 1 )

			self.blocks.append( row )
			row.setRotation( breve.matrix(  1, 0, 0, 0, 1, 0, 0, 0, 1 ) )

			x = ( x + 1 )

		return self

	def printBlocks( self, expId ):
		row = 0
		column = 0
		blockHeight = 0

		row = 0
		while ( row < 11 ):
			column = 0
			while ( column < 11 ):
				blockHeight = self.blocks[ row ][ column ].blockHeight()
				print '''block(%s,%s,%s,%s).''' % (  expId, row, column, blockHeight )

				column = ( column + 1 )


			row = ( row + 1 )



breve.Pallet = Pallet
class Brick( breve.Mobile ):
	def __init__( self ):
		breve.Mobile.__init__( self )
		self.blockHeight = 0
		self.height = 0
		Brick.init( self )

	def blockHeight( self ):
		return self.blockHeight

	def destroy( self ):
		breve.Mobile.destroy( self )

	def getHeight( self ):
		return self.height

	def init( self ):
		self.blockHeight = ( breve.randomExpression( 3 ) + 1 )
		self.height = ( 0.250000 * self.blockHeight )
		self.setShape( breve.createInstances( breve.Cube, 1 ).initWith( breve.vector( 1, self.height, 1 ) ) )
		self.enablePhysics()


breve.Brick = Brick
# Add our newly created classes to the breve namespace

breve.Bricks = Brick


# Create an instance of our controller object to initialize the simulation

BraitenbergStepfield()


