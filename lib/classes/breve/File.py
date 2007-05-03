
import breve

class File( breve.Abstract ):
	'''A File is an object used for creating textual output of your breve  simulation, or for reading in data from text files.   <p> Though some File methods can be used to encode or decode simulation objects, it is not the preferred method for archiving and dearchiving  objects--for a discussion of archiving and dearchiving objects, see the  <a href="http://www.spiderland.org/breve/docs/docs/archiving.html">Archiving and Dearchiving section</a> of the breve documentation.'''

	def __init__( self ):
		breve.Abstract.__init__( self )
		self.filePointer = None

	def close( self ):
		'''Closes the file.  This should be done when you're finished reading from or writing to a file.'''


		if self.filePointer:
			breve.breveInternalFunctionFinder.closeFile( self, self.filePointer )

		self.filePointer = 0

	def destroy( self ):
		''''''


		if self.filePointer:
			self.close()


	def isEndOfFile( self ):
		'''Returns whether the file pointer is at the end of the file.'''


		return breve.breveInternalFunctionFinder.fileEOF( self, self.filePointer )

	def openForAppending( self, fileName ):
		'''Opens fileName for appending.  If the file already exists, future writes will append text to the end of the file instead of overwriting it. If the file does not exist, it is created. <p> To be used with the methods METHOD(write), METHOD(write-line) and  METHOD(write-data) below.'''


		self.filePointer = breve.breveInternalFunctionFinder.openFileForAppending( self, fileName )
		if ( not self.filePointer ):
			return 0

		return self

	def openForReading( self, fileName ):
		'''Opens fileName for reading.  To be used with the methods  METHOD(read-as-string), METHOD(read-as-data) and METHOD(read-line) below.'''


		self.filePointer = breve.breveInternalFunctionFinder.openFileForReading( self, fileName )
		if ( not self.filePointer ):
			return 0

		return self

	def openForWriting( self, fileName ):
		'''Opens fileName for writing.  If the file already exists, it is truncated to length zero so that this method effectively overwrites files. If the file does not exist, it is created. <p> To be used with the methods METHOD(write), METHOD(write-line) and  METHOD(write-data) below.'''


		self.filePointer = breve.breveInternalFunctionFinder.openFileForWriting( self, fileName )
		if ( not self.filePointer ):
			return 0

		return self

	def readAsData( self ):
		'''Reads the entire file as binary data and returns a "data" type (not to be confused with a OBJECT(Data) object).'''


		return breve.breveInternalFunctionFinder.readFileAsData( self, self.filePointer )

	def readAsString( self ):
		'''Reads the entire file and returns it as a string.'''


		return breve.breveInternalFunctionFinder.readFileAsString( self, self.filePointer )

	def readLine( self ):
		'''Reads a single line of text from the file (ending with a newline). The size of the line read is limited to 10239.'''


		return breve.breveInternalFunctionFinder.readLine( self, self.filePointer )

	def readLineAsList( self, delimiterString ):
		'''Reads a line from the file and returns the results as a list of elements delimited by delimiterString. The size of the line read is limited to 10239.'''


		return breve.breveInternalFunctionFinder.readDelimitedList( self, self.filePointer, delimiterString )

	def readLineAsWhitespaceDelimitedList( self ):
		'''Reads a line from the file and returns the results as a list of elements delimited by whitespace. The size of the line read is limited to 10239.'''


		return breve.breveInternalFunctionFinder.readWhitespaceDelimitedList( self, self.filePointer )

	def write( self, theText ):
		'''Writes the string theText to the file.  This method does not write a newline character to the end of the string.  See METHOD(write-line) for writing a string with a newline character.'''


		breve.breveInternalFunctionFinder.writeString( self, self.filePointer, theText )

	def writeLine( self, theText ):
		'''Writes the string theText to the file, with a newline character at  the end.  Like METHOD(write), but includes a newline character.'''


		breve.breveInternalFunctionFinder.writeString( self, self.filePointer, '''%s
''' % (  theText ) )


breve.File = File
breve.Files = File



