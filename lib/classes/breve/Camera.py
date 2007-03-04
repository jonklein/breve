
import breve

class Camera( breve.Abstract ):
	'''Summary: creates a new rendering perspective in the simulated world. <P> The Camera class is used to set up a viewing perspective in a simulation. Creating a new camera object places a viewing area with the new camera  perspective in the main viewing window. <P> See the OBJECT(Image) class to read data from a Camera (or from the  main simulation window) into a pixel buffer.  This can be useful for implementing vision algorithms.'''

	__slots__ = [ 'cameraPointer', 'shared', ]

	def __init__( self ):
		breve.Abstract.__init__( self )
		self.cameraPointer = None
		self.shared = 0
		Camera.init( self )

	def delete( self ):
		''''''


		if ( self.cameraPointer and ( not self.shared ) ):
			breve.breveInternalFunctionFinder.cameraFree( self, self.cameraPointer )


	def disable( self ):
		'''Disables this camera.  The view from this camera will not be  updated or drawn to the viewing window. '''


		breve.breveInternalFunctionFinder.cameraSetEnabled( self, self.cameraPointer, 0 )

	def disableSmoothDrawing( self ):
		'''Disable smooth drawing for the main camera.   See METHOD(enable-smooth-drawing) for more information.'''


		breve.breveInternalFunctionFinder.cameraSetDrawSmooth( self, self.cameraPointer, 0 )

	def disableText( self ):
		'''Disables text for this camera.'''


		breve.breveInternalFunctionFinder.cameraTextSetEnabled( self, self.cameraPointer, 0 )

	def enable( self ):
		'''Enables the camera.  The view from this camera will be updated and drawn to the viewing window after each iteration.'''


		breve.breveInternalFunctionFinder.cameraSetEnabled( self, self.cameraPointer, 1 )

	def enableSmoothDrawing( self ):
		'''Enable smooth drawing for the camera.  Smooth drawing enables a smoother blending of colors, textures and lighting.  This feature is especially noticeable when dealing with spheres or large objects. <p> The disadvantage of smooth drawing is a potential performance hit. The degree of this performance hit depends on the number of polygons in the scene.  If speed is an issue, it is often best to disable both lighting and smooth drawing.'''


		breve.breveInternalFunctionFinder.cameraSetDrawSmooth( self, self.cameraPointer, 1 )

	def enableText( self ):
		'''Enables text for this camera.'''


		breve.breveInternalFunctionFinder.cameraTextSetEnabled( self, self.cameraPointer, 1 )

	def getHeight( self ):
		'''Returns the current camera width.'''


		return breve.breveInternalFunctionFinder.cameraGetHeight( self, self.cameraPointer )


	def getRotation( self ):
		'''Returns a vector containing the rotation of the camera about the X-  and Y-axes return cameraGetRotation(cameraPointer).'''


		return breve.breveInternalFunctionFinder.cameraGetRotation( self, self.cameraPointer )


	def getWidth( self ):
		'''Returns the current camera width.'''


		return breve.breveInternalFunctionFinder.cameraGetWidth( self, self.cameraPointer )


	def init( self ):
		''''''


		self.cameraPointer = breve.breveInternalFunctionFinder.cameraNew( self)
		self.setSize( 100, 100 )
		self.setPosition( 0, 0 )

	def look( self, target, position ):
		'''Moves the camera to position and aims it at target.  target is is the target's location <b>relative to the camera</b>, not the target's "real-world" location.'''


		breve.breveInternalFunctionFinder.cameraPosition( self, self.cameraPointer, position, target )

	def setCameraPointer( self, p ):
		'''Used internally.'''


		if ( not self.shared ):
			breve.breveInternalFunctionFinder.cameraFree( self, self.cameraPointer )

		self.cameraPointer = p
		self.shared = 1

	def setPosition( self, newX, newY ):
		'''Sets the position of the camera viewing area inside the main window.'''


		breve.breveInternalFunctionFinder.cameraPositionDisplay( self, self.cameraPointer, newX, newY )

	def setRotation( self, rx, ry ):
		'''Sets the rotation of the camera about the X- and Y-axes.'''


		breve.breveInternalFunctionFinder.cameraSetRotation( self, self.cameraPointer, rx, ry )

	def setSize( self, newHeight, newWidth ):
		'''Sets the size of the camera viewing area.'''


		breve.breveInternalFunctionFinder.cameraResizeDisplay( self, self.cameraPointer, newWidth, newHeight )

	def setZClip( self, distance ):
		'''Sets the Z clipping plan to theDistance.  The Z clipping plan determines how far the camera can see.  A short Z clipping distance means that objects far away will not be drawn. <p> The default value is 500.0 and this works well for most simulations, so there is often no need to use this method. <p> Using a short Z clipping distance improves drawing quality, avoids unnecessary rendering and can speed up drawing during the simulation. However, it may also cause objects you would like to observe in the simulation to not be drawn because they are too far away.'''


		breve.breveInternalFunctionFinder.cameraSetZClip( self, self.cameraPointer, distance )


breve.Camera = Camera
breve.Cameras = Camera



