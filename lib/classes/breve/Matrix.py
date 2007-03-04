
import breve

class Matrix( breve.Object ):
	'''Matrix2D objects store an arbitrary sized 2D grid of arbitrary floating  point data. <P> Several of these matrix operations are built on top of the BLAS library and may be hardware accelerated on certain processors if the platform provides a hardware accelerated BLAS library.  In particular, AltiVec  acceleration is provided on G4 processors with Mac OS X.   <P> Hardware accelerated methods sometimes preform multiple operations  simultaneously for the same computational cost.  The METHOD(add) operation, for example, also scales one of the matrices being added. When using these methods, it is often beneficial to structure code to take advantage of all of the operations preformed.  It is far  more efficient to scale and add a matrix simultaneously using METHOD(add) than to first scale using METHOD(scale) and then add using METHOD(add). <P> Technical note: matrix objects are implemented using single precision  floats, while much of the rest of the breve simulation environment uses  double precision floating point math.'''

	__slots__ = [ 'archiveList', 'matrixPointer', ]

	def __init__( self ):
		breve.Object.__init__( self )
		self.archiveList = []
		self.matrixPointer = None

	def getMatrixPointer( self ):
		''''''


		return self.matrixPointer



breve.Matrix = Matrix
class Matrix2D( breve.Matrix ):
	''''''

	__slots__ = [ 'xDim', 'yDim', ]

	def __init__( self ):
		breve.Matrix.__init__( self )
		self.xDim = 0
		self.yDim = 0

	def addScalar( self, scalarValue ):
		'''Adds the number scalarValue to each element in the matrix.'''


		breve.breveInternalFunctionFinder.matrix2DAddScalar( self, self.matrixPointer, scalarValue )

	def addValues( self, otherMatrix, scale = 1.000000 ):
		'''Adds this matrix to otherMatrix leaving the result in otherMatrix. The matrices must be of the same size.  The  optional argument scale allows otherMatrix to be scaled before  adding it to this matrix. <P> This method is hardware accelerated where supported.'''


		breve.breveInternalFunctionFinder.matrix2DAddScaled( self, self.matrixPointer, otherMatrix.getMatrixPointer(), scale )

	def archive( self ):
		''''''

		y = 0
		x = 0

		
		return breve.Matrix.archive( self,)


	def computeDiffusionMatrix( self, chemicalMatrix, scale = 1.000000 ):
		'''Sets the contents of this matrix to a diffusion rate from the matrix chemicalMatrix.  chemicalMatrix is treated as a matrix of spatial  chemical concentrations, and the resulting diffusion matrix gives the  approximate rates of diffusion of the chemical. <P> This is done by sampling each concentration's local neighborhood  according to the following matrix: <pre> 0 1 0 1 -4 1 0 1 0 </pre> <P> chemicalMatrix is assumed to have real boundary conditions so that	  the chemical will not flow beyond the edges of the matrix. <P> The optional scale argument may be used to scale the resulting scale  matrix.'''


		breve.breveInternalFunctionFinder.matrix2DDiffuse( self, chemicalMatrix.getMatrixPointer(), self.matrixPointer, scale )

	def computePeriodicDiffusionMatrix( self, chemicalMatrix, scale = 1.000000 ):
		'''Sets the contents of this matrix to a diffusion rate from the matrix chemicalMatrix.  chemicalMatrix is treated as a matrix of spatial  chemical concentrations, and the resulting diffusion matrix gives the  approximate rates of diffusion of the chemical. <P> This is done by sampling each concentration's local neighborhood  according to the following matrix: <pre> 0 1 0 1 -4 1 0 1 0 </pre> <P> chemicalMatrix is assumed to have periodic boundary conditions so that	  the chemical will flow freely from the edges of the matrix to the  other side. <P> The optional scale argument may be used to scale the resulting scale  matrix.'''


		breve.breveInternalFunctionFinder.matrix2DDiffusePeriodic( self, chemicalMatrix.getMatrixPointer(), self.matrixPointer, scale )

	def copy( self, otherMatrix ):
		'''Copies the contents of otherMatrix to thisMatrix. <P> This method is hardware accelerated where supported.'''


		breve.breveInternalFunctionFinder.matrix2DCopy( self, otherMatrix.getMatrixPointer(), self.matrixPointer )

	def copyToBlueChannel( self, theImage, scale = 1.000000 ):
		'''Copies the contents of this matrix to the blue channel of theImage.   The optional argument scale specifies how the matrix elements should be scaled. <P> This method, and it's counterparts METHOD(copy-to-red-channel) and  METHOD(copy-to-green-channel) are far faster than looping manually through the matrix values.'''


		breve.breveInternalFunctionFinder.matrix2DCopyToImage( self, self.matrixPointer, theImage.getImageData(), 2, scale )

	def copyToGreenChannel( self, theImage, scale = 1.000000 ):
		'''Copies the contents of this matrix to the green channel of theImage.   The optional argument scale specifies how the matrix elements should be scaled. <P> This method, and it's counterparts METHOD(copy-to-blue-channel) and  METHOD(copy-to-red-channel) are far faster than looping manually through the matrix values.'''


		breve.breveInternalFunctionFinder.matrix2DCopyToImage( self, self.matrixPointer, theImage.getImageData(), 1, scale )

	def copyToRedChannel( self, theImage, scale = 1.000000 ):
		'''Copies the contents of this matrix to the red channel of theImage.   The optional argument scale specifies how the matrix elements should be scaled. <P> This method, and it's counterparts METHOD(copy-to-green-channel) and  METHOD(copy-to-blue-channel) are far faster than looping manually through the matrix values.'''


		breve.breveInternalFunctionFinder.matrix2DCopyToImage( self, self.matrixPointer, theImage.getImageData(), 0, scale )

	def dearchive( self ):
		''''''

		y = 0
		x = 0

		self.setSize( breve.length( self.archiveList ), breve.length( self.archiveList[ 0 ] ) )
		
		return breve.Matrix.dearchive( self,)


	def destroy( self ):
		''''''


		if self.matrixPointer:
			breve.breveInternalFunctionFinder.matrix2DFree( self, self.matrixPointer )


	def getAbsoluteSum( self ):
		'''Returns the sum of the absolute values of all elements in this  matrix. <P> This method is hardware accelerated where supported.'''


		return breve.breveInternalFunctionFinder.matrix2DGetAbsoluteSum( self, self.matrixPointer )


	def getValue( self, x, y ):
		'''Returns the matrix value at position (x, y).'''


		return breve.breveInternalFunctionFinder.matrix2DGet( self, self.matrixPointer, x, y )


	def getXDimension( self ):
		'''Returns the x size of this matrix.'''


		return self.xDim


	def getYDimension( self ):
		'''Returns the y size of this matrix.'''


		return self.yDim


	def initWith( self, xSize, ySize ):
		'''sets the size to of this matrix to (sSize, ySize).'''


		self.setSize( xSize, ySize )
		return self


	def multiplyWithValues( self, otherMatrix ):
		'''Multiplies each element in this matrix with the corresponding  element in otherMatrix.  This is not regular matrix multiplication; rather, it is a way to scale each element in otherMatrix.'''


		breve.breveInternalFunctionFinder.matrix2DMulElements( self, self.matrixPointer, otherMatrix.getMatrixPointer() )

	def printMatlabStyle( self ):
		'''Prints the matrix in a style similar to MATLAB.'''

		j = 0
		i = 0

		print '''['''
		
		
		print breve.breveInternalFunctionFinder.matrix2DGet( self, self.matrixPointer, i, j )
		print ''']'''

	def scale( self, scaleValue ):
		'''Scales all elements in the matrix by scaleValue. <P> This method is hardware accelerated where supported.'''


		breve.breveInternalFunctionFinder.matrix2DScale( self, self.matrixPointer, scaleValue )

	def setAllValues( self, value ):
		'''Sets all of the values in the matrix to value.'''


		breve.breveInternalFunctionFinder.matrix2DSetAll( self, self.matrixPointer, value )

	def setMatrixPointer( self, newPointer, xSize, ySize ):
		''''''


		if self.matrixPointer:
			breve.breveInternalFunctionFinder.matrix2DFree( self, self.matrixPointer )

		self.matrixPointer = newPointer
		self.xDim = xSize
		self.yDim = ySize

	def setSize( self, xSize, ySize ):
		'''Sets the size of this matrix to (xSize, ySize).		'''


		if self.matrixPointer:
			breve.breveInternalFunctionFinder.matrix2DFree( self, self.matrixPointer )

		self.matrixPointer = breve.breveInternalFunctionFinder.matrix2DNew( self, xSize, ySize )
		self.xDim = xSize
		self.yDim = ySize

	def setValue( self, value, x, y ):
		'''Sets the matrix value at position (x, y).'''


		return breve.breveInternalFunctionFinder.matrix2DSet( self, self.matrixPointer, x, y, value )


	def subtractValues( self, otherMatrix, scale = 1.000000 ):
		'''Subtracts this matrix from this otherMatrix, leaving the result in otherMatrix.  This method uses the same mechanism as METHOD(add), but using a negative scale  argument. The optional argument scale allows otherMatrix to be scaled before  subtracting it from this matrix. <P> This method is hardware accelerated where supported.'''


		breve.breveInternalFunctionFinder.matrix2DAddScaled( self, self.matrixPointer, otherMatrix.getMatrixPointer(), ( -1.000000 * scale ) )

	def transform( self, transformation, result ):
		'''Transforms this OBJECT(Vector) with the OBJECT(Matrix2D) matrix, storing the output in result.'''


		breve.breveInternalFunctionFinder.matrix2DMatrixMultiply( self, transformation.getMatrixPointer(), self.matrixPointer, result.getMatrixPointer() )


