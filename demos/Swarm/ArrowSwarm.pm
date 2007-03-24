
use strict;
use Breve;

package Swarm;
our @ISA = qw(Breve::Control);

sub new {
	my $class = shift;
	my $self = {};
	bless $self, $class;
#  my $self->{ birds } = ();
#	my $self->{ cloudTexture } = undef;
#	my $self->{ normalMenu } = undef;
#	my $self->{ obedientMenu } = undef;
#	my $self->{ selection } = undef;
#	my $self->{ wackyMenu } = undef;
	init( $self );
   
   return $self;
}

sub click {
	my ($self, $item );
	( $self, $item ) = @_;

	if( $self->{ selection } ) {
		$self->{ selection }->hideNeighborLines();
	}

	if( $item ) {
		$item->showNeighborLines();
	}

	$self->{ selection } = $item;
	$self->SUPER::click( $item );
}

sub flockNormally {
	my $self;
	( $self ) = @_;
	my $item = undef;

	foreach $item ($self->{ birds }) {
		$item->flockNormally();
	}


	$self->{ normalMenu }->check();
	$self->{ obedientMenu }->uncheck();
	$self->{ wackyMenu }->uncheck();
}

sub flockObediently {
	my $self;
	( $self ) = @_;
	my $item = undef;

	foreach $item ($self->{ birds }) {
		$item->flockObediently();
	}


	$self->{ normalMenu }->uncheck();
	$self->{ obedientMenu }->check();
	$self->{ wackyMenu }->uncheck();
}

sub flockWackily {
	my $self;
	( $self ) = @_;
	my $item = undef;

	foreach $item ($self->{ birds }) {
		$item->flockWackily();
	}


	$self->{ normalMenu }->uncheck();
	$self->{ obedientMenu }->uncheck();
	$self->{ wackyMenu }->check();
}

sub init {
	my $self;
	( $self ) = @_;
	my $floor = undef;

	$self->addMenu( "Smoosh The Birdies", "squish" );
	$self->addMenuSeparator();
	$self->{ obedientMenu } = $self->addMenu( "Flock Obediently", "flockObediently" );
	$self->{ normalMenu } = $self->addMenu( "Flock Normally", "flockNormally" );
	$self->{ wackyMenu } = $self->addMenu( "Flock Wackily", "flockWackily" );
	$self->enableLighting();
	$self->moveLight( Breve::vector( 0, 20, 20 ) );
	$self->{ cloudTexture } = breve->createInstances( breve->Image, 1 )->load( "images/clouds.png" );
	$floor = breve->createInstances( breve->Floor, 1 );
	$floor->catchShadows();
	$self->{ birds } = breve->createInstances( breve->Birds, 60 );
	$self->flockNormally();
	$self->setBackgroundTextureImage( $self->{ cloudTexture } );
	$self->offsetCamera( breve->vector( 5, 1.500000, 6 ) );
	$self->enableShadows();
}

sub internalUserInputMethod {
	my $self;
	( $self ) = @_;

}

sub iterate {
	my $self;
	( $self ) = @_;
	my $item = undef;
	my $location = breve->vector();
	my $topDiff = 0;

	$self->updateNeighbors();
	foreach $item ($self->{ birds }) {
		$item->fly();
		$location = ( $location + $item->getLocation() );

	}


	$location = ( $location / breve->length( $self->{ birds } ) );
	$topDiff = 0.000000;
	foreach $item ($self->{ birds }) {
		if( ( $topDiff < breve->length( ( $location - $item->getLocation() ) ) ) ) {
			$topDiff = breve->length( ( $location - $item->getLocation() ) );
		}
;

	}


	$self->aimCamera( $location );
	$self->SUPER::iterate();
}

sub squish {
	my $self;
	( $self ) = @_;
	my $item = undef;

	foreach $item ($self->{ birds }) {
		$item->move( breve->vector( 0, 0, 0 ) );
	}


}

package Bird;

sub new {
	my $class = shift;
	my $self = {};
	bless $self, $class;
	my $self->{ centerConstant } = 0;
	my $self->{ cruiseDistance } = 0;
	my $self->{ landed } = 0;
	my $self->{ maxAcceleration } = 0;
	my $self->{ maxVelocity } = 0;
	my $self->{ spacingConstant } = 0;
	my $self->{ velocityConstant } = 0;
	my $self->{ wanderConstant } = 0;
	my $self->{ worldCenterConstant } = 0;
	init( $self );
	return $self;
}

