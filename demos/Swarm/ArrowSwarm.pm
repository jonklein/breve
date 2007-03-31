
use strict;
use Breve;

package Breve::Swarm;

sub new {
	my $class = shift;
	my $self = {};
	bless $self, $class;
	my $self->{ birds } = ();
	my $self->{ cloudTexture } = undef;
	my $self->{ dizzyMenu } = undef;
	my $self->{ normalMenu } = undef;
	my $self->{ obedientMenu } = undef;
	my $self->{ selection } = undef;
	my $self->{ useDizzyCameraControl } = 0;
	my $self->{ wackyMenu } = undef;
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
	$self->addMenuSeparator();
	$self->{ dizzyMenu } = $self->addMenu( "Use Dizzy Camera Control", "toggleDizzy" );
	$self->enableLighting();
	$self->moveLight( Breve::Vector->new( 0, 20, 20 ) );
	$self->{ cloudTexture } = Breve::Image->new(); #### MANUAL: MAKE 1 OF THESE!->load( "images/clouds.png" );
	$floor = Breve::Floor->new(); #### MANUAL: MAKE 1 OF THESE!;
	$floor->catchShadows();
	$self->{ birds } = Breve::Birds->new(); #### MANUAL: MAKE 60 OF THESE!;
	$self->flockNormally();
	$self->setBackgroundTextureImage( $self->{ cloudTexture } );
	$self->offsetCamera( Breve::Vector->new( 5, 1.500000, 6 ) );
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
	my $location = ();
	my $topDiff = 0;

	$self->updateNeighbors();
	foreach $item ($self->{ birds }) {
		$item->fly();
		$location = ( $location + $item->getLocation() );

	}


	$location = ( $location / Breve::length( $self->{ birds } ) );
	$topDiff = 0.000000;
	foreach $item ($self->{ birds }) {
		if( ( $topDiff < Breve::length( ( $location - $item->getLocation() ) ) ) ) {
			$topDiff = Breve::length( ( $location - $item->getLocation() ) );
		}
;

	}


	$self->aimCamera( $location );
	if( $self->{ useDizzyCameraControl } ) {
		$self->setCameraRotation( Breve::length( $location ), 0.000000 );
	}

	$self->SUPER::iterate();
}

sub squish {
	my $self;
	( $self ) = @_;
	my $item = undef;

	foreach $item ($self->{ birds }) {
		$item->move( Breve::Vector->new( 0, 0, 0 ) );
	}


}

sub toggleDizzy {
	my $self;
	( $self ) = @_;

	$self->{ useDizzyCameraControl } = ( not $self->{ useDizzyCameraControl } );
	if( $self->{ useDizzyCameraControl } ) {
		$self->{ dizzyMenu }->check();
	}	 else {
		$self->{ dizzyMenu }->uncheck();
	}

}

