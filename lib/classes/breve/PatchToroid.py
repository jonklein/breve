
import breve

class PatchToroid( breve.PatchGrid ):
	'''OBJECT(PatchToroid) is a subclass of OBJECT(PatchGrid) in which the  patch neighbors are initialized in a toroidal fashion, such that  patches on the edge of the grid are "connected" (via neighbor  relationships) to patches on the opposite edge. <P> Note that since OBJECT(PatchGrid) objects can already be 3 dimensional, that a 3D toroidal OBJECT(PatchGrid) is actually not the traditional  donut shaped toroidal topology--it's the corresponding topology in  4 dimensions.  Now I don't know about you, but I get a headache thinking about this stuff, so let's just agree that "toroid" is not the totally accurate term, but that it's the easiest way to describe what's going on here.'''

	__slots__ = [ ]

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
breve.PatchToroids = PatchToroid



