
# Note: this file was automatically converted to Python from the
# original steve-language source code.  Please see the original
# file for more detailed comments and documentation.


import breve

class Control( breve.Abstract ):
	'''Summary: a parent class for the "controller" object required for all simulations. <P> The Control object sets up and controls simulations.  Every simulation must have one Control subclass.  The user subclass of Control should  set up the simulation in the "init" method. <p> The Control class also acts as the main interaction between the user and the breve environment itself.  It provides access to elements of  the user interface, for example, so that the user can add menus and other interface features from inside simulations. <p> Because the breve engine is designed to run on a variety of systems, there are varying levels of support for some of these features.  In some cases, the features won't be supported at all on a system.  A background daemon written to use the breve engine, for example, will not place a dialog on the screen and wait for user input.'''

	def __init__( self ):
		self.breveInstance = breve.breveInternal.addInstance( breve.breveInternal, self.__class__, self )
		self.breveModule = breve.breveInternal
		breve.breveInternal.setController( breve.breveInternal, self )


		breve.Abstract.__init__( self )
		self.archivedBackgroundTextureImage = None
		self.archivedSkyboxImageList = breve.objectList()
		self.backgroundColor = breve.vector()
		self.blurFactor = 0
		self.blurFlag = 0
		self.blurMenu = None
		self.camOffset = breve.vector()
		self.camTarget = breve.vector()
		self.camera = None
		self.cameraPointer = None
		self.deltaOffset = breve.vector()
		self.deltaTarget = breve.vector()
		self.drawEveryFrame = 0
		self.drawMenu = None
		self.fogColor = breve.vector()
		self.fogFlag = 0
		self.fogIntensity = 0
		self.fogMenu = None
		self.frozen = 0
		self.genericLinkShape = None
		self.genericShape = None
		self.lightAmbientColor = breve.vector()
		self.lightDiffuseColor = breve.vector()
		self.lightFlag = 0
		self.lightMenu = None
		self.lightPosition = breve.vector()
		self.lightSpecularColor = breve.vector()
		self.movie = None
		self.movieMenu = None
		self.offsetting = 0
		self.reflectFlag = 0
		self.reflectMenu = None
		self.selectedObject = None
		self.shadowCatcherDefined = 0
		self.shadowFlag = 0
		self.shadowMenu = None
		self.shadowVolumeFlag = 0
		self.simStep = 0
		self.simTime = 0
		self.smoothFlag = 0
		self.stationaryTexture = None
		self.watchObject = None
		self.xRot = 0
		self.yRot = 0
		Control.init( self )

	def acceptUpload( self, uploadedObject, host ):
		'''This method is automatically called when an object is uploaded through a OBJECT(NetworkServer).  This implementation simply  prints out a message saying that the object has been received, but your controller may override the method to take other actions.'''

		print '''object %s sent from host %s''' % (  uploadedObject, host )

	def addMenu( self, menuName, theMethod ):
		'''Adds a menu named menuName to the application which will result in a call to theMethod for the calling instance. <p> If the calling instance is the Controller object, then the menu will become the "main" simulation menu.  Otherwise, the menu will become a contextual menu associated with the specific object in the simulation. <p> Note that unlike the METHOD(handle-collision) which sets the collision handler for the whole type (class, that is), this method affects only the instance for which it is called, meaning that each instance of a certain class may have a different menu.'''

		return breve.createInstances( breve.MenuItem, 1 ).createMenu( menuName, self, theMethod )

	def addMenuSeparator( self ):
		'''Adds a separator menu item--really just an empty menu item.'''

		return breve.createInstances( breve.MenuItem, 1 ).createMenu( '', self, '' )

	def aimCamera( self, location ):
		'''Depricated.'''

		self.setCameraTarget( location )

	def archive( self ):
		self.camTarget = breve.breveInternalFunctionFinder.cameraGetTarget( self)
		self.camOffset = breve.breveInternalFunctionFinder.cameraGetOffset( self)
		return breve.Abstract.archive( self )

	def beep( self ):
		'''Plays the system beep sound, if supported by the implementation.'''

		breve.breveInternalFunctionFinder.playSound( self)

	def bulletPanCameraOffset( self, amount, stepCount ):
		'''Sets the camera in motion to smoothly change the camera offset over stepCount iteration steps, with the physical simulation frozen in the meantime.'''

		self.frozen = 1
		self.panCameraOffset( amount, stepCount )

	def catchKey0x7FDown( self ):
		'''This method is automatically called when the delete key is pressed  down to delete the selected object.  It deletes the selected instance. Do not call this method manually--it would work, but it would be  a bit roundabout. '''

		if self.selectedObject:
			self.selectedObject.deleteInstance()


	def clearScreen( self ):
		'''Clears the camera to the current background color.  This  method clears blurred artifacts which are drawn after  enabling METHOD(enable-blur).  If blurring has not been enabled, this method has no visual effect.'''

		breve.breveInternalFunctionFinder.cameraClear( self, self.cameraPointer )

	def click( self, theObject ):
		'''Called automatically when the user clicks on an theObject from  the graphical display window of the simulation.  The default behavior of this method is to select the object that was clicked and execute its "click" method, if it exists.   <p> If you do not wish to allow users to select objects in your  simulation, you should implement your own click method in your controller object. <p> If you wish to implement your own click method, but still want to maintain the default behavior of this method, make sure you  call "super click on theObject" from your method. <p> <b>theObject may be NULL</b>--an uninitialized object.  This means that a click occurred, but no object was selected, i.e., a deselection event.  You should test theObject before calling any of its methods.'''

		if ( self.selectedObject == theObject ):
			return

		if self.selectedObject:
			self.selectedObject.hideBoundingBox()
			self.selectedObject.hideAxis()


		self.selectedObject = theObject
		if ( ( self.selectedObject and self.selectedObject.isA( 'Link' ) ) and self.selectedObject.getMultibody() ):
			self.selectedObject = self.selectedObject.getMultibody()

		if ( ( ( not self.selectedObject ) or ( not self.selectedObject.canRespond( 'showBoundingBox' ) ) ) or ( not self.selectedObject.canRespond( 'showAxis' ) ) ):
			return

		self.selectedObject.showBoundingBox()
		self.selectedObject.showAxis()

	def dearchive( self ):
		self.cameraPointer = breve.breveInternalFunctionFinder.getMainCameraPointer( self)
		self.camera.setCameraPointer( self.cameraPointer )
		if ( breve.length( self.archivedSkyboxImageList ) == 6 ):
			self.setSkyboxImages( self.archivedSkyboxImageList )

		if self.archivedBackgroundTextureImage:
			self.setBackgroundTextureImage( self.archivedBackgroundTextureImage )

		self.moveLight( self.lightPosition )
		self.setBackgroundColor( self.backgroundColor )
		self.setFogColor( self.fogColor )
		self.pointCamera( self.camTarget, self.camOffset )
		self.setBlurFactor( self.blurFactor )
		if ( self.lightFlag == 1 ):
			self.enableLighting()

		if ( self.shadowFlag == 1 ):
			self.enableShadows()

		if ( self.shadowVolumeFlag == 1 ):
			self.enableShadowVolumes()

		if ( self.fogFlag == 1 ):
			self.enableFog()

		if ( self.reflectFlag == 1 ):
			self.enableReflections()

		if ( self.smoothFlag == 1 ):
			self.enableSmoothDrawing()

		if ( self.blurFlag == 1 ):
			self.enableBlur()

		if self.drawEveryFrame:
			self.enableDrawEveryFrame()
		else:
			self.disableDrawEveryFrame()

		return breve.Abstract.dearchive( self )

	def dearchiveXml( self, filename ):
		'''Asks the controller to dearchive an object from an XML file.  The XML file must have been created using  OBJECTMETHOD(Object:archive-as-xml).'''

		return breve.breveInternalFunctionFinder.dearchiveXMLObject( self, filename )

	def disableBlur( self ):
		'''Disables blurring.  See METHOD(enable-blur) for more  information.'''

		self.blurFlag = 0
		breve.breveInternalFunctionFinder.cameraSetBlur( self, self.cameraPointer, 0 )
		self.blurMenu.uncheck()

	def disableDrawEveryFrame( self ):
		'''Allows the rendering engine to drop frames if the simulation is moving  faster than the display.  This can lead to faster simulations with choppier displays.  Not all breve development environments support this option.  The  Mac OS X application does, as do all threaded command-line breve programs. <p> There is rarely any benefit from using this method, except in instances where the drawing of a scene is complex, and the computation is simple.  The included DLA.tz demo is an example of one such simulation which benefits immensely from this feature.'''

		self.drawEveryFrame = 0
		breve.breveInternalFunctionFinder.setDrawEveryFrame( self, 0 )

	def disableFog( self ):
		'''Disables fog for the main camera.  See METHOD(enable-fog) for more information.'''

		self.fogFlag = 0
		breve.breveInternalFunctionFinder.cameraSetDrawFog( self, self.cameraPointer, 0 )
		self.fogMenu.uncheck()

	def disableFreedInstanceProtection( self ):
		'''Disabling freed instance protection means that the breve engine will not attempt to keep track of freed objects, and will yield better memory performance when large numbers of objects are being created and destroyed. <p> The downside is that improper access of freed instances may cause  crashes or unexpected behavior when freed instance protection is  disabled.  Simulations should thus always use freed instance  protection during development and testing, and the feature should only be disabled when the developer is confident that no freed instance bugs exist. <p> Freed instance protection may be reenabled with  METHOD(enable-freed-instance-protection), but only instances freed while instance protection is enabled will be protected.'''

		breve.breveInternalFunctionFinder.setFreedInstanceProtection( self, 0 )

	def disableLightExposureDetection( self ):
		'''Disables light exposure detection.  See METHOD(enable-light-exposure-detection).'''

		breve.breveInternalFunctionFinder.setDetectLightExposure( self, 0 )

	def disableLightExposureDrawing( self ):
		'''Disables drawing of the light exposure buffer to the screen.'''

		breve.breveInternalFunctionFinder.setDrawLightExposure( self, 0 )

	def disableLighting( self ):
		'''disable lighting for the main camera'''

		self.lightFlag = 0
		breve.breveInternalFunctionFinder.setDrawLights( self, self.lightFlag )
		self.lightMenu.uncheck()
		self.shadowMenu.disable()
		self.reflectMenu.disable()

	def disableOutline( self ):
		'''Disables outline drawing.'''

		breve.breveInternalFunctionFinder.setDrawOutline( self, 0 )

	def disableReflections( self ):
		'''Disable reflections for the main camera.  See METHOD(enable-reflections for  more information on reflections.'''

		self.reflectFlag = 0
		breve.breveInternalFunctionFinder.setDrawReflection( self, self.reflectFlag )
		self.reflectMenu.uncheck()

	def disableShadowVolumes( self ):
		'''Disable shadow volumes for the main camera.  See  METHOD(enable-shadow-volumes) for more information on shadows.'''

		self.shadowVolumeFlag = 0
		breve.breveInternalFunctionFinder.setDrawShadowVolumes( self, self.shadowVolumeFlag )
		self.shadowMenu.uncheck()

	def disableShadows( self ):
		'''Disable "flat" shadows for the main camera.  See  METHOD(enable-shadows) for more information on shadows.'''

		self.shadowFlag = 0
		breve.breveInternalFunctionFinder.setDrawShadow( self, self.shadowFlag )

	def disableSmoothDrawing( self ):
		'''Disable smooth drawing for the main camera.  See METHOD(enable-smooth-drawing) for more information.'''

		self.smoothFlag = 0
		breve.breveInternalFunctionFinder.cameraSetDrawSmooth( self, self.cameraPointer, self.smoothFlag )
		self.drawMenu.uncheck()

	def disableText( self ):
		'''Disable the timestamp and camera position texts (which appear when  changing the camera angle or position).  The text is on by default.   The text can be re-enabled using METHOD(enable-text).'''

		breve.breveInternalFunctionFinder.setDrawText( self, 0 )

	def enableBlur( self ):
		'''Enables blur.  Blurring simply draws a frame without  totally erasing the previous frame. '''

		self.blurFlag = 1
		breve.breveInternalFunctionFinder.cameraSetBlur( self, self.cameraPointer, 1 )
		self.blurMenu.check()
		self.clearScreen()

	def enableDrawEveryFrame( self ):
		'''If the method METHOD(disable-draw-every-frame) has been called previously,	 this method will resort to the default behavior, namely that the rendering engine will try to render an image for each and every iteration of the breve engine.'''

		self.drawEveryFrame = 1
		breve.breveInternalFunctionFinder.setDrawEveryFrame( self, 1 )

	def enableFog( self ):
		'''Enables fog for the main camera.  This adds the visual effect of fog to the world.  Fog parameters can be set using methods METHOD(set-fog-color),  METHOD(set-fog-intensity) and METHOD(set-fog-limits). <p> Fog and lightmap effects don't mix.'''

		self.fogFlag = 1
		breve.breveInternalFunctionFinder.cameraSetDrawFog( self, self.cameraPointer, 1 )
		self.fogMenu.check()

	def enableFreedInstanceProtection( self ):
		'''Freed instance protection means that the breve engine retains  instances which have been freed in order to make sure that they are not being incorrectly accessed.  This has a small memory cost associated with each freed object. <p> Freed instance protection is enabled by default, so you'll only need to call this method if it has been disabled using  METHOD(disable-freed-instance-protection).'''

		breve.breveInternalFunctionFinder.setFreedInstanceProtection( self, 1 )

	def enableLightExposureDetection( self ):
		'''<B>Experimental</B> <P> Light exposure detection will attempt to tell you how much "sunlight"  is reaching each object in your simulation.  You can set the location of the light source with METHOD(set-light-exposure-source).  Then,  use the method get-light-exposure (in OBJECT(Stationary), OBJECT(Mobile), and OBJECT(Link)) in order to find out how much light was detected for individual objects. <P> The direction of the sunlight is hardcoded towards the world point  (0, 0, 0), and only spreads out to fill an angle of 90 degrees. These limitations may be removed in the future if needed.'''

		breve.breveInternalFunctionFinder.setDetectLightExposure( self, 1 )

	def enableLightExposureDrawing( self ):
		'''Enables drawing of the light exposure buffer to the screen.'''

		breve.breveInternalFunctionFinder.setDrawLightExposure( self, 1 )

	def enableLighting( self ):
		'''enable lighting for the main camera'''

		self.lightFlag = 1
		breve.breveInternalFunctionFinder.setDrawLights( self, self.lightFlag )
		self.lightMenu.check()
		self.shadowMenu.enable()
		if self.shadowCatcherDefined:
			self.reflectMenu.enable()



	def enableOutline( self ):
		'''Enables outline drawing.  Outline drawing is a wireframe black and white draw style.  Reflections and textures are ignored when outlining is  enabled.  Outlining is useful for producing diagram-like images.  It  looks cool.'''

		breve.breveInternalFunctionFinder.setDrawOutline( self, 1 )

	def enableReflections( self ):
		'''Enable reflections for the main camera.  Reflections are used to draw a mirror image of objects in the world onto a single plane.  Because of  the complexity of drawing reflections, they can only be drawn onto a single plane of a OBJECT(Stationary) object--see the method catch-shadows of OBJECT(Stationary) for more information.  The reflection-catching object must already be defined in order for this method to take effect.'''

		if ( not self.shadowCatcherDefined ):
			return

		self.reflectFlag = 1
		breve.breveInternalFunctionFinder.setDrawReflection( self, self.reflectFlag )
		self.reflectMenu.check()

	def enableShadowVolumes( self ):
		'''Enables shadows drawn using a "shadow volume" algorithm.  This is an alternative to the shadows rendered using METHOD(enable-shadows). <p> Shadow volumes allow all objects in the simulation to shadow one-another, as opposed to having objects only shadow a single plane (as is the case with the METHOD(enable-shadows) algorithm).  Shadow volumes are in fact superior in every way but one: shadow volumes will not generate accurate shadows of bitmapped objects the way the original algorithm will. If you want high-quality bitmap shadows in your simulation, use  METHOD(enable-shadows), otherwise, shadow volumes are likely the better choice.'''

		self.shadowVolumeFlag = 1
		breve.breveInternalFunctionFinder.setDrawShadowVolumes( self, self.shadowVolumeFlag )
		self.shadowMenu.check()

	def enableShadows( self ):
		'''Enable shadows for the main camera.  Shadows use the current  position of the light in order to render shadows onto a flat  plane in the world.  Because of the complexity of drawing shadows,  they can only be drawn onto a single plane of a OBJECT(Stationary)  object--see the method catch-shadows of OBJECT(Stationary) for more  information.  The shadow-catching object must already be defined in order for this method to take effect. <P> For an improved shadowing algorithm, see METHOD(enable-shadow-volumes).'''

		if ( not self.shadowCatcherDefined ):
			return

		self.shadowFlag = 1
		breve.breveInternalFunctionFinder.setDrawShadow( self, self.shadowFlag )

	def enableSmoothDrawing( self ):
		'''Enable smooth drawing for the main camera.  Smooth drawing enables  a smoother blending of colors, textures and lighting.  This feature is especially noticeable when dealing with spheres or large objects. <p> It is strongly recommended that smooth drawing be enabled whenever  lighting is enabled (see METHOD(enable-lighting)).  Otherwise, major artifacts may be visible, especially on larger polygons. <p> The disadvantage of smooth drawing is a potential performance hit. The degree of this performance hit depends on the number of polygons in the scene.  If speed is an issue, it is often best to disable  both lighting and smooth drawing.'''

		self.smoothFlag = 1
		breve.breveInternalFunctionFinder.cameraSetDrawSmooth( self, self.cameraPointer, self.smoothFlag )
		self.drawMenu.check()

	def enableText( self ):
		'''enables the timestamp and camera position texts (which appear when changing  the camera angle or position).  This is the default setting.  The text can  be disabled using METHOD(disable-text).'''

		breve.breveInternalFunctionFinder.setDrawText( self, 1 )

	def endSimulation( self ):
		'''Ends the simulation gracefully.'''

		breve.breveInternalFunctionFinder.endSimulation( self)

	def execute( self, systemCommand ):
		'''Executes the shell command systemCommand using /bin/sh.   Returns the output of command.  Supported on UNIX-based implementations only (Mac OS X and Linux included), not supported on Windows.'''

		return breve.breveInternalFunctionFinder.system( self, systemCommand )

	def getArgument( self, theIndex ):
		'''If this instance of breve was run from the command line, this method returns the argument at index theIndex.  The argument is always returned as a string, though this may naturally be converted to other types  depending on the context.  The arguments (like arrays and lists in <i>steve</i>) are zero based, meaning that the first element has index 0.  <b>The first  argument (the one at index 0) is always the name of the simulation  file</b>.  Use this method in conjunction with  METHOD(get-argument-count).  Make sure you check the number of arguments available before calling this method--requesting an out-of-bounds argument will cause a fatal error in the simulation.'''

		return breve.breveInternalFunctionFinder.getArgv( self, theIndex )

	def getArgumentCount( self ):
		'''If this instance of breve was run from the command line, this method returns the number of arguments passed to the program.  <b>The first argument is always the name of the simulation file</b>.  Use this method in conjunction with METHOD(get-argument).'''

		return breve.breveInternalFunctionFinder.getArgc( self)

	def getCameraOffset( self ):
		'''Returns the current offset from of the camera from its target.   Note that the camera offset can be changed manually by the user,  so it may be wise to use this function in conjunction with camera  movements to ensure consistency.'''

		return breve.breveInternalFunctionFinder.cameraGetOffset( self)

	def getCameraTarget( self ):
		'''Returns the current target of the camera.  Note that the camera target can be changed manually by the user, so it may be wise  to use this function in conjunction with camera movements to  ensure consistency.'''

		return breve.breveInternalFunctionFinder.cameraGetTarget( self)

	def getDragObject( self ):
		if ( self.selectedObject and ( self.selectedObject.isA( 'Mobile' ) or self.selectedObject.isA( 'MultiBody' ) ) ):
			return self.selectedObject

		return 0

	def getGenericLinkShape( self ):
		'''Returns a "generic" shape for links, a cube with size (.1, 1, .1).'''

		return self.genericLinkShape

	def getGenericShape( self ):
		'''Returns a "generic" shape for agents, a sphere of radius 1.0.'''

		return self.genericShape

	def getHsvColor( self, rgbColor ):
		'''All colors in breve expect colors in the RGB format--a vector where  the 3 elements represent red, green and blue intensity on a scale  from 0.0 to 1.0.   <p> This method returns the HSV color vector for a given vector rgbColor in RGB color format.  '''

		return breve.breveInternalFunctionFinder.RGBtoHSV( self, rgbColor )

	def getIntegrationStep( self ):
		'''Returns the current integration step size.'''

		return self.simStep

	def getInterfaceVersion( self ):
		'''Returns a string identifying the program using the breve engine.   This string is in the format "name/version".'''

		return breve.breveInternalFunctionFinder.getInterfaceVersion( self)

	def getIterationStep( self ):
		'''Returns the current iteration step size.'''

		return self.simTime

	def getLightExposureCamera( self ):
		'''Returns a camera that can be used to control the light detection light-source.'''

		self.camera = breve.createInstances( breve.Camera, 1 )
		self.camera.setCameraPointer( breve.breveInternalFunctionFinder.getLightExposureCamera( self) )

	def getMainCamera( self ):
		'''Returns the OBJECT(Camera) object corresponding to the main camera. This allows you to directly control camera options.'''

		return self.camera

	def getMouseXCoordinate( self ):
		'''Returns the X-coordinate of the mouse relative to the simulation window. The value may be negative if the mouse is to the left of the simulation view. See also METHOD(get-mouse-y-coordinate).'''

		return breve.breveInternalFunctionFinder.getMouseX( self)

	def getMouseYCoordinate( self ):
		'''Returns the Y-coordinate of the mouse. The value may be negative if the mouse is outside of the simulation view, towards the bottom of the screen.   See also METHOD(get-mouse-x-coordinate).'''

		return breve.breveInternalFunctionFinder.getMouseY( self)

	def getRealTime( self ):
		'''Returns the number of seconds since January 1st, 1970 with microsecond precision.'''

		return breve.breveInternalFunctionFinder.getRealTime( self)

	def getRgbColor( self, hsvColor ):
		'''All colors in breve expect colors in the RGB format--a vector where  the 3 elements represent red, green and blue intensity on a scale  from 0.0 to 1.0.   <p> This method returns the RGB color vector for a given vector hsvColor in HSV color format.  '''

		return breve.breveInternalFunctionFinder.HSVtoRGB( self, hsvColor )

	def getSelection( self ):
		'''Returns the "selected" object--the object which  has been clicked on in the simulation.'''

		if ( ( self.selectedObject and self.selectedObject.isA( 'Link' ) ) and self.selectedObject.getMultibody() ):
			return self.selectedObject.getMultibody()

		return self.selectedObject

	def getStationaryTexture( self ):
		return self.stationaryTexture

	def getTime( self ):
		'''Returns the simulation time of the world.'''

		return breve.breveInternalFunctionFinder.getTime( self)

	def init( self ):
		'''Initializes the Control object by setting up default values for  variables such as the size of the integration timestep.  Subclasses of Control may override these defaults in their own init functions.'''

		self.cameraPointer = breve.breveInternalFunctionFinder.getMainCameraPointer( self)
		self.stationaryTexture = breve.createInstances( breve.Image, 1 )
		self.stationaryTexture.load( 'images/noise.png' )
		self.camera = breve.createInstances( breve.Camera, 1 )
		self.addDependency( self.camera )
		self.camera.setCameraPointer( self.cameraPointer )
		self.shadowCatcherDefined = 0
		self.setIntegrationStep( 0.005000 )
		self.setIterationStep( 0.050000 )
		self.enableDrawEveryFrame()
		self.setBackgroundColor( breve.vector( 0.500000, 0.700000, 1.000000 ) )
		self.setFogColor( breve.vector( 0.800000, 0.800000, 0.800000 ) )
		self.xRot = 0.000000
		self.yRot = 0.000000
		self.moveLight( breve.vector( 0, 0, 0 ) )
		self.pointCamera( breve.vector( 0, 0, 0 ), breve.vector( 0, 0, 30 ) )
		self.setBackgroundScrollRate( 0.001000, 0.000100 )
		self.lightMenu = self.addMenu( '''Use Lighting''', 'toggleLighting' )
		self.drawMenu = self.addMenu( '''Use Smooth Shading''', 'toggleSmooth' )
		self.shadowMenu = self.addMenu( '''Draw Shadows''', 'toggleShadows' )
		self.fogMenu = self.addMenu( '''Draw Fog''', 'toggleFog' )
		self.reflectMenu = self.addMenu( '''Draw Reflections''', 'toggleReflections' )
		self.blurMenu = self.addMenu( '''Use Motion Blur''', 'toggleBlur' )
		self.addMenuSeparator()
		self.movieMenu = self.addMenu( '''Record Movie to "simulation.mpeg"''', 'toggleRecordingToMovie' )
		self.addMenu( '''Save Snapshot to "simulation.png"''', 'saveSnapshotToFile' )
		self.enableSmoothDrawing()
		self.disableLighting()
		self.addMenuSeparator()
		self.genericShape = breve.createInstances( breve.Sphere, 1 ).initWith( 1.000000 )
		self.genericLinkShape = breve.createInstances( breve.Cube, 1 ).initWith( breve.vector( 0.100000, 1, 0.100000 ) )

	def iterate( self ):
		result = 0

		if self.watchObject:
			self.setCameraTarget( self.watchObject.getLocation() )

		if ( not self.frozen ):
			result = breve.breveInternalFunctionFinder.worldStep( self, self.simTime, self.simStep )

		if ( result == -1 ):
			raise Exception( '''An error occurred during world simulation.''' )


		if self.offsetting:
			self.setCameraOffset( ( self.getCameraOffset() + self.deltaOffset ) )
			self.setCameraTarget( ( self.getCameraTarget() + self.deltaTarget ) )
			self.offsetting = ( self.offsetting - 1 )
			if ( self.offsetting == 0 ):
				self.frozen = 0




	def loadImage( self, file ):
		'''Loads an image from a file, returning an OBJECT(Image) object. <P> This method is provided for backwards compatability only. The p <p>'''

		image = None

		print '''warning: the Control method "load-image" is now deprecated!'''
		image = breve.createInstances( breve.Image, 1 )
		if image.load( file ):
			self.addDependency( image )

		else:
			breve.deleteInstances( image )
			return -1


		return image.getTextureNumber()

	def loadImageWithoutAlpha( self, file ):
		'''Deprecated.'''

		return self.loadImage( file )

	def makeNewInstance( self, className ):
		'''Returns a new instance of the class className.'''

		return breve.breveInternalFunctionFinder.newInstanceForClassString( self, className )

	def moveLight( self, theLocation, n = 0.000000 ):
		'''Moves the source light to theLocation.  The default position is  (0, 0, 0) which is the origin of the world.  '''

		self.lightPosition = theLocation
		breve.breveInternalFunctionFinder.brISetLightPosition( self, theLocation, n )

	def offsetCamera( self, amount ):
		'''Depricated.  '''

		self.setCameraOffset( amount )

	def panCameraOffset( self, amount, stepCount ):
		'''Sets the camera in motion to smoothly change the camera offset over stepCount iteration steps.'''

		if ( stepCount < 2 ):
			self.offsetCamera( amount )
			return


		self.deltaOffset = ( ( amount - self.getCameraOffset() ) / stepCount )
		self.offsetting = ( stepCount + 1 )

	def panCameraTarget( self, newTarget, stepCount ):
		'''Sets the camera in motion to smoothly change the camera target over stepCount iteration steps.'''

		if ( stepCount < 2 ):
			self.setCameraTarget( newTarget )
			return


		self.deltaTarget = ( ( newTarget - self.getCameraTarget() ) / stepCount )
		self.offsetting = ( stepCount + 1 )

	def parseXmlNetworkRequest( self, s ):
		return breve.breveInternalFunctionFinder.dearchiveXMLObjectFromString( self, s )

	def pause( self ):
		'''Pauses the simulation as though the user had done so through the user interface.  This method is not supported on all  breve client interfaces.'''

		breve.breveInternalFunctionFinder.pauseSimulation( self)

	def pivotCamera( self, dx = 0.000000, dy = 0.000000 ):
		'''Rotates the camera (from it's current position) on the x-axis by dx and on the y-axis by dy.'''

		rot = breve.vector()

		rot = self.camera.getRotation()
		rot.x = ( rot.x + dx )
		rot.y = ( rot.y + dy )
		self.camera.setRotation( rot.x, rot.y )

	def pointCamera( self, location, offset = breve.vector( 0.000000, 0.000000, 0.000000 ) ):
		'''Points the camera at the vector location.  The optional argument offset specifies the offset of the camera relative to the location target.'''

		self.camTarget = location
		self.setCameraTarget( location )
		if ( breve.length( offset ) != 0.000000 ):
			self.camOffset = offset
			self.setCameraOffset( offset )



	def reportObjectAllocation( self ):
		'''Prints data about current object allocation to the log.'''

		breve.breveInternalFunctionFinder.objectAllocationReport( self)

	def saveAsXml( self, filename ):
		'''Writes the entire state of the world to an XML file, filename.   filename should have one of the following extensions: .xml,  .brevexml, .tzxml. <p> After saving the state of the world as an XML file, you can later start a new run of the same simulation from the saved state.  You will still need the original steve code which generated the file in order to restart the simulation.'''

		breve.breveInternalFunctionFinder.writeXMLEngine( self, filename )

	def saveDepthSnapshot( self, filename, lin, dist ):
		'''Takes a PNG snapshot of the simulation's depth buffer and saves  it to a file named filename, which should end with ".png".'''

		breve.breveInternalFunctionFinder.snapshotDepth( self, filename, lin, dist )

	def saveSnapshot( self, filename ):
		'''Takes a PNG snapshot of the current simulation display and saves  it to a file named filename, which should end with ".png".'''

		breve.breveInternalFunctionFinder.snapshot( self, filename )

	def saveSnapshotToFile( self ):
		self.saveSnapshot( 'simulation.png' )

	def setBackgroundColor( self, newColor ):
		'''Sets the background color of the rendered world to newColor.'''

		self.backgroundColor = newColor
		breve.breveInternalFunctionFinder.setBackgroundColor( self, newColor )

	def setBackgroundScrollRate( self, xValue, yValue ):
		'''Sets the rate of the background image scrolling.  Purely cosmetic.'''

		breve.breveInternalFunctionFinder.setBackgroundScroll( self, xValue, yValue )

	def setBackgroundTexture( self, newTexture ):
		print '''set-background-texture is deprecated, use set-background-texture-image instead'''
		breve.breveInternalFunctionFinder.setBackgroundTexture( self, newTexture )

	def setBackgroundTextureImage( self, newTextureImage ):
		'''Sets the background color of the rendered world to newTexture. newTexture must be a texture returned by the method  METHOD(load-image).  Setting the texture to -1 will turn off  background texturing.'''

		if self.archivedBackgroundTextureImage:
			self.removeDependency( self.archivedBackgroundTextureImage )

		self.archivedBackgroundTextureImage = newTextureImage
		self.addDependency( self.archivedBackgroundTextureImage )
		breve.breveInternalFunctionFinder.setBackgroundTexture( self, newTextureImage.getTextureNumber() )

	def setBlurFactor( self, factor ):
		'''Sets the blur level to factor.  Factor should be a value  between 1.0, which corresponds to the highest blur level,  and 0.0, which corresponds to the lowest blur level.   <p>Blur must first be enabled using the method  METHOD(enable-blur).  Note % that a blur level of 0.0 is  still a minor blur--to disable % blur completely, use the  method METHOD(disable-blur).'''

		self.blurFactor = factor
		breve.breveInternalFunctionFinder.cameraSetBlurFactor( self, self.cameraPointer, factor )

	def setCameraOffset( self, offset ):
		'''Offsets the camera from the target by amount.  The target of the camera remains the same.'''

		breve.breveInternalFunctionFinder.cameraSetOffset( self, offset )

	def setCameraRotation( self, rx, ry ):
		'''Sets the camera rotation on the x-axis to rx and the y-axis to ry.   This method sets the rotation without regard for the current rotation. If you want to offset the camera rotation from the current position,  use the method METHOD(pivot-camera) instead.'''

		self.camera.setRotation( rx, ry )

	def setCameraTarget( self, location ):
		'''Aims the camera at location.  The offset of the camera (the offset from the existing target) stays the same.'''

		breve.breveInternalFunctionFinder.cameraSetTarget( self, location )

	def setDisplayMessage( self, theString, messageNumber, xLoc, yLoc, textColor = breve.vector( 0.000000, 0.000000, 0.000000 ) ):
		'''Sets a text string in the simulation display.  xLoc and yLoc  represent the location of the text.  The coordinate system used goes from (-1, -1) to (1, 1) with (-1, -1) at the lower left hand  corner, (0, 0) in the center of the window and (1, 1) in  the top right hand corner.  The color of the text is set to textColor. <p> Up to 8 messages can be displayed in the simulation window.   messageNumber specifies which message "slot" to modify.   Subsequent calls to this method with the same slot number erase previous entries.'''

		breve.breveInternalFunctionFinder.cameraSetText( self, theString, messageNumber, xLoc, yLoc, textColor )

	def setDisplayText( self, theString, xLoc = -0.950000, yLoc = -0.950000, messageNumber = 0.000000, theColor = breve.vector( 0.000000, 0.000000, 0.000000 ) ):
		'''Sets a text string in the simulation display.  xLoc and yLoc  represent the location of the text.  The coordinate system  used goes from (-1, -1) to (1, 1) with (-1, -1) at the lower  left hand corner, (0, 0) in the center of the window and  (1, 1) in the top right hand corner. <p> The optional argument messageNumber may be used to specify  up to 8 different messages.'''

		breve.breveInternalFunctionFinder.cameraSetText( self, theString, messageNumber, xLoc, yLoc, theColor )

	def setDisplayTextColor( self, textColor ):
		'''Sets the color to the display text to textColor.'''

		breve.breveInternalFunctionFinder.cameraSetTextColor( self, self.cameraPointer, textColor )

	def setDisplayTextScale( self, scale ):
		'''Sets the scaling factor for text in the display window.   See METHOD(set-display-text) and METHOD(set-display-message)  for more information on adding text messages to the display  window.'''

		breve.breveInternalFunctionFinder.cameraSetTextScale( self, scale )

	def setFogColor( self, newColor ):
		'''Sets the fog color to newColor.  Fog must first be turned on with METHOD(enable-lighting) before fog is displayed.'''

		self.fogColor = newColor
		breve.breveInternalFunctionFinder.cameraSetFogColor( self, self.cameraPointer, newColor )

	def setFogIntensity( self, newIntensity ):
		'''Sets the fog intensity to newIntensity.  Fog must first be turned on with METHOD(enable-lighting) before fog is displayed.'''

		self.fogIntensity = newIntensity
		breve.breveInternalFunctionFinder.cameraSetFogIntensity( self, self.cameraPointer, newIntensity )

	def setFogLimits( self, fogStart, fogEnd ):
		'''The calculation which calculates fog needs to know where the fog  starts (the point at which the fog appears) and where the fog ends (the point at which the fog has reached it's highest intensity). <p> This method sets the start value to fogStart and the end value to  fogEnd.  fogStart must be greater than or equal to zero.  fogEnd must be greater than fogStart. <p> Fog must first be turned on with METHOD(enable-lighting) before fog  is displayed.'''

		breve.breveInternalFunctionFinder.setFogDistances( self, fogStart, fogEnd )

	def setIntegrationStep( self, timeStep ):
		'''Sets the integration stepsize to timeStep.  The integration stepsize determines how quickly the simulation runs: large values (perhaps as high as 1 second) mean that the simulation runs quickly at the  cost of accuracy, while low values mean more accuracy, but slower simulations. <p> The control object and its subclasses set the integration timeStep to reasonable values, so this method should only be invoked by expert users with a firm grasp of how they want their simulation to run. <p> Additionally, this value is only used as a suggestion--the  integrator itself may choose to adjust the integration stepsize according to the accuracy of previous timesteps.'''

		self.simStep = timeStep

	def setInterfaceItem( self, tag, newValue ):
		'''This method will set the interface item tag to newValue.  This is for simulations which have an OS X nib file associated with them.'''

		return breve.breveInternalFunctionFinder.setInterfaceString( self, newValue, tag )

	def setIterationStep( self, timeStep ):
		'''Sets the iteration stepsize to timeStep.  The iteration stepsize is simply the number of simulated seconds run between calling the  controller object's "iterate" method. <b>This value may not be smaller than the integration timestep</b>.  <p> The control object and its subclasses set the iteration stepsize  to reasonable values, so this method should only be invoked by expert users with a firm grasp of how they want their simulation to run.  Small values slow down the simulation considerably. <p> For physical simulations, the iteration stepsize should be  considerably larger than the integration stepsize.  The iteration stepsize, in this case, can be interpreted as the reaction time of the agents in the world to changes in their environment.'''

		self.simTime = timeStep
		if ( self.simStep > self.simTime ):
			self.simTime = self.simStep


	def setLightAmbientColor( self, newColor, n = 0.000000 ):
		'''Sets the ambient, or background, color of the light to newColor.   Only has an effect on the rendering when lighting has been turned  on using METHOD(enable-lighting).'''

		self.lightAmbientColor = newColor
		breve.breveInternalFunctionFinder.brISetLightAmbientColor( self, newColor, n )

	def setLightColor( self, newColor, n = 0.000000 ):
		'''Sets the color of the light to newColor.  Only has an effect on the rendering when lighting has been turned on using  METHOD(enable-lighting). <p> This method sets both the ambient and diffuse colors, which can also be set individually with METHOD(set-light-ambient-color) and  METHOD(set-light-diffuse-color).'''

		self.setLightAmbientColor( newColor )
		self.setLightDiffuseColor( newColor )
		self.setLightSpecularColor( newColor )

	def setLightDiffuseColor( self, newColor, n = 0.000000 ):
		'''Sets the diffuse, or foreground, color of the light to newColor.   Only has an effect on the rendering when lighting has been turned  on using METHOD(enable-lighting). <p> The diffuse color is the color coming directly from the light, as opposed to the "ambient" light that is also generated.'''

		self.lightDiffuseColor = newColor
		breve.breveInternalFunctionFinder.brISetLightDiffuseColor( self, newColor, n )

	def setLightExposureSource( self, source ):
		'''Changes the light source for calculating exposure.  See  METHOD(enable-light-exposure-detection).'''

		breve.breveInternalFunctionFinder.setLightExposureSource( self, source )

	def setLightSpecularColor( self, newColor, n = 0.000000 ):
		'''Sets the specular, or "shine", color of the light to newColor.   Only has an effect on the rendering when lighting has been turned  on using METHOD(enable-lighting).'''

		self.lightSpecularColor = newColor
		breve.breveInternalFunctionFinder.brISetLightSpecularColor( self, newColor, n )

	def setOutputFilter( self, filterLevel ):
		'''Sets the output filter level.  This value determines the level of  detail used in printing simulation engine errors and messages. The default value, 0, prints only regular output.  An output filter of 50 will print out all normal output as well as some warnings and  other information useful mostly to breve developers.  Other values may be added in the future to allow for more granularity of error  detail.'''

		breve.breveInternalFunctionFinder.setOutputFilter( self, filterLevel )

	def setRandomSeed( self, newSeed ):
		'''Sets the random seed to newSeed.  Setting the random seed determines the way random numbers are chosen.  Two runs which use the same  random seed will yield the same exact random numbers.  Thus, by setting the random number seed manually, you can make simulations repeatable.'''

		breve.breveInternalFunctionFinder.randomSeed( self, newSeed )

	def setRandomSeedFromDevRandom( self ):
		'''Sets the random seed to a value read from /dev/random (if available). <P> By default, breve sets the random seed based on the current time.  This is generally sufficient for most simulations.  However, if  you are dealing with a setup in which multiple simulations might  be launched simultaneously (such as a cluster setup), then you may  have a situation in which the same random seed would be used for  multiple runs, and this will make you unhappy.  Using this method will restore happiness and harmony to your life.'''

		return breve.breveInternalFunctionFinder.randomSeedFromDevRandom( self)

	def setShadowCatcher( self ):
		if self.lightFlag:
			self.reflectMenu.enable()


		self.shadowCatcherDefined = 1

	def setSkyboxImages( self, imageList ):
		'''Sets a skybox background from a list of images. imageList must contains 6 strings specifying image files corresponding to  the front, back, left, right, top and bottom skybox images respectively. The skybox will not be drawn unless all images load successfully.'''

		self.archivedSkyboxImageList = imageList
		breve.breveInternalFunctionFinder.setSkyboxImages( self, imageList )

	def setZClip( self, theDistance ):
		'''Sets the Z clipping plan to theDistance.  The Z clipping plan  determines how far the camera can see.  A short Z clipping distance means that objects far away will not be drawn.   <p> The default value is 200.0 and this works well for most simulations, so there is often no need to use this method. <p> Using a short Z clipping distance improves drawing quality, avoids  unnecessary rendering and can speed up drawing during the simulation.   However, it may also cause objects you would like to observe in the  simulation to not be drawn because they are too far away.'''

		breve.breveInternalFunctionFinder.setZClip( self, theDistance )

	def showDialog( self, title, message, yesString, noString ):
		'''Shows a dialog box (if supported by the current breve environment) and waits for the user to click on one of the buttons.   <p> If the "yes button" is clicked on, the method returns 1--if the  "no button" is clicked, or if the feature is not supported, 0 is returned.'''

		return breve.breveInternalFunctionFinder.dialogBox( self, title, message, yesString, noString )

	def sleep( self, s ):
		'''Pauses execution for s seconds.  s does not have to be a whole  number.  Sleeping for a fraction of a second at each iteration (inside your controller's iterate method) can effectively slow  down a simulation which is too fast to observe.'''

		breve.breveInternalFunctionFinder.sleep( self, s )

	def stacktrace( self ):
		'''Prints out a breve stacktrace--all of the methods which have been called to get to this point in the simulation.  This method is  useful for debugging.'''

		breve.breveInternalFunctionFinder.stacktrace( self)

	def toggleBlur( self ):
		'''Toggle motion blur for the main camera.  See METHOD(enable-blur) for  more information on reflections.'''

		if ( self.blurFlag == 1 ):
			self.disableBlur()
		else:
			self.enableBlur()


	def toggleFog( self ):
		'''Toggle fog for the main camera'''

		if ( self.fogFlag == 1 ):
			self.disableFog()
		else:
			self.enableFog()


	def toggleLighting( self ):
		'''toggle lighting for the main camera'''

		if ( self.lightFlag == 1 ):
			self.disableLighting()
		else:
			self.enableLighting()


	def toggleRecordingToMovie( self ):
		if self.movie:
			self.movieMenu.uncheck()
			self.movie.close()
			breve.deleteInstances( self.movie )
			return


		self.movie = breve.createInstances( breve.Movie, 1 )
		self.movie.record( 'simulation.mpeg' )
		self.movieMenu.check()

	def toggleReflections( self ):
		'''Toggle reflections for the main camera.  See METHOD(enable-reflections) for  more information on reflections.'''

		if ( not self.shadowCatcherDefined ):
			return

		self.shadowFlag = 1
		if ( self.reflectFlag == 1 ):
			self.disableReflections()
		else:
			self.enableReflections()


	def toggleShadows( self ):
		'''Toggle shadows for the main camera.  See METHOD(enable-shadow-volumes) and  METHOD(disable-shadow-volumes) for more information on shadows.  '''

		if ( self.shadowVolumeFlag == 1 ):
			self.disableShadowVolumes()
		else:
			self.enableShadowVolumes()


	def toggleSmooth( self ):
		'''Toggle smooth drawing for the main camera.  See METHOD(enable-smooth-drawing) and  METHOD(disable-smooth-drawing) for more information.'''

		if ( self.smoothFlag == 1 ):
			self.disableSmoothDrawing()
		else:
			self.enableSmoothDrawing()


	def uniqueColor( self, n ):
		'''Returns a unique color for each different value of n up  to 198.  These colors are allocated according to an  algorithm which attempts to give distinguishable colors, though this is subjective and not always possible.'''

		return breve.breveInternalFunctionFinder.uniqueColor( self, n )

	def unpause( self ):
		'''Pauses the simulation as though the user had done so through the user interface.  This method is not supported on all  breve client interfaces.'''

		breve.breveInternalFunctionFinder.unpauseSimulation( self)

	def updateNeighbors( self ):
		'''The neighborhood for each object in the simulation is the set of  other objects within a specified radius--calling this method will update the neighborhood list for each object.  This method is  only useful in conjunction with methods in OBJECT(Real) which  set the neighborhood size and then later retrieve the neighbor list.'''

		breve.breveInternalFunctionFinder.updateNeighbors( self)

	def watch( self, theObject ):
		'''Points the camera at the OBJECT(Mobile) object theObject.  If  theObject is passed in as 0, then the camera will stop watching a previously watched object.'''

		self.watchObject = theObject

	def zoomCamera( self, theDistance ):
		'''Zooms the camera to theDistance away from the current target-- whether the target is a vector or object.'''

		breve.breveInternalFunctionFinder.cameraSetZoom( self, theDistance )


breve.Control = Control
# Add our newly created classes to the breve namespace

breve.Controls = Control



