
use strict;
use Breve;

package Breve::Shape;

our @ISA = qw(Breve::Abstract);

###The Shape class is a work-in-progress which allows users to create  shapes which will be associated with OBJECT(Mobile), OBJECT(Stationary) or OBJECT(Link) objects and added to the simulated world.  An instance of the class Shape may be shared by several objects simultaneously. <p> Each Shape has it's own local coordinate frame, with the origin at the middle of the shape.

sub new {
	my $class = shift;
	my $self = {};
	bless $self, $class;
	my $self->{ density } = 0;
	my $self->{ lastScale } = ();
	my $self->{ shapeData } = undef;
	my $self->{ shapePointer } = undef;
	init( $self );
	return $self;
}

sub archive {
	my $self;
	( $self ) = @_;

	$self->{ shapeData } = $self->getDataForShape();
	return 1;
}

sub dearchive {
	my $self;
	( $self ) = @_;

	$self->{ shapePointer } = Breve::callInternal($self, "shapeForData", $self->{ shapeData } );
	return $self->SUPER::dearchive();
}

sub destroy {
	my $self;
	( $self ) = @_;

	if( $self->{ shapePointer } ) {
		Breve::callInternal($self, "freeShape", $self->{ shapePointer } );
	}

}

sub getDataForShape {
	###Returns serialized data for the shape (if the shape object has  been properly initialized).  Used for archiving/dearchiving, should generally not be called manually, unless you <i>really</i> know what you're doing.

	my $self;
	( $self ) = @_;

	if( $self->{ shapePointer } ) {
		return Breve::callInternal($self, "dataForShape", $self->{ shapePointer } );
	}

}

sub getDensity {
	###If the shape is properly initialized, this method returns the  shape's density.

	my $self;
	( $self ) = @_;

	if( $self->{ shapePointer } ) {
		return Breve::callInternal($self, "getDensity", $self->{ shapePointer } );
	}

	return 0.000000;
}

sub getLastScale {
	###Used internally...

	my $self;
	( $self ) = @_;

	return $self->{ lastScale };
}

sub getMass {
	###If the shape is properly initialized, this method returns the  shape's mass.

	my $self;
	( $self ) = @_;

	if( $self->{ shapePointer } ) {
		return Breve::callInternal($self, "getMass", $self->{ shapePointer } );
	}

	return 0.000000;
}

sub getPointOnShape {
	###This method is experimental. <p> Starting from inside the shape at the center, this function goes in  the direction of theVector until it hits the edge of the shape. The resulting point is returned.   <p> This allows you to compute link points for arbitrary shapes.  For example, if you want to compute a link point for the  "left-most" point on the shape, you can call this method with (-1, 0, 0).   <p> Returns (0, 0, 0) if the shape is not initialized or if an error occurs.

	my ($self, $theVector );
	( $self, $theVector ) = @_;

	if( $self->{ shapePointer } ) {
		return Breve::callInternal($self, "pointOnShape", $self->{ shapePointer }, $theVector );
	}	 else {
		return Breve::vector( 0, 0, 0 );
	}

}

sub getPointer {
	###Returns the shapePointer associated with this Shape object.  This  method is used internally and should not typically be used in  user simulations.

	my $self;
	( $self ) = @_;

	return $self->{ shapePointer };
}

sub init {
	my $self;
	( $self ) = @_;

	$self->SUPER::init();

	$self->{ density } = 1.000000;
}

sub initWithCube {
	###Sets this Shape object to a rectangular solid of size v.  

	my ($self, $v );
	( $self, $v ) = @_;

	$self->{ shapePointer } = Breve::callInternal($self, "newCube", $v, $self->{ density } );
	return $self;
}

sub initWithPolygonCone {
	###Sets this Shape object to a cone-like shape with sideCount sides. <p> The distance from the center of the n-gon faces to the vertices  is theRadius.  sides has a maximum value of 99.  Higher values will cause the shape not to be initialized. <p> The height, or depth of the extrusion, is theHeight. <p> This method is experimental, but seems to work okay.  Go figure.

	my ($self, $sideCount, $theHeight, $theRadius );
	( $self, $sideCount, $theHeight, $theRadius = 1.000000 ) = @_;

	$self->{ shapePointer } = Breve::callInternal($self, "newNGonCone", $sideCount, $theRadius, $theHeight, $self->{ density } );
	return $self;
}

