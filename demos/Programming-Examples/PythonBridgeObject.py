import breve

class PythonBridgeObject( breve.object ):
	def __init__( self ):
		breve.object.__init__( self )
		print "Inited Python bridge object"

	def iterate( self ):
		print "Iterating Python bridge object"