sub checkLanded {
	my $self;
	( $self ) = @_;

	return $self->{ landed };
}

sub checkVisibility {
	my ($self, $item );
	( $self, $item ) = @_;

	if( ( $item == $self ) ) {
		return 0;
	}

	if( ( not $item->isA( "Bird" ) ) ) {
		return 0;
	}

	if( $item->checkLanded() ) {
		return 0;
	}

	if( ( $self->getAngle( $item ) > 2.000000 ) ) {
		return 0;
	}

	return 1;
}

sub flockNormally {
	my $self;
	( $self ) = @_;

	$self->{ wanderConstant } = 4.000000;
	$self->{ worldCenterConstant } = 5.000000;
	$self->{ centerConstant } = 2.000000;
	$self->{ velocityConstant } = 2.000000;
	$self->{ spacingConstant } = 5.000000;
	$self->{ maxVelocity } = 15;
	$self->{ maxAcceleration } = 15;
	$self->{ cruiseDistance } = 0.400000;
}

sub flockObediently {
	my $self;
	( $self ) = @_;

	$self->{ wanderConstant } = 6.000000;
	$self->{ worldCenterConstant } = 6.000000;
	$self->{ centerConstant } = 2.000000;
	$self->{ velocityConstant } = 3.000000;
	$self->{ spacingConstant } = 4.000000;
	$self->{ maxVelocity } = 16;
	$self->{ maxAcceleration } = 20;
	$self->{ cruiseDistance } = 1;
}

sub flockWackily {
	my $self;
	( $self ) = @_;

	$self->{ wanderConstant } = 8.000000;
	$self->{ worldCenterConstant } = 14.000000;
	$self->{ centerConstant } = 1.000000;
	$self->{ velocityConstant } = 3.000000;
	$self->{ spacingConstant } = 4.000000;
	$self->{ maxVelocity } = 20;
	$self->{ maxAcceleration } = 30;
	$self->{ cruiseDistance } = 0.500000;
}

sub fly {
	my $self;
	( $self ) = @_;
	my $bird = undef;
	my $toNeighbor = breve->vector();
	my $centerUrge = breve->vector();
	my $worldCenterUrge = breve->vector();
	my $velocityUrge = breve->vector();
	my $spacingUrge = breve->vector();
	my $wanderUrge = breve->vector();
	my $acceleration = breve->vector();
	my $newVelocity = breve->vector();
	my $neighbors = ();
	my $takeOff = 0;

	foreach $bird ($self->getNeighbors()) {
		if( $self->checkVisibility( $bird ) ) {
			$neighbors.append( $bird );
		}
;

	}


	if( $self->{ landed } ) {
		$takeOff = breve->randomExpression( 40 );
		if( ( $takeOff == 1 ) ) {
			$self->{ landed } = 0;
			$self->setVelocity( ( breve->randomExpression( breve->vector( 0.100000, 1.100000, 0.100000 ) ) - breve->vector( 0.050000, 0, 0.050000 ) ) );

		}		 else {
			return;

		}
;

	}

	$centerUrge = $self->getCenterUrge( $neighbors );
	$velocityUrge = $self->getVelocityUrge( $neighbors );
	foreach $bird ($neighbors) {
		$toNeighbor = ( $self->getLocation() - $bird->getLocation() );
		if( ( breve->length( $toNeighbor ) < $self->{ cruiseDistance } ) ) {
			$spacingUrge = ( $spacingUrge + $toNeighbor );
		}
;

	}


	if( ( breve->length( $self->getLocation() ) > 10 ) ) {
		$worldCenterUrge = ( -$self->getLocation() );
	}

	$wanderUrge = ( breve->randomExpression( breve->vector( 2, 2, 2 ) ) - breve->vector( 1, 1, 1 ) );
	if( breve->length( $spacingUrge ) ) {
		$spacingUrge = ( $spacingUrge / breve->length( $spacingUrge ) );
	}

	if( breve->length( $worldCenterUrge ) ) {
		$worldCenterUrge = ( $worldCenterUrge / breve->length( $worldCenterUrge ) );
	}

	if( breve->length( $velocityUrge ) ) {
		$velocityUrge = ( $velocityUrge / breve->length( $velocityUrge ) );
	}

	if( breve->length( $centerUrge ) ) {
		$centerUrge = ( $centerUrge / breve->length( $centerUrge ) );
	}

	if( breve->length( $wanderUrge ) ) {
		$wanderUrge = ( $wanderUrge / breve->length( $wanderUrge ) );
	}

	$wanderUrge = ( $wanderUrge * $self->{ wanderConstant } );
	$worldCenterUrge = ( $worldCenterUrge * $self->{ worldCenterConstant } );
	$centerUrge = ( $centerUrge * $self->{ centerConstant } );
	$velocityUrge = ( $velocityUrge * $self->{ velocityConstant } );
	$spacingUrge = ( $spacingUrge * $self->{ spacingConstant } );
	$acceleration = ( ( ( ( $worldCenterUrge + $centerUrge ) + $velocityUrge ) + $spacingUrge ) + $wanderUrge );
	if( ( breve->length( $acceleration ) != 0 ) ) {
		$acceleration = ( $acceleration / breve->length( $acceleration ) );
	}

	$self->setAcceleration( ( $self->{ maxAcceleration } * $acceleration ) );
	$newVelocity = $self->getVelocity();
	if( ( breve->length( $newVelocity ) > $self->{ maxVelocity } ) ) {
		$newVelocity = ( ( $self->{ maxVelocity } * $newVelocity ) / breve->length( $newVelocity ) );
	}

	$self->setVelocity( $newVelocity );
	$self->point( breve->vector( 0, 1, 0 ), ( $newVelocity / breve->length( $newVelocity ) ) );
}

