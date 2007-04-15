
import breve

class Object( object ):
	'''Summary: the top level object class. <P> The Object class is the root class.  All classes used in breve have Object as an ancestor.  The object class implements some basic  services that all classes will have access to. <p> Subclassing Object directly is rare.  The classes OBJECT(Real) and  OBJECT(Abstract) are logical separations of the Object class containing  "real" objects (which correspond to a physical entity in the simulated  world) and "abstract" objects which are generally used for computation  or control of the real objects.  You should consider subclassing   one of these classes instead.'''

	slots = [ 'birthTime', 'controller' ]

	def __init__( self ):
		if not ( 'breveInstance' in self.__dict__ ):
			self.breveInstance = breve.addInstance( self.__class__, self )
			self.breveModule = breve.breveInternal
			self.controller = breve.breveInternalFunctionFinder.getController( self )

			if self.controller == None:
			 	print "No controller has been defined!"
			 	raise ValueError

                Object.init( self )

	def __del__( self ):
		pass

	def iterate( self ):
		pass

	def addDependency( self, i ):
		'''Makes this instance depend on instance i when archiving and  dearchiving.  This means that if this instance is archived, then i will also have to be archived, and that when this  instance is dearchived, that i will have to be dearchived  first. <p> Dependencies can cause large numbers of instances to be archived in response to a single archiving event (as dependencies of  dependencies, and dependencies of dependencies of dependencies, ad infinitum will also be archived).  This means that you should make dependencies sparingly, only when absolutely required. <p> Circular dependencies are forbidden.'''

		if i:
			breve.breveInternalFunctionFinder.addDependency( self, i )


	def announce( self, theMessage ):
		'''Sends a notification with the message theMessage to all observer   objects.  See METHOD(observe) for information on making an object an observer.'''

		breve.breveInternalFunctionFinder.notify( self, theMessage )

	def archive( self ):
		''''''

		return 1


	def archiveAsXml( self, fileName ):
		'''Writes the current object to the XML file fileName.'''

		breve.breveInternalFunctionFinder.archiveXMLObject( self, self, fileName )

	def callMethod( self, methodName ):
		'''Calls the method named methodName for this object.  Returns the result of the method call.'''

		return breve.breveInternalFunctionFinder.callMethodNamed( self, self, methodName, [] )


	def callMethod( self, methodName, argList ):
		'''Calls the method named methodName for this object.  Returns the result of the method call. <p> The arguments to the object are passed in using the list argList.  Since keywords are not passed in, this method relies on the order the arguments appear in the argument list and passes them to methodName in the order  in which they appear in methodName's definition. <p> Why not call a method  directly?  This method is used in circumstances where you might want to have some sort of callback method.  As an example, let's  say you write a general purpose class which can sort objects based  on different criteria.  How would the user specify these arbitrary criteria?  Using this method would allow the user to pass in the name of the method they want to use, and the sorting object could use this method to execute the callback. <p> If the concept of a callback doesn't make sense, then you can probably ignore this method.'''

		return breve.breveInternalFunctionFinder.callMethodNamed( self, self, methodName, argList )


	def canRespond( self, methodName ):
		'''Returns true or false (1 or 0) depending on whether this instance can respond to a method called methodName. <p> But wow, what an awkward declaration!  Same reason as the  method METHOD(is).  Again, works like a statement that replies with true or false: object can-respond to "run". <p> It's really not my fault that the infinitive of "can" is "be able".'''

		return breve.breveInternalFunctionFinder.respondsTo( self, self, methodName )


	def dearchive( self ):
		''''''

		return 1


	def destroy( self ):
		'''Automatically called when this object is freed.  This method should never be called manually.  If subclasses need to free objects or data, they should implement their own "destroy"  methods.'''

	def delete( self ):
		'''Automatically called when this object is freed.  This method should never be called manually.  If subclasses need to free objects or data, they should implement their own "destroy"  methods.'''

	def getAge( self ):
		'''Returns the number of seconds this object has existed in the  simulation.'''

		return ( self.controller.get_time() - self.birthTime )


	def getController( self ):
		'''Returns the controller object that associated with the current  simulation.  It's preferable to simply reference the variable "controller".'''

		return breve.breveInternalFunctionFinder.getController( self)


	def getDescription( self ):
		'''This method should provide a textual description of an object. When the "print" command prints an object, it calls this method to get a description of the object.  By default, print will show the class and pointer of an object, but by overriding this method in your own classes, you can append other sorts of data to the  output.'''


	def getType( self ):
		'''Returns as a string the type of this object.'''

		return breve.breveInternalFunctionFinder.objectName( self, self )


	def init( self ):
		''''''

		self.controller = self.getController()
		self.birthTime = self.controller.getTime()

	def isA( self, className ):
		'''This method returns true or false (1 or 0) depending on whether the instance in question belongs to class className.  This  method checks if className is a superclass of the current  object as well. <p> Deprecated because "is" is reserved in Python '''

		return breve.breveInternalFunctionFinder.isa( self, className )


	def isA( self, className ):
		'''This method returns true or false (1 or 0) depending on whether the instance in question belongs to class className.  This  method checks if className is a superclass of the current  object as well.'''

		return breve.breveInternalFunctionFinder.isa( self, className )


	def isASubclass( self, className ):
		'''Returns 1 if this object is a subclass of the class specified with className, returns 0 otherwise.'''

		return breve.breveInternalFunctionFinder.isa( self, className )


	def observe( self, theObject, theNotification, theMethod ):
		'''Causes the current object to observe theObject.  By registering as and observer, the current object will receive a call to theMethod whenever theObject calls the METHOD(announce) method with notificiation theNotification.'''

		breve.breveInternalFunctionFinder.addObserver( self, theObject, theNotification, theMethod )

	def postDearchiveSetController( self ):
		'''Used internally to set the controller instance for this variable.   Used after object dearchiving.'''

		self.controller = self.getController()

	def removeDependency( self, theObject ):
		'''Removes theObject from this object's dependency list.  See METHOD(add-dependency) for more information on dependencies.'''

		breve.breveInternalFunctionFinder.removeDependency( self, theObject )

	def schedule( self, theMethod, theTime ):
		'''Schedules a call to theMethod when the simulation time equals theTime.  The margin of error of the callback time is equal to iteration step (see METHOD(set-iteration-step)). <br> If you want to schedule an event at a time relative to the current time, use the method METHOD(get-time) to get the current simulation time and then add the offset you want.'''

		breve.breveInternalFunctionFinder.addEvent( self, theMethod, theTime, 0 )

	def scheduleRepeating( self, theMethod, theInterval ):
		''''''

		breve.breveInternalFunctionFinder.addEvent( self, theMethod, theInterval, theInterval )

	def sendOverNetwork( self, hostName, portNumber ):
		'''Sends this object over the network to a breve server on host hostName listening on port portNumber.'''

		return breve.breveInternalFunctionFinder.sendXMLObject( self, hostName, portNumber, self )


	def unobserve( self, theObject, theNotification ):
		'''Unregisters the current object as an observer of theObject with  notification theNotification.'''

		breve.breveInternalFunctionFinder.removeObserver( self, theObject, theNotification )


breve.Object = Object


