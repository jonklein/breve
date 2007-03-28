
use strict;
use Breve;

package Swarm;
our @ISA = qw(Breve::Control);

sub new {
    my $class = shift;
    my $self = {};
    bless $self, $class;
    $self->{ birds } = ();
    $self->{ cloudTexture } = undef;
    $self->{ normalMenu } = undef;
    $self->{ obedientMenu } = undef;
    $self->{ selection } = undef;
    $self->{ wackyMenu } = undef;
    $self->{ floor } = undef;
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

	foreach $item (@{$self->{ birds }}) {
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
    
    $self->SUPER::init();
    
    $self->addMenu( "Smoosh The Birdies", "squish" );
    $self->addMenuSeparator();
    $self->{ obedientMenu } = $self->addMenu( "Flock Obediently", "flockObediently" );
    $self->{ normalMenu } = $self->addMenu( "Flock Normally", "flockNormally" );
    $self->{ wackyMenu } = $self->addMenu( "Flock Wackily", "flockWackily" );
    $self->enableLighting();
    $self->moveLight( Breve::Vector->new( 0, 20, 20 ) );

    my $ctex = Breve::Image->new();
    $ctex->load("images/clouds.png");
    $self->{ cloudTexture } = $ctex;
   
    $self->{floor} = Breve::Floor->new();
    $self->{floor}->catchShadows();

    for(0..50) {
	push @{$self->{ birds }}, Bird->new();
    }

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
	my $location = Breve::Vector->new();
	my $topDiff = 0;

	$self->updateNeighbors();
	foreach $item (@{$self->{ birds }}) {
	    $item->fly();
	    #$location = ( $location + $item->getLocation() ); # MANUAL NEED TO DO +
	}

	#$location = ( $location / breve->length( $self->{ birds } ) );
	$topDiff = 0.000000;
	#foreach $item ($self->{ birds }) {
	#	if( ( $topDiff < breve->length( ( $location - $item->getLocation() ) ) ) ) {
	#		$topDiff = breve->length( ( $location - $item->getLocation() ) );
	#	}
	#}

	#$self->aimCamera( $location );
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

our @ISA = qw(Breve::Mobile);

sub new {
    my $class = shift;
    my $self = {};
    bless $self, $class;
    $self->{ centerConstant } = 0;
    $self->{ cruiseDistance } = 0;
    $self->{ landed } = 0;
    $self->{ maxAcceleration } = 0;
    $self->{ maxVelocity } = 0;
    $self->{ spacingConstant } = 0;
    $self->{ velocityConstant } = 0;
    $self->{ wanderConstant } = 0;
    $self->{ worldCenterConstant } = 0;
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

	if(ref($item) ne "Bird") {
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

	my $toNeighbor = Breve::Vector->new();
	my $centerUrge = Breve::Vector->new();
	my $worldCenterUrge = Breve::Vector->new();
	my $velocityUrge = Breve::Vector->new();
	my $spacingUrge = Breve::Vector->new();
	my $wanderUrge = Breve::Vector->new();
	my $acceleration = Breve::Vector->new();
	my $newVelocity = Breve::Vector->new();
	my @neighbors = ();
	my $takeOff = 0;

	foreach my $bird (@{$self->getNeighbors()}) {

	    if( $self->checkVisibility( $bird ) ) {
		push @neighbors, $bird;
	    }
	}

	if( $self->{ landed } ) {
	    $takeOff = rand 40;
	    if( ( $takeOff == 1 ) ) {
		$self->{ landed } = 0;

		my $randvec = Breve::Vector->newRandom(0.1, 1.1, 0.1);
		my $const = Breve::Vector->new(0.05, 0, 0.05);

		$self->setVelocity( $randvec->subtract($const) );
	    } else {
		return;	
	    }
	}

	$centerUrge = $self->getCenterUrge( \@neighbors );
	$velocityUrge = $self->getVelocityUrge( \@neighbors );
	foreach my $bird (@neighbors) {
	    $toNeighbor = ( $self->getLocation()->subtract( $bird->getLocation() ) );
	    if( $toNeighbor->length() < $self->{ cruiseDistance } ) {
		$spacingUrge = ( $spacingUrge->add($toNeighbor) );
	    }
	}

	if(  $self->getLocation()->length() > 10  ) {
		$worldCenterUrge = ( $self->getLocation()->negative() );
	}

	$wanderUrge = Breve::Vector->newRandom( 2, 2, 2 )->subtract(Breve::Vector->new( 1, 1, 1 ) );

	if( $spacingUrge->length() ) {
	    $spacingUrge->normalize_in_place();
	}

	if( $worldCenterUrge->length() ) {
	    $worldCenterUrge->normalize_in_place();
	}

	if( $velocityUrge->length() ) {
	    $velocityUrge->normalize_in_place();
	}

	if( $centerUrge->length() ) {
	    $centerUrge->normalize_in_place();
	}

	if( $wanderUrge->length() ) {
	    $wanderUrge->normalize_in_place();
	}

	$wanderUrge = ( $wanderUrge->multiplyBy( $self->{ wanderConstant } ));
	
	$worldCenterUrge = ( $worldCenterUrge->multiplyBy( $self->{ worldCenterConstant }));
	$centerUrge = ( $centerUrge->multiplyBy( $self->{ centerConstant } ));
	$velocityUrge = ( $velocityUrge->multiplyBy( $self->{ velocityConstant } ));
	$spacingUrge = ( $spacingUrge->multiplyBy( $self->{ spacingConstant } ));

	$acceleration = ( ( ( ( $worldCenterUrge
				->add( $centerUrge ) )
			      ->add( $velocityUrge) )
			    ->add( $spacingUrge ) )
			  ->add( + $wanderUrge) );

	if( $acceleration->length() != 0 ) {
	    $acceleration->normalize_in_place();
	}

	$self->setAcceleration( $acceleration->multiplyBy($self->{ maxAcceleration } ) );
	$newVelocity = $self->getVelocity();

	if( $newVelocity->length() > $self->{ maxVelocity } ) {
	    $newVelocity = $newVelocity->multiplyBy($self->{maxVelocity})
		                       ->divideBy($newVelocity->length());
	}

	$self->setVelocity( $newVelocity );

	if( $newVelocity->length() != 0 ) {
	    $newVelocity->normalize_in_place();
	}

	$self->point( Breve::Vector->new( 0, 1, 0 ), $newVelocity );

}

sub getAngle {
    my ($self, $otherMobile );
    ( $self, $otherMobile ) = @_;
    my $tempVector;

    if( $self->getVelocity()->length() == 0 ) {
	return 0;
    }
    
    $tempVector = ( $otherMobile->getLocation()->subtract($self->getLocation() ));

    my $velocity = $self->getVelocity();

    return Breve::callInternal($self, "angle", $velocity, $tempVector );
}


sub getCenterUrge {
	my ($self, $flock );
	( $self, $flock ) = @_;
	my $item = undef;
	my $count = 0;
	my $center = Breve::Vector->new(0, 0, 0);

	foreach $item (@{$flock}) {
	    $count = ( $count + 1 );
	    $center = ( $center->add( $item->getLocation() ) );
	}

	if( ( $count == 0 ) ) {
	    return Breve::Vector->new( 0, 0, 0 );
	}

	$center = $center->divideBy($count);
	return ( $center->subtract($self->getLocation()) );
}

sub getVelocityUrge {
	my ($self, $flock );
	( $self, $flock ) = @_;

	my $count = 0;
	my $aveVelocity = Breve::Vector->new();

	foreach my $item (@{$flock}) {
		$count = ( $count + 1 );
		$aveVelocity = ( $aveVelocity->add( $item->getVelocity() ) );

	}


	if( ( $count == 0 ) ) {
		return Breve::Vector->new( 0, 0, 0 );
	}

	$aveVelocity = ( $aveVelocity->divideBy( $count ) );
	return ( $aveVelocity->subtract( $self->getVelocity() ) );
}

sub init {
	my $self;
	( $self ) = @_;

	$self->SUPER::init();

	my $cone = Breve::PolygonCone->new();
	$cone->initWith(3, 0.5, 0.06);
	$self->setShape( $cone );

	my $random_place = Breve::Vector->newRandom(10,10,10); 
	my $random_velocity = Breve::Vector->newRandom(20,20,20); 

	$self->move( $random_place->subtract( Breve::Vector->new( 5, -5, 5 ) ) );
	$self->setVelocity( $random_velocity->subtract(Breve::Vector->new(10,10,10)));
	$self->setColor( Breve::Vector->newRandom( 1, 1, 1 ) );
	$self->handleCollisions( "Breve::Floor", "land" );
	
	$self->setNeighborhoodSize( 3.000000 );

}

sub land {
	my ($self, $ground );
	( $self, $ground ) = @_;

	die("don't know how to land.");

	$self->setAcceleration( breve->vector( 0, 0, 0 ) );
	$self->setVelocity( breve->vector( 0, 0, 0 ) );
	$self->{ landed } = 1;
	$self->offset( breve->vector( 0, 0.001000, 0 ) );
}

package main;

&Breve::bootstrap();
my $SwarmController = Swarm->new();

1;