sub initWithPolygonDisk {
	###Sets this Shape object to an extruded n-gon of sideCount sides, in other words, a disk with sideCount sides. <p> The distance from the center of the n-gon faces to the vertices  is theRadius.  sides has a maximum value of 99.  Higher values will cause the shape not to be initialized. <p> The height, or depth of the extrusion, is theHeight. <p> This method is experimental, but seems to work okay.  Go figure.

	my ($self, $sideCount, $theHeight, $theRadius );
	( $self, $sideCount, $theHeight, $theRadius = 1.000000 ) = @_;

	$self->{ shapePointer } = Breve::callInternal($self, "newNGonDisc", $sideCount, $theRadius, $theHeight, $self->{ density } );
	return $self;
}

sub initWithSphere {
	###Sets this Shape object to a sphere with radius theRadius.

	my ($self, $theRadius );
	( $self, $theRadius = 1.000000 ) = @_;

	$self->{ shapePointer } = Breve::callInternal($self, "newSphere", $theRadius, $self->{ density } );
	return $self;
}

sub scale {
	###If the shape is <i>not</i> a sphere, scales the shape by the x, y  and z elements of scale.  If the shape <i>is</i> a sphere, scales  the shape by only the x element such that the shape always remains  spherical. <p> After the size has been changed, the instances announces a  "size-changed" notification.

	my ($self, $scale );
	( $self, $scale ) = @_;

	Breve::callInternal($self, "scaleShape", $self->{ shapePointer }, $scale );
	$self->{ lastScale } = $scale;
	$self->announce( "sizeChanged" );
}

sub setDensity {
	###Sets the density for this Shape object.  This implicitly changes the  mass of the object.

	my ($self, $newDensity );
	( $self, $newDensity ) = @_;

	$self->{ density } = $newDensity;
	Breve::callInternal($self, "shapeSetDensity", $self->{ shapePointer }, $self->{ density } );
}

sub setMass {
	###Sets the mass for this Shape object.  This implicitly changes the  density of the object.

	my ($self, $newMass );
	( $self, $newMass ) = @_;

	if( $self->{ shapePointer } ) {
		Breve::callInternal($self, "shapeSetMass", $self->{ shapePointer }, $newMass );
	}

}



package Breve::CustomShape;

###A CustomShape is a subclass of (Shape) which allows the user to  construct an arbitrary convex shape by specifying the faces of  the shape. <P> The shapes must conform to the following rules: <li>The point (0, 0, 0) must be on <b>inside</b> (not outside or  on the surface of) the shape. <li>The shape must be convex. <li>The shape must be solid and sealed by the faces. </ul> <p> If any of these conditions are not met, you will get errors  and/or unexpected results.

sub new {
	my $class = shift;
	my $self = {};
	bless $self, $class;
	init( $self );
	return $self;
}

sub addFace {
	###Adds a face defined by the list of vectors in vertextList.

	my ($self, $vertexList );
	( $self, $vertexList ) = @_;

	Breve::callInternal($self, "addShapeFace", $self->{ shapePointer }, $vertexList );
}

sub finishShape {
	###This method must be called after all of the faces are added  to complete initialization of the shape.  The density given  here will effect the physical properties of the shape if  physical simulation is used.  A value of 1.0 is reasonable. <P> If the shape specified is invalid (according to the constraints listed above), this method will trigger an error.

	my ($self, $theDensity );
	( $self, $theDensity ) = @_;

	return Breve::callInternal($self, "finishShape", $self->{ shapePointer }, $theDensity );
}

sub init {
	my $self;
	( $self ) = @_;

	$self->SUPER::init();

	$self->{ shapePointer } = Breve::callInternal($self, "newShape");
}




package Breve::Sphere;

our @ISA = qw(Breve::Shape);

###This class is used to create a sphere shape.

sub new {
	my $class = shift;
	my $self = {};
	bless $self, $class;
	return $self;
}

