
import breveInternal
import sys, os, math, random

def clear(keep=("__builtins__", "clear")):
	keeps = {}
	
	for name, value in globals().iteritems():
		if name in keep: keeps[name] = value

		globals().clear()

	for name, value in keeps.iteritems():
		globals()[name] = value



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


def allInstances( inclass ):
	cls = globals()[ inclass ]

	if not instanceDict.has_key( cls ):
		instanceDict[ cls ] = objectList()

	return instanceDict[ cls ]

def addInstance( inclass, ininstance ):
	if not instanceDict.has_key( inclass ):
		instanceDict[ inclass ] = objectList()

	instanceDict[ inclass ].append( ininstance )

	return breveInternal.addInstance( breveInternal, inclass, ininstance )

class objectList( list ):
	def __init__( self ):
		list.__init__( self )

	def __setitem__( self, index, value ):
		if index == len( self ):
			self.append( value )
			return

		list.__setitem__( self, index, value )

	def __getattr__( self, methodName ):
		def execute( *args ):
			for i in self:
				i.__getattribute__( methodName )( *args )

		return execute


def createInstances( inClass, inCount ):
	"Creates one or more instances of a breve class"

	if type( inClass ) == str:
		inClass = breve.__dict__[ inClass ]

	if inCount == 1:
		return inClass()

	instances = objectList()

	for i in range( inCount ):
		instances.append( inClass() )

	return instances

def deleteInstances( inInstances ):
	"Delete one or more instances of a breve class"

	# print "Deleting %s" % inInstances

	if issubclass( inInstances.__class__, list ):
		for i in inInstances:
			i.destroy()
			i.delete()
			breveInternal.removeInstance( breveInternal.breveEngine, i )
			all = allInstances( i.__class__.__name__ )
			if i in all:
				all.remove( i )
			del i

		return

	inInstances.destroy()
	inInstances.delete()
	breveInternal.removeInstance( breveInternal.breveEngine, inInstances )
	all = allInstances( inInstances.__class__.__name__ )
	if inInstances in all:
		all.remove( inInstances )

	del inInstances

def executeCachedMethod( obj, method ):
	if not obj.__dict__.has_key( method ):
		obj.__dict__[ method ] = obj.__getattribute__( method.__name__ )

	obj.__dict__[ method ]()

breveInternal.executeCachedMethod = executeCachedMethod

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




#
# Utility functions for easy steve conversion
#

def length( inValue ):
	"Returns the length or size of expression inValue. Provided for convenience, and for compatibility with simulations converted from steve."

	if not hasattr( inValue, '__class__' ):
		raise Exception( "Cannot give length of expression" )

	if inValue.__class__ == vector:
		return inValue.length()

	if inValue.__class__ == int or inValue.__class__ == float:
		return abs( inValue )

	if issubclass( inValue.__class__, list ):
		return len( inValue )

	return 1


def randomExpression( inValue ):
	"Returns a random expression with a maximum of inValue, an int, float or vector.  Provided for convenience, and for compatibility with simulations converted from steve."

	if not hasattr( inValue, '__class__' ):
		raise Exception( "Cannot create random expression" )

	if inValue.__class__ == vector:
		return vector( random.uniform( 0, abs( inValue.x ) ), random.uniform( 0, abs( inValue.z ) ), random.uniform( 0, abs( inValue.z ) ) )

	if inValue.__class__ == int:
		return random.randint( 0, abs( inValue ) )

	if inValue.__class__ == float:
		return random.uniform( 0, abs( inValue ) )


#
# Standard breve matrices and vectors
#

class matrix( object ):
	"A 3x3 matrix class used for breve"

	__slots__ = ( 'x1', 'x2', 'x3', 'y1', 'y2', 'y3', 'z1', 'z2', 'z3', 'isMatrix' )

	def __init__( self, x1 = 1.0, x2 = 0.0, x3 = 0.0, y1 = 0.0, y2 = 1.0, y3 = 0.0, z1 = 0.0, z2 = 0.0, z3 = 1.0 ):
		self.x1 = x1
		self.x2 = x2
		self.x3 = x3
		self.y1 = y1
		self.y2 = y2
		self.y3 = y3
		self.z1 = z1
		self.z2 = z2
		self.z3 = z3
		self.isMatrix = 1

	def __str__( self ):
		return 'breve.matrix( %f, %f, %f, %f, %f, %f, %f, %f, %f )' % ( self.x1, self.x2, self.x3, self.y1, self.y2, self.y3, self.z1, self.z2, self.z3 )

