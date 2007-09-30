
# Note: this file was automatically converted to Python from the
# original steve-language source code.  Please see the original
# file for more detailed comments and documentation.


import breve

class JointTest( breve.PhysicalControl ):
	def __init__( self ):
		breve.PhysicalControl.__init__( self )
		self.body = None
		JointTest.init( self )

	def init( self ):
		self.setBackgroundColor( breve.vector( 0.400000, 0.400000, 0.400000 ) )
		breve.createInstances( breve.Stationary, 1 ).register( breve.createInstances( breve.Cube, 1 ).initWith( breve.vector( 100, 0.100000, 100 ) ), breve.vector( 0, 0, 0 ) )
		self.body = breve.createInstances( breve.JointTestBody, 1 )
		self.enableAutoDisable()
		self.body.showBoundingBox()
		self.body.showAxis()
		self.addMenuSeparator()
		self.addMenu( '''Fixed Joint''', 'makeFixedJoint' )
		self.addMenu( '''Prismatic Joint''', 'makePrismaticJoint' )
		self.addMenu( '''Revolute Joint''', 'makeRevoluteJoint' )
		self.addMenu( '''Universal Joint''', 'makeUniversalJoint' )
		self.addMenu( '''Ball Joint''', 'makeBallJoint' )
		self.pointCamera( breve.vector( 0, 10, 0 ), breve.vector( 20, 4, 20 ) )
		self.setDisplayText( '''Use the simulation menu to select a joint type''', -0.950000, -0.950000 )
		self.watch( self.body )

	def makeBallJoint( self ):
		self.body.makeBallJoint()
		self.setDisplayText( '''BallJoints rotate on all axes''', -0.950000, -0.950000 )

	def makeFixedJoint( self ):
		self.body.makeFixedJoint()
		self.setDisplayText( '''FixedJoints do not move''', -0.950000, -0.950000 )

	def makePrismaticJoint( self ):
		self.body.makePrismaticJoint()
		self.setDisplayText( '''PrismaticJoints slide along one axis''', -0.950000, -0.950000 )

	def makeRevoluteJoint( self ):
		self.body.makeRevoluteJoint()
		self.setDisplayText( '''RevoluteJoints rotate around one axis''', -0.950000, -0.950000 )

	def makeUniversalJoint( self ):
		self.body.makeUniversalJoint()
		self.setDisplayText( '''UniversalJoints rotate on two axes''', -0.950000, -0.950000 )


