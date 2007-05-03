
import breve

class Shape( breve.Abstract ):
	'''The Shape class is a work-in-progress which allows users to create  shapes which will be associated with OBJECT(Mobile), OBJECT(Stationary) or OBJECT(Link) objects and added to the simulated world.  An instance of the class Shape may be shared by several objects simultaneously. <p> Each Shape has it's own local coordinate frame, with the origin at the middle of the shape.'''

	def __init__( self ):
		breve.Abstract.__init__( self )
		self.density = 0
		self.lastScale = breve.vector()
		self.shapeData = None
		self.shapePointer = None
		Shape.init( self )

	def archive( self ):
		''''''


		self.shapeData = self.getDataForShape()
		return 1

	def dearchive( self ):
		''''''


		self.shapePointer = breve.breveInternalFunctionFinder.shapeForData( self, self.shapeData )
		return breve.Abstract.dearchive( self )

	def destroy( self ):
		''''''


		if self.shapePointer:
			breve.breveInternalFunctionFinder.freeShape( self, self.shapePointer )


	def getDataForShape( self ):
		'''Returns serialized data for the shape (if the shape object has  been properly initialized).  Used for archiving/dearchiving, should generally not be called manually, unless you <i>really</i> know what you're doing.'''


		if self.shapePointer:
			return breve.breveInternalFunctionFinder.dataForShape( self, self.shapePointer )


	def getDensity( self ):
		'''If the shape is properly initialized, this method returns the  shape's density.'''


		if self.shapePointer:
			return breve.breveInternalFunctionFinder.getDensity( self, self.shapePointer )

		return 0.000000

	def getLastScale( self ):
		'''Used internally...'''


		return self.lastScale

	def getMass( self ):
		'''If the shape is properly initialized, this method returns the  shape's mass.'''


		if self.shapePointer:
			return breve.breveInternalFunctionFinder.getMass( self, self.shapePointer )

		return 0.000000

	def getPointOnShape( self, theVector ):
		'''This method is experimental. <p> Starting from inside the shape at the center, this function goes in  the direction of theVector until it hits the edge of the shape. The resulting point is returned.   <p> This allows you to compute link points for arbitrary shapes.  For example, if you want to compute a link point for the  "left-most" point on the shape, you can call this method with (-1, 0, 0).   <p> Returns (0, 0, 0) if the shape is not initialized or if an error occurs.'''


		if self.shapePointer:
			return breve.breveInternalFunctionFinder.pointOnShape( self, self.shapePointer, theVector )
		else:
			return breve.vector( 0, 0, 0 )


	def getPointer( self ):
		'''Returns the shapePointer associated with this Shape object.  This  method is used internally and should not typically be used in  user simulations.'''


		return self.shapePointer

	def init( self ):
		''''''


		self.density = 1.000000

	def initWithCube( self, v ):
		'''Sets this Shape object to a rectangular solid of size v.  '''


		self.shapePointer = breve.breveInternalFunctionFinder.newCube( self, v, self.density )
		return self

	def initWithPolygonCone( self, sideCount, theHeight, theRadius = 1.000000 ):
		'''Sets this Shape object to a cone-like shape with sideCount sides. <p> The distance from the center of the n-gon faces to the vertices  is theRadius.  sides has a maximum value of 99.  Higher values will cause the shape not to be initialized. <p> The height, or depth of the extrusion, is theHeight. <p> This method is experimental, but seems to work okay.  Go figure.'''


		self.shapePointer = breve.breveInternalFunctionFinder.newNGonCone( self, sideCount, theRadius, theHeight, self.density )
		return self

	def initWithPolygonDisk( self, sideCount, theHeight, theRadius = 1.000000 ):
		'''Sets this Shape object to an extruded n-gon of sideCount sides, in other words, a disk with sideCount sides. <p> The distance from the center of the n-gon faces to the vertices  is theRadius.  sides has a maximum value of 99.  Higher values will cause the shape not to be initialized. <p> The height, or depth of the extrusion, is theHeight. <p> This method is experimental, but seems to work okay.  Go figure.'''


		self.shapePointer = breve.breveInternalFunctionFinder.newNGonDisc( self, sideCount, theRadius, theHeight, self.density )
		return self

	def initWithSphere( self, theRadius = 1.000000 ):
		'''Sets this Shape object to a sphere with radius theRadius.'''


		self.shapePointer = breve.breveInternalFunctionFinder.newSphere( self, theRadius, self.density )
		return self

	def scale( self, scale ):
		'''If the shape is <i>not</i> a sphere, scales the shape by the x, y  and z elements of scale.  If the shape <i>is</i> a sphere, scales  the shape by only the x element such that the shape always remains  spherical. <p> After the size has been changed, the instances announces a  "size-changed" notification.'''


		breve.breveInternalFunctionFinder.scaleShape( self, self.shapePointer, scale )
		self.lastScale = scale
		self.announce( 'sizeChanged' )

	def setDensity( self, newDensity ):
		'''Sets the density for this Shape object.  This implicitly changes the  mass of the object.'''


		self.density = newDensity
		breve.breveInternalFunctionFinder.shapeSetDensity( self, self.shapePointer, self.density )

	def setMass( self, newMass ):
		'''Sets the mass for this Shape object.  This implicitly changes the  density of the object.'''


		if self.shapePointer:
			breve.breveInternalFunctionFinder.shapeSetMass( self, self.shapePointer, newMass )



