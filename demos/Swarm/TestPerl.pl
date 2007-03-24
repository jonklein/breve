
use strict;
use Breve;
use Devel::Symdump;

package God;

our @ISA = qw(Breve::Control);

sub new {
	my $class = shift;
	my $self = {};
	bless $self, $class;

   $self->init();

   return $self;
}

sub init {
	my $self;
	( $self ) = @_;
   $self->SUPER::init();
}

#sub iterate {
#	my $self;
#	( $self ) = @_;
#}

#sub post_iterate {
#	my $self;
#	( $self ) = @_;
#}

package main;

&Breve::bootstrap();

my $g = new God();

#$g->setIterationStep(2);

1;

