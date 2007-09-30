
# Note: this file was automatically converted to Python from the
# original steve-language source code.  Please see the original
# file for more detailed comments and documentation.


import breve

class MultiBody( breve.Object ):
	'''The MultiBody class is used to manipulate a group of connected OBJECT(Link) objects as a single logical body.  This means that  the entire group of links can be moved or otherwise manipulated using by referring to the MultiBody. <p> After the creating and connecting of the links is complete,  one of the OBJECT(Link) objects must be declared the root of  the MultiBody using the METHOD(set-root) method.  '''

	def __init__( self ):
		breve.Object.__init__( self )
		self.menus = breve.objectList()
		self.multibodyPointer = None
		self.rootLink = None
		self.selfCollisions = 0
		MultiBody.init( self )

	def addMenu( self, menuName, theMethod ):
		'''Adds a menu named menuName to the application which will result in a call to theMethod for the calling instance. <p> If the calling instance is the Controller object, then the menu will become the "main" simulation menu.  Otherwise, the menu will become a contextual menu associated with the specific object in the simulation. <p> Note that unlike the METHOD(handle-collision) which sets the collision handler for the whole type (class, that is), this method affects only the instance for which it is called, meaning that each instance of a certain class may have a different menu.'''

		newMenu = None

		newMenu = breve.createInstances( breve.MenuItem, 1 ).createMenu( menuName, self, theMethod )
		self.addDependency( newMenu )
		self.menus.append( newMenu )
		return newMenu

	def addMenuSeparator( self ):
		'''Adds a separator menu item--really just an empty menu item.'''

		newMenu = None

		newMenu = breve.createInstances( breve.MenuItem, 1 ).createMenu( '', self, '' )
		self.addDependency( newMenu )
		self.menus.append( newMenu )
		return newMenu

	def archive( self ):
		links = breve.objectList()
		i = None

		links = self.getAllConnectedObjects()
		for i in links:
			self.addDependency( i )

		return 1

	def checkSelfPenetration( self ):
		'''Checks to see if this MultiBody is colliding with itself.   This method is useful when a MultiBody is first built  to determine whether the "native" configuration of the body  is valid or not.  If the body is in a conflicting configuration  when it's first built, it may lead to unexpected behaviors as the body attempts to correct itself.'''

		return breve.breveInternalFunctionFinder.multibodyCheckSelfPenetration( self, self.multibodyPointer )

	def dearchive( self ):
		self.multibodyPointer = breve.breveInternalFunctionFinder.multibodyNew( self)
		self.setRoot( self.rootLink )
		breve.breveInternalFunctionFinder.multibodySetHandleSelfCollisions( self, self.multibodyPointer, self.selfCollisions )
		return 1

	def destroy( self ):
		breve.deleteInstances( self.getAllConnectedObjects() )
		breve.breveInternalFunctionFinder.multibodyFree( self, self.multibodyPointer )
		breve.deleteInstances( self.menus )

	def disableSelfCollisions( self ):
		'''Disables collisions between links within this MultiBody.   See METHOD(enable-self-collisions) for more information.'''

		self.selfCollisions = 0
		breve.breveInternalFunctionFinder.multibodySetHandleSelfCollisions( self, self.multibodyPointer, self.selfCollisions )

	def enableSelfCollisions( self ):
		'''Enables collisions between objects contained in the same MultiBody.   This option is disabled by default--it can lead to unexpected  behaviors if Links are inadvertantly forced together by the  configuration of the joints.'''

		self.selfCollisions = 1
		breve.breveInternalFunctionFinder.multibodySetHandleSelfCollisions( self, self.multibodyPointer, self.selfCollisions )

	def freeAllConnectedObjects( self ):
		'''Frees all of the OBJECT(Link) and OBJECT(Joint) objects which comprise this MultiBody.'''

		breve.deleteInstances( self.getAllConnectedObjects() )

	def getAllConnectedLinks( self ):
		'''Returns all of the OBJECT(Link) objects which comprise the  MultiBody.'''

		links = breve.objectList()
		allObjects = breve.objectList()
		link = None

		allObjects = self.getAllConnectedObjects()
		for link in allObjects:
			if link.isA( 'Link' ):
				links.append( link )



		return links

	def getAllConnectedObjects( self ):
		'''Returns all of the OBJECT(Link) and OBJECT(Joint) objects which comprise this MultiBody.'''

		return breve.breveInternalFunctionFinder.multibodyAllObjects( self, self.multibodyPointer )

	def getLocation( self ):
		'''Returns the location of the root link of the multibody.'''

		return self.rootLink.getLocation()

	def getMultibodyPointer( self ):
		'''Used internally.'''

		return self.multibodyPointer

	def getRoot( self ):
		'''Returns the root link.'''

		return self.rootLink

	def hideAxis( self ):
		'''Disables the axis for all connected links.'''

		self.getAllConnectedLinks().hideAxis()

	def hideBoundingBox( self ):
		'''Disables the bounding box for all connected links.'''

		self.getAllConnectedLinks().hideBoundingBox()

	def init( self ):
		self.multibodyPointer = breve.breveInternalFunctionFinder.multibodyNew( self)

	def move( self, newLocation ):
		'''Moves the entire MultiBody to newLocation.'''

		breve.breveInternalFunctionFinder.multibodySetLocation( self, self.multibodyPointer, newLocation )

	def register( self, root ):
		'''Deprecated.  Don't use.'''

		self.setRoot( root )

	def resumePhysics( self ):
		pass

	def rotate( self, thisAxis, amount ):
		'''Sets the rotation of this multibody around vector axis thisAxis by scalar amount (in radians).  This is an "absolute" rotation--the current rotation of the object does not affect how the object will be rotated.  For a rotation relative to the current orientation, set METHOD(relative-rotate). <p> This method implicitly changes the rotation and location of all attached link objects.'''

		length = 0

		length = breve.length( thisAxis )
		if ( length == 0.000000 ):
			return

		thisAxis = ( thisAxis / length )
		breve.breveInternalFunctionFinder.multibodySetRotation( self, self.multibodyPointer, thisAxis, amount )

	def setColor( self, newColor ):
		'''Sets the color for all links.'''

		self.getAllConnectedLinks().setColor( newColor )

	def setRoot( self, root ):
		'''Associates a MultiBody with a link root.  All links attached to root, both directly and indirectly, will implicitly be  part of this MultiBody as well.  When joints are created and destroyed, the MultiBody automatically updates.'''

		if ( not root.getLinkPointer() ):
			raise Exception( '''attempting to register MultiBody with uninitialized Link object''' )


		self.rootLink = root
		if ( not root ):
			self.removeDependency( self.rootLink )
			breve.breveInternalFunctionFinder.multibodySetRoot( self, self.multibodyPointer, 0 )
			return


		self.addDependency( self.rootLink )
		breve.breveInternalFunctionFinder.multibodySetRoot( self, self.multibodyPointer, root.getLinkPointer() )
		if self.selfCollisions:
			self.enableSelfCollisions()

		return self

	def setSelfCollisionParameters( self, erpValue, cfmValue ):
		'''Sets two contact parameters which effect the hardness of intra-body contacts. <li> Currently experimental and for advanced users only.  See the ODE  user's manual for a description of these values and how they should be used.'''

		breve.breveInternalFunctionFinder.multibodySetERPCFM( self, self.multibodyPointer, erpValue, cfmValue )

	def setTextureImage( self, texture ):
		'''Sets the texture for all links.'''

		self.getAllConnectedLinks().setTextureImage( texture )

	def setTextureScale( self, textureScale ):
		'''Sets the texture scale for all links.'''

		self.getAllConnectedLinks().setTextureScale( textureScale )

	def showAxis( self ):
		'''Enables the axis for all connected links.'''

		self.getAllConnectedLinks().showAxis()

	def showBoundingBox( self ):
		'''Enables the bounding box for all connected links.'''

		self.getAllConnectedLinks().showBoundingBox()

	def suspendPhysics( self ):
		pass


breve.MultiBody = MultiBody
# Add our newly created classes to the breve namespace

breve.MultiBodies = MultiBody



