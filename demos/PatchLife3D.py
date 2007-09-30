
# Note: this file was automatically converted to Python from the
# original steve-language source code.  Please see the original
# file for more detailed comments and documentation.


import breve

class PatchLife( breve.Control ):
	def __init__( self ):
		breve.Control.__init__( self )
		self.patches = None
		PatchLife.init( self )

	def catchKeyCDown( self ):
		self.clear()

	def catchKeyRDown( self ):
		self.randomize()

	def clear( self ):
		patch = None
		patchList = breve.objectList()

		patchList = self.patches.getPatches()
		for patch in patchList:
			patch.setState( 0 )


	def init( self ):
		self.patches = breve.createInstances( breve.PatchToroid, 1 ).initWith( 20, 20, 20, breve.vector( 0, 0, 0 ), breve.vector( 1, 1, 1 ), 'LifePatch' )
		self.patches.disableVolumetricDrawing()
		self.setIntegrationStep( 1.000000 )
		self.setIterationStep( 1.000000 )
		self.setBackgroundColor( breve.vector( 0, 0, 0 ) )

	def randomize( self ):
		patch = None
		patchList = breve.objectList()

		patchList = self.patches.getPatches()
		for patch in patchList:
			patch.setState( breve.randomExpression( 1 ) )



breve.PatchLife = PatchLife
class LifePatch( breve.Patch ):
	def __init__( self ):
		breve.Patch.__init__( self )
		self.liveNeighbors = 0
		self.neighbors = breve.objectList()
		self.onCount = 0
		self.state = 0

	def getState( self ):
		return self.state

	def incrementNeighborCount( self ):
		self.liveNeighbors = ( self.liveNeighbors + 1 )

	def initPatch( self ):
		if ( breve.randomExpression( 5 ) == 0 ):
			self.setState( 1 )
		else:
			self.setState( 0 )

		self.neighbors = self.get3dMooreNeighborhood()

	def iterate( self ):
		if self.state:
			self.neighbors.incrementNeighborCount()


	def postIterate( self ):
		if ( ( ( self.state == 0 ) and ( self.liveNeighbors == 4 ) ) or ( ( self.state == 1 ) and ( ( self.liveNeighbors == 4 ) or ( self.liveNeighbors == 6 ) ) ) ):
			self.setTransparency( 0.200000 )
			self.state = 1
			self.setColor( breve.vector( ( 1.000000 - ( self.onCount / 20.000000 ) ), ( self.onCount / 20.000000 ), 0 ) )

		else:
			self.setTransparency( 0.000000 )
			self.state = 0
			self.onCount = 0


		self.onCount = ( self.onCount + 1 )
		self.liveNeighbors = 0

	def setState( self, value ):
		self.setColor( breve.vector( 1.000000, 0, 0 ) )
		self.state = value
		if self.state:
			self.setTransparency( 0.200000 )
		else:
			self.setTransparency( 0.000000 )



breve.LifePatch = LifePatch


# Create an instance of our controller object to initialize the simulation

PatchLife()


