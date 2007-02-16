
import breveInternal
import sys, os, math, random

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

def setController( inControllerClass ):
	"Creates the breve Controller class.  This method simply instantiates the class which is called.  It is provided for convenience only."
	return controller()

def addInstance( inclass, ininstance ):
	return breveInternal.addInstance( breveInternal, inclass, ininstance )

def createInstances( inClass, inCount ):
	"Creates one or more instances of a breve class"

	if inCount == 1:
		return inClass()

	instances = []

	for i in range( inCount ):
		instances.append( inClass() )

	return instances

class breveInternalFunctions:
	def __init__( self ):
		self.functionCache = {}

	def __getattr__( self, method ):
		if self.functionCache.has_key( method ):
			return self.functionCache[ method ]

		internalFunction = breveInternal.findInternalFunction( breveInternal.breveEngine, method )

		if internalFunction:
			def execute( object, *tuple ):
				return breveInternal.callInternalFunction( breveInternal, object, internalFunction, tuple )

			self.functionCache[ method ] = execute

			return execute



def length( inValue ):
	"Returns the length or size of expression inValue. Provided for convenience, and for compatibility with simulations converted from steve."

	if not hasattr( inValue, '__class__' ):
		raise Exception( "Cannot give length of expression" )

	if inValue.__class__ == int or inValue.__class__ == float:
		return abs( inValue )

	if inValue.__class__ == vector:
		return inValue.length()

	if inValue.__class__ == list:
		return len( inValue )


def randomExpression( inValue ):
	"Returns a random expression with a maximum of inValue, an int, float or vector.  Provided for convenience, and for compatibility with simulations converted from steve."

	if not hasattr( inValue, '__class__' ):
		raise Exception( "Cannot create random expression" )

	if inValue.__class__ == int:
		return random.randint( 0, abs( inValue ) )

	if inValue.__class__ == float:
		return random.uniform( 0, abs( inValue ) )

	if inValue.__class__ == vector:
		return vector( random.uniform( 0, abs( inValue.x ) ), random.uniform( 0, abs( inValue.z ) ), random.uniform( 0, abs( inValue.z ) ) )


def isSubclass( inA, inB ):
	if inA == inB:
		return 1

	for c in inA.__bases__:
	 	return isSubclass( c, inB )

	return 0


class matrix:
	"A 3x3 matrix class used for breve"

	__slots__ = ( 'x1', 'x2', 'x3', 'y1', 'y2', 'y3', 'z1', 'z2', 'z3' )

	def __init__( self, x1 = 1.0, x2 = 0.0, x3 = 0.0, y1 = 0.0, y2 = 1.0, y3 = 0.0, z1 = 0.0, z2 = 0.0, z3 = 1.0 ):
		self.x1 = float( x1 )
		self.x2 = float( x2 )
		self.x3 = float( x3 )
		self.y1 = float( y1 )
		self.y2 = float( y2 )
		self.y3 = float( y3 )
		self.z1 = float( z1 )
		self.z2 = float( z2 )
		self.z3 = float( z3 )
		self.isMatrix = 1

	def __str__( self ):
		return 'breve.matrix( %f, %f, %f, %f, %f, %f, %f, %f, %f )' % ( self.x1, self.x2, self.x3, self.y1, self.y2, self.y3, self.z1, self.z2, self.z3 )

class vector:
	"A 3D vector class used for breve"
	__slots__ = ( 'x', 'y', 'z' )

	def __init__( self, x = 0.0, y = 0.0, z = 0.0 ):
		self.x = float( x )
		self.y = float( y )
		self.z = float( z )
		self.isVector = 1

	def __len__( self ):
		return int( math.sqrt( self.x * self.x + self.y * self.y + self.z * self.z ) )

	def length( self ):
		return math.sqrt( self.x * self.x + self.y * self.y + self.z * self.z )

	def __str__( self ):
		return 'breve.vector( %f, %f, %f )' % ( self.x, self.y, self.z )

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
		if hasattr( other, '__class__' ) and other.__class__ == matrix:
			return vector( 303, 404, 505 )

		return vector( self.x * other, self.y * other, self.z * other )
	
	__rmul__ = __mul__

	def __neg__( self ):
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



breveInternalFunctionFinder = breveInternalFunctions()

breveInternal.bridgeObject = bridgeObject
breveInternal.vectorType = vector
breveInternal.matrixType = matrix

__all__ = [ 'Object', 'Abstract', 'Control' ]

from Object	import *

from Abstract	import *
from Real	import *

from Stationary	import *
from Control	import *
from Camera	import *
from MenuItem	import *
from Shape	import *
from Mobile	import *
from Image	import *