sub getAngle {
	my ($self, $otherMobile );
	( $self, $otherMobile ) = @_;
	my $tempVector = breve->vector();

	if( ( breve->length( $self->getVelocity() ) == 0 ) ) {
		return 0;
	}

	$tempVector = ( $otherMobile->getLocation() - $self->getLocation() );
   #return Breve::callInternal("angle", $self->getVelocity(), $tempVector );
}

sub getCenterUrge {
	my ($self, $flock );
	( $self, $flock ) = @_;
	my $item = undef;
	my $count = 0;
	my $center = breve->vector();

	foreach $item ($flock) {
		$count = ( $count + 1 );
		$center = ( $center + $item->getLocation() );

	}


	if( ( $count == 0 ) ) {
		return breve->vector( 0, 0, 0 );
	}

	$center = ( $center / $count );
	return ( $center - $self->getLocation() );
}

sub getVelocityUrge {
	my ($self, $flock );
	( $self, $flock ) = @_;
	my $item = undef;
	my $count = 0;
	my $aveVelocity = breve->vector();

	foreach $item ($flock) {
		$count = ( $count + 1 );
		$aveVelocity = ( $aveVelocity + $item->getVelocity() );

	}


	if( ( $count == 0 ) ) {
		return breve->vector( 0, 0, 0 );
	}

	$aveVelocity = ( $aveVelocity / $count );
	return ( $aveVelocity - $self->getVelocity() );
}

sub init {
	my $self;
	( $self ) = @_;

	$self->setShape( breve->createInstances( breve->PolygonCone, 1 )->initWith( 3, 0.500000, 0.060000 ) );
	$self->move( ( breve->randomExpression( breve->vector( 10, 10, 10 ) ) - breve->vector( 5, -5, 5 ) ) );
	$self->setVelocity( ( breve->randomExpression( breve->vector( 20, 20, 20 ) ) - breve->vector( 10, 10, 10 ) ) );
	$self->setColor( breve->randomExpression( breve->vector( 1, 1, 1 ) ) );
	$self->handleCollisions( "Floor", "land" );
	$self->setNeighborhoodSize( 3.000000 );
}

sub land {
	my ($self, $ground );
	( $self, $ground ) = @_;

	$self->setAcceleration( breve->vector( 0, 0, 0 ) );
	$self->setVelocity( breve->vector( 0, 0, 0 ) );
	$self->{ landed } = 1;
	$self->offset( breve->vector( 0, 0.001000, 0 ) );
}

package main;

&Breve::bootstrap();

my $SwarmController = Swarm->new();
Breve::brPerlSetController($SwarmController);

1;

