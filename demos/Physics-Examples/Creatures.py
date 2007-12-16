
# Note: this file was automatically converted to Python from the
# original steve-language source code.  Please see the original
# file for more detailed comments and documentation.


import breve

class VirtualCreatures( breve.PhysicalControl ):
	def __init__( self ):
		breve.PhysicalControl.__init__( self )
		self.blockTexture = None
		self.body = None
		self.flag = None
		self.floor = None
		self.ga = None
		self.parser = None
		self.running = 0
		self.startTexture = None
		self.startlocation = breve.vector()
		VirtualCreatures.init( self )

	def checkPenetration( self ):
		link = None

		self.running = 1
		if self.body.checkSelfPenetration():
			breve.deleteInstances( breve.allInstances( "Links" ) )

		breve.allInstances( "SineJoints" ).activate()
		self.startlocation = breve.vector( 0, 0, 0 )
		for link in breve.allInstances( "Links" ):
			self.startlocation = ( self.startlocation + link.getLocation() )


		if breve.length( breve.allInstances( "Links" ) ):
			self.startlocation = ( self.startlocation / breve.length( breve.allInstances( "Links" ) ) )

		self.flag.move( ( self.startlocation - breve.vector( 0, ( self.startlocation.y - 2 ), 0 ) ) )

	def getBlockTexture( self ):
		return self.blockTexture

	def getCurrentCritterFitness( self ):
		t = breve.vector()
		link = None

		for link in breve.allInstances( "Links" ):
			t = ( t + link.getLocation() )


		if breve.length( breve.allInstances( "Links" ) ):
			t = ( t / breve.length( breve.allInstances( "Links" ) ) )

		return breve.length( ( t - self.startlocation ) )

	def init( self ):
		self.startTexture = breve.createInstances( breve.Image, 1 ).load( 'images/star.png' )
		self.flag = breve.createInstances( breve.Mobile, 1 )
		self.flag.disablePhysics()
		self.flag.setBitmapImage( self.startTexture )
		self.flag.setColor( breve.vector( 1, 1, 0 ) )
		self.flag.setLabel( 'Start' )
		self.flag.disableShadows()
		self.setBackgroundTextureImage( breve.createInstances( breve.Image, 1 ).load( 'images/clouds.png' ) )
		self.blockTexture = breve.createInstances( breve.Image, 1 ).load( 'images/noise.png' )
		self.floor = breve.createInstances( breve.Floor, 1 )
		self.floor.setTextureImage( breve.createInstances( breve.Image, 1 ).load( 'images/ground.png' ) )
		self.floor.setColor( breve.vector( 0.400000, 0.300000, 0.300000 ) )
		self.setSkyboxImages( [ 'images/MountainBoxFront.png', 'images/MountainBoxBack.png', 'images/MountainBoxLeft.png', 'images/MountainBoxRight.png', 'images/MountainBoxTop.png', 'images/MountainBoxBottom.png' ] )
		self.enableFastPhysics()
		self.setFastPhysicsIterations( 20 )
		self.enableShadowVolumes()
		self.parser = breve.createInstances( breve.MorphologyParser, 1 )
		self.ga = self.controller.dearchiveXml( 'breveCreatures.xml' )
		if ( not self.ga ):
			self.ga = breve.createInstances( breve.SimsGA, 1 )
			self.ga.setFitnessTestDuration( 30 )
			self.ga.setIndividualClass( 'SimsGAIndividual' )
			self.ga.setPopulationSize( 25 )
			self.ga.setCrossoverPercent( 30 )
			self.ga.setMutationPercent( 30 )
			self.ga.setSpatialRadius( 4 )


		self.addDependency( self.ga )
		self.updateText()

	def iterate( self ):
		t = breve.vector()
		dist = 0
		link = None

		for link in breve.allInstances( "Links" ):
			t = ( t + link.getLocation() )


		if breve.length( breve.allInstances( "Links" ) ):
			t = ( t / breve.length( breve.allInstances( "Links" ) ) )

		self.pointCamera( t )
		dist = breve.length( ( t - self.startlocation ) )
		if self.running:
			self.setDisplayText( '''Distance traveled: %s''' % (  dist ), -0.950000, -0.950000 )

		self.pivotCamera( 0, 0.000100 )
		breve.PhysicalControl.iterate( self )

	def setupTest( self, i ):
		l = None
		low = breve.vector()
		lowest = 0

		for l in breve.allInstances( "Links" ):
			breve.deleteInstances( l.getShape() )

		breve.deleteInstances( breve.allInstances( "Links" ) )
		breve.deleteInstances( breve.allInstances( "Joints" ) )
		if self.body:
			breve.deleteInstances( self.body )

		self.body = breve.createInstances( breve.MultiBody, 1 )
		self.body.setRoot( self.parser.parseTopLevel( i.getGenome().getRoot() ) )
		self.body.disableSelfCollisions()
		self.addDependency( self.body )
		self.schedule( 'checkPenetration', ( self.getTime() + 8.000000 ) )
		self.setDisplayText( '''Preparing to test %s...''' % (  i ), -0.950000, -0.950000 )
		self.updateText()
		self.running = 0

	def updateText( self ):
		index = 0
		total = 0
		gen = 0
		bestDistance = 0

		if self.ga:
			gen = ( self.ga.getGeneration() + 1 )
			bestDistance = self.ga.getBestFitness()
			total = self.ga.getPopulationSize()
			index = ( self.ga.getCurrentIndividualIndex() + 1 )


		self.setDisplayText( '''Generation: %s (%s/%s)''' % (  gen, index, total ), -0.950000, -0.750000, 1 )
		self.setDisplayText( '''Best distance: %s''' % (  bestDistance ), -0.950000, -0.850000, 2 )


