
import breve

class PhysicalControl( breve.Control ):
	'''The OBJECT(PhysicalControl) class is a subclass of OBJECT(Control) which  must be used for physical simulations.  It activates a number of  features required for physical simulation and sets a number of  variables to reasonable values.'''

	__slots__ = [ 'gravityMenu' ]

	def __init__( self ):
		breve.Control.__init__( self )
		self.gravityMenu = breve.objectList()
		PhysicalControl.init( self )

	def dearchive( self ):
		''''''


		return breve.Control.dearchive( self )

	def disableAutoDisable( self ):
		''''''


		breve.breveInternalFunctionFinder.setAutoDisableFlag( self, 0 )

	def disableFastPhysics( self ):
		'''Returns to the default physics simulation.  This is slower, but  more accurate than fast physics, which are enabled using  METHOD(enable-fast-physics).'''


		breve.breveInternalFunctionFinder.setStepFast( self, 0 )

	def doubleGravity( self ):
		'''Sets gravity in the simulated world to double the regular gravity of earth.'''


		self.setGravity( breve.vector( 0.000000, ( -9.800000 * 2 ), 0.000000 ) )
		self.gravityMenu[ 0 ].uncheck()
		self.gravityMenu[ 1 ].uncheck()
		self.gravityMenu[ 2 ].uncheck()
		self.gravityMenu[ 3 ].check()

	def enableAutoDisable( self ):
		''''''


		breve.breveInternalFunctionFinder.setAutoDisableFlag( self, 1 )

	def enableCollisionResolution( self ):
		'''Obsolete.'''



	def enableFastPhysics( self ):
		'''This alternative physical simulation is faster, but less accurate than the default physical simulation.  Though it will not work well for all simulations, you can see a large boost in performance by enabling it.  Fast physics can be turned off using METHOD(disable-fast-physics).'''


		breve.breveInternalFunctionFinder.setStepFast( self, 1 )

	def fullGravity( self ):
		'''Sets gravity in the simulated world to the regular gravity of earth.'''


		self.setGravity( breve.vector( 0.000000, -9.800000, 0.000000 ) )
		self.gravityMenu[ 0 ].uncheck()
		self.gravityMenu[ 1 ].uncheck()
		self.gravityMenu[ 2 ].check()
		self.gravityMenu[ 3 ].uncheck()

	def halfGravity( self ):
		'''Sets gravity in the simulated world to half the regular gravity of earth.'''


		self.setGravity( breve.vector( 0.000000, ( -9.800000 * 0.500000 ), 0.000000 ) )
		self.gravityMenu[ 0 ].uncheck()
		self.gravityMenu[ 1 ].check()
		self.gravityMenu[ 2 ].uncheck()
		self.gravityMenu[ 3 ].uncheck()

	def init( self ):
		''''''


		self.setIntegrationStep( 0.004000 )
		self.setIterationStep( 0.050000 )
		self.setFastPhysicsIterations( 40 )
		self.gravityMenu[ 0 ] = self.addMenu( '''Zero Gravity''', 'zeroGravity' )
		self.gravityMenu[ 1 ] = self.addMenu( '''Half Gravity''', 'halfGravity' )
		self.gravityMenu[ 2 ] = self.addMenu( '''Full Gravity''', 'fullGravity' )
		self.gravityMenu[ 3 ] = self.addMenu( '''Double Gravity''', 'doubleGravity' )
		self.fullGravity()
		self.enableLighting()
		self.enableShadowVolumes()
		self.pointCamera( breve.vector( 0, 0, 0 ), breve.vector( 0, 10, 30 ) )
		self.moveLight( breve.vector( 0, 30, 15 ) )

	def setFastPhysicsIterations( self, iterations ):
		'''Sets the iterations parameter for the fast physics algorithm.  This parameter can be tuned to give better speed or better accuracy. The default value is 40.'''


		breve.breveInternalFunctionFinder.setStepFastIterations( self, iterations )

	def setGravity( self, newGravity ):
		'''Sets the gravity affecting a physical world to newGravity.  If you simply want a realistic earth-like gravity, use the convenience  method <a href="#full-gravity">full-gravity</a>.'''


		breve.breveInternalFunctionFinder.worldSetGravity( self, newGravity )

	def zeroGravity( self ):
		'''Turns off gravity in the simulated world.'''


		self.setGravity( breve.vector( 0.000000, ( -9.800000 * 0 ), 0.000000 ) )
		self.gravityMenu[ 0 ].check()
		self.gravityMenu[ 1 ].uncheck()
		self.gravityMenu[ 2 ].uncheck()
		self.gravityMenu[ 3 ].uncheck()


breve.PhysicalControl = PhysicalControl



