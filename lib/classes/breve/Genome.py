
# Note: this file was automatically converted to Python from the
# original steve-language source code.  Please see the original
# file for more detailed comments and documentation.


import breve

class Genome( breve.Data ):
	'''The Genome class is a subclass of OBJECT(Data) which allows its subclasses to be manipulated with genetic operators such as crossover and mutation. This class is primarily used in conjunction with genetic algorithms and  artificial life simulations. <p> As with its parent class, Data, the Genome class is incomplete and experimental.   The API is subject to change in future releases. '''

	def __init__( self ):
		breve.Data.__init__( self )

	def crossover( self, parent1, parent2 ):
		'''Initializes the contents of this object using a 1-point crossover from  parent1 and parent2.'''

		if ( ( not parent1 ) or ( not parent2 ) ):
			raise Exception( '''Parent object passed to crossover not initialized''' )


		breve.breveInternalFunctionFinder.simpleCrossover( self, parent1, parent2, self )


breve.Genome = Genome
# Add our newly created classes to the breve namespace

breve.Genomes = Genome



