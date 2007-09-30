
# Note: this file was automatically converted to Python from the
# original steve-language source code.  Please see the original
# file for more detailed comments and documentation.


import breve

class FFNetwork( breve.NeuralNetwork ):
	'''A new experimental class for simple feed-forward neural networks. This class is now derived from NeuralNetwork.tz which is the base of a complete set of neural network classes.'''

	def __init__( self ):
		breve.NeuralNetwork.__init__( self )
		self.inputLayer = None
		self.layers = breve.objectList()
		self.outputLayer = None

	def addLayer( self, count ):
		'''Adds a layer to the output end of the neural network, with  count neurons.  The first layer added serves as the input layer, and then subsequent layers serve as hidden or output layers.   <p> To create a network with one hidden layer, for example, you  would first call METHOD(add-layer) for the input layer, then the hidden layer, then the output layer.'''

		newLayer = None

		if ( count < 1 ):
			print '''cannot add neural network layer with %s nodes
''' % (  count )
			return


		newLayer = breve.breveInternalFunctionFinder.newFFLayer( self, count, self.outputLayer )
		if ( self.layers == 0 ):
			self.inputLayer = newLayer

		self.layers[ breve.length( self.layers ) ] = newLayer
		self.outputLayer = newLayer

	def destroy( self ):
		breve.breveInternalFunctionFinder.freeNetwork( self, self.outputLayer )

	def getOutput( self, position ):
		'''Returns the value at index position of the output layer.  This  assumes that METHOD(run-with) has already been called to actually run the neural network.'''

		return breve.breveInternalFunctionFinder.getValue( self, self.outputLayer, position )

	def getWeight( self, theLayer, fn, tn ):
		'''Sets the input to the node number tn on layer theLayer, coming from node number fn on layer number theLayer - 1 to weight.  Normally you wouldn't need to use this method, since the method  METHOD(get-output) is the preferred way to retrieve network output.'''

		return breve.breveInternalFunctionFinder.getWeight( self, self.layers[ theLayer ], tn, fn )

	def runWith( self, inputList ):
		'''Sets the inputs of the neural network to the values in inputList, and runs the network.  Output values can then be retrieved with METHOD(get-output).'''

		breve.breveInternalFunctionFinder.feedForward( self, self.outputLayer, inputList )

	def setWeight( self, theLayer, fn, tn, weight ):
		'''Sets the input to the node number tn on layer theLayer, coming from node number fn on layer number theLayer - 1 to weight.'''

		breve.breveInternalFunctionFinder.setWeight( self, self.layers[ theLayer ], tn, fn, weight )


breve.FFNetwork = FFNetwork



