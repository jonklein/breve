
import breve

class Graph( breve.Abstract ):
	'''Work in progress--not yet supported.'''

	__slots__ = [ 'graphPointer', ]

	def __init__( self ):
		breve.Abstract.__init__( self )
		self.graphPointer = None
		Graph.init( self )

	def addLine( self, lineColor ):
		'''Adds a line to the graph with the color lineColor.  This  method returns an integer corresponding to the new line.   You will need to use this number to add values to the line later on using METHOD(add-point).'''


		return breve.breveInternalFunctionFinder.graphAddLine( self, self.graphPointer, lineColor )


	def addPoint( self, lineNumber, yValue ):
		'''Adds a point on the line lineNumber, with the Y-value yValue.'''


		breve.breveInternalFunctionFinder.graphAddLineValue( self, self.graphPointer, lineNumber, yValue )

	def init( self ):
		''''''


		self.graphPointer = breve.breveInternalFunctionFinder.graphNew( self, '''Graph''' )

	def setTitle( self, title ):
		'''Sets the title of the graph to title.'''


		breve.breveInternalFunctionFinder.graphSetTitle( self, self.graphPointer, title )

	def setXAxisName( self, xName ):
		'''Sets the X-axis label to xName.'''


		breve.breveInternalFunctionFinder.graphSetXAxisName( self, self.graphPointer, xName )

	def setYAxisName( self, yName ):
		'''Sets the Y-axis label to yName.'''


		breve.breveInternalFunctionFinder.graphSetYAxisName( self, self.graphPointer, yName )


breve.Graph = Graph



