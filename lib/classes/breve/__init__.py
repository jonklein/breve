
import breveInternal
import sys, os, math, random, array

import cPickle

#
# Used internally to redirect Python output to the breve frontend
#

class breveStderrHandler:
 	def write( self, line ):
 		breveInternal.catchOutput( breveInternal, line, 1 )

class breveStdoutHandler:
 	def write( self, line ):
 		breveInternal.catchOutput( breveInternal, line, 0 )

sys.stderr = breveStderrHandler()
sys.stdout = breveStdoutHandler()


#
# Methods to implement breve engine archiving and dearchiving
#

class archiveObjectProxy( object ):
	def __init__( self, inObj ):
		try: 
			self.index = breve._archiveIndexMapping[ inObj ]
		except Exception, e:
			self.index = -1

		object.__init__( self )

def encodeToString( inObject, inMapping ):
	raise Exception( "Python instance archiving not supported" )


	breve._archiveIndexMapping = inMapping
	breve._currentArchive = inObject
 	result = cPickle.dumps( inObject )
	breve._archiveIndexMapping = None
	breve._currentArchive = None

	return result

def decodeFromString( inString ):
	raise Exception( "Python instance dearchiving not supported" )

	obj = cPickle.loads( inString )

	breveObjectInit( obj )

	return obj

import itertools

def traversePickleObject( inObj, inMapping, inFunction, seen = [] ):
	# 
	# Traverses an entire object including dicts, lists and sub-objects,
	# 

	if inObj in seen:
		return

	if hasattr( inObj, 'breveInstance' ):
		return

	seen.append( inObj )

	keys = None

	if hasattr( inObj, '__dict__' ):
		inObj = inObj.__dict__

	if issubclass( inObj.__class__, list ):
		keys = range( len( inObj ) )
	elif issubclass( inObj.__class__, dict ):
		keys = inObj.keys()

	for i in keys:
		inFunction( inObj, i, inMapping )

		if issubclass( inObj[ i ].__class__, list ) or issubclass( inObj[ i ].__class__, dict ) or hasattr( inObj[ i ], '__dict__' ):
			traversePickleObject( inObj[ i ], inMapping, inFunction, seen )

def replaceProxyWithInstance( inObject, inKey, inMapping ):
	if type( inObject[ inKey ]  ) == archiveObjectProxy:
		try:
			instance = inIndexToInstanceMapping[ inObject[ inKey ].index ]
		except:
			instance = None

		print "Replacing %s with %s [%d]" % ( inKey, instance, inObj[ inKey ].index )

		inObject[ inKey ] = instance

def replaceInstanceWithProxy( inObject, inKey, inMapping ):
	if issubclass( inObject[ inKey ].__class__, breve.Object ):
		inObject[ inKey ] = breve.archiveObjectProxy( inMapping[ inObject[ inKey ] ] )

	if breve.breveInternal.isCObject( inObject[ inKey ] ):
		inObject[ inKey ] = None

	if type( inObject[ inKey ] ) == breve.vector:
		inObject[ inKey ] = None


def finishDearchive( inIndexToInstanceMapping ):
	# The dearchive is now complete, but we need to fill in the instance pointers

	for key in inIndexToInstanceMapping:
		obj = inIndexToInstanceMapping[ key ]

		traversePickleObject( obj._archivedStateCache, inIndexToInstanceMapping, replaceProxyWithInstance )

		obj.__dict__.update( obj._archivedStateCache )

		print obj

#
# Sets the breve controller class
#

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

def addToInstanceLists( inCls, ininstance ):
	for i in inCls.__bases__:
		addToInstanceLists( i, ininstance )

	if not instanceDict.has_key( inCls ):
		instanceDict[ inCls ] = objectList()

	instanceDict[ inCls ].append( ininstance )

def removeFromInstanceLists( inCls, ininstance ):
	for i in inCls.__bases__:
		removeFromInstanceLists( i, ininstance )

	if not instanceDict.has_key( inCls ):
		instanceDict[ inCls ] = objectList()

	instanceDict[ inCls ].remove( ininstance )

def addInstance( inclass, ininstance ):
	addToInstanceLists( inclass, ininstance )

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
		if methodName.startswith( '__' ):
			return list.__getattr__( methodName )

		def execute( *args ):
			[ i.__getattribute__( methodName )( *args ) for i in self ]

		return execute

class hash( dict ):
	def __init__( self ):
		dict.__init__( self )

	def __getitem__( self, name ):
		try:
			return dict.__getitem__( self, name )
		except Exception:
			return None

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
		# the inInstances could be the "all" list -- copy it so that
		# we're not modifying the list as we traverse it!

		l = list( inInstances )
		for i in l:
			deleteInstance( i )
	else:
		deleteInstance( inInstances )

def traceDelete( inInstance, inCls = None ):
	# The breve "delete" method is special in that it is automatically
	# traced back to the parent classes, even without a "super delete"

	if inCls == None:
		inCls = inInstance.__class__

	# Only do a delete for the class itself, don't let it be passed to the 
	# superclass, because we'll trace-back manually to the superclass.

	if "delete" in inCls.__dict__:
		inCls.delete( inInstance )

	for i in inCls.__bases__:
		traceDelete( inInstance, i )

def deleteInstance( inInstance ):
	if inInstance and inInstance.breveInstance != None:
		inInstance.destroy()
		traceDelete( inInstance )
	
		breveInternal.removeInstance( breveInternal.breveEngine, inInstance )

		removeFromInstanceLists( inInstance.__class__, inInstance )

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

def usepath( inPath ):
	if not inPath in sys.path:
		sys.path.append( inPath )

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

def dictLookup( inDict, inKey ):
	try:
		return inDict[ inKey ]
	except Exception:
		return None

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

	def __new__( cls, x = 0.0, y = 0.0, z = 0.0 ):
		return array.array.__new__( vector, 'd', [ x, y, z ] )

	def __getattr__( self, symbol ):
		if symbol == 'x':
			return self[ 0 ]

		if symbol == 'y':
			return self[ 1 ]

		if symbol == 'z':
			return self[ 2 ]

		raise AttributeError

	def __setattr__( self, symbol, value ):
		if symbol == 'x':
			self[ 0 ] = value
			return

		if symbol == 'y':
			self[ 1 ] = value
			return

		if symbol == 'z':
			self[ 2 ] = value
			return

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
		return breveInternal.scaleVector( self, other )

	def normalize( self ):
		"Normalizes the vector"

		length = breveInternal.vectorLength( self )

		if length != 0.0:
			return breveInternal.scaleVector( self, 1.0 / length )

#
# Import all of the standard breve classes
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
	from SFFNetwork 	import *
	from Shape 		import *
	from Sound 		import *
	from Spring 		import *
	from Terrain 		import *
	from Tone 		import *
	from TurtleDrawing 	import *
	from Vector 		import *
	from IRSensor		import *
	from Wanderer 		import *

	from URL		import *
	from QGAME		import *
	from Push 		import *
	from PushGP 		import *



def breveObjectInit( inObj ):
	if not ( 'breveInstance' in inObj.__dict__ ):
		inObj.breveInstance = breve.addInstance( inObj.__class__, inObj )
		inObj.breveModule = breve.breveInternal

		if isinstance( inObj, breve.Control ):
			breve.breveInternal.setController( breve.breveInternal, inObj )

		inObj.controller = breve.breveInternalFunctionFinder.getController( inObj )

		if inObj.controller == None:
			raise ValueError( "Cannot locate breve controller object (no controller has been defined)" )
	


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

