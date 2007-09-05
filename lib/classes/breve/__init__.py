
import breveInternal
import sys, os, math, random, array

import pickle

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

def encodeToString( inObject ):
	return pickle.dumps( inObject )

def decodeFromString( inString ):
	pass


def setController( inControllerClass ):
	"Creates the breve Controller class.  This method simply instantiates the class which is called.  It is provided for convenience only."
	return controller()

def allInstances( inclass ):
	cls = globals()[ inclass ]

	try:
		return instanceDict[ cls ]
	except KeyError:
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
			[ i.__getattribute__( methodName )( *args ) for i in self ]

		return execute


def createInstances( inClass, inCount ):
	"Creates one or more instances of a breve class"

	if type( inClass ) == str:
		inClass = breve.__dict__[ inClass ]

	if inCount == 1:
		return inClass()

	instances = objectList()

	for i in range( int( inCount ) ):
		instances.append( inClass() )

	return instances

def deleteInstances( inInstances ):
	"Delete one or more instances of a breve class"

	if issubclass( inInstances.__class__, list ):
		for i in inInstances:
			deleteInstance( i )
	else:
		deleteInstance( inInstances )

def deleteInstance( inInstance ):
	if inInstance.breveInstance != None:
		inInstance.destroy()
		inInstance.delete()
		breveInternal.removeInstance( breveInternal.breveEngine, inInstance )
		all = allInstances( inInstance.__class__.__name__ )
		if inInstance in all:
			all.remove( inInstance )

		inInstance.breveInstance = None;
		del inInstance

class internalFunction:
	def __init__( self, function ):
		self.function = function

	def __call__( self, *args ):
		return breveInternal.callInternalFunction( self.function, args )

class breveInternalFunctions:
	def __getattr__( self, method ):
		f = breveInternal.findInternalFunction( breveInternal.breveEngine, method )

		if internalFunction:
			self.__dict__[ method ] = internalFunction( f )
			return self.__dict__[ method ]

		raise AttributeError



#
# Utility functions for easy steve conversion
#

def length( inValue ):
	"Returns the length or size of expression inValue. Provided for convenience, and for compatibility with simulations converted from steve."

	if inValue.__class__ == vector:
		return inValue.length()

	if inValue.__class__ == int or inValue.__class__ == float:
		return abs( inValue )

	if issubclass( inValue.__class__, list ):
		return len( inValue )

	return 1


def randomExpression( inValue ):
	"Returns a random expression with a maximum of inValue, an int, float or vector.  Provided for convenience, and for compatibility with simulations converted from steve."

	if inValue.__class__ == vector:
		return vector( random.uniform( 0, abs( inValue[ 0 ] ) ), random.uniform( 0, abs( inValue[ 1 ] ) ), random.uniform( 0, abs( inValue[ 2 ] ) ) )

	if inValue.__class__ == int:
		return random.randint( 0, abs( inValue ) )

	if inValue.__class__ == float:
		return random.uniform( 0, abs( inValue ) )

	raise Exception( "Cannot create random expression" )

#
# Standard breve matrices and vectors
#

class matrix( list ):
	"A 3x3 matrix class used for breve"

	def __init__( self, x1 = 1.0, x2 = 0.0, x3 = 0.0, y1 = 0.0, y2 = 1.0, y3 = 0.0, z1 = 0.0, z2 = 0.0, z3 = 1.0 ):
		list.__init__( self )
		self.append( x1 )
		self.append( x2 )
		self.append( x3 )
		self.append( y1 )
		self.append( y2 )
		self.append( y3 )
		self.append( z1 )
		self.append( z2 )
		self.append( z3 )
		self.isMatrix = 1

	def __str__( self ):
		return 'breve.matrix( %f, %f, %f, %f, %f, %f, %f, %f, %f )' % ( self[ 0 ], self[ 1 ], self[ 2 ], self[ 3 ], self[ 4 ], self[ 5 ], self[ 6 ], self[ 7 ], self[ 8 ] )



class vector( array.array ):
	"A 3D vector class used for breve"

	defaultvector = [ 0.0, 0.0, 0.0 ]

	def __new__( cls, x = 0.0, y = 0.0, z = 0.0 ):
		return array.array.__new__( cls, 'd', [ x, y, z ] )

	def __getattr__( self, symbol ):
		if symbol == 'x':
			return self[ 0 ]

		if symbol == 'y':
			return self[ 1 ]

		if symbol == 'z':
			return self[ 2 ]

		raise AttributeError

	def __str__( self ):
		return 'breve.vector( %f, %f, %f )' % ( self[ 0 ], self[ 1 ], self[ 2 ] )

	def __eq__( self, o ):
		return self[ 0 ] == o[ 0 ] and self[ 1 ] == o[ 1 ] and self[ 2 ] == o[ 2 ]

	def __cmp__( self, o ):
		return self.__eq__( o )

	def __ne__( self, o ):
		return not self.__eq__( o )

	def __sub__( self, other ):
		return breveInternal.subVectors( self, other )

	def __add__( self, other ):
		return breveInternal.addVectors( self, other )

	# __add__ = breveInternal.addVectors

	def __div__( self, other ):
		return breveInternal.scaleVector( self, 1.0 / other )

	def __mul__( self, other ):
		if type( other ) == float or type( other ) == int:
			return breveInternal.scaleVector( self, other )

		if other.__class__ == matrix:
			x = self[ 0 ] * other[ 0 ] + self[ 1 ] * other[ 1 ] + self[ 2 ] * other[ 2 ]
			y = self[ 0 ] * other[ 3 ] + self[ 1 ] * other[ 4 ] + self[ 2 ] * other[ 5 ]
			z = self[ 0 ] * other[ 6 ] + self[ 1 ] * other[ 7 ] + self[ 2 ] * other[ 8 ]

			return vector( x, y, z )

	__rmul__ = __mul__

	def __neg__( self ):
		return breveInternal.scaleVector( self, -1 )

	def length( self ):
		"Gives the length of the vector"
		return breveInternal.vectorLength( self )

	def scale( self, other ):
		"Scales the vector by a scalar"
		return breveInternal.scaleVector( self, -1 )

	def normalize( self ):
		"Normalizes the vector"

		length = breveInternal.vectorLength( self )

		if length != 0.0:
			return breveInternal.scaleVector( self, 1.0 / length )

#
# Import all of the standard breve classes
#
# Ordered to respect inter-object dependencies
#

if 1:
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





class bridgeObjectMethod( object ):
	"A callable object used to implement breve bridge method calling"
	
	def __init__( self, caller, method ):
		self.caller = caller 
		self.method = method

	def __call__( self, *tuple ):
		"Executes this object's internal bridge method"
		return breveInternal.callBridgeMethod( breveInternal, self.caller, self.method, tuple )


class bridgeObject( object ):
	"An object representing a bridge to an object in another breve language frontend"

	def __init__( self ): 
		self.breveInstance = addInstance( self.__class__, self )
		self.breveModule = breveInternal

	def __getattr__( self, method ):
		internalMethod = breveInternal.findBridgeMethod( self, method )

		if internalMethod != None:
			self.__dict__[ method ] = bridgeObjectMethod( self, internalMethod )
			return self.__dict__[ method ]

		raise AttributeError( "Cannot locate method '%s'" % method )


breveInternalFunctionFinder = breveInternalFunctions()

breveInternal.bridgeObject = bridgeObject
breveInternal.vectorType = vector
breveInternal.matrixType = matrix

instanceDict = {}

