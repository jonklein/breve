
# Note: this file was automatically converted to Python from the
# original steve-language source code.  Please see the original
# file for more detailed comments and documentation.


import breve

class PushGP( breve.Object ):
	'''This class is a special control class used to implement  experiments using the Push Genetic Programming (PushGP) system.  PushGP is a system which uses genetic programming and the Push programming language to attempt to evolve  solutions to a wide variety of problems.  For more  information on Push and PushGP, see the class OBJECT(PushInterpreter). <p> This class is appropriate for evolving Push programs which are only run once per fitness case. <p> To implement a PushGP experiment you will need to setup a PushGP object which overrides two methods: METHOD(setup-fitness-test), which sets up a fitness test and METHOD(compute-error), which is used to compute the  fitness of an individual. <p> Additionally, other methods in this class are useful for configuring the genetic programming environment and a  variety of parameters including population size,  mutation rate, crossover rate, and the Push language 	 interpreter set. <p> Finally, you may also wish to add custom instructions  to the Push interpreter objects which manipulate data or trigger actions in the simulated world.  The Push interpreter can be retrieved using the method  METHOD(get-interpreter), and adding instructions is  documented in OBJECT(PushInterpreter).'''

	def __init__( self ):
		breve.Object.__init__( self )
		self._abortFitnessTest = 0
		self._abortFitnessValue = 0
		self._bestOfGen = None
		self._solutionCountsGeneration = breve.objectList()
		self._solutionCountsTotal = breve.objectList()
		self._solutionRatesGeneration = breve.objectList()
		self._solutionRatesTotal = breve.objectList()
		self._totalErrors = 0
		self._totalSize = 0
		self._useChallenges = 0
		self.crossoverPercent = 0
		self.current = breve.objectList()
		self.currentFitnessTest = 0
		self.currentIndividual = 0
		self.deletionPercent = 0
		self.fitnessTestDuration = 0
		self.generation = 0
		self.generationLimit = 0
		self.inited = 0
		self.interpreter = None
		self.mutationPercent = 0
		self.next = breve.objectList()
		self.pop1 = breve.objectList()
		self.pop2 = breve.objectList()
		self.populationSize = 0
		self.seed = ''
		self.spatialRadius = 0
		self.tournamentSize = 0
		PushGP.init( self )

	def abortFitnessTest( self, fitness = 99999999.000000 ):
		'''Aborts the currently running fitness '''

		self._abortFitnessValue = fitness
		self._abortFitnessTest = 1

	def autoSimplify( self, p ):
		'''Programs generated through genetic programming are often complicated and  difficult to understand.  This method attempts to simplify an evolved   push program while maintaining the same fitness. <P> Each time this method is invoked, a random simplification is preformed  on the program p.  If the simplification does not harm the fitness, p is modified to contain the simplification, otherwise, nothing happens. This method should be called repeatedly -- perhaps 100 times or more  to aggressively simplify an unweildy program.'''

		newFitness = 0
		fitness = 0
		newProgram = None

		fitness = self.computeZeroDurationFitness( p )
		newProgram = breve.createInstances( breve.PushGPIndividual, 1 )
		newProgram.copyFrom( p )
		if breve.randomExpression( 1 ):
			newProgram.deleteRandomSubtree()
		else:
			newProgram.flattenRandomSubtree()

		newFitness = self.computeZeroDurationFitness( newProgram )
		if ( newFitness == fitness ):
			p.copyFrom( newProgram )

		breve.deleteInstances( newProgram )
		return p

	def computeDiversity( self, t ):
		'''Computes the diversity of the current population, as defined by  the OBJECT(PushDiversityPool) object.  Returns the number of  unique "species" which differ by less than t points.'''

		pool = None
		n = 0

		pool = breve.createInstances( breve.PushDiversityPool, 1 )
		pool.setTolerance( t )
		n = 0
		while ( n < breve.length( self.current ) ):
			pool.add( self.current[ n ] )
			n = ( n + 1 )

		n = pool.getSize()
		breve.deleteInstances( pool )
		return n

	def computeError( self, n, interpreter ):
		'''This method must be overridden by the user in order to run a  PushGP experiment.  The job of this method is to inspect the  state of the OBJECT(PushInterpreter) object and determine the fitness (in terms of error) of the specified fitness case.   This typically involves comparing values from the interpreter's  stacks against desired values. <p> For example, with a symbolic regression problem, this method  might return the difference between the top of the float stack,  and the predetermined "correct" answer for the specified fitness  case.  For example: <pre> return (interpreter get-float-stack-top) - desiredOutputs{ n }. </pre>'''

		raise Exception( '''The method compute-error must be overridden by the user -- see the class documentation for PushGPControl for more information''' )


	def computeErrorVectorFitness( self, errorList ):
		'''This method takes a list of errors returned by METHOD(compute-error) for all  fitness cases, and produces a fitness value.  The default implementation of  this method computes the fitness by summing the absolute values of the errors, which should suffice for most applications. <P> Overriding this method is optional.'''

		value = 0
		fitness = 0

		for value in errorList:
			fitness = ( fitness + breve.length( value ) )

		return fitness

	def computeZeroDurationFitness( self, p ):
		n = 0
		errors = breve.objectList()
		errorValue = 0

		self._abortFitnessTest = 0
		n = 0
		while ( n < self.countFitnessCases() ):
			self.interpreter.clearStacks()
			self.setupFitnessTest( n, self.interpreter )
			if self.interpreter.run( p ):
				errorValue = ( self.getProgramLimitPenalty() + self.computeError( n, self.interpreter ) )

			else:
				errorValue = self.computeError( n, self.interpreter )


			if ( self._useChallenges == 1 ):
				errorValue = ( errorValue * ( 2.000000 - self._solutionRatesGeneration[ n ] ) )


			if ( self._useChallenges == 2 ):
				errorValue = ( errorValue * ( 2.000000 - self._solutionRatesTotal[ n ] ) )


			errors.append( errorValue )
			if self._abortFitnessTest:
				n = self.countFitnessCases()
				return self._abortFitnessValue


			if ( errorValue == 0.000000 ):
				self._solutionCountsGeneration[ n ] = ( self._solutionCountsGeneration[ n ] + 1.000000 )
				self._solutionCountsTotal[ n ] = ( self._solutionCountsTotal[ n ] + 1.000000 )



			n = ( n + 1 )

		p.setErrors( errors )
		return self.computeErrorVectorFitness( errors )

	def countFitnessCases( self ):
		'''This method must be overriden by the user in order to run a PushGP experiment.  This method should return the number of  fitness cases to be run.'''

		raise Exception( '''The method count-fitness-cases must be overridden by the user -- see the class documentation for PushGPControl for more information''' )


	def createPopulation( self ):
		n = 0

		if self.pop1:
			breve.deleteInstances( self.pop1 )

		if self.pop2:
			breve.deleteInstances( self.pop2 )

		self.pop1 = breve.createInstances( breve.PushGPIndividuals, self.populationSize )
		self.pop2 = breve.createInstances( breve.PushGPIndividuals, self.populationSize )
		self.pop1.setInterpreter( self.interpreter )
		self.pop2.setInterpreter( self.interpreter )
		self.current = self.pop1
		self.next = self.pop2
		if self.seed:
			self.pop1[ 0 ].parse( self.seed )

		n = 0
		while ( n < self.countFitnessCases() ):
			self._solutionCountsGeneration.append( 0.000000 )
			self._solutionCountsTotal.append( 0.000000 )
			self._solutionRatesGeneration.append( 0.000000 )
			self._solutionRatesTotal.append( 0.000000 )

			n = ( n + 1 )

		self.inited = 1

	def evaluate( self ):
		fitness = 0
		n = 0

		fitness = self.computeZeroDurationFitness( self.current[ self.currentIndividual ] )
		if ( fitness < 0.000000 ):
			print '''warning: error value lower than 0 for GP individual'''
			fitness = 100000000


		if ( fitness < 0.001000 ):
			print ';;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;'
			print ''';; Solution found at generation %s:''' % (  self.generation ), self.current[ self.currentIndividual ].getString()
			self.examine( self.current[ self.currentIndividual ] )
			self.controller.endSimulation()


		if ( ( not self._bestOfGen ) or ( fitness < self._bestOfGen.getFitness() ) ):
			self._bestOfGen = self.current[ self.currentIndividual ]

		self._totalSize = ( self._totalSize + self.current[ self.currentIndividual ].getSize() )
		self._totalErrors = ( self._totalErrors + fitness )
		self.current[ self.currentIndividual ].setFitness( fitness )
		self.currentIndividual = ( self.currentIndividual + 1 )
		if ( self.currentIndividual == self.populationSize ):
			n = 0
			while ( n < breve.length( self._solutionCountsGeneration ) ):
				self._solutionRatesGeneration[ n ] = ( self._solutionCountsGeneration[ n ] / self.populationSize )
				self._solutionRatesTotal[ n ] = ( self._solutionCountsTotal[ n ] / ( self.populationSize * ( self.generation + 1 ) ) )

				n = ( n + 1 )

			self.report( self._bestOfGen )
			self.reproduce()
			self.swapCurrent()
			self.currentIndividual = 0
			self.generation = ( self.generation + 1 )
			self._bestOfGen = 0
			self._totalSize = 0
			self._totalErrors = 0
			if ( self.generation > self.generationLimit ):
				print ';;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;'
				print ''';; FAILURE: no solution found after %s generations''' % (  self.generation )
				self.controller.endSimulation()





	def examine( self, p ):
		'''This method is called when a solution is found.  You should override  this method to preform any additional analysis you'd like.  Typically this involves examining the program for generalization. <P> The default implementation simplifies the program p for 1000 iteration using METHOD(auto-simplify) and prints out the result.'''

		n = 0

		n = 0
		while ( n < 1000 ):
			self.autoSimplify( p )
			n = ( n + 1 )

		print ''';; Simplifies to:''', self.current[ self.currentIndividual ].getString()

	def getCrossoverPercent( self ):
		'''Returns the crossover percent.  See METHOD(set-crossover-percent) for more information.'''

		return self.crossoverPercent

	def getCurrentIndividual( self ):
		'''Returns the PushProgram currently being evaluated.'''

		if ( self.current and ( self.currentIndividual < breve.length( self.current ) ) ):
			return self.current[ self.currentIndividual ]

		return 0

	def getDeletionPercent( self ):
		'''Returns the deletion percent.  See METHOD(set-deletion-percent) for more information.'''

		return self.deletionPercent

	def getFitnessTestDuration( self ):
		return self.fitnessTestDuration

	def getGenerationLimit( self ):
		'''Returns the generation limit.  See METHOD(set-generation-limit) for more information.'''

		return self.generationLimit

	def getInterpreter( self ):
		'''Returns the OBJECT(PushInterpreter) object that this object is  using for fitness tests.'''

		return self.interpreter

	def getMutationPercent( self ):
		'''Returns the mutation percent.  See METHOD(set-mutation-percent) for more information.'''

		return self.mutationPercent

	def getPopulationSize( self ):
		'''Returns the population size.  See METHOD(set-population-size) for more information.'''

		return self.populationSize

	def getProgramLimitPenalty( self ):
		'''Returns a penalty value applied to program which hit their evaluation limit.  The default value is .1,  but you may override this method in order to penalize programs differently according to the problem.'''

		return 0.100000

	def getTournamentSize( self ):
		'''Returns the tournament size.  See METHOD(set-tournament-size) for more information.'''

		return self.tournamentSize

	def init( self ):
		self.populationSize = 2000
		self.mutationPercent = 40
		self.deletionPercent = 5
		self.crossoverPercent = 40
		self.tournamentSize = 7
		self.generationLimit = 300
		self._useChallenges = 1
		self.interpreter = breve.createInstances( breve.PushInterpreter, 1 )

	def iterate( self ):
		n = 0
		percent = 0

		if ( not self.inited ):
			self.createPopulation()

		n = 0
		while ( n < 100 ):
			self.evaluate()
			n = ( n + 1 )

		percent = ( ( 100.000000 * self.currentIndividual ) / self.populationSize )
		self.controller.setDisplayText( '''Generation %s: %s%% Complete''' % (  self.generation, percent ), -0.900000, -0.800000 )
		if ( self.currentIndividual >= ( self.populationSize - 100 ) ):
			self.controller.setDisplayText( '''Generation %s: breeding''' % (  self.generation ), -0.900000, -0.800000 )


	def readInterpreterConfig( self, file ):
		'''Reads a configuration file for the interpreter used in the   fitness tests.  Changing the configuration also reinitializes the evolving population (since instructions sets may be changed).'''

		self.interpreter.readConfig( file )
		self.inited = 0

	def report( self, bestIndividual ):
		'''Logs information to the output at every generation.  This method  may be overridden to provide more (or less) output data.   <P> The argument bestIndividual is the best individual in the population and is provided so that it may be inspected by the user if this method is overridden.'''

		bestFitness = 0
		errors = breve.objectList()
		n = 0

		self._totalSize = ( self._totalSize / self.populationSize )
		self._totalErrors = ( self._totalErrors / self.populationSize )
		bestFitness = bestIndividual.getFitness()
		errors = bestIndividual.getErrors()
		print ';;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;'
		print ''';; -*- Report at generation %s''' % (  self.generation )
		print ''';; Best Individual: '''
		print bestIndividual.getString()
		print ''';; Errors for best individual: ''', errors
		print ''';; Problem solution rates this generation: ''', self._solutionRatesGeneration
		print ''';; Problem solution rates all generations: ''', self._solutionRatesTotal
		print ''';; Total errors for best individual: %s''' % (  bestFitness )
		print ''';; Size of best individual (points):''', bestIndividual.getSize()
		print ';;'
		print ''';; Average total errors in population: %s''' % (  self._totalErrors )
		print ''';; Average program size in population (points): %s''' % (  self._totalSize )
		print ''
		self.controller.setDisplayText( '''Best Fitness: %s''' % (  bestFitness ), -0.900000, -0.900000, 2 )

	def reproduce( self ):
		'''Used internally.                '''

		method = 0
		n = 0
		p2 = None
		p1 = None

		n = 0
		while ( n < self.countFitnessCases() ):
			self._solutionCountsGeneration[ n ] = 0.000000

			n = ( n + 1 )

		while ( n < self.populationSize ):
			method = breve.randomExpression( 100 )
			if ( method < self.mutationPercent ):
				p1 = self.runTournament( n )
				self.next[ n ].copyFrom( p1 )
				self.next[ n ].mutate( self.interpreter )

			else:
				if ( method < ( self.crossoverPercent + self.mutationPercent ) ):
					p1 = self.runTournament( n )
					p2 = self.runTournament( n )
					self.next[ n ].crossover( p1, p2, self.interpreter )

				else:
					if ( method < ( ( self.crossoverPercent + self.mutationPercent ) + self.deletionPercent ) ):
						p1 = self.runTournament( n )
						self.next[ n ].copyFrom( p1 )
						self.next[ n ].deleteRandomSubtree()

					else:
						p1 = self.runTournament( n )
						self.next[ n ].copyFrom( p1 )




			n = ( n + 1 )



	def runTournament( self, index = -1.000000 ):
		'''Runs a tournament selection from the population.  Used internally during reproduction uses the tournament size set with  METHOD(set-tournament-size). <P> This method may be overridden to implement a custom tournament selection if desired, though the default implementation should suffice in most  situations.'''

		test = 0
		n = 0
		best = None

		if ( self.spatialRadius == 0 ):
			best = self.current[ breve.randomExpression( ( self.populationSize - 1 ) ) ]
		else:
			test = ( index + ( breve.randomExpression( ( ( 2 * self.spatialRadius ) - 1 ) ) - self.spatialRadius ) )
			if ( test >= self.populationSize ):
				test = ( test - self.populationSize )

			if ( test < 0 ):
				test = ( test + self.populationSize )

			best = self.current[ test ]


		n = 0
		while ( n < ( self.tournamentSize - 1 ) ):
			if ( self.spatialRadius == 0 ):
				test = breve.randomExpression( ( self.populationSize - 1 ) )
			else:
				test = ( index + ( breve.randomExpression( ( ( 2 * self.spatialRadius ) - 1 ) ) - self.spatialRadius ) )
				if ( test >= self.populationSize ):
					test = ( test - self.populationSize )

				if ( test < 0 ):
					test = ( test + self.populationSize )



			if ( self.current[ test ].getFitness() < best.getFitness() ):
				best = self.current[ test ]


			n = ( n + 1 )

		return best

	def seedPopulation( self, pushProgram ):
		'''Uses pushProgram to seed the initial population.'''

		self.seed = pushProgram.getString()

	def setCrossoverPercent( self, n ):
		'''Sets the percentage of agents generated by genetic crossover at  each generation.  This should be a value between 0 and 100.   The default value is 40.'''

		self.crossoverPercent = n

	def setDeletionPercent( self, n ):
		'''Sets the percentage of agents generated by genetic deletion at  each generation.  This should be a value between 0 and 100.   The default value is 5.'''

		self.deletionPercent = n

	def setFitnessTestDuration( self, d ):
		'''Sets the duration of the fitness test to d.  The duration of the  fitness test determines how much simulation time is allowed for  each fitness test.   <P> The default value is 0, a value which is appropriate for programs which do not require simulation to determine fitness values.'''

		self.fitnessTestDuration = d

	def setGenerationLimit( self, n ):
		'''Sets the generation limit to n.  If no solution is found by the generation limit, the simulation with terminate.  The  default value is 300.'''

		self.generationLimit = n

	def setMutationPercent( self, n ):
		'''Sets the percentage of agents generated by genetic mutation at  each generation.  This should be a value between 0 and 100.   The default value is 40.'''

		self.mutationPercent = n

	def setPopulationSize( self, n ):
		'''Sets the population size to n and reinitializes the population. The default value is 2000.'''

		self.populationSize = n
		self.inited = 0

	def setSpatialRadius( self, n ):
		'''Sets the spatial radius used for tournament selection during reproduction.  Setting to 0 disables spatial effects.'''

		self.spatialRadius = n

	def setTournamentSize( self, n ):
		'''Sets the tournament size to n.  The tournament size determines  how many individuals are examined during tournament selection. The default value is 7.'''

		self.tournamentSize = n

	def setupFitnessTest( self, n, interpreter ):
		'''This method must be overridden by the user in order to run a  PushGP experiment.  The job of this method is to prepare to run  the specified fitness test.  This typically means pushing input  values onto the OBJECT(PushInterpreter) object's stacks. <p> In a symbolic regression problem, for example, this method might  push a predetermined input value onto the float stack. <pre> interpreter push-float value inputValues{ n }. </pre>'''

		raise Exception( '''The method setup-fitness-test must be overridden by the user -- see the class documentation for PushGPControl for more information''' )


	def swapCurrent( self ):
		'''Used internally.'''

		if ( self.current == self.pop1 ):
			self.current = self.pop2
			self.next = self.pop1

		else:
			self.current = self.pop1
			self.next = self.pop2




