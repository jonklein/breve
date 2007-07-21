
import breve

class GADirectedGraph( breve.GeneticAlgorithmIndividual ):
	'''GADirectedGraph is a special subclass of OBJECT(GeneticAlgorithmIndividual)  which maages directed networks of nodes for evolution.  The primary use of these networks is to evolve creature morphologies.  The networks are based on the the directed graphs used by Karl Sims in "Evolving Virtual  Creatures" (1994).  Multiple connections may exist between nodes, and nodes  may have connections back to themselves. <P> GAGraph makes use of a number of other classes.  The GAGraph itself holds  a reference to the graph's root node, which is an object of  OBJECT(GADirectedGraphNode).  This root node may be connected directly or  indirectly to any other number of OBJECT(GADirectedGraphNode) objects, using the connection class OBJECT(GADirectedGraphConnection). <P> Because OBJECT(GADirectedGraphNode) and OBJECT(GADirectedGraphConnection)  are intended to be used as general evolutionary objects, it is often  neccessary to associate a set of parameters with these objects, above  and beyond the information coded in the connected graph.  For this reason, both classes are subclasses of an object which can hold a list of parameters, OBJECT(GADirectedGraphParameterObject). <P> OBJECT(GADirectedGraphParameterObject) holds a list of float values between -1.0 and 1.0.  These parameters can be interpreted in whatever way the  user desires.  In the example of directed graphs being used to specify creature morphologies, the parameters hold information about the size and orientation of limbs relative to one another.'''

	def __init__( self ):
		breve.GeneticAlgorithmIndividual.__init__( self )
		self.rootNode = None
		GADirectedGraph.init( self )

	def copy( self, other ):
		'''Copies the contents of the directed graph other.'''


		self.deleteRootNode()
		self.rootNode = other.getRoot().duplicate()
		self.addDependency( self.rootNode )

	def crossover( self, p1, p2 ):
		'''Sets this child's genome to be a genetic crossover of p1 and p2.'''

		positions = {}
		keep = {}
		childnodes = breve.objectList()
		nodes2 = breve.objectList()
		nodes1 = breve.objectList()
		source = None
		temp = None
		connection = None
		index = 0
		n = 0
		cross = 0
		max = 0
		first = 0

		self.deleteRootNode()
		if breve.randomExpression( 1 ):
			temp = p1
			p1 = p2
			p2 = temp


		nodes1 = p1.getRoot().getConnectedNodes()
		nodes2 = p2.getRoot().getConnectedNodes()
		n = 0
		while ( n < breve.length( nodes1 ) ):
			positions[ nodes1[ n ] ] = n
			n = ( n + 1 )

		n = 0
		while ( n < breve.length( nodes2 ) ):
			positions[ nodes2[ n ] ] = n
			n = ( n + 1 )

		first = breve.randomExpression( 1 )
		max = breve.length( nodes2 )
		cross = breve.randomExpression( ( max - 1 ) )
		if ( cross >= breve.length( nodes1 ) ):
			cross = breve.length( nodes1 )

		if ( max < 2 ):
			childnodes.append( breve.createInstances( breve.GADirectedGraphNodes, 1 ) )

		else:
			childnodes = breve.createInstances( breve.GADirectedGraphNodes, max )

		n = 0
		while ( n < max ):
			if ( n < cross ):
				source = nodes1[ n ]
			else:
				source = nodes2[ n ]

			childnodes[ n ].setParameters( source.getParameters() )
			for connection in source.getConnections():
				index = positions[ connection.getTarget() ]
				if ( index >= breve.length( childnodes ) ):
					index = breve.randomExpression( ( breve.length( childnodes ) - 1 ) )


				childnodes[ n ].connect( childnodes[ index ], connection.getParameters() )



			n = ( n + 1 )

		self.rootNode = self.pickRootNode( childnodes )
		if ( not self.rootNode ):
			self.rootNode = childnodes[ 0 ]


		keep[ self.rootNode ] = 1
		for temp in self.rootNode.getConnectedNodes():
			keep[ temp ] = 1


		for temp in childnodes:
			if ( not keep[ temp ] ):
				print '''freeing ''', temp
				breve.deleteInstances( temp )




		self.addDependency( self.rootNode )

	def deleteRootNode( self ):
		'''Deletes the root node and all connected objects.  This method "clears" the GADirectedGraph so that it may be reused.'''


		if self.rootNode:
			self.removeDependency( self.rootNode )
			self.rootNode.destroyConnectedObjects()
			breve.deleteInstances( self.rootNode )


		self.rootNode = 0

	def destroy( self ):

		self.deleteRootNode()

	def getRoot( self ):
		'''Returns the root node of the directed graph.'''


		return self.rootNode

	def init( self ):

		self.rootNode = breve.createInstances( breve.GADirectedGraphNode, 1 )
		self.addDependency( self.rootNode )

	def mutate( self ):
		'''Mutates the directed graph.'''


		self.rootNode.mutate()

	def pickRootNode( self, nodes ):
		node = None
		best = None
		bestCount = 0

		for node in nodes:
			if ( breve.length( node.getConnectedNodes() ) > bestCount ):
				best = node
				bestCount = node.getConnectedNodes()




		return best

	def printGraph( self ):
		'''Prints out a text representation of the network.'''


		print '''#### Graph %s''' % (  self )
		self.rootNode.printConnectedObjects()

	def randomize( self, nMax, np, cp, p = 0.500000 ):
		'''Randomizes this genome by creating a directed graph containing up to  nMax nodes.  Sets the number of node and connection parameters to np  and cp, respectively.  The optional connection probability, p, specifies the probability that a connection will be made from one node to another.'''

		count = 0
		nodes = breve.objectList()
		m = None
		n = None
		connection = None
		keep = {}

		self.deleteRootNode()
		count = ( breve.randomExpression( ( nMax - 1 ) ) + 1 )
		if ( count < 2 ):
			count = ( count + 1 )

		nodes = breve.createInstances( breve.GADirectedGraphNodes, count )
		nodes.setParameterLength( np )
		nodes.randomizeParameters()
		for n in nodes:
			for m in nodes:
				if ( ( n != m ) and ( breve.randomExpression( 1.000000 ) < p ) ):
					connection = n.connect( m )
					connection.setParameterLength( cp )
					connection.randomizeParameters()






		self.rootNode = self.pickRootNode( nodes )
		if ( not self.rootNode ):
			self.rootNode = breve.createInstances( breve.GADirectedGraphNode, 1 )
			self.rootNode.setParameterLength( np )
			self.rootNode.randomizeParameters()


		for n in self.rootNode.getConnectedNodes():
			keep[ n ] = 1


		for n in nodes:
			if ( not keep[ n ] ):
				breve.deleteInstances( n )



		self.addDependency( self.rootNode )


