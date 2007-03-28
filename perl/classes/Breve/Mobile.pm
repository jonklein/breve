
use strict;
use Breve;

package Breve::Mobile;

our @ISA = qw(Breve::Real);

###Mobile objects are objects in the simulated world which move around 		 and interact with other objects.  This is in contrast to  OBJECT(Stationary) objects which can collide and interact with  other objects but which never move. <P> When a Mobile object is created, it will be by default a simple  sphere.  You can change the appearence of this sphere by using methods in this class, or its parent class OBJECT(Real).  Or  you can change the shape altogether with the method METHOD(set-shape).

sub new {
	my $class = shift;
	my $self = {};
	bless $self, $class;
	$self->{ archiveAcceleration } = ();
	$self->{ archiveLocation } = ();
	$self->{ archiveRotation } = ();
	$self->{ archiveRvelocity } = ();
	$self->{ archiveVelocity } = ();
	$self->{ linkForce } = ();
	$self->{ linkTorque } = ();
	$self->{ physicsEnabled } = 0;
	init( $self );
	return $self;
}

sub archive {
	my $self;
	( $self ) = @_;

	$self->{ archiveLocation } = $self->getLocation();
	$self->{ archiveRotation } = $self->getRotation();
	$self->{ archiveVelocity } = $self->getVelocity();
	$self->{ archiveRvelocity } = $self->getRotationalVelocity();
	$self->{ archiveAcceleration } = $self->getAcceleration();
	return $self->SUPER::archive();
}

sub checkForPenetrations {
	###Depricated.

	my $self;
	( $self ) = @_;

	return $self->getCollidingObjects();
}

sub checkForSelfPenetrations {
	###Determines whether this link is currently penetrating with other links in the same multibody.  This is not meant as a general purpose collision detection tool -- it is meant to detect potentially troublesome configurations of links when they are created.

	my $self;
	( $self ) = @_;

	return Breve::callInternal( $self, "linkCheckSelfPenetration", $self->{ realWorldPointer } );
}

sub dearchive {
	my $self;
	( $self ) = @_;

	$self->{ realWorldPointer } = Breve::callInternal( $self, "linkNew");
	$self->{ realWorldPointer } = Breve::callInternal( $self, "linkAddToWorld", $self->{ realWorldPointer } );
	$self->setCollisionShape( $self->{ collisionShape } );
	$self->setDisplayShape( $self->{ displayShape } );
	$self->move( $self->{ archiveLocation } );
	$self->setRotationMatrix( $self->{ archiveRotation } );
	$self->setVelocity( $self->{ archiveVelocity } );
	$self->setRotationalVelocity( $self->{ archiveRvelocity } );
	$self->setAcceleration( $self->{ archiveAcceleration } );
	if( $self->{ physicsEnabled } ) {
		$self->enablePhysics();
	}	 else {
		$self->disablePhysics();
	}

	return $self->SUPER::dearchive();
}

sub disablePhysics {
	###Disables the physical simulation for a OBJECT(Mobile) object.

	my $self;
	( $self ) = @_;

	$self->{ physicsEnabled } = 0;
	Breve::callInternal( $self, "linkSetPhysics", $self->{ realWorldPointer }, 0 );
}

sub enablePhysics {
	###Enables physical simulation for a OBJECT(Mobile) object.   This must be used in conjunction with a  OBJECT(PhysicalControl) object which sets up physical  simulation for the entire world.   <p> When physics is enabled for an object, the acceleration  can no longer be assigned manually--it will be computed  from the forces applied to the object.

	my $self;
	( $self ) = @_;

	$self->{ physicsEnabled } = 1;
	Breve::callInternal( $self, "linkSetPhysics", $self->{ realWorldPointer }, 1 );
}

sub getAcceleration {
	###Returns the vector acceleration of this object.

	my $self;
	( $self ) = @_;

	return Breve::callInternal( $self, "linkGetAcceleration", $self->{ realWorldPointer } );
}

