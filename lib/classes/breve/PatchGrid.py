
# Note: this file was automatically converted to Python from the
# original steve-language source code.  Please see the original
# file for more detailed comments and documentation.


import breve

class PatchGrid( breve.Abstract ):
	'''OBJECT(PatchGrid) is a class which allows you to create "patches".   A OBJECT(Patch) is a non-physical object which is associated with  a specific area in the 3D world.  This allows you to associate  information or actions with specific areas in space, such as the  presence of a chemical or nutrient at that area. <p> Use METHOD(init-with) to create a PatchGrid.'''

	def __init__( self ):
		breve.Abstract.__init__( self )
		self.blueChemicalMatrix = None
		self.chemicalConcentrationMatrix = breve.hash()
		self.chemicals = breve.objectList()
		self.diffusingChemicals = breve.objectList()
		self.exponentiallyDecayingChemicals = breve.objectList()
		self.greenChemicalMatrix = None
		self.gridPointer = None
		self.linearlyDecayingChemicals = breve.objectList()
		self.location = breve.vector()
		self.patchSize = breve.vector()
		self.patchVolume = 0
		self.patches = breve.objectList()
		self.redChemicalMatrix = None
		self.tempMatrix = None
		self.xSize = 0
		self.ySize = 0
		self.zSize = 0

	def archive( self ):
		return 1

	def dearchive( self ):
		z = 0
		y = 0
		x = 0
		index = 0

		self.gridPointer = breve.breveInternalFunctionFinder.patchGridNew( self, self.location, self.patchSize, self.xSize, self.ySize, self.zSize )
		x = 0
		while ( x < self.xSize ):
			y = 0
			while ( y < self.ySize ):
				z = 0
				while ( z < self.zSize ):
					breve.breveInternalFunctionFinder.setPatchObjectAtIndex( self, self.gridPointer, self.patches[ index ], x, y, z )
					self.patches[ index ].setPatchPointer( breve.breveInternalFunctionFinder.getPatchAtIndex( self, self.gridPointer, x, y, z ) )
					self.patches[ index ].updateColors()
					index = ( index + 1 )

					z = ( z + 1 )


				y = ( y + 1 )


			x = ( x + 1 )

		return 1

	def decayChemicals( self ):
		'''Updates the concentration of each OBJECT(PatchChemical) by reducing the current concentraion by the decay rate.  The decay rate is treated as either a linear or exponential rate depending on the chemical's decay type.  This method is called automatically by METHOD(update-concentrations).'''

		chem = None
		chemMatrixPointer = None

		for chem in self.linearlyDecayingChemicals:
			self.chemicalConcentrationMatrix[ chem ].addScalar( ( chem.getDecayRate() * -1.000000 ) )

		for chem in self.exponentiallyDecayingChemicals:
			chemMatrixPointer = self.chemicalConcentrationMatrix[ chem ].getMatrixPointer()
			breve.breveInternalFunctionFinder.matrix3DCopy( self, chemMatrixPointer, self.tempMatrix )
			breve.breveInternalFunctionFinder.matrix3DAddScaled( self, chemMatrixPointer, self.tempMatrix, ( chem.getDecayRate() * -1.000000 ) )



	def decreaseConcentration( self, theDiffusable, theValue, xIndex, yIndex, zIndex ):
		'''Removes a quantify of the chemical to the patch.  The quantity is divided by the patch volume to get the concentration removed. You don't normally call this directly'''

		currentValue = 0

		currentValue = self.chemicalConcentrationMatrix[ theDiffusable ].getValue( xIndex, yIndex, zIndex )
		currentValue = ( currentValue - ( theValue / self.patchVolume ) )
		self.chemicalConcentrationMatrix[ theDiffusable ].setValue( currentValue, xIndex, yIndex, zIndex )

	def destroy( self ):
		chem = None

		if ( breve.length( self.diffusingChemicals ) > 0 ):
			breve.deleteInstances( self.patches )

		for chem in self.diffusingChemicals:
			breve.deleteInstances( self.chemicalConcentrationMatrix[ chem ] )

		breve.deleteInstances( self.tempMatrix )
		if self.gridPointer:
			breve.breveInternalFunctionFinder.patchGridFree( self, self.gridPointer )


	def diffuseChemicals( self ):
		'''Updates the concentration of each OBJECT(PatchChemical) being tracked in the patch grid by diffusing the chemicals according to their diffusion rates.  This method is called automatically by METHOD(update-concentrations). '''

		chem = None
		chemMatrixPointer = None

		for chem in self.diffusingChemicals:
			chemMatrixPointer = self.chemicalConcentrationMatrix[ chem ].getMatrixPointer()
			breve.breveInternalFunctionFinder.matrix3DDiffuse( self, chemMatrixPointer, self.tempMatrix, chem.getDiffusionRate() )
			breve.breveInternalFunctionFinder.matrix3DAddScaled( self, chemMatrixPointer, self.tempMatrix, self.controller.getIterationStep() )



	def diffuseChemicalsPeriodically( self ):
		'''Updates the concentration of each OBJECT(PatchChemical) being tracked in the patch grid by diffusing the chemicals according to their diffusion rates.  This method is called automatically by METHOD(update-concentrations). '''

		chem = None
		chemMatrixPointer = None

		for chem in self.diffusingChemicals:
			chemMatrixPointer = self.chemicalConcentrationMatrix[ chem ].getMatrixPointer()
			breve.breveInternalFunctionFinder.matrix3DDiffusePeriodic( self, chemMatrixPointer, self.tempMatrix, chem.getDiffusionRate() )
			breve.breveInternalFunctionFinder.matrix3DAddScaled( self, chemMatrixPointer, self.tempMatrix, self.controller.getIterationStep() )



	def disableSmoothDrawing( self ):
		'''Disables smooth drawing for this PatchGrid.  This is the default. See METHOD(enable-smooth-drawing) for details on smooth drawing.'''

		breve.breveInternalFunctionFinder.patchGridSetSmoothDrawing( self, self.gridPointer, 0 )

	def disableVolumetricDrawing( self ):
		'''Disables volumetric drawing for this PatchGrid.  The grid is drawn as individual cubes.  This technique may produce better results  visually, but is far slower than the volumetric rendering.  See  METHOD(enable-volumetric-drawing) for more details.'''

		breve.breveInternalFunctionFinder.patchGridSetDrawWithTexture( self, self.gridPointer, 0 )

	def displayConcentrationInBlue( self, theDiffusable ):
		'''Selects the OBJECT(PatchChemical)'s concentration to use as the red channel when METHOD(display-patch-chemical-concentrations) is called.'''

		self.blueChemicalMatrix = theDiffusable
		self.blueChemicalMatrix = self.chemicalConcentrationMatrix[ theDiffusable ]

	def displayConcentrationInGreen( self, theDiffusable ):
		'''Selects the OBJECT(PatchChemical)'s concentration to use as the red channel when METHOD(display-patch-chemical-concentrations) is called.'''

		self.greenChemicalMatrix = self.chemicalConcentrationMatrix[ theDiffusable ]

	def displayConcentrationInRed( self, theDiffusable ):
		'''Selects the OBJECT(PatchChemical)'s concentration to use as the red channel when METHOD(display-patch-chemical-concentrations) is called.'''

		self.redChemicalMatrix = self.chemicalConcentrationMatrix[ theDiffusable ]

	def enableSmoothDrawing( self ):
		'''Enables smooth drawing for this PatchGrid.  When smooth drawing is  enabled, colors are blended smoothly between patches.  Smooth drawing is disabled by default. <P> Smooth drawing is desirable when a PatchGrid's colors display  continuous gradients as with chemicals or temperature, but is  undesirable when dealing with discrete states, as in cellular  automata simulations. <P> The method METHOD(disable-smooth-drawing) can be used to disable smooth drawing.'''

		breve.breveInternalFunctionFinder.patchGridSetSmoothDrawing( self, self.gridPointer, 1 )

	def enableVolumetricDrawing( self ):
		'''Enables texture drawing for this PatchGrid.  This is the default. With volumetric rendering, the grid is drawn as a series of overlapping textured planes.  The rendering is fast, but the overlapping planes may  leave some visual artifacts.  To revert to the older style of cube  rendering, see the method METHOD(disable-volumetric-drawing).'''

		breve.breveInternalFunctionFinder.patchGridSetDrawWithTexture( self, self.gridPointer, 1 )

	def getConcentration( self, theDiffusable, xIndex, yIndex, zIndex ):
		'''Gets the concentration of the chemical in the patch.  Note that this is a normalized concentration.  If you want the total concentration, multiply by the OBJECT(Patch) size. You don't normally call this directly'''

		return self.chemicalConcentrationMatrix[ theDiffusable ].getValue( xIndex, yIndex, zIndex )

	def getDescription( self ):
		return '''%s x %s x %s''' % (  self.xSize, self.ySize, self.zSize )

	def getGridPointer( self ):
		return self.gridPointer

	def getLocation( self ):
		'''returns the center of the grid.'''

		return self.location

	def getPatch( self, theLocation ):
		'''Returns the OBJECT(Patch) in which theLocation resides.  Make sure you test for the validity of the returned object, since it is possible that there is no patch at the specified location.'''

		return breve.breveInternalFunctionFinder.objectAtLocation( self, self.gridPointer, theLocation )

	def getPatchAt( self, xind, yind, zind ):
		'''Returns the patch object associated with the specified indices.  Note that this method may return a NULL object in the case that the  indices given are out of bounds.'''

		return breve.breveInternalFunctionFinder.patchObjectAtIndex( self, self.gridPointer, xind, yind, zind )

	def getPatchChemicals( self ):
		'''Returns all the chemicals currently being tracked in the patch grid.'''

		return self.chemicals

	def getPatchSize( self ):
		'''Returns the vector size of a single patch.'''

		return self.patchSize

	def getPatches( self ):
		'''Returns a list of all patch objects.'''

		return self.patches

	def getXCount( self ):
		'''If the grid is initialized, returns the size of the grid on the X-axis.'''

		return self.xSize

	def getYCount( self ):
		'''If the grid is initialized, returns the size of the grid on the Y-axis.'''

		return self.ySize

	def getZCount( self ):
		'''If the grid is initialized, returns the size of the grid on the Z-axis.'''

		return self.zSize

	def increaseConcentration( self, theDiffusable, theValue, xIndex, yIndex, zIndex ):
		'''Adds a quantify of the chemical to the patch.  The quantity is divided by the patch volume to get the concentration added. You don't normally call this directly'''

		currentValue = 0

		currentValue = self.chemicalConcentrationMatrix[ theDiffusable ].getValue( xIndex, yIndex, zIndex )
		currentValue = ( currentValue + ( theValue / self.patchVolume ) )
		self.chemicalConcentrationMatrix[ theDiffusable ].setValue( currentValue, xIndex, yIndex, zIndex )

	def initAt( self, x, y, z, gridCenter = breve.vector( 0.000000, 0.000000, 0.000000 ), pSize = breve.vector( 1.000000, 1.000000, 1.000000 ), patchclass = "Patch" ):
		zCount = 0
		yCount = 0
		xCount = 0
		patch = None

		self.xSize = x
		self.ySize = y
		self.zSize = z
		self.patchSize = pSize
		self.patchVolume = ( ( self.patchSize.x * self.patchSize.y ) * self.patchSize.z )
		self.location = gridCenter
		self.gridPointer = breve.breveInternalFunctionFinder.patchGridNew( self, gridCenter, self.patchSize, x, y, z )
		xCount = 0
		yCount = 0
		zCount = 0
		while ( xCount < x ):
			yCount = 0
			while ( yCount < y ):
				zCount = 0
				while ( zCount < z ):
					patch = self.controller.makeNewInstance( patchclass )
					patch.initWith( self, xCount, yCount, zCount )
					self.patches.append( patch )
					breve.breveInternalFunctionFinder.setPatchObjectAtIndex( self, self.gridPointer, patch, xCount, yCount, zCount )
					patch.setPatchPointer( breve.breveInternalFunctionFinder.getPatchAtIndex( self, self.gridPointer, xCount, yCount, zCount ) )
					self.addDependency( patch )
					zCount = ( zCount + 1 )


				yCount = ( yCount + 1 )


			xCount = ( xCount + 1 )


		xCount = 0
		yCount = 0
		zCount = 0
		while ( xCount < x ):
			yCount = 0
			while ( yCount < y ):
				zCount = 0
				while ( zCount < z ):
					breve.breveInternalFunctionFinder.patchObjectAtIndex( self, self.gridPointer, xCount, yCount, zCount ).initNeighbors()
					breve.breveInternalFunctionFinder.patchObjectAtIndex( self, self.gridPointer, xCount, yCount, zCount ).initPatch()
					zCount = ( zCount + 1 )


				yCount = ( yCount + 1 )


			xCount = ( xCount + 1 )


		self.tempMatrix = breve.breveInternalFunctionFinder.matrix3DNew( self, x, y, z )
		return self

	def initWith( self, x, y, z, center = breve.vector( 0.000000, 0.000000, 0.000000 ), pSize = breve.vector( 1.000000, 1.000000, 1.000000 ), patchclass = "Patch" ):
		'''Creates a PatchGrid centered at center, in which each OBJECT(Patch)  is a member of patchclass and is the size pSize and in which the total grid  dimensions are x by y by z.   <p>  Bear in mind that this will create a large number of objects:  x * y * z.  If you have an iterate method in your patch-class,  then this can amount to a great deal of computation which can slow down your simulation.'''

		return self.initAt( x, y, z, center, pSize, patchclass )

	def setConcentration( self, theDiffusable, xIndex, yIndex, zIndex, theValue ):
		'''Sets the concentration of the chemical in the patch.  Note that this is a normalized concentration.  If you want to use total concentration, multiply by the OBJECT(Patch) size first. You don't normally call this directly'''

		self.chemicalConcentrationMatrix[ theDiffusable ].setValue( theValue, xIndex, yIndex, zIndex )

	def setPatchColorsToChemicalConcentrations( self ):
		color = breve.vector()

		color = breve.vector( 0, 0, 0 )
		breve.breveInternalFunctionFinder.matrix3DSetAll( self, self.tempMatrix, 0.000000 )
		if self.redChemicalMatrix:
			breve.breveInternalFunctionFinder.patchGridCopyColorFrom3DMatrix( self, self.gridPointer, self.redChemicalMatrix.getMatrixPointer(), 0, 1.000000 )
			breve.breveInternalFunctionFinder.matrix3DAddScaled( self, self.tempMatrix, self.redChemicalMatrix.getMatrixPointer(), 0.330000 )


		if self.greenChemicalMatrix:
			breve.breveInternalFunctionFinder.patchGridCopyColorFrom3DMatrix( self, self.gridPointer, self.greenChemicalMatrix.getMatrixPointer(), 1, 1.000000 )
			breve.breveInternalFunctionFinder.matrix3DAddScaled( self, self.tempMatrix, self.greenChemicalMatrix.getMatrixPointer(), 0.330000 )


		if self.blueChemicalMatrix:
			breve.breveInternalFunctionFinder.patchGridCopyColorFrom3DMatrix( self, self.gridPointer, self.blueChemicalMatrix.getMatrixPointer(), 2, 1.000000 )
			breve.breveInternalFunctionFinder.matrix3DAddScaled( self, self.tempMatrix, self.blueChemicalMatrix.getMatrixPointer(), 0.330000 )


		breve.breveInternalFunctionFinder.patchGridCopyColorFrom3DMatrix( self, self.gridPointer, self.tempMatrix, 3, 1.000000 )

	def track( self, theDiffusable ):
		'''Adds a new OBJECT(PatchChemical) to the patch grid.  This allows the patch grid to track the concentrations of the chemcial in each patch and allow diffusion between the patches.'''

		if self.chemicalConcentrationMatrix[ theDiffusable ]:
			print '''%s is already being tracked by the PatchGrid.''' % (  theDiffusable )
			return


		self.chemicals.append( theDiffusable )
		if theDiffusable.decayIsLinear():
			self.linearlyDecayingChemicals.append( theDiffusable )
		else:
			if theDiffusable.decayIsExponential():
				self.exponentiallyDecayingChemicals.append( theDiffusable )


		if ( theDiffusable.getDiffusionRate() > 0.000000 ):
			self.diffusingChemicals.append( theDiffusable )

		self.chemicalConcentrationMatrix[ theDiffusable ] = breve.createInstances( breve.Matrix3D, 1 ).initWith( self.getXCount(), self.getYCount(), self.getZCount() )

	def untrack( self, theDiffusable ):
		'''Removes a OBJECT(PatchChemcial) from the patch grid.'''

		chem = None
		newChems = breve.objectList()

		if self.chemicalConcentrationMatrix[ theDiffusable ]:
			breve.deleteInstances( self.chemicalConcentrationMatrix[ theDiffusable ] )
			for chem in self.chemicals:
				if ( chem != theDiffusable ):
					newChems.append( chem )


			self.chemicals = newChems
			for chem in self.diffusingChemicals:
				if ( chem != theDiffusable ):
					newChems.append( chem )


			self.diffusingChemicals = newChems
			for chem in self.linearlyDecayingChemicals:
				if ( chem != theDiffusable ):
					newChems.append( chem )


			self.linearlyDecayingChemicals = newChems
			for chem in self.exponentiallyDecayingChemicals:
				if ( chem != theDiffusable ):
					newChems.append( chem )


			self.exponentiallyDecayingChemicals = newChems



	def updateConcentrations( self ):
		'''Updates the concentrations of each OBJECT(PatchChemical) being tracked in the patch grid.  All chemicals with a diffusion rate greater than 0 diffuse into adjacent patches at their given rate and all chemicals with a decay rate greater than 0 decay.'''

		self.diffuseChemicals()
		self.decayChemicals()


