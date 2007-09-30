
# Note: this file was automatically converted to Python from the
# original steve-language source code.  Please see the original
# file for more detailed comments and documentation.


import breve

class myController( breve.Control ):
	def __init__( self ):
		breve.Control.__init__( self )
		self.acc = 0
		self.blur = 0
		self.file = ''
		self.image = None
		self.radius = 0
		self.shape = None
		self.vel = 0
		self.zoom = 0
		myController.init( self )

	def catchKeySDown( self ):
		self.saveSnapshotToFile()

	def getAcc( self ):
		return self.acc

	def getImage( self ):
		return self.image

	def getShape( self ):
		return self.shape

	def getVel( self ):
		return self.vel

	def init( self ):
		brush = None

		self.disableText()
		self.vel = ( 1.000000 + breve.randomExpression( 9 ) )
		self.acc = breve.randomExpression( 1000 )
		self.radius = ( 1.000000 + breve.randomExpression( 5 ) )
		if ( self.getArgumentCount() == 2 ):
			self.file = self.getArgument( 1 )

		else:
			self.file = 'images/jon.png'

		self.image = breve.createInstances( breve.Image, 1 ).load( self.file )
		self.blur = 0.992000
		if ( self.blur > 0.992000 ):
			self.blur = 0.992000

		self.blur = 1.000000
		self.setBackgroundColor( breve.vector( 0, 0, 0 ) )
		self.enableBlur()
		self.setBlurFactor( self.blur )
		self.radius = ( self.radius * ( self.image.getWidth() / 300.000000 ) )
		self.vel = ( self.vel * ( self.image.getWidth() / 300.000000 ) )
		self.acc = ( self.acc * ( self.image.getWidth() / 300.000000 ) )
		self.shape = breve.createInstances( breve.Sphere, 1 ).initWith( self.radius )
		brush = breve.createInstances( breve.Image, 1 ).load( 'images/brush.png' )
		breve.createInstances( breve.Painters, 200 ).setBitmapImage( brush )
		self.zoom = ( self.image.getWidth() + 10 )
		self.offsetCamera( breve.vector( 0, 0, self.zoom ) )
		self.setZClip( ( self.zoom + 100 ) )

	def iterate( self ):
		if ( self.zoom < ( self.image.getWidth() + 10 ) ):
			self.zoom = ( self.zoom + 0.350000 )
			self.offsetCamera( breve.vector( 0, 0, self.zoom ) )
			self.setZClip( ( self.zoom + 100 ) )


		breve.Control.iterate( self )


breve.myController = myController
class Painter( breve.Mobile ):
	def __init__( self ):
		breve.Mobile.__init__( self )
		self.acc = 0
		self.h = 0
		self.image = None
		self.vel = 0
		self.w = 0
		Painter.init( self )

	def init( self ):
		self.setShape( self.controller.getShape() )
		self.image = self.controller.getImage()
		self.w = self.image.getWidth()
		self.h = self.image.getHeight()
		self.setColor( breve.vector( 0, 0, 0 ) )
		self.reset()

	def iterate( self ):
		velocity = breve.vector()

		if ( breve.randomExpression( 20 ) == 0 ):
			self.reset()

		velocity = self.getVelocity()
		if breve.length( velocity ):
			velocity = ( velocity / breve.length( velocity ) )

		self.setVelocity( ( self.vel * velocity ) )
		self.setAcceleration( ( breve.randomExpression( breve.vector( self.acc, self.acc, 0 ) ) - breve.vector( ( self.acc / 2 ), ( self.acc / 2 ), 0 ) ) )

	def reset( self ):
		self.vel = self.controller.getVel()
		self.acc = self.controller.getAcc()
		self.move( ( breve.randomExpression( breve.vector( ( self.w - 1 ), ( self.h - 1 ), 0 ) ) - breve.vector( ( self.w / 2 ), ( self.h / 2 ), 0 ) ) )
		self.setVelocity( ( breve.randomExpression( breve.vector( self.vel, self.vel, 0 ) ) - breve.vector( ( self.vel / 2 ), ( self.vel / 2 ), 0 ) ) )
		self.setColor()

	def setColor( self ):
		z = 0
		y = 0
		x = 0
		pixelcolor = breve.vector()
		location = breve.vector()

		location = self.getLocation()
		x = location.x
		y = location.y
		z = location.z
		if ( ( ( ( ( x > ( ( self.w / 2 ) - 1 ) ) or ( x < ( ( -self.w ) / 2 ) ) ) or ( y > ( ( self.h / 2 ) - 1 ) ) ) or ( y < ( ( -self.h ) / 2 ) ) ) or ( z > 200 ) ):
			self.reset()
			location = self.getLocation()
			x = location.x
			y = location.y


		x = ( x + ( self.w / 2 ) )
		y = ( y + ( self.h / 2 ) )
		pixelcolor = self.image.getRgbPixel( x, y )
		self.setColor( pixelcolor )


breve.Painter = Painter
# Add our newly created classes to the breve namespace

breve.Painters = Painter


# Create an instance of our controller object to initialize the simulation

myController()


