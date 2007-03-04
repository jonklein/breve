
import breve

class Terrain( breve.Real ):
	'''Terrain is an experimental class to add terrains to simulations.  '''

	__slots__ = [ 'location', 'peakColor', 'valleyColor', ]

	def __init__( self ):
		breve.Real.__init__( self )
		self.location = breve.vector()
		self.peakColor = breve.vector()
		self.valleyColor = breve.vector()
		Terrain.init( self )

	def archive( self ):
		''''''


		return 1


	def dearchive( self ):
		''''''


		self.realWorldPointer = breve.breveInternalFunctionFinder.terrainNew( self, 1 )
		self.setPeakColor( self.peakColor )
		self.setValleyColor( self.valleyColor )
		return 1


	def drawSolid( self ):
		'''Draws the terrain as solid polygons.  This is the default mode.   Alternatively, the terrain can be drawn as wireframe only using  the method METHOD(draw-wireframe).'''


		breve.breveInternalFunctionFinder.terrainSetDrawMode( self, self.realWorldPointer, 1 )

	def drawWireframe( self ):
		'''Draws the terrain as a wireframe outline only.  See also METHOD(draw-solid).'''


		breve.breveInternalFunctionFinder.terrainSetDrawMode( self, self.realWorldPointer, 0 )

	def generate( self, ruggedness = 0.200000, terrainHeight = 5.000000 ):
		''''''


		self.generateFractalTerrain( ruggedness, terrainHeight )

	def generateFractalTerrain( self, ruggedness = 0.200000, terrainHeight = 5.000000 ):
		'''Generates a fractal landscape with a ruggedness (between 0.0 and 1.0), and maximum height terrainHeight.'''


		breve.breveInternalFunctionFinder.terrainGenerateFractalTerrain( self, self.realWorldPointer, ( 1.000000 - ruggedness ), terrainHeight )

	def getHeight( self, xCoordinate, yCoordinate ):
		'''Returns the height of the terrain at (xCoordinate, yCoordinate). The coordinates refer to the polygons on the terrain, not to the real world location of points on the terrain.'''


		return breve.breveInternalFunctionFinder.terrainGetHeight( self, self.realWorldPointer, xCoordinate, yCoordinate )


	def getHeightInWorld( self, xCoordinate, zCoordinate ):
		'''This method returns the height of the terrain at the <b>real world</b>  (xCoordinate, zCoodinate) location.  This is in contrast to the height in terrain grid coordinates given give with METHOD(get-height)'''


		return breve.breveInternalFunctionFinder.terrainGetHeightAtLocation( self, self.realWorldPointer, xCoordinate, zCoordinate )


	def getLocation( self ):
		'''Returns the location of this Terrain object.'''


		return self.location


	def init( self ):
		''''''


		self.realWorldPointer = breve.breveInternalFunctionFinder.terrainNew( self, 1 )
		self.setPeakColor( breve.vector( 1, 1, 1 ) )
		self.setValleyColor( breve.vector( 0, 0.800000, 0.100000 ) )
		self.setTextureImage( breve.createInstances( breve.Image, 1 ).load( '''images/grass.jpg''' ) )

	def loadGeoTiff( self, geoTIFFFile ):
		''''''


		if self.realWorldPointer:
			breve.breveInternalFunctionFinder.terrainLoadGeoTIFF( self, self.realWorldPointer, geoTIFFFile )


	def move( self, newLocation ):
		'''Sets the location of the terrain to location.  This method can be used to  set the location of the terrain when setting up the world, but should not generally be used to dynamically over the course of the simulation.'''


		self.location = newLocation
		breve.breveInternalFunctionFinder.terrainSetPosition( self, self.realWorldPointer, newLocation )

	def setColor( self, color ):
		'''Sets the color of this terrain object.  The peak and valley colors can also be set independently using METHOD(set-peak-color) and METHOD(set-valley-color).'''


		self.setPeakColor( color )
		self.setValleyColor( color )

	def setHeight( self, xCoordinate, yCoordinate, height ):
		'''Sets the height of the terrain at (xCoordinate, yCoordinate) to height. The coordinates refer to the polygons on the terrain, not to the real world location of points on the terrain.'''


		breve.breveInternalFunctionFinder.terrainSetHeight( self, self.realWorldPointer, xCoordinate, yCoordinate, height )

	def setPeakColor( self, color ):
		'''Sets the color of the highest regions of the terrain to color. As the height of the terrain decreases, There will be a gradient  towards the color specified with METHOD(set-valley-color).'''


		self.peakColor = color
		breve.breveInternalFunctionFinder.terrainSetPeakColor( self, self.realWorldPointer, color )

	def setScale( self, value ):
		'''Changes the size of a single tile in the terrain.  This does not change the number of elevation points in the terrain.'''


		breve.breveInternalFunctionFinder.terrainSetScale( self, self.realWorldPointer, value )

	def setValleyColor( self, color ):
		'''Sets the color of the lowest regions of the terrain to color. As the height of the terrain increases, There will be a gradient  towards the color specified with METHOD(set-peak-color).'''


		self.valleyColor = color
		breve.breveInternalFunctionFinder.terrainSetValleyColor( self, self.realWorldPointer, color )


breve.Terrain = Terrain



