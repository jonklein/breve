
import breve

class PushInterpreter( breve.Object ):
	'''Summary: an interface to the Push programming language. <P> The PushInterpreter object is used to setup and run programs in the  Push programming language, a language developed specifically for evolutionary computation. <P> Push is useful in breve for evolving evolved agent behaviors.  The  steve language, in which users write simulations, is a high level language which is not well suited for evolutionary computation.   The Push language, on the other hand, is designed from the ground up for evolutionary computation. <P> More information about Push is available <a href="http://hampshire.edu/lspector/push.html">here</a>.'''

	def __init__( self ):
		breve.Object.__init__( self )
		self.effort = 0
		self.pushInterpreterPointer = None
		PushInterpreter.init( self )

	def addInstruction( self, instructionName, targetInstance, methodName ):
		'''Adds a custom callback instruction named instructionName to the push interpreter.   When instructionName is called, it will trigger the method methodName in targetInstance.'''

		if ( instructionName == '''''' ):
			instructionName = methodName

		breve.breveInternalFunctionFinder.pushCallbackNew( self, self.pushInterpreterPointer, instructionName, methodName, targetInstance )

	def addMacro( self, instructionName, code ):
		'''Adds a new instruction named instructionName, which has the effect of calling  the instructions in code.'''

		breve.breveInternalFunctionFinder.pushMacroNew( self, self.pushInterpreterPointer, instructionName, code.getCodePointer() )

	def archive( self ):
		''''''


	def clearStacks( self ):
		'''Clears the stacks of the Push interpreter.'''

		breve.breveInternalFunctionFinder.pushClearStacks( self, self.pushInterpreterPointer )

	def copyCodeStackTop( self, program ):
		'''Copies the top of the code stack to an existing program. Use METHOD(get-code-stack-size) to determine whether the  code stack is empty or not.'''

		other = breve.breveInternalFunctionFinder.pushCodeStackTop( self, self.pushInterpreterPointer )
		if ( not other ):
			program.parse( '''( )''' )
			return



		program.parse( breve.breveInternalFunctionFinder.pushCodeGetString( self, other ) )

	def copyExecStackTop( self, program ):
		'''Copies the top of the code stack to an existing program. Use METHOD(get-exec-stack-size) to determine whether the  code stack is empty or not.'''

		other = breve.breveInternalFunctionFinder.pushExecStackTop( self, self.pushInterpreterPointer )
		if ( not other ):
			program.parse( '''( )''' )
			return



		program.parse( breve.breveInternalFunctionFinder.pushCodeGetString( self, other ) )

	def dearchive( self ):
		''''''


	def destroy( self ):
		''''''

		breve.breveInternalFunctionFinder.pushEnvironmentFree( self, self.pushInterpreterPointer )

	def getBooleanStackSize( self ):
		'''Returns the size of the boolean stack.'''

		return breve.breveInternalFunctionFinder.pushBoolStackSize( self, self.pushInterpreterPointer )


	def getBooleanStackTop( self ):
		'''Returns the top of the boolean stack. If the boolean stack is empty, the method returns 0 (false). Use METHOD(get-boolean-stack-size) to determine whether the  boolean stack is empty or not.'''

		return breve.breveInternalFunctionFinder.pushBoolStackTop( self, self.pushInterpreterPointer )


	def getCodeStackSize( self ):
		'''Returns the size of the code stack.'''

		return breve.breveInternalFunctionFinder.pushCodeStackSize( self, self.pushInterpreterPointer )


	def getEffort( self ):
		'''Returns the "effort" of the Push interpreter--the running tally of instructions  the  interpreter has executed. for associating energy costs with computation in evolving populations.'''

		return self.effort


	def getEvaluationLimit( self ):
		'''Returns the current push evaluation limit.  See METHOD(set-evaluation-limit)  for more information on the evaluation limit.'''

		return breve.breveInternalFunctionFinder.pushEnvironmentGetEvalLimit( self, self.pushInterpreterPointer )


	def getExecStackSize( self ):
		'''Returns the size of the code stack.'''

		return breve.breveInternalFunctionFinder.pushExecStackSize( self, self.pushInterpreterPointer )


	def getFloatStackSize( self ):
		'''Returns the size of the float stack.'''

		return breve.breveInternalFunctionFinder.pushFloatStackSize( self, self.pushInterpreterPointer )


	def getFloatStackTop( self ):
		'''Returns the top of the float stack. If the float stack is empty, the method returns 0.0. Use METHOD(get-float-stack-size) to determine whether the  boolean stack is empty or not.'''

		return breve.breveInternalFunctionFinder.pushFloatStackTop( self, self.pushInterpreterPointer )


	def getIntegerStackSize( self ):
		'''Returns the size of the integer stack.'''

		return breve.breveInternalFunctionFinder.pushIntStackSize( self, self.pushInterpreterPointer )


	def getIntegerStackTop( self ):
		'''Returns the top of the interger stack. If the integer stack is empty, the method returns 0. Use METHOD(get-integer-stack-size) to determine whether the  integer stack is empty or not.'''

		return breve.breveInternalFunctionFinder.pushIntStackTop( self, self.pushInterpreterPointer )


	def getInterpreterPointer( self ):
		'''Used internally.'''

		return self.pushInterpreterPointer


	def getListLimit( self ):
		'''Returns the size limit of lists constructed in push.  See  METHOD(set-list-limit) for more information.'''

		return breve.breveInternalFunctionFinder.pushEnvironmentGetListLimit( self, self.pushInterpreterPointer )


	def getVectorStackSize( self ):
		'''Returns the size of the vector stack.'''

		return breve.breveInternalFunctionFinder.pushVectorStackSize( self, self.pushInterpreterPointer )


	def getVectorStackTop( self ):
		'''Returns the top of the vector stack.   If the vector stack is empty, the method returns (0, 0, 0). Use METHOD(get-vector-stack-size) to determine whether the  vector stack is empty or not.'''

		return breve.breveInternalFunctionFinder.pushVectorStackTop( self, self.pushInterpreterPointer, breve.vector( 0, 0, 0 ) )


	def init( self ):
		''''''

		self.pushInterpreterPointer = breve.breveInternalFunctionFinder.pushEnvironmentNew( self,  )
		self.setEvaluationLimit( 200 )
		self.setListLimit( 200 )

	def popBooleanStack( self ):
		'''Pops the boolean stack.  If the boolean stack is empty, this method has no effect.'''

		breve.breveInternalFunctionFinder.pushFloatStackPop( self, self.pushInterpreterPointer )

	def popCodeStack( self ):
		'''Pops the code stack.  If the code stack is empty, this  method has no effect.'''

		breve.breveInternalFunctionFinder.pushCodeStackPop( self, self.pushInterpreterPointer )

	def popExecStack( self ):
		'''Pops the exec stack.  If the code stack is empty, this  method has no effect.'''

		breve.breveInternalFunctionFinder.pushExecStackPop( self, self.pushInterpreterPointer )

	def popFloatStack( self ):
		'''Pops the float stack.  If the float stack is empty, this  method has no effect.'''

		breve.breveInternalFunctionFinder.pushFloatStackPop( self, self.pushInterpreterPointer )

	def popIntegerStack( self ):
		'''Pops the integer stack.  If the integer stack is empty, this  method has no effect.'''

		breve.breveInternalFunctionFinder.pushIntStackPop( self, self.pushInterpreterPointer )

	def popVectorStack( self ):
		'''Pops the vector stack.  If the vector stack is empty, this  method has no effect.'''

		breve.breveInternalFunctionFinder.pushVectorStackPop( self, self.pushInterpreterPointer )

	def printConfig( self ):
		'''Prints out the current push configuration, including a list of the  active instructions.  Useful for debugging.'''

		print breve.breveInternalFunctionFinder.pushGetConfigString( self, self.pushInterpreterPointer )

	def printExecStack( self ):
		'''Prints out the current contents of the exec stack.  Useful for debugging.'''

		print '''(''', breve.breveInternalFunctionFinder.pushGetExecStackString( self, self.pushInterpreterPointer ), ''')'''

	def printStacks( self ):
		'''Prints out the current contents of the stacks.  Useful for debugging.'''

		print breve.breveInternalFunctionFinder.pushGetStacksString( self, self.pushInterpreterPointer )

	def pushBoolean( self, booleanValue ):
		'''Pushes the boolean booleanValue (an integer in breve, where 0 is false and all other values are true) onto the boolean stack.'''

		breve.breveInternalFunctionFinder.pushBoolStackPush( self, self.pushInterpreterPointer, booleanValue )

	def pushCode( self, codeObject ):
		'''Pushes the code codeObject onto the code stack.'''

		breve.breveInternalFunctionFinder.pushCodeStackPush( self, self.pushInterpreterPointer, codeObject.getCodePointer() )

	def pushExec( self, codeObject ):
		'''Pushes the code codeObject onto the exec stack.'''

		breve.breveInternalFunctionFinder.pushExecStackPush( self, self.pushInterpreterPointer, codeObject.getCodePointer() )

	def pushFloat( self, floatValue ):
		'''Pushes the float floatValue onto the float stack.'''

		breve.breveInternalFunctionFinder.pushFloatStackPush( self, self.pushInterpreterPointer, floatValue )

	def pushInteger( self, integerValue ):
		'''Pushes the integer integerValue onto the integer stack.'''

		breve.breveInternalFunctionFinder.pushIntStackPush( self, self.pushInterpreterPointer, integerValue )

	def pushVector( self, vectorValue ):
		'''Pushes the vector vectorValue onto the vector stack.'''

		breve.breveInternalFunctionFinder.pushVectorStackPush( self, self.pushInterpreterPointer, vectorValue )

	def readConfig( self, configFile ):
		'''Reads the push configuration file configFile.'''

		if ( not breve.breveInternalFunctionFinder.findFile( self, configFile ) ):
			print '''Could not locate Push configuration file ''' % (  configFile )
			return



		breve.breveInternalFunctionFinder.pushEnvironmentReadConfigFile( self, self.pushInterpreterPointer, breve.breveInternalFunctionFinder.findFile( self, configFile ) )

	def resetEffort( self ):
		'''Clears the effort counter to 0.'''

		self.effort = 0

	def run( self, codeObject, steps ):
		'''Runs the specified push code from the beginning of the program.  If  the steps variable is provided, evaluation will only run for the  specified number of instructions.  Otherwise, evaluation will run for  the evaluation limit, which is  set with METHOD(set-evaluation-limit). <P> The method returns 0 if the interpreter completely executes the  program, or 1 if execution stopped due to the evaluation limit. <P> If the program is not executed completely, execution may be stepped  forward using the method METHOD(step).  In some cases, it may be  desirable to step push programs forward in small increments by calling this method with steps = 0 in order to load the program, then using  METHOD(step) to step the simulatation forward.'''

		if ( steps == -1 ):
			steps = breve.breveInternalFunctionFinder.pushEnvironmentGetEvalLimit( self, self.pushInterpreterPointer )

		self.effort = ( self.effort + breve.breveInternalFunctionFinder.pushRun( self, self.pushInterpreterPointer, codeObject.getCodePointer(), steps ) )
		return ( not breve.breveInternalFunctionFinder.pushInterpreterDone( self, self.pushInterpreterPointer ) )


	def runWithExecStackDump( self, codeObject, steps ):
		'''Dumps out the exec stack at every instruction step.  Extremely  verbose.  For debugging only.'''

		if ( steps == -1 ):
			steps = breve.breveInternalFunctionFinder.pushEnvironmentGetEvalLimit( self, self.pushInterpreterPointer )

		self.run( codeObject, 0 )
		

	def setEvaluationLimit( self, newLimit ):
		'''Sets the evaluation limit to newLimit.  This specifies the number of  instructions to run when the METHOD(run) is called.  The default value for a new interpreter is 200.'''

		breve.breveInternalFunctionFinder.pushEnvironmentSetEvalLimit( self, self.pushInterpreterPointer, newLimit )

	def setListLimit( self, newLimit ):
		'''Sets the size limit of lists constructed in push.  This limit also applies to programs created using genetic operators.  The default limit is 200.'''

		breve.breveInternalFunctionFinder.pushEnvironmentSetListLimit( self, self.pushInterpreterPointer, newLimit )

	def step( self, steps ):
		'''Steps the interpreter forward by the specified number of instructions.'''

		self.effort = ( self.effort + breve.breveInternalFunctionFinder.pushStep( self, self.pushInterpreterPointer, steps ) )
		return ( not breve.breveInternalFunctionFinder.pushInterpreterDone( self, self.pushInterpreterPointer ) )



