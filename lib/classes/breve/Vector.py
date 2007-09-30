
# Note: this file was automatically converted to Python from the
# original steve-language source code.  Please see the original
# file for more detailed comments and documentation.


import breve

class Vector( breve.Object ):
	def __init__( self ):
		breve.Object.__init__( self )
		self.dim = 0
		self.vectorList = breve.objectList()
		self.vectorPointer = None
		Vector.init( self )

	def addScalar( self, scalarValue ):
		'''Adds the number scalarValue to each element in the vector.'''

		breve.breveInternalFunctionFinder.vectorAddScalar( self, self.vectorPointer, scalarValue )

	def addValues( self, otherVector, scale = 1.000000 ):
		'''Adds this vector to otherVector leaving the result in otherVector. The matrices must be of the same size.  The  optional argument scale allows otherVector to be scaled before  adding it to this vector. <P> This method is hardware accelerated where supported.'''

		breve.breveInternalFunctionFinder.vectorAddScaled( self, self.vectorPointer, otherVector.getVectorPointer(), scale )

	def archive( self ):
		n = 0

		n = 0
		while ( n < self.dim ):
			self.vectorList[ n ] = self.getValue( n )

			n = ( n + 1 )

		return breve.Object.archive( self )

	def copy( self, otherVector ):
		'''Copies the contents of otherVector to thisVector. <P> This method is hardware accelerated where supported.'''

		breve.breveInternalFunctionFinder.vectorCopy( self, otherVector.getVectorPointer(), self.vectorPointer )

	def copyToBlueChannel( self, theImage, scale = 1.000000 ):
		'''Copies the contents of this vector to the blue channel of theImage.   The optional argument scale specifies how the vector elements should be scaled. <P> This method, and it's counterparts METHOD(copy-to-red-channel) and  METHOD(copy-to-green-channel) are far faster than looping manually through the vector values.'''

		breve.breveInternalFunctionFinder.vectorCopyToImage( self, self.vectorPointer, theImage.getImageData(), 2, scale )

	def copyToGreenChannel( self, theImage, scale = 1.000000 ):
		'''Copies the contents of this vector to the green channel of theImage.   The optional argument scale specifies how the vector elements should be scaled. <P> This method, and it's counterparts METHOD(copy-to-blue-channel) and  METHOD(copy-to-red-channel) are far faster than looping manually through the vector values.'''

		breve.breveInternalFunctionFinder.vectorCopyToImage( self, self.vectorPointer, theImage.getImageData(), 1, scale )

	def copyToRedChannel( self, theImage, scale = 1.000000 ):
		'''Copies the contents of this vector to the red channel of theImage.   The optional argument scale specifies how the vector elements should be scaled. <P> This method, and it's counterparts METHOD(copy-to-green-channel) and  METHOD(copy-to-blue-channel) are far faster than looping manually through the vector values.'''

		breve.breveInternalFunctionFinder.vectorCopyToImage( self, self.vectorPointer, theImage.getImageData(), 0, scale )

	def dearchive( self ):
		n = 0

		self.setSize( breve.length( self.vectorList ) )
		n = 0
		while ( n < self.dim ):
			self.setValue( n, self.vectorList[ n ] )

			n = ( n + 1 )

		return breve.Object.archive( self )

	def destroy( self ):
		if self.vectorPointer:
			breve.breveInternalFunctionFinder.vectorFree( self, self.vectorPointer )


	def getAbsoluteSum( self ):
		'''Returns the sum of the absolute values of all elements in this  vector. <P> This method is hardware accelerated where supported.'''

		return breve.breveInternalFunctionFinder.vectorGetAbsoluteSum( self, self.vectorPointer )

	def getDimension( self ):
		'''Returns the x size of this vector.'''

		return self.dim

	def getValue( self, x ):
		'''Returns the vector value at position (x).'''

		return breve.breveInternalFunctionFinder.vectorGet( self, self.vectorPointer, x )

	def getVectorPointer( self ):
		return self.vectorPointer

	def init( self ):
		self.setSize( 3 )
		return self

	def initWith( self, newDim ):
		'''Set the dimension of the vector to size.  Clears contents.'''

		self.setSize( newDim )
		return self

	def multiplyWithValues( self, otherVector ):
		'''Multiplies each element in this vector with the corresponding  element in otherVector.  This is not regular vector multiplication; rather, it is a way to scale each element in otherVector.'''

		breve.breveInternalFunctionFinder.vectorMulElements( self, self.vectorPointer, otherVector.getVectorPointer() )

	def printMatlabStyle( self ):
		'''Prints the vector in a style similar to MATLAB.'''

		i = 0

		print '['
		i = 0
		while ( i < ( self.dim - 1 ) ):
			print breve.breveInternalFunctionFinder.vectorGet( self, self.vectorPointer, i )
			print ''' '''

			i = ( i + 1 )

		print breve.breveInternalFunctionFinder.vectorGet( self, self.vectorPointer, i )
		print ']'

	def scale( self, scaleValue ):
		'''Scales all elements in the vector by scaleValue. <P> This method is hardware accelerated where supported.'''

		breve.breveInternalFunctionFinder.vectorScale( self, self.vectorPointer, scaleValue )

	def setAllValues( self, value ):
		'''Sets all of the values in the vector to value.'''

		breve.breveInternalFunctionFinder.vectorSetAll( self, self.vectorPointer, value )

	def setSize( self, newDim ):
		'''Sets the size of this vector to the new dimension.'''

		if self.vectorPointer:
			breve.breveInternalFunctionFinder.vectorFree( self, self.vectorPointer )

		self.vectorPointer = breve.breveInternalFunctionFinder.vectorNew( self, newDim )
		self.dim = newDim

	def setValue( self, value, x ):
		'''Sets the vector value at position (x, y, z).'''

		return breve.breveInternalFunctionFinder.vectorSet( self, self.vectorPointer, x, value )

	def setVectorPointer( self, newPointer, newDim ):
		if self.vectorPointer:
			breve.breveInternalFunctionFinder.vectorFree( self, self.vectorPointer )

		self.vectorPointer = newPointer
		self.dim = newDim

	def subtractValues( self, otherVector, scale = 1.000000 ):
		'''Subtracts this vector from this otherVector, leaving the result in otherVector.  This method uses the same mechanism as METHOD(add), but using a negative scale  argument. The optional argument scale allows otherVector to be scaled before  subtracting it from this vector. <P> This method is hardware accelerated where supported.'''

		breve.breveInternalFunctionFinder.vectorAddScaled( self, self.vectorPointer, otherVector.getVectorPointer(), ( -1.000000 * scale ) )

	def transform( self, transformation, result ):
		'''Transforms this OBJECT(Vector) with the OBJECT(Matrix2D) matrix, storing the  output in result.'''

		breve.breveInternalFunctionFinder.matrix2DVectorMultiply( self, transformation.getMatrixPointer(), self.vectorPointer, result.getVectorPointer() )