sub getBoundMaximum {
	###Returns the vector representing the maximum X, Y and Z locations of points on this link.

	my $self;
	( $self ) = @_;

	return Breve::callInternal( $self, "linkGetMax", $self->{ realWorldPointer } );
}

sub getBoundMinimum {
	###Returns the vector representing the minimum X, Y and Z locations of points on this link.

	my $self;
	( $self ) = @_;

	return Breve::callInternal( $self, "linkGetMin", $self->{ realWorldPointer } );
}

sub getCollidingObjects {
	###Returns a list of objects currently colliding with this object. This is not meant as a general purpose collision detection tool -- it is meant to detect potentially troublesome configurations of links when they are created.

	my $self;
	( $self ) = @_;

	return Breve::callInternal( $self, "linkGetPenetratingObjects", $self->{ realWorldPointer } );
}

sub getDistance {
	###Returns the scalar distance from this object's center to  otherObject.

	my ($self, $otherObject );
	( $self, $otherObject ) = @_;

	return Breve::length( ( $self->getLocation() - $otherObject->getLocation() ) );
}

sub getForce {
	###Returns the force acting on the object, which was previously  set using METHOD(set-force).  

	my $self;
	( $self ) = @_;

	return $self->{ linkForce };
}

sub getLinkPointer {
	###For internal use only.

	my $self;
	( $self ) = @_;

	return $self->{ realWorldPointer };
}

sub getMass {
	###Returns the mass of the object.

	my $self;
	( $self ) = @_;

	return $self->{ collisionShape }->getMass();
}

sub getRotationalVelocity {
	###Returns the vector angular velocity of this object.

	my $self;
	( $self ) = @_;

	return Breve::callInternal( $self, "linkGetRotationalVelocity", $self->{ realWorldPointer } );
}

sub getTorque {
	###Returns the torque acting on the object, which was previously  set using METHOD(set-torque).  

	my $self;
	( $self ) = @_;

	return $self->{ linkTorque };
}

sub getVelocity {
	###Returns the vector velocity of this object.

	my $self;
	( $self ) = @_;

	return Breve::callInternal( $self, "linkGetVelocity", $self->{ realWorldPointer } );
}

sub init {
	my $self;
	( $self ) = @_;

	$self->SUPER::init();

	$self->{ e } = 0.200000;
	$self->{ eT } = 0.500000;
	$self->{ mu } = 0.200000;
	$self->{ color } = Breve::Vector->new( 1, 1, 1 );
	$self->{ realWorldPointer } = Breve::callInternal( $self, "linkNew");
	Breve::callInternal( $self, "linkAddToWorld", $self->{ realWorldPointer } );
	$self->setShape( $self->{ controller }->getGenericShape() );
}

sub offset {
	###Moves this object by amount, relative to its current position.

	my ($self, $amount );
	( $self, $amount ) = @_;

	$self->move( ( $self->getLocation() + $amount ) );
}

sub register {
	###Deprecated.  Don't use.

	my ($self, $theShape );
	( $self, $theShape ) = @_;

	print "warning: the method 'register' of Mobile is deprecated, use the method 'set-shape' instead";
	$self->setShape( $theShape );
}

sub relativeRotate {
	###Sets the rotation of this object around vector axis thisAxis  by scalar amount (in radians).  This is a rotation relative to the  current position.

	my ($self, $thisAxis, $amount );
	( $self, $thisAxis, $amount ) = @_;
	my $length = 0;

	$length = Breve::length( $thisAxis );
	Breve::callInternal( $self, "linkRotateRelative", $self->{ realWorldPointer }, $thisAxis, $amount );
}

sub removeLabel {
	###Removes the label that would be drawn next to an object.

	my $self;
	( $self ) = @_;

	Breve::callInternal( $self, "linkRemoveLabel", $self->{ realWorldPointer } );
}

