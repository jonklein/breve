
import breve

class SpringController( breve.PhysicalControl ):
	def __init__( self ):
		breve.PhysicalControl.__init__( self )
		self.NODES = 0
		self.nodes = breve.objectList()
		SpringController.init( self )

	def init( self ):
		x = 0
		y = 0
		z = 0
		startblock = 0
		endblock = 0
		numsprings = 0
		NUMEDGESONSIDE = 0
		p1 = 0
		p2 = 0
		xoff1 = 0
		yoff1 = 0
		zoff1 = 0
		xoff2 = 0
		yoff2 = 0
		zoff2 = 0
		complength = 0

		NUMEDGESONSIDE = ( 3 - 1 )
		self.NODES = ( ( 3 * 3 ) * 3 )
		self.setBackgroundColor( breve.vector( 0.700000, 0.700000, 0.700000 ) )
		self.pointCamera( breve.vector( 0, 10, 0 ), breve.vector( 70, 20, 70 ) )
		self.enableFastPhysics()
		self.setFastPhysicsIterations( 5 )
		self.nodes = breve.createInstances( breve.Nodes, self.NODES )
		self.nodes.enablePhysics()
		x = 0
		while ( x < 3 ):
			y = 0
			while ( y < 3 ):
				z = 0
				while ( z < 3 ):
					startblock = ( ( x + ( y * 3 ) ) + ( ( z * 3 ) * 3 ) )
					self.nodes[ startblock ].move( breve.vector( ( ( x * 9 ) - ( ( 3 / 2 ) * 9 ) ), ( ( y * 9 ) + 60 ), ( ( z * 9 ) - ( ( 3 / 2 ) * 9 ) ) ) )

					z = ( z + 1 )


				y = ( y + 1 )


			x = ( x + 1 )

		self.setGravity( breve.vector( 0, -9.800000, 0 ) )
		breve.createInstances( breve.Stationary, 1 ).register( breve.createInstances( breve.Cube, 1 ).initWith( breve.vector( 1200, 1, 1200 ) ) )
		x = 0
		while ( x < NUMEDGESONSIDE ):
			y = 0
			while ( y < NUMEDGESONSIDE ):
				z = 0
				while ( z < NUMEDGESONSIDE ):
					p1 = 0
					while ( p1 < 7 ):
						p2 = ( p1 + 1 )
						while ( p2 < 8 ):
							xoff1 = ( p1 % 2 )
							yoff1 = ( ( p1 / 2 ) % 2 )
							if ( p1 < 4 ):
								zoff1 = 0

							else:
								zoff1 = 1


							xoff2 = ( p2 % 2 )
							yoff2 = ( ( p2 / 2 ) % 2 )
							if ( p2 < 4 ):
								zoff2 = 0

							else:
								zoff2 = 1


							complength = ( breve.breveInternalFunctionFinder.sqrt( self, ( ( ( ( xoff1 - xoff2 ) ** 2 ) + ( ( yoff1 - yoff2 ) ** 2 ) ) + ( ( zoff1 - zoff2 ) ** 2 ) ) ) * 9 )
							if ( complength > 0 ):
								startblock = ( ( ( x + xoff1 ) + ( ( y + yoff1 ) * 3 ) ) + ( ( ( z + zoff1 ) * 3 ) * 3 ) )
								endblock = ( ( ( x + xoff2 ) + ( ( y + yoff2 ) * 3 ) ) + ( ( ( z + zoff2 ) * 3 ) * 3 ) )
								breve.createInstances( breve.Spring, 1 ).connect( self.nodes[ startblock ], self.nodes[ endblock ], breve.vector( 0, 0, 0 ), breve.vector( 0, 0, 0 ), complength, 5 )
								numsprings = ( numsprings + 1 )



							p2 = ( p2 + 1 )


						p1 = ( p1 + 1 )


					z = ( z + 1 )


				y = ( y + 1 )


			x = ( x + 1 )

		self.nodes[ 0 ].setVelocity( breve.vector( 100, 0, 0 ) )
		print '''NumSprings = %s''' % (  numsprings )


breve.SpringController = SpringController
class Node( breve.Link ):
	def __init__( self ):
		breve.Link.__init__( self )
		Node.init( self )

	def init( self ):

		self.setShape( breve.createInstances( breve.Cube, 1 ).initWith( breve.vector( 0.500000, 0.500000, 0.500000 ) ) )

	def iterate( self ):

		self.setRotationalVelocity( ( 0.950000 * self.getRotationalVelocity() ) )


breve.Node = Node
# Add our newly created classes to the breve namespace

breve.Nodes = Node


# Create an instance of our controller object to initialize the simulation

SpringController()


