
import breve

class GS( breve.Control ):
	def __init__( self ):
		breve.Control.__init__( self )
		self.chemicals = breve.objectList()
		self.colors = breve.objectList()
		self.cube = None
		self.diffusion = None
		self.inflow = None
		self.reactionRates = breve.objectList()
		self.reactions = breve.objectList()
		self.rgb = breve.objectList()
		self.temp = None
		self.tempreact = None
		self.texture = None
		self.totalConcentration = None
		GS.init( self )

	def init( self ):
		x = 0
		c = 0
		n = 0
		m = 0

		self.colors.append( breve.vector( 1.000000, 0, 0 ) )
		self.colors.append( breve.vector( 0.500000, 0, 0.500000 ) )
		self.colors.append( breve.vector( 0, 0, 0.500000 ) )
		self.colors.append( breve.vector( 0, 0.500000, 0.500000 ) )
		self.colors.append( breve.vector( 1, 1, 1 ) )
		self.colors.append( breve.vector( 1, 1, 0 ) )
		self.reactionRates = [ [ 0, 23, 0, 0, 0, 0 ], [ 0, 0, 23, 0, 0, 0 ], [ 0, 0, 0, 23, 0, 0 ], [ 0, 0, 0, 0, 23, 0 ], [ 0, 0, 0, 0, 0, 23 ], [ 23, 0, 0, 0, 0, 0 ] ]
		self.chemicals = breve.createInstances( breve.Matrix2D, 6 )
		self.chemicals.setSize( 128, 128 )
		self.diffusion = breve.createInstances( breve.Matrix2D, 1 )
		self.diffusion.setSize( 128, 128 )
		self.reactions = breve.createInstances( breve.Matrix2D, 6 )
		self.reactions.setSize( 128, 128 )
		self.rgb = breve.createInstances( breve.Matrix2D, 3 )
		self.rgb.setSize( 128, 128 )
		self.totalConcentration = breve.createInstances( breve.Matrix2D, 1 )
		self.totalConcentration.setSize( 128, 128 )
		self.setBackgroundColor( breve.vector( 0, 0, 0 ) )
		self.setIterationStep( 0.250000 )
		self.setIntegrationStep( 0.250000 )
		self.pointCamera( breve.vector( 0, 0, 0 ), breve.vector( 0, 0, 80 ) )
		x = 0
		while ( x < 3000 ):
			c = breve.randomExpression( ( 6 - 1 ) )
			n = ( 1 + breve.randomExpression( ( 128 - 3 ) ) )
			m = ( 1 + breve.randomExpression( ( 128 - 3 ) ) )
			self.chemicals[ c ].setValue( n, m, 0.200000 )
			self.chemicals[ c ].setValue( n, ( m + 1 ), 0.200000 )
			self.chemicals[ c ].setValue( n, ( m - 1 ), 0.200000 )
			self.chemicals[ c ].setValue( ( n + 1 ), m, 0.200000 )
			self.chemicals[ c ].setValue( ( n - 1 ), m, 0.200000 )

			x = ( x + 1 )

		self.texture = breve.createInstances( breve.MatrixTextureImage, 1 )
		self.texture.initWith( 128, 128 )
		self.texture.setR( self.rgb[ 0 ] )
		self.texture.setG( self.rgb[ 1 ] )
		self.texture.setB( self.rgb[ 2 ] )
		self.cube = breve.createInstances( breve.Mobile, 1 )
		self.cube.setShape( breve.createInstances( breve.Cube, 1 ).initWith( breve.vector( 60, 60, 1 ) ) )
		self.cube.setTextureImage( self.texture )
		self.cube.setTextureScale( 60 )

	def iterate( self ):
		m = 0
		n = 0
		sum = 0

		self.totalConcentration.setAllValues( 1.000000 )
		n = 0
		while ( n < 6 ):
			self.totalConcentration.subtractValues( self.chemicals[ n ] )
			self.reactions[ n ].setAllValues( 2.000000 )
			m = 0
			while ( m < 6 ):
				if ( self.reactionRates[ m ][ n ] != 0.000000 ):
					self.reactions[ n ].addValues( self.chemicals[ m ], self.reactionRates[ m ][ n ] )



				m = ( m + 1 )


			n = ( n + 1 )

		self.rgb.setAllValues( 0.000000 )
		n = 0
		while ( n < 6 ):
			self.reactions[ n ].multiplyWithValues( self.chemicals[ n ] )
			self.reactions[ n ].multiplyWithValues( self.totalConcentration )
			self.reactions[ n ].subtractValues( self.chemicals[ n ], 1.000000 )
			self.diffusion.computeDiffusionMatrix( self.chemicals[ n ] )
			self.reactions[ n ].addValues( self.diffusion, 0.100000 )
			self.chemicals[ n ].addValues( self.reactions[ n ], 0.250000 )
			self.rgb[ 0 ].addValues( self.chemicals[ n ], self.colors[ n ].x )
			self.rgb[ 1 ].addValues( self.chemicals[ n ], self.colors[ n ].y )
			self.rgb[ 2 ].addValues( self.chemicals[ n ], self.colors[ n ].z )

			n = ( n + 1 )

		breve.Control.iterate( self )


breve.GS = GS
class MatrixTextureImage( breve.Image ):
	def __init__( self ):
		breve.Image.__init__( self )
		self.b = None
		self.g = None
		self.n = 0
		self.r = None

	def iterate( self ):

		self.r.copyToRedChannel( self )
		self.g.copyToGreenChannel( self )
		self.b.copyToBlueChannel( self )
		breve.Image.iterate( self )

	def setB( self, m ):

		self.b = m

	def setG( self, m ):

		self.g = m

	def setR( self, m ):

		self.r = m


breve.MatrixTextureImage = MatrixTextureImage


# Create an instance of our controller object to initialize the simulation

GS()