breve.GADirectedGraph = GADirectedGraph
class GADirectedGraphParameterObject( breve.Abstract ):
	'''A GADirectedGraphParameterObject is a subclass of object which holds a parameter list for use with the OBJECT(GADirectedGraphNode) and OBJECT(GADirectedGraphConnection) objects.  The elements in the parameter list are to be interpreted as evolving genome values which control how the graph nodes and connections are used.'''

	def __init__( self ):
		breve.Abstract.__init__( self )
		self.parameters = breve.objectList()
		GADirectedGraphParameterObject.init( self )

	def getParameters( self ):
		'''Returns the parameter list.'''


		return self.parameters

	def init( self ):

		pass

	def mutate( self, variation = 0.100000, replace = 0.100000 ):
		'''Mutates the list of parameters in the list with a gaussian distribution  of size variation, and a replace proability chance of total replacement.'''

		n = 0

		n = 0
		while ( n < breve.length( self.parameters ) ):
			self.parameters[ n ] = ( self.parameters[ n ] + ( breve.breveInternalFunctionFinder.randomGauss( self) * variation ) )
			if ( self.parameters[ n ] < -1.000000 ):
				self.parameters[ n ] = -1.000000

			if ( self.parameters[ n ] > 1.000000 ):
				self.parameters[ n ] = 1.000000

			if ( breve.randomExpression( 1.000000 ) < replace ):
				self.parameters[ n ] = ( breve.randomExpression( 2.000000 ) - 1.000000 )


			n = ( n + 1 )


	def randomizeParameters( self ):
		'''Randomizes all of the parameters in the list to values between -1.0 and 1.0.'''

		n = 0

		n = 0
		while ( n < breve.length( self.parameters ) ):
			self.parameters[ n ] = ( breve.randomExpression( 2.000000 ) - 1.000000 )

			n = ( n + 1 )


	def setParameterLength( self, length ):
		'''Extends the size of the parameter list include at least length items by adding  zeros to the end of the list.'''


		while ( breve.length( self.parameters ) < length ):
			self.parameters.append( 0 )


	def setParameters( self, plist ):
		'''Sets the parameter list to plist.'''


		self.parameters = list( plist) 


