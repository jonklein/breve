
import breve

class GS( breve.Control ):
	def __init__( self ):
		breve.Control.__init__( self )
		self.U = None
		self.V = None
		self.cube = None
		self.delta = None
		self.deltareact = None
		self.inflow = None
		self.texture = None
		GS.init( self )

	def init( self ):
		n = 0
		m = 0

		self.setBackgroundColor( breve.vector( 1, 1, 1 ) )
		self.setIterationStep( 1.000000 )
		self.setIntegrationStep( 1.000000 )
		self.pointCamera( breve.vector( 0, 0, 0 ), breve.vector( 0, 0, 70 ) )
		self.U = breve.createInstances( breve.Matrix2D, 1 )
		self.V = breve.createInstances( breve.Matrix2D, 1 )
		self.inflow = breve.createInstances( breve.Matrix2D, 1 )
		self.delta = breve.createInstances( breve.Matrix2D, 1 )
		self.deltareact = breve.createInstances( breve.Matrix2D, 1 )
		self.U.setSize( 128, 128 )
		self.V.setSize( 128, 128 )
		self.delta.setSize( 128, 128 )
		self.deltareact.setSize( 128, 128 )
		self.inflow.setSize( 128, 128 )
		self.texture = breve.createInstances( breve.MatrixImage, 1 )
		self.texture.initWith( self.U, 1.000000 )
		self.texture.setRed( self.V )
		self.texture.setBlue( self.U )
		self.cube = breve.createInstances( breve.Mobile, 1 )
		self.cube.setShape( breve.createInstances( breve.Cube, 1 ).initWith( breve.vector( 400, 400, 1 ) ) )
		self.cube.setTextureImage( self.texture )
		self.cube.setTextureScale( 40 )
		n = 0
		while ( n < 128 ):
			m = 0
			while ( m < 128 ):
				self.U.setValue( ( ( 0.500000 + breve.breveInternalFunctionFinder.sqrt( self, breve.length( ( 0.250000 - ( ( 0.010000 * ( 1.000000 + ( 0.040000 / 0.010000 ) ) ) * ( 1.000000 + ( 0.040000 / 0.010000 ) ) ) ) ) ) ) + ( 0.020000 * ( breve.randomExpression( 1.000000 ) - 0.500000 ) ) ), m, n )
				self.V.setValue( ( ( ( 1.000000 - self.U.getValue( m, n ) ) / ( 1.000000 + ( 0.040000 / 0.010000 ) ) ) + ( 0.020000 * ( breve.randomExpression( 1.000000 ) - 0.500000 ) ) ), m, n )
				self.inflow.setValue( 0.010000, n, m )

				m = ( m + 1 )

			n = ( n + 1 )


	def iterate( self ):

		self.deltareact.copy( self.U )
		self.deltareact.multiplyWithValues( self.V )
		self.deltareact.multiplyWithValues( self.V )
		self.delta.computePeriodicDiffusionMatrix( self.U, 0.078000 )
		self.delta.addValues( self.deltareact, -1.000000 )
		self.delta.addValues( self.U, ( -0.010000 ) )
		self.delta.addValues( self.inflow )
		self.U.addValues( self.delta, 1.000000 )
		self.delta.computePeriodicDiffusionMatrix( self.V, 0.022000 )
		self.delta.addValues( self.deltareact )
		self.delta.addValues( self.V, ( -( 0.040000 + 0.010000 ) ) )
		self.V.addValues( self.delta, 1.000000 )
		breve.Control.iterate( self )


breve.GS = GS


# Create an instance of our controller object to initialize the simulation

GS()