breve.Matrix2D = Matrix2D
class Matrix3D( breve.Matrix ):
	'''Matrix3D objects store an arbitrary sized 3D grid of arbitrary floating  point data. <P> Several of these matrix operations are built on top of the BLAS library and may be hardware accelerated on certain processors if the platform provides a hardware accelerated BLAS library.  In particular, AltiVec  acceleration is provided on G4 an G5 processors with Mac OS X.   <P> Hardware accelerated methods sometimes preform multiple operations  simultaneously for the same computational cost.  The METHOD(add) operation, for example, also scales one of the matrices being added. When using these methods, it is often beneficial to structure code to take advantage of all of the operations preformed.  It is far  more efficient to scale and add a matrix simultaneously using METHOD(add) than to first scale using METHOD(scale) and then add using METHOD(add). <P> Technical note: matrix objects are implemented using single precision  floats, while much of the rest of the breve simulation environment uses  double precision floating point math.'''

	__slots__ = [ 'xDim', 'yDim', 'zDim', ]

	def __init__( self ):
		breve.Matrix.__init__( self )
		self.xDim = 0
		self.yDim = 0
		self.zDim = 0

	def addScalar( self, scalarValue ):
		'''Adds the number scalarValue to each element in the matrix.'''


		breve.breveInternalFunctionFinder.matrix3DAddScalar( self, self.matrixPointer, scalarValue )

	def addValues( self, otherMatrix, scale = 1.000000 ):
		'''Adds this matrix to otherMatrix leaving the result in otherMatrix. The matrices must be of the same size.  The  optional argument scale allows otherMatrix to be scaled before  adding it to this matrix. <P> This method is hardware accelerated where supported.'''


		breve.breveInternalFunctionFinder.matrix3DAddScaled( self, self.matrixPointer, otherMatrix.getMatrixPointer(), scale )

	def archive( self ):
		''''''

		z = 0
		y = 0
		x = 0

		
		return breve.Matrix.archive( self,)


	def computeDiffusionMatrix( self, chemicalMatrix, scale = 1.000000 ):
		'''Sets the contents of this matrix to a diffusion rate from the matrix chemicalMatrix.  chemicalMatrix is treated as a matrix of spatial  chemical concentrations, and the resulting diffusion matrix gives the  approximate rates of diffiusion of the chemical. <P> This is done by sampling each concentration's local neighborhood  according to the following matrix: <pre> 0 0 0  0 1 0  0 0 0 0 1 0  1-6 1  0 1 0 0 0 0  0 1 0  0 0 0 </pre> <P> chemicalMatrix is assumed to have real boundary conditions so that	  the chemical will not flow beyond the edges of the matrix. <P> The optional scale argument may be used to scale the resulting scale  matrix.'''


		breve.breveInternalFunctionFinder.matrix3DDiffuse( self, chemicalMatrix.getMatrixPointer(), self.matrixPointer, scale )

	def computePeriodicDiffusionMatrix( self, chemicalMatrix, scale = 1.000000 ):
		'''Sets the contents of this matrix to a diffusion rate from the matrix chemicalMatrix.  chemicalMatrix is treated as a matrix of spatial  chemical concentrations, and the resulting diffusion matrix gives the  approximate rates of diffiusion of the chemical. <P> This is done by sampling each concentration's local neighborhood  according to the following matrix: <pre> 0 0 0  0 1 0  0 0 0 0 1 0  1-6 1  0 1 0 0 0 0  0 1 0  0 0 0 </pre> <P> chemicalMatrix is assumed to have periodic boundary conditions so that	  the chemical will flow freely from the edges of the matrix to the  other side. <P> The optional scale argument may be used to scale the resulting scale  matrix.'''


		breve.breveInternalFunctionFinder.matrix3DDiffusePeriodic( self, chemicalMatrix.getMatrixPointer(), self.matrixPointer, scale )

	def copy( self, otherMatrix ):
		'''Copies the contents of otherMatrix to thisMatrix. <P> This method is hardware accelerated where supported.'''


		breve.breveInternalFunctionFinder.matrix3DCopy( self, otherMatrix.getMatrixPointer(), self.matrixPointer )

	def copySliceToAlphaChannel( self, thePlane, theImage, scale = 1.000000 ):
		'''Copies the contents of this matrix to the alpha channel of theImage, scaled by the optional scale argument. <P> This method, and it's counterparts METHOD(copy-to-red-channel) and  METHOD(copy-to-green-channel) are far faster than looping manually through the matrix values.'''


		breve.breveInternalFunctionFinder.matrix3DCopyToImage( self, self.matrixPointer, thePlane, theImage.getImageData(), 3, scale )

	def copySliceToBlueChannel( self, thePlane, theImage, scale = 1.000000 ):
		'''Copies the contents of this matrix to the blue channel of theImage.   scaled by the optional scale argument. <P> This method, and it's counterparts METHOD(copy-to-red-channel) and  METHOD(copy-to-green-channel) are far faster than looping manually through the matrix values.'''


		breve.breveInternalFunctionFinder.matrix3DCopyToImage( self, self.matrixPointer, thePlane, theImage.getImageData(), 2, scale )

	def copySliceToRedChannel( self, thePlane, theImage, scale = 1.000000 ):
		'''Copies the contents of this matrix to the red channel of theImage, scaled by the optional scale argument. <P> This method, and it's counterparts METHOD(copy-to-green-channel) and  METHOD(copy-to-blue-channel) are far faster than looping manually through the matrix values.'''


		breve.breveInternalFunctionFinder.matrix3DCopyToImage( self, self.matrixPointer, thePlane, theImage.getImageData(), 0, scale )

	def copySlicedToGreenChannel( self, thePlane, theImage, scale = 1.000000 ):
		'''Copies the contents of this matrix to the green channel of theImage, scaled by the optional scale argument. <P> This method, and it's counterparts METHOD(copy-to-blue-channel) and  METHOD(copy-to-red-channel) are far faster than looping manually through the matrix values.'''


		breve.breveInternalFunctionFinder.matrix3DCopyToImage( self, self.matrixPointer, thePlane, theImage.getImageData(), 1, scale )

	def copyToAlphaChannel( self, grid, scale = 1.000000 ):
		'''Copies the contents of this matrix to the alpha channel of the OBJECT(PatchGrid), scaled by the optional scale value.'''


		breve.breveInternalFunctionFinder.patchGridCopyColorFrom3DMatrix( self, grid.getGridPointer(), self.matrixPointer, 3, scale )

	def copyToBlueChannel( self, grid, scale = 1.000000 ):
		'''Copies the contents of this matrix to the blue channel of the OBJECT(PatchGrid), scaled by the optional scale value.'''


		breve.breveInternalFunctionFinder.patchGridCopyColorFrom3DMatrix( self, grid.getGridPointer(), self.matrixPointer, 2, scale )

	def copyToGreenChannel( self, grid, scale = 1.000000 ):
		'''Copies the contents of this matrix to the green channel of the OBJECT(PatchGrid), scaled by the optional scale value.'''


		breve.breveInternalFunctionFinder.patchGridCopyColorFrom3DMatrix( self, grid.getGridPointer(), self.matrixPointer, 1, scale )

	def copyToRedChannel( self, grid, scale = 1.000000 ):
		'''Copies the contents of this matrix to the red channel of the OBJECT(PatchGrid), scaled by the optional scale value.'''


		breve.breveInternalFunctionFinder.patchGridCopyColorFrom3DMatrix( self, grid.getGridPointer(), self.matrixPointer, 0, scale )

	def dearchive( self ):
		''''''

		z = 0
		y = 0
		x = 0

		self.setSize( breve.length( self.archiveList ), breve.length( self.archiveList[ 0 ] ), breve.length( self.archiveList[ 0 ][ 0 ] ) )
		
		return breve.Matrix.dearchive( self,)


	def destroy( self ):
		''''''


		if self.matrixPointer:
			breve.breveInternalFunctionFinder.matrix3DFree( self, self.matrixPointer )


	def getAbsoluteSum( self ):
		'''Returns the sum of the absolute values of all elements in this  matrix. <P> This method is hardware accelerated where supported.'''


		return breve.breveInternalFunctionFinder.matrix3DGetAbsoluteSum( self, self.matrixPointer )


	def getValue( self, x, y, z ):
		'''Returns the matrix value at position (x, y, z).'''


		return breve.breveInternalFunctionFinder.matrix3DGet( self, self.matrixPointer, x, y, z )


	def initWith( self, xSize, ySize, zSize ):
		'''sets the size to of this matrix to (sSize, ySize, zSize).'''


		self.setSize( xSize, ySize, zSize )
		self.xDim = xSize
		self.yDim = ySize
		self.zDim = zSize
		return self


	def multiplyWithValues( self, otherMatrix ):
		'''Multiplies each element in this matrix with the corresponding  element in otherMatrix.  This is not regular matrix multiplication; rather, it is a way to scale each element in otherMatrix.'''


		breve.breveInternalFunctionFinder.matrix3DMulElements( self, self.matrixPointer, otherMatrix.getMatrixPointer() )

	def scale( self, scaleValue ):
		'''Scales all elements in the matrix by scaleValue. <P> This method is hardware accelerated where supported.'''


		breve.breveInternalFunctionFinder.matrix3DScale( self, self.matrixPointer, scaleValue )

	def setAllValues( self, value ):
		'''Sets all of the values in the matrix to value.'''


		breve.breveInternalFunctionFinder.matrix3DSetAll( self, self.matrixPointer, value )

	def setSize( self, xSize, ySize, zSize ):
		'''Sets the size of this matrix to (xSize, ySize, zSize).'''


		if self.matrixPointer:
			breve.breveInternalFunctionFinder.matrix2DFree( self, self.matrixPointer )

		self.matrixPointer = breve.breveInternalFunctionFinder.matrix3DNew( self, xSize, ySize, zSize )

	def setValue( self, value, x, y, z ):
		'''Sets the matrix value at position (x, y, z).'''


		return breve.breveInternalFunctionFinder.matrix3DSet( self, self.matrixPointer, x, y, z, value )


	def subtractValues( self, otherMatrix, scale = 1.000000 ):
		'''Subtracts this matrix from this otherMatrix, leaving the result in otherMatrix.  This method uses the same mechanism as METHOD(add), but using a negative scale  argument. The optional argument scale allows otherMatrix to be scaled before  subtracting it from this matrix. <P> This method is hardware accelerated where supported.'''


		breve.breveInternalFunctionFinder.matrix3DAddScaled( self, self.matrixPointer, otherMatrix.getMatrixPointer(), ( -1.000000 * scale ) )