breve.PushInterpreter = PushInterpreter

class PushProgram( breve.Object ):
	'''Summary: a single program in the Push programming language. <P> A PushProgram object is a program in the Push programming language.  To  be used in conjunction with the class OBJECT(PushInterpreter).'''

	def __init__( self ):
		breve.Object.__init__( self )
		self.codePointer = None

	def copyFrom( self, otherProgram ):
		'''Copies the contents of otherProgram to this object.'''

		self.parse( otherProgram.getString() )

	def crossover( self, p1, p2, interpreter ):
		'''Sets this object to a genetic crossover of p1 and p2.'''

		self.setCodePointer( breve.breveInternalFunctionFinder.pushCodeCrossover( self, interpreter.getInterpreterPointer(), p1.getCodePointer(), p2.getCodePointer() ) )

	def deleteRandomSubtree( self ):
		'''Removes a random subtree from the code.'''

		self.setCodePointer( breve.breveInternalFunctionFinder.pushCodeDeletionMutate( self, self.getCodePointer() ) )

	def destroy( self ):
		''''''

		if self.codePointer:
			breve.breveInternalFunctionFinder.pushCodeFree( self, self.codePointer )


	def flattenRandomSubtree( self ):
		'''Flattens a random subtree in the code.'''

		self.setCodePointer( breve.breveInternalFunctionFinder.pushCodeFlattenMutate( self, self.getCodePointer() ) )

	def getCodePointer( self ):
		'''Used internally.'''

		return self.codePointer


	def getDiscrepancy( self, otherProgram ):
		'''Determines the discrepancy between this program and otherProgram. This is defined as the number of points (atoms and subtrees) found in this program which do not exist in otherProgram, plus the number of points found in otherProgram that do not exist in this program. <P> This method considers the order of points in a program, and  considers all sublists found in a program.  For this reason, it is a  good measure of similarity between two evolved programs.  For a  simpler comparison operator, suitable for comparing lists in which order should be preserved, see METHOD(get-top-level-difference).'''

		return breve.breveInternalFunctionFinder.pushCodeDiscrepancy( self, self.codePointer, otherProgram.getCodePointer() )


	def getFirstFloat( self ):
		'''Returns the first float value found in the Push program.  '''

		return breve.breveInternalFunctionFinder.pushCodeFirstFloat( self, self.codePointer )


	def getList( self ):
		'''Returns a list representation of this push program.'''

		if ( not self.codePointer ):
			return []


		return breve.breveInternalFunctionFinder.pushCodeGetEvalList( self, self.codePointer )


	def getSize( self ):
		'''Returns the size of this push program.'''

		return breve.breveInternalFunctionFinder.pushCodeSize( self, self.codePointer )


	def getString( self ):
		'''Returns the string representation of this push program.'''

		if ( not self.codePointer ):
			return '''()'''


		return breve.breveInternalFunctionFinder.pushCodeGetString( self, self.codePointer )


	def getTopLevelDifference( self, otherProgram ):
		'''Determines the top level difference between this program and otherProgram. This is calculated by comparing each element of the program against  the corresponding element of otherProgram, with every mismatch counting  as one point of difference.  Sublists are not considered; each element  in the top level list is treated as an atomic element. <P> This method is well suited for comparing lists produced as output of  push programs.  Because it does not consider sublists or order, it is  not well suited for determining the true level of similarity between two programs as one might with to do with evolved code.  For that type of functionality, see the method METHOD(get-discrepancy).'''

		return breve.breveInternalFunctionFinder.pushCodeTopLevelDiff( self, self.codePointer, otherProgram.getCodePointer() )


	def getTopLevelSize( self ):
		'''Returns the "top-level" size of this push program, meaning that sublists are counted as one item, and not counted recursively.'''

		return breve.breveInternalFunctionFinder.pushCodeTopLevelSize( self, self.codePointer )


	def makeRandomCode( self, size, interpreter ):
		'''Makes a random push program of length size.  Size is an optional argument with a default value of 100.  The size may also be equal to -1, which indicates  that the MAX-POINTS-IN-RANDOM-EXPRESSION value from the Push configuration file should be used.  The interpreter provided specifies the instruction set used to generate the random code. '''

		if ( size == -1 ):
			size = breve.breveInternalFunctionFinder.pushEnvironmentGetRandomPointLimit( self, interpreter.getInterpreterPointer() )

		if ( size < 1 ):
			size = 1

		self.setCodePointer( breve.breveInternalFunctionFinder.pushCodeRandom( self, interpreter.getInterpreterPointer(), size ) )

	def mutate( self, interpreter, leafBias, sizeScale, ignored ):
		'''Mutates this code object by adding a new subtree.  The maximum size of the subtree is specified in the optional argument treeSize, which has a default value of 50.  The size may also be equal to -1 (the default), indicating that the  MAX-POINTS-IN-RANDOM-EXPRESSION value from the Push configuration file  should be used.  The interpreter argument determines the instruction set used to generate the random code.   <P> Mutation is implemented as "fair" mutation with a range of +/- 30% of the  mutated subtree. <P> The max-new-code-size argument is ignored, but included for backwards compatibility.'''

		self.setCodePointer( breve.breveInternalFunctionFinder.pushCodeSubtreeMutate( self, interpreter.getInterpreterPointer(), self.codePointer, leafBias, sizeScale ) )

	def parse( self, codeString ):
		'''Sets the content of this code to the newly parsed string. The with-interpreter keyword is no longer required.'''

		self.setCodePointer( breve.breveInternalFunctionFinder.pushParse( self, codeString ) )
		return self


	def read( self, fileName ):
		'''Reads a push program from a text file.'''

		f = breve.File()
		f.openForReading( fileName )
		self.parse( f.readAsString() )
		del f

	def setCodePointer( self, newCodePointer ):
		'''Used internally.'''

		if self.codePointer:
			breve.breveInternalFunctionFinder.pushCodeFree( self, self.codePointer )

		if ( not newCodePointer ):
			self.parse( '''()''' )
		else:
			self.codePointer = newCodePointer

		return self


	def swapSublists( self, x, y ):
		'''Swaps the positions of the top level sublists x and y.'''

		return breve.breveInternalFunctionFinder.pushCodeSwapSublists( self, self.codePointer, x, y )


	def write( self, fileName ):
		'''Writes this push program to a text file.'''

		f = breve.File()
		f.openForWriting( fileName )
		f.write( self.getString() )
		del f