breve.VirtualCreatures = VirtualCreatures
class SimsGA( breve.GeneticAlgorithm ):
	def __init__( self ):
		breve.GeneticAlgorithm.__init__( self )
		self.url = None
		SimsGA.init( self )

	def endFitnessTest( self, o ):
		o.setFitness( self.controller.getCurrentCritterFitness() )
		if breve.breveInternalFunctionFinder.isnan( self, o.getFitness() ):
			o.setFitness( 0 )

		print '''fitness of %s: ''' % (  o ), o.getFitness()
		self.archiveAsXml( 'breveCreatures.xml' )

	def endGeneration( self, n ):
		print self.url.put( self.getBestIndividual().archiveAsXmlString(), 'http://www.deskworld.org/original/dwCritterUpload.php' )

	def init( self ):
		self.url = breve.createInstances( breve.URL, 1 )
		self.addDependency( self.url )

	def startFitnessTest( self, o ):
		newOffset = breve.vector()

		newOffset = ( breve.randomExpression( breve.vector( 40, 2, 40 ) ) + breve.vector( -20, 0.150000, -20 ) )
		if ( breve.length( newOffset ) < 20 ):
			newOffset = ( ( 20 * newOffset ) / breve.length( newOffset ) )

		self.controller.panCameraOffset( newOffset, 200 )
		self.controller.setupTest( o )


breve.SimsGA = SimsGA
class SimsGAIndividual( breve.GeneticAlgorithmIndividual ):
	def __init__( self ):
		breve.GeneticAlgorithmIndividual.__init__( self )
		self.genome = None
		self.h = breve.hash()
		SimsGAIndividual.init( self )

	def copy( self, other ):
		self.genome.copy( other.getGenome() )

	def crossover( self, p1, p2 ):
		self.genome.crossover( p1.getGenome(), p2.getGenome() )

	def destroy( self ):
		breve.deleteInstances( self.genome )

	def getGenome( self ):
		return self.genome

	def init( self ):
		self.genome = breve.createInstances( breve.GADirectedGraph, 1 )
		self.randomize()
		self.addDependency( self.genome )

	def mutate( self ):
		self.genome.mutate()

	def randomize( self ):
		self.genome.randomize( 5, 10, 10 )


