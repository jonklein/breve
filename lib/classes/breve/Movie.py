
import breve

class Movie( breve.Abstract ):
	'''Records MPEG movies of breve runs. <P> The dimensions of the movie are determined by the size of the simulation viewing area when the movie export begins.  Resizing the viewing area  while the movie is exporting will produce undesirable results. <P> The Movie class does not work when using the non-graphical  ("breve_cli") breve.'''

	def __init__( self ):
		breve.Abstract.__init__( self )
		self.moviePointer = None

	def addFrameFromDisplay( self ):
		'''Add a frame from the current simulation display.'''

		if self.moviePointer:
			breve.breveInternalFunctionFinder.movieAddWorldFrame( self, self.moviePointer )


	def close( self ):
		'''Closes the MPEG file and stops recording.'''

		if self.moviePointer:
			breve.breveInternalFunctionFinder.movieClose( self, self.moviePointer )

		self.moviePointer = 0

	def destroy( self ):
		''''''

		self.close()

	def iterate( self ):
		''''''

		self.addFrameFromDisplay()

	def record( self, filename ):
		'''Create a new MPEG movie file with the name filename. New frames will be automatically added to the movie as the simulation runs until the object is released or METHOD(close) is called.  filename should end with ".mpg" or ".mpeg".'''

		self.moviePointer = breve.breveInternalFunctionFinder.movieCreate( self, filename )


breve.Movie = Movie


