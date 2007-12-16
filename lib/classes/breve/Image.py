
# Note: this file was automatically converted to Python from the
# original steve-language source code.  Please see the original
# file for more detailed comments and documentation.


import breve

class Image( breve.Abstract ):
	'''The Image class provides an interface to work with images and  textures.  The individual pixels of the image can be read  or changed by the simulation as desired. <P> The image class can read rendered images from the screen using the  method METHOD(read-pixels), so that agents in the 3D world  can have access to real rendered data.  In addition, the method  METHOD(get-pixel-pointer) can be used to provide a pointer to the  RGBA pixel data so that plugins can access and analyze image data. This could be used, among other things, to implement agent vision.'''

	def __init__( self ):
		breve.Abstract.__init__( self )
		self.archiveFile = ''
		self.archiveImageData = None
		self.currentHeight = 0
		self.currentWidth = 0
		self.imageData = None
		self.modified = 0
		self.textureNumber = 0
		Image.init( self )

	def archive( self ):
		if ( not self.archiveFile ):
			self.archiveImageData = breve.breveInternalFunctionFinder.brIImageArchive( self, self.imageData )

		return breve.Abstract.archive( self )

	def dearchive( self ):
		if self.archiveFile:
			self.load( self.archiveFile )
		else:
			self.imageData = breve.breveInternalFunctionFinder.brIImageDearchive( self, self.archiveImageData )

		self.textureNumber = -1
		return breve.Abstract.dearchive( self )

	def destroy( self ):
		if self.imageData:
			breve.breveInternalFunctionFinder.imageDataFree( self, self.imageData )


	def getAlphaPixel( self, x, y ):
		'''Returns the alpha channel pixel at the image coordinates (x, y). The pixel value is given on a scale from 0.0 to 1.0.'''

		if ( not self.imageData ):
			return 0

		return breve.breveInternalFunctionFinder.imageGetValueAtCoordinates( self, self.imageData, ( ( x * 4 ) + 3 ), y )

	def getBluePixel( self, x, y ):
		'''Returns the blue pixel at the image coordinates (x, y). The pixel value is given on a scale from 0.0 to 1.0.'''

		if ( not self.imageData ):
			return 0

		return breve.breveInternalFunctionFinder.imageGetValueAtCoordinates( self, self.imageData, ( ( x * 4 ) + 2 ), y )

	def getCompressionSize( self ):
		'''Compresses the image and returns the compression size.  Useful for  generating simple complexity measures based on image compression.'''

		return breve.breveInternalFunctionFinder.imageGetCompressionSize( self, self.imageData )

	def getGreenPixel( self, x, y ):
		'''Returns the green pixel at the image coordinates (x, y). The pixel value is given on a scale from 0.0 to 1.0.'''

		if ( not self.imageData ):
			return 0

		return breve.breveInternalFunctionFinder.imageGetValueAtCoordinates( self, self.imageData, ( ( x * 4 ) + 1 ), y )

	def getHeight( self ):
		'''Returns the width of the image.  '''

		if ( not self.imageData ):
			return 0

		return breve.breveInternalFunctionFinder.imageGetHeight( self, self.imageData )

	def getImageData( self ):
		return self.imageData

	def getPixelPointer( self ):
		'''Returns a pointer to the pixels this image is holding in RGBA format.  The size of the buffer is 4 * height * width.  This  data is provided for plugin developers who wish to read or  write pixel data directly.  '''

		return breve.breveInternalFunctionFinder.imageGetPixelPointer( self, self.imageData )

	def getRedPixel( self, x, y ):
		'''Returns the red pixel at the image coordinates (x, y). The pixel value is given on a scale from 0.0 to 1.0.'''

		if ( not self.imageData ):
			return 0

		return breve.breveInternalFunctionFinder.imageGetValueAtCoordinates( self, self.imageData, ( x * 4 ), y )

	def getRgbPixel( self, x, y ):
		'''Returns the red, green and blue components of the pixel at image coordinates (x, y) as a vector.'''

		b = 0
		g = 0
		r = 0

		if ( not self.imageData ):
			return breve.vector( 0, 0, 0 )

		r = breve.breveInternalFunctionFinder.imageGetValueAtCoordinates( self, self.imageData, ( x * 4 ), y )
		g = breve.breveInternalFunctionFinder.imageGetValueAtCoordinates( self, self.imageData, ( ( x * 4 ) + 1 ), y )
		b = breve.breveInternalFunctionFinder.imageGetValueAtCoordinates( self, self.imageData, ( ( x * 4 ) + 2 ), y )
		return breve.vector( r, g, b )

	def getTextureNumber( self ):
		'''Internal use only.'''

		if ( self.imageData and ( self.textureNumber == -1 ) ):
			self.textureNumber = breve.breveInternalFunctionFinder.imageUpdateTexture( self, self.imageData )

		return self.textureNumber

	def getWidth( self ):
		'''Returns the width of the image.  '''

		if ( not self.imageData ):
			return 0

		return breve.breveInternalFunctionFinder.imageGetWidth( self, self.imageData )

	def init( self ):
		self.textureNumber = -1

	def initWith( self, imageWidth, imageHeight ):
		'''Deprecated.'''

		self.setSize( imageWidth, imageHeight )

	def iterate( self ):
		if self.modified:
			self.archiveFile = ''
			breve.breveInternalFunctionFinder.imageUpdateTexture( self, self.imageData )
			self.modified = 0



	def load( self, imageFile ):
		'''Loads an image from the file imageFile.'''

		if self.imageData:
			breve.breveInternalFunctionFinder.imageDataFree( self, self.imageData )

		self.imageData = breve.breveInternalFunctionFinder.imageLoadFromFile( self, imageFile )
		if ( not self.imageData ):
			print '''Error loading image %s!''' % (  imageFile )
			return 0


		self.archiveFile = imageFile
		if ( self.textureNumber != -1 ):
			self.modified = 1

		return self

	def readDepth( self, x, y, lin, dist ):
		'''Reads the depth into this Image from the rendered image on the  screen.  The resulting image can be written to a file or  analyzed if desired.  This is only supported in graphical versions of breve. The linearize option (if true) linearizes the depth buffer to a maximum  distance so that for each pixel, the value corresponds to the distance from  the centre of the camera. Values above the maximum distance (max-dist) are  set to the maximum distance.  If linearize is false, then this returns the raw depth buffer.  This is considerably faster, but the values are not linear in the distance. '''

		breve.breveInternalFunctionFinder.imageReadDepthBuffer( self, self.imageData, x, y, lin, dist )

	def readPixels( self, x, y ):
		'''Reads pixels into this Image from the rendered image on the  screen.  The resulting image can be written to a file or  analyzed if desired.  This is only supported in graphical versions of breve.'''

		breve.breveInternalFunctionFinder.imageReadPixels( self, self.imageData, x, y )

	def setAlphaPixel( self, alphaPixel, x, y ):
		'''Sets the alpha pixel value at coordinates (x, y) to alphaPixel. alphaPixel should be a value between 0.0 and 1.0.'''

		self.modified = 1
		breve.breveInternalFunctionFinder.imageSetValueAtCoordinates( self, self.imageData, ( ( x * 4 ) + 3 ), y, alphaPixel )

	def setBluePixel( self, bluePixel, x, y ):
		'''Sets the blue pixel value at coordinates (x, y) to bluePixel. bluePixel should be a value between 0.0 and 1.0.'''

		self.modified = 1
		breve.breveInternalFunctionFinder.imageSetValueAtCoordinates( self, self.imageData, ( ( x * 4 ) + 2 ), y, bluePixel )

	def setGreenPixel( self, greenPixel, x, y ):
		'''Sets the green pixel value at coordinates (x, y) to greenPixel. greenPixel should be a value between 0.0 and 1.0.'''

		self.modified = 1
		breve.breveInternalFunctionFinder.imageSetValueAtCoordinates( self, self.imageData, ( ( x * 4 ) + 1 ), y, greenPixel )

	def setPixel( self, pixelVector, x, y ):
		'''Deprecated -- for compatibility only.'''

		self.setRedPixel( pixelVector.x, x, y )
		self.setGreenPixel( pixelVector.y, x, y )
		self.setBluePixel( pixelVector.z, x, y )

	def setRedPixel( self, redPixel, x, y ):
		'''Sets the red pixel value at coordinates (x, y) to redPixel. redPixel should be a value between 0.0 and 1.0.'''

		self.modified = 1
		breve.breveInternalFunctionFinder.imageSetValueAtCoordinates( self, self.imageData, ( x * 4 ), y, redPixel )

	def setRgbPixel( self, pixelVector, x, y ):
		'''Sets the red, green and blue pixel values at image coordinates  (x, y) from the values in pixelVector.'''

		self.setRedPixel( pixelVector.x, x, y )
		self.setGreenPixel( pixelVector.y, x, y )
		self.setBluePixel( pixelVector.z, x, y )

	def setSize( self, imageWidth, imageHeight ):
		'''Creates an empty image buffer with width imageWidth and length imageLength.'''

		self.currentHeight = imageHeight
		self.currentWidth = imageWidth
		if self.imageData:
			breve.breveInternalFunctionFinder.imageDataFree( self, self.imageData )

		self.imageData = breve.breveInternalFunctionFinder.imageDataInit( self, imageWidth, imageHeight )
		return self

	def write( self, imageFile ):
		'''Write the image to imageFile.  The image is written as a  PNG file, so imageFile should end with .PNG.'''

		breve.breveInternalFunctionFinder.imageWriteToFile( self, self.imageData, imageFile, 4, 8 )

	def write16BitGrayscale( self, imageFile ):
		'''Write the image to imageFile.  The image is written as a  PNG file, so imageFile should end with .PNG.'''

		breve.breveInternalFunctionFinder.imageWriteToFile( self, self.imageData, imageFile, 1, 16 )


breve.Image = Image
# Add our newly created classes to the breve namespace

breve.Images = Image



