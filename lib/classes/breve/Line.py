
# Note: this file was automatically converted to Python from the
# original steve-language source code.  Please see the original
# file for more detailed comments and documentation.


import breve

class Line( breve.Object ):
	def __init__( self ):
		breve.Object.__init__( self )
		self.end = None
		self.linePointer = None
		self.start = None

	def connect( self, start, end, theColor = breve.vector( 0.000000, 0.000000, 0.000000 ), theStyle = "----------------" ):
		'''Adds a line to be drawn between Real objects start and end.   <P> The optional argument theColor specifies the color of the line.  The default color is the vector (0, 0, 0), black. <P> The optional argument theStyle specifies a pattern for the line.  theStyle is a string of 16 spaces and/or dashes which specify the line pattern to be drawn. A dotted line, for example, would use the pattern "- - - - - - - - ".  A thickly dashed line would use the pattern "--------        ".  If no style is given, a solid line is drawn.'''

		if ( start == end ):
			return

		self.linePointer = breve.breveInternalFunctionFinder.addObjectLine( self, start.getWorldObjectPointer(), end.getWorldObjectPointer(), theColor, theStyle )

	def destroy( self ):
		if self.linePointer:
			breve.breveInternalFunctionFinder.removeObjectLine( self, self.linePointer )


	def isLinked( self, worldObject ):
		'''Returns 1 if this line is associated with worldObject.'''

		if ( ( self.start == worldObject ) or ( self.end == worldObject ) ):
			return 1

		return 0

	def setColor( self, color ):
		'''If the line is connected, sets the color of the line.'''

		if self.linePointer:
			breve.breveInternalFunctionFinder.objectLineSetColor( self, self.linePointer, color )


	def setTransparency( self, transparency ):
		'''If the line is connected, sets the transparency of the line.'''

		if self.linePointer:
			breve.breveInternalFunctionFinder.objectLineSetTransparency( self, self.linePointer, transparency )



breve.Line = Line



