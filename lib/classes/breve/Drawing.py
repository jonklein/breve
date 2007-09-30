
# Note: this file was automatically converted to Python from the
# original steve-language source code.  Please see the original
# file for more detailed comments and documentation.


import breve

class Drawing( breve.Abstract ):
	'''A basic class to draw custom images using polygons and lines.'''

	def __init__( self ):
		breve.Abstract.__init__( self )
		self.currentPoint = breve.vector()
		self.drawPointer = None
		Drawing.init( self )

	def addLinePoint( self, endPoint ):
		'''Adds a line segment from the last point defined (by METHOD(draw-line)  or METHOD(add-line-point) to endPoint.'''

		breve.breveInternalFunctionFinder.drawListDrawLine( self, self.drawPointer, self.currentPoint, endPoint )
		self.currentPoint = endPoint

	def addPolygonPoint( self, polygonPoint ):
		'''Adds a polygon vertex at polygonPoint.  Repeated calls to this method add  vertices to the same polygon until no new points are added, or until the  method METHOD(end-polygon) or the method METHOD(draw-line) is called. <P> In order to accurately render a polygon, all of the points on the polygon must be coplanar.  If the points do not lie on the same plane, visual  artifacts may occur.'''

		breve.breveInternalFunctionFinder.drawListDrawVertex( self, self.drawPointer, polygonPoint )

	def clear( self ):
		'''Clears the drawing by removing all of its instructions.'''

		breve.breveInternalFunctionFinder.drawListClear( self, self.drawPointer )

	def destroy( self ):
		breve.breveInternalFunctionFinder.drawListFree( self, self.drawPointer )

	def drawLine( self, startPoint, endPoint ):
		'''Adds a line segment from startPoint to endPoint.  '''

		breve.breveInternalFunctionFinder.drawListDrawLine( self, self.drawPointer, startPoint, endPoint )
		self.currentPoint = endPoint

	def endPolygon( self ):
		'''This method ends the polygon currently being drawn.  '''

		breve.breveInternalFunctionFinder.drawListEndPolygon( self, self.drawPointer )

	def getSize( self ):
		'''Returns the current number of instructions in the drawing.'''

		breve.breveInternalFunctionFinder.drawListGetCommandCount( self, self.drawPointer )

	def init( self ):
		self.drawPointer = breve.breveInternalFunctionFinder.drawListNew( self)

	def move( self, location ):
		'''Moves the entire drawing to location.  Additional points and polygon vertices added to the drawing are still interpreted as relative to  the drawing's origin.'''

		pass

	def setColor( self, newColor, transparencyValue = 1.000000 ):
		'''Changes this drawing's current color to newColor.  The optional value transparencyValue specifies the transparency level between 0.0 (fully transparent) and 1.0 (fully opaque).'''

		breve.breveInternalFunctionFinder.drawListSetColor( self, self.drawPointer, newColor, transparencyValue )

	def setInstructionLimit( self, limit ):
		'''Limits to limit the number of instructions that may be added to the  drawing.  When instructions are added beyond the instruction limit,  instructions will be removed from the beginning of the instruction list. <p> This may be useful in creating drawings which act as arbitrarily long  trails on objects.  If no limit were used, the trail would consume  more and more resources.'''

		breve.breveInternalFunctionFinder.drawListSetCommandLimit( self, self.drawPointer, limit )

	def setLineStyle( self, lineStyle ):
		'''lineStyle is a string of 16 spaces and/or dashes which specify the line  pattern to be drawn.  A dotted line, for example, would use the pattern  "- - - - - - - - ".  A thickly dashed line would use the pattern  "--------        ".  '''

		breve.breveInternalFunctionFinder.drawListSetLineStyle( self, self.drawPointer, lineStyle )

	def setLineWidth( self, lineWidth ):
		'''Specifies the width of the line to be drawn.'''

		breve.breveInternalFunctionFinder.drawListSetLineWidth( self, self.drawPointer, lineWidth )


breve.Drawing = Drawing
class Tracer( breve.Drawing ):
	'''Tracer is a subclass of drawing that can be used to trace the motion of an agent over  time.  By default, it has an instruction limit of 50000 and the color is black, but  these and other variables can be controlled using methods available in OBJECT(Drawing).'''

	def __init__( self ):
		breve.Drawing.__init__( self )
		self.agent = None
		Tracer.init( self )

	def init( self ):
		self.setInstructionLimit( 50000 )

	def iterate( self ):
		if self.agent:
			self.addLinePoint( self.agent.getLocation() )


	def trace( self, theAgent ):
		'''Sets the tracer to follow theAgent.'''

		self.clear()
		if ( not theAgent ):
			return

		self.agent = theAgent
		self.drawLine( theAgent.getLocation(), theAgent.getLocation() )


breve.Tracer = Tracer



