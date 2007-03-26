
use strict;
use Breve;

package TheController;

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

package main;

&Breve::bootstrap();

my $g = new TheController();

1;

