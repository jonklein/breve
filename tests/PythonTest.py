import breve

class PythonTestObject( breve.real ):
	def __init__( self ):
		breve.real.__init__( self )

		print "Creating PythonTest instance ", self

		self.movie = self.controller.make_new_instance( "PushProgram" )

	def iterate( self ):
		if self.controller.get_time() > 8.0:
			self.movie.close()

# breve.SetController( PythonTestObject() )

