
import breveInternal
import breve
import sys
import os

#
# Used internally to redirect Python output to the breve frontend
#

class breveStderrHandler:
    def write( self, line ):
        breveInternal.catchOutput( breveInternal, line )

class breveStdoutHandler:
    def write( self, line ):
        breveInternal.catchOutput( breveInternal, line )

sys.stderr = breveStderrHandler()
sys.stdout = breveStdoutHandler()



#
# Sets the breve controller class
#

def setController( controller ):
	controller()



class breveInternalFunctions:
	def __init__( self ):
		self.functionCache = {}

	def __getattr__( self, method ):
		# if self.functionCache.has_key( method ):
		#	return self.functionCache[ method ]

		internalFunction = breveInternal.findInternalFunction( breveInternal.breveEngine, method )

		if internalFunction:
			def execute( object, *tuple ):
				return breveInternal.callInternalFunction( breveInternal, object, internalFunction, tuple )

			# self.functionCache[ method ] = execute

			return execute


class vector:
	__slots__ = ( 'x', 'y', 'z' )

	def __init__( self, x = 0.0, y = 0.0, z = 0.0 ):
		self.x = x
		self.y = y
		self.z = z
		self.isVector = 1

	def __str__( self ):
		return '(%f, %f, %f)' % ( self.x, self.y, self.z )

	def __eq__( self, o ):
		return self.x == o.x and self.y == o.y and self.z == o.z

	def __cmp__( self, o ):
		return self.__eq__( o )

	def __sub__( self, other ):
		return vector( self.x - other.x, self.y - other.y, self.z - other.z )

	__rsub__ = __sub__

	def __add__( self, other ):
		return vector( self.x + other.x, self.y + other.y, self.z + other.z )

	__radd__ = __add__

	def __div__( self, other ):
		return vector( self.x / other, self.y / other, self.z / other )

	def __mul__( self, other ):
		return vector( self.x * other, self.y * other, self.z * other )
	
	__rmul__ = __mul__

	def __neg__( self, other ):
		return vector( -self.x, -self.y, -self.z )


class bridgeObjectMethod:
	"A callable object used to implement breve bridge method calling"

	def __init__( self, caller, method ):
		self.caller = caller 
		self.method = method

	def __call__( self, *tuple ):
		"Executes this object's internal bridge method"
		return breveInternal.callBridgeMethod( breveInternal, self.caller, self.method, tuple )


class bridgeObject:
	"An object representing a bridge to an object in another breve language frontend"

	def __getattr__( self, method ):
		internalMethod = breveInternal.findBridgeMethod( self, method )

		if internalMethod:
			return bridgeObjectMethod( self, internalMethod )

		raise AttributeError( "Cannot locate attribute '%s'" % method )


class object:
	"The root breve object"

	def __init__( self ):
		if not ( 'breveInstance' in self.__dict__ ):
			self.breveInstance = breveInternal.addInstance( breveInternal, self.__class__, self )

		self.controller = breveInternalFunctionFinder.getController( self )
		self.breveModule = breveInternal

		if self.controller == None:
			print "No controller has been defined!"
			raise ValueError

class control( object ):
	"A class for implementing the breve controller object"

	def __init__( self ):
		self.breveInstance = breveInternal.addInstance( breveInternal, self.__class__, self )

		breveInternal.setController( breveInternal, self )

		object.__init__( self )

		self.setBackgroundColor( vector( .5, .7, 1.0 ) )

	def iterate( self ):
		breveInternalFunctionFinder.worldStep( self, 0.1, 0.1 )

	def getRealTime( self ):
		"Returns the number of seconds since January 1st, 1970 with microsecond precision."

		return breveInternalFunctionFinder.getRealTime( self )

	def getTime( self ):
		"Returns the simulation time of the world."

		return breveInternalFunctionFinder.getTime( self )

	def endSimulation( self ):
		"Ends the current simulation"

		return breveInternalFunctionFinder.endSimulation( self )

	def setBackgroundColor( self, vector ):
		"Sets the background color of the simulation"

		return breveInternalFunctionFinder.setBackgroundColor( self, vector )



class real( object ):
	"A breve class which has a physical presense in the simulated breve world"

	def __init__( self ):
		object.__init__( self )

		self.linkPointer = breveInternalFunctionFinder.linkNew( self )
		breveInternalFunctionFinder.linkAddToWorld( self, self.linkPointer )

		self.setShape( sphere() )

	def setShape( self, shape ):

		breveInternalFunctionFinder.linkSetShape( self, self.linkPointer, shape.shapePointer )


	def move( self, position ):
		pass		




class shape( object ):
	def __init__( self ):
		object.__init__( self )
		print self.breveInstance




class sphere( shape ):
	def __init__( self ):
		shape.__init__( self )
		print self.breveInstance

		self.shapePointer = breveInternalFunctionFinder.newSphere( self, 1.0, 1.0 )


breveInternalFunctionFinder = breveInternalFunctions()
breveInternal.bridgeObject = bridgeObject
breveInternal.vectorType = vector
