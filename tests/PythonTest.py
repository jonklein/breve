import breve

class PythonTestObject( breve.real ):
	def __init__( self ):
		breve.real.__init__( self )

		print "Creating PythonTest instance ", self

		self.movie = self.controller.make_new_instance( "Movie" )

		self.movie.record( "sim.mpg" )

	def argumentTest( self, i, f, l ):
		print i
		print f
		print l
		return 3.14

	def iterate( self ):
		print self.controller.get_time()

		if self.controller.get_time() > 8.0:
			print "The end"
			self.movie.close()
			self.controller.end_simulation()