breve.SimsGAIndividual = SimsGAIndividual
class MorphologyParser( breve.Object ):
	def __init__( self ):
		breve.Object.__init__( self )

	def createNode( self, root, n ):
		rootNode = None
		nodeParams = breve.objectList()
		size = breve.vector()

		if ( n > 3 ):
			return 0

		rootNode = breve.createInstances( breve.Link, 1 )
		rootNode.move( breve.vector( 0, 15, 0 ) )
		rootNode.setColor( ( ( ( 5 - n ) / 4.000000 ) * breve.vector( 1, 1, 1 ) ) )
		nodeParams = root.getParameters()
		size = ( 6 * breve.vector( breve.length( nodeParams[ 0 ] ), breve.length( nodeParams[ 1 ] ), breve.length( nodeParams[ 2 ] ) ) )
		rootNode.setColor( breve.vector( breve.length( nodeParams[ 3 ] ), breve.length( nodeParams[ 4 ] ), breve.length( nodeParams[ 5 ] ) ) )
		rootNode.setShape( breve.createInstances( breve.Cube, 1 ).initWith( ( ( ( 4 - n ) / 3.000000 ) * size ) ) )
		rootNode.setTextureImage( self.controller.getBlockTexture() )
		return rootNode

	def parse( self, root, n, rootNode, f = 0.000000 ):
		connections = breve.objectList()
		nodeParams = breve.objectList()
		connectionParams = breve.objectList()
		joint = None
		childNode = None
		child = None
		norm = breve.vector()
		size = breve.vector()
		point = breve.vector()
		ppoint = breve.vector()
		cpoint = breve.vector()
		jointRange = 0

		connections = root.getConnections()
		nodeParams = root.getParameters()
		size = ( 4 * breve.vector( breve.length( nodeParams[ 0 ] ), breve.length( nodeParams[ 1 ] ), breve.length( nodeParams[ 2 ] ) ) )
		if ( ( n > 3 ) or ( ( n > 1 ) and ( nodeParams[ 9 ] < 0.000000 ) ) ):
			return 0

		for child in connections:
			childNode = self.createNode( child.getTarget(), ( n + 1 ) )
			if childNode:
				connectionParams = child.getParameters()
				point = breve.vector( connectionParams[ 0 ], connectionParams[ 1 ], connectionParams[ 2 ] )
				if f:
					point.x = ( point.x * -1 )

				point.x = ( point.x * size.x )
				point.y = ( point.y * size.y )
				point.z = ( point.z * size.z )
				cpoint = childNode.getShape().getPointOnShape( point )
				ppoint = rootNode.getShape().getPointOnShape( ( -point ) )
				norm = breve.vector( connectionParams[ 4 ], connectionParams[ 5 ], connectionParams[ 6 ] )
				norm = ( norm / breve.length( norm ) )
				if f:
					norm.x = ( norm.x * -1 )

				joint = breve.createInstances( breve.SineJoint, 1 )
				joint.link( norm, ppoint, cpoint, childNode, rootNode )
				joint.setPhaseshift( ( 3.140000 * connectionParams[ 7 ] ) )
				joint.setFrequency( ( 1 + connectionParams[ 8 ] ) )
				jointRange = ( 0.700000 + ( 2 * breve.length( connectionParams[ 9 ] ) ) )
				joint.setDoubleSpring( 1000, jointRange, ( -jointRange ) )
				joint.setStrengthLimit( 4000 )
				self.parse( child.getTarget(), ( n + 1 ), childNode )




		return rootNode

	def parseTopLevel( self, root ):
		rootNode = None

		rootNode = self.createNode( root, 1 )
		self.parse( root, 1, rootNode )
		return rootNode


breve.MorphologyParser = MorphologyParser
class SineJoint( breve.RevoluteJoint ):
	'''A RevoluteJoint that moves according to a sine function.'''

	def __init__( self ):
		breve.RevoluteJoint.__init__( self )
		self.active = 0
		self.frequency = 0
		self.phaseshift = 0
		SineJoint.init( self )

	def activate( self ):
		self.active = 1

	def init( self ):
		self.phaseshift = 0
		self.frequency = 2.000000

	def iterate( self ):
		if self.active:
			self.setJointVelocity( breve.breveInternalFunctionFinder.sin( self, ( ( self.controller.getTime() * self.frequency ) + self.phaseshift ) ) )



	def setFrequency( self, f ):
		self.frequency = f

	def setPhaseshift( self, p ):
		self.phaseshift = p


breve.SineJoint = SineJoint
# Add our newly created classes to the breve namespace

breve.SineJoints = SineJoint


# Create an instance of our controller object to initialize the simulation

VirtualCreatures()