sub initWith {
	###Initializes the sphere with the radius theRadius.

	my ($self, $theRadius );
	( $self, $theRadius ) = @_;

	$self->SUPER::init();

	$self->{ shapePointer } = Breve::callInternal($self, "newSphere", $theRadius, $self->{ density } );
	if( ( not $self->{ shapePointer } ) ) {
		raise Exception( "Could not create Sphere: invalid arguments" );
	}

	return $self;
}




package Breve::Cube;

our @ISA = qw(Breve::Shape);

###This class is used to create an extruded rectangle.  Even though the class is named "Cube", the shapes do not need to be perfect cubes--they can be  rectangular solids of all sizes.

sub new {
	my $class = shift;
	my $self = {};
	bless $self, $class;
	return $self;
}

sub initWith {
	###Initializes the cube to a rectangular solid with size cubeSize. 

	my ($self, $cubeSize );
	( $self, $cubeSize ) = @_;

	$self->SUPER::init();

	$self->{ shapePointer } = Breve::callInternal($self, "newCube", $cubeSize, $self->{ density } );
	if( ( not $self->{ shapePointer } ) ) {
		raise Exception( "Could not create Cube: invalid arguments" );
	}

	return $self;
}


package Breve::PolygonDisk;

our @ISA = qw(Breve::Shape);

###This class is used to create a polygon-disk.  This is a shape which can be  described as an extruded polygon. 

sub new {
	my $class = shift;
	my $self = {};
	bless $self, $class;
	return $self;
}

sub initWith {
	###Initializes the polygon-disk. <p> The distance from the center of the n-gon faces to the vertices  is theRadius.  sides has a maximum value of 99.  Higher values will cause the shape not to be initialized. <p> The height, or depth of the extrusion, is theHeight.

	my ($self, $sideCount, $theHeight, $theRadius );
	( $self, $sideCount, $theHeight, $theRadius = 1.000000 ) = @_;

	$self->SUPER::init();

	$self->{ shapePointer } = Breve::callInternal($self, "newNGonDisc", $sideCount, $theRadius, $theHeight, $self->{ density } );
	if( ( not $self->{ shapePointer } ) ) {
		raise Exception( "Could not create PolygonDisk: invalid arguments" );
	}

	return $self;
}





package Breve::PolygonCone;

our @ISA = qw(Breve::Shape);

###This class is used to create a polygon-cone shape.  This is a shape with a polygon base which tapers off to a point.  A pyramid is an example of a polygon-cone with 4 sides.  As the number of sides increases, the base becomes more circular and the resulting shape will more closely resemble a true cone.

sub new {
	my $class = shift;
	my $self = {};
	bless $self, $class;
	return $self;
}

sub initWith {
	###Initializes the polygon-cone. <p> The distance from the center of the n-gon faces to the vertices  is theRadius.  sides has a maximum value of 99.  Higher values will cause the polygon-cone not to be initialized. <p> The height, or depth of the extrusion, is theHeight.

	my ($self, $sideCount, $theHeight, $theRadius );
	( $self, $sideCount, $theHeight, $theRadius = 1.000000 ) = @_;

	$self->SUPER::init();

	$self->{ shapePointer } = Breve::callInternal($self, "newNGonCone", $sideCount, $theRadius, $theHeight, $self->{ density } );
	if( ( not $self->{ shapePointer } ) ) {
		raise Exception( "Could not create PolygonCone: invalid arguments" );
	}

	return $self;
}




package Breve::MeshShape;

our @ISA = qw(Breve::Shape);

###An experimental class to load arbitrary 3d mesh shapes.   <p> <b>Full collision detection is not currently supported for MeshShapes</b>. MeshShapes are currently collision detected using spheres, with the radius defined by the maximum reach of the mesh.

sub new {
	my $class = shift;
	my $self = {};
	bless $self, $class;
	return $self;
}

sub loadFrom3ds {
	###Attempts to load a mesh from a 3D Studio scene file named filename. The optional argument nodename specifies which mesh in the scene  should be loaded.  If nodename is not provided, the first mesh found   in the scene is loaded.

	my ($self, $filename, $nodename );
	( $self, $filename, $nodename = "" ) = @_;

	$self->{ shapePointer } = Breve::callInternal($self, "meshShapeNew", $filename, $nodename );
	if( ( not $self->{ shapePointer } ) ) {
		raise Exception( "Could not create MeshShape: invalid arguments" );
	}

	return $self;
}


1;

