
# Note: this file was automatically converted to Python from the
# original steve-language source code.  Please see the original
# file for more detailed comments and documentation.


import breve

class QSystem( breve.Object ):
	'''The main interface to the qgame++ Quantum Gate And Measurement Emulator,  a quantum computing simulator.  <p><b>Note: the QGAME library is not built-in to the standard breve  distribution.  Use of this class may require compiling breve from the  source code with support for QGAME.'''

	def __init__( self ):
		breve.Object.__init__( self )
		self.systemPointer = None
		QSystem.init( self )

	def destroy( self ):
		breve.breveInternalFunctionFinder.qsysFree( self, self.systemPointer )

	def init( self ):
		self.systemPointer = breve.breveInternalFunctionFinder.qsysNew( self)

	def test( self, theProgram, testCases, t, qb, finalQB ):
		'''Runs a series of test cases using qb qubits, specified by testCases,  on theProgram with qb qubits and a threshold of t. <P> This call returns a list of 5 items, which are to be interpreted as: <li> Number of misses <li> Maximum error <li> Average error <li> Maximum number of oracle calls <li> Average number of oracle calls <P> If an error occurs during the execution of the program, all of the  elements of the returned list are set to -1.'''

		return breve.breveInternalFunctionFinder.qsysTestProgram( self, self.systemPointer, theProgram.getProgramPointer(), testCases, qb, finalQB, t )


breve.QSystem = QSystem
class QProgram( breve.Object ):
	'''An interface to a quantum program in qgame++ Quantum Gate And Measurement Emulator,  a quantum computing simulator.  To be used in conjunction with OBJECT(QSystem). <p><b>Note: the QGAME library is not built-in to the standard breve  distribution.  Use of this class may require compiling breve from the  source code with support for QGAME.'''

	def __init__( self ):
		breve.Object.__init__( self )
		self.programPointer = None
		QProgram.init( self )

	def addInstruction( self, instructionString ):
		'''Adds an instruction to the program parsed from instructionString. '''

		if breve.breveInternalFunctionFinder.qprogramAddInstruction( self, self.programPointer, instructionString ):
			print '''warning: error adding QProgram instruction "%s"''' % (  instructionString )


	def clear( self ):
		'''Removes all instructions from this program.'''

		breve.breveInternalFunctionFinder.qprogramClear( self, self.programPointer )

	def destroy( self ):
		breve.breveInternalFunctionFinder.qprogramFree( self, self.programPointer )

	def getProgramPointer( self ):
		return self.programPointer

	def getString( self ):
		'''Returns the text for this program.'''

		return breve.breveInternalFunctionFinder.qprogramGetString( self, self.programPointer )

	def init( self ):
		self.programPointer = breve.breveInternalFunctionFinder.qprogramNew( self)


breve.QProgram = QProgram



