import __main__
import breveInternal
import breve
import sys, os


class breveInternalFunctions:
	def __getattr__( self, method ):
		internalFunction = breveInternal.findInternalFunction( breveInternal.breveEngine, method )

		if internalFunction:
			def execute( object, *tuple ):
				return breveInternal.callInternalFunction( breveInternal, object, internalFunction, tuple )

			return execute


class vector:
	def __init__( self ):
		self.x = 0
		self.y = 0
		self.z = 0

	def __add__( self, other ):
		self.x += other.x
		self.y += other.y
		self.z += other.z
		return self

	__radd__ = __add__



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
		self.breveInstance = breveInternal.addInstance( breveInternal, self.__class__, self )
		self.controller = breveInternalFunctionFinder.getController( self )

class control( object ):
	"A class for implementing the breve controller object"

	def __init__( self ):
		object.__init__( self )


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
