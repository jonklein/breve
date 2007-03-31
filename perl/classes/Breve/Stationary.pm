
use strict;
use Breve;

package Breve::Stationary;

our @ISA = qw(Breve::Real);


###Stationary objects are objects such as floors and walls that may  collide with other objects but which do not move in response to  collisions.  Stationary objects do not have their own velocities or acceleration. <p> Starting in breve 2.6, stationary objects can be moved and  rotated after being created using methods in OBJECT(Real).   The shapes can also be changed dynamically using the set-shape  method in OBJECT(Real).

sub new {
	my $class = shift;
	my $self = {};
	bless $self, $class;
	$self->{ objectLocation } = ();
	$self->{ shadowCatcher } = 0;
	init( $self );
	return $self;
}

sub archive {
	my $self;
	( $self ) = @_;

	return 1;
}

sub catchShadows {
	###Informs this object that it should display shadows (and/or reflections)  of Mobile objects.  The shadows and reflections will always be shown on the plane of the object pointing upwards on the Y axis--that is to say, the plane with normal (0, 1, 0).  If the object does not have a plane with normal (0, 1, 0), the shadows and reflections will not be displayed correctly.  This method must be used in conjunction with the method  OBJECTMETHOD(Control:enable-shadows). <P> Before using this method, you should also refer to an improved shadowing technique outlined in OBJECTMETHOD(Control.tz:enable-shadow-volumes).  

	my $self;
	( $self ) = @_;

	if( ( not $self->{ realWorldPointer } ) ) {
		raise Exception( "method 'catch-shadows' cannot be called before Stationary object is registered." );
	}

	$self->{ shadowCatcher } = 1;
	Breve::callInternal( $self, "setShadowCatcher", $self->{ realWorldPointer }, Breve::Vector->new( 0, 1, 0 ) );
	$self->{ controller }->setFloorDefined();
}

sub dearchive {
	my $self;
	( $self ) = @_;

	$self->move( $self->{ objectLocation } );
	$self->setDisplayShape( $self->{ displayShape } );
	$self->setCollisionShape( $self->{ collisionShape } );
	if( $self->{ shadowCatcher } ) {
		$self->catchShadows();
	}

	return 1;
}

sub getWorldObject {
	###Used internally to get the pointer to the world.  Do not use this method in user simulations.

	my $self;
	( $self ) = @_;

	return $self->{ realWorldPointer };
}

sub init {
	my $self;
	( $self ) = @_;

	$self->SUPER::init();

	$self->{ realWorldPointer } = Breve::callInternal( $self, "addStationary",
							   $self->{ controller }->getGenericShape()->getPointer(),
							   Breve::Vector->new( 0, 0, 0 ),
							   Breve::Matrix->new(  1, 0, 0, 0, 1, 0, 0, 0, 1 ) );
	$self->setTexture( 0 );
	return $self;
}

sub register {
	###Registers a stationary object using shape theShape at the location specified by theLocation.

	my ($self, $theShape, $theLocation, $theRotation ) = @_;

	if(!$theLocation) {
	    $theLocation = Breve::Vector->new( 0.000000, 0.000000, 0.000000 );
	}

	if(!$theRotation) {
	    $theRotation = Breve::Matrix->new( 1.000000, 0.000000, 0.000000,
					       0.000000, 1.000000, 0.000000,
					       0.000000, 0.000000, 1.000000 );
	}

	$self->setShape( $theShape );
	$self->move( $theLocation );

	$self->setRotationMatrix( $theRotation );
	return $self;
}

package Breve::Floor;

our @ISA = qw(Breve::Stationary);

###A floor is a special case of the class OBJECT(Stationary).  It is a box  of size (1000, 5, 1000) with location (0, -2.5, 0), such that the ground  plane is placed at Y = 0.

sub new {
	my $class = shift;
	my $self = {};
	bless $self, $class;
	init( $self );
	return $self;
}

sub init {
	my $self;
	( $self ) = @_;

	$self->SUPER::init();

	my $cube = Breve::Cube->new();
	$cube->initWith( Breve::Vector->new( 1000, 5, 1000 ) );
	$self->register( $cube, Breve::Vector->new( 0, -2.500000, 0 ) );
}


1;

