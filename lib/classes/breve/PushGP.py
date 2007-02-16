
import breve

class PushGP( breve.Object ):
	'''This class is a special control class used to implement  experiments using the Push Genetic Programming (PushGP) system.  PushGP is a system which uses genetic programming and the Push programming language to attempt to evolve  solutions to a wide variety of problems.  For more  information on Push and PushGP, see the class OBJECT(PushInterpreter). <p> This class is appropriate for evolving Push programs which are only run once per fitness case. <p> To implement a PushGP experiment you will need to setup a PushGP object which overrides two methods: METHOD(setup-fitness-test), which sets up a fitness test and METHOD(compute-error), which is used to compute the  fitness of an individual. <p> Additionally, other methods in this class are useful for configuring the genetic programming environment and a  variety of parameters including population size,  mutation rate, crossover rate, and the Push language 	 interpreter set. <p> Finally, you may also wish to add custom instructions  to the Push interpreter objects which manipulate data or trigger actions in the simulated world.  The Push interpreter can be retrieved using the method  METHOD(get-interpreter), and adding instructions is  documented in OBJECT(PushInterpreter).'''

	def __init__( self ):
		breve.Object.__init__( self )
		self._abortFitnessTest = 0
		self._abortFitnessValue = 0
		self._bestOfGen = None
		self._fitnessCaseSolutionRates = []
		self._lastSolutionRates = []
		self._totalErrors = 0
		self._totalSize = 0
		self._useChallenges = 0
		self.crossoverPercent = 0
		self.current = []
		self.currentFitnessTest = 0
		self.currentIndividual = 0
		self.deletionPercent = 0
		self.fitnessTestDuration = 0
		self.generation = 0
		self.generationLimit = 0
		self.inited = 0
		self.interpreter = None
		self.mutationPercent = 0
		self.next = []
		self.pop1 = []
		self.pop2 = []
		self.populationSize = 0
		self.seed = ''
		self.spatialRadius = 0
		self.tournamentSize = 0
		PushGP.init( self )

	def abortFitnessTest( self, fitness ):
		'''Aborts the currently running fitness '''

		self._abortFitnessValue = fitness
		self._abortFitnessTest = 1

	def autoSimplify( self, p ):
		'''Programs generated through genetic programming are often complicated and  difficult to understand.  This method attempts to simplify an evolved   push program while maintaining the same fitness. <P> Each time this method is invoked, a random simplification is preformed  on the program p.  If the simplification does not harm the fitness, p is modified to contain the simplification, otherwise, nothing happens. This method should be called repeatedly -- perhaps 100 times or more  to aggressively simplify an unweildy program.'''

		fitness = self.computeZeroDurationFitness( p )
		newProgram = breve.PushGPIndividual()
		newProgram.copyFrom( p )
		if :
			newProgram.deleteRandomSubtree()
		else:
			newProgram.flattenRandomSubtree()

		newFitness = self.computeZeroDurationFitness( newProgram )
		if ( newFitness == fitness ):
			p.copyFrom( newProgram )

		del newProgram
		return p


	def computeDiversity( self, t ):
		'''Computes the diversity of the current population, as defined by  the OBJECT(PushDiversityPool) object.  Returns the number of  unique "species" which differ by less than t points.'''

		pool = breve.PushDiversityPool()
		pool.setTolerance( t )
		
		n = pool.getSize()
		del pool
		return n


	def computeError( self, n, interpreter ):
		'''This method must be overridden by the user in order to run a  PushGP experiment.  The job of this method is to inspect the  state of the OBJECT(PushInterpreter) object and determine the fitness (in terms of error) of the specified fitness case.   This typically involves comparing values from the interpreter's  stacks against desired values. <p> For example, with a symbolic regression problem, this method  might return the difference between the top of the float stack,  and the predetermined "correct" answer for the specified fitness  case.  For example: <pre> return (interpreter get-float-stack-top) - desiredOutputs{ n }. </pre>'''

		raise Exception( '''The method compute-error must be overridden by the user -- see the class documentation for PushGPControl for more information''' )


	def computeErrorVectorFitness( self, errorList ):
		'''This method takes a list of errors returned by METHOD(compute-error) for all  fitness cases, and produces a fitness value.  The default implementation of  this method computes the fitness by summing the absolute values of the errors, which should suffice for most applications. <P> Overriding this method is optional.'''

		for value in errorList:
			fitness = ( fitness + len( value ) )

		return fitness


	def computeZeroDurationFitness( self, p ):
		''''''

		self._abortFitnessTest = 0
		
		p.setErrors( errors )
		return self.computeErrorVectorFitness( errors )


	def countFitnessCases( self ):
		'''This method must be overriden by the user in order to run a PushGP experiment.  This method should return the number of  fitness cases to be run.'''

		raise Exception( '''The method count-fitness-cases must be overridden by the user -- see the class documentation for PushGPControl for more information''' )


	def createPopulation( self ):
		''''''

		if self.pop1:
			del self.pop1

		if self.pop2:
			del self.pop2

		self.pop1 = breve.PushGPIndividuals()
		self.pop2 = breve.PushGPIndividuals()
		self.pop1.setInterpreter( self.interpreter )
		self.pop2.setInterpreter( self.interpreter )
		self.current = self.pop1
		self.next = self.pop2
		if self.seed:
			self.pop1[ 0 ].parse( self.seed )

		
		self.inited = 1

	def evaluate( self ):
		''''''

		fitness = self.computeZeroDurationFitness( self.current[ self.currentIndividual ] )
		if ( fitness < 0.000000 ):
			print '''warning: error value lower than 0 for GP individual'''
			fitness = 100000000


		if ( fitness < 0.001000 ):
			print ''';;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;'''
			print ''';; Solution found at generation :''' % (  self.generation ), self.current[ self.currentIndividual ].getString()
			self.examine( self.current[ self.currentIndividual ] )
			self.controller.endSimulation()


		if ( ( not self._bestOfGen ) or ( fitness < self._bestOfGen.getFitness() ) ):
			self._bestOfGen = self.current[ self.currentIndividual ]

		self._totalSize = ( self._totalSize + self.current[ self.currentIndividual ].getSize() )
		self._totalErrors = ( self._totalErrors + fitness )
		self.current[ self.currentIndividual ].setFitness( fitness )
		self.currentIndividual = ( self.currentIndividual + 1 )
		if ( self.currentIndividual == self.populationSize ):
			
			self._lastSolutionRates = 
			self.report( self._bestOfGen )
			self.reproduce()
			self.swapCurrent()
			self.currentIndividual = 0
			self.generation = ( self.generation + 1 )
			self._bestOfGen = 0
			self._totalSize = 0
			self._totalErrors = 0
			if ( self.generation > self.generationLimit ):
				print ''';;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;'''
				print ''';; FAILURE: no solution found after  generations''' % (  self.generation )
				self.controller.endSimulation()





	def examine( self, p ):
		'''This method is called when a solution is found.  You should override  this method to preform any additional analysis you'd like.  Typically this involves examining the program for generalization. <P> The default implementation simplifies the program p for 1000 iteration using METHOD(auto-simplify) and prints out the result.'''

		
		print ''';; Simplifies to:''', self.current[ self.currentIndividual ].getString()

	def getCrossoverPercent( self ):
		'''Returns the crossover percent.  See METHOD(set-crossover-percent) for more information.'''

		return self.crossoverPercent


	def getCurrentIndividual( self ):
		'''Returns the PushProgram currently being evaluated.'''

		if ( self.current and ( self.currentIndividual < len( self.current ) ) ):
			return self.current[ self.currentIndividual ]


		return 0


	def getDeletionPercent( self ):
		'''Returns the deletion percent.  See METHOD(set-deletion-percent) for more information.'''

		return self.deletionPercent


	def getFitnessTestDuration( self ):
		''''''

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
		''''''

		self.populationSize = 2000
		self.mutationPercent = 40
		self.deletionPercent = 5
		self.crossoverPercent = 40
		self.tournamentSize = 7
		self.generationLimit = 300
		self._useChallenges = 0
		self.interpreter = breve.PushInterpreter()

	def iterate( self ):
		''''''

		if ( not self.inited ):
			self.createPopulation()

		
		percent = ( ( 100.000000 * self.currentIndividual ) / self.populationSize )
		self.controller.setDisplayMessage( 0, -0.900000, -0.800000, '''Generation : % Complete''' % (  self.generation, percent ) )
		if ( self.currentIndividual >= ( self.populationSize - 100 ) ):
			self.controller.setDisplayMessage( 0, -0.900000, -0.800000, '''Generation : breeding''' % (  self.generation ) )


	def readInterpreterConfig( self, file ):
		'''Reads a configuration file for the interpreter used in the   fitness tests.  Changing the configuration also reinitializes the evolving population (since instructions sets may be changed).'''

		self.interpreter.readConfig( file )
		self.inited = 0

	def report( self, bestIndividual ):
		'''Logs information to the output at every generation.  This method  may be overridden to provide more (or less) output data.   <P> The argument bestIndividual is the best individual in the population and is provided so that it may be inspected by the user if this method is overridden.'''

		self._totalSize = ( self._totalSize / self.populationSize )
		self._totalErrors = ( self._totalErrors / self.populationSize )
		bestFitness = bestIndividual.getFitness()
		errors = bestIndividual.getErrors()
		print ''';;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;'''
		print ''';; -*- Report at generation ''' % (  self.generation )
		print ''';; Best Individual: '''
		print bestIndividual.getString()
		print ''';; Errors for best individual: ''', errors
		print ''';; Problem solution rates: ''', self._fitnessCaseSolutionRates
		print ''';; Total errors for best individual: ''' % (  bestFitness )
		print ''';; Size of best individual (points):''', bestIndividual.getSize()
		print ''';;'''
		print ''';; Average total errors in population: ''' % (  self._totalErrors )
		print ''';; Average program size in population (points): ''' % (  self._totalSize )
		print ''''''
		self.controller.setDisplayMessage( 2, -0.900000, -0.900000, '''Best Fitness: ''' % (  bestFitness ) )

	def reproduce( self ):
		'''Used internally.                '''

		
		

	def runTournament( self, index ):
		'''Runs a tournament selection from the population.  Used internally during reproduction uses the tournament size set with  METHOD(set-tournament-size). <P> This method may be overridden to implement a custom tournament selection if desired, though the default implementation should suffice in most  situations.'''

		if ( self.spatialRadius == 0 ):
			best = self.current[  ]
		else:
			test = ( index + (  - self.spatialRadius ) )
			if ( test >= self.populationSize ):
				test = ( test - self.populationSize )

			if ( test < 0 ):
				test = ( test + self.populationSize )

			best = self.current[ test ]


		
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
		self.errors = []
		self.fitness = 0
		self.interpreter = None

	def getErrors( self ):
		'''Returns the current list of errors for an individual as determined by the fitness test.'''

		return self.errors


	def getFitness( self ):
		'''Returns the fitness of this GP individual.'''

		return self.fitness


	def setErrors( self, errorList ):
		''''''

		self.errors = errorList

	def setFitness( self, newFitness ):
		'''Sets the fitness of this object to newFitness.  You  shouldn't call this method unless you're implementing your own GP system.'''

		self.fitness = newFitness

	def setInterpreter( self, newInterpreter ):
		''''''

		self.interpreter = newInterpreter
		self.makeRandomCode( self.interpreter )