breve.Shape = Shape
class CustomShape( breve.Shape ):
	'''A CustomShape is a subclass of (Shape) which allows the user to  construct an arbitrary convex shape by specifying the faces of  the shape. <P> The shapes must conform to the following rules: <li>The point (0, 0, 0) must be on <b>inside</b> (not outside or  on the surface of) the shape. <li>The shape must be convex. <li>The shape must be solid and sealed by the faces. </ul> <p> If any of these conditions are not met, you will get errors  and/or unexpected results.'''

	def __init__( self ):
		breve.Shape.__init__( self )
		CustomShape.init( self )

	def addFace( self, vertexList ):
		'''Adds a face defined by the list of vectors in vertextList.'''


		breve.breveInternalFunctionFinder.addShapeFace( self, self.shapePointer, vertexList )

	def finishShape( self, theDensity ):
		'''This method must be called after all of the faces are added  to complete initialization of the shape.  The density given  here will effect the physical properties of the shape if  physical simulation is used.  A value of 1.0 is reasonable. <P> If the shape specified is invalid (according to the constraints listed above), this method will trigger an error.'''


		return breve.breveInternalFunctionFinder.finishShape( self, self.shapePointer, theDensity )

	def init( self ):
		''''''


		self.shapePointer = breve.breveInternalFunctionFinder.newShape( self)


breve.CustomShape = CustomShape
class Sphere( breve.Shape ):
	'''This class is used to create a sphere shape.'''

	def __init__( self ):
		breve.Shape.__init__( self )

	def initWith( self, theRadius ):
		'''Initializes the sphere with the radius theRadius.'''


		self.shapePointer = breve.breveInternalFunctionFinder.newSphere( self, theRadius, self.density )
		if ( not self.shapePointer ):
			raise Exception( '''Could not create Sphere: invalid arguments''' )


		return self


breve.Sphere = Sphere
class Cube( breve.Shape ):
	'''This class is used to create an extruded rectangle.  Even though the class is named "Cube", the shapes do not need to be perfect cubes--they can be  rectangular solids of all sizes.'''

	def __init__( self ):
		breve.Shape.__init__( self )

	def initWith( self, cubeSize ):
		'''Initializes the cube to a rectangular solid with size cubeSize. '''


		self.shapePointer = breve.breveInternalFunctionFinder.newCube( self, cubeSize, self.density )
		if ( not self.shapePointer ):
			raise Exception( '''Could not create Cube: invalid arguments''' )


		return self


breve.Cube = Cube
class PolygonDisk( breve.Shape ):
	'''This class is used to create a polygon-disk.  This is a shape which can be  described as an extruded polygon. '''

	def __init__( self ):
		breve.Shape.__init__( self )

	def initWith( self, sideCount, theHeight, theRadius = 1.000000 ):
		'''Initializes the polygon-disk. <p> The distance from the center of the n-gon faces to the vertices  is theRadius.  sides has a maximum value of 99.  Higher values will cause the shape not to be initialized. <p> The height, or depth of the extrusion, is theHeight.'''


		self.shapePointer = breve.breveInternalFunctionFinder.newNGonDisc( self, sideCount, theRadius, theHeight, self.density )
		if ( not self.shapePointer ):
			raise Exception( '''Could not create PolygonDisk: invalid arguments''' )


		return self


breve.PolygonDisk = PolygonDisk
class PolygonCone( breve.Shape ):
	'''This class is used to create a polygon-cone shape.  This is a shape with a polygon base which tapers off to a point.  A pyramid is an example of a polygon-cone with 4 sides.  As the number of sides increases, the base becomes more circular and the resulting shape will more closely resemble a true cone.'''

	def __init__( self ):
		breve.Shape.__init__( self )

	def initWith( self, sideCount, theHeight, theRadius = 1.000000 ):
		'''Initializes the polygon-cone. <p> The distance from the center of the n-gon faces to the vertices  is theRadius.  sides has a maximum value of 99.  Higher values will cause the polygon-cone not to be initialized. <p> The height, or depth of the extrusion, is theHeight.'''


		self.shapePointer = breve.breveInternalFunctionFinder.newNGonCone( self, sideCount, theRadius, theHeight, self.density )
		if ( not self.shapePointer ):
			raise Exception( '''Could not create PolygonCone: invalid arguments''' )


		return self


breve.PolygonCone = PolygonCone
class MeshShape( breve.Shape ):
	'''An experimental class to load arbitrary 3d mesh shapes.   <p> <b>Full collision detection is not currently supported for MeshShapes</b>. MeshShapes are currently collision detected using spheres, with the radius defined by the maximum reach of the mesh.'''

	def __init__( self ):
		breve.Shape.__init__( self )

	def loadFrom3ds( self, filename, nodename = "" ):
		'''Attempts to load a mesh from a 3D Studio scene file named filename. The optional argument nodename specifies which mesh in the scene  should be loaded.  If nodename is not provided, the first mesh found   in the scene is loaded.'''


		self.shapePointer = breve.breveInternalFunctionFinder.meshShapeNew( self, filename, nodename )
		if ( not self.shapePointer ):
			raise Exception( '''Could not create MeshShape: invalid arguments''' )


		return self


breve.MeshShape = MeshShape
breve.Shapes = Shape
breve.CustomShapes = CustomShape
breve.Spheres = Sphere
breve.Cubes = Cube
breve.PolygonDisks = PolygonDisk
breve.PolygonCones = PolygonCone



