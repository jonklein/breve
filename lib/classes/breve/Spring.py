
# Note: this file was automatically converted to Python from the
# original steve-language source code.  Please see the original
# file for more detailed comments and documentation.


import breve

class Spring( breve.Object ):
	'''A Spring is used to create an elastic connection between two OBJECT(Real)  objects.  Springs define only a connection between objects and are not  physical objects themselves.  Thus, <b>Springs can pass through each other  without colliding</b>.   <P> A number of paramters can be specified for springs.  The length parameter specifies the length of the spring when it is completely relaxed.  The  spring will always apply a force to attempt to expand or contract to the	 specified length.  The strength parameter specifies the amount of force that is applied to try to attain the spring's natural length.  Finally, the damping parameter specifies the friction or damping proportional to the spring's velocity. <P> If physical realism is important, springs should be used with caution. As the strength and damping constants of the spring increase, springs  can generate enormous forces that will cause numerical overflows or  other undesirable behavior.  Keep spring strength and damping constants as low as possible to avoid this behavior. <P> The Spring class is new as of version 2.1.'''

	def __init__( self ):
		breve.Object.__init__( self )
		self.damping = 0
		self.end = None
		self.length = 0
		self.maxForce = 0
		self.mode = 0
		self.springPointer = None
		self.start = None
		self.strength = 0

	def connect( self, springLink1, springLink2, point1 = breve.vector( 0.000000, 0.000000, 0.000000 ), point2 = breve.vector( 0.000000, 0.000000, 0.000000 ), springLength = 0.000000, springStrength = 100.000000, springDamping = 0.200000 ):
		'''Connects the spring from springLink1 to springLink2.   <p> The optional arguments point1 and point2 specify the locations on the objects to which the springs are attached.  The default values are (0, 0, 0). <p> If the length arugment is omitted, length defaults to 0.'''

		if ( springLink1 == springLink2 ):
			return

		self.start = springLink1
		self.end = springLink2
		self.length = springLength
		self.strength = springStrength
		self.damping = springDamping
		self.springPointer = breve.breveInternalFunctionFinder.springNew( self, self.start.getLinkPointer(), self.end.getLinkPointer(), point1, point2, self.length, self.strength, self.damping )
		self.addDependency( self.start )
		self.addDependency( self.end )
		return self.springPointer

	def destroy( self ):
		if self.springPointer:
			breve.breveInternalFunctionFinder.springRemove( self, self.springPointer )


	def getCurrentLength( self ):
		'''This method returns the <i>current</i> (stretched or compressed)  length of the spring, as opposed to the "natural" length.'''

		if self.springPointer:
			return breve.breveInternalFunctionFinder.springGetCurrentLength( self, self.springPointer )

		return 0

	def getForce( self ):
		'''Returns the amount of force applied by this spring at the last  timestep.  The returned value is a double representing the  magnitude of the force.  The direction of the force is determined by the locations of the links to which the spring is attached.'''

		if self.springPointer:
			return breve.breveInternalFunctionFinder.springGetForce( self, self.springPointer )

		return 0

	def getLength( self ):
		'''This method returns the natural length of the spring.  See also METHOD(get-current-length) which returns the current (stretched or compressed) length of the spring.'''

		return self.length

	def getLinks( self ):
		'''Returns a list with two items, the links which are joined by this spring.'''

		return [ self.start, self.end ]

	def setContractOnly( self ):
		'''Setting a spring as "contract only" means that the spring will contract if it is expanded larger than its natural length, but will not expand  if it is pushed to smaller lengths.  This makes the spring behavior  more like a rope.  See also METHOD(set-expand-only) and  METHOD(set-expand-and-contract).'''

		self.mode = 2
		if self.springPointer:
			breve.breveInternalFunctionFinder.springSetMode( self, self.springPointer, 2 )


	def setDamping( self, newDamping ):
		'''Sets the spring's damping to newDamping.'''

		self.damping = newDamping
		if self.springPointer:
			breve.breveInternalFunctionFinder.springSetDamping( self, self.springPointer, newDamping )


	def setExpandAndContract( self ):
		'''This sets a spring to apply forces to expand when the spring is pushed smaller than the natural length, and contract when the spring is pulled to be larger than the natural length.  This is the default behavior of  a spring.'''

		self.mode = 0
		if self.springPointer:
			breve.breveInternalFunctionFinder.springSetMode( self, self.springPointer, 0 )


	def setExpandOnly( self ):
		'''Setting a spring as "expand only" means that the spring will expand if it is pushed smaller than its natural length, but will not contract if it is stretched to be larger.'''

		self.mode = 1
		if self.springPointer:
			breve.breveInternalFunctionFinder.springSetMode( self, self.springPointer, 1 )


	def setLength( self, newLength ):
		'''Sets the spring's length to newLength.'''

		self.length = newLength
		if self.springPointer:
			breve.breveInternalFunctionFinder.springSetLength( self, self.springPointer, newLength )


	def setMaximumForce( self, newMax ):
		'''Sets an optional maximum force that the spring can apply.  This can  be used as a safegaurd to prevent unrealistically large forces from causing instability in simulations.  <p> If the maximum force value is set to 0, which is the default, then the springs forces are unlimited.'''

		self.maxForce = newMax
		if self.springPointer:
			breve.breveInternalFunctionFinder.springSetMaxForce( self, self.springPointer, newMax )


	def setPattern( self, patternString ):
		'''Sets the drawing style of this spring.  patternString is a string of  16 spaces and/or dashes which specify the line pattern to be drawn. A thickly dashed spring, for example, would use the pattern  "--------        ".  The string " " can be used to make the spring invisible.  If no style is given, a dotted line is drawn.'''

		breve.breveInternalFunctionFinder.objectLineSetStipple( self, self.springPointer, patternString )

	def setStrength( self, newStrength ):
		'''Sets the spring's strength to newStrength.'''

		self.strength = newStrength
		if self.springPointer:
			breve.breveInternalFunctionFinder.springSetStrength( self, self.springPointer, newStrength )



breve.Spring = Spring
# Add our newly created classes to the breve namespace

breve.Springs = Spring



