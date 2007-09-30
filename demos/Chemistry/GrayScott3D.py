
# Note: this file was automatically converted to Python from the
# original steve-language source code.  Please see the original
# file for more detailed comments and documentation.


import breve

class GS( breve.Control ):
	def __init__( self ):
		breve.Control.__init__( self )
		self.U = None
		self.V = None
		self.grid = None
		self.inflow = None
		self.temp = None
		self.tempreact = None
		GS.init( self )

	def init( self ):
		n = 0
		m = 0
		k = 0
		j = 0
		i = 0

		self.setBackgroundColor( breve.vector( 1, 1, 1 ) )
		self.setIterationStep( 1.000000 )
		self.setIntegrationStep( 1.000000 )
		self.pointCamera( breve.vector( 0, 0, 0 ), breve.vector( 30, 6, -30 ) )
		self.U = breve.createInstances( breve.Matrix3D, 1 )
		self.V = breve.createInstances( breve.Matrix3D, 1 )
		self.inflow = breve.createInstances( breve.Matrix3D, 1 )
		self.temp = breve.createInstances( breve.Matrix3D, 1 )
		self.tempreact = breve.createInstances( breve.Matrix3D, 1 )
		self.U.setSize( 32, 32, 32 )
		self.V.setSize( 32, 32, 32 )
		self.temp.setSize( 32, 32, 32 )
		self.tempreact.setSize( 32, 32, 32 )
		self.inflow.setSize( 32, 32, 32 )
		self.grid = breve.createInstances( breve.PatchGrid, 1 )
		self.grid.initWith( breve.vector( 0.500000, 0.500000, 0.500000 ), 32, 32, 32 )
		self.grid.enableSmoothDrawing()
		self.inflow.setAllValues( 0.010000 )
		i = 0
		while ( i < 32 ):
			n = 0
			while ( n < 32 ):
				m = 0
				while ( m < 32 ):
					self.U.setValue( ( ( 0.500000 + breve.breveInternalFunctionFinder.sqrt( self, breve.length( ( 0.250000 - ( ( 0.010000 * ( 1.000000 + ( 0.040000 / 0.010000 ) ) ) * ( 1.000000 + ( 0.040000 / 0.010000 ) ) ) ) ) ) ) + ( 0.010000 * ( breve.randomExpression( 1.000000 ) - 0.500000 ) ) ), m, n, i )
					self.V.setValue( ( ( ( 1.000000 - self.U.getValue( m, n, i ) ) / ( 1.000000 + ( 0.040000 / 0.010000 ) ) ) + ( 0.010000 * ( breve.randomExpression( 1.000000 ) - 0.500000 ) ) ), m, n, i )

					m = ( m + 1 )

				n = ( n + 1 )

			i = ( i + 1 )

		breve.allInstances( "Patches" ).setTransparency( 0.900000 )

	def iterate( self ):
		self.tempreact.copy( self.U )
		self.tempreact.multiplyWithValues( self.V )
		self.tempreact.multiplyWithValues( self.V )
		self.temp.computePeriodicDiffusionMatrix( self.U, 0.058000 )
		self.temp.addValues( self.tempreact, -1.000000 )
		self.temp.addValues( self.U, ( -0.010000 ) )
		self.temp.addValues( self.inflow )
		self.U.addValues( self.temp, 1.000000 )
		self.temp.computePeriodicDiffusionMatrix( self.V, 0.022000 )
		self.temp.addValues( self.tempreact )
		self.temp.addValues( self.V, ( -( 0.040000 + 0.010000 ) ) )
		self.V.addValues( self.temp, 1.000000 )
		self.V.copyToRedChannel( self.grid, 2.000000 )
		self.temp.copy( self.V )
		self.temp.multiplyWithValues( self.V )
		self.temp.copyToAlphaChannel( self.grid, 0.500000 )
		breve.Control.iterate( self )


breve.GS = GS


# Create an instance of our controller object to initialize the simulation

GS()