breve.PushGP = PushGP
class PushGPIndividual( breve.PushProgram ):
	'''A class used by OBJECT(PushGP), this is a single individual in a PushGP run.  It is not typically used directly by the user.'''

	def __init__( self ):
		breve.PushProgram.__init__( self )
		self.errors = breve.objectList()
		self.fitness = 0
		self.interpreter = None

	def getErrors( self ):
		'''Returns the current list of errors for an individual as determined by the fitness test.'''

		return self.errors

	def getFitness( self ):
		'''Returns the fitness of this GP individual.'''

		return self.fitness

	def setErrors( self, errorList ):
		self.errors = errorList

	def setFitness( self, newFitness ):
		'''Sets the fitness of this object to newFitness.  You  shouldn't call this method unless you're implementing your own GP system.'''

		self.fitness = newFitness

	def setInterpreter( self, newInterpreter ):
		self.interpreter = newInterpreter
		self.makeRandomCode( self.interpreter )


breve.PushGPIndividual = PushGPIndividual
class PushClusterGP( breve.PushGP ):
	'''PushClusterGP is a version of OBJECT(PushGP) that allows individual programs in a genetic programming run to migrate to runs on other  networked machines.   <P> Turning a single machine PushGP experiment to a multiple machine  PushClusterGP experiment is simple.  Just provide a list of the  hosts running the simulation to METHOD(set-emigration-hosts). <P> Note: using the PushClusterGP object will open a tcp server on  port 59175.'''

	def __init__( self ):
		breve.PushGP.__init__( self )
		self.emigrationHosts = breve.objectList()
		self.emigrationPercent = 0
		self.lastTransfer = None
		self.port = 0
		self.server = None
		self.transfer = None
		PushClusterGP.init( self )

	def acceptUpload( self, immigrants, h ):
		breve.deleteInstances( self.lastTransfer )
		self.lastTransfer = immigrants

	def getEmigrationPercent( self ):
		'''Returns the emigration percent.  See METHOD(set-emigration-percent) for more information.'''

		return self.emigrationPercent

	def init( self ):
		self.server = breve.createInstances( breve.NetworkServer, 1 )
		self.server.listen( 59175 )
		self.server.setUploadRecipient( self )
		self.emigrationPercent = 3
		self.transfer = breve.createInstances( breve.PushProgramTransporter, 1 )
		self.port = 59175

	def reproduce( self ):
		'''Used internally.                '''

		method = 0
		n = 0
		p2 = None
		p1 = None
		recipientHost = ''
		index = 0

		while ( n < self.populationSize ):
			method = breve.randomExpression( 100 )
			if ( method < self.mutationPercent ):
				p1 = self.runTournament( n )
				self.next[ n ].copyFrom( p1 )
				self.next[ n ].mutate( self.interpreter )

			else:
				if ( method < ( self.crossoverPercent + self.mutationPercent ) ):
					p1 = self.runTournament( n )
					p2 = self.runTournament( n )
					self.next[ n ].crossover( p1, p2, self.interpreter )

				else:
					if ( method < ( ( self.crossoverPercent + self.mutationPercent ) + self.deletionPercent ) ):
						p1 = self.runTournament( n )
						self.next[ n ].copyFrom( p1 )
						self.next[ n ].deleteRandomSubtree()

					else:
						if ( method < ( ( ( self.crossoverPercent + self.mutationPercent ) + self.deletionPercent ) + self.emigrationPercent ) ):
							if self.lastTransfer:
								p1 = self.lastTransfer.getRandomProgram()
							else:
								p1 = self.runTournament( n )

							self.next[ n ].copyFrom( p1 )
							index = ( index + 1 )

						else:
							p1 = self.runTournament( n )
							self.next[ n ].copyFrom( p1 )





			n = ( n + 1 )


		self.transfer.clear()
		if ( breve.length( self.emigrationHosts ) == 0 ):
			return

		n = 0
		while ( n < ( self.populationSize * ( self.emigrationPercent / 100.000000 ) ) ):
			self.transfer.add( self.runTournament() )
			n = ( n + 1 )

		recipientHost = self.emigrationHosts[ breve.randomExpression( ( breve.length( self.emigrationHosts ) - 1 ) ) ]
		self.transfer.sendOverNetwork( recipientHost, 59175 )

	def setEmigrationHosts( self, hosts ):
		'''Sets the PushClusterGP object to send individual programs to  hosts.'''

		self.emigrationHosts = list( hosts) 

	def setEmigrationPercent( self, n ):
		'''Sets the percentage of agents generated by genetic emigration at  each generation.  This should be a value between 0 and 100, but  should typically be relatively low: mutation and crossover should be the primary genetic operators. The default value is 3.'''

		self.emigrationPercent = n


breve.PushClusterGP = PushClusterGP
class PushProgramTransporter( breve.Object ):
	'''Used by OBJECT(PushClusterGP) to send Push programs over the network. No user-level methods are provided for this class.'''

	def __init__( self ):
		breve.Object.__init__( self )
		self.programStrings = breve.objectList()

	def add( self, p ):
		self.programStrings.append( p.getString() )

	def clear( self ):
		self.programStrings = []

	def getProgram( self, n ):
		code = None

		code = breve.createInstances( breve.PushProgram, 1 )
		if ( n >= breve.length( self.programStrings ) ):
			return code

		code.parse( self.programStrings[ n ] )
		return code

	def getRandomProgram( self ):
		return self.getProgram( breve.randomExpression( ( breve.length( self.programStrings ) - 1 ) ) )

	def parsePrograms( self, i ):
		code = ''
		program = None
		result = breve.objectList()

		for code in self.programStrings:
			program = breve.createInstances( breve.PushProgram, 1 )
			program.parse( code )
			result.append( program )


		return result


breve.PushProgramTransporter = PushProgramTransporter
# Add our newly created classes to the breve namespace

breve.PushGPIndividuals = PushGPIndividual