breve.PatchGrid = PatchGrid
class PatchToroid( breve.PatchGrid ):
	'''OBJECT(PatchToroid) is a subclass of OBJECT(PatchGrid) in which the  patch neighbors are initialized in a toroidal fashion, such that  patches on the edge of the grid are "connected" (via neighbor  relationships) to patches on the opposite edge. <P> Note that since OBJECT(PatchGrid) objects can already be 3 dimensional, that a 3D toroidal OBJECT(PatchGrid) is actually not the traditional  donut shaped toroidal topology--it's the corresponding topology in  4 dimensions.  Now I don't know about you, but I get a headache thinking about this stuff, so let's just agree that "toroid" is not the totally accurate term, but that it's the easiest way to describe what's going on here.'''

	def __init__( self ):
		breve.PatchGrid.__init__( self )

	def diffuseChemicals( self ):
		'''Updates the concentration of each OBJECT(PatchChemical) being tracked in the patch grid by diffusing the chemicals according to their diffusion rates.  This method is called automatically by METHOD(update-concentrations). '''

		chem = None
		chemMatrixPointer = None

		for chem in self.diffusingChemicals:
			chemMatrixPointer = self.chemicalConcentrationMatrix[ chem ].getMatrixPointer()
			breve.breveInternalFunctionFinder.matrix3DDiffusePeriodic( self, chemMatrixPointer, self.tempMatrix, chem.getDiffusionRate() )
			breve.breveInternalFunctionFinder.matrix3DAddScaled( self, chemMatrixPointer, self.tempMatrix, 1.000000 )



	def getPatchAt( self, xind, yind, zind ):
		'''Returns the patch object associated with the specified indices.   If the indices are out of bounds, they will be "wrapped around" to the other side of the PatchToroid.'''

		if ( xind >= self.xSize ):
			xind = ( xind % self.xSize )
		else:
			if ( xind < 0 ):
				xind = ( self.xSize + ( xind % self.xSize ) )


		if ( yind >= self.ySize ):
			yind = ( yind % self.ySize )
		else:
			if ( yind < 0 ):
				yind = ( self.ySize + ( yind % self.ySize ) )


		if ( zind >= self.zSize ):
			zind = ( zind % self.zSize )
		else:
			if ( zind < 0 ):
				zind = ( self.zSize + ( zind % self.zSize ) )


		return breve.breveInternalFunctionFinder.patchObjectAtIndex( self, self.gridPointer, xind, yind, zind )


breve.PatchToroid = PatchToroid
# Add our newly created classes to the breve namespace

breve.PatchGrids = PatchGrid
breve.PatchToroids = PatchToroid