breve.JointTest = JointTest
class JointTestBody( breve.MultiBody ):
	def __init__( self ):
		breve.MultiBody.__init__( self )
		self.joint = None
		self.links = breve.objectList()
		self.shape = None
		self.smallShape = None
		self.worldJoint = None
		JointTestBody.init( self )

	def getJoint( self ):
		return self.joint

	def init( self ):
		self.shape = breve.createInstances( breve.Cube, 1 ).initWith( breve.vector( 1.100000, 2.000000, 1.100000 ) )
		self.smallShape = breve.createInstances( breve.Cube, 1 ).initWith( breve.vector( 1.000000, 2.000000, 1.000000 ) )
		self.links = breve.createInstances( breve.Links, 2 )
		self.links[ 0 ].setShape( self.shape )
		self.links[ 1 ].setShape( self.smallShape )
		self.setRoot( self.links[ 0 ] )
		self.links[ 0 ].setColor( breve.vector( 0.800000, 0, 0 ) )
		self.links[ 1 ].setColor( breve.vector( 0.600000, 0, 0 ) )
		self.links[ 0 ].move( breve.vector( 0, 8, 0 ) )
		self.links[ 1 ].move( breve.vector( 0, 10, 0 ) )
		self.makeFixedJoint()
		self.worldJoint = breve.createInstances( breve.FixedJoint, 1 )
		self.worldJoint.link( 0, self.links[ 0 ], breve.vector( 0, 10, 0 ), breve.vector( 0, -1.000000, 0 ) )
		return self

	def makeBallJoint( self ):
		if self.joint:
			breve.deleteInstances( self.joint )

		self.joint = breve.createInstances( breve.ExampleBallJoint, 1 )
		self.joint.link( self.links[ 0 ], self.links[ 1 ], breve.vector( 0, -1.000000, 0 ), breve.vector( 0, 1.000000, 0 ) )
		self.joint.setStrengthLimit( 100 )

	def makeFixedJoint( self ):
		if self.joint:
			breve.deleteInstances( self.joint )

		self.joint = breve.createInstances( breve.FixedJoint, 1 )
		self.joint.link( self.links[ 0 ], self.links[ 1 ], breve.vector( 0, -1.000000, 0 ), breve.vector( 0, 1.000000, 0 ) )

	def makePrismaticJoint( self ):
		if self.joint:
			breve.deleteInstances( self.joint )

		self.joint = breve.createInstances( breve.ExamplePrismaticJoint, 1 )
		self.joint.link( self.links[ 0 ], self.links[ 1 ], breve.vector( 0, -1.000000, 0 ), breve.vector( 0, 1.000000, 0 ), breve.vector( 0, 1, 0 ) )
		self.joint.setStrengthLimit( 1000 )

	def makeRevoluteJoint( self ):
		if self.joint:
			breve.deleteInstances( self.joint )

		self.joint = breve.createInstances( breve.ExampleRevoluteJoint, 1 )
		self.joint.link( self.links[ 0 ], self.links[ 1 ], breve.vector( 0, -1.000000, 0 ), breve.vector( 0, 1.000000, 0 ), breve.vector( 1, 0, 0 ) )
		self.joint.setStrengthLimit( 1000 )

	def makeUniversalJoint( self ):
		if self.joint:
			breve.deleteInstances( self.joint )

		self.joint = breve.createInstances( breve.ExampleUniversalJoint, 1 )
		self.joint.link( self.links[ 0 ], self.links[ 1 ], breve.vector( 0, -1.000000, 0 ), breve.vector( 0, 1.000000, 0 ), breve.vector( 1, 0, 0 ) )
		self.joint.setStrengthLimit( 100 )


breve.JointTestBody = JointTestBody
class ExampleRevoluteJoint( breve.RevoluteJoint ):
	def __init__( self ):
		breve.RevoluteJoint.__init__( self )

	def iterate( self ):
		self.setJointVelocity( ( breve.breveInternalFunctionFinder.sin( self, ( self.controller.getTime() / 4 ) ) / 2 ) )


breve.ExampleRevoluteJoint = ExampleRevoluteJoint
class ExamplePrismaticJoint( breve.PrismaticJoint ):
	def __init__( self ):
		breve.PrismaticJoint.__init__( self )

	def iterate( self ):
		self.setJointVelocity( ( breve.breveInternalFunctionFinder.sin( self, ( self.controller.getTime() / 4 ) ) / 2 ) )


breve.ExamplePrismaticJoint = ExamplePrismaticJoint
class ExampleBallJoint( breve.BallJoint ):
	def __init__( self ):
		breve.BallJoint.__init__( self )

	def iterate( self ):
		x = 0

		x = ( breve.breveInternalFunctionFinder.sin( self, ( self.controller.getTime() / 15 ) ) / 2 )
		self.setJointVelocity( breve.vector( 0.000000, 0, x ) )
		print self.getJointVelocity()


breve.ExampleBallJoint = ExampleBallJoint
class ExampleUniversalJoint( breve.UniversalJoint ):
	def __init__( self ):
		breve.UniversalJoint.__init__( self )

	def iterate( self ):
		x = 0

		x = ( breve.breveInternalFunctionFinder.sin( self, ( self.controller.getTime() / 15 ) ) / 2 )
		self.setJointVelocity( x, 0.010000 )


breve.ExampleUniversalJoint = ExampleUniversalJoint


# Create an instance of our controller object to initialize the simulation

JointTest()


