
# Note: this file was automatically converted to Python from the
# original steve-language source code.  Please see the original
# file for more detailed comments and documentation.


import breve

class TurtleDrawing( breve.Drawing ):
	'''The TurtleDrawing class is a subclass of OBJECT(Drawing) which allows  drawing via a series of "logo turtle"-like commands.  Drawing is  accomplished by giving commands to a turtle moving around in 3D space. All commands are interpreted from the turtle's perspective, taking into account its current position and orientation. <P> Unlike other angles used by breve, this class refers to angles as degrees and not radians to more closely match the behavior of traditional Logo  implementations.'''

	def __init__( self ):
		breve.Drawing.__init__( self )
		self.currentColor = breve.vector()
		self.currentDirection = breve.vector()
		self.penDown = 0
		self.states = breve.objectList()
		self.turtle = None
		TurtleDrawing.init( self )

	def back( self, length ):
		'''Moves the turtle back by length.'''

		self.forward( ( -length ) )

	def destroy( self ):
		breve.deleteInstances( self.turtle.getShape() )
		breve.deleteInstances( self.turtle )

	def forward( self, length ):
		'''Moves the turtle forward by length.'''

		newPoint = breve.vector()

		newPoint = ( self.currentPoint + ( length * self.currentDirection ) )
		self.drawLine( self.currentPoint, newPoint )
		self.turtle.move( newPoint )
		self.turtle.point( breve.vector( 0, 1, 0 ), self.currentDirection )
		self.currentPoint = newPoint

	def getTurtle( self ):
		'''Returns the turtle agent object (an instance of OBJECT(Mobile)).'''

		return self.turtle

	def hideTurtle( self ):
		'''Hides the agent representing the turtle's current position and location.  The turtle can be shown again using the method METHOD(show-turtle).'''

		self.turtle.makeInvisible()

	def init( self ):
		self.penDown = 1
		self.currentDirection = breve.vector( 0, 1, 0 )
		self.currentPoint = breve.vector( 0, 0, 0 )
		self.turtle = breve.createInstances( breve.Mobile, 1 ).setShape( breve.createInstances( breve.PolygonCone, 1 ).initWith( 5, 0.800000, 0.200000 ) )
		self.turtle.setColor( breve.vector( 0.500000, 0.500000, 0.500000 ) )

	def penDown( self ):
		'''Puts the pen down so that lines are drawn as the turtle moves.  This is done by setting  transparency to 1.0.  '''

		self.penDown = 1
		self.setColor( self.currentColor, 1.000000 )

	def penUp( self ):
		'''Lifts the pen so that no lines are drawn as the turtle moves.  This is done by setting  transparency to 0.0.  '''

		self.penDown = 0
		self.setColor( self.currentColor, 0.000000 )

	def popState( self ):
		'''"Pops" a state from the state stack and makes it current.  This method reenables the most recently "pushed" state (done with METHOD(push-state)).'''

		state = breve.objectList()

		if ( breve.length( self.states ) < 1 ):
			return

		state = self.states.pop(  )
		self.currentPoint = state[ 0 ]
		self.currentDirection = state[ 1 ]
		self.currentColor = state[ 2 ]
		self.penDown = state[ 4 ]
		self.setColor( self.currentColor )

	def pushState( self ):
		'''"Pushes" the current direction, color and point onto a stack so that it can be retrieved later using METHOD(pop-state).  '''

		self.states.append( [ self.currentPoint, self.currentDirection, self.currentColor, 0, self.penDown ] )

	def setColor( self, color, trans = 1.000000 ):
		'''Sets the current pen color to color, with optional transparency value trans.'''

		self.currentColor = color
		if ( not self.penDown ):
			trans = 0.000000

		breve.Drawing.setColor( self , color, trans )

	def showTurtle( self ):
		'''Shows the agent representing the turtle's current position and location.  The turtle is visible by default, but can be hidden using METHOD(hide-turtle).'''

		self.turtle.makeVisible()

	def turnDown( self, angle ):
		'''Rotates the Turtle left by angle degrees.'''

		self.turnUp( ( -angle ) )

	def turnLeft( self, angle ):
		'''Rotates the Turtle left by angle degrees.'''

		self.turnRight( ( -angle ) )

	def turnRight( self, angle ):
		'''Rotates the Turtle right by angle degrees.'''

		rotation = breve.matrix()

		angle = ( angle * ( ( 2 * 3.141592 ) / 360 ) )
		rotation = breve.matrix(  breve.breveInternalFunctionFinder.cos( self, angle ), breve.breveInternalFunctionFinder.sin( self, angle ), 0, ( -breve.breveInternalFunctionFinder.sin( self, angle ) ), breve.breveInternalFunctionFinder.cos( self, angle ), 0, 0, 0, 1 )
		self.currentDirection = ( rotation * self.currentDirection )

	def turnUp( self, angle ):
		'''Rotates the Turtle right by angle degrees.'''

		rotation = breve.matrix()

		angle = ( angle * ( ( 2 * 3.141592 ) / 360 ) )
		rotation = breve.matrix(  1, 0, 0, 0, breve.breveInternalFunctionFinder.cos( self, angle ), breve.breveInternalFunctionFinder.sin( self, angle ), 0, ( -breve.breveInternalFunctionFinder.sin( self, angle ) ), breve.breveInternalFunctionFinder.cos( self, angle ) )
		self.currentDirection = ( rotation * self.currentDirection )


breve.TurtleDrawing = TurtleDrawing