sub resumePhysics {
	my $self;
	( $self ) = @_;

	Breve::callInternal( $self, "linkSetPhysics", $self->{ realWorldPointer }, $self->{ physicsEnabled } );
}

sub rotate {
	###Deprecated.  Renamed to METHOD(set-rotation).

	my ($self, $thisAxis, $amount );
	( $self, $thisAxis, $amount ) = @_;

	$self->setRotation( $thisAxis, $amount );
}

sub set {
	###Deprecated.  Don't use.

	my ($self, $theShape );
	( $self, $theShape ) = @_;

	print "warning: the method 'set' of Mobile is deprecated, use the method 'set-shape' instead";
	$self->setShape( $theShape );
}

sub setAcceleration {
	###Sets the acceleration of this object to newAcceleration. This method has no effect if physical simulation is turned  on for the object, in which case the physical simulation  engine computes acceleration.

	my ($self, $newAcceleration );
	( $self, $newAcceleration ) = @_;

	Breve::callInternal( $self, "linkSetAcceleration", $self->{ realWorldPointer }, $newAcceleration );
}

sub setForce {
	###Sets the velocity acting on the object to newForce.  This  force will remain in effect until it is disabled with a  new call to METHOD(set-force).

	my ($self, $newForce );
	( $self, $newForce ) = @_;

	if( !$self->{ realWorldPointer } ) {
	    die( "set-force called with uninitialized Mobile object" );  
	}

	$self->{ linkForce } = $newForce;
	Breve::callInternal( $self, "linkSetForce", $self->{ realWorldPointer }, $self->{ linkForce } );
}

sub setLabel {
	###Sets the label to be drawn along side the object.

	my ($self, $theLabel );
	( $self, $theLabel ) = @_;

	Breve::callInternal( $self, "linkSetLabel", $self->{ realWorldPointer }, $theLabel );
}

sub setRotationalAcceleration {
	###Sets the rotational acceleration of this object to  newAcceleration.  This method has no effect if physical  simulation is turned on for the object, in which case the  physical simulation engine computes acceleration.

	my ($self, $newAcceleration );
	( $self, $newAcceleration ) = @_;

	Breve::callInternal( $self, "linkSetRotationalAcceleration", $self->{ realWorldPointer }, $newAcceleration );
}

sub setRotationalVelocity {
	###Sets the rotational velocity of this object to  angularVelocity.

	my ($self, $angularVelocity );
	( $self, $angularVelocity ) = @_;

	Breve::callInternal( $self, "linkSetRotationalVelocity", $self->{ realWorldPointer }, $angularVelocity );
}

sub setTorque {
	###Sets the torque acting on the object to newTorque.  This  torque will remain in effect until it is disabled with a  new call to METHOD(set-torque).

	my ($self, $newTorque );
	( $self, $newTorque ) = @_;

	if( ( not $self->{ realWorldPointer } ) ) {
		print "set-torque called with uninitialized Mobile object";
		return;

	}

	$self->{ linkTorque } = $newTorque;
	Breve::callInternal( $self, "linkSetTorque", $self->{ realWorldPointer }, $self->{ linkTorque } );
}

sub setVelocity {
	###Sets the velocity of this object to newVelocity.

	my ($self, $newVelocity );
	( $self, $newVelocity ) = @_;

	if( ( not $self->{ realWorldPointer } ) ) {
		print "set-velocity called with uninitialized Mobile object";
		return;

	}

	Breve::callInternal( $self, "linkSetVelocity", $self->{ realWorldPointer }, $newVelocity );
}

sub suspendPhysics {
	my $self;
	( $self ) = @_;

	Breve::callInternal( $self, "linkSetPhysics", $self->{ realWorldPointer }, 0 );
}

sub transform {
	###Transforms theVector in the world coordinate frame to a  vector in the frame of this object.  

	my ($self, $theVector );
	( $self, $theVector ) = @_;

	return Breve::callInternal( $self, "vectorFromLinkPerspective", $self->{ realWorldPointer }, $theVector );
}


1;

