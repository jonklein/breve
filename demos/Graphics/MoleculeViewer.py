
# Note: this file was automatically converted to Python from the
# original steve-language source code.  Please see the original
# file for more detailed comments and documentation.


import breve

class PDBViewer( breve.Control ):
	def __init__( self ):
		breve.Control.__init__( self )
		self.atom = None
		self.file = None
		self.hShape = None
		self.location = breve.vector()
		PDBViewer.init( self )

	def init( self ):
		fields = breve.objectList()
		average = breve.vector()
		atomString = ''

		self.setBackgroundColor( breve.vector( 0.600000, 0.600000, 0.600000 ) )
		self.enableLighting()
		self.hShape = breve.createInstances( breve.Sphere, 1 ).initWith( 0.200000 )
		self.file = breve.createInstances( breve.File, 1 )
		if ( self.getArgumentCount() > 1 ):
			self.file.openForReading( self.getArgument( 1 ) )

		else:
			self.file.openForReading( 'DNA.pdb' )


		while ( breve.length( fields = self.file.readLineAsWhitespaceDelimitedList() ) != 0 ):
			if ( fields[ 0 ] == 'ATOM' ):
				if ( breve.length( fields ) == 10 ):
					atomString = fields[ 9 ]
					self.location = breve.vector( fields[ 5 ], fields[ 6 ], fields[ 7 ] )

				else:
					atomString = fields[ 11 ]
					self.location = breve.vector( fields[ 6 ], fields[ 7 ], fields[ 8 ] )


				average = ( average + self.location )
				self.atom = breve.createInstances( breve.Mobile, 1 )
				self.atom.move( self.location )
				if ( atomString == 'C' ):
					self.atom.setColor( breve.vector( 0, 0, 0 ) )

				if ( atomString == 'O' ):
					self.atom.setColor( breve.vector( 0, 0, 1 ) )

				if ( atomString == 'N' ):
					self.atom.setColor( breve.vector( 0, 1, 0 ) )

				if ( atomString == 'P' ):
					self.atom.setColor( breve.vector( 1, 0, 0 ) )

				if ( atomString == 'H' ):
					self.atom.setShape( self.hShape )





		average = ( average / breve.length( breve.allInstances( "Mobiles" ) ) )
		print breve.length( breve.allInstances( "Mobiles" ) ), ''' atoms'''
		self.pointCamera( average, ( average + breve.vector( 30, 0, 30 ) ) )

	def iterate( self ):
		self.pivotCamera( 0.030000, 0.000000 )
		breve.Control.iterate( self )


breve.PDBViewer = PDBViewer


# Create an instance of our controller object to initialize the simulation

PDBViewer()