class vector( object ):
	"A 3D vector class used for breve"
	__slots__ = ( 'x', 'y', 'z', 'isVector' )


	def __init__( self, x = 0.0, y = 0.0, z = 0.0 ):
		self.x = x
		self.y = y
		self.z = z
		self.isVector = 1

	def __str__( self ):
		return 'breve.vector( %f, %f, %f )' % ( self.x, self.y, self.z )

	def __eq__( self, o ):
		return self.x == o.x and self.y == o.y and self.z == o.z

	def __cmp__( self, o ):
		return self.__eq__( o )

	def __ne__( self, o ):
		return not self.__eq__( o )

	def __sub__( self, other ):
		return vector( self.x - other.x, self.y - other.y, self.z - other.z )

	def __add__( self, other ):
		return vector( self.x + other.x, self.y + other.y, self.z + other.z )

	def __div__( self, other ):
		return vector( self.x / other, self.y / other, self.z / other )

	def __mul__( self, other ):
		if hasattr( other, '__class__' ) and other.__class__ == matrix:
			x = self.x * self.x1 + self.y * self.x2 + self.z * self.x3
			y = self.x * self.y1 + self.y * self.y2 + self.z * self.y3
			z = self.x * self.z1 + self.y * self.z2 + self.z * self.z3

			return vector( x, y, z )

		return vector( self.x * other, self.y * other, self.z * other )
	
	__rmul__ = __mul__

	def __neg__( self ):
		return vector( -self.x, -self.y, -self.z )

	def length( self ):
		"Gives the length of the vector"
		return math.sqrt( self.x * self.x + self.y * self.y + self.z * self.z )

	def scale( self, other ):
		"Scales the vector by a scalar"
		self.x *= other
		self.y *= other
		self.z *= other

	def normalize( self ):
		"Normalizes the vector"
		len = math.sqrt( self.x * self.x + self.y * self.y + self.z * self.z )

		if len != 0.0:
			self.x /= len
			self.y /= len
			self.z /= len


#
# Import all of the standard breve classes
#
# Ordered to respect inter-object dependencies
#

from Object 		import *

from Abstract 		import *
from Real 		import *

from Mobile 		import *
from Stationary 	import *

from Link 		import *

from Camera 		import *
from Control 		import *
from PhysicalControl 	import *
from Data 		import *
from MultiBody 		import *

from GeneticAlgorithm 	import *

from PatchGrid 		import *
from PatchToroid 	import *
from Patch 		import *

from Braitenberg 	import *
from DirectedGraph 	import *
from Drawing 		import *
from File 		import *
from Genome 		import *
from Graph 		import *
from IRSensor 		import *
from Image 		import *
from Joint 		import *
from Line 		import *
from Matrix 		import *
from MenuItem 		import *
from Movie 		import *
from NetworkServer 	import *
from NeuralNetwork 	import *
from Push 		import *
from PushGP 		import *
from SFFNetwork 	import *
from Shape 		import *
from Sound 		import *
from Spring 		import *
from Terrain 		import *
from Tone 		import *
from TurtleDrawing 	import *
from Vector 		import *
from Wanderer 		import *





class bridgeObjectMethod( Object ):
	"A callable object used to implement breve bridge method calling"
	
	def __init__( self, caller, method ):
		self.caller = caller 
		self.method = method

	def __call__( self, *tuple ):
		"Executes this object's internal bridge method"
		return breveInternal.callBridgeMethod( breveInternal, self.caller, self.method, tuple )


class bridgeObject( object ):
	"An object representing a bridge to an object in another breve language frontend"

	__slots__ = [ 'functionCache', 'breveModule', 'breveInstance' ]

	def __init__( self ): 
		self.breveInstance = addInstance( self.__class__, self )
		self.breveModule = breveInternal

		self.functionCache = {}

	def __getattr__( self, method ):
		if not self.functionCache.has_key( method ):
			internalMethod = breveInternal.findBridgeMethod( self, method )

			if internalMethod != None:
				self.functionCache[ method ] = bridgeObjectMethod( self, internalMethod )

		if self.functionCache.has_key( method ):
			return self.functionCache[ method ]
		else:
			raise AttributeError( "Cannot locate method '%s'" % method )


breveInternalFunctionFinder = breveInternalFunctions()

breveInternal.bridgeObject = bridgeObject
breveInternal.vectorType = vector
breveInternal.matrixType = matrix

__all__ = [ 'Object', 'Abstract', 'Control' ]

instanceDict = {}