package Breve::Bird;

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
	my $toNeighbor = ();
	my $centerUrge = ();
	my $worldCenterUrge = ();
	my $velocityUrge = ();
	my $spacingUrge = ();
	my $wanderUrge = ();
	my $acceleration = ();
	my $newVelocity = ();
	my $neighbors = ();
	my $takeOff = 0;

	foreach $bird ($self->getNeighbors()) {
		if( $self->checkVisibility( $bird ) ) {
			$neighbors.append( $bird );
		}
;

	}


	if( $self->{ landed } ) {
		$takeOff = Breve::randomExpression( 40 );
		if( ( $takeOff == 1 ) ) {
			$self->{ landed } = 0;
			$self->setVelocity( ( Breve::randomExpression( Breve::Vector->new( 0.100000, 1.100000, 0.100000 ) ) - Breve::Vector->new( 0.050000, 0, 0.050000 ) ) );

		}		 else {
			return;

		}
;

	}

	$centerUrge = $self->getCenterUrge( $neighbors );
	$velocityUrge = $self->getVelocityUrge( $neighbors );
	foreach $bird ($neighbors) {
		$toNeighbor = ( $self->getLocation() - $bird->getLocation() );
		if( ( Breve::length( $toNeighbor ) < $self->{ cruiseDistance } ) ) {
			$spacingUrge = ( $spacingUrge + $toNeighbor );
		}
;

	}


	if( ( Breve::length( $self->getLocation() ) > 10 ) ) {
		$worldCenterUrge = ( -$self->getLocation() );
	}

	$wanderUrge = ( Breve::randomExpression( Breve::Vector->new( 2, 2, 2 ) ) - Breve::Vector->new( 1, 1, 1 ) );
	if( Breve::length( $spacingUrge ) ) {
		$spacingUrge = ( $spacingUrge / Breve::length( $spacingUrge ) );
	}

	if( Breve::length( $worldCenterUrge ) ) {
		$worldCenterUrge = ( $worldCenterUrge / Breve::length( $worldCenterUrge ) );
	}

	if( Breve::length( $velocityUrge ) ) {
		$velocityUrge = ( $velocityUrge / Breve::length( $velocityUrge ) );
	}

	if( Breve::length( $centerUrge ) ) {
		$centerUrge = ( $centerUrge / Breve::length( $centerUrge ) );
	}

	if( Breve::length( $wanderUrge ) ) {
		$wanderUrge = ( $wanderUrge / Breve::length( $wanderUrge ) );
	}

	$wanderUrge = ( $wanderUrge * $self->{ wanderConstant } );
	$worldCenterUrge = ( $worldCenterUrge * $self->{ worldCenterConstant } );
	$centerUrge = ( $centerUrge * $self->{ centerConstant } );
	$velocityUrge = ( $velocityUrge * $self->{ velocityConstant } );
	$spacingUrge = ( $spacingUrge * $self->{ spacingConstant } );
	$acceleration = ( ( ( ( $worldCenterUrge + $centerUrge ) + $velocityUrge ) + $spacingUrge ) + $wanderUrge );
	if( ( Breve::length( $acceleration ) != 0 ) ) {
		$acceleration = ( $acceleration / Breve::length( $acceleration ) );
	}

	$self->setAcceleration( ( $self->{ maxAcceleration } * $acceleration ) );
	$newVelocity = $self->getVelocity();
	if( ( Breve::length( $newVelocity ) > $self->{ maxVelocity } ) ) {
		$newVelocity = ( ( $self->{ maxVelocity } * $newVelocity ) / Breve::length( $newVelocity ) );
	}

	$self->setVelocity( $newVelocity );
	$self->point( Breve::Vector->new( 0, 1, 0 ), ( $newVelocity / Breve::length( $newVelocity ) ) );
}

sub getAngle {
	my ($self, $otherMobile );
	( $self, $otherMobile ) = @_;
	my $tempVector = ();

	if( ( Breve::length( $self->getVelocity() ) == 0 ) ) {
		return 0;
	}

	$tempVector = ( $otherMobile->getLocation() - $self->getLocation() );
	return Breve::callInternal( $self, "angle", $self->getVelocity(), $tempVector );
}

sub getCenterUrge {
	my ($self, $flock );
	( $self, $flock ) = @_;
	my $item = undef;
	my $count = 0;
	my $center = ();

	foreach $item ($flock) {
		$count = ( $count + 1 );
		$center = ( $center + $item->getLocation() );

	}


	if( ( $count == 0 ) ) {
		return Breve::Vector->new( 0, 0, 0 );
	}

	$center = ( $center / $count );
	return ( $center - $self->getLocation() );
}

sub getVelocityUrge {
	my ($self, $flock );
	( $self, $flock ) = @_;
	my $item = undef;
	my $count = 0;
	my $aveVelocity = ();

	foreach $item ($flock) {
		$count = ( $count + 1 );
		$aveVelocity = ( $aveVelocity + $item->getVelocity() );

	}


	if( ( $count == 0 ) ) {
		return Breve::Vector->new( 0, 0, 0 );
	}

	$aveVelocity = ( $aveVelocity / $count );
	return ( $aveVelocity - $self->getVelocity() );
}

sub init {
	my $self;
	( $self ) = @_;

	$self->setShape( Breve::PolygonCone->new(); #### MANUAL: MAKE 1 OF THESE!->initWith( 3, 0.500000, 0.060000 ) );
	$self->move( ( Breve::randomExpression( Breve::Vector->new( 10, 10, 10 ) ) - Breve::Vector->new( 5, -5, 5 ) ) );
	$self->setVelocity( ( Breve::randomExpression( Breve::Vector->new( 20, 20, 20 ) ) - Breve::Vector->new( 10, 10, 10 ) ) );
	$self->setColor( Breve::randomExpression( Breve::Vector->new( 1, 1, 1 ) ) );
	$self->handleCollisions( "Floor", "land" );
	$self->setNeighborhoodSize( 3.000000 );
}

sub land {
	my ($self, $ground );
	( $self, $ground ) = @_;

	$self->setAcceleration( Breve::Vector->new( 0, 0, 0 ) );
	$self->setVelocity( Breve::Vector->new( 0, 0, 0 ) );
	$self->{ landed } = 1;
	$self->offset( Breve::Vector->new( 0, 0.001000, 0 ) );
}


1;