breve.PushProgram = PushProgram

class PushDiversityPool( breve.Object ):
	'''Summary: a tool to test diversity in populations of PushProgram objects. <P> The PushDiversityPool allows you to determine the level of genotypic diversity of PushProgram objects, typically individuals in a genetic programming or artificial life experiment. <P> Diversity is detemined as the number of unique species which differ  from one another by at least a certain tolerance value.  "Difference" is defined by the algorithm outlined in  OBJECTMETHOD(PushProgram:get-discrepancy).  No single species will  contain any two individuals which differ by more than the specified tolerance. <P> The tolerance can be set with the method METHOD(set-tolerance).   Programs can then be added using METHOD(add).  The number of species can then be retreived using METHOD(get-size).  The method METHOD(clear) allows the diversity pool to be emptied and reused.'''

	def __init__( self ):
		breve.Object.__init__( self )
		self.poolPointer = None
		PushDiversityPool.init( self )

	def add( self, p ):
		'''Adds a program to the diversity pool.'''

		breve.breveInternalFunctionFinder.pushDiversityPoolAdd( self, self.poolPointer, p.getCodePointer() )

	def clear( self ):
		'''Clears the diversity pool so that it may be reused.'''

		breve.breveInternalFunctionFinder.pushDiversityPoolClear( self, self.poolPointer )

	def destroy( self ):
		''''''

		breve.breveInternalFunctionFinder.pushDiversityPoolFree( self, self.poolPointer )

	def getSize( self ):
		'''Returns the number of unique diversity "species" in the pool.'''

		return breve.breveInternalFunctionFinder.pushDiversityPoolGetCount( self, self.poolPointer )


	def init( self ):
		''''''

		self.poolPointer = breve.breveInternalFunctionFinder.pushDiversityPoolNew( self, 10 )

	def setTolerance( self, t ):
		'''Sets the tolerance of the diversity pool to t.  When the tolerance is set, it does not effect species counts for programs already  added.  When you change the tolerance, you must remove all program objects (using METHOD(clear)) and add them again. <P>  The default tolerance level is 10.'''

		breve.breveInternalFunctionFinder.pushDiversityPoolSetTolerance( self, self.poolPointer, t )


breve.PushDiversityPool = PushDiversityPool


