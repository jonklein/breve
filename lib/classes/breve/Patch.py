
import breve

class Patch( breve.Abstract ):
	'''A patch is a non-physical object which is associated with a specific  location in space.  A patch can hold information about the state of the world in that location, such as the presence of a nutrient, for example.  See the object OBJECT(PatchGrid) for more information on  setting up patches.  Patches must be set up as part of a OBJECT(PatchGrid). <p> Because patches are not physical, they cannot move and they cannot collide with other objects.  Though you can find the patch that an  object is in, there is no collision callback when an object enters the patch.  Patches can track the concentration of objects in their space.  Currently this is limited to OBJECT(PatchChemical) objects which are used to track information about abstract diffusable elements in the world.  You can get the concentration, set the concentration, add and remove PatchChemcicals through the patch.  See OBJECT(PatchGrid) for adding PatchChemcials to the simulation and to iterate the diffusion.  It is  important to note that concentrations are stored in single precision floats currently to improve performance on hardware that supports  SIMD processing. <p> The methods used to find neighboring patches depends on the type of  simulation you're running.  Though many of the "get-patch" methods described below behave the same as one another, they are named differently to be  consistent with different types of simulations.   <p> <b>The neighbors are not fully initialized at the time that the init method is called.</b>  This is because the CLASS(PatchGrid) must create all of the patches before it can initialize the neighbors.  Instead of accessing the  neighbors in the init method, you should implement your own METHOD(init-patch) and place all initialization inside this method.  The neighbors for the patch will be fully initialized by the time the init-patch method is called. <p> When using a 2D XY style simulation, use the following methods to get the  four cardinal directions: METHOD(get-patch-to-left), METHOD(get-patch-to-right), METHOD(get-patch-above) and METHOD(get-patch-below). <p> When using a 3D style simulation with and XZ ground plane (such that the vector (0, 1, 0) is pointing up, use the following methods to get the  six cardinal directions: METHOD(get-patch-to-north), METHOD(get-patch-to-east), METHOD(get-patch-to-south), METHOD(get-patch-to-west), METHOD(get-patch-above) and METHOD(get-patch-below). <p> These two groups of methods should suffice for most simulations, but if for some reason you use another orientation for your simulation which doesn't  fit well with the vocabulary of these methods, you can use the following methods: METHOD(get-patch-towards-plus-x), METHOD(get-patch-towards-minus-x), METHOD(get-patch-towards-plus-y), METHOD(get-patch-towards-minus-y), METHOD(get-patch-towards-plus-z) and METHOD(get-patch-towards-minus-z).'''

	__slots__ = [ 'color', 'grid', 'patchPointer', 'transparency', 'xIndex', 'yIndex', 'zIndex' ]

	def __init__( self ):
		breve.Abstract.__init__( self )
		self.color = breve.vector()
		self.grid = None
		self.patchPointer = None
		self.transparency = 0
		self.xIndex = 0
		self.yIndex = 0
		self.zIndex = 0

	def dearchive( self ):
		''''''


		return 1

	def decreaseConcentration( self, theDiffusable, theValue ):
		'''returns the concentration of a OBJECT(PatchChecmical) in this patch.'''


		self.grid.decreaseConcentnration( theDiffusable, theValue, self.xIndex, self.yIndex, self.zIndex )

	def get3dMooreNeighborhood( self ):
		'''Returns a list of the 3D Moore neighbors of this patch.  The  3D Moore neighborhood consists of the 26 patches  adjacent to this patch, including diagonally, in three dimensions. <P> If you intend to use the neighborhood frequently, you should  call this method once and keep a copy of the list.'''

		result = breve.objectList()
		z = 0
		y = 0
		x = 0
		patch = None

		x = -1
		while ( x <= 1 ):
			y = -1
			while ( y <= 1 ):
				z = -1
				while ( z <= 1 ):
					patch = self.grid.getPatchAt( ( x + self.xIndex ), ( y + self.yIndex ), ( z + self.zIndex ) )
					if ( patch and ( patch != self ) ):
						result.append( patch )


					z = ( z + 1 )


				y = ( y + 1 )


			x = ( x + 1 )

		return result

	def get3dVonNeumannNeighborhood( self ):
		'''Returns a list of the 3D Von Neumann neighbors of this patch.  The  3D Von Neumann neighborhood consists of the 6 patches directly adjacent to the faces of this patch, in three dimensions. <P> If you intend to use the neighborhood frequently, you should  call this method once and keep a copy of the list.'''

		result = breve.objectList()

		result = self.getVonNeumannNeighborhood()
		if self.getPatchTowardsPlusZ():
			result.append( self.getPatchTowardsPlusZ() )

		if self.getPatchTowardsMinusZ():
			result.append( self.getPatchTowardsMinusZ() )


	def getColor( self ):
		'''Returns the color of the patch.'''


		return self.color

	def getConcentration( self, theDiffusable ):
		'''returns the concentration of a OBJECT(PatchChecmical) in this patch.'''


		return self.grid.getConcentration( theDiffusable, self.xIndex, self.yIndex, self.zIndex )

	def getDescription( self ):
		''''''


		return '''patch at indices (%s, %s, %s)''' % (  self.xIndex, self.yIndex, self.zIndex )

	def getIndexVector( self ):
		'''Returns the x, y and z indices of this patch as a vector.'''


		return breve.vector( self.xIndex, self.yIndex, self.zIndex )

	def getLocation( self ):
		'''Returns the location of the patch.'''


		return breve.breveInternalFunctionFinder.getPatchLocation( self, self.patchPointer )

	def getMooreNeighborhood( self ):
		'''Returns a list of the 2D Moore neighbors of this patch.  The 2D Moore neighbors are the 8 patches adjacent to this patch, including diagonally, on the X/Y plane. <P> If you intend to use the neighborhood frequently, you should  call this method once and keep a copy of the list.'''

		result = breve.objectList()

		result = self.getVonNeumannNeighborhood()
		if self.getPatchToUpperLeft():
			result.append( self.getPatchToUpperLeft() )

		if self.getPatchToUpperRight():
			result.append( self.getPatchToUpperRight() )

		if self.getPatchToLowerLeft():
			result.append( self.getPatchToLowerLeft() )

		if self.getPatchToLowerRight():
			result.append( self.getPatchToLowerRight() )

		return result

	def getPatchAbove( self ):
		'''Returns the patch towards (0, 1, 0).'''


		return self.grid.getPatchAt( self.xIndex, ( self.yIndex + 1 ), self.zIndex )

	def getPatchBelow( self ):
		'''Returns the patch towards (0, -1, 0).'''


		return self.grid.getPatchAt( self.xIndex, ( self.yIndex - 1 ), self.zIndex )

	def getPatchChemicals( self ):
		'''returns a list of all patch chemicals in the simulation'''


		self.grid.getPatchChemcials()

	def getPatchToEast( self ):
		'''Assumes that the patches are being observed from an XZ-plane. Returns the patch towards (1, 0, 0). '''


		return self.grid.getPatchAt( ( self.xIndex + 1 ), self.yIndex, self.zIndex )

	def getPatchToLeft( self ):
		'''Returns the patch towards (-1, 0, 0).'''


		return self.grid.getPatchAt( ( self.xIndex - 1 ), self.yIndex, self.zIndex )

	def getPatchToLowerLeft( self ):
		'''Returns the patch towards (-1, -1, 0).'''


		return self.grid.getPatchAt( ( self.xIndex - 1 ), ( self.yIndex - 1 ), self.zIndex )

	def getPatchToLowerRight( self ):
		'''Returns the patch towards (1, -1, 0).'''


		return self.grid.getPatchAt( ( self.xIndex + 1 ), ( self.yIndex - 1 ), self.zIndex )

	def getPatchToNorth( self ):
		'''Assumes that the patches are being observed from an XZ-plane. Returns the patch towards (0, 0, -1). '''


		return self.grid.getPatchAt( self.xIndex, self.yIndex, ( self.zIndex - 1 ) )

	def getPatchToRight( self ):
		'''Returns the patch towards (1, 0, 0).'''


		return self.grid.getPatchAt( ( self.xIndex + 1 ), self.yIndex, self.zIndex )

	def getPatchToSouth( self ):
		'''Assumes that the patches are being observed from an XZ-plane. Returns the patch towards (0, 0, 1). '''


		return self.grid.getPatchAt( self.xIndex, self.yIndex, ( self.zIndex + 1 ) )

	def getPatchToUpperLeft( self ):
		'''Returns the patch towards (-1, 1, 0).'''


		return self.grid.getPatchAt( ( self.xIndex - 1 ), ( self.yIndex + 1 ), self.zIndex )

	def getPatchToUpperRight( self ):
		'''Returns the patch towards (1, 1, 0).'''


		return self.grid.getPatchAt( ( self.xIndex + 1 ), ( self.yIndex + 1 ), self.zIndex )

	def getPatchToWest( self ):
		'''Assumes that the patches are being observed from an XZ-plane. Returns the patch towards (-1, 0, 0). '''


		return self.grid.getPatchAt( ( self.xIndex - 1 ), self.yIndex, self.zIndex )

	def getPatchTowardsMinusX( self ):
		'''Returns the patch towards (-1, 0, 0).'''


		return self.grid.getPatchAt( ( self.xIndex - 1 ), self.yIndex, self.zIndex )

	def getPatchTowardsMinusXMinusY( self ):
		'''returns the patch towards (-1, -1, 0)'''


		return self.grid.getPatchAt( ( self.xIndex - 1 ), ( self.yIndex - 1 ), self.zIndex )

	def getPatchTowardsMinusXPlusY( self ):
		'''returns the patch towards (-1, 1, 0)'''


		return self.grid.getPatchAt( ( self.xIndex - 1 ), ( self.yIndex + 1 ), self.zIndex )

	def getPatchTowardsMinusY( self ):
		'''Returns the patch towards (0, -1, 0).'''


		return self.grid.getPatchAt( self.xIndex, ( self.yIndex - 1 ), self.zIndex )

	def getPatchTowardsMinusZ( self ):
		'''Returns the patch towards (0, 0, -1).'''


		return self.grid.getPatchAt( self.xIndex, self.yIndex, ( self.zIndex - 1 ) )

	def getPatchTowardsPlusX( self ):
		'''Returns the patch towards (1, 0, 0).'''


		return self.grid.getPatchAt( ( self.xIndex + 1 ), self.yIndex, self.zIndex )

	def getPatchTowardsPlusXMinusY( self ):
		'''returns the patch towards (1, -1, 0)'''


		return self.grid.getPatchAt( ( self.xIndex + 1 ), ( self.yIndex - 1 ), self.zIndex )

	def getPatchTowardsPlusXPlusY( self ):
		'''returns the patch towards (1, 1, 0)'''


		return self.grid.getPatchAt( ( self.xIndex + 1 ), ( self.yIndex + 1 ), self.zIndex )

	def getPatchTowardsPlusY( self ):
		'''Returns the patch towards (0, 1, 0).'''


		return self.grid.getPatchAt( self.xIndex, ( self.yIndex + 1 ), self.zIndex )

	def getPatchTowardsPlusZ( self ):
		'''Returns the patch towards (0, 0, 1).'''


		return self.grid.getPatchAt( self.xIndex, self.yIndex, ( self.zIndex + 1 ) )

	def getTransparency( self ):
		'''Returns the transparency value of the patch.'''


		return self.transparency

	def getVonNeumannNeighborhood( self ):
		'''Returns a list of the 2D Von Neumann neighbors of this patch.  The 2D Von Neumann neighbors are the 8 patches located above, below,  to the left of and to the right of this patch on the X/Y plane. <P> If you intend to use the neighborhood frequently, you should  call this method once and keep a copy of the list.'''

		result = breve.objectList()

		if self.getPatchAbove():
			result.append( self.getPatchAbove() )

		if self.getPatchToRight():
			result.append( self.getPatchToRight() )

		if self.getPatchBelow():
			result.append( self.getPatchBelow() )

		if self.getPatchToLeft():
			result.append( self.getPatchToLeft() )

		return result

	def getXIndex( self ):
		'''Returns the x index of the patch.'''


		return self.xIndex

	def getYIndex( self ):
		'''Returns the y index of the patch.'''


		return self.yIndex

	def getZIndex( self ):
		'''Returns the z index of the patch.'''


		return self.zIndex

	def increaseConcentration( self, theDiffusable, theValue ):
		'''increases the amount of a OBJECT(PatchChecmical) in this patch.'''


		self.grid.increaseConcentration( theDiffusable, theValue, self.xIndex, self.yIndex, self.zIndex )

	def initNeighbors( self ):
		''''''



	def initPatch( self ):
		'''This method does nothing in its default implementation.  You can implement your own init-patch method in your patch class in order to perform initialization on  the patch.  This method is called after all the neighbor objects have been  created--if you do initialization in the init method, the neighbors will not  be initialized.'''



	def initWith( self, gp, x, y, z ):
		'''Used internally by the grid patch system when setting up the  patches.  Do not call this method.  Just don't.  I know it's  tempting, but it's really better for all of us that you don't call it.'''


		self.grid = gp
		self.xIndex = x
		self.yIndex = y
		self.zIndex = z

	def setColor( self, newColor ):
		'''Sets the color of the patch to newColor.  Bear in mind that you'll also  need to set the transparency of the patch if you want the patch to be displayed.  See the method METHOD(set-transparency) for more information.'''


		if ( not self.patchPointer ):
			print '''set-color called for %s before patch was initialized''' % (  self )
			print '''use method "init-patch" instead of "init"'''
			return


		self.color = newColor
		breve.breveInternalFunctionFinder.setPatchColor( self, self.patchPointer, newColor )

	def setConcentration( self, theDiffusable, theValue ):
		'''returns the concentration of a OBJECT(PatchChecmical) in this patch.'''


		self.grid.setConcentration( theDiffusable, self.xIndex, self.yIndex, self.zIndex, theValue )

	def setPatchPointer( self, p ):
		'''Used internally when setting up the patches.  Do not call this method.'''


		self.patchPointer = p
		self.setTransparency( 0.500000 )

	def setTransparency( self, alphaValue ):
		'''Sets the transparency of the patch to alphaValue.  alphaValue is a number between 0.0 (fully transparent) and 1.0 (fully opaque).  The default value is 0.5, semi-transparent.'''


		if ( not self.patchPointer ):
			print '''set-transparency called for %s before patch was initialized''' % (  self )
			print '''use method "init-patch" instead of "init"'''
			return


		self.transparency = alphaValue
		breve.breveInternalFunctionFinder.setPatchTransparency( self, self.patchPointer, alphaValue )

	def updateColors( self ):
		'''Used internally.  Synchronizes the patch's color with recently  dearchived values. '''


		self.setColor( self.color )
		self.setTransparency( self.transparency )


breve.Patch = Patch
breve.Patches = Patch