breve.Vector = Vector
class VectorImage( breve.Image ):
	'''The VectorImage class is a subclass of Image which displays the state of a OBJECT(Vector) object.'''

	def __init__( self ):
		breve.Image.__init__( self )
		self.blue = None
		self.green = None
		self.red = None
		self.scale = 0

	def destroy( self ):
		breve.Image.destroy( self )

	def initWith( self, theVector, theScale = 1.000000 ):
		'''Initializes the VectorImage with a OBJECT(Vector) object.'''

		breve.Image.initWith( self , theVector.getDimension(), theVector.getDimension() )
		self.scale = theScale
		return self

	def iterate( self ):
		if self.red:
			self.red.copyToRedChannel( self, self.scale )

		if self.green:
			self.green.copyToBlueChannel( self, self.scale )

		if self.blue:
			self.blue.copyToBlueChannel( self, self.scale )

		breve.Image.iterate( self )

	def setBlue( self, blueVector ):
		'''Sets the blue channel of the image to vector blueVector.'''

		self.blue = blueVector

	def setGreen( self, greenVector ):
		'''Sets the green channel of the image to vector greenVector.'''

		self.green = greenVector

	def setRed( self, redVector ):
		'''Sets the red channel of the image to vector redVector.'''

		self.red = redVector


breve.VectorImage = VectorImage
# Add our newly created classes to the breve namespace

breve.Vectors = Vector