breve.PushGPIndividual = PushGPIndividual

class PushClusterGP( breve.PushGP ):
	'''PushClusterGP is a version of OBJECT(PushGP) that allows individual programs in a genetic programming run to migrate to runs on other  networked machines.   <P> Turning a single machine PushGP experiment to a multiple machine  PushClusterGP experiment is simple.  Just provide a list of the  hosts running the simulation to METHOD(set-emigration-hosts). <P> Note: using the PushClusterGP object will open a tcp server on  port 59175.'''

	def __init__( self ):
		breve.PushGP.__init__( self )
		self.emigrationHosts = []
		self.emigrationPercent = 0
		self.lastTransfer = None
		self.port = 0
		self.server = None
		self.transfer = None
		PushClusterGP.init( self )

	def acceptUpload( self, immigrants, h ):
		''''''

		del self.lastTransfer
		self.lastTransfer = immigrants

	def getEmigrationPercent( self ):
		'''Returns the emigration percent.  See METHOD(set-emigration-percent) for more information.'''

		return self.emigrationPercent


	def init( self ):
		''''''

		self.server = breve.NetworkServer()
		self.server.listen(  )
		self.server.setUploadRecipient( self )
		self.emigrationPercent = 3
		self.transfer = breve.PushProgramTransporter()
		self.port = 

	def reproduce( self ):
		'''Used internally.                '''

		
		self.transfer.clear()
		if ( len( self.emigrationHosts ) == 0 ):
			return


		
		recipientHost = self.emigrationHosts[  ]
		self.transfer.sendOverNetwork( recipientHost,  )

	def setEmigrationHosts( self, hosts ):
		'''Sets the PushClusterGP object to send individual programs to  hosts.'''

		self.emigrationHosts = 

	def setEmigrationPercent( self, n ):
		'''Sets the percentage of agents generated by genetic emigration at  each generation.  This should be a value between 0 and 100, but  should typically be relatively low: mutation and crossover should be the primary genetic operators. The default value is 3.'''

		self.emigrationPercent = n


breve.PushClusterGP = PushClusterGP

class PushProgramTransporter( breve.Object ):
	'''Used by OBJECT(PushClusterGP) to send Push programs over the network. No user-level methods are provided for this class.'''

	def __init__( self ):
		breve.Object.__init__( self )
		self.programStrings = []

	def add( self, p ):
		''''''

		self.programStrings.append( p.getString() )

	def clear( self ):
		''''''

		self.programStrings = []

	def getProgram( self, n ):
		''''''

		code = breve.PushProgram()
		if ( n >= len( self.programStrings ) ):
			return code


		code.parse( self.programStrings[ n ] )
		return code


	def getRandomProgram( self ):
		''''''

		return self.getProgram(  )


	def parsePrograms( self, i ):
		''''''

		for code in self.programStrings:
			program = breve.PushProgram()
			program.parse( code )
			result.append( program )


		return result



breve.PushProgramTransporter = PushProgramTransporter