breve.Matrix3D = Matrix3D
class MatrixImage( breve.Image ):
	'''The MatrixImage allows three 2D matrices to be copied into the red, green and blue  channels of an image for efficient display.  The image is automatically updated 	 with new matrix values at each iteration.'''

	__slots__ = [ 'blue', 'green', 'red', 'scale', ]

	def __init__( self ):
		breve.Image.__init__( self )
		self.blue = None
		self.green = None
		self.red = None
		self.scale = 0

	def destroy( self ):
		''''''


		breve.Image.destroy( self,)

	def initWith( self, theMatrix, theScale = 1.000000 ):
		'''Initializes the MatrixImage with sizes taken from theMatrix.'''


		breve.Image.initWith( self, theMatrix.getXDimension(), theMatrix.getYDimension() )
		self.scale = theScale
		return self


	def iterate( self ):
		''''''


		if self.red:
			self.red.copyToRedChannel( self, self.scale )

		if self.green:
			self.green.copyToGreenChannel( self, self.scale )

		if self.blue:
			self.blue.copyToBlueChannel( self, self.scale )

		breve.Image.iterate( self,)

	def setBlue( self, m ):
		'''Sets the green channel to the Matrix m.'''


		self.blue = m

	def setGreen( self, m ):
		'''Sets the green channel to the Matrix m.'''


		self.green = m

	def setRed( self, m ):
		'''Sets the red channel to the matrix m.'''


		self.red = m

	def setScaleToMaxValue( self, theMatrix ):
		'''Sets the scale of this matrix image object to the maximum value in the specified matrix.'''


		self.scale = theMatrix.getMaxValue()


breve.MatrixImage = MatrixImage
breve.Matricies = Matrix
breve.Matrices2D = Matrix2D
breve.Matrices3D = Matrix3D



