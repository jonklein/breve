
import breve

class Real( breve.Object ):
	'''A class which is never instantiated--just used as a logical distinction from the Abstract classes.  See the child classes OBJECT(Mobile), OBJECT(Link) and OBJECT(Stationary) for more information.  The methods documented here may be used  with any of the child classes.'''

	__slots__ = [ 'bitmap', 'collisionHandlerList', 'collisionShape', 'color', 'displayShape', 'e', 'lightmap', 'lines', 'menus', 'mu', 'neighborhoodSize', 'realWorldPointer', 'texture', 'textureScaleX', 'textureScaleY' ]

	def __init__( self ):
		breve.Object.__init__( self )
		self.bitmap = 0
		self.collisionHandlerList = breve.objectList()
		self.collisionShape = None
		self.color = breve.vector()
		self.displayShape = None
		self.e = 0
		self.lightmap = 0
		self.lines = breve.objectList()
		self.menus = breve.objectList()
		self.mu = 0
		self.neighborhoodSize = 0
		self.realWorldPointer = None
		self.texture = 0
		self.textureScaleX = 0
		self.textureScaleY = 0
		Real.init( self )

	def addDottedLine( self, otherObject, theColor = breve.vector( 0.000000, 0.000000, 0.000000 ) ):
		'''Adds a dotted line to otherObject.  See METHOD(add-line) for more information on object lines.'''


		self.addLine( otherObject, theColor, '''- - - - - - - - ''' )

	def addLine( self, otherObject, theColor = breve.vector( 0.000000, 0.000000, 0.000000 ), theStyle = "----------------" ):
		'''Adds a line to be drawn from this object to otherObject.  The line can be removed  later using METHOD(remove-line). <P> The optional argument theColor specifies the color of the line.  The default color is the vector (0, 0, 0), black. <P> The optional argument theStyle specifies a pattern for the line.  theStyle is a  string of 16 spaces and/or dashes which specify the line pattern to be drawn. A dotted line, for example, would use the pattern "- - - - - - - - ".  A thickly dashed line would use the pattern "--------		".  If no style is given, a  solid line is drawn.  <P> If a line to otherObject already exists, its color and/or style will be updated.'''

		line = None

		self.removeLine( otherObject )
		line = breve.createInstances( breve.Line, 1 )
		line.connect( self, otherObject, theColor, theStyle )
		self.lines.append( line )

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

	def dearchive( self ):
		''''''

		handler = breve.objectList()

		for handler in self.collisionHandlerList:
			if handler[ 1 ]:
				breve.breveInternalFunctionFinder.addCollisionHandler( self, self, handler[ 0 ], handler[ 1 ] )

			if handler[ 2 ]:
				breve.breveInternalFunctionFinder.setIgnoreCollisionsWith( self, self, handler[ 0 ], 1 )



		if ( self.texture > -1 ):
			self.setTexture( self.texture )

		if ( self.lightmap > -1 ):
			self.setLightmap( self.lightmap )

		if ( self.bitmap > -1 ):
			self.setBitmap( self.bitmap )

		self.setTextureScaleX( self.textureScaleX )
		self.setTextureScaleY( self.textureScaleY )
		self.setNeighborhoodSize( self.neighborhoodSize )
		self.setColor( self.color )
		return breve.Object.dearchive( self )

	def delete( self ):
		''''''


		if self.realWorldPointer:
			breve.breveInternalFunctionFinder.realRemoveObject( self, self.realWorldPointer )

		breve.deleteInstances( self.menus )

	def deleteInstance( self ):
		'''Produces a dialog box (if supported by the current breve  engine) asking if the user wants to delete the object.   This is typically used in response to a user action like a click or menu callback.'''

		result = 0

		result = self.controller.showDialog( '''Really Delete Instance?''', '''Deleting this object may cause a fatal error in the simulation.  Really remove it?''', 'Okay', 'Cancel' )
		if result:
			breve.deleteInstances( self )


	def disableShadows( self ):
		'''If shadow volumes are enabled (see OBJECT(Control)), disables shadow volumes for this object. This is the default when shadow volumes have been enabled.'''


		breve.breveInternalFunctionFinder.realSetDrawShadows( self, self.realWorldPointer, 0 )

	def drawAsPoint( self ):
		'''Draws the object as a single point.  This is by far the fastest way to display an agent.  Points can be used to draw upwards of 20,000 agents with a reasonable frame rate, while drawing as many spheres or  bitmaps would slow down the simulation significantly.'''


		breve.breveInternalFunctionFinder.realSetDrawAsPoint( self, self.realWorldPointer, 1 )

	def enableShadows( self ):
		'''If shadow volumes are enabled (see OBJECT(Control)), enables shadow volumes for this object. This is the default when shadow volumes have been enabled.'''


		breve.breveInternalFunctionFinder.realSetDrawShadows( self, self.realWorldPointer, 1 )

	def getCollisionShape( self ):
		'''Returns the OBJECT(Shape) used for collisions associated with this Mobile object.'''


		return self.collisionShape

	def getColor( self ):
		'''Returns the color of the object.'''


		return self.color

	def getDisplayShape( self ):
		'''Returns the OBJECT(Shape) used for display associated with this Mobile object.'''


		return self.displayShape

	def getLightExposure( self ):
		'''When used in conjunction with light exposure detection (OBJECTMETHOD(Control:enable-light-exposure-detection)), this method returns the level of light exposure on this object.'''


		if self.realWorldPointer:
			return breve.breveInternalFunctionFinder.realGetLightExposure( self, self.realWorldPointer )
		else:
			return 0


	def getLocation( self ):
		'''Returns the vector location of this object. '''


		return breve.breveInternalFunctionFinder.realGetLocation( self, self.realWorldPointer )

	def getNeighborhoodSize( self ):
		'''gets the neighborhood size for the current object.'''


		return self.neighborhoodSize

	def getNeighbors( self ):
		'''Returns a list of all real objects in the simulation that are within the "neighborhood" range of this object in the world.'''


		return breve.breveInternalFunctionFinder.realGetNeighbors( self, self.realWorldPointer )

	def getRotation( self ):
		'''Deprecated'''


		return breve.breveInternalFunctionFinder.realGetRotation( self, self.realWorldPointer )

	def getRotationMatrix( self ):
		'''Returns the matrix rotation of this object. '''


		return breve.breveInternalFunctionFinder.realGetRotation( self, self.realWorldPointer )

	def getShape( self ):
		'''Deprecated '''


		return self.collisionShape

	def getWorldObjectPointer( self ):
		'''Used internally.'''


		return self.realWorldPointer

	def handleCollisions( self, theType, theMethod ):
		'''Adds a collision handler for this object.  When a collision occurs between an instance of the this type and theType, the breve engine will automatically call theMethod of the colliding instance.'''


		self.collisionHandlerList.append( [ theType, theMethod, 0 ] )
		breve.breveInternalFunctionFinder.addCollisionHandler( self, self, theType, theMethod )

	def hideAxis( self ):
		'''Hides the X and Y axes for the object.  The axes are hidden by default, so you'll only need this method if you've previously enabled them using METHOD(show-axis).'''


		breve.breveInternalFunctionFinder.realSetDrawAxis( self, self.realWorldPointer, 0 )

	def hideBoundingBox( self ):
		'''Hides the bounding box for the object.  The bounding box is  hidden by default, so you'll only need this method if you've  previously enabled them using METHOD(show-axis).'''


		breve.breveInternalFunctionFinder.realSetBoundingBox( self, self.realWorldPointer, 0 )

	def hideNeighborLines( self ):
		'''Hides lines to this objects neighbors.'''


		breve.breveInternalFunctionFinder.realSetNeighborLines( self, self.realWorldPointer, 0 )

	def ignoreCollisions( self, theType ):
		'''Instructs the engine to ignore physical collisions with theType objects. This does not affect collision callbacks specified with METHOD(handle-collisions).'''


		self.collisionHandlerList.append( [ theType, 0, 1 ] )
		breve.breveInternalFunctionFinder.setIgnoreCollisionsWith( self, self, theType, 1 )

	def init( self ):
		''''''


		self.texture = -1
		self.lightmap = -1
		self.bitmap = -1
		self.textureScaleX = 16
		self.textureScaleY = 16
		self.addMenu( '''Delete Instance''', 'deleteInstance' )
		self.addMenu( '''Follow With Camera''', 'watch' )

	def makeInvisible( self ):
		'''Makes the object invisible.  Can be made visible again later using the method METHOD(make-visible).'''


		breve.breveInternalFunctionFinder.realSetVisible( self, self.realWorldPointer, 0 )

	def makeVisible( self ):
		'''Makes the object visible again (if it has previously been hidden using METHOD(make-invisible).'''


		breve.breveInternalFunctionFinder.realSetVisible( self, self.realWorldPointer, 1 )

	def move( self, newLocation ):
		'''Moves this object to location newLocation.'''


		if ( not self.realWorldPointer ):
			raise Exception( '''attempt to move uninitialized Mobile object.''' )


		breve.breveInternalFunctionFinder.realSetLocation( self, self.realWorldPointer, newLocation )

	def point( self, theVertex, theLocation ):
		'''An easier way to rotate an object--this function rotates an object such that the local point theVertex, points towards the world direction theLocation.  In other words, theLocation is where you want the object to face, and theVertex indicates which side of the object is to be considered the "front".'''

		v = breve.vector()
		a = 0

		if ( ( breve.length( theVertex ) == 0.000000 ) or ( breve.length( theLocation ) == 0.000000 ) ):
			return

		v = breve.breveInternalFunctionFinder.cross( self, theVertex, theLocation )
		a = breve.breveInternalFunctionFinder.angle( self, theVertex, theLocation )
		if ( breve.length( v ) == 0.000000 ):
			self.rotate( theVertex, 0.010000 )
			return


		self.rotate( v, a )

	def raytrace( self, theLocation, theDirection ):
		'''Computes the vector from theLocation towards theDirection that hits the shape of this object. <p> If the object was not hit vector (0, 0, 0) will be returned. <p> The location and direction vector must be given relative to the world's coordinate frame. '''


		return breve.breveInternalFunctionFinder.raytrace( self, self.realWorldPointer, theLocation, theDirection )

	def removeAllLines( self ):
		'''Removes all lines connecting this object to other objects.'''


		breve.deleteInstances( self.lines )
		self.lines = []

	def removeLine( self, otherObject ):
		'''Removes the line connecting this object to otherObject.'''

		line = None

		for line in self.lines:
			if line.isLinked( otherObject ):
				breve.deleteInstances( line )
				return





	def setBitmap( self, textureNumber ):
		'''Deprecated.'''


		self.bitmap = textureNumber
		breve.breveInternalFunctionFinder.realSetBitmap( self, self.realWorldPointer, ( textureNumber + 1 ) )

	def setBitmapHeading( self, radianAngle ):
		'''If this object is in 2d bitmap mode, the rotation of the  bitmap will be set to radianAngle.'''


		breve.breveInternalFunctionFinder.realSetBitmapRotation( self, self.realWorldPointer, radianAngle )

	def setBitmapHeadingPoint( self, rotationVector ):
		'''If this object is in 2d bitmap mode, the rotation of the  bitmap will be set to degreeAngle degrees.'''


		breve.breveInternalFunctionFinder.realSetBitmapRotationTowardsVector( self, self.realWorldPointer, rotationVector )

	def setBitmapImage( self, bitmapImage ):
		'''Changes the bitmap of this object to bitmapImage, an instance of  class image.  If bitmapImage is NULL, bitmapping is turned off for the object.'''


		if ( not bitmapImage ):
			self.bitmap = -1
		else:
			self.bitmap = bitmapImage.getTextureNumber()

		breve.breveInternalFunctionFinder.realSetBitmap( self, self.realWorldPointer, self.bitmap )

	def setBitmapTransparency( self, alphaValue ):
		'''Sets the transparency to alphaValue, a number between 0.0  (totally transparent) and 1.0 (fully opaque). '''


		breve.breveInternalFunctionFinder.realSetAlpha( self, self.realWorldPointer, alphaValue )

	def setCollisionShape( self, theShape ):
		'''Associates a OBJECT(Shape) object with this object for collision purposes.  This shape will not be displayed for this object unless METHOD(set-display-shape) is also called with this shape.  Returns this object.'''


		if ( ( not theShape ) or ( not theShape.getPointer() ) ):
			raise Exception( '''attempt to register Mobile object with uninitialized shape (%s)''' % (  theShape ) )


		if self.collisionShape:
			self.removeDependency( self.collisionShape )

		self.collisionShape = theShape
		self.addDependency( self.collisionShape )
		breve.breveInternalFunctionFinder.realSetShape( self, self.realWorldPointer, theShape.getPointer() )
		return self

	def setColor( self, newColor ):
		'''Sets the color of this object to newColor.'''


		self.color = newColor
		breve.breveInternalFunctionFinder.realSetColor( self, self.realWorldPointer, newColor )

	def setDisplayShape( self, theShape ):
		'''Associates a OBJECT(Shape) object with this object for display purposes only. This shape will not be used for collision detection unless it is passed to  METHOD(set-collision-shape) as well.  Returns this object.'''


		if ( ( not theShape ) or ( not theShape.getPointer() ) ):
			raise Exception( '''attempt to register Mobile object with uninitialized shape (%s)''' % (  theShape ) )


		if self.displayShape:
			self.removeDependency( self.displayShape )

		self.displayShape = theShape
		self.addDependency( self.displayShape )
		breve.breveInternalFunctionFinder.realSetDisplayShape( self, self.realWorldPointer, theShape.getPointer() )
		return self

	def setE( self, newE ):
		'''Sets the "coefficient of restitution" a value which determines the elasticity of the object in a collision. Valid values range from 0.0 to 1.0, with 0.0 representing a totally inelastic collision (such as a lump of clay) while 1.0 represents a totally (and unrealistically) elastic collision (such as a rubber ball).'''


		self.e = newE
		breve.breveInternalFunctionFinder.realSetCollisionProperties( self, self.realWorldPointer, self.e, self.mu )

	def setET( self, newET ):
		'''Deprecated.'''


		breve.breveInternalFunctionFinder.realSetCollisionProperties( self, self.realWorldPointer, self.e, self.mu )

	def setLightmap( self, textureNumber ):
		'''Deprecated.'''


		self.lightmap = textureNumber
		breve.breveInternalFunctionFinder.realSetLightmap( self, self.realWorldPointer, ( self.lightmap + 1 ) )

	def setLightmapImage( self, lightmapImage ):
		'''Sets the object to be displayed using a "lightmap".  A  lightmap uses the texture specified and treats it like a light source.  It's hard to explain.  Give it a try for yourself. <p> set-lightmap only has an effect on sphere shapes.  Other  shapes can be textured, but only spheres can be made into  lightmaps.'''


		if ( not lightmapImage ):
			self.lightmap = -1
		else:
			self.lightmap = lightmapImage.getTextureNumber()

		breve.breveInternalFunctionFinder.realSetLightmap( self, self.realWorldPointer, self.lightmap )

	def setMu( self, newMu ):
		'''Sets the coefficient of friction to newMu.  mu is a  parameter controlling friction between two bodies and  may be any value between 0 and infinity.'''


		self.mu = newMu
		breve.breveInternalFunctionFinder.realSetCollisionProperties( self, self.realWorldPointer, self.e, self.mu )

	def setNeighborhoodSize( self, size ):
		'''Used in conjunction with METHOD(get-neighbors), this function will set the neighborhood size for the current object.'''


		self.neighborhoodSize = size
		breve.breveInternalFunctionFinder.realSetNeighborhoodSize( self, self.realWorldPointer, size )

	def setRotation( self, theRotation ):
		''''''


		self.setRotationMatrix( theRotation )

	def setRotation( self, thisAxis, amount ):
		'''Sets the rotation of this object around vector axis thisAxis  by scalar amount (in radians).  This is an "absolute" rotation--the  current rotation of the object does not affect how the  object will be rotated.  For a rotation relative to the  current orientation, set METHOD(relative-rotate).'''

		length = 0

		length = breve.length( thisAxis )
		if ( length == 0.000000 ):
			return

		thisAxis = ( thisAxis / length )
		breve.breveInternalFunctionFinder.realSetRotation( self, self.realWorldPointer, thisAxis, amount )

	def setRotationEulerAngles( self, angles ):
		'''Sets the rotation of this object to the Euler angles specified by angles (in radians).'''

		m = breve.matrix()
		r22 = 0
		r21 = 0
		r20 = 0
		r12 = 0
		r11 = 0
		r10 = 0
		r02 = 0
		r01 = 0
		r00 = 0

		r00 = ( ( breve.breveInternalFunctionFinder.cos( self, angles.z ) * breve.breveInternalFunctionFinder.cos( self, angles.x ) ) - ( ( breve.breveInternalFunctionFinder.cos( self, angles.y ) * breve.breveInternalFunctionFinder.sin( self, angles.x ) ) * breve.breveInternalFunctionFinder.sin( self, angles.z ) ) )
		r01 = ( ( breve.breveInternalFunctionFinder.cos( self, angles.z ) * breve.breveInternalFunctionFinder.sin( self, angles.x ) ) + ( ( breve.breveInternalFunctionFinder.cos( self, angles.y ) * breve.breveInternalFunctionFinder.cos( self, angles.x ) ) * breve.breveInternalFunctionFinder.sin( self, angles.z ) ) )
		r02 = ( breve.breveInternalFunctionFinder.sin( self, angles.z ) * breve.breveInternalFunctionFinder.cos( self, angles.y ) )
		r10 = ( ( ( -breve.breveInternalFunctionFinder.sin( self, angles.z ) ) * breve.breveInternalFunctionFinder.cos( self, angles.x ) ) - ( ( breve.breveInternalFunctionFinder.cos( self, angles.y ) * breve.breveInternalFunctionFinder.sin( self, angles.x ) ) * breve.breveInternalFunctionFinder.cos( self, angles.z ) ) )
		r11 = ( ( ( -breve.breveInternalFunctionFinder.sin( self, angles.z ) ) * breve.breveInternalFunctionFinder.sin( self, angles.x ) ) + ( ( breve.breveInternalFunctionFinder.cos( self, angles.y ) * breve.breveInternalFunctionFinder.cos( self, angles.x ) ) * breve.breveInternalFunctionFinder.cos( self, angles.z ) ) )
		r12 = ( breve.breveInternalFunctionFinder.cos( self, angles.z ) * breve.breveInternalFunctionFinder.sin( self, angles.y ) )
		r20 = ( breve.breveInternalFunctionFinder.sin( self, angles.y ) * breve.breveInternalFunctionFinder.sin( self, angles.x ) )
		r21 = ( ( -breve.breveInternalFunctionFinder.sin( self, angles.y ) ) * breve.breveInternalFunctionFinder.cos( self, angles.x ) )
		r22 = breve.breveInternalFunctionFinder.cos( self, angles.y )
		m = breve.matrix(  r00, r01, r02, r10, r11, r12, r20, r21, r22 )
		self.setRotationMatrix( m )

	def setRotationMatrix( self, theRotation ):
		'''Sets the rotation of this object to the rotation matrix theRotation. Working with matrices can be complicated, so a more simple approach is to use METHOD(rotate).'''


		breve.breveInternalFunctionFinder.realSetRotationMatrix( self, self.realWorldPointer, theRotation )

	def setShape( self, theShape ):
		'''Sets the shape of this object, for both display and collision detection purposes.  To set the shape for one purpose, but not the other, use the methods METHOD(set-display-shape) or METHOD(set-collision-shape).'''


		self.setDisplayShape( theShape )
		self.setCollisionShape( theShape )

	def setTexture( self, textureNumber ):
		'''Deprecated -- use METHOD(set-texture-image) instead.'''


		breve.breveInternalFunctionFinder.realSetTexture( self, self.realWorldPointer, ( textureNumber + 1 ) )

	def setTextureImage( self, textureImage ):
		'''Changes the texture of this object to textureImage, an instance of  class Image.  If textureImage is NULL texturing is turned off for  the object.'''


		if ( not textureImage ):
			self.texture = -1
		else:
			self.texture = textureImage.getTextureNumber()

		breve.breveInternalFunctionFinder.realSetTexture( self, self.realWorldPointer, self.texture )

	def setTextureScale( self, scaleSize ):
		'''Changes the "scale" of the texture.  When a texture is applied over a shape, this value is used to decide how large the texture will be in terms of breve-world units.  The default value is 16, meaning that a 16x16 face will have one copy of the textured image. For smaller objects, this number will have to be decreased, or else the texture will be too big and will not be visible.'''


		self.textureScaleX = scaleSize
		breve.breveInternalFunctionFinder.realSetTextureScale( self, self.realWorldPointer, scaleSize, scaleSize )

	def setTextureScaleX( self, scaleSize ):
		'''Sets the texture scale in the X dimension.  The Y texture scale  value is unchanged.  See METHOD(set-texture-scale) for more information.'''


		self.textureScaleX = scaleSize
		breve.breveInternalFunctionFinder.realSetTextureScale( self, self.realWorldPointer, scaleSize, self.textureScaleY )

	def setTextureScaleY( self, scaleSize ):
		'''Sets the texture scale in the Y dimension.  The X texture scale  value is unchanged.  See METHOD(set-texture-scale) for more information.'''


		self.textureScaleY = scaleSize
		breve.breveInternalFunctionFinder.realSetTextureScale( self, self.realWorldPointer, self.textureScaleX, scaleSize )

	def setTransparency( self, alphaValue ):
		'''Sets the transparency of this object to alphaValue, a number  between 1.0 (totally opaque) and 0.0 (fully transparent).'''


		breve.breveInternalFunctionFinder.realSetAlpha( self, self.realWorldPointer, alphaValue )

	def showAxis( self ):
		'''Shows the X and Y axes for the object.'''


		breve.breveInternalFunctionFinder.realSetDrawAxis( self, self.realWorldPointer, 1 )

	def showBoundingBox( self ):
		'''Shows the bounding box for the object.'''


		breve.breveInternalFunctionFinder.realSetBoundingBox( self, self.realWorldPointer, 1 )

	def showNeighborLines( self ):
		'''Draws lines to this objects neighbors (when neighbor checking is enabled).'''


		breve.breveInternalFunctionFinder.realSetNeighborLines( self, self.realWorldPointer, 1 )

	def watch( self ):
		'''Makes the camera follow this object.'''


		self.controller.watch( self )


breve.Real = Real
breve.Reals = Real