breve.GADirectedGraphParameterObject = GADirectedGraphParameterObject
class GADirectedGraphNode( breve.GADirectedGraphParameterObject ):
	'''OBJECT(GADirectedGraphNode) is a node in a directed graph.  It is a subclass of  OBJECT(GADirectedGraphParameterObject) which can be used to associate it with  a set of parameters.'''

	def __init__( self ):
		breve.GADirectedGraphParameterObject.__init__( self )
		self.connections = breve.objectList()

	def connect( self, child ):
		'''Makes a connection to child with an empty parameter list.'''


		return self.connect( child, [] )

	def connect( self, child, plist ):
		'''Makes a connection to child with the parameter list plist.'''

		connection = None

		connection = breve.createInstances( breve.GADirectedGraphConnection, 1 )
		connection.setTarget( child )
		connection.setParameters( plist )
		self.addDependency( connection )
		self.connections.append( connection )
		return connection

	def destroy( self ):

		breve.deleteInstances( self.connections )

	def destroyConnectedObjects( self ):
		'''Initiates a recursive freeing of this object and all connected objects, both directly and indiretly connected.'''

		h = {}

		self.internalDestroyConnectedObjects( h )

	def duplicate( self ):
		'''Duplicates the object, which triggers recursive duplication of all connected objects.  To be used on root nodes only.'''

		h = {}

		return self.duplicate( h )

	def duplicate( self, seenhash ):
		connection = None
		newNode = None

		if seenhash[ self ]:
			return seenhash[ self ]

		newNode = breve.createInstances( breve.GADirectedGraphNode, 1 )
		seenhash[ self ] = newNode
		for connection in self.connections:
			newNode.connect( connection.getTarget().duplicate( seenhash ), connection.getParameters() )


		newNode.setParameters( self.getParameters() )
		return newNode

	def getConnectedNodes( self ):
		'''Recursively generates a list of all nodes connected to this  root node, both directly and indirectly.'''

		h = {}

		return self.internalGetConnectedNodes( h, [] )

	def getConnections( self ):
		'''Returns the list of connections from this node.'''


		return self.connections

	def internalDestroyConnectedObjects( self, seenhash ):
		connection = None

		if seenhash[ self ]:
			return

		seenhash[ self ] = 1
		for connection in self.connections:
			if connection.getTarget():
				connection.getTarget().internalDestroyConnectedObjects( seenhash )

			breve.deleteInstances( connection )


		breve.deleteInstances( self )

	def internalGetConnectedNodes( self, seenhash, seenlist ):
		connection = None
		nodelist = breve.objectList()

		if seenhash[ self ]:
			return []

		seenlist.append( self )
		seenhash[ self ] = 1
		for connection in self.connections:
			if connection.getTarget():
				connection.getTarget().internalGetConnectedNodes( seenhash, seenlist )



		return seenlist

	def mutate( self ):
		'''Mutates the object, which triggers recursive mutation of all connected objects.  To be used on root nodes only.'''

		h = {}

		return self.mutate( h )

	def mutate( self, seenhash ):
		connection = None

		if seenhash[ self ]:
			return

		seenhash[ self ] = 1
		breve.GADirectedGraphParameterObject.mutate( self )
		for connection in self.connections:
			connection.mutate()
			connection.getTarget().mutate( seenhash )



	def printConnectedObjects( self ):
		'''Initiates a recursive printing of all connected objects in the graph.'''

		h = {}

		self.printConnectedObjects( h )

	def printConnectedObjects( self, seenhash ):
		connection = None

		if seenhash[ self ]:
			return

		seenhash[ self ] = 1
		print '''## %s parameters %s''' % (  self, self.parameters )
		for connection in self.connections:
			print '''## %s connected to ''' % (  self ), connection.getTarget(), ''' with parameters: ''', connection.getParameters()
			connection.getTarget().printConnectedObjects( seenhash )




breve.GADirectedGraphNode = GADirectedGraphNode
class GADirectedGraphConnection( breve.GADirectedGraphParameterObject ):
	'''A OBJECT(GADirectedGraphConnection) is a connection from one  OBJECT(GADirectedGraphNode) to another.  As a subclass of  OBJECT(GADirectedGraphParameterObject), it can be associated  with a list of parameters.'''

	def __init__( self ):
		breve.GADirectedGraphParameterObject.__init__( self )
		self.target = None

	def getTarget( self ):
		'''Returns the target of this connection.'''


		return self.target

	def setTarget( self, t ):
		'''Sets the target of this connection to t.'''


		self.addDependency( t )
		self.target = t


breve.GADirectedGraphConnection = GADirectedGraphConnection
# Add our newly created classes to the breve namespace

breve.GADirectedGraphParameterObjects = GADirectedGraphParameterObject
breve.GADirectedGraphNodes = GADirectedGraphNode
breve.GADirectedGraphConnections = GADirectedGraphConnection



